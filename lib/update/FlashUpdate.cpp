// MIT License
//
// Copyright (c) 2024 Travis Smith
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software
// and associated documentation files (the "Software"), to deal in the Software without
// restriction, including without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom
// the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or
// substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
// BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

// Adapted from FlasherX -- firmware OTA update via Intel Hex file over serial or SD stream
// https://github.com/joepasquariello/FlasherX

#include "FlashUpdate.h"
#include "SendMsg.h"

void DoFlashUpdate (FS *sourceFS, const char *FilePathName)
{
    uint32_t buffer_addr, buffer_size;

    //Serial.printf( "target = %s (%dK flash in %dK sectors)\n", FLASH_ID, FLASH_SIZE/1024, FLASH_SECTOR_SIZE/1024);

    // create flash buffer to hold new firmware
    SendMsgPrintfln ("Create buffer ");
    if (firmware_buffer_init ( &buffer_addr, &buffer_size ) != FLASH_BUFFER_TYPE)
    {
        SendMsgFailed();
        return;
    }
    SendMsgOK();

    SendMsgPrintfln ("%s Buffer = %1luK of %1dK total\r\n(%08lX - %08lX)",
                     IN_FLASH (buffer_addr) ? "Flash" : "RAM", buffer_size / 1024, FLASH_SIZE / 1024,
                     buffer_addr, buffer_addr + buffer_size);

    //Already initialized to get to this point...
    //SendMsgPrintfln( "SD initialization " );
    //if (!SD.begin( BUILTIN_SDCARD ))
    //{
    //   SendMsgFailed();
    //   return;
    //}
    //SendMsgOK();

    SendMsgPrintfln ("Open: %s%s ", sourceFS == &SD ? "SD" : "USB", FilePathName);

    File hexFile = sourceFS->open (FilePathName, FILE_READ );

    if (!hexFile)
    {
        SendMsgFailed();
        return;
    }
    SendMsgOK();

    // read hex file, write new firmware to flash, clean up, reboot
    update_firmware ( &hexFile, &Serial, buffer_addr, buffer_size );

    // return from update_firmware() means error or user abort, so clean up and
    // reboot to ensure that static vars get boot-up initialized before retry(? nah)
    SendMsgPrintfln ( "Erasing Flash buffer ");
    firmware_buffer_free ( buffer_addr, buffer_size );
    SendMsgOK();

    //SendMsgPrintfln( "Rebooting  Teensy");
    //REBOOT;
}
