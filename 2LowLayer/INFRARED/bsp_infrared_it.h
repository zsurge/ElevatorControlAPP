/******************************************************************************

                  版权所有 (C), 2013-2023, 深圳博思高科技有限公司

 ******************************************************************************
  文 件 名   : bsp_infrared_it.h
  版 本 号   : 初稿
  作    者   : 张舵
  生成日期   : 2019年7月24日
  最近修改   :
  功能描述   : 红外触发驱动，以中断方式进行
  函数列表   :
  修改历史   :
  1.日    期   : 2019年7月24日
    作    者   : 张舵
    修改内容   : 创建文件

******************************************************************************/
#ifndef __bsp_INFRARED_IT_
#define __bsp_INFRARED_IT_

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "delay.h"

/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
#define RCC_ALL_SENSOR     (RCC_AHB1Periph_GPIOA|RCC_AHB1Periph_GPIOB|RCC_AHB1Periph_GPIOC)
            
#define GPIO_PORT_SENSOR14    GPIOA
#define GPIO_PORT_SENSOR56    GPIOC
#define GPIO_PORT_SENSOR78    GPIOB
        
        
            
#define GPIO_PIN_SENSOR1    GPIO_Pin_4
#define GPIO_PIN_SENSOR2    GPIO_Pin_5
#define GPIO_PIN_SENSOR3    GPIO_Pin_6
#define GPIO_PIN_SENSOR4    GPIO_Pin_7
        
#define GPIO_PIN_SENSOR5    GPIO_Pin_4
#define GPIO_PIN_SENSOR6    GPIO_Pin_5
        
#define GPIO_PIN_SENSOR7    GPIO_Pin_0
#define GPIO_PIN_SENSOR8    GPIO_Pin_1


#define GETS1 	GPIO_ReadInputDataBit(GPIO_PORT_SENSOR14,GPIO_PIN_SENSOR1) 
#define GETS2 	GPIO_ReadInputDataBit(GPIO_PORT_SENSOR14,GPIO_PIN_SENSOR2)
#define GETS3 	GPIO_ReadInputDataBit(GPIO_PORT_SENSOR14,GPIO_PIN_SENSOR3) 
#define GETS4 	GPIO_ReadInputDataBit(GPIO_PORT_SENSOR14,GPIO_PIN_SENSOR4)

#define GETS5 	GPIO_ReadInputDataBit(GPIO_PORT_SENSOR56,GPIO_PIN_SENSOR5) 
#define GETS6 	GPIO_ReadInputDataBit(GPIO_PORT_SENSOR56,GPIO_PIN_SENSOR6)

#define GETS7 	GPIO_ReadInputDataBit(GPIO_PORT_SENSOR78,GPIO_PIN_SENSOR7) 
#define GETS8 	GPIO_ReadInputDataBit(GPIO_PORT_SENSOR78,GPIO_PIN_SENSOR8)



/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/
extern __IO int16_t g_infraredIRQ;
/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/



void bsp_InitInfraredExit(void);

int16_t bsp_InfraredExitScan(void);





#endif

