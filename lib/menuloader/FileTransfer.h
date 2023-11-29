
#ifndef FILETRANSFER_H
#define FILETRANSFER_H

#include <Arduino.h>
#include <FS.h>

#define AckToken                0x64CC
#define FailToken               0x9B7F

FLASHMEM bool GetPathParameter(char FileNamePath[]);
FLASHMEM FS* GetStorageDevice(uint32_t storageType);
FLASHMEM bool GetFileStream(uint32_t SD_nUSB, char FileNamePath[], FS* sourceFS, File& file);
FLASHMEM bool EnsureDirectory(const char* path, FS& fs);
FLASHMEM bool ReceiveFileData(File& file, uint32_t len, uint32_t& CheckSum);


// Command: 
// Post File to target directory and storage type on TeensyROM.
// Automatically creates target directory if missing.
//
// Workflow:
// Receive <-- Post File Token 0x64BB 
// Send --> AckToken 0x64CC
// Receive <-- Length(4), Checksum(2), SD_nUSB(1) Destination Path(MaxNameLength, null terminator)
// Send --> 0x64CC on Pass,  0x9b7f on Fail 
// Receive <-- File(length)
// Send --> AckToken 0x64CC on Pass,  0x9b7f on Fail
//
// Notes: Once Post File Token Received, responses are 2 bytes in length
FLASHMEM void PostFileCommand();
FLASHMEM bool SendPagedDirectoryContents(FS& fileStream, const char* directoryPath, int skip, int take);


// Command: 
// List Directory Contents on TeensyROM given a take and skip value
// to faciliate batch processing.
//
// Workflow:
// Receive <-- List Directory Token 0x64DD 
// Send --> AckToken 0x64CC
// Receive <-- SD_nUSB(1), Destination Path(MaxNameLength, null terminator), sake(1), skip(1)
// Send --> StartDirectoryListToken 0x5A5A or FailToken 0x9b7f
// Send --> Write content as json
// Send --> EndDirectoryListToken 0xA5A5,  0x9b7f on Fail
FLASHMEM void GetDirectoryCommand();

#endif // FILETRANSFER_H