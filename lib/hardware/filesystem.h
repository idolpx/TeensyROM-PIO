
#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "Common_Defs.h"

#include <SD.h>

#include "Menu_Regs.h"
#include "IOH_TeensyROM.h"
#include "nfcScan.h"
#include "RemoteControl.h"
#include "midi2sid.h"

#include "Common_Defs.h"

bool SDFullInit()
{

   // begin() takes 3 seconds for fail, 20-200mS for pass, 2 seconds for unpopulated
   
   uint8_t Count = 2; //Max number of begin attempts
   uint32_t Startms = millis();
   
   Printf_dbg("[=][=][=][=][=][=][=][=][=][=][=][=][=][=][=][=][=][=][=][=][=][=][=][=][=][=][=][=][=][=][=][=][=][=][=][=][=][=][=][=][=][=][=][=][=]\n");  
   Printf_dbg("Start mediaPresent %d\n", SD.mediaPresent()); //This indicates zero regardless of actual prior to begin()
    
   while (!SD.begin(BUILTIN_SDCARD)) 
   {
      Count--;
      Printf_dbg("SD Init fail, %d tries left. Time: %lu mS\n", Count, millis()-Startms);
      if (SD.mediaPresent() == 0)
      {
         Printf_dbg("SD Not Present, Fail!  took %lu mS\n", millis()-Startms);
         return false;
      }
      if (Count == 0)
      {
         Printf_dbg("Out of tries, Fail!  took %lu mS\n", millis()-Startms);
         return false;
      }
   }
   
   Printf_dbg("SD Init OK, took %lu mS, mediaPresent %d\n", millis()-Startms, SD.mediaPresent());
   return true;
}


bool CheckLaunchSDAuto()
{         
   //returns true only if file launched
   
   // SD not present: 0mS
   // autolaunch.txt Not Found: 9-10mS
   // Launch file name too short: 10mS
   // Launch file attempted, not found: 39mS
   // Autolaunch file found, launch set-up: 10mS
   
   // _SD_DAT3 = pin 46
   pinMode(46, INPUT_PULLDOWN);
   if (digitalReadFast(46))
   {  //SD Presence detected, do full init and check for auotlaunch file    
      Printf_dbg("SD Presence detected\n");
      if (SDFullInit())
      {
         File AutoLaunchFile = SD.open("autolaunch.txt", FILE_READ);
         if (!AutoLaunchFile) 
         {
            Printf_dbg("autolaunch.txt Not Found\n");
            return false;
         }
         
         char AutoFileName[MaxPathLength];
         uint16_t CharNum = 0;
         char NextChar = 1;
         
         while (NextChar)
         {
            if(AutoLaunchFile.available()) NextChar = AutoLaunchFile.read();
            else NextChar = 0;
            
            if (NextChar=='\r' || NextChar=='\n' || CharNum == MaxPathLength-1) NextChar = 0;           
            
            AutoFileName[CharNum++]=NextChar;
         }
         AutoLaunchFile.close();
         
         Printf_dbg("SD First line: %d chars \"%s\"\n", CharNum, AutoFileName); 
         
         if (CharNum<6) 
         {
            Printf_dbg("Filename too short\n");
            return false;
         }

         char * ptrAutoFileName = AutoFileName; //pointer to move past SD/USB/TR:
         RegMenuTypes SourceID = RegMenuTypeFromFileName(&ptrAutoFileName);
         
         Printf_dbg("SD Autolaunch %d \"%s\"\n", SourceID, ptrAutoFileName); 
         
         //check if file exists????????????
         
         RemoteLaunch(SourceID, ptrAutoFileName, true); //do CRT directly 
         return true;
      }  //SD init
      else Printf_dbg("SDFullInit fail\n");
   }  //SD presence
   else Printf_dbg("No SD detected\n");

   return false;
}

bool USBFileSystemWait()
{
   //wait for USB file system to init in case it's needed by startup SID or auto-launched
   // ~7mS for direct connect, ~1000mS for connect via hub, 1500mS timeout (drive not found)
   uint32_t StartMillis = millis();

   Printf_dbg("[=][=][=][=][=][=][=][=][=][=][=][=][=][=][=][=][=][=][=][=][=][=][=][=][=][=][=][=][=][=][=][=][=][=][=][=][=][=][=][=][=][=][=][=][=]\n");  
   
   while (!firstPartition && millis()-StartMillis < 1500) myusbHost.Task();
   
   if(firstPartition) 
   {
      Printf_dbg("%dmS to init USB drive\n", millis()-StartMillis); 
      return true;
   }

   Printf_dbg("USB drive not found!\n");  
   return false;
}

#endif // FILESYSTEM_H