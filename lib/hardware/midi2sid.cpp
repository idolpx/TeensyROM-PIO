
#include "midi2sid.h"

#include <Arduino.h>

#include "Menu_Regs.h"
#include "IOH_TeensyROM.h"

// Global variable definitions
USBHost myusbHost;
USBFilesystem firstPartition(myusbHost);
MIDIDevice usbHostMIDI(myusbHost);

// MIDI input/voice handlers for MIDI2SID _________________________________________________________________________

#define NUM_VOICES 3
const char NoteName[12][3] = {" a", "a#", " b", " c", "c#", " d", "d#", " e", " f", "f#", " g", "g#"};

struct stcVoiceInfo
{
    bool Available;
    uint16_t NoteNumUsing;
};

stcVoiceInfo Voice[NUM_VOICES] =
{
    // voice table for poly synth
    true,
    0,
    true,
    0,
    true,
    0,
};

int FindVoiceUsingNote (int NoteNum)
{
    for (int VoiceNum = 0; VoiceNum < NUM_VOICES; VoiceNum++)
    {
        if (Voice[VoiceNum].NoteNumUsing == NoteNum && !Voice[VoiceNum].Available)
            return (VoiceNum);
    }
    return (-1);
}

int FindFreeVoice()
{
    for (int VoiceNum = 0; VoiceNum < NUM_VOICES; VoiceNum++)
    {
        if (Voice[VoiceNum].Available)
            return (VoiceNum);
    }
    return (-1);
}

void M2SOnNoteOn (uint8_t channel, uint8_t note, uint8_t velocity)
{
    note += 3; // offset to A centered from C
    int VoiceNum = FindFreeVoice();
    if (VoiceNum < 0)
    {
        IO1[rRegSIDOutOfVoices] = 'x';
        #ifdef DbgMsgs_M2S
        Serial.println ("Out of Voices!");
        #endif
        return;
    }

    float Frequency = 440 * pow (1.059463094359, note - 60);
    uint32_t RegVal = Frequency * 16777216 / NTSCBusFreq;

    if (RegVal > 0xffff)
    {
        #ifdef DbgMsgs_M2S
        Serial.println ("Too high!");
        #endif
        return;
    }

    Voice[VoiceNum].Available = false;
    Voice[VoiceNum].NoteNumUsing = note;
    IO1[rRegSIDFreqLo1 + VoiceNum * 7] = RegVal; // 7 regs per voice
    IO1[rRegSIDFreqHi1 + VoiceNum * 7] = (RegVal >> 8);
    IO1[rRegSIDVoicCont1 + VoiceNum * 7] |= 0x01; // start ADSR
    IO1[rRegSIDStrStart + VoiceNum * 4 + 0] = NoteName[note % 12][0];
    IO1[rRegSIDStrStart + VoiceNum * 4 + 1] = NoteName[note % 12][1];
    IO1[rRegSIDStrStart + VoiceNum * 4 + 2] = '0' + note / 12;

    #ifdef DbgMsgs_M2S
    Serial.print ("MIDI Note On, ch=");
    Serial.print (channel);
    Serial.print (", voice=");
    Serial.print (VoiceNum);
    Serial.print (", note=");
    Serial.print (note);
    Serial.print (", velocity=");
    Serial.print (velocity);
    Serial.print (", reg ");
    Serial.print (IO1[rRegSIDFreqHi1]);
    Serial.print (":");
    Serial.print (IO1[rRegSIDFreqLo1]);
    Serial.println();
    #endif
}

void M2SOnNoteOff (uint8_t channel, uint8_t note, uint8_t velocity)
{
    note += 3; // offset to A centered from C
    IO1[rRegSIDOutOfVoices] = ' ';
    int VoiceNum = FindVoiceUsingNote (note);

    if (VoiceNum < 0)
    {
        #ifdef DbgMsgs_M2S
        Serial.print ("No voice using note ");
        Serial.println (note);
        #endif
        return;
    }
    Voice[VoiceNum].Available = true;
    IO1[rRegSIDVoicCont1 + VoiceNum * 7] &= 0xFE; // stop note
    IO1[rRegSIDStrStart + VoiceNum * 4 + 0] = '-';
    IO1[rRegSIDStrStart + VoiceNum * 4 + 1] = '-';
    IO1[rRegSIDStrStart + VoiceNum * 4 + 2] = ' ';

    #ifdef DbgMsgs_M2S
    Serial.print ("MIDI Note Off, ch=");
    Serial.print (channel);
    Serial.print (", voice=");
    Serial.print (VoiceNum);
    Serial.print (", note=");
    Serial.print (note);
    Serial.print (", velocity=");
    Serial.print (velocity);
    Serial.println();
    #endif
}

void M2SOnControlChange (uint8_t channel, uint8_t control, uint8_t value)
{
    #ifdef DbgMsgs_M2S
    Serial.print ("MIDI Control Change, ch=");
    Serial.print (channel);
    Serial.print (", control=");
    Serial.print (control);
    Serial.print (", value=");
    Serial.print (value);
    Serial.println();
    #endif
}

void M2SOnPitchChange (uint8_t channel, int pitch)
{
    #ifdef DbgMsgs_M2S
    Serial.print ("Pitch Change, ch=");
    Serial.print (channel, DEC);
    Serial.print (", pitch=");
    Serial.println (pitch, DEC);
    Serial.printf ("     0-6= %02x, 7-13=%02x\n", pitch & 0x7f, (pitch >> 7) & 0x7f);
    #endif
}
