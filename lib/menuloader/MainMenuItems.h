
#ifndef MAINMENUITEMS_H
#define MAINMENUITEMS_H

#include "../../include/TeensyROM.h"

//add PROGMEM to declaration of all main menu binaries:
#include "ROMs/ccgms_2021_Swiftlink_DE_38400.prg.h"
#include "ROMs/cynthcart_201.prg.h"
#include "ROMs/sta64_v2_6.prg.h"
#include "ROMs/Epyx_Fast_Load.crt.h"
#include "ROMs/80columns.prg.h"
#include "ROMs/hex_mon.prg.h"
#include "ROMs/586220ast_Diagnostics.h"
#include "ROMs/781220_Dead_Test.h"
#include "ROMs/1541_Diagnostics.h"
#include "ROMs/SID_Tester.h"
#include "ROMs/game_controller_tester.prg.h"
#include "ROMs/rom_chksum_137kernals.prg.h"
#include "ROMs/C128_789010.crt.h"
#include "ROMs/Jupiter_Lander.h"
#include "ROMs/joust.prg.h"
#include "ROMs/Terminator_2.crt.h"
#include "ROMs/Beach_Head_II.crt.h"
#include "ROMs/Joystick_Tester.h"
#include "ROMs/Keyboard_Tester.h"
#include "ROMs/Donkey_Kong.h"
#include "ROMs/Dig_Dug.bin.h"
#include "ROMs/DualCopy.prg.h"
#include "ROMs/ember_head.prg.h"
#include "ROMs/disp_fract.prg.h"
#include "ROMs/draw01.prg.h"
#include "ROMs/super.m.bros.64.prg.h"
#include "ROMs/Ms._Pac_Man.crt.h"
#include "ROMs/C64618_Gorf_8000.crt.h"
#include "ROMs/KawariQuickChange.prg.h"
#include "ROMs/Mario_Bros.prg.h"
#include "ROMs/minesweeper_game.prg.h"
#include "ROMs/sam.prg.h"
#include "ROMs/SID_check.prg.h"
#include "ROMs/swinth_LIGHT_FANTASTIC.PRG.h"
#include "ROMs/Tetris.prg.h"
#include "ROMs/super_expander_64.crt.h"
#include "ROMs/Robocop_2_EasyFlash.crt.h"


StructMenuItem dirGames[] =
{
    rtDirectory, IOH_None, (char *)UpDirString, NULL, 0,                                  //one dir level max, up Dir is always root
    rtFileCrt, IOH_None, (char *)"Beach Head II         256k MC", (uint8_t *)Beach_Head_II_crt, sizeof (Beach_Head_II_crt),
    rtBin16k, IOH_None, (char *)"Dig Dug", (uint8_t *)Dig_Dug_bin, sizeof (Dig_Dug_bin),
    rtBin16k, IOH_None, (char *)"Donkey Kong", (uint8_t *)Donkey_Kong_BIN, sizeof (Donkey_Kong_BIN),
    rtFileCrt, IOH_None, (char *)"Gorf!", (uint8_t *)C64618_Gorf_8000_crt, sizeof (C64618_Gorf_8000_crt),
    rtFilePrg, IOH_None, (char *)"Joust!", (uint8_t *)joust_prg, sizeof (joust_prg),
    rtBin8kHi, IOH_None, (char *)"Jupiter Lander", (uint8_t *)Jupiter_Lander_BIN, sizeof (Jupiter_Lander_BIN),
    rtFilePrg, IOH_None, (char *)"Mario Brothers", (uint8_t *)Mario_Bros_prg, sizeof (Mario_Bros_prg),
    rtFilePrg, IOH_None, (char *)"Minesweeper", (uint8_t *)minesweeper_game_prg, sizeof (minesweeper_game_prg),
    rtFileCrt, IOH_None, (char *)"Ms. Pac-Man", (uint8_t *)Ms__Pac_Man_crt, sizeof (Ms__Pac_Man_crt),
    rtFileCrt, IOH_None, (char *)"Robocop 2             224k EZF", (uint8_t *)Robocop_2_EasyFlash_crt, sizeof (Robocop_2_EasyFlash_crt),
    rtFilePrg, IOH_None, (char *)"Super Mario Brothers   48k PRG", (uint8_t *)super_m_bros_64_prg, sizeof (super_m_bros_64_prg),
    rtFileCrt, IOH_None, (char *)"Terminator 2 +MM+IM   512k Ocean", (uint8_t *)Terminator_2_crt, sizeof (Terminator_2_crt),
    rtFilePrg, IOH_None, (char *)"Tetris", (uint8_t *)Tetris_prg, sizeof (Tetris_prg),
};

StructMenuItem dirTest_Diags[] =
{
    rtDirectory, IOH_None, (char *)UpDirString, NULL, 0,                                  //one dir level max, up Dir is always root
    rtBin8kLo, IOH_None, (char *)"1541 Diagnostics", (uint8_t *)a1541_Diagnostics_BIN, sizeof (a1541_Diagnostics_BIN),
    rtBin8kLo, IOH_None, (char *)"586220* C64 Diagnostics", (uint8_t *)a586220ast_Diagnostics_BIN, sizeof (a586220ast_Diagnostics_BIN),
    rtBin8kHi, IOH_None, (char *)"781220 C64 Dead Test", (uint8_t *)a781220_Dead_Test_BIN, sizeof (a781220_Dead_Test_BIN),
    rtFileCrt, IOH_None, (char *)"789010 C128 Diagnostics", (uint8_t *)C128_789010_crt, sizeof (C128_789010_crt),
    rtFilePrg, IOH_None, (char *)"Game Controller Tester", (uint8_t *)game_controller_tester_prg, sizeof (game_controller_tester_prg),
    rtBin8kLo, IOH_None, (char *)"Joystick Tester", (uint8_t *)Joystick_Tester_BIN, sizeof (Joystick_Tester_BIN),
    rtBin8kLo, IOH_None, (char *)"Keyboard Tester", (uint8_t *)Keyboard_Tester_BIN, sizeof (Keyboard_Tester_BIN),
    rtFilePrg, IOH_None, (char *)"ROM Checksum read", (uint8_t *)rom_chksum_137kernals_prg, sizeof (rom_chksum_137kernals_prg),
    rtBin8kLo, IOH_None, (char *)"SID Tester", (uint8_t *)SID_Tester_BIN, sizeof (SID_Tester_BIN),
    rtFilePrg, IOH_None, (char *)"SID checker/finder", (uint8_t *)SID_check_prg, sizeof (SID_check_prg),
};

StructMenuItem dirUtilities[] =
{
    rtDirectory, IOH_None, (char *)UpDirString, NULL, 0,                                  //one dir level max, up Dir is always root
    rtFilePrg, IOH_None, (char *)"80 Columns", (uint8_t *)a80columns_prg, sizeof (a80columns_prg),
    rtFilePrg, IOH_None, (char *)"DualCopy", (uint8_t *)DualCopy_prg, sizeof (DualCopy_prg),
    rtFileCrt, IOH_None, (char *)"Epyx Fast Load Cart", (uint8_t *)Epyx_Fast_Load_crt, sizeof (Epyx_Fast_Load_crt),
    rtFilePrg, IOH_None, (char *)"Hex Mon", (uint8_t *)hex_mon_prg, sizeof (hex_mon_prg),
    rtFilePrg, IOH_None, (char *)"Kawari Quick Change", (uint8_t *)KawariQuickChange_prg, sizeof (KawariQuickChange_prg),
    rtFilePrg, IOH_None, (char *)"S.A.M.     ]RECITER   SAY\"hello\"", (uint8_t *)sam_prg, sizeof (sam_prg),
    rtFileCrt, IOH_None, (char *)"Super Expander 64", (uint8_t *)super_expander_64_crt, sizeof (super_expander_64_crt),
};

StructMenuItem dirMultimedia[] =
{
    rtDirectory, IOH_None, (char *)UpDirString, NULL, 0,                                  //one dir level max, up Dir is always root
    rtFilePrg, IOH_None, (char *)"Display Fractal", (uint8_t *)disp_fract_prg, sizeof (disp_fract_prg),
    rtFilePrg, IOH_None, (char *)"Draw!", (uint8_t *)draw01_prg, sizeof (draw01_prg),
    rtFilePrg, IOH_None, (char *)"Ember Head", (uint8_t *)ember_head_prg, sizeof (ember_head_prg),
    rtFilePrg, IOH_None, (char *)"Swinth/Light Fantastic", (uint8_t *)swinth_LIGHT_FANTASTIC_PRG, sizeof (swinth_LIGHT_FANTASTIC_PRG),
};

//define this last:
StructMenuItem TeensyROMMenu[] =
{
//regItemTypes, enumIOHandlers
// ItemType   , IOHndlrAssoc     , Name                                      , *Code_Image                          , Size ,

    rtDirectory, IOH_None, (char *)"/Games", (uint8_t *)dirGames, sizeof (dirGames),
    rtDirectory, IOH_None, (char *)"/Multimedia", (uint8_t *)dirMultimedia, sizeof (dirMultimedia),
    rtDirectory, IOH_None, (char *)"/Test+Diags", (uint8_t *)dirTest_Diags, sizeof (dirTest_Diags),
    rtDirectory, IOH_None, (char *)"/Utilities", (uint8_t *)dirUtilities, sizeof (dirUtilities),
    rtFilePrg, IOH_Swiftlink, (char *)"CCGMS 2021 Term       +SwiftLink ", (uint8_t *)ccgms_2021_Swift_DE_38k_prg, sizeof (ccgms_2021_Swift_DE_38k_prg),
    rtFilePrg, IOH_MIDI_Datel, (char *)"Cynthcart 2.0.1      +Datel MIDI ", (uint8_t *)cynthcart_201_prg, sizeof (cynthcart_201_prg),
    rtFilePrg, IOH_MIDI_Passport, (char *)"Station64 2.6     +Passport MIDI ", (uint8_t *)sta64_v2_6_prg, sizeof (sta64_v2_6_prg),
};

#endif // MAINMENUITEMS_H