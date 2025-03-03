#include "stm32f0xx.h"
#include <stdbool.h>
#include "wdg.h"
#include "rcc.h"
#include "gpio.h"
#include "tim.h"
#include "usart.h"
#include "tm1637.h"
#include "i2c.h"
#include "pca9685.h"
#include "flash.h"

#define KEY_INC KEY_MATRIX_S15
#define KEY_DEC KEY_MATRIX_S16
#define KEY_BOTH (KEY_INC & KEY_DEC)
#define KEY_NONE KEY_MATRIX_NO_KEYS_PRESSED
#define KEY_STEPS_TO_SPEEDUP 12
#define KEY_ACTION_DELAY_MS 80
#define KEY_ACTION_REPEAT_DELAY_MS 200
#define KEY_BOTH_ACTION_DELAY_COUNT 30
#define DOT_POSITION 1

bool debugMode; // Debug mode forces all outputs to max values ignoring DMX input
bool addressIncDecLockMode; // DMX address inc/dec actions disable mode

void _setDebugMode(bool mode) {
    debugMode = mode;
    USART1_setDebugMode(mode);
}

void _setAddressIncDecLockMode(bool mode) {
    addressIncDecLockMode = mode;
    if (mode)
        TM1637_setDotPosition(0);
    else
        TM1637_setDotPosition(DOT_POSITION);
}

void _processKeys(void) {
    uint8_t keys = TM1637_readKeys();

    // DMX address increment/decrement
    if (!addressIncDecLockMode && ((keys == KEY_INC) || (keys == KEY_DEC))) {
        uint8_t actionsCount = 0;
        const uint8_t keysState = keys;
        TIM_delayMs(KEY_ACTION_DELAY_MS);
        do {
            if (keys == keysState) {
                if (actionsCount < KEY_STEPS_TO_SPEEDUP) {
                    if (keysState == KEY_INC)
                        USART1_incDmxAddress();
                    if (keysState == KEY_DEC)
                        USART1_decDmxAddress();
                    actionsCount++;
                } else {
                    if (keysState == KEY_INC)
                        USART1_inc10DmxAddress();
                    if (keysState == KEY_DEC)
                        USART1_dec10DmxAddress();
                }
            }
            TIM_delayMs(KEY_ACTION_REPEAT_DELAY_MS);
            keys = TM1637_readKeys();
            WDG_reset();
        } while (keys == keysState);
    }

    // DMX address inc/dec keys lock disable
    if (keys == KEY_BOTH) {
        uint8_t delayCounter = 0;
        do {
            TIM_delayMs(KEY_ACTION_DELAY_MS);
            keys = TM1637_readKeys();
            if (keys == KEY_BOTH) {
                if (delayCounter == KEY_BOTH_ACTION_DELAY_COUNT)
                    _setAddressIncDecLockMode(false);
                else
                    delayCounter++;
            }
            WDG_reset();
        } while (keys == KEY_BOTH);
    }
}

void _processInputs(void) {
    bool input1 = GPIO_getInput1State();
    if (input1) { // input is active
        if (!debugMode)
           _setDebugMode(true);
    } else { // input is not active
        if (debugMode)
           _setDebugMode(false);
    }
}

void _enterConfigModeIfRequired(void) {
    uint8_t keys = TM1637_readKeys();
    if (keys == KEY_BOTH) {
        uint8_t delayCounter = 0;
        do {
            TIM_delayMs(KEY_ACTION_DELAY_MS);
            keys = TM1637_readKeys();
            if (keys == KEY_BOTH) {
                if (delayCounter == KEY_BOTH_ACTION_DELAY_COUNT) {
                    // enter config menu
                    uint8_t param = 1;
                    uint8_t value = FLASH_getConfig(param);
                    TM1637_updateConfigDisplay(param, value);
                    TIM_delayMs(KEY_ACTION_REPEAT_DELAY_MS*4);
                    for(;;) {
                        keys = TM1637_readKeys();
                        if (keys == KEY_INC) {
                            switch (param) {
                                case PARAMS_BRIGHTNESS:
                                    if (value >= PARAMS_BRIGHTNESS_MAX)
                                        value = PARAMS_BRIGHTNESS_MIN;
                                    else
                                        value++;
                                    TM1637_setBrightness(value);
                                    break;
                                case PARAMS_DMX_ADDRESS_OFFSET:
                                    if (value >= PARAMS_DMX_ADDRESS_OFFSET_MAX)
                                        value = PARAMS_DMX_ADDRESS_OFFSET_MIN;
                                    else
                                        value++;
                                    USART1_updateDmxAddressOffset(value);
                                    break;
                                case PARAMS_DMX_DISABLE_TIMEOUT:
                                    if (value >= PARAMS_DMX_DISABLE_TIMEOUT_MAX)
                                        value = PARAMS_DMX_DISABLE_TIMEOUT_MIN;
                                    else
                                        value++;
                                    TIM_setDmxTimeout(value);
                                    break;
                            }
                            while (!FLASH_setConfig(param, value));
                            TM1637_updateConfigDisplay(param, value);
                            TIM_delayMs(KEY_ACTION_REPEAT_DELAY_MS*2);
                        }
                        if (keys == KEY_DEC) {
                            if (param == PARAMS_LENGTH-1)
                                param = PARAMS_BRIGHTNESS;
                            else
                                param++;
                            value = FLASH_getConfig(param);
                            TM1637_updateConfigDisplay(param, value);
                            TIM_delayMs(KEY_ACTION_REPEAT_DELAY_MS*2);
                        }
                        WDG_reset();
                    }
                } else
                    delayCounter++;
            }
            WDG_reset();
        } while (keys == KEY_BOTH);
    }
}

int main(void) {
    WDG_init(2000);
    RCC_init();
    GPIO_init();
    TIM_init();
    TIM_delayMs(100);

    TM1637_init();
    I2C1_init(400E3); // 400kHz
    PCA9685_init();
    USART1_init();
    _setDebugMode(false);
    _setAddressIncDecLockMode(true); // DMX address inc/dec actions are disabled after boot
    _enterConfigModeIfRequired();

    for(;;) {
        _processKeys();
        _processInputs();
        WDG_reset();
    }
}
