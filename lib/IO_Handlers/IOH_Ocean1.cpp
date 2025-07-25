// MIT License
// 
// Copyright (c) 2023 Travis Smith
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

#include "IOH_Ocean1.h"

#include "Common_Defs.h"
#include "IOH_defs.h"
#include "FileParsers.h"

void InitHndlr_Ocean1()
{
   if (NumCrtChips == 64) // Type B, 512k
   {
      //8kLo config
      SetGameDeassert;
      SetExROMAssert;
   }
   else                  // Type A, all other sizes
   {
      //16k config, 8k mirrored
      SetGameAssert;
      SetExROMAssert;
   }

   LOROM_Image = CrtChips[0].ChipROM;
   HIROM_Image = CrtChips[0].ChipROM;
}

void IO1Hndlr_Ocean1(uint8_t Address, bool R_Wn)
{
   if (!R_Wn) // IO1 write    -------------------------------------------------
   {
      uint8_t BankReg = DataPortWaitRead(); 
      
      TraceLogAddValidData(BankReg);
      BankReg &= 0x3f;
      if (BankReg < NumCrtChips)
      {
         LOROM_Image = CrtChips[BankReg].ChipROM;
         HIROM_Image = CrtChips[BankReg].ChipROM;
      }
   }
}

