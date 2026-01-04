#include "Common_Defs.h"
#include "Menu_Regs.h"

//add PROGMEM to declaration of all main menu binaries:
#include "ROMs/ccgms_2021_Swiftlink_DE_38400.prg.h"
#include "ROMs/SST.prg.h"
#include "ROMs/cynthcart_201.prg.h"
#include "ROMs/sta64_v2_6.prg.h"
#include "ROMs/sid_wizard_1.9.prg.h"
#include "ROMs/SID_Wizard_2SID.prg.h"
#include "ROMs/SW_1.9_UserManual.prg.h"
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
#include "ROMs/jupiter_lander_crack.prg.h"
#include "ROMs/joust.prg.h"
#include "ROMs/Beach_Head_II.crt.h"
#include "ROMs/Joystick_Tester.h"
#include "ROMs/Keyboard_Tester.h"
#include "ROMs/Donkey_Kong.h"
#include "ROMs/Dig_Dug.bin.h"
#include "ROMs/DualCopy.prg.h"
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
#include "ROMs/MIDI2SID.prg.h"
#include "ROMs/ASIDPlayer.prg.h"
#include "ROMs/TRCBC.prg.h"
#include "ROMs/destest_max.rom.h"
#include "ROMs/empty.prg.h"
#include "ROMs/DMACheck.prg.h"
#include "ROMs/cia_tod.prg.h"
#include "ROMs/Load8Run.prg.h"
#include "ROMs/retromate_teensyrom.prg.h"
#include "ROMs/TeensyROMC64.h"
//#include "ROMs/Terminator_2.crt.h" //can require minimal build if Ethernet, etc enabled
//#include "ROMs/ember_head.prg.h"   //now /Pics/T_Ember_Head.kla
#include "ROMs/disp_fract.prg.h"   //now /Pics/T_BlackHole.kla
//#include "SIDs/YYZ.sid.h"          //mem conflict with TR app

#include "PETSCII/Text_Viewer_Instructions.seq.h"
#include "PETSCII/color_weave.seq.h"
#include "PETSCII/Cygnus_X_1_Book_I___The_Voyage.txt.h"
#include "PETSCII/do_not_panic.seq.h"
#include "PETSCII/electric_guitar.seq.h"
#include "PETSCII/inter.circuits.seq.h"
#include "PETSCII/irnmdn_aceshigh.seq.h"
#include "PETSCII/Jousters___c_g179.seq.h"
#include "PETSCII/living_color.1.seq.h"
#include "PETSCII/Small_Kindnesses.txt.h"
#include "PETSCII/Stripes__Shoeshine_Boy___13picturedisk_2.seq.h"
#include "PETSCII/tiger.cg.seq.h"
#include "PETSCII/Time_Travel__Shoeshine_Boy__11picturedisk_2.seq.h"
#include "PETSCII/Trains_II__Yoda______12picturedisk_6.seq.h"
#include "PETSCII/Wish_You_Were_Here.txt.h"

#include "SIDs/Echoes.sid.h"
#include "SIDs/SleepDirt_norm_ntsc_1000_6581.sid.h"
#include "SIDs/Aces_High.sid.h"
#include "SIDs/Comfortably_Numb.sid.h"
#include "SIDs/Medley.sid.h"
#include "SIDs/Pink_Floyd.sid.h"
#include "SIDs/Popcorn.sid.h"
#include "SIDs/Switch_625.sid.h"
#include "SIDs/Tom_Sawyer.sid.h"
#include "SIDs/Wish_You_Were_Here.sid.h"
#include "SIDs/Odisey_2001_AD.sid.h"
#include "SIDs/When_Im_64.sid.h"

#include "Pics/Ex_Pie2.art.h"
#include "Pics/HA_Bluejay.kla.h"
#include "Pics/HA_Hummingbird.kla.h"
#include "Pics/HA_Sugar_Skull.kla.h"
#include "Pics/KP_Bird.kla.h"
#include "Pics/KP_Burger.kla.h"
#include "Pics/KP_Koala.kla.h"
#include "Pics/KP_Tiger.kla.h"
#include "Pics/LP_Boston.kla.h"
#include "Pics/LP_ELP_Tarkus.kla.h"
#include "Pics/LP_Zappa_Apostrophe.kla.h"
#include "Pics/LP_Zappa_Sleep_Dirt.kla.h"
#include "Pics/T_BlackHole.kla.h"
#include "Pics/T_Ember_Head.kla.h"
#include "Pics/T_Sensorium_Heatmap.kla.h"
#include "Pics/T_TeensyROM.kla.h"


#define TeensyROMMenu_Count 8

extern StructMenuItem dirGames[];
extern StructMenuItem dirTest_Diags[];
extern StructMenuItem dirUtilities[];
extern StructMenuItem dirMultimedia[];
extern StructMenuItem dirSID_Files[];
extern StructMenuItem dirPic_Files[];
extern StructMenuItem dirMIDI_ASID[];
extern StructMenuItem dirTEXT_PETSCII[];
extern StructMenuItem TeensyROMMenu[TeensyROMMenu_Count];

