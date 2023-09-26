#include "camera.h"
#include "fpioa.h"
#include "plic.h"
#include "iomem.h"
#include <stddef.h>
#include <string.h>

#if defined(CAMERA_SENSOR_OV2640)
#include "ov2640.h"
#endif

/* Variable for sensor */
static const camera_sensor_t *sensor;
static const camera_sensor_t *sensors[] =
{
#if defined(CAMERA_SENSOR_OV2640)
    &camera_ov2640,
#endif
};

/* Variable for framebuffer */
#if CAMERA_FRAMEBUFFER_NUM < 1
#error The number of framebuffer must be greater than 0
#endif
typedef struct camera_fb
{
    /* Base parameter */
    uint16_t width;
    uint16_t height;
    uint8_t bpp;
    pixformat_t pixformat;

    /* Pointer to memory */
    uint8_t *disp[CAMERA_FRAMEBUFFER_NUM];
    uint8_t *ai[CAMERA_FRAMEBUFFER_NUM];

    /* Index for access */
    volatile uint8_t read_index;
    volatile uint8_t write_index;

    /* Status */
    volatile uint8_t full;
    volatile uint8_t empty;
} camera_fb_t;
static camera_fb_t fb;

static void camera_dvp_lowlevel_init(uint8_t reg_len, uint32_t xclk_rate)
{
    /* Initialize connect pin */
    fpioa_set_function(CAMERA_SDA_PIN, FUNC_SCCB_SDA);
    fpioa_set_function(CAMERA_SCL_PIN, FUNC_SCCB_SCLK);
    fpioa_set_function(CAMERA_RST_PIN, FUNC_CMOS_RST);
    fpioa_set_function(CAMERA_VSYNC_PIN, FUNC_CMOS_VSYNC);
    fpioa_set_function(CAMERA_PWDN_PIN, FUNC_CMOS_PWDN);
    fpioa_set_function(CAMERA_HSYNC_PIN, FUNC_CMOS_HREF);
    fpioa_set_function(CAMERA_XCLK_PIN, FUNC_CMOS_XCLK);
    fpioa_set_function(CAMERA_PCLK_PIN, FUNC_CMOS_PCLK);

    /* Pre-initialize DVP */
    dvp_init(reg_len);
    dvp_set_xclk_rate(xclk_rate);
}

static int camera_sensor_init(uint32_t xclk_rate)
{
    uint8_t index;

    /* Set sensor to NULL by default */
    sensor = (camera_sensor_t *)NULL;

    /* Initialize each supported sensor until successful */
    for (index=0; index<(sizeof(sensors)/sizeof(sensors[0])); index++)
    {
        if (xclk_rate == 0)
        {
            xclk_rate = sensors[index]->xclk_rate;
        }
        camera_dvp_lowlevel_init(sensors[index]->reg_len, xclk_rate);

        if (sensors[index]->init() == 0)
        {
            sensor = sensors[index];
            return 0;
        }
    }

    return 1;
}

static void camera_fb_reset(void)
{
    uint8_t index;

    /* Reset base parameter */
    fb.width = 0;
    fb.height = 0;
    fb.bpp = 0;
    fb.pixformat = PIXFORMAT_INVLAID;

    /* Free memory */
    for (index=0; index<CAMERA_FRAMEBUFFER_NUM; index++)
    {
        if (fb.disp[index] != NULL)
        {
            iomem_free(fb.disp[index]);
            fb.disp[index] = NULL;
        }
        if (fb.ai[index] != NULL)
        {
            iomem_free(fb.ai[index]);
            fb.ai[index] = NULL;
        }
    }

    /* Reset access index */
    fb.read_index = 0;
    fb.write_index = 0;

    /* Reset status */
    fb.full = 0;
    fb.empty = 1;
}

static void camera_dvp_run(uint8_t enable)
{
    if (enable != 0)
    {
        /* Enable DVP interrupt */
        dvp_clear_interrupt(DVP_STS_FRAME_START | DVP_STS_FRAME_FINISH);
        dvp_config_interrupt(DVP_CFG_START_INT_ENABLE | DVP_CFG_FINISH_INT_ENABLE, 1);
        plic_irq_enable(IRQN_DVP_INTERRUPT);
    }
    else
    {
        /* Disable DVP interrupt */
        plic_irq_disable(IRQN_DVP_INTERRUPT);
        dvp_config_interrupt(DVP_CFG_START_INT_ENABLE | DVP_CFG_FINISH_INT_ENABLE, 0);
    }
}

static int camera_dvp_irq_callback(void *ctx)
{
    if (dvp_get_interrupt(DVP_STS_FRAME_START))
    {
        /* Start DVP capture if framebuffer is not full */
        if (fb.full == 0)
        {
            dvp_start_convert();
        }

        dvp_clear_interrupt(DVP_STS_FRAME_START);
    }
    else if (dvp_get_interrupt(DVP_STS_FRAME_FINISH))
    {
        /* Clean framebuffer empty flag, cause DVP capture is finished */
        fb.empty = 0;

        /* Update framebuffer write index for next DVP capture */
        fb.write_index++;
        if (fb.write_index == CAMERA_FRAMEBUFFER_NUM)
        {
            fb.write_index = 0;
        }

        /* Check framebuffer is full or not */
        if (fb.write_index == fb.read_index)
        {
            /* 
             * If write index is increased to equal read index,
             * it means framebuffer is full
             */
            fb.full = 1;
        }
        else
        {
            /* 
             * Set next capture's framebuffer only when framebuffer is not full,
             * otherwise do it after the first time framebuffer read release after this full status
             */
            dvp_set_ai_addr((uint32_t)fb.ai[fb.write_index] + (0 * fb.width * fb.height),
                            (uint32_t)fb.ai[fb.write_index] + (1 * fb.width * fb.height),
                            (uint32_t)fb.ai[fb.write_index] + (2 * fb.width * fb.height));
            dvp_set_display_addr((uint32_t)fb.disp[fb.write_index]);
        }

        dvp_clear_interrupt(DVP_STS_FRAME_FINISH);
    }
    else
    {
        /* Impossible situation */
    }

    return 0;
}

static void camera_dvp_init(void)
{
    /* Configure DVP base parameter */
    dvp_enable_burst();
    dvp_set_output_enable(DVP_OUTPUT_AI, 1);
    dvp_set_output_enable(DVP_OUTPUT_DISPLAY, 1);
    dvp_set_image_format(DVP_CFG_RGB_FORMAT);
    dvp_set_image_size(fb.width, fb.height);
    dvp_set_ai_addr((uint32_t)fb.ai[fb.write_index] + (0 * fb.width * fb.height),
                    (uint32_t)fb.ai[fb.write_index] + (1 * fb.width * fb.height),
                    (uint32_t)fb.ai[fb.write_index] + (2 * fb.width * fb.height));
    dvp_set_display_addr((uint32_t)fb.disp[fb.write_index]);
    dvp_disable_auto();

    /* Configure interrupt */
    plic_irq_disable(IRQN_DVP_INTERRUPT);
    plic_set_priority(IRQN_DVP_INTERRUPT, 1);
    plic_irq_register(IRQN_DVP_INTERRUPT, camera_dvp_irq_callback, NULL);
}

int camera_init(uint32_t xclk_rate)
{
    /* Reset framebuffer */
    camera_fb_reset();

    /* Automatically detect and initialize sensor */
    if (camera_sensor_init(xclk_rate) != 0)
    {
        return 1;
    }

    /* Configure DVP and disable DVP capture */
    camera_dvp_init();
    camera_dvp_run(0);

    return 0;
}

int camera_set_pixformat(pixformat_t format)
{
    if (format == PIXFORMAT_INVLAID)
    {
        return 1;
    }

    /* Reconfigure sensor's pixel format if that is changed */
    if (fb.pixformat != format)
    {
        if (sensor->set_pixformat(format) != 0)
        {
            return 1;
        }

        /* Recode frame parameters */
        fb.pixformat = format;
        switch (fb.pixformat)
        {
            case PIXFORMAT_RGB565:
            {
                fb.bpp = 2;
                break;
            }
            default:
            {
                break;
            }
        }
    }

    return 0;
}

int camera_set_framesize(uint16_t width, uint16_t height)
{
    uint8_t index;
    uint8_t rec_index;

    /* Reset sensor frame size */
    if (sensor->set_framesize(width, height) != 0)
    {
        return 1;
    }

    /* Realloc framebuffer memory if frame size is changed */
    if ((fb.width != width) || (fb.height != height))
    {
        for (index=0; index<CAMERA_FRAMEBUFFER_NUM; index++)
        {
            /* Free current framebuffer memory */
            if (fb.disp[index] != NULL)
            {
                iomem_free(fb.disp[index]);
                fb.disp[index] = NULL;
            }
            if (fb.ai[index] != NULL)
            {
                iomem_free(fb.ai[index]);
                fb.ai[index] = NULL;
            }

            /* Alloc memory for display framebuffer */
            fb.disp[index] = (uint8_t *)iomem_malloc(width * height * fb.bpp);
            if (fb.disp[index] == NULL)
            {
                for (rec_index=0; rec_index<index; rec_index++)
                {
                    iomem_free(fb.disp[rec_index]);
                    fb.disp[rec_index] = NULL;
                    fb.disp[rec_index] = (uint8_t *)iomem_malloc(fb.width * fb.height * fb.bpp);
                    iomem_free(fb.ai[rec_index]);
                    fb.ai[rec_index] = NULL;
                    fb.ai[rec_index] = (uint8_t *)iomem_malloc(fb.width * fb.height * 3);
                }
                fb.disp[index] = (uint8_t *)iomem_malloc(fb.width * fb.height * fb.bpp);
                sensor->set_framesize(fb.width, fb.height);
                return 1;
            }

            /* Alloc memory for AI framebuffer */
            fb.ai[index] = (uint8_t *)iomem_malloc(width * height * 3);
            if (fb.ai[index] == NULL)
            {
                for (rec_index=0; rec_index<index; rec_index++)
                {
                    iomem_free(fb.disp[rec_index]);
                    fb.disp[rec_index] = NULL;
                    fb.disp[rec_index] = (uint8_t *)iomem_malloc(fb.width * fb.height * fb.bpp);
                    iomem_free(fb.ai[rec_index]);
                    fb.ai[rec_index] = NULL;
                    fb.ai[rec_index] = (uint8_t *)iomem_malloc(fb.width * fb.height * 3);
                }
                iomem_free(fb.disp[index]);
                fb.disp[index] = NULL;
                fb.disp[index] = (uint8_t *)iomem_malloc(fb.width * fb.height * fb.bpp);
                fb.ai[index] = (uint8_t *)iomem_malloc(fb.width * fb.height * 3);
                sensor->set_framesize(fb.width, fb.height);
                return 1;
            }
        }

        /* Recode frame parameters */
        fb.width = width;
        fb.height = height;
    }

    /* Reconfigure DVP base parameter */
    dvp_set_image_size(width, height);
    dvp_set_ai_addr((uint32_t)fb.ai[fb.write_index] + (0 * fb.width * fb.height),
                    (uint32_t)fb.ai[fb.write_index] + (1 * fb.width * fb.height),
                    (uint32_t)fb.ai[fb.write_index] + (2 * fb.width * fb.height));
    dvp_set_display_addr((uint32_t)fb.disp[fb.write_index]);

    /* Enable DVP capture */
    camera_dvp_run(1);

    return 0;
}

int camera_snapshot(uint8_t **display, uint8_t **ai)
{
    /* Return error if framebuffer is empty */
    if (fb.empty == 1)
    {
        return 1;
    }

    /* Get data */
    if (display != NULL)
    {
        *display = fb.disp[fb.read_index];
    }
    if (ai != NULL)
    {
        *ai = fb.ai[fb.read_index];
    }

    return 0;
}

int camera_snapshot_release(void)
{
    /* Return error if framebuffer is empty */
    if (fb.empty == 1)
    {
        return 1;
    }

    /* Update framebuffer read index for next framebuffer read */
    fb.read_index++;
    if (fb.read_index == CAMERA_FRAMEBUFFER_NUM)
    {
        fb.read_index = 0;
    }

    /* Check framebuffer is empty or not */
    if (fb.read_index == fb.write_index)
    {
        /* 
         * If read index is increased to equal read index,
         * it means framebuffer is empty
         */
        fb.empty = 1;
    }

    /* Set next capture's framebuffer and clean framebuffer's full flag,
     * cause there is a framebuffer to be released when framebuffer is full
     */
    if (fb.full == 1)
    {
        dvp_set_ai_addr((uint32_t)fb.ai[fb.write_index] + (0 * fb.width * fb.height),
                        (uint32_t)fb.ai[fb.write_index] + (1 * fb.width * fb.height),
                        (uint32_t)fb.ai[fb.write_index] + (2 * fb.width * fb.height));
        dvp_set_display_addr((uint32_t)fb.disp[fb.write_index]);
        fb.full = 0;
    }

    return 0;
}

int camera_snapshot_copy(uint8_t *display, uint8_t *ai)
{
    /* Return error if framebuffer is empty */
    if (fb.empty == 1)
    {
        return 1;
    }

    /* Copy data */
    if (display != NULL)
    {
        memcpy(display, fb.disp[fb.read_index], fb.width * fb.height * fb.bpp);
    }
    if (ai != NULL)
    {
        memcpy(ai, fb.ai[fb.read_index], fb.width * fb.height * 3);
    }

    /* Release framebuffer that has been read */
    camera_snapshot_release();

    return 0;
}
