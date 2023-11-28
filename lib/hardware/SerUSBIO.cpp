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

#include "SerUSBIO.h"

#include <Arduino.h>
#include <SD.h>

//#include "main.h"
#include "IOHandlers.h"
#include "../../include/TeensyROM.h"
#include "FlashTxx.h"
#include "MainMenuItems.h"

// synch with win app:
#define SendFileToken 0x64AA
#define AckToken 0x64CC
#define FailToken 0x9B7F

FLASHMEM void ServiceSerial()
{
    uint8_t inByte = Serial.read();
    switch (inByte)
    {
    case 0x64: //'d' command from app
         if(!SerialAvailabeTimeout()) return;
        inByte = Serial.read(); // READ NEXT BYTE
         //only commands available when busy:
         if (inByte == 0xEE) //Reset C64
        {
            Serial.println("Reset cmd received");
            BtnPressed = true;
            return;
        }
         else if (inByte == 0x44) //Launch File
         {
            LaunchFile();
            return;
         }
         
        if (CurrentIOHandler != IOH_TeensyROM)
        {
            SendU16(FailToken);
            Serial.print("Busy!\n");
            return;
        }
         //TeensyROM IO Handler is active...

        switch (inByte)
        {
        case 0x55: // ping
            Serial.printf("TeensyROM %s ready!\n", strVersionNumber);
            break;
        case 0xAA: // file x-fer pc->TR
            case 0xBB:  // v2 file x-fer pc->TR.  For use with v2 UI.
               PostFileCommand();
            break;
            case 0xDD:  // v2 directory listing from TR
               GetDirectoryCommand();
               break;
            case 0x66: //Pause SID
               if(RemotePauseSID()) SendU16(AckToken);
               else SendU16(FailToken);
               break;
            case 0x67: //'dg'Test/debug
            // for (int a=0; a<256; a++) Serial.printf("\n%3d, // %3d   '%c'", ToPETSCII(a), a, a);
            PrintDebugLog();
            break;
        default:
               Serial.printf("Unk cmd: 0x64%02x\n", inByte); 
            break;
        }
        break;
    case 'e': // Reset EEPROM to defaults
        SetEEPDefaults();
        Serial.println("Applied upon reboot");
        break;

// l, c, i, f, x
#ifdef Dbg_SerLogMem
    case 'l': // Show Debug Log
        PrintDebugLog();
        break;
    case 'c': // Clear Debug Log buffer
        BigBufCount = 0;
        Serial.println("Buffer Reset");
        break;
    case 'i':
    case 'f': // show build info+free mem.  Menu must be idle, interferes with any serialstring in progress
    {
        MakeBuildCPUInfoStr();
        Serial.println("\n***** Build & Mem info *****");
        Serial.println(SerialStringBuf);
        Serial.printf("RAM2 Bytes Free: %lu (%luK)\n\n", RAM2BytesFree(), RAM2BytesFree() / 1024);
        memInfo();
        getFreeITCM();

        Serial.printf("\nMem usage:\n");
        uint32_t TotalSize = 0;
        if (DriveDirMenu != NULL)
        {
               for(uint16_t Num=0; Num < NumDrvDirMenuItems; Num++) TotalSize += strlen(DriveDirMenu[Num].Name)+1;
            Serial.printf("Filenames: %lu (%luk) @ $%08x\nDriveDirMenu: %lu (%luk) @ $%08x\n",
                          TotalSize, TotalSize / 1024, (uint32_t)DriveDirMenu[0].Name,
                          MaxMenuItems * sizeof(StructMenuItem), MaxMenuItems * sizeof(StructMenuItem) / 1024, (uint32_t)DriveDirMenu);
            TotalSize += MaxMenuItems * sizeof(StructMenuItem);
        }
        Serial.printf("DriveDirMenu+Filenames: %lu (%luk)\n",
                      TotalSize, TotalSize / 1024);

        Serial.printf("RAM_Image: %lu (%luk) @ $%08x\n",
                      sizeof(RAM_Image), sizeof(RAM_Image) / 1024, (uint32_t)RAM_Image);

        TotalSize = 0;
        uint32_t TotalStructSize = sizeof(TeensyROMMenu);
        for (uint8_t ROMNum = 0; ROMNum < sizeof(TeensyROMMenu) / sizeof(TeensyROMMenu[0]); ROMNum++)
        {
            if (TeensyROMMenu[ROMNum].ItemType == rtDirectory)
            {
                StructMenuItem *subTROMMenu = (StructMenuItem *)TeensyROMMenu[ROMNum].Code_Image;
                TotalStructSize += TeensyROMMenu[ROMNum].Size;
                for (uint8_t subROMNum = 0; subROMNum < TeensyROMMenu[ROMNum].Size / sizeof(StructMenuItem); subROMNum++)
                {
                     if(subTROMMenu[subROMNum].ItemType != rtDirectory) AddAndCheckSource(subTROMMenu[subROMNum], &TotalSize);
                }
            }
               else AddAndCheckSource(TeensyROMMenu[ROMNum], &TotalSize);
        }
        Serial.printf("TeensyROMMenu/sub struct: %lu (%luk) @ $%08x\n",
                      TotalStructSize, TotalStructSize / 1024, (uint32_t)TeensyROMMenu);
        Serial.printf("TeensyROMMenu/sub Items: %d (%dk) of Flash\n\n", TotalSize, TotalSize / 1024);
    }
    break;
    case 'x':
    {                      // see how many 8k banks will fit in RAM2
        char *ptrChip[70]; // 64 8k blocks would be 512k (size of RAM2)
        uint16_t ChipNum = 0;
        while (1)
        {
            ptrChip[ChipNum] = (char *)malloc(8192);
               if (ptrChip[ChipNum] == NULL) break;
            ChipNum++;
        }
            for(uint16_t Cnt=0; Cnt < ChipNum; Cnt++) free(ptrChip[Cnt]);
        Serial.printf("Created/freed %d  8k blocks (%dk total) in RAM2\n", ChipNum, ChipNum * 8);
    }
    break;
#endif

// m, p, k, r, s
#ifdef Dbg_SerSwift
    case 'm':
    {
        // RxIn 497+551=1048
        // big 8-bit playground buffer for testing
        uint8_t inbuf[] = {
            168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 13, 32, 32,
            18, 28, 220, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 162, 162, 162, 32, 172, 162, 162, 162, 172, 162, 162, 162,
            187, 32, 44, 146, 166, 13, 32, 32, 18, 220, 32, 32, 161, 146, 223, 18, 32, 32, 32, 32, 162, 146, 169, 18, 32, 172, 161, 32, 146, 223, 18, 32,
            146, 161, 18, 152, 190, 153, 162, 30, 188, 28, 161, 146, 161, 18, 32, 150, 162, 129, 188, 146, 32, 18, 154, 32, 156, 32, 5, 32, 28, 161, 206, 223,
            146, 13, 32, 32, 18, 220, 146, 190, 18, 162, 146, 158, 58, 46, 28, 184, 188, 183, 158, 58, 46, 58, 28, 188, 32, 158, 46, 18, 28, 162, 146, 158,
            58, 32, 28, 190, 18, 153, 187, 146, 30, 162, 18, 151, 172, 146, 129, 172, 28, 187, 18, 150, 32, 146, 129, 162, 18, 31, 172, 146, 32, 156, 187, 18,
            5, 32, 146, 156, 172, 18, 28, 190, 180, 146, 219, 165, 13, 32, 32, 18, 156, 220, 163, 163, 163, 163, 163, 163, 163, 163, 163, 163, 163, 163, 163, 163,
            163, 163, 146, 161, 18, 30, 32, 146, 32, 18, 129, 32, 146, 28, 188, 150, 190, 18, 129, 32, 146, 32, 18, 154, 32, 5, 161, 146, 161, 18, 152, 32,
            156, 161, 167, 175, 175, 146, 13, 32, 32, 18, 220, 32, 220, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 46, 32, 32, 32, 188, 146, 151, 188, 18,
            129, 162, 146, 28, 190, 18, 156, 190, 146, 161, 18, 31, 162, 154, 162, 146, 156, 190, 5, 188, 151, 190, 18, 162, 154, 161, 172, 31, 190, 146, 220, 13,
            32, 32, 154, 188, 18, 185, 185, 185, 185, 168, 175, 175, 175, 164, 164, 164, 164, 39, 185, 185, 185, 185, 185, 185, 185, 185, 164, 175, 175, 175, 175, 185,
            185, 185, 146, 156, 184, 31, 168, 59, 13, 32, 32, 151, 167, 18, 31, 32, 32, 32, 32, 163, 163, 220, 163, 183, 183, 183, 183, 184, 32, 32, 32, 32,
            32, 166, 220, 32, 183, 163, 163, 163, 163, 168, 32, 32, 32, 168, 146, 151, 165, 13, 32, 32, 167, 18, 5, 32, 32, 146, 161, 18, 32, 31, 32, 32,
            5, 190, 32, 167, 32, 31, 32, 5, 32, 31, 32, 5, 190, 32, 167, 32, 32, 31, 32, 5, 190, 32, 188, 165, 31, 32, 5, 167, 32, 31, 166, 5,
            167, 32, 32, 188, 146, 151, 165, 13, 32, 32, 167, 18, 5, 32, 31, 163, 5, 167, 32, 31, 32, 32, 5, 32, 31, 163, 5, 167, 32, 31, 32, 5,
            32, 31, 32, 5, 32, 31, 34, 163, 5, 32, 31, 163, 5, 167, 32, 31, 163, 5, 167, 32, 31, 32, 5, 167, 32, 146, 187, 18, 167, 32, 31, 163,
            5, 32, 146, 151, 165, 13, 32, 32, 170, 18, 5, 32, 31, 32, 5, 167, 32, 31, 32, 32, 5, 32, 31, 166, 5, 167, 187, 32, 172, 31, 32, 5,
            32, 31, 33, 32, 5, 32, 31, 32, 5, 167, 32, 31, 32, 5, 167, 32, 31, 58, 5, 167, 32, 32, 167, 32, 31, 166, 5, 32, 146, 151, 165, 13,
            32, 32, 167, 18, 5, 32, 32, 146, 190, 18, 32, 31, 166, 32, 5, 32, 32, 167, 31, 182, 5, 32, 31, 32, 32, 5, 32, 146, 182, 18, 182, 32,
            32, 146, 161, 18, 32, 31, 46, 5, 167, 32, 31, 32, 5, 167, 32, 146, 188, 18, 167, 32, 31, 161, 5, 32, 146, 151, 165, 13, 32, 32, 167, 18,
            5, 32, 146, 31, 166, 18, 166, 5, 32, 146, 31, 58, 18, 166, 5, 32, 146, 31, 33, 18, 5, 32, 146, 31, 221, 18, 5, 32, 31, 187, 166, 5,
            32, 31, 46, 5, 167, 32, 31, 166, 5, 167, 32, 31, 187, 5, 167, 32, 31, 172, 5, 167, 32, 146, 31, 58, 18, 5, 167, 32, 146, 31, 39, 18,
            5, 32, 146, 151, 165, 13, 32, 32, 167, 18, 5, 32, 146, 31, 33, 166, 18, 5, 32, 32, 170, 32, 146, 31, 58, 18, 5, 32, 146, 31, 33, 18,
            5, 32, 146, 32, 18, 31, 182, 5, 32, 32, 167, 32, 146, 32, 18, 167, 187, 32, 172, 146, 188, 18, 32, 146, 190, 18, 32, 146, 151, 164, 18, 5,
            167, 32, 32, 172, 146, 151, 165, 13, 32, 32, 32, 163, 163, 163, 152, 172, 187, 151, 163, 163, 163, 163, 163, 152, 191, 151, 163, 163, 18, 152, 191, 146,
            151, 163, 163, 163, 163, 163, 152, 191, 18, 191, 146, 151, 163, 163, 163, 163, 163, 152, 172, 18, 32, 146, 187, 151, 163, 163, 13, 32, 32, 32, 32, 172,
            18, 162, 162, 152, 162, 162, 146, 187, 32, 32, 151, 172, 18, 172, 32, 152, 32, 187, 146, 187, 32, 32, 18, 151, 190, 187, 162, 152, 162, 172, 188, 146,
            32, 32, 151, 172, 18, 172, 32, 152, 187, 146, 187, 13, 32, 32, 32, 32, 18, 149, 161, 146, 151, 185, 18, 188, 190, 146, 152, 185, 161, 32, 32, 18,
            149, 172, 151, 32, 32, 32, 152, 32, 187, 146, 32, 32, 18, 151, 172, 32, 32, 32, 152, 32, 187, 146, 32, 32, 32, 18, 151, 187, 185, 172, 146, 13,
            32, 32, 32, 32, 32, 18, 149, 191, 146, 32, 32, 151, 191, 32, 32, 32, 32, 18, 149, 188, 146, 32, 32, 18, 151, 190, 146, 32, 32, 32, 149, 190,
            18, 190, 151, 162, 162, 188, 146, 152, 188, 32, 32, 32, 18, 149, 191, 151, 32, 146, 191, 13, 13, 32, 32, 32, 195, 85, 82, 82, 69, 78, 84, 76,
            89, 32, 68, 79, 87, 78, 32, 70, 79, 82, 32, 77, 65, 73, 78, 84, 69, 78, 65, 78, 67, 69, 44, 13, 32, 32, 32, 32, 32, 32, 80, 76,
            69,65,83,69,32,67,65,76,76,32,66,65,67,75,32,76,65,84,69,82,46,13,13,13,13,13,13
            };

            for(uint16_t Cnt=0; Cnt<sizeof(inbuf); Cnt++) AddRawCharToRxQueue(inbuf[Cnt]);
    }
    break;
    case 'p':
        AddToPETSCIIStrToRxQueueLN("0123456789abcdef");
        break;
    case 'k': // kill client connection
        client.stop();
        Serial.printf("Client stopped\n");
        break;
    case 'r':                                // reset status/NMI
        SwiftRegStatus = SwiftStatusTxEmpty; // default reset state
        SwiftRegCommand = SwiftCmndDefault;
        SwiftRegControl = 0;
        RxQueueHead = RxQueueTail = 0;
        // SwiftRegStatus &= ~(SwiftStatusRxFull | SwiftStatusIRQ); //no longer full, ready to receive more
        SetNMIDeassert;
        Serial.printf("Swiftlink Reset\n");
        break;
    case 's': // status
    {
        char stNot[] = " Not";

        Serial.printf("Swiftlink status:\n");
        Serial.printf("  client is");
            if (!client.connected()) Serial.printf(stNot);
        Serial.printf(" connected\n");

        Serial.printf("  Rx Queue Used: %d\n", RxQueueUsed);

        Serial.printf("  RxIRQ is");
            if((SwiftRegCommand & SwiftCmndRxIRQEn) != 0) Serial.printf(stNot); 
        Serial.printf(" enabled\n");

        Serial.printf("  RxIRQ is");
            if((SwiftRegStatus & (SwiftStatusRxFull | SwiftStatusIRQ)) == 0) Serial.printf(stNot); 
        Serial.printf(" set\n");
    }
    break;
#endif

// t...
#ifdef Dbg_SerTimChg
    case 't': // timing commands, 2 letters and 3-4 numbers
        switch (Serial.read())
        {
        case 'm': // nS_MaxAdj change
            GetDigits(4, &nS_MaxAdj);
            break;
        case 'r': // nS_RWnReady change
            GetDigits(3, &nS_RWnReady);
            break;
        case 'p': // nS_PLAprop change
            GetDigits(3, &nS_PLAprop);
            break;
        case 's': // nS_DataSetup change
            GetDigits(3, &nS_DataSetup);
            break;
        case 'h': // nS_DataHold change
            GetDigits(3, &nS_DataHold);
            break;
        case 'v': // VIC timing change
            GetDigits(3, &nS_VICStart);
            break;
        default:
            Serial.printf("No changes\n");
            break;
        }
        Serial.printf("   nS_MaxAdj(tm)  %04d\n", nS_MaxAdj);
        Serial.printf(" nS_RWnReady(tr)  %03d\n", nS_RWnReady);
        Serial.printf("  nS_PLAprop(tp)  %03d\n", nS_PLAprop);
        Serial.printf("nS_DataSetup(ts)  %03d\n", nS_DataSetup);
        Serial.printf(" nS_DataHold(th)  %03d\n", nS_DataHold);
        Serial.printf(" nS_VICStart(tv)  %03d\n", nS_VICStart);
        break;
#endif
   
    }
}

FLASHMEM void AddAndCheckSource(StructMenuItem SourceMenu, uint32_t *TotalSize)
{
    *TotalSize += SourceMenu.Size;
    Printf_dbg(" $%08x %7d %s\n", (uint32_t)SourceMenu.Code_Image, SourceMenu.Size, SourceMenu.Name);
    if (((uint32_t)SourceMenu.Code_Image & 0xF0000000) == 0x20000000)
        Serial.printf("%s is using RAM!!!\n", SourceMenu.Name);
}

FLASHMEM void GetDigits(uint8_t NumDigits, uint32_t *SetInt)
{
    char inStr[NumDigits + 1];

    for (uint8_t DigNum = 0; DigNum < NumDigits; DigNum++)
    {
        if (!Serial.available())
        {
            Serial.println("\nNot enough Digits!\n");
            return;
        }
        inStr[DigNum] = Serial.read();
    }
    inStr[NumDigits] = 0;
    *SetInt = atol(inStr);
    Serial.printf("\nVal Set to: %d\n\n", *SetInt);
}

FLASHMEM void PrintDebugLog()
{
    bool LogDatavalid = false;

#ifdef DbgIOTraceLog
    Serial.println("DbgIOTraceLog enabled");
    LogDatavalid = true;
#endif

#ifdef DbgCycAdjLog
    Serial.println("DbgCycAdjLog enabled");
    LogDatavalid = true;
#endif

#ifdef DbgSpecial
    Serial.println("DbgSpecial enabled");
    LogDatavalid = true;
#endif

    if (CurrentIOHandler == IOH_Debug)
    {
        Serial.println("Debug IO Handler enabled");
        LogDatavalid = true;
    }

    if (!LogDatavalid)
    {
        Serial.println("No logging enabled");
        return;
    }

    bool BufferFull = (BigBufCount == BigBufSize);

   if  (BufferFull) BigBufCount--; //last element invalid

    for (uint16_t Cnt = 0; Cnt < BigBufCount; Cnt++)
    {
        Serial.printf("#%04d ", Cnt);
        if (BigBuf[Cnt] & DbgSpecialData)
        {
            // BigBuf[Cnt] &= ~DbgSpecialData;
            Serial.printf("DbgSpecialData %04x : %02x\n", BigBuf[Cnt] & 0xFFFF, (BigBuf[Cnt] >> 24));
            // code used previously, in-situ:
            // #ifdef DbgSpecial
            //    if (BigBuf != NULL){
            //      BigBuf[BigBufCount] = Address | (HIROM_Image[Address & 0x1FFF]<<24) | DbgSpecialData;
            //      if (BigBufCount < BigBufSize) BigBufCount++;
            //    }
            // #endif
        }
        else if (BigBuf[Cnt] & AdjustedCycleTiming)
        {
            BigBuf[Cnt] &= ~AdjustedCycleTiming;
            Serial.printf("skip %lu ticks = %lu nS, adj = %lu nS\n", BigBuf[Cnt], CycTonS(BigBuf[Cnt]), CycTonS(BigBuf[Cnt]) - nS_MaxAdj);
        }
        else
        {
            Serial.printf("%s 0xde%02x : ", (BigBuf[Cnt] & IOTLRead) ? "Read" : "\t\t\t\tWrite", BigBuf[Cnt] & 0xff);

         if (BigBuf[Cnt] & IOTLDataValid) Serial.printf("%02x\n", (BigBuf[Cnt]>>8) & 0xff); //data is valid
         else Serial.printf("n/a\n");
        }
    }

   if (BigBufCount == 0) Serial.println("Buffer empty");
   if (BufferFull) Serial.println("Buffer was full");
    Serial.println("Buffer Reset");
    BigBufCount = 0;
}

FLASHMEM void LaunchFile()
{
   //   App: LaunchFileToken 0x6444
    // Teensy: AckToken 0x64CC
   //   App: Send SD_nUSB(1), DestPath/Name(up to MaxNamePathLength, null term)
    // Teensy: AckToken 0x64CC

   //Launch file token has been received, only 2 byte responses until after final response
    SendU16(AckToken);

    uint32_t SD_nUSB;
    char FileNamePath[MaxNamePathLength];
   if (ReceiveFileName(&SD_nUSB, FileNamePath))
    {
      RemoteLaunch(SD_nUSB !=0 , FileNamePath);
      SendU16(AckToken);
        }
    }

FLASHMEM bool ReceiveFileName(uint32_t *SD_nUSB, char *FileNamePath)
    {
   if (!GetUInt(SD_nUSB, 1)) return false;

   uint16_t CharNum=0;
   while (1) 
    {
      if(!SerialAvailabeTimeout()) return false;
      FileNamePath[CharNum] = Serial.read();
      if (FileNamePath[CharNum]==0) return true;
      if (++CharNum == MaxNamePathLength)
        {
            SendU16(FailToken);
         Serial.print("Path too long!\n");  
         return false;
        }
    }
}

FLASHMEM bool GetUInt(uint32_t *InVal, uint8_t NumBytes)
{
    *InVal = 0;
    for (int8_t ByteNum = NumBytes - 1; ByteNum >= 0; ByteNum--)
    {
      if(!SerialAvailabeTimeout()) return false;
        uint32_t ByteIn = Serial.read();
        *InVal += (ByteIn << (ByteNum * 8));
    }
    return true;
}

FLASHMEM void SendU16(uint16_t SendVal)
{
    Serial.write((uint8_t)(SendVal & 0xff));
    Serial.write((uint8_t)((SendVal >> 8) & 0xff));
}

FLASHMEM bool SerialAvailabeTimeout()
{
    uint32_t StartTOMillis = millis();

   while(!Serial.available() && (millis() - StartTOMillis) < SerialTimoutMillis); // timeout loop
   if (Serial.available()) return(true);

    SendU16(FailToken);
    Serial.print("Timeout!\n");
    return (false);
}

uint32_t RAM2BytesFree()
{
    extern char _heap_end[];
    extern char *__brkval;
    return (_heap_end - __brkval);
}

// memory info display via:
//  https://forum.pjrc.com/threads/33443-How-to-display-free-ram
//  https://www.pjrc.com/store/teensy41.html#memory

#define printf Serial.printf
#if ARDUINO_TEENSY41
extern "C" uint8_t external_psram_size;
#endif

FLASHMEM void memInfo()
{
    constexpr auto RAM_BASE = 0x2020'0000;
    constexpr auto RAM_SIZE = 512 << 10;
    constexpr auto FLASH_BASE = 0x6000'0000;

#if ARDUINO_TEENSY40
    constexpr auto FLASH_SIZE = 2 << 20;
#elif ARDUINO_TEENSY41
    // constexpr auto FLASH_SIZE = 8 << 20;
#endif

    // note: these values are defined by the linker, they are not valid memory
    // locations in all cases - by defining them as arrays, the C++ compiler
    // will use the address of these definitions - it's a big hack, but there's
    // really no clean way to get at linker-defined symbols from the .ld file

    extern char _stext[], _etext[], _sbss[], _ebss[], _sdata[], _edata[],
        _estack[], _heap_start[], _heap_end[], _itcm_block_count[], *__brkval;

    auto sp = (char *)__builtin_frame_address(0);

    printf("MemInfo:\n");
    printf("_stext        %08x\n", _stext);
    printf("_etext        %08x +%db\n", _etext, _etext - _stext);
    printf("_sdata        %08x\n", _sdata);
    printf("_edata        %08x +%db\n", _edata, _edata - _sdata);
    printf("_sbss         %08x\n", _sbss);
    printf("_ebss         %08x +%db\n", _ebss, _ebss - _sbss);
    printf("curr stack    %08x +%db\n", sp, sp - _ebss);
    printf("_estack       %08x +%db\n", _estack, _estack - sp);
    printf("_heap_start   %08x\n", _heap_start);
    printf("__brkval      %08x +%db\n", __brkval, __brkval - _heap_start);
    printf("_heap_end     %08x +%db\n", _heap_end, _heap_end - __brkval);

#if ARDUINO_TEENSY41
    extern char _extram_start[], _extram_end[], *__brkval;
    printf("_extram_start %08x\n", _extram_start);
    printf("_extram_end   %08x +%db\n", _extram_end,
           _extram_end - _extram_start);
#endif
    printf("\n");

    printf("<ITCM>  %08x .. %08x\n",
           _stext, _stext + ((int)_itcm_block_count << 15) - 1);
    printf("<DTCM>  %08x .. %08x\n",
           _sdata, _estack - 1);
    printf("<RAM>   %08x .. %08x\n",
           RAM_BASE, RAM_BASE + RAM_SIZE - 1);
    printf("<FLASH> %08x .. %08x\n",
           FLASH_BASE, FLASH_BASE + FLASH_SIZE - 1);
#if ARDUINO_TEENSY41
    if (external_psram_size > 0)
        printf("<PSRAM> %08x .. %08x\n",
               _extram_start, _extram_start + (external_psram_size << 20) - 1);
#endif
    printf("\n");

    auto stack = sp - _ebss;
    printf("avail STACK (RAM1) %8d b %5d kb\n", stack, stack >> 10);

    auto heap = _heap_end - __brkval;
    printf("avail HEAP  (RAM2) %8d b %5d kb\n", heap, heap >> 10);

#if ARDUINO_TEENSY41
    auto psram = _extram_start + (external_psram_size << 20) - _extram_end;
    printf("avail PSRAM (ext)  %8d b %5d kb\n", psram, psram >> 10);
#endif
}


uint32_t *ptrFreeITCM;   // Set to Usable ITCM free RAM
uint32_t sizeofFreeITCM; // sizeof free RAM in uint32_t units.
uint32_t SizeLeft_etext;
extern char _stext[], _etext[];

FLASHMEM void  getFreeITCM() { // end of CODE ITCM, skip full 32 bits
    Serial.println("\ngetFreeITCM:");
    SizeLeft_etext = (32 * 1024) - (((uint32_t)&_etext - (uint32_t)&_stext) % (32 * 1024));
    sizeofFreeITCM = SizeLeft_etext - 4;
    sizeofFreeITCM /= sizeof(ptrFreeITCM[0]);
    ptrFreeITCM = (uint32_t *)((uint32_t)&_stext + (uint32_t)&_etext + 4);
    printf("Size of Free ITCM in Bytes = %u\n", sizeofFreeITCM * sizeof(ptrFreeITCM[0]));
    printf("Start of Free ITCM = %u [%X] \n", ptrFreeITCM, ptrFreeITCM);
    printf("End of Free ITCM = %u [%X] \n", ptrFreeITCM + sizeofFreeITCM, ptrFreeITCM + sizeofFreeITCM);
  for ( uint32_t ii = 0; ii < sizeofFreeITCM; ii++) ptrFreeITCM[ii] = 1;
    uint32_t jj = 0;
  for ( uint32_t ii = 0; ii < sizeofFreeITCM; ii++) jj += ptrFreeITCM[ii];
    printf("ITCM DWORD cnt = %u [#bytes=%u] \n", jj, jj * 4);
}


