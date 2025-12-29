
#ifndef MIDI2SID_H
#define MIDI2SID_H

#include <Arduino.h>
#include <USBHost_t36.h>
#include <MIDI.h>

#include "IOHandlers.h"
#include "midiDevName.h"

#define usbDevMIDI usbMIDI

extern USBHost myusbHost;
//extern USBHub hub1;
//extern USBHub hub2;
//extern USBDrive myDrive;
extern USBFilesystem firstPartition;

extern MIDIDevice usbHostMIDI;
extern USBSerial USBHostSerial;

// MIDI input/voice handlers for MIDI2SID _________________________________________________________________________

int FindVoiceUsingNote (int NoteNum);
int FindFreeVoice();
void M2SOnNoteOn (uint8_t channel, uint8_t note, uint8_t velocity);
void M2SOnNoteOff (uint8_t channel, uint8_t note, uint8_t velocity);
void M2SOnControlChange (uint8_t channel, uint8_t control, uint8_t value);
void M2SOnPitchChange (uint8_t channel, int pitch);

#endif // MIDI2SID_H