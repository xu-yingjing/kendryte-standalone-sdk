#ifndef __LCD_H
#define __LCD_H

#include <stdint.h>

#define LCD_WIDTH   320
#define LCD_HEIGHT  240

typedef enum
{
    DIR_XY_RLUD = 0x00,
    DIR_YX_RLUD = 0x20,
    DIR_XY_LRUD = 0x40,
    DIR_YX_LRUD = 0x60,
    DIR_XY_RLDU = 0x80,
    DIR_YX_RLDU = 0xA0,
    DIR_XY_LRDU = 0xC0,
    DIR_YX_LRDU = 0xE0,
} lcd_dir_t;

void lcd_init(void);
void lcd_clear(uint16_t color);
void lcd_draw_rectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t width, uint16_t color);
void lcd_draw_fill_rectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
void lcd_draw_picture(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t *pic);
void lcd_set_direction(lcd_dir_t dir);

#endif
