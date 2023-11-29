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

#include "main.h"

#include <SD.h>
#include <SPI.h>
#include <EEPROM.h>

#include <NativeEthernet.h>
#include <NativeEthernetUdp.h>

#include "../../include/TeensyROM.h"

#include "midi2sid.h"
#include "ISRs.h"
#include "IOHandlers.h"
#include "eeprom_util.h"

#include "Menu.h"
#include "DriveDirLoad.h"
#include "Menu_Regs.h"
#include "MainMenuItems.h"
#include "RemoteControl.h"

#include "Swift_Browser.h"
#include "Swift_RxQueue.h"

uint8_t RAM_Image[RAM_ImageSize]; //Main RAM1 file storage buffer

extern "C" uint32_t set_arm_clock(uint32_t frequency);
extern float tempmonGetTemp(void);

void setup() 
{
   set_arm_clock(816000000);  //slight overclocking, no cooling required
   
   Serial.begin(115200);
   if (CrashReport) Serial.print(CrashReport);

   for(uint8_t PinNum=0; PinNum<sizeof(OutputPins); PinNum++) pinMode(OutputPins[PinNum], OUTPUT); 
   DataBufDisable; //buffer disabled
   SetDataPortDirOut; //default to output (for C64 Read)
   SetDMADeassert;
   SetIRQDeassert;
   SetNMIDeassert;
   SetLEDOn;
   SetDebugDeassert;
   SetResetAssert; //assert reset until main loop()
  
   for(uint8_t PinNum=0; PinNum<sizeof(InputPins); PinNum++) pinMode(InputPins[PinNum], INPUT); 
   pinMode(Reset_Btn_In_PIN, INPUT_PULLUP);  //also makes it Schmitt triggered (PAD_HYS)
   pinMode(PHI2_PIN, INPUT_PULLUP);   //also makes it Schmitt triggered (PAD_HYS)
   attachInterrupt( digitalPinToInterrupt(Reset_Btn_In_PIN), isrButton, FALLING );
   attachInterrupt( digitalPinToInterrupt(PHI2_PIN), isrPHI2, RISING );
   NVIC_SET_PRIORITY(IRQ_GPIO6789,16); //set HW ints as high priority, otherwise ethernet int timer causes misses
   
   myusbHost.begin(); // Start USBHost_t36, HUB(s) and USB devices.

   EEPcheck();

   IO1 = (uint8_t*)calloc(IO1_Size, sizeof(uint8_t)); //allocate IO1 space and init to 0
   IO1[rwRegStatus]        = rsReady;
   IO1[rWRegCurrMenuWAIT] = rmtTeensy;
   IO1[rRegPresence1]     = 0x55;   
   IO1[rRegPresence2]     = 0xAA;   
   for (uint16_t reg=rRegSIDStrStart; reg<rRegSIDStringTerm; reg++) IO1[reg]=' '; 
   IO1[rRegSIDStringTerm] = 0;   
   IO1[rwRegPwrUpDefaults]= EEPROM.read(eepAdPwrUpDefaults);
   IO1[rwRegTimezone]     = EEPROM.read(eepAdTimezone);  
   //IO1[rwRegNextIOHndlr] = EEPROM.read(eepAdNextIOHndlr); //done each entry into menu
   SetUpMainMenuROM();
   MenuChange(); //set up drive path, menu source/size

   for(uint8_t cnt=0; cnt<IOH_Num_Handlers; cnt++) PadSpace(IOHandler[cnt]->Name, IOHNameLength-1); //done so selection shown on c64 overwrites previous

   for(uint8_t cnt=0; cnt<NumPageLinkBuffs; cnt++) PageLinkBuff[cnt] = NULL; //initialize page link buffer for swiftlink browser mode
   for(uint8_t cnt=0; cnt<NumPrevURLQueues; cnt++) PrevURLQueue[cnt] = NULL; //initialize previous link buffer for swiftlink browser mode
   for(uint8_t cnt=0; cnt<RxQueueNumBlocks; cnt++) RxQueue[cnt] = NULL;      //initialize RxQueue for swiftlink

   BigBuf = (uint32_t*)malloc(BigBufSize*sizeof(uint32_t));
   MakeBuildCPUInfoStr();
   Serial.printf("\n%sTeensyROM %s is on-line\n", SerialStringBuf, strVersionNumber);
} 
     
void loop()
{
   if (BtnPressed)
   {
      Serial.print("Button detected\n"); 
      SetLEDOn;
      BtnPressed=false;
      IO1[rwRegIRQ_CMD] = ricmdNone; //just to be sure, should already be 0/none
      if (RemoteLaunched)
      {
         IO1[rWRegCurrMenuWAIT] = rmtTeensy;
         MenuChange();
         RemoteLaunched = false;
         Printf_dbg("Remote recovery\n"); 
      }   
      SetUpMainMenuROM(); //back to main menu
   }
   
   if (doReset)
   {
      SetResetAssert; 
      Serial.println("Resetting C64"); 
      Serial.flush();
      delay(50); 
      while(ReadButton==0); //avoid self reset detection
      doReset=false;
      BtnPressed = false;
      SetResetDeassert;
   }
  
   if (Serial.available()) ServiceSerial();
   myusbHost.Task();
   
   //handler specific polling items:
   if (IOHandler[CurrentIOHandler]->PollingHndlr != NULL) IOHandler[CurrentIOHandler]->PollingHndlr();
}
