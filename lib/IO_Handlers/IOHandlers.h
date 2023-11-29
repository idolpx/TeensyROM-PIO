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

#ifndef IOHANDLERS_H
#define IOHANDLERS_H

#include <Arduino.h>

#include "IOH_defs.h"

#include "IOH_None.h"
#include "IOH_MIDI.h"
#include "IOH_Debug.h"
#include "IOH_TeensyROM.h"
#include "IOH_Swiftlink.h"
#include "IOH_EpyxFastLoad.h"
#include "IOH_MagicDesk.h"
#include "IOH_Dinamic.h"
#include "IOH_Ocean1.h"
#include "IOH_FunPlay.h"
#include "IOH_SuperGames.h"
#include "IOH_C64GameSystem3.h"
#include "IOH_EasyFlash.h"
#include "IOH_ZaxxonSuper.h"


stcIOHandlers *IOHandler[] =  //Synch order/qty with enum enumIOHandlers
{
    &IOHndlr_None,               //IOH_None,
    &IOHndlr_SwiftLink,          //IOH_Swiftlink,
    &IOHndlr_MIDI_Datel,         //IOH_MIDI_Datel,
    &IOHndlr_MIDI_Sequential,    //IOH_MIDI_Sequential,
    &IOHndlr_MIDI_Passport,      //IOH_MIDI_Passport,
    &IOHndlr_MIDI_NamesoftIRQ,   //IOH_MIDI_NamesoftIRQ,
    &IOHndlr_Debug,              //IOH_Debug, //last manually selectable, see LastSelectableIOH

    &IOHndlr_TeensyROM,          //IOH_TeensyROM,
    &IOHndlr_EpyxFastLoad,       //IOH_EpyxFastLoad,
    &IOHndlr_MagicDesk,          //IOH_MagicDesk,
    &IOHndlr_Dinamic,            //IOH_Dinamic,
    &IOHndlr_Ocean1,             //IOH_Ocean1,
    &IOHndlr_FunPlay,            //IOH_FunPlay,
    &IOHndlr_SuperGames,         //IOH_SuperGames,
    &IOHndlr_C64GameSystem3,     //IOH_C64GameSystem3,
    &IOHndlr_EasyFlash,          //IOH_EasyFlash,
    &IOHndlr_ZaxxonSuper,        //IOH_ZaxxonSuper,

};

uint8_t CurrentIOHandler = IOH_None;

void IOHandlerInit (uint8_t NewIOHandler);
void IOHandlerInitToNext();

// F0 SysEx single call, message larger than buffer is truncated
void NothingOnSystemExclusive (uint8_t *data, unsigned int size);
void SetMIDIHandlersNULL();

#endif // IOHANDLERS_H