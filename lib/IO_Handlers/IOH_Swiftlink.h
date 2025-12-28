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


//Network, 6551 ACIA interface emulation
//with AT Command sub-system and Internet Browser

#ifndef IOH_SWIFTLINK_H
#define IOH_SWIFTLINK_H

#include <stdint.h>
#include <stdbool.h>

#include "Swift_ATcommands.h"
#include "Swift_Browser.h"
#include "Swift_RxQueue.h"

#include "IOH_defs.h"

void IO1Hndlr_SwiftLink(uint8_t Address, bool R_Wn);
void PollingHndlr_SwiftLink();
void InitHndlr_SwiftLink();
void CycleHndlr_SwiftLink();

extern stcIOHandlers IOHndlr_SwiftLink;


// 6551 ACIA interface emulation
//register locations (IO1, DExx)
#define IORegSwiftData     0x00   // Swift Emulation Data Reg
#define IORegSwiftStatus   0x01   // Swift Emulation Status Reg
#define IORegSwiftCommand  0x02   // Swift Emulation Command Reg
#define IORegSwiftControl  0x03   // Swift Emulation Control Reg
#define IORegEnhancedSpeed 0x07   // Turbo232 Enhanced-Speed Reg

//status reg flags
#define SwiftStatusIRQ     0x80   // high if ACIA caused interrupt;
#define SwiftStatusDCD     0x40   // reflects state of DCD line
#define SwiftStatusDSR     0x20   // reflects state of DSR line
#define SwiftStatusTxEmpty 0x10   // high if xmit-data register is empty
#define SwiftStatusRxFull  0x08   // high if receive-data register full
#define SwiftStatusErrOver 0x04   // high if overrun error
#define SwiftStatusErrFram 0x02   // high if framing error
#define SwiftStatusErrPar  0x01   // high if parity error
#define SwiftStatusDefault (SwiftStatusTxEmpty|SwiftStatusDCD)   // default/power-up/reset value

//command reg flags
#define SwiftCmndRxIRQEn   0b00000010 // low if Rx IRQ enabled
#define SwiftCmndRTSTx     0b00001100 // RTS and Transmitter Ctl
#define SwiftCmndRTSRMask  0b00001110 //mask bits to check in ReadyToSendRx
#define SwiftCmndRTSRMatch 0b00001000 //masked ReadyToSendRx matching
#define SwiftCmndDefault   0b11100000 // Default command reg state

//Control Reg baud rate settings
enum enBaudRates
{
   Baud_Enhanced = 0,
   Baud_100         ,
   Baud_150         ,
   Baud_219         ,
   Baud_269         ,
   Baud_300         , 
   Baud_600         ,
   Baud_1200        ,
   Baud_2400        ,
   Baud_3600        ,
   Baud_4800        ,
   Baud_7200        ,
   Baud_9600        ,
   Baud_14400       ,
   Baud_19200       ,
   Baud_38400       , //15
   Baud_57600       ,
   Baud_115200      ,
   Baud_230400      ,
};

//PETSCII Colors/Special Symbols
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
#define PETSCIIorange      0x81

#define PETSCIIreturn      0x0d
#define PETSCIIrvsOn       0x12
#define PETSCIIrvsOff      0x92
#define PETSCIIclearScreen 0x93
#define PETSCIIcursorUp    0x91
#define PETSCIIhorizBar    0x60
#define PETSCIIspace       0x20
#define PETSCIIhiLoChrSet  0x0e
#define PETSCIIupGfxChrSet 0x8e



extern volatile uint32_t CycleCountdown;
extern volatile uint8_t BtnPressed;
extern void FreeDriveDirMenu();
extern void SetRandomSeed();

extern char* TxMsg;  //to hold messages (AT/browser commands) when off line
extern char* PageLinkBuff[NumPageLinkBuffs]; //hold links from tags for user selection in browser
extern stcURLParse* PrevURLQueue[NumPrevURLQueues]; //For browse previous
extern char CurrPageTitle[eepBMTitleSize]; //keep current page title, could move to RAM2

extern uint8_t  PrevURLQueueNum;   //current/latest in the link history queue
extern uint8_t  UsedPageLinkBuffs;   //how many PageLinkBuff elements have been Used

extern bool Verbose, EchoOn, ConnectedToHost, BrowserMode, PagePaused, PrintingHyperlink;
extern uint32_t PageCharsReceived;
extern uint32_t NMIassertMicros;
extern volatile uint8_t SwiftTxBuf, SwiftRxBuf;
extern volatile uint8_t SwiftRegStatus, SwiftRegCommand, SwiftRegControl, TurboRegEnhancedSpeed;
extern uint8_t PlusCount;
extern uint32_t C64CycBetweenRx, LastTxMillis;


// Browser mode: Buffer saved in ASCII from host, converted before sending out
//               Uses Send...Immediate  commands for direct output
// AT/regular:   Buffer saved in (usually) PETSCII from host
//               Uses Add...ToRxQueue for direct output

void ParseHTMLTag();
void ParseEntityReference();
void SetEthEEPDefaults();
void SendBrowserCommandsImmediate();
void UnPausePage();

void SetBaud(uint8_t BaudNum);
void FreeSwiftlinkBuffs();
FLASHMEM bool EthernetInit();
void SetRandomSeed();

#endif // IOH_SWIFTLINK_H