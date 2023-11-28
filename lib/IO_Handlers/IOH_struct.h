#ifndef IOH_STRUCT_H
#define IOH_STRUCT_H

#include <Arduino.h>

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

#endif // IOH_STRUCT_H