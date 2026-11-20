#ifndef SSD1306_MIN_H
#define SSD1306_MIN_H

#include "stm32f4xx_hal.h"
#include <stdint.h>
#include "fonts.h"

#define SSD1306_ADDR 0x78

#define SSD1306_DEFAULT_FONT 0
#define SSD1306_MEDIUM_FONT 1
#define SSD1306_LARGE_FONT 2

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

void ssd1306_set_current_font(int i_font, int i_permanent);

#endif
