#include <LPC214x.h>

#include "types.h"
#include "rtc.h"
#include "lcd.h"
#include "lcd_defines.h"

/* System Clock */
#define FOSC   12000000
#define CCLK   (5*FOSC)
#define PCLK   (CCLK/4)

/* RTC Prescaler */
#define PREINT_VAL  ((int)(PCLK/32768)-1)
#define PREFRAC_VAL (PCLK-((PREINT_VAL+1)*32768))

/* CCR Bits */
#define RTC_ENABLE  (1<<0)
#define RTC_RESET   (1<<1)
#define RTC_CLKSRC  (1<<4)

/* Day Names */
static char week[][4]=
{
    "SUN",
    "MON",
    "TUE",
    "WED",
    "THU",
    "FRI",
    "SAT"
};

/*************************************************
RTC Initialization
**************************************************/
void RTC_Init(void)
{
    CCR = RTC_RESET;

#ifdef CPU_LPC2148

    /* LPC2148 uses external 32.768kHz crystal */

    CCR = RTC_ENABLE | RTC_CLKSRC;

#else

    PREINT = PREINT_VAL;
    PREFRAC = PREFRAC_VAL;

    CCR = RTC_ENABLE;

#endif
}

/*************************************************
Set Time
**************************************************/
void SetRTCTimeInfo(u32 hour,u32 minute,u32 second)
{
    HOUR = hour;
    MIN  = minute;
    SEC  = second;
}

/*************************************************
Get Time
**************************************************/
void GetRTCTimeInfo(s32 *hour,s32 *minute,s32 *second)
{
    *hour = HOUR;
    *minute = MIN;
    *second = SEC;
}

/*************************************************
Set Date
**************************************************/
void SetRTCDateInfo(u32 date,u32 month,u32 year)
{
    DOM = date;
    MONTH = month;
    YEAR = year;
}

/*************************************************
Get Date
**************************************************/
void GetRTCDateInfo(s32 *date,s32 *month,s32 *year)
{
    *date = DOM;
    *month = MONTH;
    *year = YEAR;
}

/*************************************************
Set Day
**************************************************/
void SetRTCDay(u32 day)
{
    DOW = day;
}

/*************************************************
Get Day
**************************************************/
void GetRTCDay(s32 *day)
{
    *day = DOW;
}

/*************************************************
Display Time
**************************************************/
void DisplayRTCTime(u32 hour,u32 minute,u32 second)
{
    CmdLCD(GOTO_LINE1_POS0);

    CharLCD(hour/10+'0');
    CharLCD(hour%10+'0');

    CharLCD(':');

    CharLCD(minute/10+'0');
    CharLCD(minute%10+'0');

    CharLCD(':');

    CharLCD(second/10+'0');
    CharLCD(second%10+'0');
}

/*************************************************
Display Date
**************************************************/
void DisplayRTCDate(u32 date,u32 month,u32 year)
{
    CmdLCD(GOTO_LINE2_POS0);

    CharLCD(date/10+'0');
    CharLCD(date%10+'0');

    CharLCD('/');

    CharLCD(month/10+'0');
    CharLCD(month%10+'0');

    CharLCD('/');

    U32LCD(year);
}

/*************************************************
Display Day
**************************************************/
void DisplayRTCDay(u32 day)
{
    CmdLCD(GOTO_LINE1_POS0+10);

    StrLCD((s8 *)week[day]);
}
