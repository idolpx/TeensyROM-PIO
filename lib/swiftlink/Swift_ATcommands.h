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

#ifndef SWIFT_ATCOMMANDS_H
#define SWIFT_ATCOMMANDS_H

#include <NativeEthernet.h>

#include "IOH_Swiftlink.h"
#include "eeprom_dev.h"

enum ATRespCode
{  //http://www.messagestick.net/modem/Hayes_Ch1-2.html
   //http://www.messagestick.net/modem/hayes_modem.html
   //match spec and Verbose_RCs below
   ATRC_OK          , // 0
   ATRC_CONNECT     , // 1
   ATRC_RING        , // 2
   ATRC_NO_CARRIER  , // 3
   ATRC_ERROR       , // 4
   ATRC_CONNECT_1200, // 5
   ATRC_NO_DIALTONE , // 6
   ATRC_BUSY        , // 7
   ATRC_NO_ANSWER   , // 8
   NumATResponseCodes
};

#define MaxATcmdLength   20

struct stcATCommand
{
  char Command[MaxATcmdLength];
  ATRespCode (*Function)(char*); 
};

FLASHMEM void SendATresponse(ATRespCode ResponseCode);
FLASHMEM bool VerifySingleBinArg(const char* CmdArg);
void AddVerboseToPETSCIIStrToRxQueueLN(const char* s);
void AddVerboseToPETSCIIStrToRxQueue(const char* s);
FLASHMEM ATRespCode StrToIPToEE(char* Arg, uint8_t EEPaddress);

// Swiftlink AT Commands

FLASHMEM ATRespCode AT_BROWSE(char* CmdArg);
FLASHMEM ATRespCode AT_C(char* CmdArg);
FLASHMEM ATRespCode AT_DT(char* CmdArg);
FLASHMEM ATRespCode AT_S(char* CmdArg);
FLASHMEM ATRespCode AT_RNDMAC(char* CmdArg);
FLASHMEM ATRespCode AT_MAC(char* CmdArg);
FLASHMEM ATRespCode AT_DHCP(char* CmdArg);
FLASHMEM ATRespCode AT_DHCPTIME(char* CmdArg);
FLASHMEM ATRespCode AT_DHCPRESP(char* CmdArg);
FLASHMEM ATRespCode AT_MYIP(char* CmdArg);
FLASHMEM ATRespCode AT_DNSIP(char* CmdArg);
FLASHMEM ATRespCode AT_GTWYIP(char* CmdArg);
FLASHMEM ATRespCode AT_MASKIP(char* CmdArg);
FLASHMEM ATRespCode AT_DEFAULTS(char* CmdArg);
FLASHMEM ATRespCode AT_HELP(char* CmdArg);
FLASHMEM ATRespCode AT_Info(char* CmdArg);
FLASHMEM ATRespCode AT_Echo(char* CmdArg);
FLASHMEM ATRespCode AT_Verbose(char* CmdArg);
FLASHMEM ATRespCode AT_ZSoftReset(char* CmdArg);
FLASHMEM ATRespCode AT_Hook(char* CmdArg);
FLASHMEM ATRespCode ProcessATCommand();
#endif // SWIFT_ATCOMMANDS_H