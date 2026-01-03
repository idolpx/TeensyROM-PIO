
#ifndef _FILE_TRANSFER_
#define _FILE_TRANSFER_

#include <Arduino.h>
#include <FS.h>
#include <SPI.h>
#include <SD.h>
#include "DriveDirLoad.h"
#include "Common_Defs.h"

FLASHMEM bool GetPathParameter (char FileNamePath[]);
FLASHMEM FS *GetStorageDevice (uint32_t storageType);
FLASHMEM bool GetFileStream (uint32_t SD_nUSB, char FileNamePath[], FS* sourceFS, File& file);
FLASHMEM bool EnsureDirectory (const char* path, FS& fs);
FLASHMEM bool ReceiveFileData (File& file, uint32_t len, uint32_t &checksum);

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
FLASHMEM bool SendPagedDirectoryContents (FS& fileStream, const char* directoryPath, int skip, int take);

// Command:
// List Directory Contents on TeensyROM given a take and skip value
// to faciliate batch processing.
//
// Workflow:
// Receive <-- List Directory Token 0x64DD
// Send --> AckToken 0x64CC
// Receive <-- SD_nUSB(1), Destination Path(MaxNameLength, null terminator), sake(1), skip(1)
// Send --> AckToken 0x64CC on successful check of directory existence, 0x9b7f on Fail
// Send --> StartDirectoryListToken 0x5A5A or FailToken 0x9b7f
// Send --> Write content as json or ndjson (if SetNDJSONformat=true)
// Send --> EndDirectoryListToken 0xA5A5,  0x9b7f on Fail
FLASHMEM void GetDirectoryCommand (bool SetNDJSONformat = false);
FLASHMEM bool CopyFile (const char* sourcePath, const char* destinationPath, FS& fs);

// Command:
// Copies a file from one folder to the other in the USB/SD storage.
// If the file with the same name already exists at the destination,
// it will be overwritten.
//
// Workflow:
// Receive <-- Post File Token 0x64FF
// Send --> AckToken 0x64CC
// Receive <-- Source Path(MaxNameLength, null terminator), Destinationn Path(MaxNameLength, null terminator)
// Send --> 0x64CC on Pass,  0x9b7f on Fail
//
// Notes: Once Copy File Token Received, responses are 2 bytes in length
FLASHMEM void CopyFileCommand();
FLASHMEM void DeleteFile (const char* filePath, FS& fileSystem);

// Command:
// Delete a file from the specified storage device on TeensyROM.
//
// Workflow:
// Receive <-- Delete File Token (e.g., 0x64EE)
// Send --> AckToken 0x64CC
// Receive <-- SD_nUSB(1), File Path(MaxNameLength, null terminator)
// Send --> 0x64CC on Pass, 0x9b7f on Fail
//
// Notes: Once Delete File Token Received, responses are 2 bytes in length
FLASHMEM void DeleteFileCommand();
FLASHMEM bool SendFileData (File& file, uint32_t len);
FLASHMEM uint32_t CalculateChecksum (File& file);

// Command:
// Retrieve File from specified directory and storage type on TeensyROM.
//
// Workflow:
// Receive <-- Get File Token (e.g., 0x64B0)
// Send --> AckToken 0x64CC
// Receive <-- SD_nUSB(1) Source Path(MaxNameLength, null terminator)
// Send --> 0x64CC on Pass, 0x9b7f on Fail
// Arduino reads the file from storage
// Send --> File Length(4), Checksum(2)
// Send --> File Data in chunks
// Send --> AckToken 0x64CC on successful check of storage availability, 0x9b7f on Fail
// Send --> AckToken 0x64CC on successful completion, 0x9b7f on Fail
//
// Notes: Once Get File Token is received, initial responses are 2 bytes in length. File data is sent in subsequent operations. Checksum is calculated for file validation.
FLASHMEM void GetFileCommand();

#endif // FileTransfer_h