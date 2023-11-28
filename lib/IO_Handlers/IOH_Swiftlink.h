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
#include <NativeEthernetUdp.h>

#include "IOH_struct.h"
#include "TeensyROM.h"

#include "Swift_RxQueue.h"
#include "Swift_ATcommands.h"
#include "Swift_Browser.h"

EthernetUDP udp;
EthernetClient client;

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

#define NumPageLinkBuffs    99
#define NumPrevURLQueues    8
#define MaxTagSize          300
#define TxMsgMaxSize        128
#define BytesPerDot         (25*1024) //dot every 25k when downloading
#define RxQueueNumBlocks    40 
#define RxQueueBlockSize    (1024*8) // 40*8k=320k
#define RxQueueSize         (RxQueueNumBlocks*RxQueueBlockSize) 
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

// status reg flags
#define SwiftStatusIRQ      0x80     // high if ACIA caused interrupt;
#define SwiftStatusDSR      0x40     // reflects state of DSR line
#define SwiftStatusDCD      0x20     // reflects state of DCD line
#define SwiftStatusTxEmpty  0x10 // high if xmit-data register is empty
#define SwiftStatusRxFull   0x08  // high if receive-data register full
#define SwiftStatusErrOver  0x04 // high if overrun error
#define SwiftStatusErrFram  0x02 // high if framing error
#define SwiftStatusErrPar   0x01  // high if parity error

// command reg flags
#define SwiftCmndRxIRQEn    0x02 // low if Rx IRQ enabled
#define SwiftCmndDefault    0xE0 // Default command reg state

extern volatile uint32_t CycleCountdown;
extern void EEPreadNBuf(uint16_t addr, uint8_t *buf, uint8_t len);
extern void EEPwriteNBuf(uint16_t addr, const uint8_t *buf, uint8_t len);
extern void EEPwriteStr(uint16_t addr, const char *buf);
extern void EEPreadStr(uint16_t addr, char *buf);

uint8_t* RxQueue[RxQueueNumBlocks];  //circular queue to pipe data to the c64, divided into blocks for better malloc
char *TxMsg = NULL;                          // to hold messages (AT/browser commands) when off line
char *PageLinkBuff[NumPageLinkBuffs];        // hold links from tags for user selection in browser
stcURLParse *PrevURLQueue[NumPrevURLQueues]; // For browse previous
char CurrPageTitle[eepBMTitleSize];          // keep current page title, could move to RAM2

uint8_t PrevURLQueueNum;   // current/latest in the link history queue
uint8_t UsedPageLinkBuffs; // how many PageLinkBuff elements have been Used
uint32_t RxQueueHead, RxQueueTail, TxMsgOffset;
bool ConnectedToHost, BrowserMode, PagePaused, PrintingHyperlink;
uint32_t PageCharsReceived;
uint32_t NMIassertMicros;
volatile uint8_t SwiftTxBuf, SwiftRxBuf;
volatile uint8_t SwiftRegStatus, SwiftRegCommand, SwiftRegControl;
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