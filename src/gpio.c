#include "stm32f0xx.h"
#include "gpio.h"

void GPIO_init(void) {
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    GPIOA->MODER |= GPIO_MODER_MODER4_0;
    GPIO_statusLedOff();
}

void GPIO_statusLedOn(void) {
    GPIOA->BSRR = GPIO_BSRR_BR_4;
}

void GPIO_statusLedOff(void) {
    GPIOA->BSRR = GPIO_BSRR_BS_4;
}
