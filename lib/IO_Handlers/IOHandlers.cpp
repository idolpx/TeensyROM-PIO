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

#ifdef MinimumBuild
// Variables and stubs normally defined in IOH_TeensyROM.cpp, which is excluded in MinimumBuild
volatile uint8_t* IO1 = NULL;
volatile uint8_t doReset = false;
uint16_t NumItemsFull = 0;
StructMenuItem* MenuSource = NULL;
uint16_t SelItemFullIdx = 0;
uint16_t StreamOffsetAddr = 0;

// Simple ASCII to PETSCII conversion table (normally in IOH_TeensyROM.cpp)
uint8_t ASCIItoPETSCII[128] = {
   0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,
  16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
  32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,
  48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,
  64,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,
  112,113,114,115,116,117,118,119,120,121,122,91,92,93,94,32,
  96,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,
  80,81,82,83,84,85,86,87,88,89,90,123,124,125,126,127
};

// Stub functions for MinimumBuild
void MakeBuildInfo() {}
int16_t FindTRMenuItem(StructMenuItem* SrcMenuItem, uint16_t NumSrcItems, char* SearchName) { return -1; }
void SetSIDSpeed(bool IsNTSC, int16_t ClockAdj) {}
void UpDirectory() {}
#endif

stcIOHandlers* IOHandler[] = {
   &IOHndlr_None,               //IOH_None,
#ifndef MinimumBuild
   &IOHndlr_SwiftLink,          //IOH_Swiftlink,
   &IOHndlr_MIDI_Datel,         //IOH_MIDI_Datel,
   &IOHndlr_MIDI_Sequential,    //IOH_MIDI_Sequential,
   &IOHndlr_MIDI_Passport,      //IOH_MIDI_Passport,
   &IOHndlr_MIDI_NamesoftIRQ,   //IOH_MIDI_NamesoftIRQ,
   &IOHndlr_Debug,              //IOH_Debug, //last manually selectable, see LastSelectableIOH

   &IOHndlr_TeensyROM,          //IOH_TeensyROM,
   &IOHndlr_ASID,               //IOH_ASID,
   &IOHndlr_TR_BASIC,           //IOH_TR_BASIC,
#endif
   &IOHndlr_EpyxFastLoad,       //IOH_EpyxFastLoad,
   &IOHndlr_MagicDesk,          //IOH_MagicDesk,
   &IOHndlr_Dinamic,            //IOH_Dinamic,
   &IOHndlr_Ocean1,             //IOH_Ocean1,
   &IOHndlr_FunPlay,            //IOH_FunPlay,
   &IOHndlr_SuperGames,         //IOH_SuperGames,
   &IOHndlr_C64GameSystem3,     //IOH_C64GameSystem3,
   &IOHndlr_EasyFlash,          //IOH_EasyFlash,
   &IOHndlr_ZaxxonSuper,        //IOH_ZaxxonSuper,
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
#ifndef MinimumBuild
   if (IO1[rWRegCurrMenuWAIT] == rmtTeensy && MenuSource[SelItemFullIdx].IOHndlrAssoc != IOH_None)
   {
      Printf_dbg("IO Handler set by Teensy Menu\n");
      IOHandlerInit(MenuSource[SelItemFullIdx].IOHndlrAssoc);
   }
   else
#endif
      IOHandlerNextInit();
}

void IOHandlerInit(uint8_t NewIOHandler)
{ //called from above and directly from SetUpMainMenuROM
#ifndef MinimumBuild
   SetMIDIHandlersNULL();
   MIDIRxIRQEnabled = false;
   MIDIRxBytesToSend = 0;
   rIORegMIDIStatus = 0;
#endif
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

#ifndef MinimumBuild
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
#endif
