/******************************************************************************

                  版权所有 (C), 2013-2023, 深圳博思高科技有限公司

 ******************************************************************************
  文 件 名   : mqtt_send.c
  版 本 号   : 初稿
  作    者   : 张舵
  生成日期   : 2019年12月17日
  最近修改   :
  功能描述   : 通过MQTT主动对服务器发送消息
  函数列表   :
  修改历史   :
  1.日    期   : 2019年12月17日
    作    者   : 张舵
    修改内容   : 创建文件

******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "mqtt_send.h"
#include "MQTTPacket.h"
#include "transport.h"
#include "FreeRTOS.h"
#include "task.h"
#include "malloc.h"
#include <string.h>
#include <stdio.h>
#include "cJSON.h"
#include "ini.h"
#include "easyflash.h"

#define LOG_TAG    "mqtt_send"
#include "elog.h"




/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/
static void upgradeDataPacket(const uint8_t *jsonbuff,uint8_t *outBuf);
static int print_preallocated(cJSON *root);


void mqttSend(void)
{
    MQTTString topicString = MQTTString_initializer;
     char *jsonPacket;
     uint32_t len = 0;
     int32_t rc = 0;
     unsigned char buf[1024];
     int buflen = sizeof(buf);
     unsigned char payload_out[512];
     int payload_out_len = 0;

     unsigned short msgid = 1;
     int req_qos = 0;
     unsigned char retained = 0;  

//    jsonPacket = mymalloc(SRAMIN,512);
// 
//  	if(jsonPacket== NULL)
//  	{
//    		printf("malloc error\r\n");
//  	}
    

    if(gConnectStatus == 1)
    {
        jsonPacket = ef_get_env("upData");        

        upgradeDataPacket(jsonPacket,payload_out);
        
        payload_out_len = strlen((char*)payload_out);
        topicString.cstring = DEVICE_PUBLISH;       //属性上报 发布

        len = MQTTSerialize_publish((unsigned char*)buf, buflen, 0, req_qos, retained, msgid, topicString, payload_out, payload_out_len);//发布消息
        rc = transport_sendPacketBuffer(mysock, (unsigned char*)buf, len);
        if(rc == len)                                                           //
            printf("send PUBLISH Successfully\r\n");
        else
            printf("send PUBLISH failed\r\n");  
        
            //升级完成并上报后，置位
            ef_set_env("up_status","101799");
       
    }   

}


static void upgradeDataPacket(const uint8_t *jsonbuff,uint8_t *outBuf)
{
    unsigned char payload_out[512] = {0};
    char *up_status;
    cJSON *root ,*dataObj;

    char *tmpBuf;

    if(jsonbuff == NULL)
    {
        log_d("error json data\r\n");
        return;
    }    
    
    root = cJSON_Parse((char *)jsonbuff);    //解析数据包
    if (!root)  
    {  
        log_d("Error before: [%s]\r\n",cJSON_GetErrorPtr());  
        return;
    } 

     cJSON_AddStringToObject(root,"commandCode","1017");

    //获取dataobject
    dataObj = cJSON_GetObjectItem( root , "data" ); 

    up_status = ef_get_env("up_status");

    //升级失败
    if(memcmp(up_status,"101700",6) == 0)
    {
        cJSON_AddStringToObject(dataObj,"status","2");
    }
    else if(memcmp(up_status,"101711",6) == 0) //升级成功
    {
        cJSON_AddStringToObject(dataObj,"status","1");
    }
    else if(memcmp(up_status,"101722",6) == 0) //升级失败
    {
        cJSON_AddStringToObject(dataObj,"status","2"); 
    }
    else if(memcmp(up_status,"101733",6) == 0) //禁止升级
    {
        cJSON_AddStringToObject(dataObj,"status","3");
    }
    else
    {
        //无升级动作
    }

    tmpBuf = cJSON_PrintUnformatted(root); 

    if(tmpBuf == NULL)
    {
        log_d("cJSON_PrintUnformatted error \r\n");
        return ;
    }    

    strcpy((char *)outBuf,tmpBuf);


    log_d("send json data = %s\r\n",tmpBuf);

    cJSON_Delete(root);

    my_free(tmpBuf);
}



/* Create a bunch of objects as demonstration. */
static int print_preallocated(cJSON *root)
{
    /* declarations */
    char *out = NULL;
    char *buf = NULL;
    char *buf_fail = NULL;
    size_t len = 0;
    size_t len_fail = 0;

    /* formatted print */
    out = cJSON_Print(root);

    printf("cJSON_Print = %s\r\n",out);

    /* create buffer to succeed */
    /* the extra 5 bytes are because of inaccuracies when reserving memory */
    len = strlen(out) + 5;
    buf = (char*)my_malloc(len);
    if (buf == NULL)
    {
        printf("Failed to allocate memory.\n");
        exit(1);
    }

    /* create buffer to fail */
    len_fail = strlen(out);
    buf_fail = (char*)my_malloc(len_fail);
    if (buf_fail == NULL)
    {
        printf("Failed to allocate memory.\n");
        exit(1);
    }

    /* Print to buffer */
    if (!cJSON_PrintPreallocated(root, buf, (int)len, 1)) {
        printf("cJSON_PrintPreallocated failed!\n");
        if (strcmp(out, buf) != 0) {
            printf("cJSON_PrintPreallocated not the same as cJSON_Print!\n");
            printf("cJSON_Print result:\n%s\n", out);
            printf("cJSON_PrintPreallocated result:\n%s\n", buf);
        }
        my_free(out);
        my_free(buf_fail);
        my_free(buf);
        return -1;
    }

    /* success */
    printf("%s\n", buf);

    /* force it to fail */
    if (cJSON_PrintPreallocated(root, buf_fail, (int)len_fail, 1)) {
        printf("cJSON_PrintPreallocated failed to show error with insufficient memory!\n");
        printf("cJSON_Print result:\n%s\n", out);
        printf("cJSON_PrintPreallocated result:\n%s\n", buf_fail);
        my_free(out);
        my_free(buf_fail);
        my_free(buf);
        return -1;
    }

    my_free(out);
    my_free(buf_fail);
    my_free(buf);
    return 0;
}



