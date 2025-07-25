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

stcIOHandlers IOHndlr_TeensyROM =
{
  "TeensyROM",              //Name of handler
  &InitHndlr_TeensyROM,     //Called once at handler startup
  &IO1Hndlr_TeensyROM,      //IO1 R/W handler
  NULL,                     //IO2 R/W handler
  NULL,                     //ROML Read handler, in addition to any ROM data sent
  NULL,                     //ROMH Read handler, in addition to any ROM data sent
  &PollingHndlr_TeensyROM,  //Polled in main routine
  NULL,                     //called at the end of EVERY c64 cycle
};

int16_t SidSpeedAdjust = 0;
bool    SidLogConv = false; //true=Log, false=linear
volatile uint8_t* IO1;  //io1 space/regs
volatile uint16_t StreamOffsetAddr, StringOffset = 0;

volatile uint8_t doReset = true;


volatile uint8_t eepDataToWrite;
volatile uint16_t eepAddrToWrite;

StructMenuItem *MenuSource;
uint16_t SelItemFullIdx = 0;  //logical full index into menu for selected item
uint16_t NumItemsFull;  //Num Items in Current Menu

uint8_t ASCIItoPETSCII[128]=
{
 /*   ASCII   */  //PETSCII
 /*   0 'null'*/    0,
 /*   1 '' */    0,
 /*   2 '' */    0,
 /*   3 '' */    0,
 /*   4 '' */    0,
 /*   5 '' */    0,
 /*   6 '' */    0,
 /*   7 '' */    0,
 /*   8 ''  */    0,
 /*   9 '\t'  */   32, // tab -> space
 /*  10 '\n'  */   10, //let this slide: won't print anything in petscii, but maybe if prog is in terminal mode?
 /*  11 ''  */    0,
 /*  12 ''  */    0,
 /*  13 '\r'  */   13,
 /*  14 ''  */    0,
 /*  15 ''  */    0,
 /*  16 '' */    0,
 /*  17 '' */    0,
 /*  18 '' */    0,
 /*  19 '' */    0,
 /*  20 '' */    0,
 /*  21 '' */    0,
 /*  22 '' */    0,
 /*  23 '' */    0,
 /*  24 '' */    0,
 /*  25 ''  */    0,
 /*  26 '' */    0,
 /*  27 '' */    0,
 /*  28 ''  */    0,
 /*  29 ''  */    0,
 /*  30 ''  */    0,
 /*  31 ''  */    0,
 /*  32 ' '   */   32,
 /*  33 '!'   */   33,
 /*  34 '"'   */   34,
 /*  35 '#'   */   35,
 /*  36 '$'   */   36,
 /*  37 '%'   */   37,
 /*  38 '&'   */   38,
 /*  39 '''   */   39,
 /*  40 '('   */   40,
 /*  41 ')'   */   41,
 /*  42 '*'   */   42,
 /*  43 '+'   */   43,
 /*  44 ','   */   44,
 /*  45 '-'   */   45,
 /*  46 '.'   */   46,
 /*  47 '/'   */   47,
 /*  48 '0'   */   48,
 /*  49 '1'   */   49,
 /*  50 '2'   */   50,
 /*  51 '3'   */   51,
 /*  52 '4'   */   52,
 /*  53 '5'   */   53,
 /*  54 '6'   */   54,
 /*  55 '7'   */   55,
 /*  56 '8'   */   56,
 /*  57 '9'   */   57,
 /*  58 ':'   */   58,
 /*  59 ';'   */   59,
 /*  60 '<'   */   60,
 /*  61 '='   */   61,
 /*  62 '>'   */   62,
 /*  63 '?'   */   63,
 /*  64 '@'   */   64,
 /*  65 'A'   */   97,
 /*  66 'B'   */   98,
 /*  67 'C'   */   99,
 /*  68 'D'   */  100,
 /*  69 'E'   */  101,
 /*  70 'F'   */  102,
 /*  71 'G'   */  103,
 /*  72 'H'   */  104,
 /*  73 'I'   */  105,
 /*  74 'J'   */  106,
 /*  75 'K'   */  107,
 /*  76 'L'   */  108,
 /*  77 'M'   */  109,
 /*  78 'N'   */  110,
 /*  79 'O'   */  111,
 /*  80 'P'   */  112,
 /*  81 'Q'   */  113,
 /*  82 'R'   */  114,
 /*  83 'S'   */  115,
 /*  84 'T'   */  116,
 /*  85 'U'   */  117,
 /*  86 'V'   */  118,
 /*  87 'W'   */  119,
 /*  88 'X'   */  120,
 /*  89 'Y'   */  121,
 /*  90 'Z'   */  122,
 /*  91 '['   */   91,
 /*  92 '\'   */   47, // backslash    -> forward slash
 /*  93 ']'   */   93,                 
 /*  94 '^'   */   94, // caret        -> up arrow
 /*  95 '_'   */  164, // underscore   -> bot line graphic char
 /*  96 '`'   */   39, // grave accent -> single quote
 /*  97 'a'   */   65,
 /*  98 'b'   */   66,
 /*  99 'c'   */   67,
 /* 100 'd'   */   68,
 /* 101 'e'   */   69,
 /* 102 'f'   */   70,
 /* 103 'g'   */   71,
 /* 104 'h'   */   72,
 /* 105 'i'   */   73,
 /* 106 'j'   */   74,
 /* 107 'k'   */   75,
 /* 108 'l'   */   76,
 /* 109 'm'   */   77,
 /* 110 'n'   */   78,
 /* 111 'o'   */   79,
 /* 112 'p'   */   80,
 /* 113 'q'   */   81,
 /* 114 'r'   */   82,
 /* 115 's'   */   83,
 /* 116 't'   */   84,
 /* 117 'u'   */   85,
 /* 118 'v'   */   86,
 /* 119 'w'   */   87,
 /* 120 'x'   */   88,
 /* 121 'y'   */   89,
 /* 122 'z'   */   90,
 /* 123 '{'   */  179, // left curly brace  ->   -|   graphic char 
 /* 124 '|'   */  125, // pipe              ->    |   graphic char
 /* 125 '}'   */  171, // right curly brace ->    |-  graphic char 
 /* 126 '~'   */  178, // tilde             ->   Half height T graphic char
 /* 127 '' */   95, // delete            ->   left arrow
};

extern bool EthernetInit();
extern void MenuChange();
extern void HandleExecution();
extern bool PathIsRoot();

extern void FreeDriveDirMenu();
extern void RedirectEmptyDriveDirMenu();
extern void IOHandlerSelectInit();
extern void IOHandlerNextInit();
extern void ParseSIDHeader(const char *filename);
extern stcIOHandlers* IOHandler[];

extern uint8_t RAM_Image[];
extern char* StrSIDInfo;
extern char* LatestSIDLoaded;
extern char StrMachineInfo[];
extern uint8_t nfcState;

extern void nfcWriteTag(const char* TxtMsg);
extern void nfcInit();

#define DecToBCD(d) ((int((d)/10)<<4) | ((d)%10))

//#define ToPETSCII(x) (x==95 ? 32 : x>64 ? x^32 : x)
#define ToPETSCII(x) ASCIItoPETSCII[(x) & 0x7f]

FLASHMEM void getNtpTime();
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

void (*StatusFunction[rsNumStatusTypes])() = //match RegStatusTypes order
{
   &MenuChange,          // rsChangeMenu 
   &HandleExecution,     // rsStartItem  
   &getNtpTime,          // rsGetTime    
   &IOHandlerSelectInit, // rsIOHWSelInit   
   &WriteEEPROM,         // rsWriteEEPROM
   &MakeBuildInfo,       // rsMakeBuildCPUInfoStr
   &UpDirectory,         // rsUpDirectory
   &SearchForLetter,     // rsSearchForLetter
   &LoadMainSIDforXfer,  // rsLoadSIDforXfer
   &NextPicture,         // rsNextPicture    
   &LastPicture,         // rsLastPicture    
   &WriteNFCTagCheck,    // rsWriteNFCTagCheck
   &WriteNFCTag,         // rsWriteNFCTag
   &NFCReEnable,         // rsNFCReEnable
   &SetBackgroundSID,    // rsSetBackgroundSID
   &SetAutoLaunch,       // rsSetAutoLaunch
   &ClearAutoLaunch,     // rsClearAutoLaunch
   &NextTextFile,        // rsNextTextFile
   &LastTextFile,        // rsLastTextFile
   &IOHandlerNextInit    // rsIOHWNextInit

};

#endif // IOH_TEENSYROM_H