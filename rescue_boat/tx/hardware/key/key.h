#ifndef __KEY_H_
#define __KEY_H_
#include "stm32f10x.h"
//Mini STM32开发板
//SPI 驱动 V1.1
//正点原子@ALIENTEK
//2010/5/13	
						
void KeyInit(void);
u8 KeyScan(void);
void KeyPorc(void);

/*
#define KEY6_VAL  ((~0x01)&0x7f)
#define KEY4_VAL  ((~0x02)&0x7f)
#define KEY3_VAL  ((~0x04)&0x7f)
#define KEY2_VAL  ((~0x08)&0x7f)
#define KEY1_VAL  ((~0x10)&0x7f)   //key1
#define KEY5_VAL  ((~0x20)&0x7f)
#define KEY7_VAL  ((~0x40)&0x7f)
#define KEYNULL    0x7f
*/
#define KEYNULL    0x208

extern u8 g_srMode;
		 
#endif

