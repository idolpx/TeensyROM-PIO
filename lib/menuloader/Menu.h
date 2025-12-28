#ifndef MENU_H
#define MENU_H

#include <Arduino.h>

#include "Menu_Regs.h"

extern StructMenuItem *DriveDirMenu;
extern uint16_t NumDrvDirMenuItems;

void SetUpMainMenuROM();
void SetNumItems (uint16_t NumItems);
void PadSpace (char *StrToPad, uint8_t PadToLength);

#endif // MENU_H