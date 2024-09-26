#include "stm32f0xx.h"
#include "wdg.h"
#include "rcc.h"
#include "gpio.h"
#include "tim.h"
#include "usart.h"
#include "tm1637.h"
#include "i2c.h"

#define KEY_INC KEY_MATRIX_S16
#define KEY_DEC KEY_MATRIX_S15
#define KEY_BOTH (KEY_INC & KEY_DEC)
#define KEY_ANTINOIZE_COUNT 4
#define KEY_ANTINOIZE_DELAY_MS 8
#define KEY_STEPS_TO_SPEEDUP 15
#define KEY_ACTION_DELAY_MS 200

void _processKeys(void) {
    uint8_t keys = TM1637_readInputs();
    if ((keys == KEY_INC) || (keys == KEY_DEC)) {
        int actionsCount = 0;
        uint8_t keyState = keys;
        do {
            for (int i = 0; i <= KEY_ANTINOIZE_COUNT; i++) {
                TIM_delayMs(KEY_ANTINOIZE_DELAY_MS);
                keys = TM1637_readInputs();
                if (keys == keyState) {
                    if (i == KEY_ANTINOIZE_COUNT) {
                        if (actionsCount < KEY_STEPS_TO_SPEEDUP) {
                            if (keyState == KEY_INC) {
                                USART1_incDmxAddress();
                            }
                            if (keyState == KEY_DEC) {
                                USART1_decDmxAddress();
                            }
                            actionsCount++;
                        } else {
                            if (keyState == KEY_INC) {
                                USART1_inc10DmxAddress();
                            }
                            if (keyState == KEY_DEC) {
                                USART1_dec10DmxAddress();
                            }
                        }
                        TIM_delayMs(KEY_ACTION_DELAY_MS);
                    }
                    continue;
                } else {
                    break;
                }
            }
            WDG_reset();
        } while (keys == keyState);
    }
    if (keys == KEY_BOTH) {
    }
}

int main(void) {
    WDG_init(2000);
    RCC_init();
    GPIO_init();
    TIM_init();
    TIM_delayMs(1000);

    TM1637_init();
    TM1637_setBrightness(4);
    USART1_init();
    I2C1_init();

    for(;;) {
        _processKeys();

        // I2C1_WriteBytes(1, dmxBuf, 1);
        // TIM_delayMs(100);
        WDG_reset();
    }
}
