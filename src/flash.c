#include "stm32f0xx.h"
#include "flash.h"

#define FLASH_KEY1 ((uint32_t)0x45670123)
#define FLASH_KEY2 ((uint32_t)0xCDEF89AB)
#define STM32F0xx_PAGE_SIZE 0x400
#define STM32F0xx_FLASH_PAGE0_STARTADDR 0x8000000
#define STM32F0xx_FLASH_PAGE15_STARTADDR (STM32F0xx_FLASH_PAGE0_STARTADDR+15*STM32F0xx_PAGE_SIZE)
#define PAGE15_ADDR STM32F0xx_FLASH_PAGE15_STARTADDR // 0x08003C00

void _FLASH_unlock(void) {
    while ((FLASH->SR & FLASH_SR_BSY) != 0); // Wait till no operation is on going
    if ((FLASH->CR & FLASH_CR_LOCK) != 0) { // Check that the Flash is unlocked
        FLASH->KEYR = FLASH_FKEY1; // Perform unlock sequence
        FLASH->KEYR = FLASH_FKEY2;
    }
}

void _FLASH_lock(void) {
    FLASH->CR |= FLASH_CR_LOCK;
}

uint8_t _FLASH_ready(void) {
    return !(FLASH->SR & FLASH_SR_BSY); // Wait until the BSY bit is reset in the FLASH_SR register
}

uint8_t _FLASH_checkEOP(void) {
    if(FLASH->SR & FLASH_SR_EOP) { // Check the 'End of operation' flag
        FLASH->SR |= FLASH_SR_EOP; //  Clear 'End of operation' flag by software
        return 1;
    }
    return 0;
}

uint32_t _FLASH_read(uint32_t flash_addr) {
    return (*(__IO uint32_t*)flash_addr);
}

uint8_t _FLASH_erasePage(uint32_t page_addr) {
    while(!_FLASH_ready()); // wait

    FLASH->CR|= FLASH_CR_PER; // enable page erasing
    FLASH->AR = page_addr; // set page address
    FLASH->CR|= FLASH_CR_STRT; // start the erasing
    while(!_FLASH_ready());  // wait for page erase complete
    FLASH->CR &= ~FLASH_CR_PER; // disable the page erase

    return _FLASH_checkEOP(); // complete, clear flag
}

uint8_t _FLASH_write(uint32_t flash_addr, uint32_t data) {
    while(!_FLASH_ready());
    
    FLASH->CR |= FLASH_CR_PG; // Set the PG bit in the FLASH_CR register to enable programming
    *(__IO uint16_t*)flash_addr = (uint16_t)data; // write two L bytes
    while(!_FLASH_ready()); // wait
    if(!_FLASH_checkEOP())
        return 0;

    flash_addr+=2;
    data>>=16;
    *(__IO uint16_t*)flash_addr = (uint16_t)data; // write two H bytes
    while(!_FLASH_ready()); // wait
    FLASH->CR &= ~(FLASH_CR_PG); // Reset the PG Bit to disable programming

    return _FLASH_checkEOP();
}

uint32_t FLASH_readOne(void) {
    return _FLASH_read(PAGE15_ADDR);
}

uint8_t FLASH_writeOne(uint32_t data) {
    _FLASH_unlock();
    _FLASH_erasePage(PAGE15_ADDR);
    const uint8_t err = _FLASH_write(PAGE15_ADDR, data);
    _FLASH_lock();
    return err;
}
