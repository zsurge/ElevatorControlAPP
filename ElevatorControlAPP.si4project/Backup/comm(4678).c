/******************************************************************************

                  版权所有 (C), 2013-2023, 深圳博思高科技有限公司

 ******************************************************************************
  文 件 名   : comm.c
  版 本 号   : 初稿
  作    者   : 张舵
  生成日期   : 2019年6月18日
  最近修改   :
  功能描述   : 解析串口指令
  函数列表   :
  修改历史   :
  1.日    期   : 2019年6月18日
    作    者   : 张舵
    修改内容   : 创建文件

******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "comm.h"
#include "tool.h"
#include "bsp_led.h"
#include "malloc.h"
#include "ini.h"
#include "bsp_uart_fifo.h"
#include "version.h"



/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/

typedef void(*COMMAND_FUNCTION)(char *msg_buf); //获取SN的回调

 typedef struct 
 {
 ? ? const char *cmd_id; ?           /* 命令id */
 ? ? COMMAND_FUNCTION  fun_ptr; ?    /* 函数指针 */
 }CMDPROC_T;
 
 CMDPROC_T CmdList[] =
 {
 ?? ?{"201",  OpenDoor},? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ??
 ?? ?{"1006", AbnormalAlarm},? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ?
 ?? ?{"1012", AddCardNo},? ? ? ? ? ? ? ? ? ? ? ? ? ??
 ?? ?{"1013", DelCardNo},? ? ? ? ? ? ? ? ? ? ? ? ? ? ??
 ?? ?{"1016", UpgradeDev},? ? ?
  ?? {"1017", UpgradeAck},? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ??
 ?? ?{"1021", EnableDev},? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ?
 ?? ?{"1022", DisableDev},? ? ? ? ? ? ? ? ? ? ? ? ? ??
 ?? ?{"1023", SetDevParam},? ? ? ? ? ? ? ? ? ? ? ? ? ? ??
 ?? ?{"1024", SetJudgeMode},? ?? 
  ?? {"1026", GetDevInfo}?
 };
 

 void exec_proc(char *cmd_id, char *msg_buf)
 {
 ?? ?int type_num = sizeof(CmdList) / sizeof(CMDPROC_T);
 ? ? int i = 0;
 ?? ?
 ?? ?printf("cmd_id %s\n",cmd_id);
 ? ? for (i = 0; i < type_num; i++)
 ? ? {
 ? ? ? ? if (0 == strcmp(CmdList[i].cmd_id, cmd_id))
 ? ? ? ? {
 ? ? ? ? ? ? CmdList[i].fun_ptr(msg_buf);
 ? ? ? ? ? ? return ;
 ? ? ? ? }
 ? ? }
 ? ? printf("invalid id %s\n", cmd_id);
 }
  
 void OpenDoor(char *buf)
 {
    
 }

 
 void AbnormalAlarm(char *buf)
 {
    
 }


  void AddCardNo(char *buf)
 {
    
 }

 
 void DelCardNo(char *buf)
 {
    
 }


  void UpgradeDev(char *buf)
 {
    
 }

 
 void UpgradeAck(char *buf)
 {
    
 }


  void EnableDev(char *buf)
 {
    
 }

 
 void DisableDev(char *buf)
 {
    
 }
 

  void SetDevParam(char *buf)
 {
    
 }

 
 void SetJudgeMode(char *buf)
 {
    
 }


  void GetDevInfo(char *buf)
 {
    
 }

  
