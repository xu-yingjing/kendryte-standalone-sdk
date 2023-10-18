#include "sysctl.h"
#include "bsp.h"
#include "lv_port_disp.h"

static int lvgl_tick(void *ctx)
{
    while (1)
    {
        msleep(5);
        lv_tick_inc(5);
    }

    return 0;
}

int main(void)
{
    sysctl_pll_set_freq(SYSCTL_PLL0, 800000000);
    sysctl_pll_set_freq(SYSCTL_PLL1, 400000000);
    sysctl_pll_set_freq(SYSCTL_PLL2, 45158400);
    sysctl_set_power_mode(SYSCTL_POWER_BANK6, SYSCTL_POWER_V18);
    sysctl_set_power_mode(SYSCTL_POWER_BANK7, SYSCTL_POWER_V18);
    sysctl_set_spi0_dvp_data(1);

    lv_init();
    lv_port_disp_init();
    disp_enable_update();

    lv_obj_t *btn = lv_btn_create(lv_scr_act());
    lv_obj_set_size(btn, 120, 50);
    lv_obj_center(btn);
    
    lv_obj_t *label = lv_label_create(btn);
    lv_label_set_text(label, "Button");
    lv_obj_center(label);

    register_core1(lvgl_tick, NULL);

    while (1)
    {
        lv_timer_handler();
    }
}
