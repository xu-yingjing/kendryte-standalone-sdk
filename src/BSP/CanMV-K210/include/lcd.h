#ifndef __LCD_H
#define __LCD_H

#include <stdint.h>

/* Configure connect pin */
#if !defined(LCD_CSX_PIN)
#define LCD_CSX_PIN 36
#endif
#if !defined(LCD_RST_PIN)
#define LCD_RST_PIN 37
#endif
#if !defined(LCD_DCX_PIN)
#define LCD_DCX_PIN 38
#endif
#if !defined(LCD_WRX_PIN)
#define LCD_WRX_PIN 39
#endif
#if !defined(LCD_DCX_GPIOHS_NUM)
#define LCD_DCX_GPIOHS_NUM 31
#endif
#if !defined(LCD_RST_GPIOHS_NUM)
#define LCD_RST_GPIOHS_NUM 30
#endif

/* Configure SPI interface */
#if !defined(LCD_SPI)
#define LCD_SPI SPI_DEVICE_0
#endif
#if !defined(LCD_SPI_CS_NUM)
#define LCD_SPI_CS_NUM SPI_CHIP_SELECT_3
#endif
#if !defined(LCD_SPI_DMA_CH)
#define LCD_SPI_DMA_CH DMAC_CHANNEL2
#endif

/* LCD information */
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
