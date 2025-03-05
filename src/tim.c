#include "stm32f0xx.h"
#include "tim.h"
#include "gpio.h"
#include "usart.h"
#include "flash.h"

#define SYSTEMCOREFREQ 48000000U // System Clock Frequency
#define TIM16FREQ 10000 // 10kHz
uint32_t tim14_count = 0;
uint8_t* dmxBuf;

void TIM_dmxTimeoutCounterReset(void) {
    TIM16->CNT = 0;
}

void TIM_setDmxTimeout(uint8_t seconds) {
    if (!seconds) {
        TIM16->CR1 &= ~TIM_CR1_CEN; // stop timer
        TIM16->DIER &= ~TIM_DIER_UIE; // disable interrupt
        NVIC_DisableIRQ(TIM16_IRQn);
    }
    else {
        TIM16->ARR = seconds*10000 - 1; // Auto-Reload Register
        TIM16->DIER |= TIM_DIER_UIE; // enable interrupt
        NVIC_EnableIRQ(TIM16_IRQn);
        TIM16->CR1 |= TIM_CR1_CEN; // start timer
    }
}

void TIM_init(void) {
    // TIM3 is used for delays functions
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN; // TIM3 clock enable
    TIM3->PSC = 24000-1; // prescaler for 500us delay
    TIM3->CR1 |= TIM_CR1_OPM; // one pulse mode

    // TIM14 is used for output LEDs blinking
    RCC->APB1ENR |= RCC_APB1ENR_TIM14EN; // TIM14 clock enable
    TIM14->PSC = 16000; // prescaler
    TIM14->ARR = 4;
    TIM14->CNT = 0;
    TIM14->DIER |= TIM_DIER_UIE; // enable interrupt
    NVIC_EnableIRQ(TIM14_IRQn);
    TIM14->CR1 |= TIM_CR1_CEN; // start timer

    // TIM16 is used for output disabling on DMX receiver timeout
    RCC->APB2ENR |= RCC_APB2ENR_TIM16EN; // TIM16 clock enable
    TIM16->PSC = (SYSTEMCOREFREQ/TIM16FREQ) - 1; // prescaler
    TIM_dmxTimeoutCounterReset();
    TIM_setDmxTimeout(FLASH_getUserConfig(USER_CONFIG_DMX_DISABLE_TIMEOUT));
}

void TIM_delayMs(uint16_t value) {
    TIM3->ARR = (value-1)*2;
    TIM3->CNT = 0;
    TIM3->CR1 |= TIM_CR1_CEN; // start timer
    while((TIM3->SR & TIM_SR_UIF) == 0) {} // wait
    TIM3->SR &= ~TIM_SR_UIF; // reset flag
}

void TIM_delayUs(uint32_t micros) {
    micros *= (SYSTEMCOREFREQ / 1000000) / 10;
    while (micros--);
}

// Outputs LEDs blinking
void TIM14_IRQHandler(void) {
    dmxBuf = USART1_getDmxBuffer();

    for (uint8_t n=0; n<DMX_CHANNELS_NUM; n++) {
        if (dmxBuf[n] == 0) {
            GPIO_outLedOff(n+1);
        } else {
            uint8_t period = (dmxBuf[n] ^ 0xFF)/2 + 10;
            if ((tim14_count/period)%2 != 0)
                GPIO_outLedOn(n+1);
            else
                GPIO_outLedOff(n+1);
        }
    }

    tim14_count++;
    TIM14->SR &= ~TIM_SR_UIF;
}

// DMX512 timout
void TIM16_IRQHandler(void) {
    dmxBuf = USART1_getDmxBuffer();
    for (uint8_t n=0; n<DMX_CHANNELS_NUM; n++)
        dmxBuf[n] = 0;

    TIM16->SR &= ~TIM_SR_UIF;
}
