
#ifndef SERUSBIO_H
#define SERUSBIO_H

#include <Arduino.h>

#include "Menu_Regs.h"

FLASHMEM void ServiceSerial();
FLASHMEM void AddAndCheckSource(StructMenuItem SourceMenu, uint32_t *TotalSize);
FLASHMEM void GetDigits(uint8_t NumDigits, uint32_t *SetInt);
FLASHMEM void PrintDebugLog();
FLASHMEM bool ReceiveFileName(uint32_t *SD_nUSB, char *FileNamePath);
FLASHMEM void LaunchFile();
FLASHMEM bool GetUInt(uint32_t *InVal, uint8_t NumBytes);
FLASHMEM void SendU16(uint16_t SendVal);
FLASHMEM bool SerialAvailabeTimeout();
uint32_t RAM2BytesFree();

FLASHMEM void memInfo();
FLASHMEM void  getFreeITCM();

#endif // SERUSBIO_H