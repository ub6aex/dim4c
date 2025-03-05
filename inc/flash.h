#ifndef _FLASH_H
#define _FLASH_H

#include <stdbool.h>

// User config parameters
#define USER_CONFIG_DMX_ADDRESS 0
#define USER_CONFIG_BRIGHTNESS 1
#define USER_CONFIG_DMX_ADDRESS_OFFSET 2
#define USER_CONFIG_DMX_DISABLE_TIMEOUT 3
#define USER_CONFIG_LENGTH 4

// User config parameters min/max values
#define USER_CONFIG_BRIGHTNESS_MIN 1
#define USER_CONFIG_BRIGHTNESS_MAX 8
#define USER_CONFIG_DMX_ADDRESS_OFFSET_MIN 0
#define USER_CONFIG_DMX_ADDRESS_OFFSET_MAX 1
#define USER_CONFIG_DMX_DISABLE_TIMEOUT_MIN 0
#define USER_CONFIG_DMX_DISABLE_TIMEOUT_MAX 5

// User config parameters default values
#define USER_CONFIG_DMX_ADDRESS_DEFAULT 1
#define USER_CONFIG_BRIGHTNESS_DEFAULT 4
#define USER_CONFIG_DMX_ADDRESS_OFFSET_DEFAULT 1
#define USER_CONFIG_DMX_DISABLE_TIMEOUT_DEFAULT 0

uint32_t FLASH_getUserConfig(uint16_t parameter);
bool FLASH_setUserConfig(uint16_t parameter, uint32_t value);

#endif
