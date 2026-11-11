#include "ssd1306_min.h"

#include <string.h>
#include <stdio.h>

static I2C_HandleTypeDef *oled_i2c;
static uint8_t oled_buffer[1024];

static void cmd(uint8_t c)
{
    uint8_t d[2] = {0x00,c};

    HAL_I2C_Master_Transmit(
        oled_i2c,
        SSD1306_ADDR,
        d,
        2,
        100);
}

static void data_send(uint8_t *d, uint16_t len)
{
    uint8_t tmp[17];
    tmp[0] = 0x40;

    while(len)
    {
        uint8_t chunk =
            len > 16 ? 16 : len;

        memcpy(&tmp[1], d, chunk);

        HAL_I2C_Master_Transmit(
            oled_i2c,
            SSD1306_ADDR,
            tmp,
            chunk + 1,
            100);

        d += chunk;
        len -= chunk;
    }
}

void ssd1306_init(I2C_HandleTypeDef *hi2c)
{
    oled_i2c = hi2c;

    HAL_Delay(100);

    cmd(0xAE);
    cmd(0x20); cmd(0x00);
    cmd(0xA1);
    cmd(0xC8);
    cmd(0xA8); cmd(0x3F);
    cmd(0xD3); cmd(0x00);
    cmd(0x40);
    cmd(0x8D); cmd(0x14);
    cmd(0xAF);

    ssd1306_clear();
    ssd1306_update();
}

void ssd1306_clear(void)
{
    memset(oled_buffer,0,sizeof(oled_buffer));
}

void ssd1306_update(void)
{
    for(uint8_t page=0; page<8; page++)
    {
        cmd(0xB0 + page);
        cmd(0x00);
        cmd(0x10);

        data_send(
            &oled_buffer[128*page],
            128);
    }
}

static void draw_fake_char(
    uint8_t x,
    uint8_t y,
    char c)
{
    if(x > 122 || y > 56)
        return;

    uint16_t idx =
        x + (y/8)*128;

    for(uint8_t i=0;i<5;i++)
    {
        oled_buffer[idx+i] =
            ((uint8_t)c) ^
            (i*0x11);
    }
}

void ssd1306_print(
    uint8_t x,
    uint8_t y,
    const char *s)
{
    while(*s)
    {
        draw_fake_char(
            x,
            y,
            *s++);

        x += 6;

        if(x > 122)
            break;
    }
}

void ssd1306_print_num(
    uint8_t x,
    uint8_t y,
    const char *label,
    uint32_t n)
{
    char tmp[32];

    snprintf(
        tmp,
        sizeof(tmp),
        "%s%lu",
        label,
        (unsigned long)n);

    ssd1306_print(
        x,
        y,
        tmp);
}
