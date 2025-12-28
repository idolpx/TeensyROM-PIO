#include "Common_Defs.h"

// Global variable definitions
char strVersionNumber[] = "v0.6.7+9.pio"; //*VERSION*

bool (*fBusSnoop)(uint16_t Address, bool R_Wn) = NULL;

uint16_t BigBufCount = 0;
uint32_t* BigBuf = NULL;

uint8_t RAM_Image[RAM_ImageSize]; // Main RAM1 file storage buffer

uint8_t *XferImage = NULL; //pointer to image being transfered to C64
uint32_t XferSize = 0;  //size of image being transfered to C64

volatile uint32_t StartCycCnt, LastCycCnt = 0;

uint32_t nS_MaxAdj    = Def_nS_MaxAdj;
uint32_t nS_RWnReady  = Def_nS_RWnReady;
uint32_t nS_PLAprop   = Def_nS_PLAprop;
uint32_t nS_DataSetup = Def_nS_DataSetup;
uint32_t nS_DataHold  = Def_nS_DataHold;
uint32_t nS_VICStart  = Def_nS_VICStart;
uint32_t nS_VICDHold  = Def_nS_VICDHold;

volatile uint8_t EmulateVicCycles = false; //set to true to emulate VIC cycles, false to use normal C64 timing
