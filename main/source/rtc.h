#ifndef _RTC_H
#define _RTC_H

#include "types.h"

typedef struct rtc_data_t
{
    u8 year;
    u8 month;
    u8 day;
    u8 weekday;
    u8 hours;
    u8 minutes;
    u8 secs;
} rtc_data_t;;

int rtc_enable(void);
int rtc_get(rtc_data_t *pOut);
int rtc_get_bcd(rtc_data_t *pOut);
void rtc_set(rtc_data_t *pIn);

uint32_t get_fattime();

#endif // _RTC_H