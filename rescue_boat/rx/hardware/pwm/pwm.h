#ifndef __PWM_H__
#define __PWM_H__

#include "stm32f10x.h"
#include <stdio.h>

void TIM2_Configuration(uint32_t period);
void TIM1_Configuration(uint32_t period);
void setDuty(uint8_t ch, uint16_t duty);

#endif
