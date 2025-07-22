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

//  TeensyROM: A C64 ROM emulator and loader/interface cartidge based on the Teensy 4.1
//  Copyright (c) 2023 Travis Smith <travis@sensoriumembedded.com> 

#ifndef NFCSCAN_H
#define NFCSCAN_H

#include <USBHost_t36.h>
#include "PN532.h"
#include "PN532_HSU.h"

#include "midi2sid.h"

USBHIDParser hid1(myusbHost);
USBHIDParser hid2(myusbHost);
USBHIDParser hid3(myusbHost);  //need all 3?

USBSerial userial(myusbHost);  // works only for those Serial devices who transfer <=64 bytes (like T3.x, FTDI...)
//USBSerial_BigBuffer userial(myusbHost, 1); // Handles anything up to 512 bytes
//USBSerial_BigBuffer userial(myusbHost); // Handles up to 512 but by default only for those > 64 bytes
PN532_UHSU pn532uhsu(userial);
PN532 nfc(pn532uhsu);

#define MaxNfcConfRetries    20
#define NFCReReadTimeout   1000  // mS since of no scan to re-accept same tag

uint8_t  Lastuid[7];  // Buffer to store the last UID read
uint8_t  LastuidLength = 7;
uint32_t LastTagMillis = 0; //stores last good tag time for Lastuid timeout/allow retag

FLASHMEM void nfcInit();
FLASHMEM bool nfcConfigCheck();
void nfcCheck();
RegMenuTypes RegMenuTypeFromFileName(char** ptrptrFileName);
FS *FSfromSourceID(RegMenuTypes SourceID);
bool nfcReadTagLaunch(uint8_t* uid, uint8_t uidLength);
FLASHMEM void nfcWriteTag(const char* TxtMsg);

#endif // NFCSCAN_H