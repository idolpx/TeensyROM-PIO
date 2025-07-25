
#include "Menu.h"

#include <Arduino.h>

//#include "../../include/TeensyROM.h"
#include "DriveDirLoad.h"
#include "IOHandlers.h"
#include "IOH_TeensyROM.h"
#include "IOH_Swiftlink.h"
#include "IOH_MIDI.h"

void SetUpMainMenuROM()
{
   SetDMADeassert;
   SetIRQDeassert;
   SetNMIDeassert;
   SetGameDeassert;
   SetExROMAssert; //emulate 8k cart ROM
   LOROM_Image = TeensyROMC64_bin;
   HIROM_Image = NULL;
   LOROM_Mask = HIROM_Mask = 0x1fff;
   NVIC_ENABLE_IRQ(IRQ_ENET); //make sure ethernet interrupt is back on
   NVIC_ENABLE_IRQ(IRQ_PIT);
   EmulateVicCycles = false;
   nfcState &= ~nfcStateBitPaused; //clear paused bit in case paused by time critical function
   
   FreeCrtChips();
   FreeSwiftlinkBuffs();
   RedirectEmptyDriveDirMenu();
   free((void*)MIDIRxBuf); 
   MIDIRxBuf=NULL;
   free(TgetQueue); TgetQueue=NULL;
   free(LSFileName); LSFileName=NULL;

   IOHandlerInit(IOH_TeensyROM);   
   doReset = true;
}

void SetNumItems(uint16_t NumItems)
{
   NumItemsFull = NumItems;
   IO1[rRegNumItemsOnPage] = (NumItemsFull > MaxItemsPerPage ? MaxItemsPerPage : NumItemsFull);
   IO1[rwRegPageNumber] = 1;
   IO1[rRegNumPages] = 
      NumItems/MaxItemsPerPage + 
      (NumItems%MaxItemsPerPage!=0 ? 1 : 0) +
      (NumItems==0 ? 1 : 0);
}

void PadSpace(char* StrToPad, uint8_t PadToLength)
{
   while(strlen(StrToPad)<PadToLength) strcat(StrToPad, " ");
}