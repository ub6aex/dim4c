#ifndef _GPIO_H
#define _GPIO_H

void GPIO_init(void);

void GPIO_statusLedOn(void);
void GPIO_statusLedOff(void);
void GPIO_outLedOn(uint8_t num);
void GPIO_outLedOff(uint8_t num);

uint8_t GPIO_inputState(void);

#endif
