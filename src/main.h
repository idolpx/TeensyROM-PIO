
#ifndef MAIN_H
#define MAIN_H

#include <Arduino.h>

#include "IOHandlers.h"

volatile uint8_t BtnPressed = false;

void EEPwriteNBuf(uint16_t addr, const uint8_t* buf, uint8_t len);
void EEPwriteStr(uint16_t addr, const char* buf);
void EEPreadNBuf(uint16_t addr, uint8_t* buf, uint8_t len);
void EEPreadStr(uint16_t addr, char* buf);
void SetEEPDefaults();

#endif // MAIN_H