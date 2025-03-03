#ifndef _FLASH_H
#define _FLASH_H

#include <stdbool.h>

#define PARAMS_DMX_ADDRESS 0
#define PARAMS_BRIGHTNESS 1
#define PARAMS_DMX_ADDRESS_OFFSET 2
#define PARAMS_DMX_DISABLE_TIMEOUT 3
#define PARAMS_LENGTH 4

#define PARAMS_BRIGHTNESS_MIN 1
#define PARAMS_BRIGHTNESS_MAX 8
#define PARAMS_DMX_ADDRESS_OFFSET_MIN 0
#define PARAMS_DMX_ADDRESS_OFFSET_MAX 1
#define PARAMS_DMX_DISABLE_TIMEOUT_MIN 0
#define PARAMS_DMX_DISABLE_TIMEOUT_MAX 5

uint32_t FLASH_getConfig(uint16_t parameter);
bool FLASH_setConfig(uint16_t parameter, uint32_t value);

#endif
