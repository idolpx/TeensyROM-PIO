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

#ifndef IOHANDLERS_H_
#define IOHANDLERS_H_

#define nfcStateEnabled       0
#define nfcStateBitDisabled   1
#define nfcStateBitPaused     2

#ifndef MinimumBuild
#include "IOH_ASID.h"
#include "IOH_Debug.h"
#include "IOH_MIDI.h"
#include "IOH_Swiftlink.h"
#include "IOH_TeensyROM.h"
#include "IOH_TR_BASIC.h"
#endif

#include "IOH_C64GameSystem3.h"
#include "IOH_Dinamic.h"
#include "IOH_EasyFlash.h"
#include "IOH_EpyxFastLoad.h"
#include "IOH_FunPlay.h"
#include "IOH_GMod2.h"
#include "IOH_MagicDesk.h"
#include "IOH_MagicDesk2.h"
#include "IOH_None.h"
#include "IOH_Ocean1.h"
#include "IOH_SuperGames.h"
#include "IOH_ZaxxonSuper.h"


extern stcIOHandlers *IOHandler[];  //Synch order/qty with enum enumIOHandlers
extern uint8_t CurrentIOHandler;

#ifdef MinimumBuild
// Variables normally defined in IOH_TeensyROM.cpp, which is excluded in MinimumBuild
extern volatile uint8_t *IO1;
extern volatile uint8_t doReset;
extern uint16_t NumItemsFull;
#endif

void IOHandlerNextInit();
void IOHandlerSelectInit();
void IOHandlerInit (uint8_t NewIOHandler);
void NothingOnSystemExclusive (uint8_t *data, unsigned int size);
void SetMIDIHandlersNULL();

#endif // IO_HANDLERS