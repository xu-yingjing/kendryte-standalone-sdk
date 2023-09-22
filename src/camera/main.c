#include "sysctl.h"
#include "lcd.h"
#include "camera.h"
#include "iomem.h"
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
    
    disp = (uint8_t *)iomem_malloc(320 * 240 * 2);
    while (1)
    {
        while (camera_snapshot(NULL, NULL) == 0)
        {
            camera_snapshot_release();
        }
        while (camera_snapshot_copy(disp, NULL) != 0);
        lcd_draw_picture(0, 0, 320, 240, (uint16_t *)disp);
    }
}
