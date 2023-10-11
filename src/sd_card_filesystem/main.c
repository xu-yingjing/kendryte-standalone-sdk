#include <stddef.h>
#include <stdio.h>
#include "iomem.h"
#include "sysctl.h"
#define SD_SPI_CLK_HIGH_RATE 25000000
#include "sdcard.h"
#include "ff.h"

int main(void)
{
    FRESULT res;
    FATFS fs;
    DIR dir;
    FILINFO info;

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
    printf("SD card initialization succeed!\n");

    /* Filesystem mount SD card */
    res = f_mount(&fs, _T("/SD"), 1);
    if (res != FR_OK)
    {
        printf("SD card mount failed! Error code: %d\n", res);
        while (1);
    }
    printf("SD card mount succeed!\n");

    /* Open SD card root folder */
    res = f_opendir(&dir, _T("/SD"));
    if (res != FR_OK)
    {
        printf("SD card root folder open failed! Error code: %d\n", res);
        while (1);
    }
    printf("SD card root folder open succeed!\n");

    /* Read SD card root folder */
    while (1)
    {
        res = f_readdir(&dir, &info);
        if (res != FR_OK)
        {
            printf("SD card root folder read failed! Error code: %d\n", res);
            while (1);
        }
        if (info.fname[0] == '\0')
        {
            break;
        }
        printf("\t%s\n", info.fname);
    }
    printf("SD card root folder read succeed!\n");

    /* Close SD card root folder */
    res = f_closedir(&dir);
    if (res != FR_OK)
    {
        printf("SD card root folder close failed! Error code: %d\n", res);
        while (1);
    }
    printf("SD card root folder close succeed!\n");

    /* Filesystem unmount SD card */
    res = f_unmount(_T("/SD"));
    if (res != FR_OK)
    {
        printf("SD card unmount failed! Error code: %d\n", res);
        while (1);
    }
    printf("SD card unmount succeed!\n");

    while (1)
    {
        
    }
}
