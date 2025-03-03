#ifndef _TIM_H
#define _TIM_H

void TIM_dmxTimeoutCounterReset(void);
void TIM_setDmxTimeout(uint8_t seconds);
void TIM_init(void);
void TIM_delayMs(uint16_t value);
void TIM_delayUs(uint32_t micros);

#endif
