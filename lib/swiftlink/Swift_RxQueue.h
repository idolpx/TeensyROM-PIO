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
bool inet_aton(const char* address, IPAddress& result);
FLASHMEM void AddDHCPTimeoutToRxQueueLN();
FLASHMEM void AddDHCPRespTOToRxQueueLN();
FLASHMEM void StrToIPToEE(char* Arg, uint8_t EEPaddress);


#endif // SWIFTLINK_RXQUEUE_H