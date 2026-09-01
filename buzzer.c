#include <LPC214x.h>
#include "types.h"
#include "delay.h"
#include "buzzer.h"
// Function to initialize the buzzer
void InitBuzzer(void)
{
		// Configure the buzzer pin P1.27 as an output
    IODIR1 |= (1 << BUZZER);
		//buzzer is OFF initially
    BuzzerOFF();
}

void BuzzerON(void)
{
    IOSET1 = (1<<BUZZER);
}

void BuzzerOFF(void)
{
    IOCLR1 = (1<<BUZZER);
}

void Beep(u32 ms)
{
    BuzzerON();
    delay_ms(ms);
		BuzzerOFF();
}

