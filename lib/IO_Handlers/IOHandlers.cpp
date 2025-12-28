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

#include "IOHandlers.h"

#include "midi2sid.h"

// Global variable definitions
const unsigned char *HIROM_Image = NULL;
const unsigned char *LOROM_Image = NULL;
uint16_t LOROM_Mask = 0, HIROM_Mask = 0;
volatile uint8_t DMA_State = DMA_S_DisableReady;

stcIOHandlers* IOHandler[] = {
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
   &IOHndlr_ASID,               //IOH_ASID,
   &IOHndlr_TR_BASIC,           //IOH_TR_BASIC,
   &IOHndlr_GMod2,              //IOH_GMod2,
   &IOHndlr_MagicDesk2,         //IOH_MagicDesk2,
};

uint8_t CurrentIOHandler = IOH_None;

void IOHandlerNextInit()
{
   Printf_dbg("Default IO Handler\n");
   IOHandlerInit(IO1[rwRegNextIOHndlr]);
}

void IOHandlerSelectInit()
{ //called after cart loaded, PRG x-fer finished, or exit to basic (rsIOHWSelInit)
   if (IO1[rWRegCurrMenuWAIT] == rmtTeensy && MenuSource[SelItemFullIdx].IOHndlrAssoc != IOH_None)
   {
      Printf_dbg("IO Handler set by Teensy Menu\n");
      IOHandlerInit(MenuSource[SelItemFullIdx].IOHndlrAssoc); 
   }
   else IOHandlerNextInit();
}

void IOHandlerInit(uint8_t NewIOHandler)
{ //called from above and directly from SetUpMainMenuROM
   SetMIDIHandlersNULL();
   MIDIRxIRQEnabled = false;
   MIDIRxBytesToSend = 0;
   rIORegMIDIStatus = 0;
   BigBufCount = 0;
   
   if (NewIOHandler>=IOH_Num_Handlers)
   {
      Printf_dbg("***IOHandler out of range\n");
      return;
   }
   
   Serial.printf("Loading IO handler: %s\n", IOHandler[NewIOHandler]->Name);
   
   if (IOHandler[NewIOHandler]->InitHndlr != NULL) IOHandler[NewIOHandler]->InitHndlr();
   
   Serial.flush();
   CurrentIOHandler = NewIOHandler;
}

// F0 SysEx single call, message larger than buffer is truncated
void NothingOnSystemExclusive(uint8_t *data, unsigned int size) 
{
   //Setting handler to NULL creates ambiguous error
}

void SetMIDIHandlersNULL()
{
   usbHostMIDI.setHandleNoteOff             (NULL); // 8x
   usbHostMIDI.setHandleNoteOn              (NULL); // 9x
   usbHostMIDI.setHandleAfterTouchPoly      (NULL); // Ax
   usbHostMIDI.setHandleControlChange       (NULL); // Bx
   usbHostMIDI.setHandleProgramChange       (NULL); // Cx
   usbHostMIDI.setHandleAfterTouch          (NULL); // Dx
   usbHostMIDI.setHandlePitchChange         (NULL); // Ex
   usbHostMIDI.setHandleSystemExclusive     (NothingOnSystemExclusive); // F0   
   usbHostMIDI.setHandleTimeCodeQuarterFrame(NULL); // F1
   usbHostMIDI.setHandleSongPosition        (NULL); // F2
   usbHostMIDI.setHandleSongSelect          (NULL); // F3
   usbHostMIDI.setHandleTuneRequest         (NULL); // F6
   usbHostMIDI.setHandleRealTimeSystem      (NULL); // F8-FF (except FD)

   usbDevMIDI.setHandleNoteOff              (NULL); // 8x
   usbDevMIDI.setHandleNoteOn               (NULL); // 9x
   usbDevMIDI.setHandleAfterTouchPoly       (NULL); // Ax
   usbDevMIDI.setHandleControlChange        (NULL); // Bx
   usbDevMIDI.setHandleProgramChange        (NULL); // Cx
   usbDevMIDI.setHandleAfterTouch           (NULL); // Dx
   usbDevMIDI.setHandlePitchChange          (NULL); // Ex
   usbDevMIDI.setHandleSystemExclusive      (NothingOnSystemExclusive); // F0   
   usbDevMIDI.setHandleTimeCodeQuarterFrame (NULL); // F1
   usbDevMIDI.setHandleSongPosition         (NULL); // F2
   usbDevMIDI.setHandleSongSelect           (NULL); // F3
   usbDevMIDI.setHandleTuneRequest          (NULL); // F6
   usbDevMIDI.setHandleRealTimeSystem       (NULL); // F8-FF (except FD)
}
