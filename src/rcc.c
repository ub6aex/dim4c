#include "stm32f0xx.h"
#include "rcc.h"

void RCC_init(void)
{
    __IO uint32_t StartUpCounter = 0, HSEStatus = 0, HSEStartUp_TimeOut = 0x500, RESET = 1;

    RCC->CR |= ((uint32_t)RCC_CR_HSEON); // enable HSE
    
    // wait for HSE ready flag or timeout
    do {
        HSEStatus = RCC->CR & RCC_CR_HSERDY;
        StartUpCounter++;  
    } while((HSEStatus == 0) && (StartUpCounter != HSEStartUp_TimeOut));

    if ( (RCC->CR & RCC_CR_HSERDY) != RESET)
        HSEStatus = (uint32_t)0x01;
    else
        HSEStatus = (uint32_t)0x00;
 
    // If HSE started ok
    if ( HSEStatus == (uint32_t)0x01) {
        FLASH->ACR |= FLASH_ACR_PRFTBE; // Flash prefetch buffer enable    
        FLASH->ACR |= (uint32_t)FLASH_ACR_LATENCY; // Flash latency
        RCC->CFGR |= RCC_CFGR_PLLSRC; // set PLL source HSE
        RCC->CFGR |= RCC_CFGR_PLLMULL6;
        RCC->CR |= RCC_CR_PLLON; // Enable PLL
        while((RCC->CR & RCC_CR_PLLRDY) == 0) {} // Wait for PLL ready flag

        // Choose PLL as a system clock
        RCC->CFGR &= (uint32_t)((uint32_t)~(RCC_CFGR_SW));
        RCC->CFGR |= (uint32_t)RCC_CFGR_SW_PLL;    

        while ((RCC->CFGR & (uint32_t)RCC_CFGR_SWS) != (uint32_t)0x08) {} // Wait for PLL is system clock
    } else {
        for(;;) {
            // rcc init failed. Wait for WDG reset.
        };
    }
}
