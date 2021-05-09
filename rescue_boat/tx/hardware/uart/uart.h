#ifndef __UART_H__
#define __UART_H__

#include "stm32f10x.h"
#include <stdio.h>

void USART1_Init(unsigned int BaudRate);

#define UART_DEBUG

#ifdef UART_DEBUG
#define dprint(arg,fmt...) printf(arg,##fmt)
#else
#define dprint(arg,fmt...) /*##*/
#endif

#endif
