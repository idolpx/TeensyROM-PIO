// MIT License
// 
// Copyright (c) 2024 Travis Smith
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

#ifndef FILEPARSERS_H_
#define FILEPARSERS_H_

#include <Arduino.h>
#include "DriveDirLoad.h"

uint8_t NumCrtChips = 0;
StructCrtChip CrtChips[MAX_CRT_CHIPS];
char* StrSIDInfo;  // allocated to RAM2 via StrSIDInfoSize
char* LatestSIDLoaded; // allocated to RAM2 via MaxPathLength
char StrMachineInfo[16]; //~5 extra
bool SendC64Msgs = true;

void ParseP00File(StructMenuItem* MyMenuItem);
bool ParseCRTHeader(StructMenuItem* MyMenuItem, uint8_t *EXROM, uint8_t *GAME);
bool ParseChipHeader(uint8_t* ChipHeader, const char *FullFilePath);
FLASHMEM bool ParseARTHeader();
FLASHMEM bool ParseKLAHeader();
FLASHMEM void SIDLoadError(const char* ErrMsg);
FLASHMEM void ParseSIDHeader(const char *filename);
void RedirectEmptyDriveDirMenu();
bool PathIsRoot();
bool SetTypeFromCRT(StructMenuItem* MyMenuItem, uint8_t EXROM, uint8_t GAME);
//Big endian byte to int conversions:
uint32_t toU32(uint8_t* src);
uint16_t toU16(uint8_t* src);
bool AssocHWID_IOH(uint16_t HWType);

void SendMsgSerialStringBuf();

#endif // FileParsers