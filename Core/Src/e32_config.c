#include "e32_config.h"

HAL_StatusTypeDef e32_write_config_fixed(
    e32_t *e,
    uint16_t local_address,
    uint8_t channel
)
{
    uint8_t cfg[6];

    cfg[0] = 0xC0;                          // save parameters
    cfg[1] = (local_address >> 8) & 0xFF;   // ADDH
    cfg[2] = local_address & 0xFF;          // ADDL
    cfg[3] = E32_UART_BAUD_9600_AIR_24K;    // 9600 UART, 2.4 kbps air
    cfg[4] = channel;                       // CHAN
    cfg[5] = E32_OPTION_FIXED_30DBM;        // fixed mode, push-pull, FEC, 30dBm

    e32_config(e);

    HAL_StatusTypeDef st =
        HAL_UART_Transmit(e->huart, cfg, 6, 500);

    HAL_Delay(200);

    e32_normal(e);

    return st;
}
