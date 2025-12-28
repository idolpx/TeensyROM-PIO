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

#ifndef SWIFT_BROWSER_H
#define SWIFT_BROWSER_H

#include "Common_Defs.h"
#include "Swift_RxQueue.h"
#include "Menu_Regs.h"
#include "ethernet_dev.h"

#define ModColor    PETSCIIpink
#define TxtColor    PETSCIIgreen
#define CmdColor    PETSCIIwhite

struct stcURLParse
{
   char host[MaxURLHostSize];
   uint16_t port;
   char path[MaxURLPathSize];
   char postpath[MaxURLPathSize];
};

void SwiftBrowserInit();
void SwiftTxBufToLcaseASSCII();
void SendPETSCIICharImmediate(uint8_t CharToSend);
void SendASCIIStrImmediate(const char* CharsToSend);
void SendASCIIErrorStrImmediate(const char* CharsToSend);
FLASHMEM void SendCommandSummaryImmediate(bool Paused);
FLASHMEM void SendColonSpaceImmediate();
FLASHMEM void SendBrowserCommandsImmediate();
FLASHMEM uint8_t HexCharToInt(uint8_t HexChar);
FLASHMEM bool CheckAndDecode(const char *ptrChars, uint8_t *ptrRetChar);
FLASHMEM void CopyDecode(const char *FromEncoded, char *ToDecoded);
FLASHMEM void CopyEncode(const char *FromRaw, char *ToEncoded);
void DumpQueueUnPausePage();
void UnPausePage();
void ParseEntityReference();
void ParseHTMLTag();
void ParseURL(const char * URL, stcURLParse *URLParse);
bool ReadClientLine(char* linebuf, uint16_t MaxLen);
void ClearClientStop();
void AddToPrevURLQueue(const stcURLParse *URL);
uint32_t WebConnect(const stcURLParse *DestURL);
FLASHMEM void DoSearch(const char *Term);
FLASHMEM bool InitCheckSD();
FLASHMEM void DownloadFile(stcURLParse *DestURL);
FLASHMEM bool ValidModifier(const char cMod);
bool isURLFiltered(const stcURLParse *URL);
FLASHMEM bool DLExtension(const char * Filename);
void ModWebConnect(stcURLParse *DestURL, char cMod);
FLASHMEM void BC_Bookmarks(char* CmdMsg);
FLASHMEM void BC_Downloads(char* CmdMsg);
FLASHMEM void BC_FollowHyperlink(char* CmdMsg);
FLASHMEM void ProcessBrowserCommand();

#endif // SWIFT_BROWSER_H