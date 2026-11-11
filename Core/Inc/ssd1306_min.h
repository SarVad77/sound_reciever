#ifndef SSD1306_MIN_H
#define SSD1306_MIN_H

#include "stm32f4xx_hal.h"
#include <stdint.h>

#define SSD1306_ADDR 0x78

void ssd1306_init(I2C_HandleTypeDef *hi2c);
void ssd1306_clear(void);
void ssd1306_update(void);

void ssd1306_print(
    uint8_t x,
    uint8_t y,
    const char *s
);

void ssd1306_print_num(
    uint8_t x,
    uint8_t y,
    const char *label,
    uint32_t n
);

#endif
