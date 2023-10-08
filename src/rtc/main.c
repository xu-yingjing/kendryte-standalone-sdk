#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "sleep.h"
#include "sysctl.h"
#include "rtc.h"

static const char *find_field(const char *string, const char interval, uint8_t index)
{
    uint8_t index_loop = 0;
    const char *field = string;

    while (index_loop < index)
    {
        while (field[0] != interval)
        {
            field++;
        }
        while (field[0] == interval)
        {
            field++;
        }
        index_loop++;
    }

    return field;
}

static void get_compile_time(rtc_date_time_t *compile_time)
{
    const char date[12] = {__DATE__};
    const char time[9] = {__TIME__};
    uint8_t length;
    const char *ptr;
    char buffer[100];
    uint8_t month_index;
    const char *month_list[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

    if (compile_time == NULL)
    {
        return;
    }

    /* Set to default */
    compile_time->sec = 0;
    compile_time->min = 0;
    compile_time->hour = 0;
    compile_time->week = 6;
    compile_time->day = 1;
    compile_time->month = 1;
    compile_time->year = 2000;

    /* Month */
    ptr = find_field(date, ' ', 0);
    length = (uint8_t)(strchr(ptr, ' ') - ptr);
    strncpy(buffer, ptr, length);
    buffer[length] = '\0';
    for (month_index=0; month_index<sizeof(month_list)/sizeof(month_list[0]); month_index++)
    {
        if (strcmp(buffer, month_list[month_index]) == 0)
        {
            compile_time->month = month_index + 1;
        }
    }
    
    /* Day */
    ptr = find_field(date, ' ', 1);
    length = (uint8_t)(strchr(ptr, ' ') - ptr);
    strncpy(buffer, ptr, length);
    buffer[length] = '\0';
    compile_time->day = atoi(buffer);

    /* Year */
    ptr = find_field(date, ' ', 2);
    length = (uint8_t)(strchr(ptr, '\0') - ptr);
    strncpy(buffer, ptr, length);
    buffer[length] = '\0';
    compile_time->year = atoi(buffer);

    /* Hour */
    ptr = find_field(time, ':', 0);
    length = (uint8_t)(strchr(ptr, ':') - ptr);
    strncpy(buffer, ptr, length);
    buffer[length] = '\0';
    compile_time->hour = atoi(buffer);

    /* Minute */
    ptr = find_field(time, ':', 1);
    length = (uint8_t)(strchr(ptr, ':') - ptr);
    strncpy(buffer, ptr, length);
    buffer[length] = '\0';
    compile_time->min = atoi(buffer);

    /* Second */
    ptr = find_field(time, ':', 2);
    length = (uint8_t)(strchr(ptr, '\0') - ptr);
    strncpy(buffer, ptr, length);
    buffer[length] = '\0';
    compile_time->sec = atoi(buffer);

    /* Week */
    compile_time->week = rtc_get_wday(compile_time->year, compile_time->month, compile_time->day);
}

int main(void)
{
    rtc_date_time_t time;
    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;
    int second_prev = 0;

    sysctl_pll_set_freq(SYSCTL_PLL0, 800000000);
    sysctl_pll_set_freq(SYSCTL_PLL1, 400000000);
    sysctl_pll_set_freq(SYSCTL_PLL2, 45158400);
    sysctl_set_power_mode(SYSCTL_POWER_BANK6, SYSCTL_POWER_V18);
    sysctl_set_power_mode(SYSCTL_POWER_BANK7, SYSCTL_POWER_V18);
    sysctl_set_spi0_dvp_data(1);

    get_compile_time(&time);

    rtc_init();
    rtc_timer_set(time.year, time.month, time.day, time.hour, time.min, time.sec);

    while (1)
    {
        msleep(100);
        rtc_timer_get(&year, &month, &day, &hour, &minute, &second);
        if (second_prev != second)
        {
            second_prev = second;
            printf("%4d-%02d-%02d %02d:%02d:%02d\n", year, month, day, hour, minute, second);
        }
    }
}
