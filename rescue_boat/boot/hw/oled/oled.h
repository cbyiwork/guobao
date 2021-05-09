#ifndef __OLED_H
#define __OLED_H			  	 
#include "../sys/sys.h"
#include "stdlib.h"	    
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//Mini STM32开发板
//SSD1306 OLED 驱动IC驱动代码
//驱动方式:8080并口/4线串口
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2010/6/3
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 正点原子 2009-2019
//All rights reserved
////////////////////////////////////////////////////////////////////////////////// 	  


//OLED模式设置
//0:4线串行模式
//1:并行8080模式
		    						  
//-----------------OLED端口定义----------------  					   
//#define OLED_CS PBout(15)
//#define OLED_RST PBout(14)
//#define OLED_RS PBout(13)

#define SET_CS(x) {if (x){GPIO_SetBits(GPIOB, GPIO_Pin_15);}else{GPIO_ResetBits(GPIOB, GPIO_Pin_15);}} 
#define SET_RST(x) {if (x){GPIO_SetBits(GPIOB, GPIO_Pin_14);}else{GPIO_ResetBits(GPIOB, GPIO_Pin_14);}} 
#define SET_CD(x) {if (x){GPIO_SetBits(GPIOB, GPIO_Pin_13);}else{GPIO_ResetBits(GPIOB, GPIO_Pin_13);}} 

  
//使用4线串行接口时使用 
//#define OLED_SCLK PBout(10)
//#define OLED_SDIN PBout(11)
#define SET_SCK(x) {if (x){GPIO_SetBits(GPIOB, GPIO_Pin_10);}else{GPIO_ResetBits(GPIOB, GPIO_Pin_10);}} 
#define SET_SDA(x) {if (x){GPIO_SetBits(GPIOB, GPIO_Pin_11);}else{GPIO_ResetBits(GPIOB, GPIO_Pin_11);}} 


#define LCD_PWR_CLR()	GPIO_ResetBits(GPIOB,GPIO_Pin_12)
#define LCD_PWR_SET()	GPIO_SetBits(GPIOB,GPIO_Pin_12)

#define PWR_LED_ON()		GPIO_SetBits(GPIOA,GPIO_Pin_15)
#define PWR_LED_OFF()		GPIO_ResetBits(GPIOA,GPIO_Pin_15)

#define USB_EN()  GPIO_SetBits(GPIOA, GPIO_Pin_10)
		     
#define OLED_CMD  0	//写命令
#define OLED_DATA 1	//写数据
//OLED控制用函数
void OLED_WR_Byte(u8 dat,u8 cmd);	    
void OLED_Display_On(void);
void OLED_Display_Off(void);
void OLED_Refresh_Gram(void);		   
							   		    
void OLED_Init(void);
void OLED_Clear(void);
void OLED_DrawPoint(u8 x,u8 y,u8 t);
void OLED_Fill(u8 x1,u8 y1,u8 x2,u8 y2,u8 dot);
void OLED_ShowChar(u8 x,u8 y,u8 chr,u8 size,u8 mode);
void OLED_ShowNum(u8 x,u8 y,u32 num,u8 len,u8 size);
void OLED_ShowString(u8 x,u8 y,const u8 *p);	 
void refreshScreen(void) ;

extern u8 OLED_GRAM[128][8];

#endif  
	 



