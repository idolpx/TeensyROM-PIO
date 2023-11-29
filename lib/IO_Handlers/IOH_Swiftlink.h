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

// Network, 6551 ACIA interface emulation
// with AT Command sub-system and Internet Browser

#ifndef IOH_SWIFTLINK_H
#define IOH_SWIFTLINK_H

#include <Arduino.h>
#include <NativeEthernet.h>

#include "../../include/TeensyROM.h"

#include "IOH_defs.h"

extern EthernetClient client;

void FreeSwiftlinkBuffs();
void IO1Hndlr_SwiftLink(uint8_t Address, bool R_Wn);
void PollingHndlr_SwiftLink();
void InitHndlr_SwiftLink();
void CycleHndlr_SwiftLink();

stcIOHandlers IOHndlr_SwiftLink =
{
    "SwiftLink/Modem",       // Name of handler
    &InitHndlr_SwiftLink,    // Called once at handler startup
    &IO1Hndlr_SwiftLink,     // IO1 R/W handler
    NULL,                    // IO2 R/W handler
    NULL,                    // ROML Read handler, in addition to any ROM data sent
    NULL,                    // ROMH Read handler, in addition to any ROM data sent
    &PollingHndlr_SwiftLink, // Polled in main routine
    &CycleHndlr_SwiftLink,   // called at the end of EVERY c64 cycle
};


#define MaxTagSize          300
#define TxMsgMaxSize        128

#define C64CycBetweenRx     2300   //stops NMI from re-asserting too quickly. chars missed in large buffs when lower
#define NMITimeoutnS        300    //if Rx data not read within this time, deassert NMI anyway
#define Drive_USB           1
#define Drive_SD            2

// 6551 ACIA interface emulation
// register locations (IO1, DExx)
#define IORegSwiftData      0x00    // Swift Emulation Data Reg
#define IORegSwiftStatus    0x01  // Swift Emulation Status Reg
#define IORegSwiftCommand   0x02 // Swift Emulation Command Reg
#define IORegSwiftControl   0x03 // Swift Emulation Control Reg


extern volatile uint32_t CycleCountdown;
extern void EEPreadNBuf(uint16_t addr, uint8_t *buf, uint8_t len);
extern void EEPwriteNBuf(uint16_t addr, const uint8_t *buf, uint8_t len);
extern void EEPwriteStr(uint16_t addr, const char *buf);
extern void EEPreadStr(uint16_t addr, char *buf);

char *TxMsg = NULL;                          // to hold messages (AT/browser commands) when off line
bool ConnectedToHost, PagePaused, PrintingHyperlink;

uint32_t NMIassertMicros;
volatile uint8_t SwiftTxBuf, SwiftRxBuf;

uint8_t PlusCount;
uint32_t LastTxMillis = millis();

// Browser mode: Buffer saved in ASCII from host, converted before sending out
//               Uses Send...Immediate  commands for direct output
// AT/regular:   Buffer saved in (usually) PETSCII from host
//               Uses Add...ToRxQueue for direct output

void ParseHTMLTag();
void SetEthEEPDefaults();
void SendBrowserCommandsImmediate();
void UnPausePage();

FLASHMEM bool EthernetInit();
FLASHMEM void SetEthEEPDefaults();

#endif // IOH_SWIFTLINK_H