#ifndef _I2C_H
#define _I2C_H

void I2C1_init(uint32_t speed);
void I2C1_writeBytes(uint8_t deviceAddr, uint8_t registerAddr, uint8_t *sendBuf, uint8_t bytesCount);
void I2C1_readBytes(uint8_t deviceAddr, uint8_t registerAddr, uint8_t *readBuf, uint8_t bytesCount);

#endif
