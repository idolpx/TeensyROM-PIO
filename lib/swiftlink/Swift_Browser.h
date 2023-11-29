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


// Swiftlink Browser Functions

#ifndef SWIFTLINK_BROWSER_H
#define SWIFTLINK_BROWSER_H

#include <Arduino.h>

#include "../../include/TeensyROM.h"

// PETSCII Colors/Special Symbols
#define PETSCIIpurple      0x9c
#define PETSCIIwhite       0x05
#define PETSCIIlightBlue   0x9a
#define PETSCIIyellow      0x9e
#define PETSCIIbrown       0x95
#define PETSCIIpink        0x96
#define PETSCIIlightGreen  0x99
#define PETSCIIgreen       0x1e
#define PETSCIIlightGrey   0x9b
#define PETSCIIdarkGrey    0x97
#define PETSCIIgrey        0x98

#define PETSCIIreturn      0x0d
#define PETSCIIrvsOn       0x12
#define PETSCIIrvsOff      0x92
#define PETSCIIclearScreen 0x93
#define PETSCIIcursorUp    0x91
#define PETSCIIhorizBar    0x60
#define PETSCIIspace       0x20
#define PETSCIIhiLoChrSet  0x0e
#define PETSCIIupGfxChrSet 0x8e

#define MaxURLHostSize      100
#define MaxURLPathSize      300

#define NumPageLinkBuffs    99
#define NumPrevURLQueues    8

#define RxQueueUsed ((RxQueueHead >= RxQueueTail) ? (RxQueueHead - RxQueueTail) : (RxQueueHead + RxQueueSize - RxQueueTail))

struct stcURLParse
{
   char host[MaxURLHostSize];
   uint16_t port;
   char path[MaxURLPathSize];
   char postpath[MaxURLPathSize];
};

char *PageLinkBuff[NumPageLinkBuffs];        // hold links from tags for user selection in browser
stcURLParse *PrevURLQueue[NumPrevURLQueues]; // For browse previous
char CurrPageTitle[eepBMTitleSize];          // keep current page title, could move to RAM2

uint8_t PrevURLQueueNum;   // current/latest in the link history queue
uint8_t UsedPageLinkBuffs; // how many PageLinkBuff elements have been Used

void SendPETSCIICharImmediate(uint8_t CharToSend);
void SendASCIIStrImmediate(const char* CharsToSend);
void SendASCIIErrorStrImmediate(const char* CharsToSend);

FLASHMEM void SendCommandSummaryImmediate(bool Paused);
FLASHMEM void SendBrowserCommandsImmediate();

bool BrowserMode;
uint32_t PageCharsReceived;
uint8_t HexCharToInt(uint8_t HexChar);
bool CheckAndDecode(const char *ptrChars, uint8_t *ptrRetChar);
void UnPausePage();
void ParseHTMLTag();
void ParseURL(const char * URL, stcURLParse &URLParse);
bool ReadClientLine(char* linebuf, uint16_t MaxLen);
void ClearClientStop();
void AddToPrevURLQueue(const stcURLParse *URL);
bool WebConnect(const stcURLParse *DestURL, bool AddToHist);
void DoSearch(const char *Term);

void DownloadFile(stcURLParse *DestURL);
bool ValidModifier(const char cMod);
bool isURLFiltered(const stcURLParse *URL);
FLASHMEM bool DLExtension(const char * Extension);
void ModWebConnect(stcURLParse *DestURL, char cMod, bool AddToHist);
void ProcessBrowserCommand();

#endif // SWIFTLINK_BROWSER_H