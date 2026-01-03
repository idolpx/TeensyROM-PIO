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

//  TeensyROM: A C64 ROM emulator and loader/interface cartidge based on the Teensy 4.1
//  Copyright (c) 2023 Travis Smith <travis@sensoriumembedded.com>

#include <Arduino.h>

#include <EEPROM.h>

#include "Common_Defs.h"
#include "nfcScan.h"
#include "ISRs.h"
#include "SendMsg.h"
#include "eeprom_dev.h"
#include "filesystem.h"
#include "SerUSBIO.h"
#include "ServiceTCP.h"


extern "C" uint32_t set_arm_clock (uint32_t frequency);
extern float tempmonGetTemp (void);

void setup_min();
void setup_max();
void loop_min();
void loop_max();

void setup()
{
    set_arm_clock (816000000); //slight overclocking, no cooling required

    Serial.begin (115200);
    if (CrashReport)
        Serial.print (CrashReport);

    for (uint8_t PinNum = 0; PinNum < sizeof (OutputPins); PinNum++)
        pinMode (OutputPins[PinNum], OUTPUT);

    DataBufDisable;    // buffer disabled
    SetDataPortDirOut; // default to output (for C64 Read)
    SetDMADeassert;
    SetIRQDeassert;
    SetNMIDeassert;
    SetResetAssert; // assert reset until main loop()

#ifdef DbgSignalSenseReset
    pinMode (DotClk_Debug_PIN, INPUT_PULLUP); //use Dot_Clk input as reset sense input
#else
#ifdef DbgFab0_3plus
    pinMode (DotClk_Debug_PIN, OUTPUT); //p28 is Debug output on fab 0.3+
    SetDebugDeassert;
#else
    pinMode (DotClk_Debug_PIN, INPUT_PULLUP); //p28 is Dot_Clk input (unused) on fab 0.2x
#endif
#endif

    for (uint8_t PinNum = 0; PinNum < sizeof (InputPins); PinNum++)
        pinMode (InputPins[PinNum], INPUT);

    pinMode (Reset_Btn_In_PIN, INPUT_PULLUP); //also makes it Schmitt triggered (PAD_HYS)
    pinMode (PHI2_PIN, INPUT_PULLUP);  //also makes it Schmitt triggered (PAD_HYS)
    attachInterrupt ( digitalPinToInterrupt (Reset_Btn_In_PIN), isrButton, FALLING );
    attachInterrupt ( digitalPinToInterrupt (PHI2_PIN), isrPHI2, RISING );
    NVIC_SET_PRIORITY (IRQ_GPIO6789, 16); //set HW ints as high priority, otherwise ethernet int timer causes misses
    // Minimal boot: check if we should run minimal mode or full app
#ifdef Dbg_TestMin

    //EEPwriteStr(eepAdCrtBootName, "/OneLoad v5/Main- MagicDesk CRTs/Auriga.crt");
    //EEPwriteStr(eepAdCrtBootName, "/validation/FileSize/802k Briley Witch Chronicles 2 v1.0.3.crt");
    //EEPwriteStr(eepAdCrtBootName, "/validation/FileSize/770k Where in USA is Carmen Sandiego [EasyFlash].crt");  //PAL only
        // Not executing minimal boot, would jump to main app
        // For now, continue with setup for testing
    //EEPwriteStr(eepAdCrtBootName, "/validation/crts/32_EasyFlash/tools/Digi Player Demo (2012-06-17)(Onslaught)[EasyFlash 2012-06-17].crt");
    //EEPwriteStr(eepAdCrtBootName, "/validation/crts/32_EasyFlash/tools/Multi Easy (2013-04-11)(Lord Crass)[EasyFlash].crt");                                                                                                                                       //  Swaps?   performance
    //EEPwriteStr(eepAdCrtBootName, "/validation/crts/32_EasyFlash/commercial_20XX_releases/A_Pig_Quest_1.02_ef.crt");                            //  some     no fails observed
    //EEPwriteStr(eepAdCrtBootName, "/validation/crts/32_EasyFlash/commercial_20XX_releases/A Pig Quest +2 {EasyFlash}[EX].crt");             //not on SD!
    //EEPwriteStr(eepAdCrtBootName, "/validation/crts/32_EasyFlash/commercial_20XX_releases/a_pig_quest_v102_+9_[trex].crt");               //not on SD!
    //EEPwriteStr(eepAdCrtBootName, "/validation/crts/32_EasyFlash/oneload64v4/Extras/OtherCRTs/Turrican & Turrican II [EasyFlash].crt");
    //EEPwriteStr(eepAdCrtBootName, "/validation/crts/32_EasyFlash/oneload64v4/AlternativeFormats/EasyFlash/OneLoad64-Vol#5.crt");
    //EEPwriteStr(eepAdCrtBootName, "/validation/FileSize/882k Maniac Mansion & Zak McKracken [EasyFlash].crt");
    //EEPwriteStr(eepAdCrtBootName, "/validation/FileSize/882k Last Ninja 1 + 2, The [EasyFlash].crt");

    //EEPwriteStr(eepAdCrtBootName, "/validation/crts/ezf 48Khz_hifi_Elvis_Costello_[EASYFLASH].crt");   //good test of all banks, *does not* click during swaps at the end
    //EEPwriteStr(eepAdCrtBootName, "/validation/crts/32_EasyFlash/Other-Large/hf_audio_playback_01.crt"); //good test of all banks, clicks during swaps at the end
    //EEPwriteStr(eepAdCrtBootName, "/validation/FileSize/954k Eye of the Beholder - v1.00 [EasyFlash].crt");        //swaps quickly during play                            //   Lots!
    //EEPwriteStr(eepAdCrtBootName, "/validation/crts/32_EasyFlash/Other-Large/svc64_update2.crt");  //SNK vs CAPCOM,  swaps quickly during play                                //   Lots!
    //EEPwriteStr(eepAdCrtBootName, "/svc64_md2.crt");
    //EEPwriteStr(eepAdCrtBootName, "/validation/FileSize/Very Large CRTs/svc64_md2.crt");  //SNK vs CAPCOM Strong Edition: Magic Desk 2
    EEPwriteStr (eepAdCrtBootName, "/validation/FileSize/Very Large CRTs/SNKvsCap/svc64_stronger.crt"); //SNK vs CAPCOM Strong Edition: Magic Desk 2

    EEPROM.write (eepAdMinBootInd, MinBootInd_ExecuteMin);
#endif

    // Determine which mode to run based on EEPROM setting
    bTeensyROMRunMode = true; // Default to full TeensyROM mode
    uint32_t MagNumRead;
    EEPROM.get (eepAdMagicNum, MagNumRead);
    if (MagNumRead == eepMagicNum) // valid EEPROM
    {
        // EEPROM not initialized
        if (EEPROM.read (eepAdMinBootInd) == MinBootInd_ExecuteMin || ReadButton == 0)
        {
            bTeensyROMRunMode = false; // MinimalBoot mode
        }
    }


    if (!bTeensyROMRunMode)
        setup_min();
    else
        setup_max();

    SetLEDOn; // done last as indicator of init completion
}
    //     setup_min();
void setup_min()
{
    EEPROM.write (eepAdMinBootInd, MinBootInd_SkipMin);

    // MinimalBuild: simpler initialization
    LOROM_Image = NULL;
    HIROM_Image = NULL;
    LOROM_Mask = HIROM_Mask = 0x1fff;
    EmulateVicCycles = false;
    FreeCrtChips();

    strcpy (DriveDirPath, "/");
    SD.begin (BUILTIN_SDCARD); // refresh, takes 3 seconds for fail/unpopulated, 20-200mS populated

    BigBuf = (uint32_t *)malloc (BigBufSize * sizeof (uint32_t));

    Serial.printf ("\nTeensyROM minimal %s is on-line\n", strVersionNumber);
    Serial.printf (" %luMHz  %.1fC\n", (F_CPU_ACTUAL / 1000000), tempmonGetTemp());

#ifdef Dbg_TestMin
    //calc/show free RAM space for CRT:
    uint32_t CrtMax = (RAM_ImageSize & 0xffffe000) / 1024; //round down to k bytes rounded to nearest 8k
    Serial.printf (" RAM1    Buffer: %luK (%lu blks)\n", CrtMax, CrtMax / 8);
    Serial.printf (" RAM1 Swap Blks: %luK (%lu blks)\n", Num8kSwapBuffers * 8, Num8kSwapBuffers);
    uint8_t NumChips = RAM2blocks();
    //Serial.printf("RAM2 Blks: %luK (%lu blks)\n", NumChips*8, NumChips);
    NumChips = RAM2blocks() - 1; //do it again, sometimes get one more, minus one to match reality, not clear why
    Serial.printf (" RAM2      Blks: %luK (%lu blks)\n", NumChips * 8, NumChips);
    CrtMax += NumChips * 8 + Num8kSwapBuffers * 8;
    Serial.printf (" %luk max RAM for CRT w/o swaps\n", (uint32_t) (CrtMax * 1.004)); //larger File size due to header info.
#endif

    // assuming it's a .crt file, and present on SD drive (verified in main image)
    char *CrtBootNamePath = (char*)malloc (MaxPathLength);
    EEPreadNBuf (eepAdCrtBootName, (uint8_t*)CrtBootNamePath, MaxPathLength); //load the source/path/name from EEPROM
    Serial.printf ("Sel CRT: %s\n", CrtBootNamePath);
    LoadCRT (CrtBootNamePath);
    if (!doReset)
    {
        Serial.print ("CRT not loaded, Abort!\n");
        bTeensyROMRunMode = true; //safety
    }
}

void setup_max()
{
    myusbHost.begin(); // Start USBHost_t36, HUB(s) and USB devices.

    uint32_t MagNumRead;
    EEPROM.get (eepAdMagicNum, MagNumRead);
    if (MagNumRead != eepMagicNum)
        SetEEPDefaults();

    // Full TeensyROM initialization
    IO1 = (uint8_t *)calloc (IO1Size, sizeof (uint8_t)); // allocate IO1 space and init to 0
    IO1[rwRegStatus] = rsReady;
    IO1[rWRegCurrMenuWAIT] = rmtTeensy;
    IO1[rRegPresence1] = 0x55;
    IO1[rRegPresence2] = 0xAA;
    for (uint8_t reg = rRegSIDStrStart; reg < rRegSIDStringTerm; reg++)
        IO1[reg] = ' ';
    IO1[rRegSIDStringTerm] = 0;
    IO1[rwRegPwrUpDefaults] = EEPROM.read (eepAdPwrUpDefaults);
    IO1[rwRegPwrUpDefaults2] = EEPROM.read (eepAdPwrUpDefaults2);
    IO1[rwRegTimezone] = EEPROM.read (eepAdTimezone);
    for (uint8_t reg = 0; reg < NumColorRefs; reg++)
        IO1[rwRegColorRefStart + reg] = EEPROM.read (eepAdColorRefStart + reg);
    // IO1[rwRegNextIOHndlr] = EEPROM.read(eepAdNextIOHndlr); //done each entry into menu
    SetUpMainMenuROM();
    MenuChange(); // set up drive path, menu source/size

    for (uint8_t cnt = 0; cnt < IOH_Num_Handlers; cnt++)
        PadSpace (IOHandler[cnt]->Name, IOHNameLength - 1); // done so selection shown on c64 overwrites previous

    SwiftBrowserInit();

    StrSIDInfo = (char *)calloc (StrSIDInfoSize, sizeof (char)); // SID header info storage
    LatestSIDLoaded = (char *)malloc (MaxPathLength);          // Last loaded Source/SID path/filename
    BigBuf = (uint32_t *)malloc (BigBufSize * sizeof (uint32_t));

    MakeBuildInfo();
    Serial.printf ("\n%s\nTeensyROM %s is on-line\n", SerialStringBuf, strVersionNumber);
    Printf_dbg ("Debug messages enabled!\n");
    Printf_dbg_sw ("Swiftlink debug messages enabled!\n");

    if (IO1[rwRegPwrUpDefaults2] & rpud2NFCEnabled)
        nfcInit(); // connect to nfc scanner

    if (IO1[rwRegPwrUpDefaults2] & rpud2TRContEnabled) // connect to TR Control device
    {   // takes 200mS typical, 5 seconds if usb serial device not present!
        // takes 200mS typical, 5 seconds if usb serial device not present!
        USBHostSerial.begin (115200, USBHOST_SERIAL_8N1); // 115200 460800 2000000
        Serial.println ("USB Host Control Enabled");
        // USBHostSerial.printf("USB Host Serial Control Ready\n");
    }

    switch (EEPROM.read (eepAdMinBootInd))
    {
        case MinBootInd_SkipMin: // normal first power up
            if (ReadButton != 0) // skip autolaunch checks if button pressed
            {
                uint32_t AutoStartmS = millis();
                if (!CheckLaunchSDAuto()) // if nothing autolaunched from SD autolaunch file
                {
                    if (EEPROM.read (eepAdAutolaunchName) && (ReadButton != 0)) // If name is non zero length & button not pressed
                    {
                        EEPRemoteLaunch (eepAdAutolaunchName);
                    }
                }
                Printf_dbg ("Autolaunch checks: %lumS\n", millis() - AutoStartmS);
            }
            break;

        case MinBootInd_LaunchFull: // Launch command received in minimal, launch it from full
            EEPROM.write (eepAdMinBootInd, MinBootInd_SkipMin);
            EEPRemoteLaunch (eepAdCrtBootName);
            break;

        default: // ignore anything else (most likely MinBootInd_FromMin), set back to default for next time
            EEPROM.write (eepAdMinBootInd, MinBootInd_SkipMin);
            break;
    }
}

void loop()
{
    // Runtime mode selection
    if (!bTeensyROMRunMode)
        loop_min();
    else
        loop_max();

    if (Serial.available())
        ServiceSerial();

#ifdef FeatTCPListen
    if (NetListenEnable && bTeensyROMRunMode)  // TCP only in full mode
    {
        EthernetClient tcpclient = tcpServer.available();
        if (tcpclient)
            ServiceTCP (tcpclient);
    }
#endif

    // handler specific polling items:
    if (IOHandler[CurrentIOHandler]->PollingHndlr != NULL)
        IOHandler[CurrentIOHandler]->PollingHndlr();
}

void loop_min()
{
    // MinimalBuild: simplified loop
    if (BtnPressed)
    {
        Serial.print ("Button detected (minimal)\n");
        // In minimal mode, button press could trigger reboot or return to main app
        BtnPressed = false;
    }

    if (doReset)
    {
        SetResetAssert;
        Serial.println ("Resetting C64");
        Serial.flush();
        delay (50);
        doReset = false;
        SetResetDeassert;
    }
}

void loop_max()
{
    // Full build loop
    if (BtnPressed)
    {
        Serial.print ("Button detected\n");
        SetLEDOn;
        BtnPressed = false;
        IO1[rwRegIRQ_CMD] = ricmdNone; // just to be sure, should already be 0/none
        if (RemoteLaunched)
        {
            IO1[rWRegCurrMenuWAIT] = rmtTeensy;
            MenuChange();
            RemoteLaunched = false;
            Printf_dbg ("Remote recovery\n");
        }
        if (IO1[rwRegPwrUpDefaults2] & rpud2NFCEnabled)
            nfcInit();      // connect to nfc scanner
        SetUpMainMenuROM(); // back to main menu, also sets doReset
    }

    if (doReset)
    {
        SetResetAssert;
        Serial.println ("Resetting C64");
        Serial.flush();
        delay (50);
        uint32_t NextInterval = 10000, beginWait = millis();
        bool LEDState = true, DefEEPReboot = false;
        while (ReadButton == 0)
        { // avoid self reset detection, check for long press
            // avoid self reset detection, check for long press
            if (millis() - beginWait > NextInterval)
            {
                DefEEPReboot = true;
                NextInterval += 150;
                LEDState = !LEDState;
                if (LEDState)
                    SetLEDOn;
                else
                    SetLEDOff;
            }
        }
        if (DefEEPReboot)
        {
            SetEEPDefaults();
            REBOOT;
        }
        doReset = false;
        BtnPressed = false;
        SetResetDeassert;
    }

    myusbHost.Task();

    if (nfcState == nfcStateEnabled)
        nfcCheck();

    if (IO1[rwRegPwrUpDefaults2] & rpud2TRContEnabled)
        if (USBHostSerial.available())
            ServiceSerial (&USBHostSerial);
}
