/******************************************************************************

                  ��Ȩ���� (C), 2013-2023, ���ڲ�˼�߿Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : bsp.h
  �� �� ��   : ����
  ��    ��   : �Ŷ�
  ��������   : 2019��7��9��
  ����޸�   :
  ��������   : ���ǵײ�����ģ�����е�h�ļ��Ļ����ļ���
            Ӧ�ó���ֻ�� #include bsp.h ���ɣ�����Ҫ#include ÿ��ģ��� h �ļ�
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2019��7��9��
    ��    ��   : �Ŷ�
    �޸�����   : �����ļ�

******************************************************************************/
#ifndef __BSP_H
#define __BSP_H
/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
//��׼��ͷ�ļ�
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

//�м��
#include "stmflash.h"
#include "easyflash.h"
//#include "sfud.h"

#include "lwip_comm.h"
#include "lwip/netif.h"
#include "lwipopts.h"




//������ͷ�ļ�
#include "bsp_flash.h"
#include "bsp_uart_fifo.h"

#include "LAN8720.h"

//#include "bsp_usart.h"
//#include "bsp_usart1.h"
//#include "bsp_usart2.h"

//#include "bsp_usart3.h"
//#include "bsp_usart4.h"
//#include "bsp_usart5.h"
#include "bsp_key.h"
#include "bsp_time.h"
#include "bsp_led.h"
#include "bsp_beep.h" 
#include "bsp_spi_flash.h"

//#include "bsp_digitaltube.h"
#include "bsp_iwdg.h"
//#include "bsp_infrared_it.h"
#include "bsp_wiegand.h"
#include "bsp_tim_pwm.h"

#ifndef RS485TEST	
#include "bsp_rs485.h"
#endif




/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ��������                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/

/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/
void bsp_Init(void);



#endif



