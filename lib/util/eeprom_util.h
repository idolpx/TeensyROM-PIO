#ifndef EEPROM_H
#define EEPROM_H

#include <Arduino.h>

#define eepMagicNum         0xfeed6404 // 01: 6/22/23 net settings added 
// 02: 9/07/23 Joy2 speed added
// 03: 11/3/23 Browser Bookmarks added
// 04: 11/4/23 Browser DL drive/path added
#define eepBMTitleSize       75  //max chars in bookmark title
#define eepBMURLSize        225  //Max Chars in bookmark URL path
#define eepNumBookmarks       9  //Num Bookmarks saved

enum InternalEEPROMmap
{
    eepAdMagicNum      =  0, // (4:uint32_t)   Indicated if internal EEPROM has been initialized
    eepAdPwrUpDefaults =  4, // (1:uint8_t)    power up default reg, see bit mask defs rpudMusicMask, rpudNetTimeMask
    eepAdTimezone      =  5, // (1:int8_t)     signed char for timezone: UTC +14/-12
    eepAdNextIOHndlr   =  6, // (1:uint8_t)    default IO handler to load upon TR exit
    eepAdDHCPEnabled   =  7, // (1:uint8_t)    non-0=DHCP enabled, 0=DHCP disabled
    eepAdMyMAC         =  8, // (6:uint8_t x6) default IO handler to load upon TR exit
    eepAdMyIP          = 14, // (4:uint8_t x4) My IP address (static)
    eepAdDNSIP         = 18, // (4:uint8_t x4) DNS IP address (static)
    eepAdGtwyIP        = 22, // (4:uint8_t x4) Gtwy IP address (static)
    eepAdMaskIP        = 26, // (4:uint8_t x4) Mask IP address (static)
    eepAdDHCPTimeout   = 30, // (2:uint16_t)   DNS Timeout
    eepAdDHCPRespTO    = 32, // (2:uint16_t)   DNS Response Timeout
    eepAdDLPathSD_USB  = 34, // (1:uint8_t)    Download path is on SD or USB per Drive_SD/USB
    eepAdDLPath        = 35, // (TxMsgMaxSize=128)   Download path
    eepAdBookmarks     = 163, // (75+225)*9     Bookmark Titles and Full Paths
    //Max size = 4284 (4k, emulated in flash)
};

void EEPcheck();
void EEPwriteNBuf (uint16_t addr, const uint8_t *buf, uint8_t len);
void EEPwriteStr (uint16_t addr, const char *buf);
void EEPreadNBuf (uint16_t addr, uint8_t *buf, uint8_t len);
void EEPreadStr (uint16_t addr, char *buf);
void SetEEPDefaults();
FLASHMEM void SetEthEEPDefaults();

#endif // EEPROM_H