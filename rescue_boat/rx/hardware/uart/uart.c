#include "uart.h"

/** @addtogroup USART
  * @brief ????
  * @{
  */

#ifdef __GNUC__
/* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
     set to 'Yes') calls __io_putchar() */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */

/*****************************************************
*function:	?????1
*param:		?????
*return:		
******************************************************/
void USART1_Init(unsigned int BaudRate)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef	USART_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);			//??USART1,GPIOA??
	
	/* TX - PA.9 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;							//PA.9
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;							//??????
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	/* RX - PA.10 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;							//PA.10
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;						//????
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	USART_InitStructure.USART_BaudRate = BaudRate;							//???
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;					//??8?
	USART_InitStructure.USART_StopBits = USART_StopBits_1;						//???1?
	USART_InitStructure.USART_Parity = USART_Parity_No;						//?????
	USART_InitStructure.USART_HardwareFlowControl	= USART_HardwareFlowControl_None;		//????????
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;					//?/???
	
	USART_Init(USART1, &USART_InitStructure);
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);							//??????
	USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);							//??????
	USART_Cmd(USART1, ENABLE);
}

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

