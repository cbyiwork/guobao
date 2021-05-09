#ifndef __TIMER_H__
#define __TIMER_H__

#include "stm32f10x.h"
#include <stdio.h>

void TIM3_Configuration(void);
u32 getTickMs(u32* tmr);
void refreshTimer(u32* tmr);

#endif
