#include "types.h"
extern volatile u8 MenuFlag;
void Init_EINT0(void);
void EINT0_ISR(void)__irq;

