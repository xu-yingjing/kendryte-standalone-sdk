#include "lcd.h"
#include "fpioa.h"
#include "gpiohs.h"
#include "spi.h"
#include "sleep.h"

static void lcd_write_command(uint8_t cmd)
{
    gpiohs_set_pin(LCD_DCX_GPIOHS_NUM, GPIO_PV_LOW);
    spi_init(LCD_SPI, SPI_WORK_MODE_0, SPI_FF_OCTAL, 8, 0);
    spi_init_non_standard(LCD_SPI, 8, 0, 0, SPI_AITM_AS_FRAME_FORMAT);
    spi_send_data_normal_dma(LCD_SPI_DMA_CH, LCD_SPI, LCD_SPI_CS_NUM, &cmd, 1, SPI_TRANS_CHAR);
}

static void lcd_write_data_8b(uint8_t *dat, uint32_t len)
{
    gpiohs_set_pin(LCD_DCX_GPIOHS_NUM, GPIO_PV_HIGH);
    spi_init(LCD_SPI, SPI_WORK_MODE_0, SPI_FF_OCTAL, 8, 0);
    spi_init_non_standard(LCD_SPI, 0, 8, 0, SPI_AITM_AS_FRAME_FORMAT);
    spi_send_data_normal_dma(LCD_SPI_DMA_CH, LCD_SPI, LCD_SPI_CS_NUM, dat, len, SPI_TRANS_CHAR);
}

static void lcd_write_data_32b(uint32_t *dat, uint32_t len)
{
    gpiohs_set_pin(LCD_DCX_GPIOHS_NUM, GPIO_PV_HIGH);
    spi_init(LCD_SPI, SPI_WORK_MODE_0, SPI_FF_OCTAL, 32, 0);
    spi_init_non_standard(LCD_SPI, 0, 32, 0, SPI_AITM_AS_FRAME_FORMAT);
    spi_send_data_normal_dma(LCD_SPI_DMA_CH, LCD_SPI, LCD_SPI_CS_NUM, dat, len, SPI_TRANS_INT);
}

static void lcd_write_data_32b_fill(uint32_t *dat, uint32_t len)
{
    gpiohs_set_pin(LCD_DCX_GPIOHS_NUM, GPIO_PV_HIGH);
    spi_init(LCD_SPI, SPI_WORK_MODE_0, SPI_FF_OCTAL, 32, 0);
    spi_init_non_standard(LCD_SPI, 0, 32, 0, SPI_AITM_AS_FRAME_FORMAT);
    spi_fill_data_dma(LCD_SPI_DMA_CH, LCD_SPI, LCD_SPI_CS_NUM, dat, len);
}

static void lcd_set_area(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    uint8_t data[4] = {0};

    data[0] = (uint8_t)(x1 >> 8);
    data[1] = (uint8_t)(x1);
    data[2] = (uint8_t)(x2 >> 8);
    data[3] = (uint8_t)(x2);
    lcd_write_command(0x2A);
    lcd_write_data_8b(data, 4);

    data[0] = (uint8_t)(y1 >> 8);
    data[1] = (uint8_t)(y1);
    data[2] = (uint8_t)(y2 >> 8);
    data[3] = (uint8_t)(y2);
    lcd_write_command(0x2B);
    lcd_write_data_8b(data, 4);

    lcd_write_command(0x2C);
}

void lcd_init(void)
{
    uint8_t data;

    /* Initialize DCX pin */
    fpioa_set_function(LCD_DCX_PIN, FUNC_GPIOHS0 + LCD_DCX_GPIOHS_NUM);
    gpiohs_set_drive_mode(LCD_DCX_GPIOHS_NUM, GPIO_DM_OUTPUT);
    gpiohs_set_pin(LCD_DCX_GPIOHS_NUM, GPIO_PV_HIGH);
    
    /* Initialize RESET pin */
    fpioa_set_function(LCD_RST_PIN, FUNC_GPIOHS0 + LCD_RST_GPIOHS_NUM);
    gpiohs_set_drive_mode(LCD_RST_GPIOHS_NUM, GPIO_DM_OUTPUT);
    gpiohs_set_pin(LCD_RST_GPIOHS_NUM, GPIO_PV_HIGH);

    /* Initialize CSX pin */
    fpioa_set_function(LCD_CSX_PIN, FUNC_SPI0_SS0 + LCD_SPI_CS_NUM);

    /* Initialize WRX pin */
    fpioa_set_function(LCD_WRX_PIN, FUNC_SPI0_SCLK);

    /* Initialize SPI interface */
    spi_init(LCD_SPI, SPI_WORK_MODE_0, SPI_FF_OCTAL, 8, 0);
    spi_set_clk_rate(LCD_SPI, LCD_SPI_CLK_RATE);
    
    /* Hardware reset */
    gpiohs_set_pin(LCD_RST_GPIOHS_NUM, GPIO_PV_LOW);
    msleep(50);
    gpiohs_set_pin(LCD_RST_GPIOHS_NUM, GPIO_PV_HIGH);
    msleep(50);

    /* Software reset */
    lcd_write_command(0x01);
    msleep(50);

    /* Exit sleep */
    lcd_write_command(0x11);
    msleep(120);

    /* Set pixel format */
    lcd_write_command(0x3A);
    data = 0x55;
    lcd_write_data_8b(&data, 1);
    msleep(10);

    /* Set direction */
    lcd_write_command(0x36);
    data = 0xA0;
    lcd_write_data_8b(&data, 1);
    
    /* Display on */
    lcd_write_command(0x13);
    msleep(10);
    lcd_write_command(0x29);

    /* Clear display */
    lcd_clear(0xFFFF);
}

void lcd_clear(uint16_t color)
{
    lcd_draw_fill_rectangle(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1, color);
}

void lcd_draw_rectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t width, uint16_t color)
{
    lcd_draw_fill_rectangle(x1, y1, x2, y1 + width - 1, color);
    lcd_draw_fill_rectangle(x2 - width + 1, y1, x2, y2, color);
    lcd_draw_fill_rectangle(x1, y2 - width + 1, x2, y2, color);
    lcd_draw_fill_rectangle(x1, y1, x1 + width - 1, y2, color);
}

void lcd_draw_fill_rectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
    uint32_t data = ((uint32_t)color << 16) | color;
    uint32_t length = (x2 - x1 + 1) * (y2 - y1 + 1);

    lcd_set_area(x1, y1, x2, y2);
    if (length & 1UL)
    {
        lcd_write_data_32b_fill(&data, length - 1);
    }
    else
    {
        lcd_write_data_32b_fill(&data, length);
    }
}

void lcd_draw_picture(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t *pic)
{
    uint32_t length = width * height;

    lcd_set_area(x, y, x + width - 1, y + height - 1);
    lcd_write_data_32b((uint32_t *)pic, length >> 1);
}

void lcd_set_direction(lcd_dir_t dir)
{
    lcd_write_command(0x36);
    lcd_write_data_8b((uint8_t *)&dir, 1);
}
