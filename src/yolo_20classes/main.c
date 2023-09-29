#include <stdio.h>
#include <stddef.h>
#include "sysctl.h"
#include "kpu.h"
#define LCD_SPI_CLK_RATE 15000000
#include "lcd.h"
#define CAMERA_FRAMEBUFFER_NUM 2
#include "camera.h"

#define INCBIN_STYLE INCBIN_STYLE_SNAKE
#define INCBIN_PREFIX
#include "incbin.h"
INCBIN(model, "yolo.kmodel");

#include "region_layer.h"
#define ANCHOR_NUM 7
static float g_anchor[ANCHOR_NUM * 2] = {1.08, 1.19, 3.42, 4.41, 6.63, 11.38, 9.42, 5.11, 16.62, 10.52};
char *classes_lable[20] =
{
    "aeroplane", "bicycle", "bird", "boat", "bottle",
    "bus", "car", "cat", "chair", "cow",
    "diningtable", "dog", "horse", "motorbike", "person",
    "pottedplant", "sheep", "sofa", "train", "tvmonitor"
};

static volatile uint8_t ai_done_flag;

static void ai_done_callback(void *userdata)
{
    ai_done_flag = 1;
}

void draw_boxes_callback(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint32_t label, float prob)
{
    if (x1 >= 320)
    {
        x1 = 319;
    }
    if (x2 >= 320)
    {
        x2 = 319;
    }
    if (y1 >= 240)
    {
        y1 = 239;
    }
    if (y2 >= 240)
    {
        y2 = 239;
    }

    printf("(%d, %d) (%d, %d) label: %s prob: %f\r\n", x1, y1, x2, y2, classes_lable[label], prob);
    lcd_draw_rectangle(x1, y1, x2, y2, 2, 0xF800);
}

int main(void)
{
    uint8_t *disp;
    uint8_t *ai;
    kpu_model_context_t task;
    float *output;
    size_t output_size;
    region_layer_t detect_rl;

    sysctl_pll_set_freq(SYSCTL_PLL0, 800000000);
    sysctl_pll_set_freq(SYSCTL_PLL1, 400000000);
    sysctl_pll_set_freq(SYSCTL_PLL2, 45158400);
    sysctl_clock_enable(SYSCTL_CLOCK_AI);
    sysctl_set_power_mode(SYSCTL_POWER_BANK6, SYSCTL_POWER_V18);
    sysctl_set_power_mode(SYSCTL_POWER_BANK7, SYSCTL_POWER_V18);
    sysctl_set_spi0_dvp_data(1);

    lcd_init();
    lcd_set_direction(DIR_YX_RLDU);
    camera_init(24000000);
    camera_set_pixformat(PIXFORMAT_RGB565);
    camera_set_framesize(320, 240);
    camera_set_vflip(1);
    camera_set_hmirror(1);

    if (kpu_load_kmodel(&task, (const uint8_t *)model_data) != 0)
    {
        printf("Kmodel load failed!\n");
        while (1);
    }

    detect_rl.anchor_number = ANCHOR_NUM;
    detect_rl.anchor = g_anchor;
    detect_rl.threshold = 0.5;
    detect_rl.nms_value = 0.2;
    region_layer_init(&detect_rl, 10, 7, 125, 320, 240);
    
    while (1)
    {
        if (camera_snapshot(&disp, &ai) == 0)
        {
            ai_done_flag = 0;
            if (kpu_run_kmodel(&task, (const uint8_t *)ai, DMAC_CHANNEL5, ai_done_callback, NULL) != 0)
            {
                printf("Kmodel run failed!\n");
                while (1);
            }
            while (ai_done_flag == 0);

            if (kpu_get_output(&task, 0, (uint8_t **)&output, &output_size) != 0)
            {
                printf("Output get failed!\n");
                while (1);
            }

            detect_rl.input = output;
            region_layer_run(&detect_rl, NULL);
            
            lcd_draw_picture(0, 0, 320, 240, (uint16_t *)disp);
            region_layer_draw_boxes(&detect_rl, draw_boxes_callback);

            camera_snapshot_release();
        }
    }
}
