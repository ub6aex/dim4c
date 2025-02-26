#include "stm32f0xx.h"
#include "gpio.h"

void _GPIO_out1LedOn(void) {
    GPIOA->BSRR = GPIO_BSRR_BR_5;
}

void _GPIO_out1LedOff(void) {
    GPIOA->BSRR = GPIO_BSRR_BS_5;
}

void _GPIO_out2LedOn(void) {
    GPIOA->BSRR = GPIO_BSRR_BR_6;
}

void _GPIO_out2LedOff(void) {
    GPIOA->BSRR = GPIO_BSRR_BS_6;
}

void _GPIO_out3LedOn(void) {
    GPIOA->BSRR = GPIO_BSRR_BR_7;
}

void _GPIO_out3LedOff(void) {
    GPIOA->BSRR = GPIO_BSRR_BS_7;
}

void _GPIO_out4LedOn(void) {
    GPIOB->BSRR = GPIO_BSRR_BR_1;
}

void _GPIO_out4LedOff(void) {
    GPIOB->BSRR = GPIO_BSRR_BS_1;
}

void GPIO_init(void) {
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN; // enable port A
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN; // enable port B

    /*
     * Status LED is connected to port A4.
     * Set GPIO A4 as general purpose output.
     */
    GPIOA->MODER |= GPIO_MODER_MODER4_0;
    GPIOA->MODER &= ~GPIO_MODER_MODER4_1;
    GPIO_statusLedOff();

    /*
     * Outout LEDs are connected to ports A5, A6, A7, PB1
     * Set the GPIOs as general purpose outputs.
     */
    GPIOA->MODER |= GPIO_MODER_MODER5_0;
    GPIOA->MODER &= ~GPIO_MODER_MODER5_1;
    _GPIO_out1LedOff();
    GPIOA->MODER |= GPIO_MODER_MODER6_0;
    GPIOA->MODER &= ~GPIO_MODER_MODER6_1;
    _GPIO_out2LedOff();
    GPIOA->MODER |= GPIO_MODER_MODER7_0;
    GPIOA->MODER &= ~GPIO_MODER_MODER7_1;
    _GPIO_out3LedOff();
    GPIOB->MODER |= GPIO_MODER_MODER1_0;
    GPIOB->MODER &= ~GPIO_MODER_MODER1_1;
    _GPIO_out4LedOff();

    // PA2 input
    GPIOA->MODER &= ~GPIO_MODER_MODER2; // input
    GPIOA->PUPDR &= ~GPIO_PUPDR_PUPDR2; // No pull-up and no pull-down
    GPIOA->OSPEEDR |= GPIO_OSPEEDR_OSPEEDR2; // max speed
}

void GPIO_statusLedOn(void) {
    GPIOA->BSRR = GPIO_BSRR_BR_4;
}

void GPIO_statusLedOff(void) {
    GPIOA->BSRR = GPIO_BSRR_BS_4;
}

void GPIO_outLedOn(uint8_t num) {
    switch (num) {
    case 1:
        _GPIO_out1LedOn();
        break;
    case 2:
        _GPIO_out2LedOn();
        break;
    case 3:
        _GPIO_out3LedOn();
        break;
    case 4:
        _GPIO_out4LedOn();
        break;
    default:
        break;
    }
}

void GPIO_outLedOff(uint8_t num) {
    switch (num) {
    case 1:
        _GPIO_out1LedOff();
        break;
    case 2:
        _GPIO_out2LedOff();
        break;
    case 3:
        _GPIO_out3LedOff();
        break;
    case 4:
        _GPIO_out4LedOff();
        break;
    default:
        break;
    }
}

uint8_t GPIO_input1State(void) {
    return !(GPIOA->IDR & GPIO_IDR_2);
}
