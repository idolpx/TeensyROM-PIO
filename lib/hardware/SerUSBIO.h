
#ifndef SERUSBIO_H
#define SERUSBIO_H

#include <Arduino.h>

#include "Menu_Regs.h"

FLASHMEM void ServiceSerial();
FLASHMEM void AddAndCheckSource(StructMenuItem SourceMenu, uint32_t *TotalSize);
FLASHMEM void GetDigits(uint8_t NumDigits, uint32_t *SetInt);
FLASHMEM void PrintDebugLog();
FLASHMEM void ReceiveFile();
FLASHMEM bool GetUInt(uint32_t *InVal, uint8_t NumBytes);
FLASHMEM void SendU16(uint16_t SendVal);
FLASHMEM bool SerialAvailabeTimeout();

uint32_t RAM2BytesFree();

//memory info display via:
// https://forum.pjrc.com/threads/33443-How-to-display-free-ram
// https://www.pjrc.com/store/teensy41.html#memory

FLASHMEM void memInfo ();

uint32_t *ptrFreeITCM;  // Set to Usable ITCM free RAM
uint32_t  sizeofFreeITCM; // sizeof free RAM in uint32_t units.
uint32_t  SizeLeft_etext;
extern char _stext[], _etext[];

FLASHMEM void  getFreeITCM();

#endif // SERUSBIO_H