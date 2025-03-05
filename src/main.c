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
#define KEY_ACTION_DELAY_MS 200
#define KEY_ACTION_REPEAT_DELAY_MS 200
#define KEY_BOTH_ACTION_DELAY_COUNT 15
#define DOT_POSITION 1

#define DEBUG_OFF 0
#define DEBUG_BY_KEYS 1
#define DEBUG_BY_INPUT 2

uint8_t debugMode; // Debug mode forces all outputs to max values ignoring DMX input
bool addressIncDecLockMode; // DMX address inc/dec actions disable mode

void _setAddressIncDecLockMode(bool mode) {
    addressIncDecLockMode = mode;
    if (mode)
        TM1637_setDotPosition(0);
    else
        TM1637_setDotPosition(DOT_POSITION);
}

void _setDebugMode(uint8_t mode) {
    debugMode = mode;
    if (mode)
        _setAddressIncDecLockMode(!!mode);
    USART1_setDebugMode(!!mode);
}

void _processKeys(void) {
    uint8_t keys = TM1637_readKeys();

    if ((keys == KEY_INC) || (keys == KEY_DEC)) {
        if (addressIncDecLockMode) {
            // debug mode on/off
            const uint8_t keysState = keys;
            TIM_delayMs(KEY_ACTION_DELAY_MS);
            keys = TM1637_readKeys();
            if (keys == keysState) {
                if ((keys == KEY_DEC) && (debugMode == DEBUG_OFF))
                    _setDebugMode(DEBUG_BY_KEYS);
                if ((keys == KEY_INC) && (debugMode == DEBUG_BY_KEYS))
                    _setDebugMode(DEBUG_OFF);
            }
        } else {
            // DMX address increment/decrement
            uint8_t actionsCount = 0;
            const uint8_t keysState = keys;
            do {
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
                TIM_delayMs(KEY_ACTION_REPEAT_DELAY_MS);
                keys = TM1637_readKeys();
                WDG_reset();
            } while (keys == keysState);
        }
    }

    // DMX address inc/dec keys lock disable
    if ((keys == KEY_BOTH) && (debugMode == DEBUG_OFF)) {
        uint8_t delayCounter = 0;
        do {
            TIM_delayMs(KEY_ACTION_DELAY_MS);
            keys = TM1637_readKeys();
            if (keys == KEY_BOTH) {
                if (delayCounter == KEY_BOTH_ACTION_DELAY_COUNT) {
                    _setAddressIncDecLockMode(false);
                    do {
                        keys = TM1637_readKeys();
                        WDG_reset();
                    } while (keys != KEY_NONE);
                } else
                    delayCounter++;
            }
            WDG_reset();
        } while (keys == KEY_BOTH);
    }
}

void _processInputs(void) {
    bool input1 = GPIO_getInput1State();
    if (input1) { // input is active
        if (debugMode == DEBUG_OFF)
           _setDebugMode(DEBUG_BY_INPUT);
    } else { // input is not active
        if (debugMode == DEBUG_BY_INPUT)
           _setDebugMode(DEBUG_OFF);
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
                    uint8_t value = FLASH_getUserConfig(param);
                    TM1637_updateConfigDisplay(param, value);
                    TIM_delayMs(KEY_ACTION_REPEAT_DELAY_MS*4);
                    for(;;) { // config mode is endless loop (reboot to exit)
                        keys = TM1637_readKeys();
                        if (keys == KEY_INC) {
                            switch (param) {
                                case USER_CONFIG_BRIGHTNESS:
                                    if (value >= USER_CONFIG_BRIGHTNESS_MAX)
                                        value = USER_CONFIG_BRIGHTNESS_MIN;
                                    else
                                        value++;
                                    TM1637_setBrightness(value);
                                    break;
                                case USER_CONFIG_DMX_ADDRESS_OFFSET:
                                    if (value >= USER_CONFIG_DMX_ADDRESS_OFFSET_MAX)
                                        value = USER_CONFIG_DMX_ADDRESS_OFFSET_MIN;
                                    else
                                        value++;
                                    USART1_updateDmxAddressOffset(value);
                                    break;
                                case USER_CONFIG_DMX_DISABLE_TIMEOUT:
                                    if (value >= USER_CONFIG_DMX_DISABLE_TIMEOUT_MAX)
                                        value = USER_CONFIG_DMX_DISABLE_TIMEOUT_MIN;
                                    else
                                        value++;
                                    TIM_setDmxTimeout(value);
                                    break;
                            }
                            while (!FLASH_setUserConfig(param, value));
                            TM1637_updateConfigDisplay(param, value);
                            TIM_delayMs(KEY_ACTION_REPEAT_DELAY_MS*2);
                        }
                        if (keys == KEY_DEC) {
                            if (param == USER_CONFIG_LENGTH-1)
                                param = USER_CONFIG_BRIGHTNESS;
                            else
                                param++;
                            value = FLASH_getUserConfig(param);
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
    _setDebugMode(DEBUG_OFF);
    _setAddressIncDecLockMode(true); // DMX address inc/dec actions are disabled after boot
    _enterConfigModeIfRequired();

    for(;;) {
        _processKeys();
        _processInputs();
        WDG_reset();
    }
}
