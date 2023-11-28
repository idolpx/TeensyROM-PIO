
// Adapted from:

//******************************************************************************
// FlasherX -- firmware OTA update via Intel Hex file over serial or SD stream
// https://github.com/joepasquariello/FlasherX
//******************************************************************************
//
// Based on Flasher3 (Teensy 3.x) and Flasher4 (Teensy 4.x) by Jon Zeeff
//
// Jon Zeeff 2016, 2019, 2020 This code is in the public domain.
// Please retain my name in distributed copies, and let me know about any bugs
//
// I, Jon Zeeff, give no warranty, expressed or implied for this software and/or
// documentation provided, including, without limitation, warranty of
// merchantability and fitness for a particular purpose.
//
// WARNING: You can brick your Teensy with incorrect flash erase/write, such as
// incorrect flash config (0x400-40F). This code may or may not prevent that.

// 10/09/22 (v2.3) JWP - option for reading hex file from serial or SD
//   - move hex file support functions to new file FXUtil.cpp
//   - update_firmware() now takes two Stream* arguments ("in" and "out")
//   - FlasherX.ino lets user choose between hex file via serial or SD
// 09/01/22 (v2.2) JWP - change FlashTxx from CPP to C file
//   - rename FlashTxx.cpp to FlashTxx.c (resolve link error when calling from C)
//   - FlasherX.ino place #include "FlashTxx.h" inside extern "C" block
// 01/07/22 (v2.1) JWP - use TD 1.56 core functions for T4x wait/write/erase
//   - FlashTxx.h update FLASH_SIZE for Teensy Micromod from 8 to 16 MB
//   - option to artificially increase code size via const array (in flash)
// 11/18/21 JWP - bug fix in file FlashTXX.cpp
//   - fix logic in while loop in flash_block_write() in FlashTXX
// 10/27/21 JWP - add support for Teensy Micromod
//   - define macros for TEENSY_MICROMOD w/ same values as for TEENSY40
//   - update FLASH_SIZE for T4.1 and TMM from 2MB to 8MB
// JWP - merge of Flasher3/4 and new features
//   - FLASH buffer dynamically sized from top of existing code to FLASH_RESERVE
//   - optional RAM buffer option for T4.x via macro RAM_BUFFER_SIZE > 0
//   - Stream* (USB or UART) and buffer addr/size set at run-time
//   - incorporate Frank Boesing's FlashKinetis routines for T3.x
//   - add support for Teensy 4.1 and Teensy LC
//    This code is released into the public domain.
// JWP - Joe Pasquariello - modifications for T3.5 and T3.6 in Dec 2020
//    This code is released into the public domain
// Deb Hollenback at GiftCoder -- Modifications for teensy 3.5/3/6
//    This code is released into the public domain.
//    see https://forum.pjrc.com/threads/43165-Over-the-Air-firmware-updates-changes-for-flashing-Teensy-3-5-amp-3-6
// Jon Zeeff modifications
//    see https://forum.pjrc.com/threads/29607-Over-the-air-updates
// Original by Niels A. Moseley, 2015.
//    This code is released into the public domain.
//    https://namoseley.wordpress.com/2015/02/04/freescale-kinetis-mk20dx-series-flash-erasing/

#include "FlashUpdate.h"

#include <Arduino.h>
#include <SD.h>

#include "DriveDirLoad.h"

#include "FXUtil.h" // read_ascii_line(), hex file support
extern "C"
{
#include "FlashTxx.h" // TLC/T3x/T4x/TMM flash primitives
}

void DoFlashUpdate(FS *sourceFS, const char *FilePathName)
{
    uint32_t buffer_addr, buffer_size;

    // Serial.printf( "target = %s (%dK flash in %dK sectors)\n", FLASH_ID, FLASH_SIZE/1024, FLASH_SECTOR_SIZE/1024);

    // create flash buffer to hold new firmware
    SendMsgPrintfln("Create buffer ");
    if (firmware_buffer_init(&buffer_addr, &buffer_size) != FLASH_BUFFER_TYPE)
    {
        SendMsgFailed();
        return;
    }
    SendMsgOK();

    SendMsgPrintfln("%s Buffer = %1luK of %1dK total\r\n(%08lX - %08lX)",
                    IN_FLASH(buffer_addr) ? "Flash" : "RAM", buffer_size / 1024, FLASH_SIZE / 1024,
                    buffer_addr, buffer_addr + buffer_size);

    // Already initialized to get to this point...
    // SendMsgPrintfln( "SD initialization " );
    // if (!SD.begin( BUILTIN_SDCARD ))
    //{
    //    SendMsgFailed();
    //    return;
    // }
    // SendMsgOK();

    SendMsgPrintfln("Open: %s%s ", sourceFS == &SD ? "SD" : "USB", FilePathName);

    File hexFile = sourceFS->open(FilePathName, FILE_READ);

    if (!hexFile)
    {
        SendMsgFailed();
        return;
    }
    SendMsgOK();

    // read hex file, write new firmware to flash, clean up, reboot
    update_firmware(&hexFile, &Serial, buffer_addr, buffer_size);

    // return from update_firmware() means error or user abort, so clean up and
    // reboot to ensure that static vars get boot-up initialized before retry(? nah)
    SendMsgPrintfln("Erasing Flash buffer ");
    firmware_buffer_free(buffer_addr, buffer_size);
    SendMsgOK();

    // SendMsgPrintfln( "Rebooting  Teensy");
    // REBOOT;
}
