#ifndef __ADC_H__
#define __ADC_H__

#include "stm32f10x.h"
#include <stdio.h>

#define CTL_STICK_ON()		GPIO_ResetBits(GPIOA,GPIO_Pin_8)
#define CTL_STICK_OFF()		GPIO_SetBits(GPIOA,GPIO_Pin_8)

void AdcInit(void);
//#define ADC1_DR_Address ((u32)0x4001244C)
extern uint32_t ADC_ConvertedResult[4];
#define ADC1_DR_Address        ((uint32_t)(&ADC1->DR))
//void getAdcConvertResult(void);
void checkCtlStick(void);

#define ADC_STEP 20

//  实测摇杆不动时，各个通道稳定在900附近
//  ch[0]: 933, ch[1]: 961, ch[2]: 891, ch[3]: 914
//  左满舵: ch[0]: 958, ch[1]: 880, ch[2]: 2307, ch[3]: 511
//  右满舵: ch[0]: 969, ch[1]: 880, ch[2]: 481, ch[3]: 2219
//  前满舵: ch[0]: 2433, ch[1]: 493, ch[2]: 895, ch[3]: 923,
//  后满舵: ch[0]: 537, ch[1]: 2465, ch[2]: 1001, ch[3]: 845,


#endif

