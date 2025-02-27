#include "stm32f0xx.h"
#include "wdg.h"
#include "rcc.h"
#include "gpio.h"
#include "tim.h"
#include "usart.h"
#include "tm1637.h"
#include "i2c.h"
#include "pca9685.h"

#define KEY_INC KEY_MATRIX_S15
#define KEY_DEC KEY_MATRIX_S16
#define KEY_BOTH (KEY_INC & KEY_DEC)
#define KEY_STEPS_TO_SPEEDUP 12
#define KEY_ACTION_DELAY_MS 80
#define KEY_ACTION_REPEAT_DELAY_MS 200
#define KEY_BOTH_ACTION_DELAY_COUNT 30
#define TRUE 1
#define FALSE 0

uint8_t debugMode = FALSE; // Debug mode forces all outputs to max values ignoring DMX input
uint8_t addressIncDecLockMode = TRUE; // DMX address inc/dec actions are disabled after boot

void _processKeys(void) {
    uint8_t keys = TM1637_readKeys();

    // DMX address increment/decrement
    if (!addressIncDecLockMode && ((keys == KEY_INC) || (keys == KEY_DEC))) {
        int actionsCount = 0;
        const uint8_t keysState = keys;
        TIM_delayMs(KEY_ACTION_DELAY_MS);
        do {
            if (keys == keysState) {
                if (actionsCount < KEY_STEPS_TO_SPEEDUP) {
                    if (keysState == KEY_INC) {
                        USART1_incDmxAddress();
                    }
                    if (keysState == KEY_DEC) {
                        USART1_decDmxAddress();
                    }
                    actionsCount++;
                } else {
                    if (keysState == KEY_INC) {
                        USART1_inc10DmxAddress();
                    }
                    if (keysState == KEY_DEC) {
                        USART1_dec10DmxAddress();
                    }
                }
            }
            TIM_delayMs(KEY_ACTION_REPEAT_DELAY_MS);
            keys = TM1637_readKeys();
            WDG_reset();
        } while (keys == keysState);
    }

    // Keys lock off
    if (keys == KEY_BOTH) {
        uint8_t delayCounter = 0;
        do {
            TIM_delayMs(KEY_ACTION_DELAY_MS);
            keys = TM1637_readKeys();
            if (keys == KEY_BOTH) {
                if (delayCounter == KEY_BOTH_ACTION_DELAY_COUNT) {
                    addressIncDecLockMode = FALSE; // enable DMX address inc/dec actions
                    TM1637_setDotPosition(1);
                } else
                    delayCounter++;
            }
            WDG_reset();
        } while (keys == KEY_BOTH);
    }
}

void _processInputs(void) {
    uint8_t input1 = GPIO_input1State();
    if (input1 != 0) { // input is active
        if (!debugMode) {
            debugMode = TRUE;
            USART1_setDebugMode(debugMode);
        }
    } else { // input is not active
        if (debugMode) {
            debugMode = FALSE;
            USART1_setDebugMode(debugMode);
        }
    }
}

int main(void) {
    WDG_init(2000);
    RCC_init();
    GPIO_init();
    TIM_init();
    TIM_delayMs(100);

    TM1637_init();
    TM1637_setBrightness(6);
    I2C1_init(400E3); // 400kHz
    PCA9685_init();
    USART1_init();

    for(;;) {
        _processKeys();
        _processInputs();
        WDG_reset();
    }
}
