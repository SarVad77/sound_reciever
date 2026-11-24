#include "delayer.h"

void set_delay(delay_t *delay, uint32_t i_sleep_time)
{
    delay->start_time = HAL_GetTick();
    delay->sleep_time = i_sleep_time;
}

int8_t get_delay(delay_t *delay)
{
    return (HAL_GetTick() - delay->start_time > delay->sleep_time ? 1 : 0);
}

void reset_delay(delay_t *delay)
{
    delay->start_time = 0;
    delay->sleep_time = 0;
}

