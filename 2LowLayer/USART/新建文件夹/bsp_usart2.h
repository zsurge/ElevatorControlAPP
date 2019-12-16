/******************************************************************************

                  版权所有 (C), 2013-2023, 深圳博思高科技有限公司

 ******************************************************************************
  文 件 名   : drv_usart2.h
  版 本 号   : 初稿
  作    者   : 张舵
  生成日期   : 2019年5月22日
  最近修改   :
  功能描述   : 串口2 指令接收
  函数列表   :
  修改历史   :
  1.日    期   : 2019年5月22日
    作    者   : 张舵
    修改内容   : 创建文件

******************************************************************************/

#ifndef __DRV_USART2_H
#define __DRV_USART2_H


#include "stdio.h"	
#include "string.h"
#include "stm32f4xx_conf.h"

//引脚定义
/*******************************************************/
#define USART2_CLK                         RCC_APB1Periph_USART2
#define USART2_GPIO_PORT                   GPIOA
#define USART2_GPIO_CLK                    RCC_AHB1Periph_GPIOA

#define USART2_RX_PIN                      GPIO_Pin_3
#define USART2_RX_AF                       GPIO_AF_USART2
#define USART2_RX_SOURCE                   GPIO_PinSource3

#define USART2_TX_PIN                      GPIO_Pin_2
#define USART2_TX_AF                       GPIO_AF_USART2
#define USART2_TX_SOURCE                   GPIO_PinSource2





#define USART2MAXBUFFSIZE 1024

void bsp_Usart2_Init (u32 BaudRate);
void bsp_Usart2_SendString (const u8 *Buff);
void bsp_Usart2_SendData (const u8 *Buff, u16 SendSize);
void bsp_Usart2_RecvReset (void);
u8 bsp_Usart2_RecvOne (u8 *Str);
u16 bsp_Usart2_RecvAtTime (u8 *Buff, u16 RecvSize, u32 timeout_MilliSeconds);
u8 bsp_Usart2_Read(u8 *buf, u16 len);


    
#endif


