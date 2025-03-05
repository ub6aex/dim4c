#include "stm32f0xx.h"
#include "i2c.h"
#include "tim.h"
#include "pca9685.h"

#define PCA9685_ADDRESS 0x40
#define PCA9685_MODE1 0x00
#define PCA9685_MODE2 0x01
#define PCA9685_PRE_SCALE 0xFE
#define PCA9685_ALL_LED_ON_L 0xFA
#define PCA9685_LED0_ON_L 0x06

void PCA9685_init(void) {
    uint8_t mode1 = 0b00110001; // SLEEP, AI, ALLCALL
    I2C1_writeBytes(PCA9685_ADDRESS, PCA9685_MODE1, &mode1, 1);

    uint8_t mode2 = 0b00000100; // OUTDRV
    I2C1_writeBytes(PCA9685_ADDRESS, PCA9685_MODE2, &mode2, 1);

    /*
     * Set PWM prescaler.
     * The maximum PWM frequency is 1526 Hz if the PRE_SCALE register is set "0x03h".
     */
    uint8_t prescaler = 0x03;  
    I2C1_writeBytes(PCA9685_ADDRESS, PCA9685_PRE_SCALE, &prescaler, 1);

    // Turn off all leds
    uint8_t allLedOff[] = {0x00, 0x00, 0x00, 0x10};
    I2C1_writeBytes(PCA9685_ADDRESS, PCA9685_ALL_LED_ON_L, allLedOff, 4);

    // Wakeup
    uint8_t mode1reset = 0b10100001; // RESTART, AI, ALLCALL
    I2C1_writeBytes(PCA9685_ADDRESS, PCA9685_MODE1, &mode1reset, 1);
    TIM_delayMs(5);
}

void PCA9685_setOutputs(uint8_t *values, uint8_t length) {
    uint8_t buf[length*4];

    for (uint8_t i=0; i<length; i++) {
        if (values[i] == 255) {
            buf[i*4] = 0x00;
            buf[i*4+1] = 0x10;
            buf[i*4+2] = 0x00;
            buf[i*4+3] = 0x00;
        } else {
            buf[i*4] = 0x00;
            buf[i*4+1] = 0x00;
            buf[i*4+2] = values[i] << 4;
            buf[i*4+3] = values[i] >> 4;
        }
    }

    I2C1_writeBytes(PCA9685_ADDRESS, PCA9685_LED0_ON_L, buf, length*4);
}
