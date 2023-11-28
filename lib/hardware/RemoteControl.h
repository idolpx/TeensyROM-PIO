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

#include "Menu_Regs.h"

bool RemoteLaunched = false; //last app was launched remotely

bool InterruptC64(RegIRQCommands IRQCommand);
bool DoC64IRQ();
bool RemotePauseSID();
void RemoteLaunch(bool SD_nUSB, const char *FileNamePath);

#endif // REMOTECONTROL_H