#include <stddef.h>
#include <stdio.h>
#include "iomem.h"
#include "sysctl.h"
#define SD_SPI_CLK_HIGH_RATE 25000000
#include "sdcard.h"

#define TIMES_FOR_TEST      200
#define SECS_PER_TIMES      10
#define START_SEC_FOR_TEST  10000

/* Buffer for read/write speed test */
static uint8_t *rbuff = NULL;
static uint8_t *wbuff = NULL;

int main(void)
{
    uint32_t buffer_index;
    uint32_t times_index;
    uint64_t time_start;
    uint64_t time_end;
    uint64_t time_write;
    uint64_t time_read;

    sysctl_pll_set_freq(SYSCTL_PLL0, 800000000);
    sysctl_pll_set_freq(SYSCTL_PLL1, 400000000);
    sysctl_pll_set_freq(SYSCTL_PLL2, 45158400);
    sysctl_set_power_mode(SYSCTL_POWER_BANK6, SYSCTL_POWER_V18);
    sysctl_set_power_mode(SYSCTL_POWER_BANK7, SYSCTL_POWER_V18);
    sysctl_set_spi0_dvp_data(1);

    /* Initialize SD card */
    if (sd_init() != 0)
    {
        printf("SD card initialization failed!\n");
        while (1);
    }

    /* Show SD information */
    printf("SD card capacity: %ldMB\n", cardinfo.CardCapacity >> 20);
    printf("SD card sector size: %dB\n", cardinfo.CardBlockSize);

    /* Alloc memory for read/write buffer */
    rbuff = (uint8_t *)iomem_malloc(SECS_PER_TIMES * cardinfo.CardBlockSize);
    wbuff = (uint8_t *)iomem_malloc(SECS_PER_TIMES * cardinfo.CardBlockSize);
    if ((rbuff == NULL) || (wbuff == NULL))
    {
        printf("Out of memory!\n");
        while (1);
    }

    /* Write speed test */
    time_write = 0;
    for (times_index=0; times_index<TIMES_FOR_TEST; times_index++)
    {
        for (buffer_index=0; buffer_index<SECS_PER_TIMES * cardinfo.CardBlockSize; buffer_index++)
        {
            wbuff[buffer_index] = (uint8_t)(buffer_index & 0xFF);
        }
        time_start = sysctl_get_time_us();
        if (sd_write_sector_dma(wbuff, START_SEC_FOR_TEST, SECS_PER_TIMES) != 0)
        {
            printf("Write failed!\n");
            while (1);
        }
        time_end = sysctl_get_time_us();
        time_write += (time_end - time_start);
    }
    printf("[Write]Size: %fMB\n", (float)(TIMES_FOR_TEST * SECS_PER_TIMES * cardinfo.CardBlockSize) / (1u << 20));
    printf("[Write]Time: %fS\n", (float)time_write / 1000000.0);
    printf("[Write]Speed: %fMB/S\n", ((float)(TIMES_FOR_TEST * SECS_PER_TIMES * cardinfo.CardBlockSize) / (1u << 20)) / ((float)time_write / 1000000.0));

    /* Read speed test */
    time_read = 0;
    for (times_index=0; times_index<TIMES_FOR_TEST; times_index++)
    {
        for (buffer_index=0; buffer_index<SECS_PER_TIMES * cardinfo.CardBlockSize; buffer_index++)
        {
            rbuff[buffer_index] = 0;
        }
        time_start = sysctl_get_time_us();
        if (sd_read_sector_dma(rbuff, START_SEC_FOR_TEST, SECS_PER_TIMES) != 0)
        {
            printf("Read failed!\n");
            while (1);
        }
        time_end = sysctl_get_time_us();
        time_read += (time_end - time_start);
        for (buffer_index=0; buffer_index<SECS_PER_TIMES * cardinfo.CardBlockSize; buffer_index++)
        {
            if (rbuff[buffer_index] != (uint8_t)(buffer_index & 0xFF))
            {
                printf("Verity data failed!\n");
                while (1);
            }
        }
    }
    printf("[Read]Size: %fMB\n", (float)(TIMES_FOR_TEST * SECS_PER_TIMES * cardinfo.CardBlockSize) / (1u << 20));
    printf("[Read]Time: %fS\n", (float)time_read / 1000000.0);
    printf("[Read]Speed: %fMB/S\n", ((float)(TIMES_FOR_TEST * SECS_PER_TIMES * cardinfo.CardBlockSize) / (1u << 20)) / ((float)time_read / 1000000.0));

    while (1)
    {
        
    }
}
