#ifndef _I2C_H
#define _I2C_H

void I2C1_init(void);
void I2C1_writeBytes(uint16_t addr,uint8_t *buf, uint16_t bytes_count);
void I2C1_readBytes(uint16_t addr, uint8_t *buf, uint16_t bytes_count);

#endif
