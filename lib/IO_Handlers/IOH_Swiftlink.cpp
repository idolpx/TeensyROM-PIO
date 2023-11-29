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


//Network, 6551 ACIA interface emulation
//with AT Command sub-system and Internet Browser

#include "IOH_Swiftlink.h"

#include <Arduino.h>
#include <EEPROM.h>

#include "../../include/TeensyROM.h"

#include "ethernet_dev.h"

#include "DriveDirLoad.h"
#include "SerUSBIO.h"

#include "Swift_ATcommands.h"
#include "Swift_Browser.h"
#include "Swift_RxQueue.h"

void FreeSwiftlinkBuffs()
{
   for(uint8_t cnt=0; cnt<NumPageLinkBuffs; cnt++) {free(PageLinkBuff[cnt]); PageLinkBuff[cnt]=NULL;}
   for(uint8_t cnt=0; cnt<NumPrevURLQueues; cnt++) {free(PrevURLQueue[cnt]); PrevURLQueue[cnt]=NULL;}
   for(uint8_t cnt=0; cnt<RxQueueNumBlocks; cnt++) {free(RxQueue[cnt]); RxQueue[cnt]=NULL;}
   free(TxMsg); TxMsg = NULL;   
}


//_____________________________________Handlers_____________________________________________________

FLASHMEM void InitHndlr_SwiftLink()
{
   EthernetInit();
   SwiftRegStatus = SwiftStatusTxEmpty; //default reset state
   SwiftRegCommand = SwiftCmndDefault;
   SwiftRegControl = 0;
   CycleCountdown=0;
   PlusCount=0;
   PrevURLQueueNum = 0;
   NMIassertMicros = 0;
   PlusCount=0;
   ConnectedToHost = false;
   BrowserMode = false;
   UnPausePage(); // UsedPageLinkBuffs = 0; PageCharsReceived = 0; PagePaused = false;   
   PrintingHyperlink = false;
   
   FreeDriveDirMenu(); //clear out drive menu to make space in RAM2
   // RAM2 usage as of 11/7/23:
   //    Queues/link buffs (below): 320k+128+29k+5.5k= ~355k total
   //    RAM2 free w/ ethernet loaded & drive menu cleared: 392k (though will show less if fragmented)
   Printf_dbg("RAM2 Bytes Free: %lu (%luK)\n\n", RAM2BytesFree(), RAM2BytesFree()/1024);

   RxQueueHead = RxQueueTail = TxMsgOffset =0;
   
   for(uint8_t cnt=0; cnt<RxQueueNumBlocks; cnt++) 
   {
      RxQueue[cnt] = (uint8_t*)malloc(RxQueueBlockSize);
      if(RxQueue[cnt] == NULL) Serial.println("OOM RxQ");
   }
   TxMsg = (char*)malloc(TxMsgMaxSize);
   if(TxMsg == NULL) Serial.println("OOM TxQ");
   
   for(uint8_t cnt=0; cnt<NumPageLinkBuffs; cnt++)
   {
      PageLinkBuff[cnt] = (char*)malloc(MaxTagSize);
      if(PageLinkBuff[cnt] == NULL) Serial.println("OOM PageLinkBuff");
   }
   for(uint8_t cnt=0; cnt<NumPrevURLQueues; cnt++) 
   {
      PrevURLQueue[cnt] = (stcURLParse*)malloc(sizeof(stcURLParse));
      if(PrevURLQueue[cnt] == NULL) Serial.println("OOM PrevURLQueue");
      PrevURLQueue[cnt]->path[0] = 0;
      PrevURLQueue[cnt]->postpath[0] = 0;
      PrevURLQueue[cnt]->host[0] = 0;
      PrevURLQueue[cnt]->port = 80;
   }
   strcpy(CurrPageTitle, "None");
   randomSeed(ARM_DWT_CYCCNT);
}

void IO1Hndlr_SwiftLink(uint8_t Address, bool R_Wn)
{
   uint8_t Data;
   
   if (R_Wn) //IO1 Read  -------------------------------------------------
   {
      switch(Address)
      {
         case IORegSwiftData:   
            DataPortWriteWaitLog(SwiftRxBuf);
            CycleCountdown = C64CycBetweenRx;
            SetNMIDeassert;
            SwiftRegStatus &= ~(SwiftStatusRxFull | SwiftStatusIRQ); //no longer full, ready to receive more
            break;
         case IORegSwiftStatus:  
            DataPortWriteWaitLog(SwiftRegStatus);
            break;
         case IORegSwiftCommand:  
            DataPortWriteWaitLog(SwiftRegCommand);
            break;
         case IORegSwiftControl:
            DataPortWriteWaitLog(SwiftRegControl);
            break;
      }
   }
   else  // IO1 write    -------------------------------------------------
   {
      Data = DataPortWaitRead();
      switch(Address)
      {
         case IORegSwiftData:  
            //add to input buffer
            SwiftTxBuf=Data;
            SwiftRegStatus &= ~SwiftStatusTxEmpty; //Flag full until Tx processed
            break;
         case IORegSwiftStatus:  
            //Write to status reg is a programmed reset
            SwiftRegCommand = SwiftCmndDefault;
            break;
         case IORegSwiftControl:
            SwiftRegControl = Data;
            //Could confirm setting 8N1 & acceptable baud?
            break;
         case IORegSwiftCommand:  
            SwiftRegCommand = Data;
            //check for Tx/Rx IRQs enabled?
            //handshake line updates?
            break;
      }
      TraceLogAddValidData(Data);
   }
}

void PollingHndlr_SwiftLink()
{
   //detect connection change
   if (ConnectedToHost != client.connected())
   {
      ConnectedToHost = client.connected();
      if (BrowserMode)
      {
         if (!ConnectedToHost) AddRawStrToRxQueue("<eoftag>");  //add special tag to catch when complete
      }
      else
      {
         AddToPETSCIIStrToRxQueue("\r\r\r*** ");
         if (ConnectedToHost) AddToPETSCIIStrToRxQueueLN("connected to host");
         else AddToPETSCIIStrToRxQueueLN("not connected");
      }
   }
   
   //if client data available, add to Rx Queue
   #ifdef DbgMsgs_IO
      if(client.available())
      {
         uint16_t Cnt = 0;
         //Serial.printf("RxIn %d+", RxQueueUsed);
         while (client.available())
         {
            AddRawCharToRxQueue(client.read());
            Cnt++;
         }
         //Serial.printf("%d=%d\n", Cnt, RxQueueUsed);
         if (RxQueueUsed>3000) Serial.printf("Lrg RxQueue add: %d  total: %d\n", Cnt, RxQueueUsed);
      }
   #else
      while (client.available()) AddRawCharToRxQueue(client.read());
   #endif
   
   //if Tx data available, get it from C64
   if ((SwiftRegStatus & SwiftStatusTxEmpty) == 0) 
   {
      if (client.connected() && !BrowserMode) //send Tx data to host
      {
         //Printf_dbg("send %02x: %c\n", SwiftTxBuf, SwiftTxBuf);
         client.print((char)SwiftTxBuf);  //send it
         if(SwiftTxBuf=='+')
         {
            if(millis()-LastTxMillis>1000 || PlusCount!=0) //Must be preceded by at least 1 second of no characters
            {   
               if(++PlusCount>3) PlusCount=0;
            }
         }
         else PlusCount=0;
         
         SwiftRegStatus |= SwiftStatusTxEmpty; //Ready for more
      }
      else  //off-line/at commands or BrowserMode..................................
      {         
         Printf_dbg("echo %02x: %c -> ", SwiftTxBuf, SwiftTxBuf);
         
         if(BrowserMode)
         {
            SendPETSCIICharImmediate(SwiftTxBuf); //echo it now, buffer may be paused or filling
            if (SwiftTxBuf & 0x80) SwiftTxBuf &= 0x7f; //Cap to ascii
            else if (SwiftTxBuf & 0x40) SwiftTxBuf |= 0x20;  //lower case to ascii
         }
         else 
         {
            AddRawCharToRxQueue(SwiftTxBuf); //echo it at end of buffer
            SwiftTxBuf &= 0x7f; //bit 7 is Cap in Graphics mode
            if (SwiftTxBuf & 0x40) SwiftTxBuf |= 0x20;  //conv to lower case
         }
         Printf_dbg("%02x: %c\n", SwiftTxBuf);
         
         if (TxMsgOffset && (SwiftTxBuf==0x08 || SwiftTxBuf==0x14)) TxMsgOffset--; //Backspace in ascii  or  Delete in PETSCII
         else TxMsg[TxMsgOffset++] = SwiftTxBuf; //otherwise store it
         
         if (SwiftTxBuf == 13 || TxMsgOffset == TxMsgMaxSize) //return hit or max size
         {
            SwiftRegStatus |= SwiftStatusTxEmpty; //clear the flag after last SwiftTxBuf access
            TxMsg[TxMsgOffset-1] = 0; //terminate it
            Printf_dbg("TxMsg: %s\n", TxMsg);
            if(BrowserMode) 
            {
               ProcessBrowserCommand();
               SendPETSCIICharImmediate(PETSCIIwhite); 
            }
            else
            {
               ProcessATCommand();
               if (!BrowserMode) AddToPETSCIIStrToRxQueueLN("ok\r");
            }
            TxMsgOffset = 0;
         }
         else SwiftRegStatus |= SwiftStatusTxEmpty; //clear the flag after last SwiftTxBuf access
      }
      LastTxMillis = millis();
   }
   
   if(PlusCount==3 && millis()-LastTxMillis>1000) //Must be followed by one second of no characters
   {
      PlusCount=0;
      ClearClientStop();
      AddToPETSCIIStrToRxQueueLN("\r*click*");
   }

   if (PageCharsReceived < 880 || PrintingHyperlink) CheckSendRxQueue();
   else
   {
      if (!PagePaused)
      {
         PagePaused = true;
         SendCommandSummaryImmediate(true);
      }
   }
}

void CycleHndlr_SwiftLink()
{
   if (CycleCountdown) CycleCountdown--;
}


