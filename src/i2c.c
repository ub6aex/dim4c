/*
 *https://github.com/EZdenki/STM32F030-CMSIS-I2C-lib
 */

#include "stm32f0xx.h"
#include "tim.h"
#include "i2c.h"

void I2C1_init(uint32_t speed) {
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN; // enable port A

    // SCL
    GPIOA->OTYPER |= GPIO_OTYPER_OT_9; // open drain
    GPIOA->PUPDR &= ~GPIO_PUPDR_PUPDR9; // No pull-up and no pull-down

    // SDA
    GPIOA->OTYPER |= GPIO_OTYPER_OT_10; // open drain
    GPIOA->PUPDR &= ~GPIO_PUPDR_PUPDR10; // No pull-up and no pull-down

    // Select Alternate Function mode (10) on PA9 and PA10
    GPIOA->MODER &= ~GPIO_MODER_MODER9_0;
    GPIOA->MODER |= GPIO_MODER_MODER9_1;
    GPIOA->MODER &= ~GPIO_MODER_MODER10_0;
    GPIOA->MODER |= GPIO_MODER_MODER10_1;

    GPIOA->AFR[1] = (GPIOA->AFR[1] & ~(GPIO_AFRH_AFRH1 | GPIO_AFRH_AFRH2)) | (4 << (1 * 4)) | (4 << (2 * 4)); // Alternate Function 4 for I2C signals

    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN; // Enable the peripheral clock I2C1

    // Reset registers
    I2C1->CR1 &= ~( I2C_CR1_DFN | I2C_CR1_ANFOFF | I2C_CR1_SMBHEN | I2C_CR1_SMBDEN );
    I2C1->CR2 &= ~( I2C_CR2_RD_WRN | I2C_CR2_NACK | I2C_CR2_RELOAD | I2C_CR2_AUTOEND );

    // Ensure that the specified speed is within operable bounds.
    // If not, then default to the high or low limit.
    if(speed > 400E3)
        speed = 400E3;
    else if(speed < 10E3)
        speed = 10E3;

    uint32_t presc;
    uint32_t sclh;
    uint32_t scll;

    if(speed < 50E3) {
        // Calculate lower speeds with the prescaler set to 1
        presc = 1;
        sclh  = (uint32_t)2E6 / speed - 5;
        scll  = sclh + 3;
    } else {
        // Calculte higher speeds with the prescaler set to 0
        presc = 0;
        sclh  = (uint32_t)4E6 / speed - 9;
        scll  = sclh + 5;
    }

    // Supposedly these can add a little speed in some cases, but probably not strictly required.
    uint32_t scldel = 0;
    uint32_t sdadel = 0;

    // Set the I2C timing values into the timing register
    I2C1->TIMINGR |= (presc << 28) |
                     (scldel << 20) |
                     (sdadel << 16) |
                     (sclh << 8) |
                     (scll << 0);

    I2C1->CR1 |= I2C_CR1_PE; // Periph enable
}

// Write the address to the SADD bits of the CR2 register.
void _I2C1_setAddress(uint8_t address) {
    I2C1->CR2 &= ~I2C_CR2_SADD; // Clear Address bits in CR2 register
    I2C1->CR2 |= (address << 1); // Write address to CR2 register
}

// Set the start bit and wait for acknowledge that it was set.
void _I2C1_sendStart() {
    I2C1->CR2 |= I2C_CR2_START; // Set START bit in I2C CR2 register
    while(I2C1->CR2 & I2C_CR2_START); // Wait until START bit is cleared
}

// Set and then clear the stop bit.
void _I2C1_sendStop() {
    I2C1->CR2 |= I2C_CR2_STOP; // Set STOP bit in I2C CR2 register
    while(I2C1->CR2 & I2C_CR2_STOP); // Wait until STOP bit is cleared

    I2C1->ICR |= I2C_ICR_STOPCF; // Clear the STOPF flag in the I2C ISR register
    while(I2C1->ICR & I2C_ICR_STOPCF); // Wait until the STOPF flag is cleared. Note that
                                       // the stop flag is cleared by writing to the
                                       // ICR register but is read from the ISR register.
}

// Set the number of bytes to be transferred.
void _I2C1_setNBytes(uint8_t nBytes) {
    I2C1->CR2 &= ~(I2C_CR2_NBYTES); // Mask out byte-count bits
    I2C1->CR2 |= (nBytes << 16); // Set number of bytes
}

// Set the I2C interface into the read mode.
void _I2C1_setReadMode() {
    I2C1->CR2 |= I2C_CR2_RD_WRN; // Set I2C interface to read operation
}

// Set the I2C interface into the write mode.
void _I2C1_setWriteMode() {
    I2C1->CR2 &= ~I2C_CR2_RD_WRN; // Restore read/write bit to write
}

// Write a byte of data to the I2C interface.
void _I2C1_writeByte(uint8_t data) {
    I2C1->TXDR = (I2C1->TXDR & 0xFFFFFF00) | data;
    /*
     * Wait until both the the TXDR register is empty (TXIS=1) and the transfer-complete
     * flag (TC) is set, indicating the end of the transfer.
     */
    while(!(I2C1->ISR & (I2C_ISR_TXIS | I2C_ISR_TC))) ;
}

// Read a byte from the I2C interface.
uint8_t _I2C1_readByte() {
    while(!(I2C1->ISR & I2C_ISR_RXNE)); // Wait for byte to appear
    uint8_t result = I2C1->RXDR & 0xFF; // Read received byte
    return result;
}

void I2C1_writeBytes(uint8_t deviceAddr, uint8_t registerAddr, uint8_t *sendBuf, uint8_t bytesCount) {
    _I2C1_setAddress(deviceAddr);
    _I2C1_setWriteMode();
    _I2C1_setNBytes(bytesCount + 1);
    _I2C1_sendStart();

    _I2C1_writeByte(registerAddr);
    uint8_t i;
    for(i = 0; i < bytesCount; i++)
        _I2C1_writeByte(sendBuf[i]);
    _I2C1_sendStop();
}

void I2C1_readBytes(uint8_t deviceAddr, uint8_t registerAddr, uint8_t *readBuf, uint8_t bytesCount) {
    _I2C1_setAddress(deviceAddr);

    // send register address
    _I2C1_setNBytes(1);
    _I2C1_setWriteMode();
    _I2C1_sendStart();
    _I2C1_writeByte(registerAddr);

    _I2C1_setNBytes(bytesCount);
    _I2C1_setReadMode();
    _I2C1_sendStart(); // restart

    uint8_t i;
    for(i = 0; i < bytesCount; i++)
        readBuf[i] = _I2C1_readByte();
    _I2C1_sendStop();
    _I2C1_setWriteMode();
}
