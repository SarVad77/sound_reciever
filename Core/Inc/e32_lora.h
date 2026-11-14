#ifndef E32_LORA_H
#define E32_LORA_H

#include "stm32f4xx_hal.h"
#include <stdint.h>

#define E32_CHANNEL 0x17

typedef struct
{
    UART_HandleTypeDef *huart;

    GPIO_TypeDef *m0_port;
    uint16_t m0_pin;

    GPIO_TypeDef *m1_port;
    uint16_t m1_pin;

    GPIO_TypeDef *aux_port;
    uint16_t aux_pin;

} e32_t;

void e32_normal(e32_t *e);
void e32_config(e32_t *e);
void e32_wait_aux(e32_t *e);
void e32_bypass_aux(int i_bypass_aux, int i_permanent);

HAL_StatusTypeDef e32_send_fixed(
    e32_t *e,
    uint16_t address,
    uint8_t channel,
    uint8_t *payload,
    uint16_t len
);

HAL_StatusTypeDef e32_receive(
    e32_t *e,
    uint8_t *data,
    uint16_t len,
    uint32_t timeout
);

#endif
