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

/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
#define MAX_CMD_LEN 5
#define MAX_SEND_LEN 37
#define QUEUE_BUF_LEN   64
#define CMD_STX     0x5A


#define AUTH_QR 0
#define AUTH_READER 1
#define AUTH_REMOTE 2

typedef struct
{
    uint8_t userID[6];      //用户ID，根据用户ID判定是否有权限
    uint8_t cardID[4];      //卡号
    uint8_t authFloor[4];   //所拥有的楼层
}USER_T;

typedef struct
{
    uint8_t currentFloor;   //当前楼层
    uint8_t targetFloor;    //目标楼层
    USER_T *user;
}USER_INFO_T;




typedef struct
{
    uint8_t authorizationMode;           //刷卡，QR，远程
    uint8_t dataLen;                     //数据长度
    uint8_t data[QUEUE_BUF_LEN];         //需要发送给android板的数据
}QUEUE_BUFF_T;

extern QUEUE_BUFF_T gQueueMsg;
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
void packetSendBuf(QUEUE_BUFF_T *pQueue,uint8_t *buf);








#endif


