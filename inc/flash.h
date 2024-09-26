#ifndef _FLASH_H
#define _FLASH_H

#define FLASH_KEY1 ((uint32_t)0x45670123)
#define FLASH_KEY2 ((uint32_t)0xCDEF89AB)

#define STM32F0xx_PAGE_SIZE 0x400
#define STM32F0xx_FLASH_PAGE0_STARTADDR 0x8000000
#define STM32F0xx_FLASH_PAGE15_STARTADDR (STM32F0xx_FLASH_PAGE0_STARTADDR+15*STM32F0xx_PAGE_SIZE)
#define ADDR STM32F0xx_FLASH_PAGE15_STARTADDR // 0x08003C00

uint8_t FLASH_erasePage(uint32_t address);
uint8_t FLASH_write(uint32_t address,uint32_t data);
uint32_t FLASH_read(uint32_t flash_addr);
uint32_t FLASH_readOne();
uint8_t FLASH_writeOne(uint32_t data);

#endif
