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

#ifndef IOH_MIDI_H
#define IOH_MIDI_H

#include "midiDevName.h"

#include <Arduino.h>

#include "IOH_defs.h"
#include "SerUSBIO.h"

// IO Handler for MIDI (6580 ACIA interface) Emulation

void IO1Hndlr_MIDI(uint8_t Address, bool R_Wn);
void PollingHndlr_MIDI();
void InitHndlr_MIDI_Datel();
void InitHndlr_MIDI_Sequential();
void InitHndlr_MIDI_Passport();
void InitHndlr_MIDI_NamesoftIRQ();

stcIOHandlers IOHndlr_MIDI_Datel =
{
    "MIDI:Datel/Siel",     // Name of handler
    &InitHndlr_MIDI_Datel, // Called once at handler startup
    &IO1Hndlr_MIDI,        // IO1 R/W handler
    NULL,                  // IO2 R/W handler
    NULL,                  // ROML Read handler, in addition to any ROM data sent
    NULL,                  // ROMH Read handler, in addition to any ROM data sent
    &PollingHndlr_MIDI,    // Polled in main routine
    NULL,                  // called at the end of EVERY c64 cycle
};

stcIOHandlers IOHndlr_MIDI_Sequential =
{
    "MIDI:Sequential",          // Name of handler
    &InitHndlr_MIDI_Sequential, // Called once at handler startup
    &IO1Hndlr_MIDI,             // IO1 R/W handler
    NULL,                       // IO2 R/W handler
    NULL,                       // ROML Read handler, in addition to any ROM data sent
    NULL,                       // ROMH Read handler, in addition to any ROM data sent
    &PollingHndlr_MIDI,         // Polled in main routine
    NULL,                       // called at the end of EVERY c64 cycle
};

stcIOHandlers IOHndlr_MIDI_Passport =
{
    "MIDI:Passport/Sent",     // Name of handler
    &InitHndlr_MIDI_Passport, // Called once at handler startup
    &IO1Hndlr_MIDI,           // IO1 R/W handler
    NULL,                     // IO2 R/W handler
    NULL,                     // ROML Read handler, in addition to any ROM data sent
    NULL,                     // ROMH Read handler, in addition to any ROM data sent
    &PollingHndlr_MIDI,       // Polled in main routine
    NULL,                     // called at the end of EVERY c64 cycle
};

stcIOHandlers IOHndlr_MIDI_NamesoftIRQ =
{
    "MIDI:Namesoft IRQ",         // Name of handler
    &InitHndlr_MIDI_NamesoftIRQ, // Called once at handler startup
    &IO1Hndlr_MIDI,              // IO1 R/W handler
    NULL,                        // IO2 R/W handler
    NULL,                        // ROML Read handler, in addition to any ROM data sent
    NULL,                        // ROMH Read handler, in addition to any ROM data sent
    &PollingHndlr_MIDI,          // Polled in main routine
    NULL,                        // called at the end of EVERY c64 cycle
};

// see https://codebase64.org/doku.php?id=base:c64_midi_interfaces
//  6580 ACIA interface emulation
// rIORegMIDIStatus:
#define MIDIStatusIRQReq 0x80 // Interrupt Request
#define MIDIStatusDCD 0x04    // Data Carrier Detect (Ready to receive Tx data)
#define MIDIStatusTxRdy 0x02  // Transmit Data Register Empty (Ready to receive Tx data)
#define MIDIStatusRxFull 0x01 // Receive Data Register Full (Rx Data waiting to be read)

// #define NumMIDIControls   16  //must be power of 2, may want to do this differently?
// uint8_t MIDIControlVals[NumMIDIControls];

volatile uint8_t rIORegMIDIStatus = 0;
volatile uint8_t MIDIRxIRQEnabled = false;
volatile uint16_t MIDIRxBytesToSend = 0;
volatile uint8_t MIDIRxBuf[USB_MIDI_SYSEX_MAX]; // currently 290, defined in cores\teensy4\usb_midi.h
volatile uint8_t MIDITxBytesReceived = 0;
volatile uint8_t MIDITxBuf[3];
uint8_t wIORegAddrMIDIControl, rIORegAddrMIDIStatus, wIORegAddrMIDITransmit, rIORegAddrMIDIReceive;

// MIDI input handlers for HW Emulation _________________________________________________________________________
// Only called if MIDIRxBytesToSend==0 (No data waiting)

void SetMidiIRQ();
void HWEOnNoteOff(uint8_t channel, uint8_t note, uint8_t velocity);
void HWEOnNoteOn(uint8_t channel, uint8_t note, uint8_t velocity);
void HWEOnAfterTouchPoly(uint8_t channel, uint8_t note, uint8_t velocity);
void HWEOnControlChange(uint8_t channel, uint8_t control, uint8_t value);
void HWEOnProgramChange(uint8_t channel, uint8_t program);
void HWEOnAfterTouch(uint8_t channel, uint8_t pressure);
void HWEOnPitchChange(uint8_t channel, int pitch);

// F0 SysEx single call, message larger than buffer is truncated
void HWEOnSystemExclusive(uint8_t *data, unsigned int size);
void HWEOnTimeCodeQuarterFrame(uint8_t data);
void HWEOnSongPosition(uint16_t beats);
void HWEOnSongSelect(uint8_t songnumber);
void HWEOnTuneRequest();

// F8-FF (except FD)
void HWEOnRealTimeSystem(uint8_t realtimebyte);

#endif // IOH_MIDI_H