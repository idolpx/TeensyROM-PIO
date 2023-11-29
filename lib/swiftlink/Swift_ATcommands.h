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


// Swiftlink AT Commands

#ifndef SWIFTLINK_ATCOMMANDS_H
#define SWIFTLINK_ATCOMMANDS_H

#include <Arduino.h>

FLASHMEM void AT_BROWSE (char *CmdArg);
FLASHMEM void AT_DT (char *CmdArg);
FLASHMEM void AT_C (char *CmdArg);
FLASHMEM void AT_S (char *CmdArg);
FLASHMEM void AT_RNDMAC (char *CmdArg);
FLASHMEM void AT_MAC (char *CmdArg);
FLASHMEM void AT_DHCP (char *CmdArg);
FLASHMEM void AT_DHCPTIME (char *CmdArg);
FLASHMEM void AT_DHCPRESP (char *CmdArg);
FLASHMEM void AT_MYIP (char *CmdArg);
FLASHMEM void AT_DNSIP (char *CmdArg);
FLASHMEM void AT_GTWYIP (char *CmdArg);
FLASHMEM void AT_MASKIP (char *CmdArg);
FLASHMEM void AT_DEFAULTS (char *CmdArg);
FLASHMEM void AT_HELP (char *CmdArg);

#define MaxATcmdLength   20

struct stcATCommand
{
    char Command[MaxATcmdLength];
    void (*Function) (char *);
};

void ProcessATCommand();

#endif // SWIFTLINK_ATCOMMANDS_H