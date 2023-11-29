#ifndef MENU_H
#define MENU_H

#include <Arduino.h>

#include "Menu_Regs.h"

StructMenuItem *DriveDirMenu = NULL;
uint16_t NumDrvDirMenuItems = 0;

void SetUpMainMenuROM();
void SetNumItems (uint16_t NumItems);
void PadSpace (char *StrToPad, uint8_t PadToLength);

#endif // MENU_H