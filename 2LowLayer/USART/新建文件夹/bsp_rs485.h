#ifndef __BSP_RS485_H
#define __BSP_RS485_H			 
#include "sys.h"	 	

//引脚定义
/*******************************************************/
//#define USART2_CLK                         RCC_APB1Periph_USART2
//#define USART2_GPIO_PORT                   GPIOA
//#define USART2_GPIO_CLK                    RCC_AHB1Periph_GPIOA

//#define USART2_RX_PIN                      GPIO_Pin_3
//#define USART2_RX_AF                       GPIO_AF_USART2
//#define USART2_RX_SOURCE                   GPIO_PinSource3

//#define USART2_TX_PIN                      GPIO_Pin_2
//#define USART2_TX_AF                       GPIO_AF_USART2
//#define USART2_TX_SOURCE                   GPIO_PinSource2

//#define RCC_RS485_TXEN                     RCC_AHB1Periph_GPIOE
//#define PORT_RS485_TXEN                    GPIOE
//#define PIN_RS485_TXEN                     GPIO_Pin_2


//模式控制
#define RS485_TX_EN		                    PGout(0)	//485模式控制.0,接收;1,发送.

//#define RS485_RX_EN()   GPIO_ResetBits(PORT_RS485_TXEN,PIN_RS485_TXEN);
//#define RS485_TX_EN()   GPIO_SetBits(PORT_RS485_TXEN,PIN_RS485_TXEN);



#define USART5_CLK                         RCC_APB1Periph_UART5
#define USART5_AF                          GPIO_AF_UART5


#define USART5_GPIO_RX_PORT                GPIOD
#define USART5_GPIO_RX_CLK                 RCC_AHB1Periph_GPIOD

#define USART5_GPIO_TX_PORT                GPIOC
#define USART5_GPIO_TX_CLK                 RCC_AHB1Periph_GPIOC

			
#define USART5_RX_PIN                      GPIO_Pin_2
#define USART5_RX_SOURCE                   GPIO_PinSource2

#define USART5_TX_PIN                      GPIO_Pin_12
#define USART5_TX_SOURCE                   GPIO_PinSource12

#define RCC_RS485_TXEN                     RCC_AHB1Periph_GPIOG
#define PORT_RS485_TXEN                    GPIOG
#define PIN_RS485_TXEN                     GPIO_Pin_0



#define RS485_MAX_LEN                       255

	  	
extern uint8_t RS485_RX_BUF[RS485_MAX_LEN]; 		//接收缓冲,最大64个字节
extern uint8_t RS485_RX_CNT;   			//接收到的数据长度







														 
void bsp_RS485_Init(uint32_t bound);
void bsp_RS485_Send_Data(uint8_t *buf,uint16_t len);
void bsp_RS485_Receive_Data(uint8_t *buf,uint16_t *len);		 
//uint16_t RS485_Receive_Data (uint8_t *buf,uint16_t len,uint32_t timeout_MilliSeconds);

#endif	   
















