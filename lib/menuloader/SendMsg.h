
#ifndef SENDMSG_H
#define SENDMSG_H

#include "Menu_Regs.h"
#include "Common_Defs.h"

extern volatile char    *ptrSerialString; //pointer to selected serialstring
extern char SerialStringBuf[MaxPathLength]; // used for message passing to C64, up to full path length

void SendMsgOK();
void SendMsgFailed();
void SendMsgPrintfln (const char *Fmt, ...);
void SendMsgPrintf (const char *Fmt, ...);
void SendMsgSerialStringBuf();

#endif // SENDMSG_H