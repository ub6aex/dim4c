#ifndef _PCA9685_H
#define _PCA9685_H

void PCA9685_init(void);
void PCA9685_setOutputs(uint8_t *values, uint8_t length);

#endif