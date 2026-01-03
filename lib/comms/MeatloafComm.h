

#ifndef MEATLOAFCOMM_H
#define MEATLOAFCOMM_H

#include <CRC32.h>

#define MeatloafBaud      2000000   // 115200 460800 2000000
#define ChunkSize              64   //bytes to send in chunk before ack
#define ChunkAckChar          '+'   //char sent to ack chunk

FLASHMEM bool HostSerialAvailabeTimeout (uint32_t TimeoutmS);
FLASHMEM void FlushUSBHostRx();
FLASHMEM bool WaitCheckresponse (const char *Name, const char CheckChar);
FLASHMEM void MountDxxFile();

#endif // MEATLOAFCOMM_H