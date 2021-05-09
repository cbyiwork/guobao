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
u8 const address_0[5]={'L','O','V','E','!'};//ʹ��LOVE��Ϊ��Ƶ����
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
	status = Nrf_WriteByte(reg);//���ͼĴ���ֵ(λ��),����ȡ״ֵ̬
	for(u8_ctr=0; u8_ctr<len; u8_ctr++){
		Nrf_WriteByte(*pBuf++); //д������	
	}
	NRF_SCN(1);
	return status;          //���ض�����״ֵ̬
	}

u8 Nrf_ReadBuf(u8 reg,u8 *pBuf,u8 len)
	{
	u8 status,u8_ctr;	       
	NRF_SCN(0);
	status=Nrf_WriteByte(reg);//���ͼĴ���ֵ(λ��),����ȡ״ֵ̬   	   
	for(u8_ctr=0;u8_ctr<len;u8_ctr++){
		pBuf[u8_ctr]=Nrf_WriteByte(0XFF);//��������
	}
	NRF_SCN(1);
	return status;        //���ض�����״ֵ̬
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

void FIFO_read(u8 data[],u8 lengh)		//��ȡ�������ݻ�����
{
	u8 i;
	NRF_SCN(0);
	Nrf_WriteByte(0x61);	//��ȡ����
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

void NRF_power(u8 P)				//���书������
{
	NRF_CE(0);
	if(P==3){
		Nrf_RegWrite(0x06,0x27);		  //0db ����֮ǰע�ʹ���
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
	Nrf_RegWrite(0x01,0x00); //��ֹ �Զ�Ӧ��
	Nrf_RegWrite(0x02,0x01); //���� P0�ŵ�
	Nrf_RegWrite(0x04,0x00); //��ֹ �Զ��ط�
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
	while(!cancel&connecting)					 //�Ѷ�Ƶ��Ϣ�������ջ������յ��ظ�����ͨ�ųɹ���
	{								 //�ղ�����������
		TX_mode();
		FIFO_write(tx_order,11);
		delay_ms(1);
		RX_mode();
		
		while(1)
		{		
			delay_ms(1);
			if(NRF_WIRQ()==RESET)
			{
				FIFO_read(rx,11);		//��ȡ��������
				NRF_CE(0);
				Nrf_RegWrite(0x07,0x40);	//���RX�ж��ź�
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
	NRF_power(TX_power);//�ָ�����״̬
	NRF_size(11);
	TX_address(address);
	RX_address(address); 
	 		 
}

//���24L01�Ƿ����
//����ֵ:0���ɹ�;1��ʧ��	
u8 NRF24L01_Check(void)
{
	u8 buf[5] = {0XA5,0XA5,0XA5,0XA5,0XA5};
	u8 rBuf[5] = {0x00};
	u8 i = 0;
	Nrf_ReadBuf(0x20+0x10,buf,5);//д��5���ֽڵĵ�ַ.	
	Nrf_ReadBuf(0x10,rBuf,5); //����д��ĵ�ַ  
	
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
		return 1;//���24L01����	
	} else {
		return 0;		 //��⵽24L01
	}
}
*/

	 
const u8 TX_ADDRESS[TX_ADR_WIDTH]={0x34,0x43,0x10,0x10,0x01}; //���͵�ַ
const u8 RX_ADDRESS[RX_ADR_WIDTH]={0x34,0x43,0x10,0x10,0x01}; //���͵�ַ
							    
//��ʼ��24L01��IO��
void NRF24L01_Init(void)
	{
	GPIO_InitTypeDef GPIO_InitStructure;
	//RCC->APB2ENR|=1<<2;    //ʹ��PORTA��ʱ�� 
	//RCC->APB2ENR|=1<<4;    //ʹ��PORTC��ʱ�� 
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOC, ENABLE );	
	//GPIOA->CRL&=0XFFF000FF;//PA4���
	//GPIOA->CRL|=0X00033300; 
	GPIO_InitStructure.GPIO_Pin = NRF24L01_CE_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP ;   //�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(NRF24L01_CE_PORT, &GPIO_InitStructure);
		
	//GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2|GPIO_Pin_3;
	//GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP ;   //�������
	//GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	//GPIO_Init(GPIOA, &GPIO_InitStructure);
	//GPIOA->ODR|=7<<2;	   //PA2.3.4 ���1	
	Set_NRF24L01_CE();	 
	//GPIO_SetBits(GPIOA,GPIO_Pin_2);
	//GPIO_SetBits(GPIOA,GPIO_Pin_3);
	//GPIOC->CRL&=0XFF00FFFF;//PC4��� PC5���
	//GPIOC->CRL|=0X00830000; 
	//GPIOC->ODR|=3<<4;	   //����	 
	GPIO_InitStructure.GPIO_Pin = NRF24L01_CSN_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP ;   //�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(NRF24L01_CSN_PORT, &GPIO_InitStructure);
	Set_NRF24L01_CSN(); 
	GPIO_InitStructure.GPIO_Pin = NRF24L01_IRQ_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU  ;   //��������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(NRF24L01_IRQ_PORT, &GPIO_InitStructure);
	SPIx_Init();    //��ʼ��SPI
	Clr_NRF24L01_CE(); 	//ʹ��24L01
	Set_NRF24L01_CSN();	//SPIƬѡȡ��		  		 		  
	}

//���24L01�Ƿ����
//����ֵ:0���ɹ�;1��ʧ��	
u8 NRF24L01_Check(void)
	{
	u8 buf[5]={0XA5,0XA5,0XA5,0XA5,0XA5};
	u8 i;
	SPIx_SetSpeed(SPI_SPEED_8); //spi�ٶ�Ϊ9Mhz��24L01�����SPIʱ��Ϊ10Mhz��   	 
	NRF24L01_Write_Buf(NRF24L01_WRITE_REG+TX_ADDR,buf,5);//д��5���ֽڵĵ�ַ.	
	NRF24L01_Read_Buf(TX_ADDR,buf,5); //����д��ĵ�ַ  
	for(i=0;i<5;i++)if(buf[i]!=0XA5)break;	 							   
	if(i!=5)return 1;//���24L01����	
	return 0;		 //��⵽24L01
	}	
 	 
//SPIд�Ĵ���
//reg:ָ���Ĵ�����ַ
//value:д���ֵ
u8 NRF24L01_Write_Reg(u8 reg,u8 value)
	{
	u8 status;	
	Clr_NRF24L01_CSN();                 //ʹ��SPI����
	status =SPIx_ReadWriteByte(reg);//���ͼĴ����� 
	SPIx_ReadWriteByte(value);      //д��Ĵ�����ֵ
	Set_NRF24L01_CSN();                 //��ֹSPI����	   
	return(status);       			//����״ֵ̬
	}

//��ȡSPI�Ĵ���ֵ
//reg:Ҫ���ļĴ���
u8 NRF24L01_Read_Reg(u8 reg)
	{
	u8 reg_val;	    
	Clr_NRF24L01_CSN();          //ʹ��SPI����		
	SPIx_ReadWriteByte(reg);   //���ͼĴ�����
	reg_val=SPIx_ReadWriteByte(0XFF);//��ȡ�Ĵ�������
	Set_NRF24L01_CSN();          //��ֹSPI����		    
	return(reg_val);           //����״ֵ̬
	}	

//��ָ��λ�ö���ָ�����ȵ�����
//reg:�Ĵ���(λ��)
//*pBuf:����ָ��
//len:���ݳ���
//����ֵ,�˴ζ�����״̬�Ĵ���ֵ 
u8 NRF24L01_Read_Buf(u8 reg,u8 *pBuf,u8 len)
	{
	u8 status,u8_ctr;	       
	Clr_NRF24L01_CSN();           //ʹ��SPI����
	status=SPIx_ReadWriteByte(reg);//���ͼĴ���ֵ(λ��),����ȡ״ֵ̬   	   
	for(u8_ctr=0;u8_ctr<len;u8_ctr++)pBuf[u8_ctr]=SPIx_ReadWriteByte(0XFF);//��������
	Set_NRF24L01_CSN();       //�ر�SPI����
	return status;        //���ض�����״ֵ̬
	}

//��ָ��λ��дָ�����ȵ�����
//reg:�Ĵ���(λ��)
//*pBuf:����ָ��
//len:���ݳ���
//����ֵ,�˴ζ�����״̬�Ĵ���ֵ
u8 NRF24L01_Write_Buf(u8 reg, u8 *pBuf, u8 len)
	{
	u8 status,u8_ctr;	    
	Clr_NRF24L01_CSN();          //ʹ��SPI����
	status = SPIx_ReadWriteByte(reg);//���ͼĴ���ֵ(λ��),����ȡ״ֵ̬
	for(u8_ctr=0; u8_ctr<len; u8_ctr++)SPIx_ReadWriteByte(*pBuf++); //д������	 
	Set_NRF24L01_CSN();       //�ر�SPI����
	return status;          //���ض�����״ֵ̬
	}
				   
//����NRF24L01����һ������
//txbuf:�����������׵�ַ
//����ֵ:�������״��
u8 NRF24L01_TxPacket(u8 *txbuf)
	{
	u8 sta;
	SPIx_SetSpeed(SPI_SPEED_8);//spi�ٶ�Ϊ9Mhz��24L01�����SPIʱ��Ϊ10Mhz��   
	Clr_NRF24L01_CE();
	NRF24L01_Write_Buf(NRF24L01_WR_TX_PLOAD,txbuf,TX_PLOAD_WIDTH);//д���ݵ�TX BUF  32���ֽ�
	Set_NRF24L01_CE();//��������	   
	while(NRF24L01_IRQ()!=0);//�ȴ��������
	sta=NRF24L01_Read_Reg(STATUS);  //��ȡ״̬�Ĵ�����ֵ	   
	NRF24L01_Write_Reg(NRF24L01_WRITE_REG+STATUS,sta); //���TX_DS��MAX_RT�жϱ�־
	if(sta&MAX_TX)//�ﵽ����ط�����
		{
		NRF24L01_Write_Reg(NRF24L01_FLUSH_TX,0xff);//���TX FIFO�Ĵ��� 
		return MAX_TX; 
		}
	if(sta&TX_OK)//�������
		{
		return TX_OK;
		}
	return 0xff;//����ԭ����ʧ��
	}

//����NRF24L01����һ������
//txbuf:�����������׵�ַ
//����ֵ:0��������ɣ��������������
u8 NRF24L01_RxPacket(u8 *rxbuf) {
	u8 sta;		    							   
	SPIx_SetSpeed(SPI_SPEED_8); //spi�ٶ�Ϊ9Mhz��24L01�����SPIʱ��Ϊ10Mhz��   
	sta=NRF24L01_Read_Reg(STATUS);  //��ȡ״̬�Ĵ�����ֵ    	 
	NRF24L01_Write_Reg(NRF24L01_WRITE_REG+STATUS,sta); //���TX_DS��MAX_RT�жϱ�־
	if(sta&RX_OK) {
		
		NRF24L01_Read_Buf(NRF24L01_RD_RX_PLOAD,rxbuf,RX_PLOAD_WIDTH);//��ȡ����
		NRF24L01_Write_Reg(NRF24L01_FLUSH_RX,0xff);//���RX FIFO�Ĵ��� 
		return 0; 
	}	   
	return 1;//û�յ��κ�����
}

void ClrRxFifo() {
	NRF24L01_Write_Reg(NRF24L01_FLUSH_RX,0xff);
}
					    
//�ú�����ʼ��NRF24L01��RXģʽ
//����RX��ַ,дRX���ݿ��,ѡ��RFƵ��,�����ʺ�LNA HCURR
//��CE��ߺ�,������RXģʽ,�����Խ���������		   
void RX_Mode(void)
	{
	Clr_NRF24L01_CE();	  
	NRF24L01_Write_Buf(NRF24L01_WRITE_REG+RX_ADDR_P0,(u8*)RX_ADDRESS,RX_ADR_WIDTH);//дRX�ڵ��ַ
	
	//NRF24L01_Write_Reg(NRF24L01_WRITE_REG+EN_AA,0x01);    //ʹ��ͨ��0���Զ�Ӧ��    
	NRF24L01_Write_Reg(NRF24L01_WRITE_REG+EN_AA,0x00);
	NRF24L01_Write_Reg(NRF24L01_WRITE_REG+EN_RXADDR,0x01);//ʹ��ͨ��0�Ľ��յ�ַ  	 
	NRF24L01_Write_Reg(NRF24L01_WRITE_REG+SETUP_RETR,0x00);
	NRF24L01_Write_Reg(NRF24L01_WRITE_REG+RF_CH,40);	     //����RFͨ��Ƶ��		  
	NRF24L01_Write_Reg(NRF24L01_WRITE_REG+RX_PW_P0,RX_PLOAD_WIDTH);//ѡ��ͨ��0����Ч���ݿ�� 	    
	NRF24L01_Write_Reg(NRF24L01_WRITE_REG+RF_SETUP,0x0f);//����TX�������,0db����,2Mbps,���������濪��   
	NRF24L01_Write_Reg(NRF24L01_WRITE_REG+CONFIG, 0x0f);//���û�������ģʽ�Ĳ���;PWR_UP,EN_CRC,16BIT_CRC,����ģʽ 
	Set_NRF24L01_CE(); //CEΪ��,�������ģʽ 
	}
							 
//�ú�����ʼ��NRF24L01��TXģʽ
//����TX��ַ,дTX���ݿ��,����RX�Զ�Ӧ��ĵ�ַ,���TX��������,ѡ��RFƵ��,�����ʺ�LNA HCURR
//PWR_UP,CRCʹ��
//��CE��ߺ�,������RXģʽ,�����Խ���������		   
//CEΪ�ߴ���10us,����������.	 
void TX_Mode(void)
	{														 
	Clr_NRF24L01_CE();	    
	NRF24L01_Write_Buf(NRF24L01_WRITE_REG+TX_ADDR,(u8*)TX_ADDRESS,TX_ADR_WIDTH);//дTX�ڵ��ַ 
	NRF24L01_Write_Buf(NRF24L01_WRITE_REG+RX_ADDR_P0,(u8*)RX_ADDRESS,RX_ADR_WIDTH); //����TX�ڵ��ַ,��ҪΪ��ʹ��ACK	  
	
	//NRF24L01_Write_Reg(NRF24L01_WRITE_REG+EN_AA,0x01);     //ʹ��ͨ��0���Զ�Ӧ��    
	NRF24L01_Write_Reg(NRF24L01_WRITE_REG+EN_AA,0x00);
	NRF24L01_Write_Reg(NRF24L01_WRITE_REG+EN_RXADDR,0x01); //ʹ��ͨ��0�Ľ��յ�ַ  
	//NRF24L01_Write_Reg(NRF24L01_WRITE_REG+SETUP_RETR,0x1a);//�����Զ��ط����ʱ��:500us + 86us;����Զ��ط�����:10��
	NRF24L01_Write_Reg(NRF24L01_WRITE_REG+SETUP_RETR,0x00);
	NRF24L01_Write_Reg(NRF24L01_WRITE_REG+RF_CH,40);       //����RFͨ��Ϊ40
	NRF24L01_Write_Reg(NRF24L01_WRITE_REG+RF_SETUP,0x0f);  //����TX�������,0db����,2Mbps,���������濪��   
	NRF24L01_Write_Reg(NRF24L01_WRITE_REG+CONFIG,0x0e);    //���û�������ģʽ�Ĳ���;PWR_UP,EN_CRC,16BIT_CRC,����ģʽ,���������ж�
	Set_NRF24L01_CE();//CEΪ��,10us����������
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
	
	// 1.ʹ��GPIO��AFIOʱ��,ֵ��ע����ǣ���ʹ���ⲿ�жϵ�ʱ�����ʹ��AFIOʱ�ӡ
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_AFIO,ENABLE);

  // 2.GPIO��ʼ��
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;  //��������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  //IO�ٶ�Ϊ50MHz
	GPIO_Init(GPIOB,&GPIO_InitStructure);
    
	// 3.����EXTI��
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource2);  //��EXIT��9���ӵ�PB9
	EXTI_InitStructure.EXTI_Line = EXTI_Line2;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;     //�����½��ش���
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;//ʹ���ж���
  EXTI_Init(&EXTI_InitStructure);//��ʼ���ж�
		
		
  // 4.�ж�����
  NVIC_InitStructure.NVIC_IRQChannel = EXTI2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
}

void EXTI2_IRQHandler(void) {
	
  if(EXTI_GetITStatus(EXTI_Line2) != RESET){
		  // nrf24l01 �����жϣ�������Ӧ����
		//dprint("ext2 interrupt\r\n");
	}
	EXTI_ClearITPendingBit(EXTI_Line2);
	
}


