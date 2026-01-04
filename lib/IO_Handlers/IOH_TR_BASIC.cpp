#ifndef MinimumBuild
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


#include "IOH_TR_BASIC.h"

#include <SD.h>

#include "IOH_TeensyROM.h"
#include "Common_Defs.h"
#include "Swift_RxQueue.h"
#include "midi2sid.h"

// Global variable definitions
stcIOHandlers IOHndlr_TR_BASIC =
{
    "TR_BASIC",              //Name of handler
    &InitHndlr_TR_BASIC,     //Called once at handler startup
    &IO1Hndlr_TR_BASIC,      //IO1 R/W handler
    NULL,                     //IO2 R/W handler
    NULL,                     //ROML Read handler, in addition to any ROM data sent
    NULL,                     //ROMH Read handler, in addition to any ROM data sent
    &PollingHndlr_TR_BASIC,  //Polled in main routine
    NULL,                     //called at the end of EVERY c64 cycle
};

uint8_t *TgetQueue = NULL;
uint8_t *LSFileName = NULL;
uint16_t FNCount;
uint8_t  TR_BASContRegAction, TR_BASStatRegVal, TR_BASStrAvailableRegVal;



FS *FSfromFileName (char** ptrptrLSFileName)
{
    //returns file system type (default USB) and changes pointer to skip USB:/SD:

    FS *sourceFS = &firstPartition; //default to USB
    char *ptrLSFileName = *ptrptrLSFileName;
    char LSFileNameTmp[4];

    //copy first 4 chars as lower case:
    for (uint8_t num = 0; num < 4; num++) LSFileNameTmp[num] = tolower (ptrLSFileName[num]);

    if (memcmp (LSFileNameTmp, "sd:", 3) == 0)
    {
        ptrLSFileName += 3;
        sourceFS = &SD;
        Printf_dbg ("SD:*\n");
        if (!SD.begin (BUILTIN_SDCARD)) // refresh, takes 3 seconds for fail/unpopulated, 20-200mS populated
        {
            Printf_dbg ("SD Init Fail\n");
            return NULL;   //return BAS_ERROR_DEVICE_NOT_PRESENT;
        }
    }
    else
        if (memcmp (LSFileNameTmp, "usb:", 4) == 0)
        {
            ptrLSFileName += 4;
            //sourceFS = &firstPartition; //already default
            Printf_dbg ("USB:*\n");
        }
        //else if(memcmp(ptrLSFileName, "TR:", 3) == 0)
        //{
        //   MenuSourceID = rmtTeensy;
        //   ptrLSFileName += 3;
        //}
        else
        {
            Printf_dbg ("SD:/USB: not found, default USB\n"); //default to USB if not specified
        }

    *ptrptrLSFileName = ptrLSFileName; //update the pointer
    return sourceFS;
}


void AddToRAM_Image (const char *ToAdd)
{
    //and convert to petscii
    uint32_t count = 0;

    while (1)
    {
        RAM_Image[XferSize] = ToPETSCII (ToAdd[count]);
        if (ToAdd[count] == 0) return;
        XferSize++;
        count++;
    }
}


uint8_t ContRegAction_LoadPrep()
{
    //load file into RAM, returns TR_BASStatRegVal
    //check that file exists & load into RAM_Image

    char *ptrLSFileName = (char*)LSFileName; //local pointer
    FS *sourceFS = FSfromFileName (&ptrLSFileName);

    if (sourceFS == NULL) return BAS_ERROR_DEVICE_NOT_PRESENT;

    Printf_dbg ("Load: %s\n", ptrLSFileName);
    File myFile = sourceFS->open (ptrLSFileName, FILE_READ);

    if (!myFile)
    {
        Printf_dbg ("File Not Found\n");
        return BAS_ERROR_FILE_NOT_FOUND;
    }

    if (myFile.isDirectory())
    {
        Printf_dbg ("File is Dir\n");
        myFile.close();
        return BAS_ERROR_NOT_INPUT_FILE;
    }

    XferSize = myFile.size();
    Printf_dbg ("Size: %d bytes\n", XferSize);
    if (XferSize > 0xc000) //Max to fit below custom commands at $c000
    {
        Printf_dbg ("File too large\n");
        myFile.close();
        return BAS_ERROR_OUT_OF_MEMORY;
    }

    uint32_t count = 0;
    while (myFile.available() && count < XferSize) RAM_Image[count++] = myFile.read();

    myFile.close();
    if (count != XferSize)
    {
        Printf_dbg ("Size Mismatch\n");
        return BAS_ERROR_FILE_DATA;
    }

    uint16_t PStart = (RAM_Image[1] << 8) | RAM_Image[0];
    Printf_dbg ("Addr: $%04x to $%04x\n", PStart, PStart + XferSize);
    if (PStart < MainMemLocEnd && PStart + XferSize >= MainMemLoc)
    {
        Printf_dbg ("Conflict with ext code\n");
        return BAS_ERROR_OUT_OF_MEMORY;  //make this a custom message?
    }
    //if (RAM_Image[0] != 0x01 || RAM_Image[1] != 0x08)  //BAS_ERROR_TYPE_MISMATCH;

    TR_BASStrAvailableRegVal = 0xff;    // transfer available flag
    Printf_dbg ("Done\n");
    return TR_BASStat_Ready;   //TR RAM Load Sussceful, ready to x-fer to C64
}


uint8_t ContRegAction_SaveFinish()
{
    //file was transfered to RAM_Image[], size=StreamOffsetAddr
    //save file from RAM, returns TR_BASStatRegVal

    char *ptrLSFileName = (char*)LSFileName; //local pointer
    FS *sourceFS = FSfromFileName (&ptrLSFileName);

    if (sourceFS == NULL) return BAS_ERROR_DEVICE_NOT_PRESENT;

    Printf_dbg ("Save: %s\nSize: %d bytes\n", ptrLSFileName, StreamOffsetAddr);
    sourceFS->remove (ptrLSFileName); //del prev version to overwrite!
    File myFile = sourceFS->open (ptrLSFileName, FILE_WRITE); //O_RDWR | O_CREAT <- doesn't reduce filesize if smaller

    if (!myFile)
    {
        Printf_dbg ("Failed to open\n");
        return BAS_ERROR_FILE_NOT_OPEN;
    }

    uint32_t BytesWritten = myFile.write (RAM_Image, StreamOffsetAddr);
    myFile.close();

    if (BytesWritten != StreamOffsetAddr)
    {
        Printf_dbg ("Not Fully Written\n");
        return BAS_ERROR_OUT_OF_DATA;
    }

    return TR_BASStat_Ready;   //Save Sussceful
}


uint8_t ContRegAction_DirPrep()
{
    //load dir into RAM, returns TR_BASStatRegVal
    //check that dir exists & load into RAM_Image

    char *ptrLSFileName = (char*)LSFileName; //local pointer
    FS *sourceFS = FSfromFileName (&ptrLSFileName);
    if (sourceFS == NULL) return BAS_ERROR_DEVICE_NOT_PRESENT;

    if (ptrLSFileName[0] == 0) sprintf (ptrLSFileName, "/"); // default to root if zero len
    Printf_dbg ("Dir: \"%s\"\n", ptrLSFileName);

    File dir = sourceFS->open (ptrLSFileName);

    if (!dir)
    {
        Printf_dbg ("Dir Not Found\n");
        return BAS_ERROR_FILE_NOT_FOUND;
    }

    const char *filename;
    XferSize = 0; //initialize RAM_Image size/count
    AddToRAM_Image ("Contents of: \"");
    AddToRAM_Image (ptrLSFileName);
    AddToRAM_Image ("\"\r");

    while (File entry = dir.openNextFile())
    {
        filename = entry.name();

        if (entry.isDirectory()) AddToRAM_Image (" /");
        else AddToRAM_Image ("  ");

        AddToRAM_Image (filename);
        AddToRAM_Image ("\r");

        Printf_dbg ("%s\n", filename);

        entry.close();
        if (XferSize >= RAM_ImageSize() - 80)
        {
            AddToRAM_Image ("*** Too many files!\r");
            break;
        }
    }

    dir.close();
    TR_BASStrAvailableRegVal = 0xff;    // transfer available flag   Need this???????????
    Printf_dbg ("Done\n");
    return TR_BASStat_Ready;   //Save Sussceful
}

uint8_t ContRegAction_DmaTest()
{
    // Assert DMA for 100mS and release
    Printf_dbg ("DMA Trig\n");
    delay (100); //100
    DMA_State = DMA_S_StartDisable;
    return TR_BASStat_Ready;
}

//__________________________________________________________________________________

void InitHndlr_TR_BASIC()
{
    if (TgetQueue == NULL) TgetQueue = (uint8_t * )malloc (TgetQueueSize);
    if (LSFileName == NULL) LSFileName = (uint8_t * )malloc (MaxPathLength);

    RxQueueHead = RxQueueTail = 0; //as used in Swiftlink & ASID

    TR_BASStatRegVal = TR_BASStat_Ready;
    TR_BASContRegAction = TR_BASCont_None; //default to no action
}

void IO1Hndlr_TR_BASIC (uint8_t Address, bool R_Wn)
{
    uint8_t Data;
    if (R_Wn) //High (IO1 Read)
    {
        switch (Address)
        {
            case TR_BASDataReg:
                if (TgetQueueUsed)
                {
                    DataPortWriteWaitLog (TgetQueue[RxQueueTail++]);
                    if (RxQueueTail == TgetQueueSize) RxQueueTail = 0;
                }
                else  //no data to send, send 0
                {
                    DataPortWriteWaitLog (0);
                }
                break;
            case TR_BASStatReg:
                DataPortWriteWaitLog (TR_BASStatRegVal);
                break;
            case TR_BASStreamDataReg:
                DataPortWriteWait (RAM_Image[StreamOffsetAddr]);
                //inc on read, check for end:
                if (++StreamOffsetAddr >= XferSize) TR_BASStrAvailableRegVal = 0; //signal end of transfer
                break;
            case TR_BASStrAvailableReg:
                DataPortWriteWait (TR_BASStrAvailableRegVal);
            default: //used for all other IO1 reads
                //DataPortWriteWaitLog(0);
                break;
        }
    }
    else  // IO1 write
    {
        Data = DataPortWaitRead();
        TraceLogAddValidData (Data);
        switch (Address)
        {
            case TR_BASDataReg:
                Serial.write (Data); //a bit risky doing this here, but seems fast enough in testing
                break;

            case TR_BASContReg:
                //Control reg actions:
                switch (Data)
                {
                    case TR_BASCont_SendFN: //file name being sent next
                        FNCount = 0;
                        StreamOffsetAddr = 0; //initialize for file load/save
                        break;

                    //these commandd require action outside of interrupt:
                    case TR_BASCont_DmaTest:    // Assert DMA for 100mS
                        DMA_State = DMA_S_StartActive; //must start this write cycle
                    // break keyword is not present, all the cases after the matching case are executed
                    case TR_BASCont_LoadPrep:   //load file into RAM
                    case TR_BASCont_SaveFinish: //save file from RAM
                    case TR_BASCont_DirPrep:    // Load Dir into RAM
                        TR_BASContRegAction = Data; //pass it to process outside of interrupt
                        TR_BASStatRegVal = TR_BASStat_Processing; //initialize status
                        break;
                }
                break;

            case TR_BASFileNameReg: //receive file name characters
                //// PETSCII To Lcase ASSCII:
                //Data &= 0x7f; //bit 7 is Cap in Graphics mode
                //if (Data & 0x40) Data |= 0x20;  //conv to lower case

                // PETSCII To ASSCII:
                if (Data & 0x80) Data &= 0x7f; //bit 7 is Cap in Graphics mode
                else
                    if (Data & 0x40) Data |= 0x20;  //conv to lower case

                LSFileName[FNCount++] = Data;
                if (Data == 0)
                {
                    Printf_dbg ("Received FN: \"%s\"\n", LSFileName);
                }
                break;
            case TR_BASStreamDataReg: //receive save data
                RAM_Image[StreamOffsetAddr++] = Data;
                break;
        }
    } //write
}

void PollingHndlr_TR_BASIC()
{
    if (TR_BASContRegAction != TR_BASCont_None)
    {
        switch (TR_BASContRegAction)
        {
            case TR_BASCont_LoadPrep:
                TR_BASStatRegVal = ContRegAction_LoadPrep();
                break;
            case TR_BASCont_SaveFinish:
                TR_BASStatRegVal = ContRegAction_SaveFinish();
                break;
            case TR_BASCont_DirPrep:
                TR_BASStatRegVal = ContRegAction_DirPrep();
                break;
            case TR_BASCont_DmaTest:
                TR_BASStatRegVal = ContRegAction_DmaTest();
                break;
            default:
                Printf_dbg ("Unexpected TR_BASContRegAction: %d\n", TR_BASContRegAction);
        }
        TR_BASContRegAction = TR_BASCont_None;
    }

    while (Serial.available())
    {
        uint8_t Cin = Serial.read();
        if (TgetQueueUsed >= TgetQueueSize - 1)
        {
            Printf_dbg ("\n**Queue Full!\n");
            //loose the char and return
        }
        else
        {
            //add to queue:
            Printf_dbg ("#%d= %d (%c)\n", TgetQueueUsed, Cin, Cin);
            TgetQueue[RxQueueHead++] = Cin;
            if (RxQueueHead == TgetQueueSize) RxQueueHead = 0;
        }

    }
}


#endif
