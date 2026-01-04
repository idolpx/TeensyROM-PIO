// MIT License
//
// Copyright (c) 2025 Travis Smith
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

// https://github.com/crystalct/MagicDesk2

#ifndef IOH_MagicDesk2_H
#define IOH_MagicDesk2_H

#include "DriveDirLoad.h"
#include "IOH_defs.h"
#include "Common_Defs.h"

extern uint8_t *ImageCheckAssign (uint8_t* BankRequested);
extern bool PathIsRoot();
extern char DriveDirPath[];
extern StructMenuItem *DriveDirMenu;
extern File myFile;

extern void LoadBank (uint32_t SeekTo, uint8_t* ptrImage);


void InitHndlr_MagicDesk2();
void IO1Hndlr_MagicDesk2 (uint8_t Address, bool R_Wn);
void PollingHndlr_MagicDesk2();

extern stcIOHandlers IOHndlr_MagicDesk2;

#endif // IOH_MAGICDESK2_H
