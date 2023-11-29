// MIT License
//
// Copyright (c) 2023 Travis Smith
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software
// and associated documentation files (the "Software"), to deal in the Software without
// restriction, including without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom
// the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or
// substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
// BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

// IO Handler for TeensyROM

#include "IOH_TeensyROM.h"

#include <SD.h>
#include <EEPROM.h>

#include "ethernet_dev.h"
#include "midi2sid.h"

#include "../../include/TeensyROM.h"
#include "DriveDirLoad.h"

#include "eeprom_dev.h"

void getNtpTime()
{
    if (!EthernetInit())
    {
        IO1[rRegLastSecBCD] = 0;
        IO1[rRegLastMinBCD] = 0;
        IO1[rRegLastHourBCD] = 0;
        return;
    }

    unsigned int localPort = 8888;               // local port to listen for UDP packets
    const char timeServer[] = "us.pool.ntp.org"; // time.nist.gov     NTP server

    udp.begin (localPort);

    const int NTP_PACKET_SIZE = 48;     // NTP time stamp is in the first 48 bytes of the message
    byte packetBuffer[NTP_PACKET_SIZE]; // buffer to hold incoming and outgoing packets

    Serial.printf ("Updating time from: %s\n", timeServer);
    while (udp.parsePacket() > 0)
        ; // discard any previously received packets

    // send an NTP request to the time server at the given address
    // set all bytes in the buffer to 0
    memset (packetBuffer, 0, NTP_PACKET_SIZE);
    // Initialize values needed to form NTP request
    packetBuffer[0] = 0b11100011; // LI, Version, Mode
    packetBuffer[1] = 0;          // Stratum, or type of clock
    packetBuffer[2] = 6;          // Polling Interval
    packetBuffer[3] = 0xEC;       // Peer Clock Precision
    // 8 bytes of zero for Root Delay & Root Dispersion
    packetBuffer[12] = 49;
    packetBuffer[13] = 0x4E;
    packetBuffer[14] = 49;
    packetBuffer[15] = 52;
    // all NTP fields have been given values, now send a packet requesting a timestamp:
    udp.beginPacket (timeServer, 123); // NTP requests are to port 123
    udp.write (packetBuffer, NTP_PACKET_SIZE);
    udp.endPacket();

    uint32_t beginWait = millis();
    while (millis() - beginWait < 1500)
    {
        int size = udp.parsePacket();
        if (size >= NTP_PACKET_SIZE)
        {
            udp.read (packetBuffer, NTP_PACKET_SIZE); // read packet into the buffer
            uint32_t secsSince1900;
            // convert four bytes starting at location 40 to a long integer
            secsSince1900 = (unsigned long)packetBuffer[40] << 24;
            secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
            secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
            secsSince1900 |= (unsigned long)packetBuffer[43];
            Serial.printf ("Received NTP Response in %d mS\n", (millis() - beginWait));

            // since we don't need the date, leaving out TimeLib.h all together
            IO1[rRegLastSecBCD] = DecToBCD (secsSince1900 % 60);
            secsSince1900 /= 60; // to  minutes
            IO1[rRegLastMinBCD] = DecToBCD (secsSince1900 % 60);
            secsSince1900 = (secsSince1900 / 60 + (int8_t)IO1[rwRegTimezone]) % 24; // to hours, offset timezone
            if (secsSince1900 >= 12)
                IO1[rRegLastHourBCD] = 0x80 | DecToBCD (secsSince1900 - 12); // change to 0 based 12 hour and add pm flag
            else
                IO1[rRegLastHourBCD] = DecToBCD (secsSince1900); // default to AM (bit 7 == 0)

            Serial.printf ("Time: %02x:%02x:%02x %sm\n", (IO1[rRegLastHourBCD] & 0x7f), IO1[rRegLastMinBCD], IO1[rRegLastSecBCD], (IO1[rRegLastHourBCD] & 0x80) ? "p" : "a");
            return;
        }
    }
    Serial.println ("NTP Response timeout!");
}

void WriteEEPROM()
{
    Printf_dbg ("Wrote $%02x to EEP addr %d\n", eepDataToWrite, eepAddrToWrite);
    EEPROM.write (eepAddrToWrite, eepDataToWrite);
}

void MakeBuildCPUInfoStr()
{
    // Serial.printf("\nBuild Date/Time: %s  %s\nCPU Freq: %lu MHz   Temp: %.1f°C\n", __DATE__, __TIME__, (F_CPU_ACTUAL/1000000), tempmonGetTemp());
    sprintf (SerialStringBuf, " Build Date/Time: %s, %s\r\n    Teensy Freq: %luMHz  Temp: %.1fC\r\n", __DATE__, __TIME__, (F_CPU_ACTUAL / 1000000), tempmonGetTemp());
}

void UpDirectory()
{
    // non-root of Teensy, SD or USB drive only
    if (PathIsRoot())
        return;

    if (IO1[rWRegCurrMenuWAIT] == rmtTeensy)
        MenuChange(); // back to root, only 1 dir level
    else
    {
        char *LastSlash = strrchr (DriveDirPath, '/'); // find last slash
        if (LastSlash == NULL)
            return;
        LastSlash[0] = 0; // terminate it there
        if (IO1[rWRegCurrMenuWAIT] == rmtSD)
            LoadDirectory (&SD);
        else
            LoadDirectory (&firstPartition);
        IO1[rwRegCursorItemOnPg] = 0;
        IO1[rwRegPageNumber] = 1;
    }
}

void SearchForLetter()
{
    uint16_t ItemNum = 0;
    uint8_t SearchFor = IO1[wRegSearchLetterWAIT];

    // ascii upper case (toupper) matches petscii lower case ('a'=65)
    while (ItemNum < NumItemsFull)
    {
        if (toupper (MenuSource[ItemNum].Name[0]) >= SearchFor)
        {
            IO1[rwRegPageNumber] = ItemNum / MaxItemsPerPage + 1;
            IO1[rwRegCursorItemOnPg] = ItemNum % MaxItemsPerPage;
            IO1[rRegNumItemsOnPage] = (NumItemsFull > IO1[rwRegPageNumber] * MaxItemsPerPage ? MaxItemsPerPage : NumItemsFull - (IO1[rwRegPageNumber] - 1) * MaxItemsPerPage);
            return;
        }
        ItemNum++;
    }
}

void LoadMainSIDforXfer()
{
    XferImage = RAM_Image;
    XferSize = sizeof (SIDforBackground);
    memcpy (XferImage, SIDforBackground, XferSize);
    ParseSIDHeader(); // returns pass/fail, but assuming it passes for buit-in
}

void (*StatusFunction[rsNumStatusTypes])() = // match RegStatusTypes order
{
    &MenuChange,          // rsChangeMenu
    &HandleExecution,     // rsStartItem
    &getNtpTime,          // rsGetTime
    &IOHandlerInitToNext, // rsIOHWinit
    &WriteEEPROM,         // rsWriteEEPROM
    &MakeBuildCPUInfoStr, // rsMakeBuildCPUInfoStr
    &UpDirectory,         // rsUpDirectory
    &SearchForLetter,     // rsSearchForLetter
    &LoadMainSIDforXfer,  // rsLoadSIDforXfer
};


//__________________________________________________________________________________

void InitHndlr_TeensyROM()
{
    IO1[rwRegNextIOHndlr] = EEPROM.read (eepAdNextIOHndlr); // in case it was over-ridden by .crt
    // MIDI handlers for MIDI2SID:
    usbHostMIDI.setHandleNoteOff (M2SOnNoteOff);            // 8x
    usbHostMIDI.setHandleNoteOn (M2SOnNoteOn);              // 9x
    usbHostMIDI.setHandleControlChange (M2SOnControlChange); // Bx
    usbHostMIDI.setHandlePitchChange (M2SOnPitchChange);    // Ex

    usbDevMIDI.setHandleNoteOff (M2SOnNoteOff);            // 8x
    usbDevMIDI.setHandleNoteOn (M2SOnNoteOn);              // 9x
    usbDevMIDI.setHandleControlChange (M2SOnControlChange); // Bx
    usbDevMIDI.setHandlePitchChange (M2SOnPitchChange);    // Ex
}

void IO1Hndlr_TeensyROM (uint8_t Address, bool R_Wn)
{
    uint8_t Data;
    if (R_Wn) // High (IO1 Read)
    {
        switch (Address)
        {
            case rRegItemTypePlusIOH:
                Data = MenuSource[SelItemFullIdx].ItemType;
                if (IO1[rWRegCurrMenuWAIT] == rmtTeensy && MenuSource[SelItemFullIdx].IOHndlrAssoc != IOH_None)
                    Data |= 0x80; // bit 7 indicates an assigned IOHandler
                DataPortWriteWaitLog (Data);
                break;
            case rRegStreamData:
                DataPortWriteWait (XferImage[StreamOffsetAddr]);
                // inc on read, check for end:
                if (++StreamOffsetAddr >= XferSize)
                    IO1[rRegStrAvailable] = 0; // signal end of transfer
                break;
            case rwRegSerialString:
                Data = ptrSerialString[StringOffset++];
                DataPortWriteWaitLog (ToPETSCII (Data));
                break;
            default: // used for all other IO1 reads
                DataPortWriteWaitLog (IO1[Address]);
                break;
        }
    }
    else // IO1 write
    {
        Data = DataPortWaitRead();
        TraceLogAddValidData (Data);
        switch (Address)
        {
            case rwRegSelItemOnPage:
                SelItemFullIdx = Data + (IO1[rwRegPageNumber] - 1) * MaxItemsPerPage;
            case rwRegStatus:
            case wRegVid_TOD_Clks:
            case rwRegSIDSpeedHi:
            case rwRegSIDSpeedLo:
            case wRegIRQ_ACK:
            case rwRegIRQ_CMD:
            case rwRegCursorItemOnPg:
                IO1[Address] = Data;
                break;

            case rwRegPageNumber:
                IO1[rwRegPageNumber] = Data;
                IO1[rRegNumItemsOnPage] = (NumItemsFull > Data * MaxItemsPerPage ? MaxItemsPerPage : NumItemsFull - (Data - 1) * MaxItemsPerPage);
                break;
            case rwRegNextIOHndlr:
                if (Data & 0x80)
                    Data = LastSelectableIOH; // wrap around to last item if negative
                else
                    if (Data > LastSelectableIOH)
                        Data = 0; // wrap around to first item if above max
                IO1[rwRegNextIOHndlr] = Data;
                eepAddrToWrite = eepAdNextIOHndlr;
                eepDataToWrite = Data;
                IO1[rwRegStatus] = rsWriteEEPROM; // work this in the main code
                break;
            case rWRegCurrMenuWAIT:
                IO1[rWRegCurrMenuWAIT] = Data;
                IO1[rwRegStatus] = rsChangeMenu; // work this in the main code
                break;
            case rwRegPwrUpDefaults:
                IO1[rwRegPwrUpDefaults] = Data;
                eepAddrToWrite = eepAdPwrUpDefaults;
                eepDataToWrite = Data;
                IO1[rwRegStatus] = rsWriteEEPROM; // work this in the main code
                break;
            case rwRegTimezone:
                IO1[rwRegTimezone] = Data;
                eepAddrToWrite = eepAdTimezone;
                eepDataToWrite = Data;
                IO1[rwRegStatus] = rsWriteEEPROM; // work this in the main code
                break;
            case wRegSearchLetterWAIT:
                IO1[wRegSearchLetterWAIT] = Data;
                IO1[rwRegStatus] = rsSearchForLetter; // work this in the main code
                break;
            case rwRegSerialString: // Select/build(no waiting) string to set ptrSerialString and read out serially
                StringOffset = 0;
                switch (Data)
                {
                    case rsstItemName:
                        memcpy (SerialStringBuf, MenuSource[SelItemFullIdx].Name, MaxItemDispLength);
                        SerialStringBuf[MaxItemDispLength - 1] = 0;
                        ptrSerialString = SerialStringBuf;
                        break;
                    case rsstNextIOHndlrName:
                        ptrSerialString = IOHandler[IO1[rwRegNextIOHndlr]]->Name;
                        break;
                    case rsstSerialStringBuf:
                        // assumes SerialStringBuf built first...(FWUpd msg or BuildInfo)
                        ptrSerialString = SerialStringBuf;
                        break;
                    case rsstVersionNum:
                        ptrSerialString = strVersionNumber;
                        break;
                    case rsstShortDirPath:
                    {
                        uint16_t Len = strlen (DriveDirPath);
                        if (Len >= 40)
                        {
                            strcpy (SerialStringBuf, "...");
                            strcat (SerialStringBuf, DriveDirPath + Len - 36);
                            ptrSerialString = SerialStringBuf;
                        }
                        else
                            ptrSerialString = DriveDirPath;
                    }
                    break;
                }
                break;

            case wRegControl:
                switch (Data)
                {
                    case rCtlVanishROM:
                        SetGameDeassert;
                        SetExROMDeassert;
                        LOROM_Image = NULL;
                        HIROM_Image = NULL;
                        break;
                    case rCtlBasicReset:
                        // SetLEDOff;
                        doReset = true;
                        IO1[rwRegStatus] = rsIOHWinit; // Support IO handlers at reset
                        break;
                    case rCtlStartSelItemWAIT:
                        IO1[rwRegStatus] = rsStartItem; // work this in the main code
                        break;
                    case rCtlGetTimeWAIT:
                        IO1[rwRegStatus] = rsGetTime; // work this in the main code
                        break;
                    case rCtlRunningPRG:
                        IO1[rwRegStatus] = rsIOHWinit; // Support IO handlers in PRG
                        break;
                    case rCtlMakeInfoStrWAIT:
                        IO1[rwRegStatus] = rsMakeBuildCPUInfoStr; // work this in the main code
                        break;
                    case rCtlUpDirectoryWAIT:
                        IO1[rwRegStatus] = rsUpDirectory; // work this in the main code
                        break;
                    case rCtlLoadSIDWAIT:
                        IO1[rwRegStatus] = rsLoadSIDforXfer; // work this in the main code
                        break;
                }
                break;
        }
    } // write
}

void PollingHndlr_TeensyROM()
{
    if (IO1[rwRegStatus] != rsReady)
    {
        // ISR requested work
        if (IO1[rwRegStatus] < rsNumStatusTypes)
            StatusFunction[IO1[rwRegStatus]]();
        else
            Serial.printf ("?Stat: %02x\n", IO1[rwRegStatus]);
        Serial.flush();
        IO1[rwRegStatus] = rsReady;
    }
    usbHostMIDI.read();
    usbDevMIDI.read();
}
