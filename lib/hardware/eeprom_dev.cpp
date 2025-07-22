
#include "eeprom_dev.h"

#include "Common_Defs.h"

#include <Arduino.h>
#include <EEPROM.h>
#include <IPAddress.h>

//#include "../../include/TeensyROM.h"

#include "Menu_Regs.h"
#include "IOH_None.h"

void EEPcheck()
{
    uint32_t MagNumRead;
    EEPROM.get (eepAdMagicNum, MagNumRead);
    if (MagNumRead != eepMagicNum)
        SetEEPDefaults();
}


void EEPwriteNBuf(uint16_t addr, const uint8_t* buf, uint16_t len)
{
   while (len--) EEPROM.write(addr+len, buf[len]);    
}

void EEPwriteStr(uint16_t addr, const char* buf)
{
   EEPwriteNBuf(addr, (uint8_t*)buf, strlen(buf)+1); //include terminator    
}

void EEPreadNBuf(uint16_t addr, uint8_t* buf, uint16_t len)
{
   while (len--) buf[len] = EEPROM.read(addr+len);   
}

void EEPreadStr(uint16_t addr, char* buf)
{
   uint16_t CharNum = 0;
   
   do
   {
      buf[CharNum] = EEPROM.read(addr+CharNum); 
   } while (buf[CharNum++] !=0); //end on termination, but include it in buffer
}

void SetEEPDefaults()
{
   Serial.println("--> Setting EEPROM to defaults");
   EEPROM.write(eepAdPwrUpDefaults, 0x90 | rpudRWReadyDly); //default med js speed, music on, eth time synch off, NFC off, RW delay on
   EEPROM.write(eepAdPwrUpDefaults2, 0x00); //default 12 hour clock mode
   EEPROM.write(eepAdTimezone, -14); //default to pacific time
   EEPROM.write(eepAdNextIOHndlr, IOH_None); //default to no Special HW
   SetEthEEPDefaults();
   EEPROM.write(eepAdDefaultSID, DefSIDSource);  
   EEPwriteStr(eepAdDefaultSID+1, DefSIDPath);
   EEPwriteStr(eepAdDefaultSID+strlen(DefSIDPath)+2, DefSIDName);  
   EEPROM.write(eepAdMinBootInd, MinBootInd_SkipMin);
   EEPROM.write(eepAdAutolaunchName, 0); //disable auto Launch
   //default color scheme:
   EEPROM.write(eepAdColorRefStart+EscBackgndColor , PokeBlack  ); 
   EEPROM.write(eepAdColorRefStart+EscBorderColor  , PokePurple ); 
   EEPROM.write(eepAdColorRefStart+EscTRBannerColor, PokePurple ); 
   EEPROM.write(eepAdColorRefStart+EscTimeColor    , PokeOrange ); 
   EEPROM.write(eepAdColorRefStart+EscOptionColor  , PokeYellow ); 
   EEPROM.write(eepAdColorRefStart+EscSourcesColor , PokeLtBlue ); 
   EEPROM.write(eepAdColorRefStart+EscNameColor    , PokeLtGreen); 
   
   EEPROM.put(eepAdMagicNum, (uint32_t)eepMagicNum); //set this last in case of power down, etc.
}

FLASHMEM void SetEthEEPDefaults()
{
   EEPROM.write(eepAdDHCPEnabled, 1); //DHCP enabled
   uint8_t buf[6]={0xBE, 0x0C, 0x64, 0xC0, 0xFF, 0xEE};
   EEPwriteNBuf(eepAdMyMAC, buf, 6);
   EEPROM.put(eepAdMyIP       , (uint32_t)IPAddress(192,168,1,10));
   EEPROM.put(eepAdDNSIP      , (uint32_t)IPAddress(192,168,1,1));
   EEPROM.put(eepAdGtwyIP     , (uint32_t)IPAddress(192,168,1,1));
   EEPROM.put(eepAdMaskIP     , (uint32_t)IPAddress(255,255,255,0));
   EEPROM.put(eepAdDHCPTimeout, (uint16_t)9000);
   EEPROM.put(eepAdDHCPRespTO , (uint16_t)4000);  
   EEPROM.write(eepAdDLPathSD_USB, Drive_SD); //default to root of SD card
   EEPwriteStr(eepAdDLPath, "/"); 

   const char * DefBookmarks[eepNumBookmarks][2] =
   {
      "TinyWeb64 @ Sensorium",              "http://sensoriumembedded.com/tinyweb64/",
      "Digitalman TeensyROM Demo",          "http://digitalman.azurewebsites.net/",
      "68k.news Headlines from the Future", "http://68k.news/",
      "CNN Lite (filtered)",                "http://www.frogfind.com/read.php?a=http://lite.cnn.com/",
      "CBC Lite News (filtered)",           "http://www.frogfind.com/read.php?a=http://www.cbc.ca/lite/news",
      "textfiles.com",                      "http://textfiles.com/directory.html",
      "Hyperlinked Text (filtered)",        "http://www.frogfind.com/read.php?a=http://sjmulder.nl/en/textonly.html",
      "legiblenews.com (filtered)",         "http://www.frogfind.com/read.php?a=http://legiblenews.com/",
      "text-only news sites (filtered)",    "http://www.frogfind.com/read.php?a=http://greycoder.com/a-list-of-text-only-new-sites",
   };
   
   for (uint8_t BMNum=0; BMNum<eepNumBookmarks; BMNum++)
   {
      EEPwriteStr(eepAdBookmarks+BMNum*(eepBMTitleSize+eepBMURLSize),DefBookmarks[BMNum][0]);
      EEPwriteStr(eepAdBookmarks+BMNum*(eepBMTitleSize+eepBMURLSize)+eepBMTitleSize,DefBookmarks[BMNum][1]);
   }
}
