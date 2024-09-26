#ifndef _USART_H
#define _USART_H

#define DMX_START_CODE 0 // DMX512 start code to react to (0 is for dimmers)
#define DMX_CHANNELS_NUM 4 // maximum number of channels supported by hardware
#define DMX_ADDRESS_MIN 0
#define DMX_ADDRESS_MAX 255

void USART1_init();
void USART1_sendByte(unsigned char ucSend_Data);
void USART1_sendString(char *pucString);
void USART1_sendUInt(uint32_t number);
// uint8_t* getDmxBuferPtr(void);
void USART1_incDmxAddress(void);
void USART1_decDmxAddress(void);
void USART1_inc10DmxAddress(void);
void USART1_dec10DmxAddress(void);

#endif
