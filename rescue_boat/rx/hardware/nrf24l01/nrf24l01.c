#include "nrf24l01.h"
#include "../sys/delay.h"
#include "stdlib.h"
#include "stdio.h"
#include "../spi/spi.h"
#include "../uart/uart.h"

/**************************************************************************/
u8 rx_state;
u8 tx_buff[11],tx_data[11],rx[11]={0,0,0};
u8 tx_order[11];
u8 address[5];
u8 hopping[5];
u8 const address_0[5]={'L','O','V','E','!'};//Ê¹ÓÃLOVE×÷Îª¶ÔÆµ°µÓï
u8 TX_power=3;
u8 cancel,connecting;

/*
void Nrf_PortInit() {
	GPIO_InitTypeDef gpio_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB, ENABLE);

	gpio_InitStructure.GPIO_Pin = GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_7;
	gpio_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	gpio_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&gpio_InitStructure);

	gpio_InitStructure.GPIO_Pin = GPIO_Pin_1;                 // w_ce
	gpio_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	gpio_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&gpio_InitStructure);

	gpio_InitStructure.GPIO_Pin = GPIO_Pin_6;									// miso
	gpio_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	gpio_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&gpio_InitStructure);

	gpio_InitStructure.GPIO_Pin = GPIO_Pin_2;									// wirq
	gpio_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	gpio_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&gpio_InitStructure);

	
}

//NRF24L01+
u8 Nrf_WriteByte(u8 byte)
{
	u8 i;
	for(i=0;i<8;i++)
	{
		NRF_MOSI(byte&0x80);
		NRF_SCK(1);
		byte<<=1;
		byte|=NRF_MISO();
		NRF_SCK(0);
	}
	return byte;
}

void Nrf_RegWrite(u8 address,u8 command)
{
	NRF_SCN(0);
	Nrf_WriteByte(0x20+address);
	Nrf_WriteByte(command);
	NRF_SCN(1);
}

u8 Nrf_RegRead(u8 address)
{
	u8 m;
	NRF_SCN(0);
	Nrf_WriteByte(address);
	m=Nrf_WriteByte(0);
	NRF_SCN(1);
	return m;
}

u8 Nrf_WriteBuf(u8 reg, u8 *pBuf, u8 len)
	{
	u8 status,u8_ctr;	    
	NRF_SCN(0);
	status = Nrf_WriteByte(reg);//·¢ËÍ¼Ä´æÆ÷Öµ(Î»ÖÃ),²¢¶ÁÈ¡×´Ì¬Öµ
	for(u8_ctr=0; u8_ctr<len; u8_ctr++){
		Nrf_WriteByte(*pBuf++); //Ð´ÈëÊý¾Ý	
	}
	NRF_SCN(1);
	return status;          //·µ»Ø¶Áµ½µÄ×´Ì¬Öµ
	}

u8 Nrf_ReadBuf(u8 reg,u8 *pBuf,u8 len)
	{
	u8 status,u8_ctr;	       
	NRF_SCN(0);
	status=Nrf_WriteByte(reg);//·¢ËÍ¼Ä´æÆ÷Öµ(Î»ÖÃ),²¢¶ÁÈ¡×´Ì¬Öµ   	   
	for(u8_ctr=0;u8_ctr<len;u8_ctr++){
		pBuf[u8_ctr]=Nrf_WriteByte(0XFF);//¶Á³öÊý¾Ý
	}
	NRF_SCN(1);
	return status;        //·µ»Ø¶Áµ½µÄ×´Ì¬Öµ
	}

void FIFO_write(u8 data[],u8 lengh)
{
	u8 i;
	NRF_SCN(0);
	Nrf_WriteByte(0xa0);
	for(i=0;i<lengh;i++){
		Nrf_WriteByte(data[i]);
	}
	NRF_SCN(1);
}

void FIFO_read(u8 data[],u8 lengh)		//¶ÁÈ¡½ÓÊÕÊý¾Ý»º³åÇø
{
	u8 i;
	NRF_SCN(0);
	Nrf_WriteByte(0x61);	//¶ÁÈ¡ÃüÁî
	for(i=0;i<lengh;i++) {
		data[i]=Nrf_WriteByte(0);	   
	}
	NRF_SCN(1);
}

void TX_address(u8 taddr[])
{
	NRF_SCN(0);	 
	Nrf_WriteByte(0x20+0x10);
	Nrf_WriteByte(taddr[0]);
	Nrf_WriteByte(taddr[1]);
	Nrf_WriteByte(taddr[2]);
	Nrf_WriteByte(taddr[3]);
	Nrf_WriteByte(taddr[4]);
	NRF_SCN(1);
}  
void RX_address(u8 raddr[])
{
	NRF_SCN(0);	 	 
	Nrf_WriteByte(0x20+0x0a);
	Nrf_WriteByte(raddr[0]);
	Nrf_WriteByte(raddr[1]);
	Nrf_WriteByte(raddr[2]);
	Nrf_WriteByte(raddr[3]);
	Nrf_WriteByte(raddr[4]);
	NRF_SCN(1);  
}
void RX_mode()				 
{
	NRF_CE(0);
	Nrf_RegWrite(0x00,0x3b); //CRC,8 bit,Power on,RX
	NRF_CE(1);
} 				   
	
void TX_mode()				 
{
	NRF_CE(0);
	Nrf_RegWrite(0x00,0x0a);
  	NRF_CE(1);
}

void NRF_power(u8 P)				//·¢Éä¹¦ÂÊÉèÖÃ
{
	NRF_CE(0);
	if(P==3){
		Nrf_RegWrite(0x06,0x27);		  //0db ÐÞÕýÖ®Ç°×¢ÊÍ´íÎó
	} else if(P==2) {
	 	Nrf_RegWrite(0x06,0x25);	  //-6db
	} else if(P==1) {
		Nrf_RegWrite(0x06,0x23);	  //-12db
	} else if(P==0) {
		Nrf_RegWrite(0x06,0x21);    //-18db
	}
	NRF_CE(1);
}

void NRF_size(u8 l)
{
	NRF_CE(0);
	Nrf_RegWrite(0x11,l);  
	NRF_CE(1);
}

void NRF_channel(u8 c)
{
	NRF_CE(0);
	Nrf_RegWrite(0x05,c);  
	NRF_CE(1);
}


void NRF_init()
{	
	Nrf_PortInit();
	NRF_CE(0);
	NRF_SCK(0);
	Nrf_RegWrite(0x01,0x00); //½ûÖ¹ ×Ô¶¯Ó¦´ð
	Nrf_RegWrite(0x02,0x01); //ÔÊÐí P0ÐÅµÀ
	Nrf_RegWrite(0x04,0x00); //½ûÖ¹ ×Ô¶¯ÖØ·¢
	TX_mode(); //REG_write(0x1d,0x01);			 
	NRF_channel(66);
	NRF_power(TX_power);
	NRF_size(11);
	TX_address(address);
	RX_address(address);
}

void device_connect()
{
	u8 n;
	
	rx_state = 0;
	 
	tx_order[0]=0xa0;
	tx_order[1]=hopping[0];
	tx_order[2]=hopping[1];
	tx_order[3]=hopping[2];
	tx_order[4]=hopping[3];
	tx_order[5]=hopping[4];
	tx_order[6]=address[0];
	tx_order[7]=address[1];
	tx_order[8]=address[2];
	tx_order[9]=address[3];
	tx_order[10]=address[4];	
		 
	NRF_channel(33);
	NRF_power(0);
	NRF_size(11);
	TX_address((u8*)address_0);
	RX_address((u8*)address_0);
	
	cancel=0;
	connecting=1;
	while(!cancel&connecting)					 //°Ñ¶ÔÆµÐÅÏ¢·¢¸ø½ÓÊÕ»ú£¬ÈôÊÕµ½»Ø¸´±íÃæÍ¨ÐÅ³É¹¦£¬
	{								 //ÊÕ²»µ½¼ÌÐø·¢ËÍ
		TX_mode();
		FIFO_write(tx_order,11);
		delay_ms(1);
		RX_mode();
		
		while(1)
		{		
			delay_ms(1);
			if(NRF_WIRQ()==RESET)
			{
				FIFO_read(rx,11);		//¶ÁÈ¡½ÓÊÕÊý¾Ý
				NRF_CE(0);
				Nrf_RegWrite(0x07,0x40);	//Çå³ýRXÖÐ¶ÏÐÅºÅ
				NRF_CE(1);	
				if(rx[0]=='O'&&rx[1]=='K')
				{
					cancel=0;
					connecting=0;
				}
			}
				
			n++;if(n>50){n=0;break;}
		}
	}


	rx_state = 1;
	NRF_power(TX_power);//»Ö¸´·¢Éä×´Ì¬
	NRF_size(11);
	TX_address(address);
	RX_address(address); 
	 		 
}

//¼ì²â24L01ÊÇ·ñ´æÔÚ
//·µ»ØÖµ:0£¬³É¹¦;1£¬Ê§°Ü	
u8 NRF24L01_Check(void)
{
	u8 buf[5] = {0XA5,0XA5,0XA5,0XA5,0XA5};
	u8 rBuf[5] = {0x00};
	u8 i = 0;
	Nrf_ReadBuf(0x20+0x10,buf,5);//Ð´Èë5¸ö×Ö½ÚµÄµØÖ·.	
	Nrf_ReadBuf(0x10,rBuf,5); //¶Á³öÐ´ÈëµÄµØÖ·  
	
	printf("read 24l01 addr:\r\n");
	for (i=0;i<5;i++){
		printf("%d, ", rBuf[i]);
	}
	printf("\r\n");
	
	for(i=0;i<5;i++) {
		if(buf[i]!=rBuf[i]) {
			break;	 
		}
	}
	if(i!=5){
		return 1;//¼ì²â24L01´íÎó	
	} else {
		return 0;		 //¼ì²âµ½24L01
	}
}
*/

	 
const u8 TX_ADDRESS[TX_ADR_WIDTH]={0x34,0x43,0x10,0x10,0x01}; //·¢ËÍµØÖ·
const u8 RX_ADDRESS[RX_ADR_WIDTH]={0x34,0x43,0x10,0x10,0x01}; //·¢ËÍµØÖ·
							    
//³õÊ¼»¯24L01µÄIO¿Ú
void NRF24L01_Init(void)
	{
	GPIO_InitTypeDef GPIO_InitStructure;
	//RCC->APB2ENR|=1<<2;    //Ê¹ÄÜPORTA¿ÚÊ±ÖÓ 
	//RCC->APB2ENR|=1<<4;    //Ê¹ÄÜPORTC¿ÚÊ±ÖÓ 
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOC, ENABLE );	
	//GPIOA->CRL&=0XFFF000FF;//PA4Êä³ö
	//GPIOA->CRL|=0X00033300; 
	GPIO_InitStructure.GPIO_Pin = NRF24L01_CE_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP ;   //ÍÆÍìÊä³ö
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(NRF24L01_CE_PORT, &GPIO_InitStructure);
		
	//GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2|GPIO_Pin_3;
	//GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP ;   //ÍÆÍìÊä³ö
	//GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	//GPIO_Init(GPIOA, &GPIO_InitStructure);
	//GPIOA->ODR|=7<<2;	   //PA2.3.4 Êä³ö1	
	Set_NRF24L01_CE();	 
	//GPIO_SetBits(GPIOA,GPIO_Pin_2);
	//GPIO_SetBits(GPIOA,GPIO_Pin_3);
	//GPIOC->CRL&=0XFF00FFFF;//PC4Êä³ö PC5Êä³ö
	//GPIOC->CRL|=0X00830000; 
	//GPIOC->ODR|=3<<4;	   //ÉÏÀ­	 
	GPIO_InitStructure.GPIO_Pin = NRF24L01_CSN_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP ;   //ÍÆÍìÊä³ö
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(NRF24L01_CSN_PORT, &GPIO_InitStructure);
	Set_NRF24L01_CSN(); 
	GPIO_InitStructure.GPIO_Pin = NRF24L01_IRQ_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU  ;   //ÉÏÀ­ÊäÈë
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(NRF24L01_IRQ_PORT, &GPIO_InitStructure);
	SPIx_Init();    //³õÊ¼»¯SPI
	Clr_NRF24L01_CE(); 	//Ê¹ÄÜ24L01
	Set_NRF24L01_CSN();	//SPIÆ¬Ñ¡È¡Ïû		  		 		  
	}

//¼ì²â24L01ÊÇ·ñ´æÔÚ
//·µ»ØÖµ:0£¬³É¹¦;1£¬Ê§°Ü	
u8 NRF24L01_Check(void)
	{
	u8 buf[5]={0XA5,0XA5,0XA5,0XA5,0XA5};
	u8 i;
	SPIx_SetSpeed(SPI_SPEED_8); //spiËÙ¶ÈÎª9Mhz£¨24L01µÄ×î´óSPIÊ±ÖÓÎª10Mhz£©   	 
	NRF24L01_Write_Buf(NRF24L01_WRITE_REG+TX_ADDR,buf,5);//Ð´Èë5¸ö×Ö½ÚµÄµØÖ·.	
	NRF24L01_Read_Buf(TX_ADDR,buf,5); //¶Á³öÐ´ÈëµÄµØÖ·  
	for(i=0;i<5;i++)if(buf[i]!=0XA5)break;	 							   
	if(i!=5)return 1;//¼ì²â24L01´íÎó	
	return 0;		 //¼ì²âµ½24L01
	}	
 	 
//SPIÐ´¼Ä´æÆ÷
//reg:Ö¸¶¨¼Ä´æÆ÷µØÖ·
//value:Ð´ÈëµÄÖµ
u8 NRF24L01_Write_Reg(u8 reg,u8 value)
	{
	u8 status;	
	Clr_NRF24L01_CSN();                 //Ê¹ÄÜSPI´«Êä
	status =SPIx_ReadWriteByte(reg);//·¢ËÍ¼Ä´æÆ÷ºÅ 
	SPIx_ReadWriteByte(value);      //Ð´Èë¼Ä´æÆ÷µÄÖµ
	Set_NRF24L01_CSN();                 //½ûÖ¹SPI´«Êä	   
	return(status);       			//·µ»Ø×´Ì¬Öµ
	}

//¶ÁÈ¡SPI¼Ä´æÆ÷Öµ
//reg:Òª¶ÁµÄ¼Ä´æÆ÷
u8 NRF24L01_Read_Reg(u8 reg)
	{
	u8 reg_val;	    
	Clr_NRF24L01_CSN();          //Ê¹ÄÜSPI´«Êä		
	SPIx_ReadWriteByte(reg);   //·¢ËÍ¼Ä´æÆ÷ºÅ
	reg_val=SPIx_ReadWriteByte(0XFF);//¶ÁÈ¡¼Ä´æÆ÷ÄÚÈÝ
	Set_NRF24L01_CSN();          //½ûÖ¹SPI´«Êä		    
	return(reg_val);           //·µ»Ø×´Ì¬Öµ
	}	

//ÔÚÖ¸¶¨Î»ÖÃ¶Á³öÖ¸¶¨³¤¶ÈµÄÊý¾Ý
//reg:¼Ä´æÆ÷(Î»ÖÃ)
//*pBuf:Êý¾ÝÖ¸Õë
//len:Êý¾Ý³¤¶È
//·µ»ØÖµ,´Ë´Î¶Áµ½µÄ×´Ì¬¼Ä´æÆ÷Öµ 
u8 NRF24L01_Read_Buf(u8 reg,u8 *pBuf,u8 len)
	{
	u8 status,u8_ctr;	       
	Clr_NRF24L01_CSN();           //Ê¹ÄÜSPI´«Êä
	status=SPIx_ReadWriteByte(reg);//·¢ËÍ¼Ä´æÆ÷Öµ(Î»ÖÃ),²¢¶ÁÈ¡×´Ì¬Öµ   	   
	for(u8_ctr=0;u8_ctr<len;u8_ctr++)pBuf[u8_ctr]=SPIx_ReadWriteByte(0XFF);//¶Á³öÊý¾Ý
	Set_NRF24L01_CSN();       //¹Ø±ÕSPI´«Êä
	return status;        //·µ»Ø¶Áµ½µÄ×´Ì¬Öµ
	}

//ÔÚÖ¸¶¨Î»ÖÃÐ´Ö¸¶¨³¤¶ÈµÄÊý¾Ý
//reg:¼Ä´æÆ÷(Î»ÖÃ)
//*pBuf:Êý¾ÝÖ¸Õë
//len:Êý¾Ý³¤¶È
//·µ»ØÖµ,´Ë´Î¶Áµ½µÄ×´Ì¬¼Ä´æÆ÷Öµ
u8 NRF24L01_Write_Buf(u8 reg, u8 *pBuf, u8 len)
	{
	u8 status,u8_ctr;	    
	Clr_NRF24L01_CSN();          //Ê¹ÄÜSPI´«Êä
	status = SPIx_ReadWriteByte(reg);//·¢ËÍ¼Ä´æÆ÷Öµ(Î»ÖÃ),²¢¶ÁÈ¡×´Ì¬Öµ
	for(u8_ctr=0; u8_ctr<len; u8_ctr++)SPIx_ReadWriteByte(*pBuf++); //Ð´ÈëÊý¾Ý	 
	Set_NRF24L01_CSN();       //¹Ø±ÕSPI´«Êä
	return status;          //·µ»Ø¶Áµ½µÄ×´Ì¬Öµ
	}
				   
//Æô¶¯NRF24L01·¢ËÍÒ»´ÎÊý¾Ý
//txbuf:´ý·¢ËÍÊý¾ÝÊ×µØÖ·
//·µ»ØÖµ:·¢ËÍÍê³É×´¿ö
u8 NRF24L01_TxPacket(u8 *txbuf)
	{
	u8 sta;
	SPIx_SetSpeed(SPI_SPEED_8);//spiËÙ¶ÈÎª9Mhz£¨24L01µÄ×î´óSPIÊ±ÖÓÎª10Mhz£©   
	Clr_NRF24L01_CE();
	NRF24L01_Write_Buf(NRF24L01_WR_TX_PLOAD,txbuf,TX_PLOAD_WIDTH);//Ð´Êý¾Ýµ½TX BUF  32¸ö×Ö½Ú
	Set_NRF24L01_CE();//Æô¶¯·¢ËÍ	   
	while(NRF24L01_IRQ()!=0);//µÈ´ý·¢ËÍÍê³É
	sta=NRF24L01_Read_Reg(STATUS);  //¶ÁÈ¡×´Ì¬¼Ä´æÆ÷µÄÖµ	   
	NRF24L01_Write_Reg(NRF24L01_WRITE_REG+STATUS,sta); //Çå³ýTX_DS»òMAX_RTÖÐ¶Ï±êÖ¾
	if(sta&MAX_TX)//´ïµ½×î´óÖØ·¢´ÎÊý
		{
		NRF24L01_Write_Reg(NRF24L01_FLUSH_TX,0xff);//Çå³ýTX FIFO¼Ä´æÆ÷ 
		return MAX_TX; 
		}
	if(sta&TX_OK)//·¢ËÍÍê³É
		{
		return TX_OK;
		}
	return 0xff;//ÆäËûÔ­Òò·¢ËÍÊ§°Ü
	}

//Æô¶¯NRF24L01·¢ËÍÒ»´ÎÊý¾Ý
//txbuf:´ý·¢ËÍÊý¾ÝÊ×µØÖ·
//·µ»ØÖµ:0£¬½ÓÊÕÍê³É£»ÆäËû£¬´íÎó´úÂë
u8 NRF24L01_RxPacket(u8 *rxbuf) {
	u8 sta;		    							   
	SPIx_SetSpeed(SPI_SPEED_8); //spiËÙ¶ÈÎª9Mhz£¨24L01µÄ×î´óSPIÊ±ÖÓÎª10Mhz£©   
	sta=NRF24L01_Read_Reg(STATUS);  //¶ÁÈ¡×´Ì¬¼Ä´æÆ÷µÄÖµ    	 
	NRF24L01_Write_Reg(NRF24L01_WRITE_REG+STATUS,sta); //Çå³ýTX_DS»òMAX_RTÖÐ¶Ï±êÖ¾
	if(sta&RX_OK) {
		
		NRF24L01_Read_Buf(NRF24L01_RD_RX_PLOAD,rxbuf,RX_PLOAD_WIDTH);//¶ÁÈ¡Êý¾Ý
		NRF24L01_Write_Reg(NRF24L01_FLUSH_RX,0xff);//Çå³ýRX FIFO¼Ä´æÆ÷ 
		return 0; 
	}	   
	return 1;//Ã»ÊÕµ½ÈÎºÎÊý¾Ý
}

void ClrRxFifo() {
	NRF24L01_Write_Reg(NRF24L01_FLUSH_RX,0xff);
}
					    
//¸Ãº¯Êý³õÊ¼»¯NRF24L01µ½RXÄ£Ê½
//ÉèÖÃRXµØÖ·,Ð´RXÊý¾Ý¿í¶È,Ñ¡ÔñRFÆµµÀ,²¨ÌØÂÊºÍLNA HCURR
//µ±CE±ä¸ßºó,¼´½øÈëRXÄ£Ê½,²¢¿ÉÒÔ½ÓÊÕÊý¾ÝÁË		   
void RX_Mode(void)
	{
	Clr_NRF24L01_CE();	  
	NRF24L01_Write_Buf(NRF24L01_WRITE_REG+RX_ADDR_P0,(u8*)RX_ADDRESS,RX_ADR_WIDTH);//Ð´RX½ÚµãµØÖ·
	
	//NRF24L01_Write_Reg(NRF24L01_WRITE_REG+EN_AA,0x01);    //Ê¹ÄÜÍ¨µÀ0µÄ×Ô¶¯Ó¦´ð    
	NRF24L01_Write_Reg(NRF24L01_WRITE_REG+EN_AA,0x00);
	NRF24L01_Write_Reg(NRF24L01_WRITE_REG+EN_RXADDR,0x01);//Ê¹ÄÜÍ¨µÀ0µÄ½ÓÊÕµØÖ·  	 
	NRF24L01_Write_Reg(NRF24L01_WRITE_REG+SETUP_RETR,0x00);
	NRF24L01_Write_Reg(NRF24L01_WRITE_REG+RF_CH,40);	     //ÉèÖÃRFÍ¨ÐÅÆµÂÊ		  
	NRF24L01_Write_Reg(NRF24L01_WRITE_REG+RX_PW_P0,RX_PLOAD_WIDTH);//Ñ¡ÔñÍ¨µÀ0µÄÓÐÐ§Êý¾Ý¿í¶È 	    
	NRF24L01_Write_Reg(NRF24L01_WRITE_REG+RF_SETUP,0x0f);//ÉèÖÃTX·¢Éä²ÎÊý,0dbÔöÒæ,2Mbps,µÍÔëÉùÔöÒæ¿ªÆô   
	NRF24L01_Write_Reg(NRF24L01_WRITE_REG+CONFIG, 0x0f);//ÅäÖÃ»ù±¾¹¤×÷Ä£Ê½µÄ²ÎÊý;PWR_UP,EN_CRC,16BIT_CRC,½ÓÊÕÄ£Ê½ 
	Set_NRF24L01_CE(); //CEÎª¸ß,½øÈë½ÓÊÕÄ£Ê½ 
	}
							 
//¸Ãº¯Êý³õÊ¼»¯NRF24L01µ½TXÄ£Ê½
//ÉèÖÃTXµØÖ·,Ð´TXÊý¾Ý¿í¶È,ÉèÖÃRX×Ô¶¯Ó¦´ðµÄµØÖ·,Ìî³äTX·¢ËÍÊý¾Ý,Ñ¡ÔñRFÆµµÀ,²¨ÌØÂÊºÍLNA HCURR
//PWR_UP,CRCÊ¹ÄÜ
//µ±CE±ä¸ßºó,¼´½øÈëRXÄ£Ê½,²¢¿ÉÒÔ½ÓÊÕÊý¾ÝÁË		   
//CEÎª¸ß´óÓÚ10us,ÔòÆô¶¯·¢ËÍ.	 
void TX_Mode(void)
	{														 
	Clr_NRF24L01_CE();	    
	NRF24L01_Write_Buf(NRF24L01_WRITE_REG+TX_ADDR,(u8*)TX_ADDRESS,TX_ADR_WIDTH);//Ð´TX½ÚµãµØÖ· 
	NRF24L01_Write_Buf(NRF24L01_WRITE_REG+RX_ADDR_P0,(u8*)RX_ADDRESS,RX_ADR_WIDTH); //ÉèÖÃTX½ÚµãµØÖ·,Ö÷ÒªÎªÁËÊ¹ÄÜACK	  
	
	//NRF24L01_Write_Reg(NRF24L01_WRITE_REG+EN_AA,0x01);     //Ê¹ÄÜÍ¨µÀ0µÄ×Ô¶¯Ó¦´ð    
	NRF24L01_Write_Reg(NRF24L01_WRITE_REG+EN_AA,0x00);
	NRF24L01_Write_Reg(NRF24L01_WRITE_REG+EN_RXADDR,0x01); //Ê¹ÄÜÍ¨µÀ0µÄ½ÓÊÕµØÖ·  
	//NRF24L01_Write_Reg(NRF24L01_WRITE_REG+SETUP_RETR,0x1a);//ÉèÖÃ×Ô¶¯ÖØ·¢¼ä¸ôÊ±¼ä:500us + 86us;×î´ó×Ô¶¯ÖØ·¢´ÎÊý:10´Î
	NRF24L01_Write_Reg(NRF24L01_WRITE_REG+SETUP_RETR,0x00);
	NRF24L01_Write_Reg(NRF24L01_WRITE_REG+RF_CH,40);       //ÉèÖÃRFÍ¨µÀÎª40
	NRF24L01_Write_Reg(NRF24L01_WRITE_REG+RF_SETUP,0x0f);  //ÉèÖÃTX·¢Éä²ÎÊý,0dbÔöÒæ,2Mbps,µÍÔëÉùÔöÒæ¿ªÆô   
	NRF24L01_Write_Reg(NRF24L01_WRITE_REG+CONFIG,0x0e);    //ÅäÖÃ»ù±¾¹¤×÷Ä£Ê½µÄ²ÎÊý;PWR_UP,EN_CRC,16BIT_CRC,½ÓÊÕÄ£Ê½,¿ªÆôËùÓÐÖÐ¶Ï
	Set_NRF24L01_CE();//CEÎª¸ß,10usºóÆô¶¯·¢ËÍ
}


void NrfPower(u8 p) {
	Clr_NRF24L01_CE();
	if (p == 3) {
		NRF24L01_Write_Reg(0x06, 0x27);   // 0db
	} else if (p==2) {
		NRF24L01_Write_Reg(0x06, 0x25);   // -6db
	} else if (p==1) {
		NRF24L01_Write_Reg(0x06, 0x23);   // -12db
	} else if (p==0) {
		NRF24L01_Write_Reg(0x06, 0x21);   //-18db
	}
	Set_NRF24L01_CE();

}


void EXTI_PB2_Init(void) {
	
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
	
	// 1.Ê¹ÄÜGPIOºÍAFIOÊ±ÖÓ,ÖµµÃ×¢ÒâµÄÊÇ£¬µ±Ê¹ÓÃÍâ²¿ÖÐ¶ÏµÄÊ±ºò±ØÐëÊ¹ÄÜAFIOÊ±ÖÓ¡
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_AFIO,ENABLE);

  // 2.GPIO³õÊ¼»¯
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;  //¸¡¿ÕÊäÈë
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  //IOËÙ¶ÈÎª50MHz
	GPIO_Init(GPIOB,&GPIO_InitStructure);
    
	// 3.ÉèÖÃEXTIÏß
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource2);  //½«EXITÏß9Á¬½Óµ½PB9
	EXTI_InitStructure.EXTI_Line = EXTI_Line2;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;     //ÉÏÉýÏÂ½µÑØ´¥·¢
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;//Ê¹ÄÜÖÐ¶ÏÏß
  EXTI_Init(&EXTI_InitStructure);//³õÊ¼»¯ÖÐ¶Ï
		
		
  // 4.ÖÐ¶ÏÏòÁ¿
  NVIC_InitStructure.NVIC_IRQChannel = EXTI2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
}

void EXTI2_IRQHandler(void) {
	
  if(EXTI_GetITStatus(EXTI_Line2) != RESET){
		  // nrf24l01 ½ÓÊÕÖÐ¶Ï£¬Ðè×öÏàÓ¦´¦Àí
		//dprint("ext2 interrupt\r\n");
	}
	EXTI_ClearITPendingBit(EXTI_Line2);
	
}


