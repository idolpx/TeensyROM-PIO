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

//Functions to control C64/TR via USB connection


//  TR: Set up wRegIRQ_ACK, rwRegIRQ_CMD, & launch menu info (if needed)
//  TR: Assert IRQ, wait for ack1
// C64: IRQ handler catches, sets local reg (smcIRQFlagged) and sends ack1 to wRegIRQ_ACK
//  TR: sees ack1, deasserts IRQ, waits for ack2 (echo of command)
// C64: Main code sees local reg, reads IRQ command, and echoes it to ack (ack 2)
//  TR: sees ack2, success/return
// C64: Executes command

#ifndef REMOTECONTROL_H
#define REMOTECONTROL_H

#include "Common_Defs.h"
#include "IOH_TeensyROM.h"
#include "Menu_Regs.h"

extern bool RemoteLaunched;            // last app was launched remotely

bool InterruptC64 (RegIRQCommands IRQCommand);
bool DoC64IRQ();
FLASHMEM bool RemotePauseSID();

// Command:
// Set one of the colors in the TR UI
//    Color will be stored in EEPROM and IO space, TR menu will need to be reset to take visual effect
//
// Workflow:
// Receive <-- SetColorToken Token 0x6422
// Receive <-- Color reference to set (Range 0 to NumColorRefs-1) See enum ColorRefOffsets
// Receive <-- Color to set (Range 0 to 15)  See enum PokeColors
// Send --> AckToken 0x64CC or FailToken 0x9B7F
FLASHMEM bool SetColorRef();

// Command:
// Set sub-song number of currently loaded SID
//
// Workflow:
// Receive <-- SetSIDSongToken Token 0x6488
// Receive <-- Song number to set (1 byte, zero based, song 1 is 0)
// Send --> AckToken 0x64CC or FailToken 0x9B7F
FLASHMEM bool SetSIDSong();

// Command:
// Set SID playback speed of currently loaded SID
//
// Workflow:
// Receive <-- SIDSpeedLinToken  0x6499 -or- SIDSpeedLogToken  0x649A
// Receive <-- playback rate (16 bit signed int as 2 bytes: hi, lo)
//                Linear Range is -68(*256) to <128(*256), argument represents speed change percent from nominal
//                Logrithmic Range is -127(*256) to 99(*256) argument to percentage shown in "SID playback speed-log.txt"
// Send --> AckToken 0x64CC or FailToken 0x9B7F
//
// Example 1: 0x64, 0x99, 0xf0, 0x40 = Set to -15.75 via linear equation
// Example 2: 0x64, 0x9a, 0x20, 0x40 = set to +32.25 via logarithmic equation
FLASHMEM bool RemoteSetSIDSpeed (bool LogConv);

// Command:
// Set individual SID voice muting
//
// Workflow:
// Receive <-- SIDVoiceMuteToken   0x6433
// Receive <-- voice mute info (1 byte)
//                bit 0=  Voice 1  on=0, mute=1
//                bit 1=  Voice 2  on=0, mute=1
//                bit 2=  Voice 3  on=0, mute=1
//             bits 7:3= Zero
// Send --> AckToken 0x64CC or FailToken 0x9B7F
FLASHMEM bool RemoteSetSIDVoiceMute();
void EEPRemoteLaunch (uint16_t eepAdNameToLaunch);
void RemoteLaunch (RegMenuTypes MenuSourceID, const char *FileNamePath, bool DoCartDirect);

#endif // REMOTECONTROL_H