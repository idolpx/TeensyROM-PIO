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

#ifndef D64_H
#define D64_H

#include <FS.h>
#include "DriveDirLoad.h"

enum enDxxFileNameBytes
{
    DxxFNB_NameLength = 16,
    DxxFNB_NameTerm =   16,
    DxxFNB_Track =      17,
    DxxFNB_Sector =     18,
    DxxFNB_DiskType =   19,
    DxxFNB_Bytes =      20,
};

enum enDxxFileTypes
{
    DxxFT_DEL = 0,
    DxxFT_SEQ = 1,
    DxxFT_PRG = 2,
    DxxFT_USR = 3,
    DxxFT_REL = 4,
};

FLASHMEM uint32_t D64Offset (uint8_t Track, uint8_t Sector);
FLASHMEM uint32_t DxxOffset (uint8_t DiskType, uint8_t Track, uint8_t Sector);
FLASHMEM uint8_t ItemTypeFromDxxFileType (uint8_t FileType);
FLASHMEM void LoadDxxDirectory (FS *sourceFS, uint8_t DiskType);
FLASHMEM bool LoadDxxFile (StructMenuItem* MyMenuItem, FS *sourceFS);

#endif // D64_H