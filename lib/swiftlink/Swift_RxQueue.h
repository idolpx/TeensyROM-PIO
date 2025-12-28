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

#ifndef SWIFT_RXQUEUE_H
#define SWIFT_RXQUEUE_H

#include <Arduino.h>
#include <stdint.h>
#include <IPAddress.h>

#define RxQueueNumBlocks    40 
#define RxQueueBlockSize   (1024*8) // 40*8k=320k
#define RxQueueSize        (RxQueueNumBlocks*RxQueueBlockSize) 
#define NMITimeoutuS       300    //if Rx data not read within this time, deassert NMI anyway

extern uint32_t RxQueueHead, RxQueueTail, TxMsgOffset;
extern uint8_t* RxQueue[RxQueueNumBlocks];  //circular queue to pipe data to the c64, divided into blocks for better malloc

#define RxQueueUsed ((RxQueueHead>=RxQueueTail)?(RxQueueHead-RxQueueTail):(RxQueueHead+RxQueueSize-RxQueueTail))

uint8_t PullFromRxQueue();
bool ReadyToSendRx();
bool CheckRxNMITimeout();
void SendRxByte(uint8_t ToSend);
void CheckSendRxQueue();
void FlushRxQueue();
void AddRawCharToRxQueue(uint8_t c);
void AddRawStrToRxQueue(const char* s);
void AddToPETSCIIStrToRxQueue(const char* s);
void AddToPETSCIIStrToRxQueueLN(const char* s);

FLASHMEM void AddIPaddrToRxQueueLN(IPAddress ip);
FLASHMEM void AddMACToRxQueueLN(uint8_t* mac);
FLASHMEM void AddInvalidFormatToRxQueueLN();
FLASHMEM void AddUpdatedToRxQueueLN();
FLASHMEM void AddDHCPEnDisToRxQueueLN();
FLASHMEM void AddDHCPTimeoutToRxQueueLN();
FLASHMEM void AddDHCPRespTOToRxQueueLN();
FLASHMEM void Add_BR_ToRxQueue();

//From Dns.cpp of ethernet lib, didn't want the baggage
//  Arduino DNS client for WizNet5100-based Ethernet shield
//  (c) Copyright 2009-2010 MCQN Ltd.
//  Released under Apache License, version 2.0
bool inet_aton(const char* address, IPAddress& result);

#endif // SWIFT_RXQUEUE_H