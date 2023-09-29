#include "sysctl.h"
#define LCD_SPI_CLK_RATE 15000000
#include "lcd.h"

int main(void)
{
    sysctl_pll_set_freq(SYSCTL_PLL0, 800000000);
    sysctl_pll_set_freq(SYSCTL_PLL1, 400000000);
    sysctl_pll_set_freq(SYSCTL_PLL2, 45158400);
    sysctl_set_power_mode(SYSCTL_POWER_BANK6, SYSCTL_POWER_V18);
    sysctl_set_power_mode(SYSCTL_POWER_BANK7, SYSCTL_POWER_V18);
    sysctl_set_spi0_dvp_data(1);

    lcd_init();
    lcd_set_direction(DIR_YX_LRUD);
    
    lcd_draw_fill_rectangle(10, 10, 50, 100, 0xF800);

    while (1)
    {
        
    }
}
