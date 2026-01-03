// MIT License
//
// Copyright (c) 2024 Travis Smith
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

#ifndef IOH_ASID_H
#define IOH_ASID_H

#include "IOH_defs.h"

//IO Handler for MIDI ASID SysEx streams

extern IntervalTimer ASIDPlaybackTimer;

void IO1Hndlr_ASID (uint8_t Address, bool R_Wn);
void PollingHndlr_ASID();
void InitHndlr_ASID();

extern stcIOHandlers IOHndlr_ASID;

enum ASIDregsMatching  //synch with ASIDPlayer.asm
{
    // registers:
    ASIDAddrReg         = 0xc2,   // (Read only)  Data type and SID Address Register
    ASIDDataReg         = 0xc4,   // (Read only)  ASID data, increment queue Tail
    ASIDQueueUsed       = 0xc8,   // (Read only)  Current Queue amount used
    ASIDContReg         = 0xca,   // (Write only) Control Reg
    ASIDVoiceMuteReg    = 0xcc,   // (write only) bits 0-2 set voice muting

    // Control Reg Commands
    // Timer controls match TblMsgTimerState, start at 0
    ASIDContTimerOff    = 0x00,   //disable Frame Timer
    ASIDContTimerOnAuto = 0x01,   //enable Frame Timer, auto seed time
    ASIDContTimerOn50Hz = 0x02,   //enable Frame Timer, 50Hz seed time
    NumTimerStates      = 0x03,   //Always last, number of states
    // ...
    ASIDContIRQOn       = 0x10,   //enable ASID IRQ
    ASIDContIRQOff      = 0x11,   //disable ASID IRQ
    ASIDContExit        = 0x12,   //Disable IRQ, Send TR to main menu
    ASIDContDisWriteOrd = 0x13,   //Disable Forced Reg Write Order
    // ...
    ASIDContBufTiny     = 0x20,   //Set buffer to size Tiny
    ASIDContBufSmall    = 0x21,   //Set buffer to size Small
    ASIDContBufMedium   = 0x22,   //Set buffer to size Medium
    ASIDContBufLarge    = 0x23,   //Set buffer to size Large
    ASIDContBufXLarge   = 0x24,   //Set buffer to size XLarge
    ASIDContBufXXLarge  = 0x25,   //Set buffer to size XXLarge
    ASIDContBufFirstItem = ASIDContBufTiny,   //First seq item on list
    ASIDContBufLastItem = ASIDContBufXXLarge, //Last seq item on list
    ASIDContBufMask     = 0x07,   //Mask to get zero based item #

    // queue message types/masks
    ASIDAddrType_Skip   = 0x00,   // No data/skip, also indicates End Of Frame
    ASIDAddrType_Char   = 0x20,   // Character data
    ASIDAddrType_Start  = 0x40,   // ASID Start message
    ASIDAddrType_Stop   = 0x60,   // ASID Stop message
    ASIDAddrType_SID1   = 0x80,   // Lower 5 bits are SID1 reg address
    ASIDAddrType_SID2   = 0xa0,   // Lower 5 bits are SID2 reg address
    ASIDAddrType_SID3   = 0xc0,   // Lower 5 bits are SID3 reg address
    ASIDAddrType_Error  = 0xe0,   // Error from parser

    ASIDAddrType_Mask   = 0xe0,   // Mask for Type
    ASIDAddrAddr_Mask   = 0x1f,   // Mask for Address
}; //end enum synch

#define RegValToBuffSize(X)  (1<<((X & ASIDContBufMask)+8)); // 256, 512, 1024, 2048, 4096, 8192; make sure MIDIRxBufSize is >= max (8192)         
#define ASIDRxQueueUsed      ((RxQueueHead>=RxQueueTail)?(RxQueueHead-RxQueueTail):(RxQueueHead+ASIDQueueSize-RxQueueTail))
#define FramesBetweenChecks  12    //frames between frame alignments check & timing adjust
#define SIDFreq50HzuS        19975 // (PAL) 19950=50.125Hz (SFII, real C64 HW),  20000=50.0Hz (DeepSID)
//Splitting the difference for now, todo: standardize when DS updated!
#define SIDFreq60HzuS        16715 // (NTSC) 16715=59.827Hz (real C64 HW)

//ASID protocol packet types
#define APT_StartPlaying     0x4c
#define APT_StopPlayback     0x4d
#define APT_DisplayChars     0x4f
#define APT_SID1RegData      0x4e
#define APT_SID2RegData      0x50
#define APT_SID3RegData      0x51
#define APT_WriteOrder       0x30
#define APT_ContFramerate    0x31
#define APT_SIDTypes         0x32

#ifdef DbgMsgs_IO  //Debug msgs mode
#define Printf_dbg_SysExInfo {Serial.printf("\nSysEx: size=%d, data=", size); for(uint16_t Cnt=0; Cnt<size; Cnt++) Serial.printf(" $%02x", data[Cnt]);Serial.println();}
#else //Normal mode
#define Printf_dbg_SysExInfo {}
#endif

#ifdef DbgSignalASIDIRQ
bool DbgInputState;  //togles LED on SysEx arrival
bool DbgOutputState; //togles debug signal on IRQ assert
#endif

#ifdef Dbg_SerASID
int32_t MaxB, MinB;
uint32_t MaxT, MinT;
uint32_t BufByteTarget;
#endif

extern bool QueueInitialized, FrameTimerMode;
extern int32_t DeltaFrames;
extern uint32_t ASIDQueueSize, NumPackets, TotalInituS, ForceIntervalUs, TimerIntervalUs;
extern uint8_t MutedVoiceFlags;

#define MaxNumRegisters   28 //Max registers in an ASID Sysex Packet
extern int8_t RegisterOrder[MaxNumRegisters];  //index is reg order, value is sysex reg #
extern bool ForcedRegOrder;

extern uint8_t ASIDidToReg[];

void InitTimedASIDQueue();
void AddToASIDRxQueue (uint8_t Addr, uint8_t Data);
void FlushASIDRxQueue();
void SetASIDIRQ();
void PrintflnToASID (const char *Fmt, ...);
void AddErrorToASIDRxQueue();
void DecodeSendSIDRegData (uint8_t SID_ID, uint8_t *data, unsigned int size);
FASTRUN void SendTimedASID();
//MIDI input handlers for HW Emulation _________________________________________________________________________

// F0 SysEx single call, message larger than buffer is truncated
void ASIDOnSystemExclusive (uint8_t *data, unsigned int size);

#endif // IOH_ASID_H