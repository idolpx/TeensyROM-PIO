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


// Swiftlink Rx Queue Functions

#ifndef SWIFTLINK_RXQUEUE_H
#define SWIFTLINK_RXQUEUE_H

#include <Arduino.h>
#include <IPAddress.h>

#define BytesPerDot         (25*1024) //dot every 25k when downloading
#define RxQueueNumBlocks    40
#define RxQueueBlockSize    (1024*8) // 40*8k=320k
#define RxQueueSize         (RxQueueNumBlocks*RxQueueBlockSize)

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

#define NMITimeoutnS        300    //if Rx data not read within this time, deassert NMI anyway

uint32_t NMIassertMicros;
volatile uint8_t SwiftTxBuf, SwiftRxBuf;

uint8_t *RxQueue[RxQueueNumBlocks];  //circular queue to pipe data to the c64, divided into blocks for better malloc
uint32_t RxQueueHead, RxQueueTail, TxMsgOffset;

volatile uint8_t SwiftRegStatus, SwiftRegCommand, SwiftRegControl;

uint8_t PullFromRxQueue();
bool ReadyToSendRx();
bool CheckRxNMITimeout();
void SendRxByte (uint8_t ToSend);
void CheckSendRxQueue();
void FlushRxQueue();

void AddRawCharToRxQueue (uint8_t c);
void AddRawStrToRxQueue (const char *s);
void AddToPETSCIIStrToRxQueue (const char *s);
void AddToPETSCIIStrToRxQueueLN (const char *s);

FLASHMEM void AddIPaddrToRxQueueLN (IPAddress ip);
FLASHMEM void AddMACToRxQueueLN (uint8_t *mac);
FLASHMEM void AddInvalidFormatToRxQueueLN();
FLASHMEM void AddUpdatedToRxQueueLN();
FLASHMEM void AddDHCPEnDisToRxQueueLN();
bool inet_aton (const char *address, IPAddress &result);
FLASHMEM void AddDHCPTimeoutToRxQueueLN();
FLASHMEM void AddDHCPRespTOToRxQueueLN();
FLASHMEM void StrToIPToEE (char *Arg, uint8_t EEPaddress);


#endif // SWIFTLINK_RXQUEUE_H