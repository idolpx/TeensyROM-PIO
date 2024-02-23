
#ifndef PINMAP_H
#define PINMAP_H

#include <Arduino.h>

#define PHI2_PIN            1
#define Reset_Btn_In_PIN    31
const uint8_t InputPins[] = {
   19,18,14,15,40,41,17,16,22,23,20,21,38,39,26,27,  //address bus
   2, 3, 4, 5, PHI2_PIN, 0,   // IO1n, IO2n, ROML, ROMH, PHI2_PIN, R_Wn
   28, 29, Reset_Btn_In_PIN,  // DOT clk, BA, Reset button
   };

const uint8_t OutputPins[] = {
   35, 9, 32,   // DataCEn, ExROM, Game
   30, 25, 24,  // DMA, NMI, IRQ
   34,33, 6,    // LED, debug, Reset_Out_PIN,
   };

#endif 