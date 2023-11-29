
#include "eeprom.h"

#include <Arduino.h>
#include <EEPROM.h>

void EEPcheck()
{
    uint32_t MagNumRead;
    EEPROM.get(eepAdMagicNum, MagNumRead);
    if (MagNumRead != eepMagicNum)
        SetEEPDefaults();
}

void EEPwriteNBuf(uint16_t addr, const uint8_t *buf, uint8_t len)
{
    while (len--)
        EEPROM.write(addr + len, buf[len]);
}

void EEPwriteStr(uint16_t addr, const char *buf)
{
    EEPwriteNBuf(addr, (uint8_t *)buf, strlen(buf) + 1); // include terminator
}

void EEPreadNBuf(uint16_t addr, uint8_t *buf, uint8_t len)
{
    while (len--)
        buf[len] = EEPROM.read(addr + len);
}

void EEPreadStr(uint16_t addr, char *buf)
{
    uint16_t CharNum = 0;

    do
    {
        buf[CharNum] = EEPROM.read(addr + CharNum);
    } while (buf[CharNum++] != 0); // end on termination, but include it in buffer
}

void SetEEPDefaults()
{
    Serial.println("Setting EEPROM to defaults");
    EEPROM.put(eepAdMagicNum, (uint32_t)eepMagicNum);
    EEPROM.write(eepAdPwrUpDefaults, 0x90 | rpudMusicMask /* | rpudNetTimeMask */); // default med js speed, music on, eth time synch off
    EEPROM.write(eepAdTimezone, -7);                                                // default to pacific time
    EEPROM.write(eepAdNextIOHndlr, IOH_None);                                       // default to no Special HW
    SetEthEEPDefaults();
}
