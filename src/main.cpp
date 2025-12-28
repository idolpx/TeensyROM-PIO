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
#include "filesystem.h"


extern "C" uint32_t set_arm_clock(uint32_t frequency);
extern float tempmonGetTemp(void);

void setup()
{
    set_arm_clock(816000000); // slight overclocking, no cooling required

    Serial.begin(115200);
    if (CrashReport)
        Serial.print(CrashReport);

    for (uint8_t PinNum = 0; PinNum < sizeof(OutputPins); PinNum++)
        pinMode(OutputPins[PinNum], OUTPUT);
    DataBufDisable;    // buffer disabled
    SetDataPortDirOut; // default to output (for C64 Read)
    SetDMADeassert;
    SetIRQDeassert;
    SetNMIDeassert;
    SetResetAssert; // assert reset until main loop()
#ifdef DbgFab0_3plus
    pinMode(DotClk_Debug_PIN, OUTPUT); // p28 is Debug output on fab 0.3+
    SetDebugDeassert;
#else
    pinMode(DotClk_Debug_PIN, INPUT_PULLUP); // p28 is Dot_Clk input (unused) on fab 0.2x
#endif

    for (uint8_t PinNum = 0; PinNum < sizeof(InputPins); PinNum++)
        pinMode(InputPins[PinNum], INPUT);
    pinMode(Reset_Btn_In_PIN, INPUT_PULLUP); // also makes it Schmitt triggered (PAD_HYS)
    pinMode(PHI2_PIN, INPUT_PULLUP);         // also makes it Schmitt triggered (PAD_HYS)
    attachInterrupt(digitalPinToInterrupt(Reset_Btn_In_PIN), isrButton, FALLING);
    attachInterrupt(digitalPinToInterrupt(PHI2_PIN), isrPHI2, RISING);
    NVIC_SET_PRIORITY(IRQ_GPIO6789, 16); // set HW ints as high priority, otherwise ethernet int timer causes misses

    myusbHost.begin(); // Start USBHost_t36, HUB(s) and USB devices.

    uint32_t MagNumRead;
    EEPROM.get(eepAdMagicNum, MagNumRead);
    if (MagNumRead != eepMagicNum)
        SetEEPDefaults();

    IO1 = (uint8_t *)calloc(IO1Size, sizeof(uint8_t)); // allocate IO1 space and init to 0
    IO1[rwRegStatus] = rsReady;
    IO1[rWRegCurrMenuWAIT] = rmtTeensy;
    IO1[rRegPresence1] = 0x55;
    IO1[rRegPresence2] = 0xAA;
    for (uint8_t reg = rRegSIDStrStart; reg < rRegSIDStringTerm; reg++)
        IO1[reg] = ' ';
    IO1[rRegSIDStringTerm] = 0;
    IO1[rwRegPwrUpDefaults] = EEPROM.read(eepAdPwrUpDefaults);
    IO1[rwRegPwrUpDefaults2] = EEPROM.read(eepAdPwrUpDefaults2);
    IO1[rwRegTimezone] = EEPROM.read(eepAdTimezone);
    for (uint8_t reg = 0; reg < NumColorRefs; reg++)
        IO1[rwRegColorRefStart + reg] = EEPROM.read(eepAdColorRefStart + reg);
    // IO1[rwRegNextIOHndlr] = EEPROM.read(eepAdNextIOHndlr); //done each entry into menu
    SetUpMainMenuROM();
    MenuChange(); // set up drive path, menu source/size

    for (uint8_t cnt = 0; cnt < IOH_Num_Handlers; cnt++)
        PadSpace(IOHandler[cnt]->Name, IOHNameLength - 1); // done so selection shown on c64 overwrites previous

    SwiftBrowserInit();

    StrSIDInfo = (char *)calloc(StrSIDInfoSize, sizeof(char)); // SID header info storage
    LatestSIDLoaded = (char *)malloc(MaxPathLength);           // Last loaded Source/SID path/filename
    BigBuf = (uint32_t *)malloc(BigBufSize * sizeof(uint32_t));

    MakeBuildInfo();
    Serial.printf("\n%s\nTeensyROM %s is on-line\n", SerialStringBuf, strVersionNumber);
    Printf_dbg("Debug messages enabled!\n");
    Printf_dbg_sw("Swiftlink debug messages enabled!\n");

    if (IO1[rwRegPwrUpDefaults] & rpudNFCEnabled)
        nfcInit(); // connect to nfc scanner

    if (IO1[rwRegPwrUpDefaults] & rpudRWReadyDly)
        nS_RWnReady = Def_nS_RWnReady_dly; // delay RW read timing

    if (EEPROM.read(eepAdMinBootInd) == MinBootInd_SkipMin) // normal first power up
    {
        if (ReadButton != 0) // skip autolaunch checks if button pressed
        {
            uint32_t AutoStartmS = millis();
            if (!CheckLaunchSDAuto()) // if nothing autolaunched from SD autolaunch file
            {
                if (EEPROM.read(eepAdAutolaunchName) && (ReadButton != 0)) // If name is non zero length & button not pressed
                {
                    char AutoFileName[MaxPathLength];
                    EEPreadStr(eepAdAutolaunchName, AutoFileName);
                    char *ptrAutoFileName = AutoFileName; // pointer to move past SD/USB/TR:
                    RegMenuTypes MenuSourceID = RegMenuTypeFromFileName(&ptrAutoFileName);

                    Printf_dbg("EEP Autolaunch %d \"%s\"\n", MenuSourceID, ptrAutoFileName);
                    RemoteLaunch(MenuSourceID, ptrAutoFileName, true); // do CRT directly
                }
            }
            Printf_dbg("Autolaunch checks: %lumS\n", millis() - AutoStartmS);
        }
    }
    else
    { // if it's not skip min (most likely MinBootInd_FromMin), set back to default for next time
        EEPROM.write(eepAdMinBootInd, MinBootInd_SkipMin);
    }

    SetLEDOn; // done last as indicator of init completion
}

void loop()
{
    if (BtnPressed)
    {
        Serial.print("Button detected\n");
        SetLEDOn;
        BtnPressed = false;
        IO1[rwRegIRQ_CMD] = ricmdNone; // just to be sure, should already be 0/none
        if (RemoteLaunched)
        {
            IO1[rWRegCurrMenuWAIT] = rmtTeensy;
            MenuChange();
            RemoteLaunched = false;
            Printf_dbg("Remote recovery\n");
        }
        if (IO1[rwRegPwrUpDefaults] & rpudNFCEnabled)
            nfcInit();      // connect to nfc scanner
        SetUpMainMenuROM(); // back to main menu, also sets doReset
    }

    if (doReset)
    {
        SetResetAssert;
        Serial.println("Resetting C64");
        Serial.flush();
        delay(50);
        uint32_t NextInterval = 10000, beginWait = millis();
        bool LEDState = true, DefEEPReboot = false;
        while (ReadButton == 0)
        { // avoid self reset detection, check for long press
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

    if (Serial.available())
        ServiceSerial();

    myusbHost.Task();
    if (nfcState == nfcStateEnabled)
        nfcCheck();

    // handler specific polling items:
    if (IOHandler[CurrentIOHandler]->PollingHndlr != NULL)
        IOHandler[CurrentIOHandler]->PollingHndlr();
}
