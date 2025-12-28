
#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "Common_Defs.h"

#include <SD.h>

#include "Menu_Regs.h"
#include "IOH_TeensyROM.h"
#include "nfcScan.h"
#include "RemoteControl.h"
#include "midi2sid.h"

bool SDFullInit();
bool CheckLaunchSDAuto();
bool USBFileSystemWait();

#endif // FILESYSTEM_H