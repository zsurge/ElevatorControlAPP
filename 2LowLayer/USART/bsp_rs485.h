#ifndef __BSP_RS485_H
#define __BSP_RS485_H			 
#include "sys.h"	 	

//引脚定义
/*******************************************************/
#define USART2_CLK                         RCC_APB1Periph_USART2
#define USART2_GPIO_PORT                   GPIOD
#define USART2_GPIO_CLK                    RCC_AHB1Periph_GPIOD
#define USART2_AF                          GPIO_AF_USART2


#define USART2_RX_PIN                      GPIO_Pin_6
#define USART2_RX_SOURCE                   GPIO_PinSource6
#define USART2_TX_PIN                      GPIO_Pin_5
#define USART2_TX_SOURCE                   GPIO_PinSource5

#define RCC_RS485_TXEN                     RCC_AHB1Periph_GPIOE
#define PORT_RS485_TXEN                    GPIOE
#define PIN_RS485_TXEN                     GPIO_Pin_1


//模式控制
#define RS485_TX_EN		                    PEout(1)	//485模式控制.0,接收;1,发送.

//#define RS485_RX_EN()   GPIO_ResetBits(PORT_RS485_TXEN,PIN_RS485_TXEN);
//#define RS485_TX_EN()   GPIO_SetBits(PORT_RS485_TXEN,PIN_RS485_TXEN);



#define RS485_MAX_LEN                       255

	  	
extern uint8_t RS485_RX_BUF[RS485_MAX_LEN]; //接收缓冲,最大64个字节
extern uint8_t RS485_RX_CNT;   			    //接收到的数据长度







														 
void bsp_RS485_Init(uint32_t bound);
void bsp_RS485_Send_Data(uint8_t *buf,uint16_t len);
void bsp_RS485_Receive_Data(uint8_t *buf,uint16_t *len);		 
//uint16_t RS485_Receive_Data (uint8_t *buf,uint16_t len,uint32_t timeout_MilliSeconds);

#endif	   
















