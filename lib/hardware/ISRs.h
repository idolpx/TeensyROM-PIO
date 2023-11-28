
#ifndef ISRS_H
#define ISRS_H

#include <Arduino.h>

volatile uint8_t BtnPressed = false;

FASTRUN void isrButton();

//Phi2 rising edge:
FASTRUN void isrPHI2();

#endif // ISRS_H