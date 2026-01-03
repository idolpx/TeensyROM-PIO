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


//IO Handler for TR_BASIC

#ifndef IOH_TR_BASIC_H
#define IOH_TR_BASIC_H

#include <stdint.h>
#include <stdbool.h>
#include <FS.h>

#include "IOH_defs.h"

void IO1Hndlr_TR_BASIC (uint8_t Address, bool R_Wn);
void PollingHndlr_TR_BASIC();
void InitHndlr_TR_BASIC();

extern stcIOHandlers IOHndlr_TR_BASIC;

#define TgetQueueSize      4096
#define TgetQueueUsed      ((RxQueueHead>=RxQueueTail)?(RxQueueHead-RxQueueTail):(RxQueueHead+TgetQueueSize-RxQueueTail))
#define MainMemLoc         0xc000  //see MainMemLoc in assy code, start of BASIC extension code
#define MainMemLocEnd      0xd000  //see MainMemLocEnd in compiled symbols ($caf5 as of 4/20/25, adding buffer)

extern uint8_t *TgetQueue;  //to hold incoming messages
extern uint8_t *LSFileName;

extern uint16_t FNCount;
extern uint8_t  TR_BASContRegAction, TR_BASStatRegVal, TR_BASStrAvailableRegVal;


enum TR_BASregsMatching  //synch with TRCustomBasicCommands\source\main.asm
{
    // registers:
    TR_BASDataReg         = 0xb2,   // (R/W) for TPUT/TGET data
    TR_BASContReg         = 0xb4,   // (Write only) Control Reg
    TR_BASStatReg         = 0xb6,   // (Read only) Status Reg
    TR_BASFileNameReg     = 0xb8,   // (Write only) File name transfer
    TR_BASStreamDataReg   = 0xba,   // (R/W Only) File transfer stream data
    TR_BASStrAvailableReg = 0xbc,   // (Read Only) Signals stream data available

    // Control Reg Commands:
    TR_BASCont_None       = 0x00,   // No Action to be taken
    TR_BASCont_SendFN     = 0x02,   // Prep to send Filename from BAS to TR
    TR_BASCont_LoadPrep   = 0x04,   // Prep to load file from TR RAM
    TR_BASCont_SaveFinish = 0x06,   // Save file from TR RAM to SD/USB
    TR_BASCont_DirPrep    = 0x08,   // Load Dir into TR RAM
    TR_BASCont_DmaTest    = 0x0a,   // Assert DMA for 100mS

    // StatReg Values:
    TR_BASStat_Processing = 0x00,   // No update, still processing
    //Do not conflict with BASIC_Error_Codes (basic.ERROR_*)
    TR_BASStat_Ready      = 0x55,   // Ready to Transfer

}; //end enum synch

enum BASIC_Error_Codes
{
    BAS_ERROR_TOO_MANY_FILES         = 0x01,
    BAS_ERROR_FILE_OPEN              = 0x02,
    BAS_ERROR_FILE_NOT_OPEN          = 0x03,
    BAS_ERROR_FILE_NOT_FOUND         = 0x04,
    BAS_ERROR_DEVICE_NOT_PRESENT     = 0x05,
    BAS_ERROR_NOT_INPUT_FILE         = 0x06,
    BAS_ERROR_NOT_OUTPUT_FILE        = 0x07,
    BAS_ERROR_MISSING_FILENAME       = 0x08,
    BAS_ERROR_ILLEGAL_DEVICE_NUM     = 0x09,
    BAS_ERROR_NEXT_WITHOUT_FOR       = 0x0a,
    BAS_ERROR_SYNTAX                 = 0x0b,
    BAS_ERROR_RETURN_WITHOUT_GOSUB   = 0x0c,
    BAS_ERROR_OUT_OF_DATA            = 0x0d,
    BAS_ERROR_ILLEGAL_QUANTITY       = 0x0e,
    BAS_ERROR_OVERFLOW               = 0x0f,
    BAS_ERROR_OUT_OF_MEMORY          = 0x10,
    BAS_ERROR_UNDEFD_STATEMENT       = 0x11,
    BAS_ERROR_BAD_SUBSCRIPT          = 0x12,
    BAS_ERROR_REDIMD_ARRAY           = 0x13,
    BAS_ERROR_DIVISION_BY_ZERO       = 0x14,
    BAS_ERROR_ILLEGAL_DIRECT         = 0x15,
    BAS_ERROR_TYPE_MISMATCH          = 0x16,
    BAS_ERROR_STRING_TOO_LONG        = 0x17,
    BAS_ERROR_FILE_DATA              = 0x18,
    BAS_ERROR_FORMULA_TOO_COMPLEX    = 0x19,
    BAS_ERROR_CANT_CONTINUE          = 0x1a,
    BAS_ERROR_UNDEFD_FUNCTION        = 0x1b,
    BAS_ERROR_VERIFY                 = 0x1c,
    BAS_ERROR_LOAD                   = 0x1d,
}; //end BASIC_Error_Codes


//__________________________________________________________________________________


FS *FSfromFileName (char** ptrptrLSFileName);
void AddToRAM_Image (const char *ToAdd);
uint8_t ContRegAction_LoadPrep();
uint8_t ContRegAction_SaveFinish();
uint8_t ContRegAction_DirPrep();
uint8_t ContRegAction_DmaTest();

#endif // IOH_TR_BASIC_H