#ifndef IOH_STRUCT_H
#define IOH_STRUCT_H

#include <Arduino.h>
#include <USBHost_t36.h>
#include <MIDI.h>
#include <MIDIUSB.h>

#define IOHNameLength 20  //limited by display location on C64

#define usbDevMIDI usbMIDI

USBHost myusbHost;
USBHub hub1(myusbHost);
USBHub hub2(myusbHost);

USBDrive myDrive(myusbHost);
USBFilesystem firstPartition(myusbHost);

MIDIDevice usbHostMIDI(myusbHost);


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