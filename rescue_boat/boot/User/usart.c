/**
  ******************************************************************************
  * @file    usart.c
  * $Author: wdluo $
  * $Revision: 67 $
  * $Date:: 2012-08-15 19:00:29 +0800 #$
  * @brief   串口相关函数。
  ******************************************************************************
  * @attention
  *
  *<h3><center>&copy; Copyright 2009-2012, ViewTool</center>
  *<center><a href="http:\\www.viewtool.com">http://www.viewtool.com</a></center>
  *<center>All Rights Reserved</center></h3>
  * 
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "usart.h"

/** @addtogroup USART
  * @brief 串口模块
  * @{
  */

#ifdef __GNUC__
/* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
     set to 'Yes') calls __io_putchar() */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */


/**
  * @brief  开启GPIOA,串口1时钟 
  * @param  None
  * @retval None
  * @note  对于某些GPIO上的默认复用功能可以不开启服用时钟，如果用到复用功能的重
           映射，则需要开启复用时钟
  */
void USART_RCC_Configuration(void)
{
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);//开复用时钟
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_USART1,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB1Periph_USART2,ENABLE);
}

/**
  * @brief  设置串口1发送与接收引脚的模式 
  * @param  None
  * @retval None
  */
void USART_GPIO_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	//GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;

	GPIO_Init(GPIOA, &GPIO_InitStruct);

	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	//GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3;
	
	GPIO_Init(GPIOA, &GPIO_InitStruct);
}

/**
  * @brief  配置串口1，并使能串口1
  * @param  None
  * @retval None
  */
void USART_Configuration(void)
{
	USART_InitTypeDef USART_InitStruct;

	USART_RCC_Configuration();

	RCC->APB1RSTR|=1<<17;   //复位串口1
	RCC->APB1RSTR&=~(1<<17);//停止复位	 

//	GPIOA->CRH&=0xFFFFF00F; 
//	GPIOA->CRH|=0x000008B0;//IO状态设置

	USART_InitStruct.USART_BaudRate = 115200;
	USART_InitStruct.USART_StopBits = USART_StopBits_1;
	USART_InitStruct.USART_WordLength = USART_WordLength_8b;
	USART_InitStruct.USART_Parity = USART_Parity_No;
	USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
	
//	USART_Init(USART1, &USART_InitStruct);
	USART_Init(USART2, &USART_InitStruct);
//	USART_ITConfig(USART1,USART_IT_RXNE,ENABLE);//使能接收中断
//	USART_Cmd(USART1, ENABLE);//使能串口1
	USART_Cmd(USART2, ENABLE);

	//USART_GPIO_Configuration();

	//RCC->APB2RSTR|=1<<14;   //复位串口1
	//RCC->APB2RSTR&=~(1<<14);//停止复位	 
	GPIOA->CRL&=0xFFFF00FF; 
	GPIOA->CRL|=0x00008B00;//IO状态设置
	
}

void uart2_init(u32 pclk2,u32 bound)
{  	 
	float temp;
	u16 mantissa;
	u16 fraction;	   
	temp=(float)(pclk2*1000000)/(bound*16);//得到USARTDIV
	mantissa=temp;				 //得到整数部分
	fraction=(temp-mantissa)*16; //得到小数部分	 
    	mantissa<<=4;
	mantissa+=fraction; 
	RCC->APB2ENR|=1<<2;   //使能PORTA口时钟  
	RCC->APB1ENR|=1<<17;  //使能串口2 时钟 
	//GPIOA->CRH&=0XFFFFF00F; 
	//GPIOA->CRH|=0X000008B0;//IO状态设置
	GPIOA->CRL&=0XFFFF00FF;
	GPIOA->CRL|=0X00008B00;

	
	RCC->APB1RSTR|=1<<17;   //复位串口2
	RCC->APB1RSTR&=~(1<<17);//停止复位	   	   
	//波特率设置
 	USART2->BRR=mantissa; // 波特率设置	 
	USART2->CR1|=0X200C;  //1位停止,无校验位.
	
}

void uart1_init(u32 pclk2,u32 bound)
{  	 
	float temp;
	u16 mantissa;
	u16 fraction;	   
	temp=(float)(pclk2*1000000)/(bound*16);//得到USARTDIV
	mantissa=temp;				 //得到整数部分
	fraction=(temp-mantissa)*16; //得到小数部分	 
    	mantissa<<=4;
	mantissa+=fraction; 
	RCC->APB2ENR|=1<<2;   //使能PORTA口时钟  
	RCC->APB2ENR|=1<<14;  //使能串口时钟 
	GPIOA->CRH&=0XFFFFFF0F; 
	GPIOA->CRH|=0X000000B0;//IO状态设置
		  
	RCC->APB2RSTR|=1<<14;   //复位串口1
	RCC->APB2RSTR&=~(1<<14);//停止复位	   	   
	//波特率设置
 	USART1->BRR=mantissa; // 波特率设置	 
	USART1->CR1|=0X200C;  //1位停止,无校验位.
	
}


void U2SendData(u8 dat)
{
	USART2->DR = dat;
	while(!(USART2->SR&(1<<6)));
	USART2->SR = USART2->SR;

}

void uartSendData(unsigned char *buf, unsigned char len)
{
	unsigned char i;
	for(i=0;i<len;i++)
	{
		//USART_SendData(USART2,buf[i]);//putchar(buf[i]);
		//USART_SendData();
		U2SendData(buf[i]);
	}
}

/*
void uartSendData(unsigned char *buf, unsigned char len)
{
	unsigned char i;
	for(i=0;i<len;i++)
	{
		USART_SendData(USART2,buf[i]);//putchar(buf[i]);
		//USART_SendData();
	}
}
*/


PUTCHAR_PROTOTYPE
{
	/* Place your implementation of fputc here */
	/* e.g. write a character to the USART */
	USART_SendData(USART1,(u8)ch);
	//USART_SendData(USART2,(u8)ch);

	/* Loop until the end of transmission */
	while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
	//while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);

	return ch;
}
/**
  * @}
  */

/*********************************END OF FILE**********************************/

