#include "tm1637.h"
#include "stm32f0xx.h"
#include "tim.h"

const char segmentMap[] = {
    0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, // 0-7
    0x7f, 0x6f, 0x77, 0x7c, 0x39, 0x5e, 0x79, 0x71, // 8-9, A-F
    0x00
};

void _setClkHigh(void) {
    GPIOA->BSRR = GPIO_BSRR_BS_0;
}

void _setClkLow(void) {
    GPIOA->BSRR = GPIO_BSRR_BR_0;
}

void _setDioHigh(void) {
    GPIOA->BSRR = GPIO_BSRR_BS_1;
}

void _setDioLow(void) {
    GPIOA->BSRR = GPIO_BSRR_BR_1;
}

void _setDioOutputMode(void) {
    GPIOA->MODER |= GPIO_MODER_MODER1_0;
    GPIOA->MODER &= ~GPIO_MODER_MODER1_1;
}

void _setDioInputMode(void) {
    GPIOA->MODER &= ~GPIO_MODER_MODER1;
}

void _sendStart(void) {
    _setClkHigh();
    _setDioHigh();
    TIM_delayUs(2);
    _setDioLow();
}

void _sendStop(void) {
    _setClkLow();
    TIM_delayUs(2);
    _setDioLow();
    TIM_delayUs(2);
    _setClkHigh();
    TIM_delayUs(2);
    _setDioHigh();
}

void _readACK(void) {
    _setClkLow();
    TIM_delayUs(5);
    // while (dio); // We're cheating here and not actually reading back the response.
    _setClkHigh();
    TIM_delayUs(2);
    _setClkLow();
}

void _writeByte(unsigned char b) {
    for (int i = 0; i < 8; ++i) {
        _setClkLow();
        if (b & 0x01) {
            _setDioHigh();
        }
        else {
            _setDioLow();
        }
        TIM_delayUs(3);
        b >>= 1;
        _setClkHigh();
        TIM_delayUs(3);
    }
}

void _sendWriteDataCommand(void) {
    _sendStart();
    _writeByte(0x40);
    _readACK();
    _sendStop();
}

void _sendReadKeyScanCommand(void) {
    _sendStart();
    _writeByte(0x42);
    _readACK();
    _sendStop();
}

void _clearIndicator(void) {
    _sendWriteDataCommand();

    // Send display address command to set address C0H
    _sendStart();
    _writeByte(0xc0);
    _readACK();

    // Send data
    for (int i = 0; i < MAX_DIGITS; i++) {
        _writeByte(0);
        _readACK();
    }

    _sendStop();
}

int _pow(int base, int power) {
    int res = base;
    for (int i = 1; i < power; i++) {
        res = res * res;
    }
    return res;
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
    _setDioOutputMode();
    GPIOA->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR1;
    GPIOA->PUPDR &= ~GPIO_PUPDR_PUPDR1;

    _clearIndicator();
    TM1637_setBrightness(8);
}

// Valid brightness values: 0 - 8.
// 0 = display off.
void TM1637_setBrightness(uint8_t brightness) {
    // Brightness command:
    // 1000 0XXX = display off
    // 1000 1BBB = display on, brightness 0-7
    // X = don't care
    // B = brightness
    _sendStart();
    _writeByte(0x87 + brightness);
    _readACK();
    _sendStop();
}

void TM1637_displayDecimal(uint16_t value, uint8_t displaySeparator) {
    unsigned char digitArr[NUM_OF_DIGITS];
    uint32_t v = value;
    for (int i = 0; i < NUM_OF_DIGITS; i++) {
        digitArr[i] = segmentMap[v % 10];
        if (i == 2 && displaySeparator) {
            digitArr[i] |= 1 << 7;
        }
        v /= 10;
    }

    // Remove trailing zeros
    for (int i = 1; i < NUM_OF_DIGITS; i++) {
        if(value < _pow(10,i)) {
            digitArr[i] = segmentMap[16];
        }
    }

    _sendWriteDataCommand();

    // Send display address command to set address C0H
    _sendStart();
    _writeByte(0xc0);
    _readACK();

    // Send data
    for (int i = 1; i <= NUM_OF_DIGITS; i++) {
        _writeByte(digitArr[NUM_OF_DIGITS - i]);
        _readACK();
    }
    _sendStop();
}

// Read keys matrix
uint8_t TM1637_readInputs(void) {
    _sendStart();
    _writeByte(0x42);
    _readACK();

    uint8_t keys = 0;
    _setDioInputMode();
    for (int i = 0; i < 8; i++) {
        keys <<= 1;
        _setClkLow();
        TIM_delayUs(30);
        if ((GPIOA->IDR & GPIO_IDR_1)) {
            keys++;
        }
        _setClkHigh();
        TIM_delayUs(30);
    }

    _setDioOutputMode();
    _readACK();
    _sendStop();
    return keys;
}
