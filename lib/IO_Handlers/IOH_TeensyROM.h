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


//IO Handler for TeensyROM 

#ifndef IOH_TEENSYROM_H
#define IOH_TEENSYROM_H

#include "SerUSBIO.h"

#include "IOH_defs.h"

void IO1Hndlr_TeensyROM(uint8_t Address, bool R_Wn);
void PollingHndlr_TeensyROM();
void InitHndlr_TeensyROM();

extern stcIOHandlers IOHndlr_TeensyROM;

extern int16_t SidSpeedAdjust;
extern bool    SidLogConv; //true=Log, false=linear
extern volatile uint8_t* IO1;  //io1 space/regs
extern volatile uint16_t StreamOffsetAddr, StringOffset;

extern volatile uint8_t doReset;

extern volatile uint8_t eepDataToWrite;
extern volatile uint16_t eepAddrToWrite;

extern StructMenuItem *MenuSource;
extern uint16_t SelItemFullIdx;  //logical full index into menu for selected item
extern uint16_t NumItemsFull;  //Num Items in Current Menu

extern uint8_t ASCIItoPETSCII[128];

extern bool PathIsRoot();
extern char StrMachineInfo[];

#define DecToBCD(d) ((int((d)/10)<<4) | ((d)%10))

//#define ToPETSCII(x) (x==95 ? 32 : x>64 ? x^32 : x)
#define ToPETSCII(x) ASCIItoPETSCII[(x) & 0x7f]

FLASHMEM void SendStrPrintfln(const char* Msg);
FLASHMEM void NetListenInit();
FLASHMEM void getNtpTime();
FLASHMEM void HotKeySetLaunch();
void WriteEEPROM();
FLASHMEM uint8_t RAM2blocks();
FLASHMEM void MakeBuildInfo();

//FLASHMEM void MakeBuildCPUInfoStr()
//{
//   FreeDriveDirMenu(); //Will mess up navigation if not on TR menu!
//   RedirectEmptyDriveDirMenu(); //OK since we're on the TR settings screen
//  
//   uint32_t CrtMax = (RAM_ImageSize & 0xffffe000)/1024; //round down to k bytes rounded to nearest 8k
//   //Serial.printf("\n\nRAM1 Buff: %luK (%lu blks)\n", CrtMax, CrtMax/8);
//      
//   uint8_t NumChips = RAM2blocks();
//   //Serial.printf("RAM2 Blks: %luK (%lu blks)\n", NumChips*8, NumChips);
//   NumChips = RAM2blocks()-1; //do it again, sometimes get one more, minus one to match reality, not clear why
//   //Serial.printf("RAM2 Blks: %luK (%lu blks)\n", NumChips*8, NumChips);
//  
//   CrtMax += NumChips*8;
//   char FreeStr[20];
//   sprintf(FreeStr, "  %luk free\r", (uint32_t)(CrtMax*1.004));  //larger File size due to header info.
//
//   MakeBuildInfo();
//   strcat(SerialStringBuf, FreeStr);
//}

void UpDirectory();
bool SetSIDSpeed(bool LogConv, int16_t PlaybackSpeedIn);
void SetCursorToItemNum(uint16_t ItemNum);
FLASHMEM void NextFileType(uint8_t FileType1, uint8_t FileType2);
FLASHMEM void LastFileType(uint8_t FileType1, uint8_t FileType2);
FLASHMEM void NextTextFile();
FLASHMEM void LastTextFile();
FLASHMEM void NextPicture();
FLASHMEM void LastPicture();
void SearchForLetter();
FLASHMEM void GetCurrentFilePathName(char* FilePathName);
FLASHMEM void WriteNFCTagCheck();
FLASHMEM void WriteNFCTag();
FLASHMEM void NFCReEnable();
FLASHMEM void SetAutoLaunch();
FLASHMEM void ClearAutoLaunch();
FLASHMEM void SetBackgroundSID();
FLASHMEM int16_t FindTRMenuItem(StructMenuItem* MyMenu, uint16_t NumEntries, char* EntryName);
FLASHMEM void LoadMainSIDforXfer();

extern void (*StatusFunction[rsNumStatusTypes])(); //match RegStatusTypes order

#endif // IOH_TEENSYROM_H