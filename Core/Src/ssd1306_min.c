#include "ssd1306_min.h"
#include "ssd1306.h"
#include "fonts.h"



#include <string.h>
#include <stdio.h>

static I2C_HandleTypeDef *oled_i2c;

static int _current_font = SSD1306_DEFAULT_FONT;
static SSD1306_COLOR _current_color = White;

static int _permanent_font = 0;


 


void ssd1306_init(I2C_HandleTypeDef *hi2c)
{
    ssd1306_Init(hi2c);  // Call funxtion from library
    oled_i2c = hi2c;   // Remember I2C config
}

void ssd1306_clear(void)
{
    ssd1306_Fill(Black);
    ssd1306_SetCursor(0,0);
}

void ssd1306_update(void)
{
    ssd1306_UpdateScreen(oled_i2c); // Call function from library
}


void ssd1306_print(
    uint8_t x,
    uint8_t y,
    const char *s)
{
    ssd1306_SetCursor(x,y);
    if (_current_font == SSD1306_DEFAULT_FONT)
       ssd1306_WriteString(s,Font_7x10, _current_color);
    if (_current_font == SSD1306_MEDIUM_FONT)
       ssd1306_WriteString(s,Font_11x18, _current_color);
    if (_current_font == SSD1306_LARGE_FONT)
       ssd1306_WriteString(s,Font_16x26, _current_color);

    if (!_permanent_font) 
	_current_font = SSD1306_DEFAULT_FONT;

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


void ssd1306_set_current_font(int i_font, int i_permanent)
{
    _current_font = i_font;
    _permanent_font = i_permanent;
}
