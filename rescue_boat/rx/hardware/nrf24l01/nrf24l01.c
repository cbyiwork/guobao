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
u8 const address_0[5]={'L','O','V','E','!'};//使用LOVE作为对频暗语
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
	status = Nrf_WriteByte(reg);//发送寄存器值(位置),并读取状态值
	for(u8_ctr=0; u8_ctr<len; u8_ctr++){
		Nrf_WriteByte(*pBuf++); //写入数据	
	}
	NRF_SCN(1);
	return status;          //返回读到的状态值
	}

u8 Nrf_ReadBuf(u8 reg,u8 *pBuf,u8 len)
	{
	u8 status,u8_ctr;	       
	NRF_SCN(0);
	status=Nrf_WriteByte(reg);//发送寄存器值(位置),并读取状态值   	   
	for(u8_ctr=0;u8_ctr<len;u8_ctr++){
		pBuf[u8_ctr]=Nrf_WriteByte(0XFF);//读出数据
	}
	NRF_SCN(1);
	return status;        //返回读到的状态值
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

void FIFO_read(u8 data[],u8 lengh)		//读取接收数据缓冲区
{
	u8 i;
	NRF_SCN(0);
	Nrf_WriteByte(0x61);	//读取命令
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

void NRF_power(u8 P)				//发射功率设置
{
	NRF_CE(0);
	if(P==3){
		Nrf_RegWrite(0x06,0x27);		  //0db 修正之前注释错误
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
	Nrf_RegWrite(0x01,0x00); //禁止 自动应答
	Nrf_RegWrite(0x02,0x01); //允许 P0信道
	Nrf_RegWrite(0x04,0x00); //禁止 自动重发
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
	while(!cancel&connecting)					 //把对频信息发给接收机，若收到回复表面通信成功，
	{								 //收不到继续发送
		TX_mode();
		FIFO_write(tx_order,11);
		delay_ms(1);
		RX_mode();
		
		while(1)
		{		
			delay_ms(1);
			if(NRF_WIRQ()==RESET)
			{
				FIFO_read(rx,11);		//读取接收数据
				NRF_CE(0);
				Nrf_RegWrite(0x07,0x40);	//清除RX中断信号
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
	NRF_power(TX_power);//恢复发射状态
	NRF_size(11);
	TX_address(address);
	RX_address(address); 
	 		 
}

//检测24L01是否存在
//返回值:0，成功;1，失败	
u8 NRF24L01_Check(void)
{
	u8 buf[5] = {0XA5,0XA5,0XA5,0XA5,0XA5};
	u8 rBuf[5] = {0x00};
	u8 i = 0;
	Nrf_ReadBuf(0x20+0x10,buf,5);//写入5个字节的地址.	
	Nrf_ReadBuf(0x10,rBuf,5); //读出写入的地址  
	
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
		return 1;//检测24L01错误	
	} else {
		return 0;		 //检测到24L01
	}
}
*/

	 
const u8 TX_ADDRESS[TX_ADR_WIDTH]={0x34,0x43,0x10,0x10,0x01}; //发送地址
const u8 RX_ADDRESS[RX_ADR_WIDTH]={0x34,0x43,0x10,0x10,0x01}; //发送地址
							    
//初始化24L01的IO口
void NRF24L01_Init(void)
	{
	GPIO_InitTypeDef GPIO_InitStructure;
	//RCC->APB2ENR|=1<<2;    //使能PORTA口时钟 
	//RCC->APB2ENR|=1<<4;    //使能PORTC口时钟 
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOC, ENABLE );	
	//GPIOA->CRL&=0XFFF000FF;//PA4输出
	//GPIOA->CRL|=0X00033300; 
	GPIO_InitStructure.GPIO_Pin = NRF24L01_CE_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP ;   //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(NRF24L01_CE_PORT, &GPIO_InitStructure);
		
	//GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2|GPIO_Pin_3;
	//GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP ;   //推挽输出
	//GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	//GPIO_Init(GPIOA, &GPIO_InitStructure);
	//GPIOA->ODR|=7<<2;	   //PA2.3.4 输出1	
	Set_NRF24L01_CE();	 
	//GPIO_SetBits(GPIOA,GPIO_Pin_2);
	//GPIO_SetBits(GPIOA,GPIO_Pin_3);
	//GPIOC->CRL&=0XFF00FFFF;//PC4输出 PC5输出
	//GPIOC->CRL|=0X00830000; 
	//GPIOC->ODR|=3<<4;	   //上拉	 
	GPIO_InitStructure.GPIO_Pin = NRF24L01_CSN_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP ;   //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(NRF24L01_CSN_PORT, &GPIO_InitStructure);
	Set_NRF24L01_CSN(); 
	GPIO_InitStructure.GPIO_Pin = NRF24L01_IRQ_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU  ;   //上拉输入
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(NRF24L01_IRQ_PORT, &GPIO_InitStructure);
	SPIx_Init();    //初始化SPI
	Clr_NRF24L01_CE(); 	//使能24L01
	Set_NRF24L01_CSN();	//SPI片选取消		  		 		  
	}

//检测24L01是否存在
//返回值:0，成功;1，失败	
u8 NRF24L01_Check(void)
	{
	u8 buf[5]={0XA5,0XA5,0XA5,0XA5,0XA5};
	u8 i;
	SPIx_SetSpeed(SPI_SPEED_8); //spi速度为9Mhz（24L01的最大SPI时钟为10Mhz）   	 
	NRF24L01_Write_Buf(NRF24L01_WRITE_REG+TX_ADDR,buf,5);//写入5个字节的地址.	
	NRF24L01_Read_Buf(TX_ADDR,buf,5); //读出写入的地址  
	for(i=0;i<5;i++)if(buf[i]!=0XA5)break;	 							   
	if(i!=5)return 1;//检测24L01错误	
	return 0;		 //检测到24L01
	}	
 	 
//SPI写寄存器
//reg:指定寄存器地址
//value:写入的值
u8 NRF24L01_Write_Reg(u8 reg,u8 value)
	{
	u8 status;	
	Clr_NRF24L01_CSN();                 //使能SPI传输
	status =SPIx_ReadWriteByte(reg);//发送寄存器号 
	SPIx_ReadWriteByte(value);      //写入寄存器的值
	Set_NRF24L01_CSN();                 //禁止SPI传输	   
	return(status);       			//返回状态值
	}

//读取SPI寄存器值
//reg:要读的寄存器
u8 NRF24L01_Read_Reg(u8 reg)
	{
	u8 reg_val;	    
	Clr_NRF24L01_CSN();          //使能SPI传输		
	SPIx_ReadWriteByte(reg);   //发送寄存器号
	reg_val=SPIx_ReadWriteByte(0XFF);//读取寄存器内容
	Set_NRF24L01_CSN();          //禁止SPI传输		    
	return(reg_val);           //返回状态值
	}	

//在指定位置读出指定长度的数据
//reg:寄存器(位置)
//*pBuf:数据指针
//len:数据长度
//返回值,此次读到的状态寄存器值 
u8 NRF24L01_Read_Buf(u8 reg,u8 *pBuf,u8 len)
	{
	u8 status,u8_ctr;	       
	Clr_NRF24L01_CSN();           //使能SPI传输
	status=SPIx_ReadWriteByte(reg);//发送寄存器值(位置),并读取状态值   	   
	for(u8_ctr=0;u8_ctr<len;u8_ctr++)pBuf[u8_ctr]=SPIx_ReadWriteByte(0XFF);//读出数据
	Set_NRF24L01_CSN();       //关闭SPI传输
	return status;        //返回读到的状态值
	}

//在指定位置写指定长度的数据
//reg:寄存器(位置)
//*pBuf:数据指针
//len:数据长度
//返回值,此次读到的状态寄存器值
u8 NRF24L01_Write_Buf(u8 reg, u8 *pBuf, u8 len)
	{
	u8 status,u8_ctr;	    
	Clr_NRF24L01_CSN();          //使能SPI传输
	status = SPIx_ReadWriteByte(reg);//发送寄存器值(位置),并读取状态值
	for(u8_ctr=0; u8_ctr<len; u8_ctr++)SPIx_ReadWriteByte(*pBuf++); //写入数据	 
	Set_NRF24L01_CSN();       //关闭SPI传输
	return status;          //返回读到的状态值
	}
				   
//启动NRF24L01发送一次数据
//txbuf:待发送数据首地址
//返回值:发送完成状况
u8 NRF24L01_TxPacket(u8 *txbuf)
	{
	u8 sta;
	SPIx_SetSpeed(SPI_SPEED_8);//spi速度为9Mhz（24L01的最大SPI时钟为10Mhz）   
	Clr_NRF24L01_CE();
	NRF24L01_Write_Buf(NRF24L01_WR_TX_PLOAD,txbuf,TX_PLOAD_WIDTH);//写数据到TX BUF  32个字节
	Set_NRF24L01_CE();//启动发送	   
	while(NRF24L01_IRQ()!=0);//等待发送完成
	sta=NRF24L01_Read_Reg(STATUS);  //读取状态寄存器的值	   
	NRF24L01_Write_Reg(NRF24L01_WRITE_REG+STATUS,sta); //清除TX_DS或MAX_RT中断标志
	if(sta&MAX_TX)//达到最大重发次数
		{
		NRF24L01_Write_Reg(NRF24L01_FLUSH_TX,0xff);//清除TX FIFO寄存器 
		return MAX_TX; 
		}
	if(sta&TX_OK)//发送完成
		{
		return TX_OK;
		}
	return 0xff;//其他原因发送失败
	}

//启动NRF24L01发送一次数据
//txbuf:待发送数据首地址
//返回值:0，接收完成；其他，错误代码
u8 NRF24L01_RxPacket(u8 *rxbuf) {
	u8 sta;		    							   
	SPIx_SetSpeed(SPI_SPEED_8); //spi速度为9Mhz（24L01的最大SPI时钟为10Mhz）   
	sta=NRF24L01_Read_Reg(STATUS);  //读取状态寄存器的值    	 
	NRF24L01_Write_Reg(NRF24L01_WRITE_REG+STATUS,sta); //清除TX_DS或MAX_RT中断标志
	if(sta&RX_OK) {
		
		NRF24L01_Read_Buf(NRF24L01_RD_RX_PLOAD,rxbuf,RX_PLOAD_WIDTH);//读取数据
		NRF24L01_Write_Reg(NRF24L01_FLUSH_RX,0xff);//清除RX FIFO寄存器 
		return 0; 
	}	   
	return 1;//没收到任何数据
}

void ClrRxFifo() {
	NRF24L01_Write_Reg(NRF24L01_FLUSH_RX,0xff);
}
					    
//该函数初始化NRF24L01到RX模式
//设置RX地址,写RX数据宽度,选择RF频道,波特率和LNA HCURR
//当CE变高后,即进入RX模式,并可以接收数据了		   
void RX_Mode(void)
	{
	Clr_NRF24L01_CE();	  
	NRF24L01_Write_Buf(NRF24L01_WRITE_REG+RX_ADDR_P0,(u8*)RX_ADDRESS,RX_ADR_WIDTH);//写RX节点地址
	
	//NRF24L01_Write_Reg(NRF24L01_WRITE_REG+EN_AA,0x01);    //使能通道0的自动应答    
	NRF24L01_Write_Reg(NRF24L01_WRITE_REG+EN_AA,0x00);
	NRF24L01_Write_Reg(NRF24L01_WRITE_REG+EN_RXADDR,0x01);//使能通道0的接收地址  	 
	NRF24L01_Write_Reg(NRF24L01_WRITE_REG+SETUP_RETR,0x00);
	NRF24L01_Write_Reg(NRF24L01_WRITE_REG+RF_CH,40);	     //设置RF通信频率		  
	NRF24L01_Write_Reg(NRF24L01_WRITE_REG+RX_PW_P0,RX_PLOAD_WIDTH);//选择通道0的有效数据宽度 	    
	NRF24L01_Write_Reg(NRF24L01_WRITE_REG+RF_SETUP,0x0f);//设置TX发射参数,0db增益,2Mbps,低噪声增益开启   
	NRF24L01_Write_Reg(NRF24L01_WRITE_REG+CONFIG, 0x0f);//配置基本工作模式的参数;PWR_UP,EN_CRC,16BIT_CRC,接收模式 
	Set_NRF24L01_CE(); //CE为高,进入接收模式 
	}
							 
//该函数初始化NRF24L01到TX模式
//设置TX地址,写TX数据宽度,设置RX自动应答的地址,填充TX发送数据,选择RF频道,波特率和LNA HCURR
//PWR_UP,CRC使能
//当CE变高后,即进入RX模式,并可以接收数据了		   
//CE为高大于10us,则启动发送.	 
void TX_Mode(void)
	{														 
	Clr_NRF24L01_CE();	    
	NRF24L01_Write_Buf(NRF24L01_WRITE_REG+TX_ADDR,(u8*)TX_ADDRESS,TX_ADR_WIDTH);//写TX节点地址 
	NRF24L01_Write_Buf(NRF24L01_WRITE_REG+RX_ADDR_P0,(u8*)RX_ADDRESS,RX_ADR_WIDTH); //设置TX节点地址,主要为了使能ACK	  
	
	//NRF24L01_Write_Reg(NRF24L01_WRITE_REG+EN_AA,0x01);     //使能通道0的自动应答    
	NRF24L01_Write_Reg(NRF24L01_WRITE_REG+EN_AA,0x00);
	NRF24L01_Write_Reg(NRF24L01_WRITE_REG+EN_RXADDR,0x01); //使能通道0的接收地址  
	//NRF24L01_Write_Reg(NRF24L01_WRITE_REG+SETUP_RETR,0x1a);//设置自动重发间隔时间:500us + 86us;最大自动重发次数:10次
	NRF24L01_Write_Reg(NRF24L01_WRITE_REG+SETUP_RETR,0x00);
	NRF24L01_Write_Reg(NRF24L01_WRITE_REG+RF_CH,40);       //设置RF通道为40
	NRF24L01_Write_Reg(NRF24L01_WRITE_REG+RF_SETUP,0x0f);  //设置TX发射参数,0db增益,2Mbps,低噪声增益开启   
	NRF24L01_Write_Reg(NRF24L01_WRITE_REG+CONFIG,0x0e);    //配置基本工作模式的参数;PWR_UP,EN_CRC,16BIT_CRC,接收模式,开启所有中断
	Set_NRF24L01_CE();//CE为高,10us后启动发送
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
	
	// 1.使能GPIO和AFIO时钟,值得注意的是，当使用外部中断的时候必须使能AFIO时钟?
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_AFIO,ENABLE);

  // 2.GPIO初始化
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;  //浮空输入
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  //IO速度为50MHz
	GPIO_Init(GPIOB,&GPIO_InitStructure);
    
	// 3.设置EXTI线
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource2);  //将EXIT线9连接到PB9
	EXTI_InitStructure.EXTI_Line = EXTI_Line2;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;     //上升下降沿触发
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;//使能中断线
  EXTI_Init(&EXTI_InitStructure);//初始化中断
		
		
  // 4.中断向量
  NVIC_InitStructure.NVIC_IRQChannel = EXTI2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
}

void EXTI2_IRQHandler(void) {
	
  if(EXTI_GetITStatus(EXTI_Line2) != RESET){
		  // nrf24l01 接收中断，需做相应处理
		//dprint("ext2 interrupt\r\n");
	}
	EXTI_ClearITPendingBit(EXTI_Line2);
	
}


