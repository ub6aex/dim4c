#include "stm32f0xx.h"
#include "usart.h"
#include "tm1637.h"
#include "flash.h"
#include <stdio.h>
#include "gpio.h"
#include "pca9685.h"

#define DMX_START_CODE 0 // DMX512 start code to react to (0 is for dimmers)
#define DMX_CHANNELS_NUM 4 // maximum number of channels supported by hardware
#define DMX_ADDRESS_MIN 0
#define DMX_ADDRESS_MAX 255

uint16_t dmxAddress; // DMX512 base address
uint8_t dmxBuffer[DMX_CHANNELS_NUM];
uint16_t dmxFrameNum;
enum DMX_STATES {
    IDLE,
    BREAK,
    DATA_RECEIVE
};
enum DMX_STATES dmxState; // current DMX reception state

void _USART1_setDmxAddress(uint16_t addr) {
    if (addr > DMX_ADDRESS_MAX) {
        addr = DMX_ADDRESS_MIN;
    }

    dmxAddress = addr;

    // Write DMX address to flash memory
    FLASH_writeOne(addr);

    // Update indicator data to display actual DMX address
    TM1637_displayDecimal(addr, 0);
}

uint16_t _USART1_getDmxAddress(void) {
    return dmxAddress;
}

void USART1_init() {
    //PINs init
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN; // port A enable

    // RX
    GPIOA->MODER &= ~GPIO_MODER_MODER3_0;
    GPIOA->MODER |= GPIO_MODER_MODER3_1;

    // TX
    GPIOA->MODER &= ~GPIO_MODER_MODER2_0;
    GPIOA->MODER |= GPIO_MODER_MODER2_1;

    // AF1 for USART1 signals
    GPIOA->AFR[0] = (GPIOA->AFR[0] &~ (GPIO_AFRL_AFRL2 | GPIO_AFRL_AFRL3)) | (1 << (2 * 4)) | (1 << (3 * 4));

    // HSI clock as USART1 clock
    RCC->CFGR3 |= RCC_CFGR3_USART1SW;

    // Enable the peripheral clock USART1
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;

    // baudrate 250kbs
    USART1->BRR = 8*1000000/250000;

    // 2 stop bits
    USART1->CR2 |= USART_CR2_STOP_1;

    // enable USART1
    // USART1->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;
    USART1->CR1 = USART_CR1_RE | USART_CR1_UE;

    dmxState = IDLE;

    // Read DMX address from flash memory and set
    _USART1_setDmxAddress(FLASH_readOne());

    // Clean buffer
    for (uint8_t i = 0; i < DMX_CHANNELS_NUM; i++) dmxBuffer[i] = 0;

    // receive interrupt inable
    USART1->CR1 |= USART_CR1_RXNEIE;
    NVIC_EnableIRQ(USART1_IRQn);
}

void USART1_sendByte(unsigned char ucSend_Data) {
    USART1->TDR = ucSend_Data & 0x01FF;
    while ((USART1->ISR & USART_ISR_TC) != USART_ISR_TC);
}

void USART1_sendString(char *pucString) {
    while(*pucString!='\0') {
        USART1_sendByte(*pucString);
        pucString++;
    }
}

void USART1_sendUInt(uint32_t number) {
    char str[8] = "";
    sprintf(str, "%lu", number);
    USART1_sendString(str);
}

void USART1_incDmxAddress() {
    _USART1_setDmxAddress(_USART1_getDmxAddress() + 1);
}

void USART1_decDmxAddress() {
    _USART1_setDmxAddress(_USART1_getDmxAddress() - 1);
}

void USART1_inc10DmxAddress() {
    _USART1_setDmxAddress(_USART1_getDmxAddress() + 10);
}

void USART1_dec10DmxAddress() {
    _USART1_setDmxAddress(_USART1_getDmxAddress() - 10);
}

void _USART1_updatePCA9685Outputs() {
    PCA9685_setOutputs(dmxBuffer, DMX_CHANNELS_NUM);
}

// DMX 512 receiver
void USART1_IRQHandler()
{
    USART1->CR1 &= ~USART_CR1_RXNEIE; // receive interrupt disable 

    uint8_t overrunError = USART1->ISR & USART_ISR_ORE;
    if (overrunError) {
        dmxState = IDLE;
        USART1->ICR |= USART_ICR_ORECF; // clear ORE
    } else if (USART1->ISR & USART_ISR_RXNE) {
        uint8_t byte = (uint8_t)(USART1->RDR); // catch data from RDR register to enable reception of the next byte
        uint8_t framingError = USART1->ISR & USART_ISR_FE;
        if (framingError) {
            if (byte == 0) { // it's a BREAK
                dmxState = BREAK; // valid BREAK, ready for reception of Start Code)
            } else {
                dmxState = IDLE; // error: bits set during BREAK - set inactive until next BREAK
            }
            USART1->ICR |= USART_ICR_FECF; // clear FE
        } else { // valid byte detected
            if (dmxState == BREAK) {
                if (byte == DMX_START_CODE) { // valid Start Code detected
                    dmxState = DATA_RECEIVE;
                    dmxFrameNum = 1;
                } else {
                    dmxState = IDLE; // not addressed to us - wait for BREAK
                }
            } else if (dmxState == DATA_RECEIVE) {
                GPIO_statusLedOn();
                if ((dmxFrameNum >= _USART1_getDmxAddress()) && (dmxFrameNum < (_USART1_getDmxAddress() + DMX_CHANNELS_NUM))) { // addressed to us
                    uint8_t n = dmxFrameNum - _USART1_getDmxAddress();
                    if (n < DMX_CHANNELS_NUM) {
                        dmxBuffer[n] = byte;
                    }
                }
                dmxFrameNum++;
                GPIO_statusLedOff();
                if (dmxFrameNum == (_USART1_getDmxAddress() + DMX_CHANNELS_NUM)) {
                    _USART1_updatePCA9685Outputs();
                }
            }
        }
    }

    USART1->CR1 |= USART_CR1_RXNEIE; // receive interrupt enable 
}
