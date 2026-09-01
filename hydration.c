/*
 * Project : AquaGuardian - Smart Hydration Reminder System
 * File    : hydration.c
 * MCU     : LPC2148 (ARM7)
 *
 * Description:
 * Handles hydration tracking, LCD status display, water intake,
 * reminder management, missed-reminder counting, daily reset
 * and the project welcome message.
 */
	#include <LPC214x.h>
	#include "lcd.h"
	#include "rtc.h"
	#include "led.h"
	#include "buzzer.h"
	#include "defines.h"
	#include "lcd_defines.h"
	#include "hydration.h"
	#include "delay.h"
	
	/* Hydration and reminder status variables */
	u8 water_count = 0;
	u8 daily_goal = 8;
	u8 reminder_hour = 8;
	u8 reminder_min = 0;
	u8 reminder_active = 0;
	u8 reminder_done = 0;
	u8 missed_reminders = 0;
	
	/* Reminder timing variables */
	u32 reminder_interval;
	u32 reminder_duration;
	u32 reminder_start_seconds;
	
	/* RTC time and date variables */
	s32 hour,min,sec;
	s32 date,month,year;
	
	/* Hydration percentage and LCD position variables */
	u8 hydration;s32 pos;
	
	/*
	 * Function : DisplayStatus
	 * Description:
	 * Displays the current RTC time, date/next reminder and
	 * hydration status on the 16x2 LCD.
	 */
	void DisplayStatus(void)
	{
			GetRTCTimeInfo(&hour,&min,&sec);
			hydration = (water_count * 100) / daily_goal;
			CmdLCD(GOTO_LINE1_POS0);

			CharLCD(hour/10+'0');
			CharLCD(hour%10+'0');
			CharLCD(':');
			CharLCD(min/10+'0');
			CharLCD(min%10+'0');
			CharLCD(':');
			CharLCD(sec/10+'0');
			CharLCD(sec%10+'0');
			CmdLCD(GOTO_LINE1_POS0 +8);
			if((sec / 10) % 2 == 0)
			{
					GetRTCDateInfo(&date, &month, &year);
					CharLCD(' ');
					CharLCD(date/10 + '0');
					CharLCD(date%10 + '0');
					CharLCD('/');
					CharLCD(month/10 + '0');
					CharLCD(month%10 + '0');
					StrLCD("  ");
			}
			else
			{
					CharLCD(' ');
					CharLCD('N');
					CharLCD(':');
					CharLCD(reminder_hour/10 + '0');
					CharLCD(reminder_hour%10 + '0');
					CharLCD(':');
					CharLCD(reminder_min/10 + '0');
					CharLCD(reminder_min%10 + '0');
			}
			CmdLCD(GOTO_LINE2_POS0);
			if(water_count >= daily_goal)
			{
					GreenLED_ON();
					StrLCD("GOAL REACHED!   ");
			}
			else if(reminder_active == 1)
			{
					GreenLED_OFF();
					CharLCD(1);
					StrLCD(" DRINK WATER!   ");
			}
			else if((sec % 13) <10)
			{
					GreenLED_OFF();
					CharLCD(0);
					StrLCD("C:");
					U32LCD(water_count);
					StrLCD(" G:");
					U32LCD(daily_goal);
					StrLCD(" H:");
					U32LCD(hydration);
					CharLCD('%');
					StrLCD("  ");
			}
			else
			{
					GreenLED_OFF();
					CharLCD(2);
					StrLCD(" MISSED: ");
					U32LCD(missed_reminders);
					StrLCD("       ");
			}
	}
	/*
	 * Function : DrinkWater
	 * Description:
	 * Detects the drink button press, increments the water count,
	 * clears the active reminder and updates the reminder schedule.
	 */
	void DrinkWater(void)
	{
			if(READBIT(IOPIN0, DRINK_BUTTON) == 0)  
			{
					delay_ms(20);                         

					if(READBIT(IOPIN0, DRINK_BUTTON) == 0)
					{
							if(water_count < daily_goal)
							{
									water_count++;
									reminder_active = 0;
									missed_reminders = 0;
									RedLED_OFF();
									BuzzerOFF();
									if(water_count >= daily_goal)
									{
											GreenLED_ON();
											CmdLCD(CLEAR_LCD);
											CmdLCD(GOTO_LINE1_POS0);
											CharLCD(3);
											StrLCD(" GOAL REACHED!");
											CmdLCD(GOTO_LINE2_POS0);
											StrLCD("   WELL DONE!   ");
											delay_ms(1000);
									}
									else
									{
											SetNextReminder();
											CmdLCD(CLEAR_LCD);
											CmdLCD(GOTO_LINE1_POS0);
											StrLCD("GOOD JOB!");
											CmdLCD(GOTO_LINE2_POS0);
											StrLCD("WATER RECORDED");
											delay_ms(500);
									}
									ShowGlassProgress();
							}
							while(READBIT(IOPIN0, DRINK_BUTTON) == 0);
							delay_ms(20);
					}
			}
	}
	/*
	 * Function : ShowGlassProgress
	 * Description:
	 * Displays the user's hydration progress using custom LCD
	 * glass characters.
	 */
	void ShowGlassProgress(void)
	{
			u8 i;
			u8 total_icons;
			u8 filled_icons;
			CmdLCD(CLEAR_LCD);
			CmdLCD(GOTO_LINE1_POS0);
			StrLCD("YOUR PROGRESS:");
			CmdLCD(GOTO_LINE2_POS0);
			if(daily_goal <= GLASS_DISPLAY_WIDTH)
			{
					total_icons = daily_goal;
					filled_icons = (water_count < daily_goal) ? water_count : daily_goal;
			}
			else
			{
					total_icons = GLASS_DISPLAY_WIDTH;
					filled_icons = (u8)(((u32)water_count * GLASS_DISPLAY_WIDTH) / daily_goal);
			}
			for(i = 0; i < total_icons; i++)
			{
					if(i < filled_icons)
							CharLCD(GLASS_EMPTY);
					else
							CharLCD(GLASS_FILLED);
			}
			delay_ms(1500);
	}
	/*
	 * Function : CheckReminder
	 * Description:
	 * Checks the RTC against the scheduled reminder time.
	 * Activates the reminder, monitors its duration and increments
	 * the missed-reminder count when the user does not respond.
	 */
	void CheckReminder(void)
	{
			u32 current_seconds;
			if(water_count >= daily_goal)
			{
					reminder_active = 0;
					return;
			}

			GetRTCTimeInfo(&hour, &min, &sec);
			current_seconds = (hour * 3600) +(min * 60) +sec;
			if((hour == reminder_hour) &&
				 (min == reminder_min) &&
				 (reminder_done == 0))
			{
					reminder_active = 1;
					reminder_done = 1;

					reminder_start_seconds= (hour*3600)+(min*60)+sec;
			}
			if((reminder_active == 1) &&(missed_reminders < 3))
			{

							if((current_seconds - reminder_start_seconds) >= reminder_duration)
							{
									reminder_active = 0;
									YellowLED_OFF();
									missed_reminders++;
									SetNextReminder();
									CmdLCD(GOTO_LINE2_POS0);
									CharLCD(2);
									StrLCD(" MISSED: ");
									U32LCD(missed_reminders);
									StrLCD("       ");
									delay_ms(1500);
									if(missed_reminders >= 3)
									{
											RedLED_ON();
											reminder_active = 1;
									}
							}
			}
			if(missed_reminders >= 3)
			{
					RedLED_ON();
					reminder_active = 1;
			}

			if(min != reminder_min)
			{
					reminder_done = 0;
			}
	}
	/*
	 * Function : SetNextReminder
	 * Description:
	 * Calculates the next hydration reminder time by adding the
	 * configured reminder interval to the current RTC time.
	 */
	void SetNextReminder(void)
	{
			u32 total_minutes;
			GetRTCTimeInfo(&hour, &min, &sec);
			total_minutes = (hour * 60) + min;
			total_minutes += reminder_interval;
			if(total_minutes >= 1440)
			{
					total_minutes -= 1440;
			}
			reminder_hour = total_minutes / 60;
			reminder_min  = total_minutes % 60;
			reminder_done = 0;
	}
	/*
	 * Function : MidnightReset
	 * Description:
	 * Resets daily hydration and reminder information at midnight.
	 */
	void MidnightReset(void)
	{
			if((hour == 0) && (min == 0) && (sec == 0))
			{
					water_count = 0;
					reminder_hour = 8;
					reminder_min = 0;
					BuzzerOFF();
					reminder_active = 0;
					reminder_done = 0;
					GreenLED_OFF();
					RedLED_OFF();
					missed_reminders=0;
					YellowLED_OFF();
			}
	}
	void WelcomeTitle(void)
	{
			s32 pos, i;
			char msg[] = "                AquaGuardian - Smart Water Drinking Reminder System                ";
			s32 len = sizeof(msg) - 1;
			CmdLCD(CLEAR_LCD);
			CmdLCD(GOTO_LINE1_POS0 + 2);
			CharLCD(0);
			StrLCD(" V25HE10D1");
			for(pos = 0; pos <= len - 16; pos++)
			{
					CmdLCD(GOTO_LINE2_POS0);
					for(i = 0; i < 16; i++)
							CharLCD(msg[pos + i]);
					delay_ms(250);
			}
	}
