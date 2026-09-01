/* Drink button connected to P0.23 */
#define DRINK_BUTTON 23
/* Maximum number of progress icons displayed on the LCD */
#define GLASS_DISPLAY_WIDTH 16
/* Custom LCD character codes for hydration progress */
#define GLASS_FILLED 4
#define GLASS_EMPTY  5
#include "types.h"
/* Hydration and reminder status variables */
extern u8 water_count;
extern u8 daily_goal;
extern u8 reminder_hour;
extern u8 reminder_min;
extern u8 reminder_active;
extern u8 missed_reminders;
/* Reminder timing variables */
extern u32 reminder_interval;
extern u32 reminder_duration;

void DisplayStatus(void);
void CheckReminder(void);
void SetNextReminder(void);
void DrinkWater(void);
void ShowGlassProgress(void);
void MidnightReset(void);
void WelcomeTitle(void);
