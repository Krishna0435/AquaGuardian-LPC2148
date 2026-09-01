#include <LPC214x.h>
#include "interrupt.h"

volatile u8 MenuFlag = 0;
#define EINT0_PIN_FUNC 0x0000000C
#define EINT0_VIC_CHNO 14
/************************************************
Initialize External Interrupt-0
P0.1 --> EINT0
************************************************/
void Init_EINT0(void)
{
    /* Select EINT0 Function on P0.1*/
    PINSEL0 &= ~(3<<(1*2));
    PINSEL0 |=  EINT0_PIN_FUNC;

    /* Edge Triggered */
    EXTMODE |= (1<<0);

    /* Falling Edge Trigger */
    EXTPOLAR &= ~(1<<0);

    /* Clear Interrupt Flag */
    EXTINT = (1<<0);

    /* Enable EINT0 Interrupt */
    VICIntSelect &= ~(1<<14);

    VICVectAddr0 = (u32)EINT0_ISR;

    VICVectCntl0 = (1<<5)|EINT0_VIC_CHNO;

    VICIntEnable = (1<<EINT0_VIC_CHNO);
}

/************************************************
EINT0 ISR
************************************************/
void EINT0_ISR(void)__irq
{
    MenuFlag = 1;

    EXTINT = (1<<0);

    VICVectAddr = 0;
}
