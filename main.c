/*
 * Project : AquaGuardian - Smart Hydration Reminder System
 * File    : main.c
 * MCU     : LPC2148 (ARM7)
 *
 * Description:
 * Main application file responsible for system initialization,
 * hydration reminder handling, LCD status display, water intake
 * monitoring and user configuration through the keypad.
 */
#include <lpc214x.h>
#include "lcd.h"
#include "lcd_defines.h"
#include "rtc.h"
#include "led.h"
#include "interrupt.h"
#include "buzzer.h"
#include "kpm.h"
#include "hydration.h"
#include "delay.h"
/* Default hydration reminder interval and reminder duration */
#define REMINDER_INTERVAL_MIN 2
#define REMINDER_DURATION_SEC 30
/* Custom LCD characters used for hydration status indication */
u8 custom_chars[] =
{
    0x1F,0x1F,0x1F,0x1F,0x0E,0x04,0x0E,0x00,
		0x04,0x0E,0x0E,0x0E,0x1F,0x1F,0x04,0x00,
		0x1F,0x15,0x1F,0x1F,0x0E,0x0A,0x1B,0x00,
		0x04,0x15,0x0E,0x1F,0x0E,0x15,0x04,0x00,
		0x11,0x11,0x11,0x1F,0x1F,0x1F,0x0E,0x00,
		0x11,0x11,0x11,0x11,0x11,0x11,0x0E,0x00
};
int main(void)
{
		u8 key;
		u32 goal;
		/* Initialize all peripherals used by the system */
		InitLCD();
		BuildCGRAM(custom_chars, 48);
		RTC_Init();
		InitLED();
		InitBuzzer();
		Init_KPM();
		Init_EINT0();
		/* Set initial RTC time, date and day */
		SetRTCTimeInfo(7,59,40);
		SetRTCDateInfo(20,8,2026);
		SetRTCDay(3);
		WelcomeTitle();
		CmdLCD(CLEAR_LCD);
		/* Configure drink button pins as inputs */
		IODIR0 &=~(1<<1);
		IODIR0 &= ~(1 << DRINK_BUTTON);     
		reminder_interval=REMINDER_INTERVAL_MIN;
		reminder_duration = REMINDER_DURATION_SEC;
		while(1)
		{
			DisplayStatus();
			CheckReminder();
			MidnightReset();
			if(reminder_active == 1)
			{
				if(missed_reminders >= 3)
				{
					YellowLED_OFF();
					RedLED_ON();
					BuzzerON();
				}
				else
				{
					YellowLED_ON();
					Beep(100);
					YellowLED_OFF();
					delay_ms(100);
				}
			}
			DrinkWater();
			/*
       * Enter configuration mode when the external interrupt
       * sets MenuFlag.
       */
			if(MenuFlag == 1)
			{
				/*
				 * Configuration menu:
				 * 1 - Set daily hydration goal
				 * 2 - Configure RTC time/date
				 * 3 - Configure reminder interval
				 * C - Exit configuration mode
				 */
					MenuFlag = 0;
					CmdLCD(CLEAR_LCD);
					CmdLCD(GOTO_LINE1_POS0);
					StrLCD("CONFIG MODE");
					delay_ms(500);
					CmdLCD(CLEAR_LCD);
					CmdLCD(GOTO_LINE1_POS0);
					StrLCD("1.GOAL  2.RTC");
					CmdLCD(GOTO_LINE2_POS0);
					StrLCD("3.INT   C:EXIT");
					while(1)
					{
						key=keyscan();
						if(key=='1')
						{
						L1 : CmdLCD(CLEAR_LCD);
								StrLCD("GOAL SELECTED");
								CmdLCD(GOTO_LINE2_POS0);
								StrLCD("SET GOAL:");
								goal = ReadNum();
								if(goal == 0xFFFFFFFF)
								{
										CmdLCD(CLEAR_LCD);
										StrLCD("GOAL CANCELLED");
										delay_ms(500);
								}
								else if(goal == 0xFFFFFFFE)
								{
										CmdLCD(CLEAR_LCD);
										StrLCD("GOAL SKIPPED");
										delay_ms(500);
								}
								else if(goal >= 1 && goal <= 99)
								{
										daily_goal = goal;

										CmdLCD(CLEAR_LCD);
										StrLCD("GOAL SAVED");
										delay_ms(1000);
								}
								else
								{
										CmdLCD(CLEAR_LCD);
										StrLCD("INVALID GOAL");
										delay_ms(1000);
										goto L1;
								}
								CmdLCD(CLEAR_LCD);
								CmdLCD(GOTO_LINE1_POS0);
								StrLCD("1.GOAL  2.RTC");
								CmdLCD(GOTO_LINE2_POS0);
								StrLCD("3.INT   C:EXIT");
						}
						else if(key=='2')
						{
							CmdLCD(CLEAR_LCD);
							CmdLCD(GOTO_LINE1_POS0);
							StrLCD("RTC MENU");
							CmdLCD(GOTO_LINE2_POS0);
							StrLCD("1.TIM 2.DAT C:E");
							while(1)
							{
								key=keyscan();
								if(key == '1')
								{
									 /*
										* RTC time configuration:
										* User can modify hour, minute and second.
										* Current values are retained when a field is skipped.
										*/
										u32 value;
										s32 hh, mm, ss;
										u8 cancel = 0;
										GetRTCTimeInfo(&hh, &mm, &ss);
							 L2 : CmdLCD(CLEAR_LCD);
										StrLCD("SET HOUR:");
										CmdLCD(GOTO_LINE2_POS0);
										value = ReadNum();
										if(value == 0xFFFFFFFF)
										{
												cancel = 1;
										}
										else if(value != 0xFFFFFFFE)
										{
												if(value <= 23)
												hh = value;
												else
												{
														CmdLCD(CLEAR_LCD);
														StrLCD("INVALID HOUR");
														delay_ms(1000);
														goto L2;
												}
										}
										if(cancel == 0)
										{
									 L3 : CmdLCD(CLEAR_LCD);
												StrLCD("SET MIN:");
												CmdLCD(GOTO_LINE2_POS0);
												value = ReadNum();
												if(value == 0xFFFFFFFF)
												{
														cancel = 1;
												}
												else if(value != 0xFFFFFFFE)
												{
														if(value <= 59)
															mm = value;
														else
														{
																CmdLCD(CLEAR_LCD);
																StrLCD("INVALID MIN");
																delay_ms(1000);
																goto L3;
														}
												}
										}

										if(cancel == 0)
										{
										L4 : CmdLCD(CLEAR_LCD);
												StrLCD("SET SEC:");
												CmdLCD(GOTO_LINE2_POS0);
												value = ReadNum();
												if(value == 0xFFFFFFFF)
												{
														cancel = 1;
												}
												else if(value != 0xFFFFFFFE)
												{
														if(value <= 59)
															ss = value;
														else
														{
																	CmdLCD(CLEAR_LCD);
																	StrLCD("INVALID SEC");
																	delay_ms(1000);
																	goto L4;
														}
												}
										}
										if(cancel == 0)
										{
												SetRTCTimeInfo(hh, mm, ss);
												SetNextReminder();
												CmdLCD(CLEAR_LCD);
												StrLCD("TIME SAVED");
												delay_ms(1000);
										}
										else
										{
												CmdLCD(CLEAR_LCD);
												StrLCD("TIME CANCELLED");
												delay_ms(500);
										}
											CmdLCD(CLEAR_LCD);
											CmdLCD(GOTO_LINE1_POS0);
											StrLCD("RTC MENU");
											CmdLCD(GOTO_LINE2_POS0);
											StrLCD("1.TIM 2.DAT C:E");
								}
								else if(key == '2')
								{
										/*
										 * RTC date configuration:
										 * User can modify day, month and year.
										 * Input ranges are validated before updating the RTC.
										 */
										u32 value;
										s32 dd, mm, yyyy;
										u8 cancel = 0;
										GetRTCDateInfo(&dd, &mm, &yyyy);
							 L5 : CmdLCD(CLEAR_LCD);
										CmdLCD(GOTO_LINE1_POS0);
										StrLCD("SET DATE:");
										CmdLCD(GOTO_LINE2_POS0);
										value = ReadNum();
										if(value == 0xFFFFFFFF)
										{
												cancel = 1;
										}
										else if(value != 0xFFFFFFFE)
										{
												if(value >= 1 && value <= 31)
												{
														dd = value;
												}
												else
												{
														CmdLCD(CLEAR_LCD);
														StrLCD("INVALID DATE");
														delay_ms(1000);
														goto L5;
												}
										}
										if(cancel == 0)
										{
									 L6 : CmdLCD(CLEAR_LCD);
												CmdLCD(GOTO_LINE1_POS0);
												StrLCD("SET MONTH:");
												CmdLCD(GOTO_LINE2_POS0);
												value = ReadNum();
												if(value == 0xFFFFFFFF)
												{
														cancel = 1;
												}
												else if(value != 0xFFFFFFFE)
												{
														if(value >= 1 && value <= 12)
														{
																mm = value;
														}
														else
														{
																CmdLCD(CLEAR_LCD);
																StrLCD("INVALID MONTH");
																delay_ms(1000);
																goto L6;
														}
												}
										}
										if(cancel == 0)
										{
									 L7 : CmdLCD(CLEAR_LCD);
												CmdLCD(GOTO_LINE1_POS0);
												StrLCD("SET YEAR:");
												CmdLCD(GOTO_LINE2_POS0);
												value = ReadNum();
												if(value == 0xFFFFFFFF)
												{
														cancel = 1;
												}
												else if(value != 0xFFFFFFFE)
												{
														if(value >= 2000 && value <= 2099)
														{
																yyyy = value;
														}
														else
														{
																CmdLCD(CLEAR_LCD);
																StrLCD("INVALID YEAR");
																delay_ms(1000);
																goto L7;
														}
												}
										}
										if(cancel == 0)
										{
												SetRTCDateInfo(dd, mm, yyyy);

												CmdLCD(CLEAR_LCD);
												StrLCD("DATE SAVED");
												delay_ms(1000);
										}
										else
										{
												CmdLCD(CLEAR_LCD);
												StrLCD("DATE CANCELLED");
												delay_ms(500);
										}
										CmdLCD(CLEAR_LCD);
										CmdLCD(GOTO_LINE1_POS0);
										StrLCD("RTC MENU");
										CmdLCD(GOTO_LINE2_POS0);
										StrLCD("1.TIM 2.DAT C:E");
								}
								else if(key == 'C')
								{
										CmdLCD(CLEAR_LCD);
										StrLCD("EXIT RTC");
										delay_ms(500);
										break;
								}
							}
						CmdLCD(CLEAR_LCD);
						CmdLCD(GOTO_LINE1_POS0);
						StrLCD("1.GOAL  2.RTC");
						CmdLCD(GOTO_LINE2_POS0);
						StrLCD("3.INT   C:EXIT");
						}
						else if(key == '3')
						{
								/* Option 3: Configure hydration reminder interval */
								u32 interval;
					 L8 : CmdLCD(CLEAR_LCD);
								CmdLCD(GOTO_LINE1_POS0);
								StrLCD("REM INTERVAL");
								CmdLCD(GOTO_LINE2_POS0);
								StrLCD("SET MIN:");
								interval = ReadNum();
								if(interval == 0xFFFFFFFF)
								{
										CmdLCD(CLEAR_LCD);
										StrLCD("CANCELLED");
										delay_ms(500);
								}
								else if(interval == 0xFFFFFFFE)
								{
										CmdLCD(CLEAR_LCD);
										StrLCD("INTERVAL SKIPPED");
										delay_ms(500);
								}
								else if(interval >= 1 && interval <= 999)
								{
										reminder_interval = interval;
										SetNextReminder();
										CmdLCD(CLEAR_LCD);
										StrLCD("INTERVAL SAVED");
										delay_ms(1000);
								}
								else
								{
											CmdLCD(CLEAR_LCD);
											StrLCD("INVALID INTERVAL");
											delay_ms(1000);
											goto L8;
								}
								CmdLCD(CLEAR_LCD);
								CmdLCD(GOTO_LINE1_POS0);
								StrLCD("1.GOAL  2.RTC");
								CmdLCD(GOTO_LINE2_POS0);
								StrLCD("3.INT   C:EXIT");
						}
						else if(key == 'C')
						{
							CmdLCD(CLEAR_LCD);
							StrLCD("EXIT CONFIG");
							delay_ms(500);
							CmdLCD(CLEAR_LCD);
							break;
						}
					}
			 }
		}
}
