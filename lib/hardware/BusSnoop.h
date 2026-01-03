
#ifndef BUS_SNOOP_H_
#define BUS_SNOOP_H_

#include <Arduino.h>
#include <stdlib.h>

extern uint32_t *BusBitCount;
extern volatile uint32_t BusSampleCount;

#define BusSampleMaxSize   100000
#define BusSampleTimeoutmS   2000
#define NumBusBits             25

FLASHMEM void BusAnalysis();
bool BusCount (uint16_t Address, bool R_Wn);

#endif // BUS_SNOOP_H_