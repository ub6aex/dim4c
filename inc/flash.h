#ifndef _FLASH_H
#define _FLASH_H

uint8_t FLASH_erasePage(uint32_t address);
uint8_t FLASH_write(uint32_t address,uint32_t data);
uint32_t FLASH_read(uint32_t flash_addr);
uint32_t FLASH_readOne();
uint8_t FLASH_writeOne(uint32_t data);

#endif
