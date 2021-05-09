/**
  ******************************************************************************
  * @file    main.h
  * $Author: wdluo $
  * $Revision: 67 $
  * $Date:: 2012-08-15 19:00:29 +0800 #$
  * @brief   主函数包含的头文件.
  ******************************************************************************
  * @attention
  *
  *<h3><center>&copy; Copyright 2009-2012, ViewTool</center>
  *<center><a href="http:\\www.viewtool.com">http://www.viewtool.com</a></center>
  *<center>All Rights Reserved</center></h3>
  * 
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H
/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_conf.h"

/* Exported Functions --------------------------------------------------------*/
void Stm32_Clock_Init(u8 PLL);

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
extern uint8_t USB_Received_Flag;
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
#define USBHID_REC_LEN	128
#define RCV_CMD_LEN 16

#define UPDATE_FAST

#define BITBAND(addr, bitnum) ((addr & 0xF0000000)+0x2000000+((addr &0xFFFFF)<<5)+(bitnum<<2)) 
#define MEM_ADDR(addr)  *((volatile unsigned long  *)(addr)) 
#define BIT_ADDR(addr, bitnum)   MEM_ADDR(BITBAND(addr, bitnum)) 


#define CHARGE_LED1 		PDout(14)
#define CHARGE_LED2 		PDout(15)
#define D3V3_EN				PBout(12)
#define CC2530_RESET      PDout(7)
#define CC2530_PWR         PBout(6)
//#define RF_RESET			PBout(6)
//#define CHARGE_LED 		PAout(15)
#define CHARGE_LED1_ON()			{CHARGE_LED1=0;}
#define CHARGE_LED1_OFF()		{CHARGE_LED1=1;}

#define CHARGE_LED2_ON()			{CHARGE_LED2=0;}
#define CHARGE_LED2_OFF()		{CHARGE_LED2=1;}

#define STM32_IAP_WRITE_PAGE            0x41
#define STM32_IAP_ERASE_DEVICE	      0x42

#define STM32_IAP_CHECK_SUM		       0x44
#define STM32_IAP_READ_STATUS		       0x45
#define STM32_IAP_SECURITYCHECK	       0x46
#define STM32_IAP_READ_64BYTE            0x47 

#define STM32_IAP_WRITE_OK		0x48
#define STM32_IAP_VER			0x49
#define STM32_IAP_SERNUM		0x4A
#define STM32_IAP_USER			0x4B
#define STM32_IAP_START		0x4C

#define STM32_IAP_VERIRQ			0x50
#define STM32_IAP_CC2530			0x51

#define VALID_INIT_SN				(978137684)
#define VALID_START_SN				(260636838)


//7E FF 47 00 02 FF FF 46
//u8 usbHidRxBuf[USBHID_REC_LEN] __attribute__ ((at(0X20001000)));
//0x1FFFF7E8
u8 usbHidRxBuf[USBHID_REC_LEN];
//char *g_decKey = "WorkForYourDream";
char *g_decKey = "MakeTomorrowBest";			//NBK闪光灯

char g_cupIdKey[16] = {0x24,0x98,0xb7,0xA0,0xCB,0xDF,0xE6,0x5D,0xD1,0x4B,0xF7,0x51,0xA6,0x01,0x98,0x22};
//char g_cupIdRst[16] = {0xE8,0xF7,0xff,0x1f,0xCB,0xDF,0xE6,0x5D,0xD1,0x4B,0xF7,0x51,0xA6,0x01,0x98,0x22};
char g_cupIdRst[16] = {0x08,0x3f,0x37,0x9f,0xb6,0xe6,0x5d,0x1f,0x3d,0xb9,0xdb,0x9c,0x4f,0xf3,0x6f,0xf0};

unsigned char start_flag=0;

#define ucLDR_Message (*(STCOMMANDER *)usbHidRxBuf)

char *g_bootVer = "A001";

extern u8 nonside[];

/**
  * @brief  串口打印输出
  * @param  None
  * @retval None
  */
typedef struct U_STCOMMANDER
{
	unsigned char head;
	unsigned char cmd_no;
	unsigned char cmd;
	unsigned char rsv;                        // use as start address
	unsigned char data_len;
 	char buf[RCV_CMD_LEN];
	unsigned char sum;
	//unsigned char sum;
}STCOMMANDER;



#endif/* __MAIN_H */

/*********************************END OF FILE**********************************/
