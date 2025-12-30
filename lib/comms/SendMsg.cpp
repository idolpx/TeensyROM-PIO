
#ifndef MinimumBuild
#include "SendMsg.h"

#include <Arduino.h>

#include "IOH_TeensyROM.h"

// Global variable definitions
volatile char    *ptrSerialString = NULL; //pointer to selected serialstring
char SerialStringBuf[MaxPathLength] = "err"; // used for message passing to C64, up to full path length

void SendMsgOK()
{
    SendMsgPrintf ("OK");
}

void SendMsgFailed()
{
    SendMsgPrintf ("Failed!");
}

void SendMsgPrintfln (const char *Fmt, ...)
{
    va_list ap;
    va_start (ap, Fmt);
    vsprintf (SerialStringBuf, Fmt, ap);
    va_end (ap);

    // add \r\n to the beginning:
    for (uint16_t pos = strlen (SerialStringBuf) + 2; pos > 1; pos--) SerialStringBuf[pos] = SerialStringBuf[pos - 2];
    SerialStringBuf[0] = '\r';
    SerialStringBuf[1] = '\n';

    SendMsgSerialStringBuf();
}

void SendMsgPrintf (const char *Fmt, ...)
{
    va_list ap;
    va_start (ap, Fmt);
    vsprintf (SerialStringBuf, Fmt, ap);
    va_end (ap);
    SendMsgSerialStringBuf();
}

void SendMsgSerialStringBuf()
{
    // SerialStringBuf already populated
    Serial.printf ("%s<--", SerialStringBuf);
    Serial.flush();
    IO1[rwRegStatus] = rsC64Message; // tell C64 there's a message
    uint32_t beginWait = millis();
    // wait up to 3 sec for C64 to read message:
    while (millis() - beginWait < 3000) if (IO1[rwRegStatus] == rsContinue) return;
    Serial.printf ("\nTimeout!\n");
}

#endif