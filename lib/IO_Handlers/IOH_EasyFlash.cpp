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

#include "IOH_EasyFlash.h"

#include "Common_Defs.h"
#include "FileParsers.h"

#define Printf_Swaps     Printf_dbg   //Serial.printf  //
#define NumDecodeBanks   64
uint8_t *BankDecode[NumDecodeBanks][2];
uint8_t EZFlashRAM[256];

#ifdef MinimumBuild
   struct stcSwapBuffers
   {
      uint8_t  Image[8192]; // 8k swap image
      uint32_t Offset; // chip swap file offsets to check for same & not reload
   };
 
   stcSwapBuffers SwapBuffers[Num8kSwapBuffers];
    
   extern bool PathIsRoot();
   extern char DriveDirPath[];
   extern StructMenuItem DriveDirMenu;
   extern File myFile;
#endif


void InitHndlr_EasyFlash();  
void IO1Hndlr_EasyFlash(uint8_t Address, bool R_Wn);  
void IO2Hndlr_EasyFlash(uint8_t Address, bool R_Wn);  
void PollingHndlr_EasyFlash();                           

stcIOHandlers IOHndlr_EasyFlash =
{
  "EasyFlash",                //Name of handler
  &InitHndlr_EasyFlash,       //Called once at handler startup
  &IO1Hndlr_EasyFlash,        //IO1 R/W handler
  &IO2Hndlr_EasyFlash,        //IO2 R/W handler
  NULL,                       //ROML Read handler, in addition to any ROM data sent
  NULL,                       //ROMH Read handler, in addition to any ROM data sent
  &PollingHndlr_EasyFlash,    //Polled in main routine
  NULL,                       //called at the end of EVERY c64 cycle
};

#ifdef MinimumBuild
void LoadBank(uint32_t SeekTo, uint8_t* ptrImage)
{
   if(!myFile)
   {
      char FullFilePath[MaxNamePathLength];

      //uint32_t Startms = millis();

      if (PathIsRoot()) sprintf(FullFilePath, "%s%s", DriveDirPath, DriveDirMenu.Name);  // at root
      else sprintf(FullFilePath, "%s/%s", DriveDirPath, DriveDirMenu.Name);
         
      //Printf_dbg("Loading:\r\n%s", FullFilePath);

      myFile = SD.open(FullFilePath, FILE_READ);
      
      if (!myFile) 
      {
         Printf_dbg("File Not Found");
         return;
      }
   }
   //Printf_dbg("\n %lu mS Open ", millis()-Startms);
   
   myFile.seek(SeekTo);
   for (uint16_t count = 0; count < 8192; count++) ptrImage[count]=myFile.read();
   //myFile.close();
   //Printf_dbg("\n %lu mS Load ", millis()-Startms);
}
#endif

void InitHndlr_EasyFlash()
{
    
   //initialize bank decode pointers
   for(uint8_t BankNum = 0; BankNum < NumDecodeBanks; BankNum++) {BankDecode[BankNum][0]=NULL;BankDecode[BankNum][1]=NULL;}
   for(uint8_t ChipNum = 0; ChipNum < NumCrtChips; ChipNum++)
   {
      Printf_dbg("Bank# %02d,%d  chip# %03d  addr %08x\n", CrtChips[ChipNum].BankNum, (CrtChips[ChipNum].LoadAddress == 0x8000 ? 0 : 1), ChipNum, (uint32_t)CrtChips[ChipNum].ChipROM);
      if (CrtChips[ChipNum].BankNum < NumDecodeBanks)
         BankDecode[CrtChips[ChipNum].BankNum][CrtChips[ChipNum].LoadAddress == 0x8000 ? 0 : 1] = CrtChips[ChipNum].ChipROM;
      else Serial.printf("Unexp Bank# (%d) in chip %d\n", CrtChips[ChipNum].BankNum, ChipNum);
   }
   
   memset(EZFlashRAM, 0, 256);
   
#ifdef MinimumBuild
   //initialize/invalidate swap buffer, pre-load first swappable chips
   uint8_t ChipNum = 0;
   for(uint8_t BuffNum = 0; BuffNum < Num8kSwapBuffers; BuffNum++) 
   {
      SwapBuffers[BuffNum].Offset=0; //default to zero/invalid
      //find next chip marked as swappable
      while (ChipNum < NumCrtChips)
      {
         if(((uint32_t)CrtChips[ChipNum].ChipROM & SwapSeekAddrMask) == SwapSeekAddrMask)
         {
            //pre-load and update
            LoadBank((uint32_t)CrtChips[ChipNum].ChipROM & ~SwapSeekAddrMask, SwapBuffers[BuffNum].Image);
            SwapBuffers[BuffNum].Offset = (uint32_t)CrtChips[ChipNum].ChipROM;   
            Printf_Swaps("Pre%d: %08x\n", BuffNum, (uint32_t) SwapBuffers[BuffNum].Offset & ~SwapSeekAddrMask);
            ChipNum++;
            break;  //exit while
         }
         ChipNum++;
      }
   }
#endif
   
   //start with Bank 0:
   LOROM_Image = BankDecode[0][0];
   HIROM_Image = BankDecode[0][1];
   //Printf_dbg("Jmp vect: $%02x%02x\n", HIROM_Image[0x1FFD], HIROM_Image[0x1FFC]);
   if(memcmp(HIROM_Image+0x1800, "eapi", 4)==0) Serial.printf("EAPI found\n");
   else Printf_dbg("EAPI *not* found: %d %d %d %d\n", HIROM_Image[0x1800], HIROM_Image[0x1801], HIROM_Image[0x1802], HIROM_Image[0x1803]);

   //force ultimax mode:
   SetGameAssert;
   SetExROMDeassert;
   //but turn off VIC emmulation:
   EmulateVicCycles = false;
}

#ifdef MinimumBuild
uint8_t* ImageCheckAssign(uint8_t* BankRequested)
{
   //Printf_dbg(" Ad %08x", (uint32_t)BankRequested);
   if (((uint32_t)BankRequested & SwapSeekAddrMask) == SwapSeekAddrMask) //requested bank is a swap bank
   {
      //Printf_dbg(" Sr");
      //check swap banks to see if it's already loaded
      for(uint8_t BuffNum = 0; BuffNum < Num8kSwapBuffers; BuffNum++) 
      {
         if ((uint32_t)BankRequested == SwapBuffers[BuffNum].Offset) 
         {
            //Printf_dbg(" Ht\n");
            return SwapBuffers[BuffNum].Image; //it's a match!
         }
      }
      DMA_State = DMA_S_StartActive;  //no match, pause via DMA for swap
   }
   //Printf_dbg(" \n");
   return BankRequested;
}
#endif

void IO1Hndlr_EasyFlash(uint8_t Address, bool R_Wn)
{
   if (!R_Wn) //IO1 Write  -------------------------------------------------
   {
      uint8_t Data = DataPortWaitRead(); 
      TraceLogAddValidData(Data);
      switch (Address)
      {
         case 0x00:   // Register $DE00 – EasyFlash Bank (write-only)
            Data &= 0x3f;
            
#ifdef MinimumBuild
            //check if swapped bank is being selected, check for same or initiate swap
            LOROM_Image = ImageCheckAssign(BankDecode[Data][0]);
            HIROM_Image = ImageCheckAssign(BankDecode[Data][1]);
#else
            LOROM_Image = BankDecode[Data][0];
            HIROM_Image = BankDecode[Data][1];
#endif

            break;
         case 0x02:   // Register $DE02 – EasyFlash Control (write-only)
            if (Data & 0x80) SetLEDOn; //Status LED, 1 = on
            else SetLEDOff;

            if (Data & 0x02) SetExROMAssert; //EXROM state, 0 = /EXROM high
            else SetExROMDeassert;

            if (Data & 0x01) SetGameAssert; //GAME state if M (bit 2 sel bit 0 or jumper for GAME) = 1, 0 = /GAME high
            else SetGameDeassert;
            break;
      }
   }
}

void IO2Hndlr_EasyFlash(uint8_t Address, bool R_Wn)
{
   if (R_Wn) //IO2 Read  -------------------------------------------------
   {
      DataPortWriteWaitLog(EZFlashRAM[Address]);
   }
   else  // IO2 write    -------------------------------------------------
   {
      uint8_t Data = DataPortWaitRead(); 
      EZFlashRAM[Address] = Data;
      TraceLogAddValidData(Data);
   }
}

void PollingHndlr_EasyFlash()
{
#ifdef MinimumBuild   
   if (DMA_State == DMA_S_ActiveReady) 
   {
      //DMA asserted, paused for bank swap from SD
      uint32_t Startms = millis();
      static uint8_t NextSwapBuffNum = 0; 
      
      if (((uint32_t)LOROM_Image & SwapSeekAddrMask) == SwapSeekAddrMask)
      {
         //make sure NextSwapBuff isn't being used for high bank
         if(HIROM_Image && HIROM_Image == SwapBuffers[NextSwapBuffNum].Image)
            if(++NextSwapBuffNum==Num8kSwapBuffers) NextSwapBuffNum=0;
         
         LoadBank((uint32_t)LOROM_Image & ~SwapSeekAddrMask, SwapBuffers[NextSwapBuffNum].Image);
         Printf_Swaps("L%02d: %08x  ", NextSwapBuffNum, (uint32_t)LOROM_Image & ~SwapSeekAddrMask);
         SwapBuffers[NextSwapBuffNum].Offset = (uint32_t)LOROM_Image;
         LOROM_Image = SwapBuffers[NextSwapBuffNum].Image;
         if(++NextSwapBuffNum==Num8kSwapBuffers) NextSwapBuffNum=0;
      }         
      
      if (((uint32_t)HIROM_Image & SwapSeekAddrMask) == SwapSeekAddrMask)
      {
         //make sure NextSwapBuff isn't being used for low bank
         if(LOROM_Image && LOROM_Image == SwapBuffers[NextSwapBuffNum].Image)
            if(++NextSwapBuffNum==Num8kSwapBuffers) NextSwapBuffNum=0;
         
         LoadBank((uint32_t)HIROM_Image & ~SwapSeekAddrMask, SwapBuffers[NextSwapBuffNum].Image);
         Printf_Swaps("H%02d: %08x  ", NextSwapBuffNum, (uint32_t)HIROM_Image & ~SwapSeekAddrMask);
         SwapBuffers[NextSwapBuffNum].Offset = (uint32_t)HIROM_Image;
         HIROM_Image = SwapBuffers[NextSwapBuffNum].Image;
         if(++NextSwapBuffNum==Num8kSwapBuffers) NextSwapBuffNum=0;
      }
      
      Printf_Swaps(" %lu mS swp\n", millis()-Startms);
      Serial.flush();
      DMA_State = DMA_S_StartDisable;
   }
#endif
}