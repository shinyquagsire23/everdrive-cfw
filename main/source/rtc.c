#include "rtc.h"

#include "types.h"

#define RTC_DATA   ((vu16 *)0x080000C4)
#define RTC_RW     ((vu16 *)0x080000C6)
#define RTC_ENABLE ((vu16 *)0x080000C8)

#define FROM_BCD(x) (((x) & 0xF) + (((x) >> 4) * 10))
#define TO_BCD(x)   ((((x) / 10)<<4) + ((x) % 10))

#define RTC_CMD_READ(x)  (((x)<<1) | 0x61)
#define RTC_CMD_WRITE(x) (((x)<<1) | 0x60)

#define _YEAR   0
#define _MONTH  1
#define _DAY    2
#define _WKD    3
#define _HOUR   4
#define _MIN    5
#define _SEC    6

#define RTC_CMD_RESET       (0)
#define RTC_CMD_STATUS      (1)
#define RTC_CMD_DATA1       (2)
#define RTC_CMD_DATA2       (3)
#define RTC_CMD_ALARMFRQ1   (4)
#define RTC_CMD_ALARMFRQ2   (5)
#define RTC_CMD_TEST_START  (6)
#define RTC_CMD_TEST_END    (7)

//this sends commands to the RTC so it knows what to send/receive
//see the data sheet for the Seiko S-3511 for more details
//0x65 to read the 7byte date/time BCDs and 0x64 to write them
void rtc_cmd(int v)
{
    v <<= 1;

    for(int l = 7; l >= 0; l--)
    {
        u16 b = (v>>l) & 0x2;
        *RTC_DATA = b | 4;
        *RTC_DATA = b | 4;
        *RTC_DATA = b | 4;
        *RTC_DATA = b | 5;
    }
}

//this pipes data out to the RTC
//remember that data must be BCDs
void rtc_data(int v)
{
    v <<= 1;

    for(int l = 0; l < 8; l++)
    {
        u16 b = (v>>l) & 0x2;
        *RTC_DATA = b | 4;
        *RTC_DATA = b | 4;
        *RTC_DATA = b | 4;
        *RTC_DATA = b | 5;
    }
}

//this pipes data in from the RTC
int rtc_read(void)
{
    int v = 0;
    for(int l = 0; l < 8; l++)
    {
        for(int j = 0; j < 5; j++)
            *RTC_DATA = 4;

        *RTC_DATA = 5;
        u16 b = *RTC_DATA;
        v = v | ((b & 2)<<l);
    }
    v = v>>1;
    return v;
}

int rtc_enable(void)
{
    *RTC_ENABLE = 1;
    *RTC_DATA = 1;
    *RTC_DATA = 5;
    *RTC_RW = 7;

    rtc_cmd(RTC_CMD_READ(RTC_CMD_STATUS));
    *RTC_RW = 5;

    return (rtc_read() & 0x40); // If it's initted, read returns 0x40
}

int rtc_get_ex(rtc_data_t *out, int bRawBCD)
{
    *RTC_DATA = 1;
    *RTC_RW = 7;
    *RTC_DATA = 1;
    *RTC_DATA = 5;

    rtc_cmd(RTC_CMD_READ(RTC_CMD_DATA1));
    *RTC_RW = 5;

    out->year = (rtc_read() & 0xFF);
    out->month = (rtc_read() & 0xFF);
    out->day = (rtc_read() & 0xFF);
    out->weekday = (rtc_read() & 0xFF);

    *RTC_RW = 5;

    out->hours = (rtc_read() & 0xFF);
    out->minutes = (rtc_read() & 0xFF);
    out->secs = (rtc_read() & 0xFF);

    if (!bRawBCD)
    {
        out->year = FROM_BCD(out->year);
        out->month = FROM_BCD(out->month);
        out->day = FROM_BCD(out->day);
        out->weekday = FROM_BCD(out->weekday);

        out->hours = FROM_BCD(out->hours & 0x7F); // don't include AM/PM bit
        out->minutes = FROM_BCD(out->minutes);
        out->secs = FROM_BCD(out->secs);
    }

    return 0;
}

int rtc_get(rtc_data_t *pOut)
{
    rtc_get_ex(pOut, 0);
}

int rtc_get_bcd(rtc_data_t *pOut)
{
    rtc_get_ex(pOut, 1);
}

void rtc_set(rtc_data_t *pIn) 
{
    u8 tmp[7];

    tmp[0] = TO_BCD(pIn->year);
    tmp[1] = TO_BCD(pIn->month);
    tmp[2] = TO_BCD(pIn->day);
    tmp[3] = TO_BCD(pIn->weekday);
    tmp[4] = TO_BCD(pIn->hours);
    tmp[5] = TO_BCD(pIn->minutes);
    tmp[6] = TO_BCD(pIn->secs);
    
    *RTC_ENABLE = 1;
    *RTC_DATA = 1;
    *RTC_DATA = 5;
    *RTC_RW = 7;

    rtc_cmd(RTC_CMD_WRITE(RTC_CMD_DATA1));

    for(int i = 0; i < 4; i++) {
        rtc_data(tmp[i]);
    }

    for(int i = 4; i < 7; i++) {
        rtc_data(tmp[i]);
    }
}

// for fatfs
uint32_t get_fattime()
{
    rtc_data_t rtc_data;
    memset(&rtc_data, 0, sizeof(rtc_data));
    rtc_enable();
    rtc_get(&rtc_data);

    return (((2000 + (rtc_data.year)) - 1980) << 25)
           | ((rtc_data.month) << 21)
           | ((rtc_data.day) << 16);
}