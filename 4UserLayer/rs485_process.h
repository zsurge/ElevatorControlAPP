/******************************************************************************

                  版权所有 (C), 2013-2023, 深圳博思高科技有限公司

 ******************************************************************************
  文 件 名   : rs485_process.h
  版 本 号   : 初稿
  作    者   : 张舵
  生成日期   : 2019年12月23日
  最近修改   :
  功能描述   : 电梯控制器的指令处理文件
  函数列表   :
  修改历史   :
  1.日    期   : 2019年12月23日
    作    者   : 张舵
    修改内容   : 创建文件

******************************************************************************/
#ifndef __RS485_PROCESS_H_
#define __RS485_PROCESS_H_


/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "errorcode.h"
#include "bsp_uart_fifo.h"
#include "tool.h"
#include "bsp_dipSwitch.h"
#include "easyflash.h"
#include "comm.h"
#include "MQTTPacket.h"
#include "transport.h"
#include "jsonUtils.h"

/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
#define MAX_CMD_LEN 5
#define MAX_SEND_LEN 37
#define QUEUE_BUF_LEN   64
#define CMD_STX     0x5A

#define AUTH_MODE_CARD  2
#define AUTH_MODE_QR    7

#pragma pack(1)
typedef struct
{
    uint8_t data[QUEUE_BUF_LEN];         //需要发送给服务器的数据
    uint8_t authMode;                     //鉴权模式,刷卡=2；QR=7
    uint8_t dataLen;                     //数据长度    
}READER_BUFF_T;
#pragma pack()

extern READER_BUFF_T gReaderMsg;


/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/


/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/

void packetDefaultSendBuf(uint8_t *buf);
void packetSendBuf(READER_BUFF_T *pQueue,uint8_t *buf);

uint8_t authReader(READER_BUFF_T *pQueue,LOCAL_USER_T *localUserData);



#endif


