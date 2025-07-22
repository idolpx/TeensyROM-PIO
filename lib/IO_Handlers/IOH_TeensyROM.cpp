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


//IO Handler for TeensyROM 

#include "IOH_TeensyROM.h"

#include <SD.h>
#include <EEPROM.h>

#include "Common_Defs.h"

#include "DriveDirLoad.h"
#include "SendMsg.h"
#include "MainMenuItems.h"
#include "midi2sid.h"
#include "ethernet_dev.h"
#include "filesystem.h"

FLASHMEM void getNtpTime() 
{
   //IO1[rRegLastHourBCD] = 0x0; //91;   // 11pm
   //IO1[rRegLastMinBCD]  = 0x59;      
   //IO1[rRegLastSecBCD]  = 0x53;      
   //Serial.printf("Time: %02x:%02x:%02x %sm\n", (IO1[rRegLastHourBCD] & 0x7f) , IO1[rRegLastMinBCD], IO1[rRegLastSecBCD], (IO1[rRegLastHourBCD] & 0x80) ? "p" : "a");        
   //return;

   if (!EthernetInit()) 
   {
      IO1[rRegLastSecBCD]  = 0;      
      IO1[rRegLastMinBCD]  = 0;      
      IO1[rRegLastHourBCD] = 0;      
      return;
   }
   
   unsigned int localPort = 8888;       // local port to listen for UDP packets
   const char timeServer[] = "us.pool.ntp.org"; // time.nist.gov     NTP server

   udp.begin(localPort);
   
   const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message
   byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets
   
   Serial.printf("Updating time from: %s\n", timeServer);
   while (udp.parsePacket() > 0) ; // discard any previously received packets
   
   // send an NTP request to the time server at the given address
   // set all bytes in the buffer to 0
   memset(packetBuffer, 0, NTP_PACKET_SIZE);
   // Initialize values needed to form NTP request
   packetBuffer[0] = 0b11100011;   // LI, Version, Mode
   packetBuffer[1] = 0;     // Stratum, or type of clock
   packetBuffer[2] = 6;     // Polling Interval
   packetBuffer[3] = 0xEC;  // Peer Clock Precision
   // 8 bytes of zero for Root Delay & Root Dispersion
   packetBuffer[12]  = 49;
   packetBuffer[13]  = 0x4E;
   packetBuffer[14]  = 49;
   packetBuffer[15]  = 52;
   // all NTP fields have been given values, now send a packet requesting a timestamp:
   udp.beginPacket(timeServer, 123); // NTP requests are to port 123
   udp.write(packetBuffer, NTP_PACKET_SIZE);
   udp.endPacket();

   uint32_t beginWait = millis();
   while (millis() - beginWait < 1500) 
   {
      int size = udp.parsePacket();
      if (size >= NTP_PACKET_SIZE) 
      {
         udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
         uint32_t secsSince1900;
         // convert four bytes starting at location 40 to a long integer
         secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
         secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
         secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
         secsSince1900 |= (unsigned long)packetBuffer[43];
         Serial.printf("Received NTP Response in %d mS\n", (millis() - beginWait));

         //since we don't need the date, leaving out TimeLib.h all together
         IO1[rRegLastSecBCD] = DecToBCD(secsSince1900 % 60);
         secsSince1900 = secsSince1900/60 + 30*(int8_t)IO1[rwRegTimezone]; //to  minutes, offset timezone (30 min increments)
         IO1[rRegLastMinBCD] = DecToBCD(secsSince1900 % 60);
         secsSince1900 = (secsSince1900/60) % 24; //to hours
         if (secsSince1900 >= 12) IO1[rRegLastHourBCD] = 0x80 | DecToBCD(secsSince1900-12); //change to 0 based 12 hour and add pm flag
         else IO1[rRegLastHourBCD] =DecToBCD(secsSince1900); //default to AM (bit 7 == 0)
   
         Serial.printf("Time: %02x:%02x:%02x %sm\n", (IO1[rRegLastHourBCD] & 0x7f) , IO1[rRegLastMinBCD], IO1[rRegLastSecBCD], (IO1[rRegLastHourBCD] & 0x80) ? "p" : "a");        
         return;
      }
   }
   Serial.println("NTP Response timeout!");
}

void WriteEEPROM()
{
   Printf_dbg("Wrote $%02x to EEP addr %d\n", eepDataToWrite, eepAddrToWrite);
   EEPROM.write(eepAddrToWrite, eepDataToWrite);
}

FLASHMEM uint8_t RAM2blocks()
{  //see how many 8k banks will fit in RAM2
   char *ptrChip[70]; //64 8k blocks would be 512k (size of RAM2)
   uint8_t ChipNum = 0;
   while(1)
   {
      ptrChip[ChipNum] = (char *)malloc(8192);
      if (ptrChip[ChipNum] == NULL) break;
      ChipNum++;
   } 
   for(uint8_t Cnt=0; Cnt < ChipNum; Cnt++) free(ptrChip[Cnt]);
   //Serial.printf("Created/freed %d  8k blocks (%dk total) in RAM2\n", ChipNum, ChipNum*8);
   return ChipNum;
}

FLASHMEM void MakeBuildInfo()
{
   //Serial.printf("\nBuild Date/Time: %s  %s\nCPU Freq: %lu MHz   Temp: %.1f°C\n", __DATE__, __TIME__, (F_CPU_ACTUAL/1000000), tempmonGetTemp());
   sprintf(SerialStringBuf, "     FW: %s, %s\r\n       Teensy: %luMHz  %.1fC\r", __DATE__, __TIME__, (F_CPU_ACTUAL/1000000), tempmonGetTemp());
}

//FLASHMEM void MakeBuildCPUInfoStr()
//{
//   FreeDriveDirMenu(); //Will mess up navigation if not on TR menu!
//   RedirectEmptyDriveDirMenu(); //OK since we're on the TR settings screen
//  
//   uint32_t CrtMax = (RAM_ImageSize & 0xffffe000)/1024; //round down to k bytes rounded to nearest 8k
//   //Serial.printf("\n\nRAM1 Buff: %luK (%lu blks)\n", CrtMax, CrtMax/8);
//      
//   uint8_t NumChips = RAM2blocks();
//   //Serial.printf("RAM2 Blks: %luK (%lu blks)\n", NumChips*8, NumChips);
//   NumChips = RAM2blocks()-1; //do it again, sometimes get one more, minus one to match reality, not clear why
//   //Serial.printf("RAM2 Blks: %luK (%lu blks)\n", NumChips*8, NumChips);
//  
//   CrtMax += NumChips*8;
//   char FreeStr[20];
//   sprintf(FreeStr, "  %luk free\r", (uint32_t)(CrtMax*1.004));  //larger File size due to header info.
//
//   MakeBuildInfo();
//   strcat(SerialStringBuf, FreeStr);
//}

void UpDirectory()
{
   //non-root of Teensy, SD or USB drive only
   if(PathIsRoot()) return;

   if(IO1[rWRegCurrMenuWAIT] == rmtTeensy) MenuChange(); //back to root, only 1 dir level
   else
   {   
      char * LastSlash = strrchr(DriveDirPath, '/'); //find last slash
      if (LastSlash == NULL) return;
      LastSlash[0] = 0;  //terminate it there 
      if (IO1[rWRegCurrMenuWAIT] == rmtSD) LoadDirectory(&SD); 
      else LoadDirectory(&firstPartition); 
      IO1[rwRegCursorItemOnPg] = 0;
      IO1[rwRegPageNumber]     = 1;
   }
}

bool SetSIDSpeed(bool LogConv, int16_t PlaybackSpeedIn)
{  //called from IO handler, must be quick...
   float PlaybackSpeedPct = PlaybackSpeedIn; //number from -128*256 to 127*256   
   PlaybackSpeedPct = PlaybackSpeedPct/256/100;
   
   int32_t SIDSpeed = IO1[rRegSIDDefSpeedLo]+256*IO1[rRegSIDDefSpeedHi]; //start with default value 
   
   if (LogConv) SIDSpeed -= SIDSpeed*PlaybackSpeedPct; 
   else SIDSpeed = SIDSpeed/(PlaybackSpeedPct+1);
   
   //Printf_dbg("SID Speed: %+0.2f\nReg val 0x%04x\n", PlaybackSpeedPct*100, SIDSpeed);
   if(SIDSpeed > 0xffff || SIDSpeed < 1) 
   {
      //Printf_dbg("Out of reg range (0001 to ffff)\n");
      return false;
   }
   
   IO1[rwRegSIDCurSpeedLo] = SIDSpeed & 0xff;
   IO1[rwRegSIDCurSpeedHi] = (SIDSpeed>>8) & 0xff;
   SidSpeedAdjust = PlaybackSpeedIn; //update C64 side setting
   SidLogConv = LogConv; //in case of remote change
   return true;
}

void SetCursorToItemNum(uint16_t ItemNum)
{
   IO1[rwRegPageNumber] = ItemNum/MaxItemsPerPage +1;
   IO1[rwRegCursorItemOnPg] = ItemNum % MaxItemsPerPage;
   IO1[rRegNumItemsOnPage] = (NumItemsFull > IO1[rwRegPageNumber]*MaxItemsPerPage ? MaxItemsPerPage : NumItemsFull-(IO1[rwRegPageNumber]-1)*MaxItemsPerPage);
}

FLASHMEM void NextFileType(uint8_t FileType1, uint8_t FileType2)
{
   SelItemFullIdx = IO1[rwRegCursorItemOnPg] + (IO1[rwRegPageNumber]-1) * MaxItemsPerPage;
   uint16_t InitItemNum = SelItemFullIdx;
   do
   {
      if (++SelItemFullIdx >= NumItemsFull) SelItemFullIdx = 0;
      if (MenuSource[SelItemFullIdx].ItemType == FileType1 ||
          MenuSource[SelItemFullIdx].ItemType == FileType2)
      {
         SetCursorToItemNum(SelItemFullIdx);
         return;
      }
   } while (SelItemFullIdx != InitItemNum); //just 1 time through, but should stop on same initial one unless changed externally
}

FLASHMEM void LastFileType(uint8_t FileType1, uint8_t FileType2)
{
   SelItemFullIdx = IO1[rwRegCursorItemOnPg] + (IO1[rwRegPageNumber]-1) * MaxItemsPerPage;
   uint16_t InitItemNum = SelItemFullIdx;
   
   do
   {
      if (SelItemFullIdx == 0) SelItemFullIdx = NumItemsFull-1;
      else SelItemFullIdx--;
      
      if (MenuSource[SelItemFullIdx].ItemType == FileType1 ||
          MenuSource[SelItemFullIdx].ItemType == FileType2)
      {
         SetCursorToItemNum(SelItemFullIdx);
         return;
      }
   } while (SelItemFullIdx != InitItemNum); //just 1 time through, but should stop on same initial one unless changed externally   
}

FLASHMEM void NextTextFile()
{
   NextFileType(rtFileTxt, rtFilePETSCII);
}

FLASHMEM void LastTextFile()
{
   LastFileType(rtFileTxt, rtFilePETSCII);   
}

FLASHMEM void NextPicture()
{
   NextFileType(rtFileKla, rtFileArt);
}

FLASHMEM void LastPicture()
{
   LastFileType(rtFileKla, rtFileArt);
}

void SearchForLetter()
{
   uint16_t ItemNum = 0;
   uint8_t SearchFor = IO1[wRegSearchLetterWAIT];
   
   //ascii upper case (toupper) matches petscii lower case ('a'=65)
   while (ItemNum < NumItemsFull)
   {
      if (toupper(MenuSource[ItemNum].Name[0]) >= SearchFor)
      {
         SetCursorToItemNum(ItemNum);
         return;
      }
      ItemNum++;
   }
}

FLASHMEM void GetCurrentFilePathName(char* FilePathName)
{
   char *LclFilename = MenuSource[SelItemFullIdx].Name;
   char Rand[] = "?";
   
   if (IO1[rwRegScratch]) LclFilename = Rand; //random dir
  
   if (IO1[rWRegCurrMenuWAIT] == rmtTeensy) 
   {
      //figure out what menu dir we're in
      char DirName[45] = "/";

      if (MenuSource != TeensyROMMenu)
      {
         //find sub-dir
         uint8_t DirNum = 0;
         while(MenuSource != (StructMenuItem*)TeensyROMMenu[DirNum].Code_Image)
         {
            //MenuSelCpy.Code_Image;
            if (++DirNum == sizeof(TeensyROMMenu)/sizeof(TeensyROMMenu[0]))
            {
               Printf_dbg("TR Dir not found\n"); //what now?
               sprintf(FilePathName, "TR:Dir not found");
               return;
            }
         }
         strcpy(DirName, TeensyROMMenu[DirNum].Name);
      }
      
      sprintf(FilePathName, "TR:%s/%s", DirName, LclFilename);
   }
   else
   {
      char SDUSB[6] = "SD";
      if (IO1[rWRegCurrMenuWAIT] == rmtUSBDrive) strcpy(SDUSB, "USB");
      
      if (PathIsRoot()) sprintf(FilePathName, "%s:/%s", SDUSB, LclFilename);  // at root
      else sprintf(FilePathName, "%s:%s/%s", SDUSB, DriveDirPath, LclFilename);   
   }
}

FLASHMEM void WriteNFCTagCheck()
{
   //IO1[rwRegScratch] 1=rand dir, 0=single file
   IO1[rRegLastHourBCD] = 0; //using this reg as scratch to communicate outcome
   
   if (nfcState != nfcStateEnabled)
   {
      SendMsgPrintfln(" NFC not enabled/found\r");
      return;      
   }
   
   SelItemFullIdx = IO1[rwRegCursorItemOnPg]+(IO1[rwRegPageNumber]-1)*MaxItemsPerPage;

   if (!IO1[rwRegScratch] && MenuSource[SelItemFullIdx].ItemType < rtFilePrg) //single file but not executable
   {
      SendMsgPrintfln(" Invalid File Type (%d)\r", MenuSource[SelItemFullIdx].ItemType);
      return;
   }
   
   char PathMsg[MaxPathLength];
   GetCurrentFilePathName(PathMsg);
   SendMsgPrintfln("File Selected:\r%s\r", PathMsg);
   
   nfcState |= nfcStateBitDisabled; //keep if from trigerring if re-using prev programmed tag
   IO1[rRegLastHourBCD] = 0xff; //checks look good!
}

FLASHMEM void WriteNFCTag()
{   
   //checks have been done, ready to write tag
   //nfc polling not Enabled here
   
   char PathMsg[MaxPathLength];
   GetCurrentFilePathName(PathMsg);
   
   SendMsgPrintfln("Preparing...");
   //Serial.printf("WriteNFCTag: %s\n", PathMsg);

   nfcWriteTag(PathMsg);

   //pause for removal (in assy)
}

FLASHMEM void NFCReEnable()
{              
   // nfc not currently enabled (just wrote a tag)
   nfcInit(); //this should pass, was enabled/initialized previously...
}

FLASHMEM void SetAutoLaunch()
{
   SelItemFullIdx = IO1[rwRegCursorItemOnPg]+(IO1[rwRegPageNumber]-1)*MaxItemsPerPage;

   char PathMsg[MaxPathLength];
   GetCurrentFilePathName(PathMsg);
   SendMsgPrintfln("File Selected:\r%s\r", PathMsg);

   if(MenuSource[SelItemFullIdx].ItemType < rtFilePrg)
   {
      SendMsgPrintfln("Invalid File Type (%d)\r\rAuto Launch *not* updated\r", MenuSource[SelItemFullIdx].ItemType);
      return;
   }

   SendMsgPrintfln("Auto Launch updated:\r  * Will take effect next power-up\r  * See Settings menu to disable\r");
   
   EEPwriteStr(eepAdAutolaunchName, PathMsg);  //set autolaunch in EEPROM:

}

FLASHMEM void ClearAutoLaunch()
{
   EEPROM.write(eepAdAutolaunchName, 0); //disable auto Launch
}

FLASHMEM void SetBackgroundSID()
{
   EEPwriteNBuf(eepAdDefaultSID, (uint8_t*)LatestSIDLoaded, MaxPathLength); //write the source/path/name to EEPROM   
}

FLASHMEM int16_t FindTRMenuItem(StructMenuItem* MyMenu, uint16_t NumEntries, char* EntryName)
{
   for(uint16_t EntryNum=0; EntryNum < NumEntries; EntryNum++)
   {
      if(strcmp(MyMenu[EntryNum].Name, EntryName) == 0) return EntryNum;
   }
   return -1;
}

FLASHMEM void LoadMainSIDforXfer()
{
   //Load EEPROM default SID into TR RAM and prep for transfer
   //if missing, load default
   //Set XferImage and XferSize
      
   EEPreadNBuf(eepAdDefaultSID, (uint8_t*)LatestSIDLoaded, MaxPathLength); //load the source/path/name from EEPROM
   char* LatestSIDName = LatestSIDLoaded+strlen(LatestSIDLoaded+1)+2;
   Printf_dbg("Sel SID: %d %s / %s\n", LatestSIDLoaded[0], LatestSIDLoaded+1, LatestSIDName);

   if (LatestSIDLoaded[0] != rmtTeensy) // SD or USB
   {
      StructMenuItem MyMenuItem;
      FS *sourceFS = &firstPartition;
      if(LatestSIDLoaded[0] == rmtSD)
      {
         sourceFS = &SD;
         SDFullInit(); // SD.begin(BUILTIN_SDCARD); with retry if presence detected
      }
      else USBFileSystemWait(); //wait up to 1.5 sec in case USB drive just changed or powered up
      
      MyMenuItem.Name = LatestSIDName;
      MyMenuItem.ItemType = rtFileSID;

      if(!LoadFile(sourceFS, LatestSIDLoaded+1, &MyMenuItem))  
      { //error, load default from TR     
         Printf_dbg("Ld Err, Default SID\n");
         LatestSIDLoaded[0] = DefSIDSource;  
         strcpy(LatestSIDLoaded+1, DefSIDPath);
         LatestSIDName = LatestSIDLoaded+strlen(DefSIDPath)+2;
         strcpy(LatestSIDName, DefSIDName);
      }
      else 
      {
         XferSize = MyMenuItem.Size;
      }
   }
 
   if (LatestSIDLoaded[0] == rmtTeensy)
   {
      int16_t MenuNum;
      StructMenuItem* DefSIDTRMenu = TeensyROMMenu;  //default to root menu
      uint16_t NumMenuItems = sizeof(TeensyROMMenu)/sizeof(StructMenuItem);
      
      if(strcmp(LatestSIDLoaded+1, "/") !=0 )
      {//find dir menu
         MenuNum = FindTRMenuItem(DefSIDTRMenu, NumMenuItems, LatestSIDLoaded+1);
         if(MenuNum<0)
         {
            Printf_dbg("No SID Dir\n");
            //empty fields????  Shouldn't happen unless compile change
            return;
         }
         DefSIDTRMenu = (StructMenuItem*)TeensyROMMenu[MenuNum].Code_Image;
         NumMenuItems = TeensyROMMenu[MenuNum].Size/sizeof(StructMenuItem);
      }
      //Printf_dbg("SID Dir#%d, %d items\n", MenuNum, NumMenuItems);
 
      //find SID name
      MenuNum = FindTRMenuItem(DefSIDTRMenu, NumMenuItems, LatestSIDName);
      if(MenuNum<0)
      {
         Printf_dbg("No SID Name\n");
         //empty fields????  Shouldn't happen unless compile change
         return;
      }   
      //Printf_dbg("SID #%d\n", MenuNum);
      XferSize = DefSIDTRMenu[MenuNum].Size; 
      memcpy(RAM_Image, DefSIDTRMenu[MenuNum].Code_Image, XferSize);
   }
   
   //Printf_dbg("Load SID: %d %s / %s\n", LatestSIDLoaded[0], LatestSIDLoaded+1, LatestSIDName);
   XferImage = RAM_Image; 
   ParseSIDHeader(LatestSIDName);
}


//__________________________________________________________________________________


void InitHndlr_TeensyROM()
{
   IO1[rwRegNextIOHndlr] = EEPROM.read(eepAdNextIOHndlr);  //in case it was over-ridden by .crt
   //MIDI handlers for MIDI2SID:
   usbHostMIDI.setHandleNoteOff      (M2SOnNoteOff);             // 8x
   usbHostMIDI.setHandleNoteOn       (M2SOnNoteOn);              // 9x
   usbHostMIDI.setHandleControlChange(M2SOnControlChange);       // Bx
   usbHostMIDI.setHandlePitchChange  (M2SOnPitchChange);         // Ex

   usbDevMIDI.setHandleNoteOff       (M2SOnNoteOff);             // 8x
   usbDevMIDI.setHandleNoteOn        (M2SOnNoteOn);              // 9x
   usbDevMIDI.setHandleControlChange (M2SOnControlChange);       // Bx
   usbDevMIDI.setHandlePitchChange   (M2SOnPitchChange);         // Ex
}   

void IO1Hndlr_TeensyROM(uint8_t Address, bool R_Wn)
{
   uint8_t Data;
   if (R_Wn) //High (IO1 Read)
   {
      switch(Address)
      {
         case rRegItemTypePlusIOH:
            Data = MenuSource[SelItemFullIdx].ItemType;
            if(IO1[rWRegCurrMenuWAIT] == rmtTeensy && MenuSource[SelItemFullIdx].IOHndlrAssoc != IOH_None) Data |= 0x80; //bit 7 indicates an assigned IOHandler
            DataPortWriteWaitLog(Data);  
            break;
         case rRegStreamData:
            DataPortWriteWait(XferImage[StreamOffsetAddr]);
            //inc on read, check for end:
            if (++StreamOffsetAddr >= XferSize) IO1[rRegStrAvailable]=0; //signal end of transfer
            break;
         case rwRegSerialString:
            Data = ptrSerialString[StringOffset++];
            DataPortWriteWaitLog(ToPETSCII(Data));            
            break;
         default: //used for all other IO1 reads
            DataPortWriteWaitLog(IO1[Address]); //will read garbage if above IO1Size
            break;
      }
   }
   else  // IO1 write
   {
      Data = DataPortWaitRead(); 
      TraceLogAddValidData(Data);
      switch(Address)
      {
         case rwRegSelItemOnPage:
            SelItemFullIdx = Data+(IO1[rwRegPageNumber]-1)*MaxItemsPerPage;
         case rwRegStatus:
         case wRegVid_TOD_Clks:
         case wRegIRQ_ACK:
         case rwRegIRQ_CMD:
         case rwRegCodeStartPage:
         case rwRegCodeLastPage:
         case rwRegCursorItemOnPg:
         case rwRegSIDSongNumZ:
         case rwRegSIDCurSpeedHi:
         case rwRegSIDCurSpeedLo:
         case rwRegScratch:
            IO1[Address]=Data;
            break;    
            
         case rwRegPageNumber:
            IO1[rwRegPageNumber]=Data;
            IO1[rRegNumItemsOnPage] = (NumItemsFull > Data*MaxItemsPerPage ? MaxItemsPerPage : NumItemsFull-(Data-1)*MaxItemsPerPage);
            break;
         case rwRegNextIOHndlr:
            if (Data & 0x80) Data = LastSelectableIOH; //wrap around to last item if negative
            else if (Data > LastSelectableIOH) Data = 0; //wrap around to first item if above max
            IO1[rwRegNextIOHndlr]= Data;
            eepAddrToWrite = eepAdNextIOHndlr;
            eepDataToWrite = Data;
            IO1[rwRegStatus] = rsWriteEEPROM; //work this in the main code
            break;
         case rWRegCurrMenuWAIT:
            IO1[rWRegCurrMenuWAIT]=Data;
            IO1[rwRegStatus] = rsChangeMenu; //work this in the main code
            break;
         case rwRegPwrUpDefaults:
            IO1[rwRegPwrUpDefaults]= Data;
            eepAddrToWrite = eepAdPwrUpDefaults;
            eepDataToWrite = Data;
            IO1[rwRegStatus] = rsWriteEEPROM; //work this in the main code
            break;
         case rwRegPwrUpDefaults2:
            IO1[rwRegPwrUpDefaults2]= Data;
            eepAddrToWrite = eepAdPwrUpDefaults2;
            eepDataToWrite = Data;
            IO1[rwRegStatus] = rsWriteEEPROM; //work this in the main code
            break;
         case rwRegTimezone:
            IO1[rwRegTimezone]= Data;
            eepAddrToWrite = eepAdTimezone;
            eepDataToWrite = Data;
            IO1[rwRegStatus] = rsWriteEEPROM; //work this in the main code
            break;
         case rwRegColorRefStart ... (rwRegColorRefStart+NumColorRefs-1):
            IO1[Address]= Data;
            eepAddrToWrite = Address-rwRegColorRefStart +eepAdColorRefStart;
            eepDataToWrite = Data;
            IO1[rwRegStatus] = rsWriteEEPROM; //work this in the main code
            break;
         case wRegSearchLetterWAIT:
            IO1[wRegSearchLetterWAIT] = Data;
            IO1[rwRegStatus] = rsSearchForLetter; //work this in the main code
            break;
         case wRegSIDSpeedChange:
            {
               int16_t SidSpeedAdjustTemp = SidSpeedAdjust;
               switch(Data)
               {
                  case rsscIncMajor:
                     SidSpeedAdjustTemp+=2*256;  // 2%
                     break;
                  case rsscDecMajor:
                     SidSpeedAdjustTemp-=2*256;
                     break;
                  case rsscIncMinor:
                     SidSpeedAdjustTemp+=64;  // 0.25%
                     break;
                  case rsscDecMinor:
                     SidSpeedAdjustTemp-=64;
                     break;
                  case rsscSetDefault:
                     SidSpeedAdjustTemp=0;
                     //SidLogConv = false; //def to linear
                     break;
                  case rsscToggleLogLin:
                     SidSpeedAdjustTemp=0;
                     SidLogConv = !SidLogConv;
                     break;
               }
               SetSIDSpeed(SidLogConv, SidSpeedAdjustTemp); //regs & settings updated if pass
            }
            break;
         case rwRegSerialString: //Select/build(no waiting) string to set ptrSerialString and read out serially
            StringOffset = 0;
            switch(Data)
            {
               case rsstItemName:
                  memcpy(SerialStringBuf, MenuSource[SelItemFullIdx].Name, MaxItemDispLength);
                  SerialStringBuf[MaxItemDispLength-1] = 0;
                  ptrSerialString = SerialStringBuf;
                  break;
               case rsstNextIOHndlrName:
                  ptrSerialString = IOHandler[IO1[rwRegNextIOHndlr]]->Name;
                  break;
               case rsstSerialStringBuf:
                  //assumes SerialStringBuf built first...(FWUpd msg or BuildInfo)
                  ptrSerialString = SerialStringBuf; 
                  break;
               case rsstVersionNum:
                  ptrSerialString = strVersionNumber;
                  break;      
               case rsstSIDInfo:
                  ptrSerialString = StrSIDInfo;
                  break;      
               case rsstMachineInfo:
                  ptrSerialString = StrMachineInfo;
                  break;  
               case rsstSIDSpeed:
               {
                  int32_t DefSIDSpeed = IO1[rRegSIDDefSpeedLo]+256*IO1[rRegSIDDefSpeedHi];
                  int32_t CurSIDSpeed = IO1[rwRegSIDCurSpeedLo]+256*IO1[rwRegSIDCurSpeedHi];
                  sprintf(SerialStringBuf, "%0.2f%%  ", (float)DefSIDSpeed/CurSIDSpeed*100);
                  ptrSerialString = SerialStringBuf;       
               }          
                  break;
               case rsstSIDSpeedCtlType:
                  strcpy(SerialStringBuf, (SidLogConv ? "Log" : "Lin"));
                  ptrSerialString = SerialStringBuf;               
                  break;
               case rsstShortDirPath:
                  {
                     uint16_t Len = strlen(DriveDirPath);
                     if (Len >= 40) 
                     {
                        strcpy(SerialStringBuf, "...");
                        strcat(SerialStringBuf, DriveDirPath+Len-36);
                        ptrSerialString = SerialStringBuf;
                     }
                     else ptrSerialString = DriveDirPath;
                  }
                  break;
            }
            break;
            
         case wRegControl:
            switch(Data)
            {
               case rCtlVanishROM:
                  SetGameDeassert;
                  SetExROMDeassert;      
                  LOROM_Image = NULL;
                  HIROM_Image = NULL;  
                  break;
               case rCtlBasicReset:  
                  //SetLEDOff;
                  doReset=true;
                  IO1[rwRegStatus] = rsIOHWNextInit; //Support IO handler at reset
                  break;
               case rCtlStartSelItemWAIT:
                  IO1[rwRegStatus] = rsStartItem; //work this in the main code
                  break;
               case rCtlGetTimeWAIT:
                  IO1[rwRegStatus] = rsGetTime;   //work this in the main code
                  break;
               case rCtlRunningPRG:
                  IO1[rwRegStatus] = rsIOHWSelInit; //Support IO handlers in PRG
                  break;
               case rCtlMakeInfoStrWAIT:
                  IO1[rwRegStatus] = rsMakeBuildCPUInfoStr; //work this in the main code
                  break;
               case rCtlUpDirectoryWAIT:
                  IO1[rwRegStatus] = rsUpDirectory; //work this in the main code
                  break;
               case rCtlLoadSIDWAIT:
                  IO1[rwRegStatus] = rsLoadSIDforXfer; //work this in the main code
                  break;
               case rCtlNextPicture:
                  IO1[rwRegStatus] = rsNextPicture; //work this in the main code
                  break;
               case rCtlLastPicture:
                  IO1[rwRegStatus] = rsLastPicture; //work this in the main code
                  break;
               case rCtlWriteNFCTagCheckWAIT:
                  IO1[rwRegStatus] = rsWriteNFCTagCheck; //work this in the main code
                  break;
               case rCtlWriteNFCTagWAIT:
                  IO1[rwRegStatus] = rsWriteNFCTag; //work this in the main code
                  break;
               case rCtlNFCReEnableWAIT:
                  IO1[rwRegStatus] = rsNFCReEnable; //work this in the main code
                  break;
               case rCtlRebootTeensyROM:
                  REBOOT;
                  break;
               case rCtlSetBackgroundSIDWAIT:
                  IO1[rwRegStatus] = rsSetBackgroundSID; //work this in the main code
                  break;
               case rCtlSetAutoLaunchWAIT:
                  IO1[rwRegStatus] = rsSetAutoLaunch; //work this in the main code
                  break;
               case rCtlClearAutoLaunchWAIT:
                  IO1[rwRegStatus] = rsClearAutoLaunch; //work this in the main code
                  break;
               case rCtlNextTextFile:
                  IO1[rwRegStatus] = rsNextTextFile; //work this in the main code
                  break;
               case rCtlLastTextFile:
                  IO1[rwRegStatus] = rsLastTextFile; //work this in the main code
                  break;
               
            }
            break;
      }
   } //write
}

void PollingHndlr_TeensyROM()
{
   if (IO1[rwRegStatus] != rsReady) 
   {  //ISR requested work
      if (IO1[rwRegStatus]<rsNumStatusTypes) StatusFunction[IO1[rwRegStatus]]();
      else Serial.printf("?Stat: %02x\n", IO1[rwRegStatus]);
      Serial.flush();
      IO1[rwRegStatus] = rsReady;
   }
   usbHostMIDI.read();
   usbDevMIDI.read();
}
