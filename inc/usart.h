#ifndef _USART_H
#define _USART_H

void USART1_init();
void USART1_sendByte(unsigned char ucSend_Data);
void USART1_sendString(char *pucString);
void USART1_sendUInt(uint32_t number);
void USART1_incDmxAddress(void);
void USART1_decDmxAddress(void);
void USART1_inc10DmxAddress(void);
void USART1_dec10DmxAddress(void);

#endif
