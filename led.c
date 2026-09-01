#include <LPC214x.h>
#include "led.h"
// Function to initialize all LED pins
void InitLED(void)
{
		// Configure P1.24, P1.25 and P1.26 as output pins
    IODIR1 |= (1<<YELLOW_LED) |
              (1<<GREEN_LED)  |
              (1<<RED_LED);
		// Turn OFF all LEDs initially
    AllLED_OFF();
}
void YellowLED_ON(void)
{
    IOSET1 = (1<<YELLOW_LED);
}
void YellowLED_OFF(void)
{
    IOCLR1 = (1<<YELLOW_LED);
}

void GreenLED_ON(void)
{
    IOSET1 = (1<<GREEN_LED);
}

void GreenLED_OFF(void)
{
    IOCLR1 = (1<<GREEN_LED);
}

void RedLED_ON(void)
{
    IOSET1 = (1<<RED_LED);
}

void RedLED_OFF(void)
{
    IOCLR1 = (1<<RED_LED);
}
void AllLED_OFF(void)
{
    IOCLR1 = (1<<YELLOW_LED) |
             (1<<GREEN_LED)  |
             (1<<RED_LED);
}
