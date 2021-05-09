#ifndef __IAP_H__
#define __IAP_H__
//#include "sys.h"  
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK战舰STM32开发板
//IAP 代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2012/9/24
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved									  
//////////////////////////////////////////////////////////////////////////////////	    
typedef  void (*iapfun)(void);				//定义一个函数类型的参数.   
#define FLASH_APP1_ADDR		0x08005000  	//第一个应用程序起始地址(存放在FLASH)	

//#define FLASH_VER_ADDR		0x0801E000		   //版本号
#define FLASH_APPBASE_ADDR      0x08000000
#define FLASH_VER_ADDR			(FLASH_APPBASE_ADDR+127*1024)//0x0801FC00
#define FLASH_SERNUM_ADDR        (FLASH_VER_ADDR+16)
//#define FLASH_SERNUM_ADDR	       (FLASH_VER_ADDR+4)		// 序列号和硬件地址
//#define FLASH_VERI_SN_ADDR (FLASH_APP1_ADDR+0x1000)
//#define FLASH_VERI_FLAG_ADDR (FLASH_VER_ADDR+16+16)
//#define FLASH_VERI_SN_ADDR         (FLASH_VERI_FLAG_ADDR+4)
//#define FLASH_USER_ADDR		(FLASH_SERNUM_ADDR+16)
//#define FLASH_FACTORY_ADDR	(FLASH_USER_ADDR+128)
//#define FLASH_USER_ADDR		(0x0801E000+1024)
//#define FLASH_USER_ADDR		(FLASH_SERNUM_ADDR+1024)
#define FLASH_USER_ADDR             (FLASH_VER_ADDR+512)
#define FLASH_PARA_SAVE_ADDR   (FLASH_USER_ADDR+256)


#define CPU_ID_BASE				(0x1FFFF7E8)
//#define FLASH_AVAILABLE			(100*1024)
#define FLASH_AVAILABLE			(106*1024)

#define FLASH_VERI_FLAG_ADDR      (FLASH_APPBASE_ADDR+126*1024)//0x0801F800  

											//保留0X08000000~0X08004FFF的空间为Bootloader使用	   
void iap_load_app(u32 appxaddr);			//跳转到APP程序执行
void iap_write_appbin(u32 appxaddr,u8 *appbuf,u32 applen);	//在指定地址开始,写入bin
#endif







































