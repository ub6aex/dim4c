#include "tm1637.h"
#include "stm32f0xx.h"
#include "tim.h"
#include "flash.h"

#define NUM_OF_DIGITS 3 // Number of digits on 7-segment indicator.
#define MAX_DIGITS 6 // Maximum digits tm1637 supports.

const uint8_t segmentMap[] = {
    0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, // 0-7
    0x7f, 0x6f, 0x77, 0x7c, 0x39, 0x5e, 0x79, 0x71, // 8-9, A-F
    0x00
};

uint16_t displayValue; // current value displayed
uint8_t dotPosition = 0;

void _TM1637_setClkHigh(void) {
    GPIOA->BSRR = GPIO_BSRR_BS_0;
}

void _TM1637_setClkLow(void) {
    GPIOA->BSRR = GPIO_BSRR_BR_0;
}

void _TM1637_setDioHigh(void) {
    GPIOA->BSRR = GPIO_BSRR_BS_1;
}

void _TM1637_setDioLow(void) {
    GPIOA->BSRR = GPIO_BSRR_BR_1;
}

void _TM1637_setDioOutputMode(void) {
    GPIOA->MODER |= GPIO_MODER_MODER1_0;
    GPIOA->MODER &= ~GPIO_MODER_MODER1_1;
}

void _TM1637_setDioInputMode(void) {
    GPIOA->MODER &= ~GPIO_MODER_MODER1;
}

void _TM1637_sendStart(void) {
    _TM1637_setClkHigh();
    _TM1637_setDioHigh();
    TIM_delayUs(2);
    _TM1637_setDioLow();
}

void _TM1637_sendStop(void) {
    _TM1637_setClkLow();
    TIM_delayUs(2);
    _TM1637_setDioLow();
    TIM_delayUs(2);
    _TM1637_setClkHigh();
    TIM_delayUs(2);
    _TM1637_setDioHigh();
}

void _TM1637_readACK(void) {
    _TM1637_setClkLow();
    TIM_delayUs(5);
    // while (dio); // We're cheating here and not actually reading back the response.
    _TM1637_setClkHigh();
    TIM_delayUs(2);
    _TM1637_setClkLow();
}

void _TM1637_writeByte(unsigned char b) {
    for (uint8_t i=0; i<8; ++i) {
        _TM1637_setClkLow();
        if (b & 0x01)
            _TM1637_setDioHigh();
        else
            _TM1637_setDioLow();
        TIM_delayUs(3);
        b >>= 1;
        _TM1637_setClkHigh();
        TIM_delayUs(3);
    }
}

void _TM1637_sendWriteDataCommand(void) {
    _TM1637_sendStart();
    _TM1637_writeByte(0x40);
    _TM1637_readACK();
    _TM1637_sendStop();
}

void _TM1637_sendReadKeyScanCommand(void) {
    _TM1637_sendStart();
    _TM1637_writeByte(0x42);
    _TM1637_readACK();
    _TM1637_sendStop();
}

void _TM1637_clearIndicator(void) {
    _TM1637_sendWriteDataCommand();

    // Send display address command to set address C0h
    _TM1637_sendStart();
    _TM1637_writeByte(0xc0);
    _TM1637_readACK();

    // Send data
    for (uint8_t i=0; i<MAX_DIGITS; i++) {
        _TM1637_writeByte(0);
        _TM1637_readACK();
    }

    _TM1637_sendStop();
}

uint32_t _TM1637_pow(uint32_t base, uint32_t power) {
    uint32_t res = base;
    for (uint32_t i=1; i<power; i++)
        res = res*res;
    return res;
}

void _TM1637_writeData(uint8_t *data, uint8_t length) {
    _TM1637_sendWriteDataCommand();

    // Send display address command to set address C0h
    _TM1637_sendStart();
    _TM1637_writeByte(0xc0);
    _TM1637_readACK();

    // Send data
    for (uint8_t i=1; i<=length; i++) {
        _TM1637_writeByte(data[length-i]);
        _TM1637_readACK();
    }
    _TM1637_sendStop();
}

/*
 * Dot Position:
 *     0 = do not display
 *     1-n = display dot at position
 */
void _TM1637_displayDecimal(uint16_t value, uint8_t dotPosition) {
    uint8_t digitArr[NUM_OF_DIGITS];
    uint32_t v = value;
    for (uint8_t i=0; i<NUM_OF_DIGITS; i++) {
        digitArr[i] = segmentMap[v % 10];
        if (dotPosition)
            digitArr[dotPosition-1] |= 1 << 7;
        v /= 10;
    }

    // Remove trailing zeros
    for (uint8_t i=1; i<NUM_OF_DIGITS; i++) {
        if(value < _TM1637_pow(10,i))
            digitArr[i] = segmentMap[16];
    }

    _TM1637_writeData(digitArr, NUM_OF_DIGITS);
}

/*
 * Valid brightness values: 0 - 8.
 * 0 = display off.
 */
void TM1637_setBrightness(uint8_t brightness) {
    /*
     * Brightness command:
     * 1000 0XXX = display off
     * 1000 1BBB = display on, brightness 0-7
     * X = don't care
     * B = brightness
     */
    _TM1637_sendStart();
    _TM1637_writeByte(0x87 + brightness);
    _TM1637_readACK();
    _TM1637_sendStop();
}

void TM1637_init(void) {
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;

    // CLK
    // GPIO A0 output open-drain
    GPIOA->MODER |= GPIO_MODER_MODER0_0;
    GPIOA->MODER &= ~GPIO_MODER_MODER0_1;
    GPIOA->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR0;
    GPIOA->PUPDR &= ~GPIO_PUPDR_PUPDR0;

    // DIO
    // GPIO A1 output open-drain
    _TM1637_setDioOutputMode();
    GPIOA->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR1;
    GPIOA->PUPDR &= ~GPIO_PUPDR_PUPDR1;

    TIM_delayMs(10);
    _TM1637_clearIndicator();
    TIM_delayMs(10);
    TM1637_setBrightness(FLASH_getUserConfig(USER_CONFIG_BRIGHTNESS));
    TIM_delayMs(10);
}

// Read keys matrix
uint8_t TM1637_readKeys(void) {
    _TM1637_sendStart();
    _TM1637_writeByte(0x42);
    _TM1637_readACK();

    uint8_t keys = 0;
    _TM1637_setDioInputMode();
    for (uint8_t i = 0; i < 8; i++) {
        keys <<= 1;
        _TM1637_setClkLow();
        TIM_delayUs(30);
        if ((GPIOA->IDR & GPIO_IDR_1))
            keys++;
        _TM1637_setClkHigh();
        TIM_delayUs(30);
    }

    _TM1637_setDioOutputMode();
    _TM1637_readACK();
    _TM1637_sendStop();
    return keys;
}

void TM1637_updateDisplay(uint16_t value) {
    displayValue = value;
    _TM1637_displayDecimal(displayValue, dotPosition);
}

void TM1637_setDotPosition(uint8_t pos) {
    dotPosition = pos;
    _TM1637_displayDecimal(displayValue, dotPosition);
}

void TM1637_updateConfigDisplay(uint8_t param, uint8_t value) {
    uint8_t digitArr[3];
    digitArr[0] = segmentMap[value % 10];
    digitArr[1] = segmentMap[16];
    digitArr[2] = segmentMap[param % 10];
    digitArr[2] |= 1 << 7; // add dot to param

    _TM1637_writeData(digitArr, 3);
}
