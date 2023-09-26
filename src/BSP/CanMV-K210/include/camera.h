#ifndef __CAMERA_H
#define __CAMERA_H

#include "dvp.h"

/* Configure connect pin */
#if !defined(CAMERA_SDA_PIN)
#define CAMERA_SDA_PIN 40
#endif
#if !defined(CAMERA_SCL_PIN)
#define CAMERA_SCL_PIN 41
#endif
#if !defined(CAMERA_RST_PIN)
#define CAMERA_RST_PIN 42
#endif
#if !defined(CAMERA_VSYNC_PIN)
#define CAMERA_VSYNC_PIN 43
#endif
#if !defined(CAMERA_PWDN_PIN)
#define CAMERA_PWDN_PIN 44
#endif
#if !defined(CAMERA_HSYNC_PIN)
#define CAMERA_HSYNC_PIN 45
#endif
#if !defined(CAMERA_XCLK_PIN)
#define CAMERA_XCLK_PIN 46
#endif
#if !defined(CAMERA_PCLK_PIN)
#define CAMERA_PCLK_PIN 47
#endif

/* Configure support sensor */
#if !defined(CAMERA_SENSOR_OV2640)
#define CAMERA_SENSOR_OV2640
#endif

/* Configure number of framebuffer */
#if !defined(CAMERA_FRAMEBUFFER_NUM)
#define CAMERA_FRAMEBUFFER_NUM 2
#endif

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
int camera_snapshot_release(void);
int camera_snapshot_copy(uint8_t *display, uint8_t *ai);

#endif
