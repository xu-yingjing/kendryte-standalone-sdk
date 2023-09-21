#ifndef __CAMERA_H
#define __CAMERA_H

#include "dvp.h"

/* Configure connect pin */
#define CAMERA_SDA_PIN      40
#define CAMERA_SCL_PIN      41
#define CAMERA_RST_PIN      42
#define CAMERA_VSYNC_PIN    43
#define CAMERA_PWDN_PIN     44
#define CAMERA_HSYNC_PIN    45
#define CAMERA_XCLK_PIN     46
#define CAMERA_PCLK_PIN     47

/* Configure support sensor */
#define CAMERA_SENSOR_OV2640

/* Configure number of framebuffer */
#define CAMERA_FRAMEBUFFER_NUM 2

typedef enum
{
    PIXFORMAT_INVLAID = 0,
    PIXFORMAT_RGB565,
} pixformat_t;

typedef struct
{
    uint8_t reg_len;    /* Length of register */
    uint32_t xclk_rate; /* Rate of XCLK */
    int (*init)(void);
    int (*set_pixformat)(pixformat_t format);
    int (*set_framesize)(uint16_t width, uint16_t height);
} camera_sensor_t;

int camera_init(uint32_t xclk_rate);
int camera_set_pixformat(pixformat_t format);
int camera_set_framesize(uint16_t width, uint16_t height);
int camera_snapshot(uint8_t **display, uint8_t **ai);

#endif
