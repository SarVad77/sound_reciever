#ifndef E32_CONFIG_H
#define E32_CONFIG_H

#include "e32_lora.h"
#include <stdint.h>

#define E32_UART_BAUD_9600_AIR_24K  0x1A
#define E32_OPTION_FIXED_30DBM      0xC4

HAL_StatusTypeDef e32_write_config_fixed(
    e32_t *e,
    uint16_t local_address,
    uint8_t channel
);

#endif
