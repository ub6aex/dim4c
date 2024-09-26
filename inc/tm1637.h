#ifndef _TM1637_H
#define _TM1637_H

#include "stm32f0xx.h"

#define NUM_OF_DIGITS 3 // Number of digits on 7-segment indicator.
#define MAX_DIGITS 6 // Maximum digits tm1637 supports.

#define KEY_MATRIX_NO_KEYS_PRESSED 0b11111111
#define KEY_MATRIX_S1 0b11101111
#define KEY_MATRIX_S2 0b11110111
#define KEY_MATRIX_S3 0b11010111
#define KEY_MATRIX_S4 0b11001111
#define KEY_MATRIX_S5 0b01101111
#define KEY_MATRIX_S6 0b01110111 
#define KEY_MATRIX_S7 0b01010111
#define KEY_MATRIX_S8 0b01001111
#define KEY_MATRIX_S9 0b10101111
#define KEY_MATRIX_S10 0b10110111
#define KEY_MATRIX_S11 0b10010111
#define KEY_MATRIX_S12 0b10001111
#define KEY_MATRIX_S13 0b00101111
#define KEY_MATRIX_S14 0b00110111
#define KEY_MATRIX_S15 0b00010111
#define KEY_MATRIX_S16 0b00001111

void TM1637_init(void);
void TM1637_setBrightness(uint8_t brightness);
void TM1637_displayDecimal(uint16_t value, uint8_t displaySeparator);
uint8_t TM1637_readInputs(void);

#endif
