#ifndef IOH_DEFS_H
#define IOH_DEFS_H

#include <Arduino.h>

#include "../../include/TeensyROM.h"

#define IOHNameLength 20  //limited by display location on C64

struct stcIOHandlers
{
  char Name[IOHNameLength];                        //Name of handler
  void (*InitHndlr)();                             //Called once at handler startup
  void (*IO1Hndlr)(uint8_t Address, bool R_Wn);    //IO1 R/W handler
  void (*IO2Hndlr)(uint8_t Address, bool R_Wn);    //IO2 R/W handler
  void (*ROMLHndlr)(uint32_t Address);             //ROML Read handler, in addition to any ROM data sent
  void (*ROMHHndlr)(uint32_t Address);             //ROMH Read handler, in addition to any ROM data sent
  void (*PollingHndlr)();                          //Polled in main routine
  void (*CycleHndlr)();                            //called at the end of EVERY c64 cycle
};

const unsigned char *HIROM_Image = NULL;
const unsigned char *LOROM_Image = NULL;
uint16_t LOROM_Mask, HIROM_Mask;

#endif // IOH_DEFS_H