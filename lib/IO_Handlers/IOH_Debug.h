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

#ifndef IOH_DEBUG_H
#define IOH_DEBUG_H

#include "IOH_defs.h"

#define DEBUG_MEMLOC   FLASHMEM

void IO1Hndlr_Debug (uint8_t Address, bool R_Wn);
void PollingHndlr_Debug();
void InitHndlr_Debug();

extern stcIOHandlers IOHndlr_Debug;


//MIDI input handlers for Debug _________________________________________________________________________

DEBUG_MEMLOC void DbgOnNoteOff (uint8_t channel, uint8_t note, uint8_t velocity);
DEBUG_MEMLOC void DbgOnNoteOn (uint8_t channel, uint8_t note, uint8_t velocity);
DEBUG_MEMLOC void DbgOnAfterTouchPoly (uint8_t channel, uint8_t note, uint8_t velocity);
DEBUG_MEMLOC void DbgOnControlChange (uint8_t channel, uint8_t control, uint8_t value);
DEBUG_MEMLOC void DbgOnProgramChange (uint8_t channel, uint8_t program);
DEBUG_MEMLOC void DbgOnAfterTouch (uint8_t channel, uint8_t pressure);
DEBUG_MEMLOC void DbgOnPitchChange (uint8_t channel, int pitch);

// F0 SysEx single call, message larger than buffer is truncated
DEBUG_MEMLOC void DbgOnSystemExclusive (uint8_t *data, unsigned int size);
DEBUG_MEMLOC void DbgOnTimeCodeQuarterFrame (uint8_t data);
DEBUG_MEMLOC void DbgOnSongPosition (uint16_t beats);
DEBUG_MEMLOC void DbgOnSongSelect (uint8_t songnumber);
DEBUG_MEMLOC void DbgOnTuneRequest (void);

// F8-FF (except FD)
DEBUG_MEMLOC void DbgOnRealTimeSystem (uint8_t realtimebyte);

#endif // IOH_DEBUG_H