#include "e32_lora.h"

void e32_wait_aux(e32_t *e)
{
    while (HAL_GPIO_ReadPin(e->aux_port, e->aux_pin) == GPIO_PIN_RESET)
    {
    }

    HAL_Delay(2);
}

void e32_normal(e32_t *e)
{
    HAL_GPIO_WritePin(e->m0_port, e->m0_pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(e->m1_port, e->m1_pin, GPIO_PIN_RESET);

    HAL_Delay(10);
    e32_wait_aux(e);
}

void e32_config(e32_t *e)
{
    HAL_GPIO_WritePin(e->m0_port, e->m0_pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(e->m1_port, e->m1_pin, GPIO_PIN_SET);

    HAL_Delay(10);
    e32_wait_aux(e);
}

HAL_StatusTypeDef e32_send_fixed(
    e32_t *e,
    uint16_t address,
    uint8_t channel,
    uint8_t *payload,
    uint16_t len
)
{
    uint8_t header[3];

    header[0] = address >> 8;
    header[1] = address & 0xFF;
    header[2] = channel;

    e32_wait_aux(e);

    if (HAL_UART_Transmit(e->huart, header, 3, 200) != HAL_OK)
        return HAL_ERROR;

    if (HAL_UART_Transmit(e->huart, payload, len, 300) != HAL_OK)
        return HAL_ERROR;

    e32_wait_aux(e);

    return HAL_OK;
}

HAL_StatusTypeDef e32_receive(
    e32_t *e,
    uint8_t *data,
    uint16_t len,
    uint32_t timeout
)
{
    return HAL_UART_Receive(e->huart, data, len, timeout);
}
