#include "sysctl.h"
#include "lcd.h"
#include "camera.h"
#include <stddef.h>

int main(void)
{
    uint8_t *disp;

    sysctl_pll_set_freq(SYSCTL_PLL0, 800000000);
    sysctl_pll_set_freq(SYSCTL_PLL1, 400000000);
    sysctl_pll_set_freq(SYSCTL_PLL2, 45158400);
    sysctl_set_power_mode(SYSCTL_POWER_BANK6, SYSCTL_POWER_V18);
    sysctl_set_power_mode(SYSCTL_POWER_BANK7, SYSCTL_POWER_V18);
    sysctl_set_spi0_dvp_data(1);

    lcd_init();
    lcd_set_direction(DIR_YX_RLUD);
    camera_init(24000000);
    camera_set_pixformat(PIXFORMAT_RGB565);
    camera_set_framesize(320, 240);
    camera_set_framesize(224, 224);

    while (1)
    {
        camera_snapshot(&disp, NULL);
        lcd_draw_picture(8, 8, 224, 224, (uint16_t *)disp);
    }
}
