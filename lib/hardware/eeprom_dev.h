#ifndef EEPROM_H
#define EEPROM_H

#include <Arduino.h>

#include "Common_Defs.h"

void EEPcheck();
void EEPwriteNBuf(uint16_t addr, const uint8_t* buf, uint16_t len);
void EEPwriteStr (uint16_t addr, const char *buf);
void EEPreadNBuf (uint16_t addr, uint8_t *buf, uint16_t len);
void EEPreadStr (uint16_t addr, char *buf);
void SetEEPDefaults();
FLASHMEM void SetEthEEPDefaults();

#endif // EEPROM_H