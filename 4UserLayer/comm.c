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
#include "easyflash.h"
#include "MQTTPacket.h"
#include "transport.h"
#include "jsonUtils.h"
#include "version.h"


#define LOG_TAG    "comm"
#include "elog.h"						



/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
#define DIM(x)  (sizeof(x)/sizeof(x[0]))


/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/
    

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/
QueueHandle_t xTransQueue = NULL; 
int gConnectStatus = 0;
int	gMySock = 0;
READER_BUFF_T gReaderMsg;

static SYSERRORCODE_E SendToQueue(uint8_t *buf,int len,uint8_t authMode);

static SYSERRORCODE_E OpenDoor ( uint8_t* msgBuf ); //开门
static SYSERRORCODE_E AbnormalAlarm ( uint8_t* msgBuf ); //远程报警
static SYSERRORCODE_E AddCardNo ( uint8_t* msgBuf ); //添加卡号
static SYSERRORCODE_E DelCardNo ( uint8_t* msgBuf ); //删除卡号
static SYSERRORCODE_E UpgradeDev ( uint8_t* msgBuf ); //对设备进行升级
static SYSERRORCODE_E UpgradeAck ( uint8_t* msgBuf ); //升级应答
static SYSERRORCODE_E EnableDev ( uint8_t* msgBuf ); //开启设备
static SYSERRORCODE_E DisableDev ( uint8_t* msgBuf ); //关闭设备
static SYSERRORCODE_E SetDevParam ( uint8_t* msgBuf ); //设置参数
static SYSERRORCODE_E SetJudgeMode ( uint8_t* msgBuf ); //设置识别模式
static SYSERRORCODE_E GetDevInfo ( uint8_t* msgBuf ); //获取设备信息
static SYSERRORCODE_E GetTemplateParam ( uint8_t* msgBuf ); //获取模板参数
static SYSERRORCODE_E GetServerIp ( uint8_t* msgBuf ); //获取模板参数
static SYSERRORCODE_E GetUserInfo ( uint8_t* msgBuf ); //获取用户信息
static SYSERRORCODE_E RemoteOptDev ( uint8_t* msgBuf ); //远程呼梯
static SYSERRORCODE_E ClearUserInof ( uint8_t* msgBuf ); //删除用户信息
static SYSERRORCODE_E AddSingleUser( uint8_t* msgBuf ); //添加单个用户
static SYSERRORCODE_E UnbindDev( uint8_t* msgBuf ); //解除绑定



static SYSERRORCODE_E ReturnDefault ( uint8_t* msgBuf ); //返回默认消息


typedef SYSERRORCODE_E ( *cmd_fun ) ( uint8_t *msgBuf ); 

typedef struct
{
	const char* cmd_id;            /* 命令id */
	cmd_fun  fun_ptr;     /* 函数指针 */
} CMD_HANDLE_T;

CMD_HANDLE_T CmdList[] =
{
	{"201",  OpenDoor},
	{"1006", AbnormalAlarm},
	{"1012", AddCardNo},
	{"1013", DelCardNo},
	{"1015", AddSingleUser},
	{"1016", UpgradeDev},
	{"1017", UpgradeAck},
	{"1021", EnableDev},
	{"1023", SetDevParam},
	{"1024", SetJudgeMode},
	{"1026", GetDevInfo},
    {"3002", GetServerIp},
    {"3003", GetTemplateParam},
    {"3004", GetUserInfo},   
    {"3005", RemoteOptDev},        
    {"3006", ClearUserInof},   
    {"3009", UnbindDev},          
};


SYSERRORCODE_E exec_proc ( char* cmd_id, uint8_t *msg_buf )
{
	SYSERRORCODE_E result = NO_ERR;
	int i = 0;

    if(cmd_id == NULL)
    {
        log_d("empty cmd \r\n");
        return CMD_EMPTY_ERR; 
    }

	for ( i = 0; i < DIM ( CmdList ); i++ )
	{
		if ( 0 == strcmp ( CmdList[i].cmd_id, cmd_id ) )
		{
			CmdList[i].fun_ptr ( msg_buf );
			return result;
		}
	}
	log_d ( "invalid id %s\n", cmd_id );

    
    ReturnDefault(msg_buf);
	return result;
}


//这个是为了方便服务端调试，给写的默认返回的函数
static SYSERRORCODE_E ReturnDefault ( uint8_t* msgBuf ) //返回默认消息
{
        SYSERRORCODE_E result = NO_ERR;
        uint8_t buf[MQTT_MAX_LEN] = {0};
        uint16_t len = 0;
    
        if(!msgBuf)
        {
            return STR_EMPTY_ERR;
        }
    
        result = modifyJsonItem(packetBaseJson(msgBuf),"status","1",1,buf);      
        result = modifyJsonItem(packetBaseJson(buf),"UnknownCommand","random return",1,buf);   
    
        if(result != NO_ERR)
        {
            return result;
        }
    
        len = strlen((const char*)buf);
    
        log_d("OpenDoor len = %d,buf = %s\r\n",len,buf);
    
        PublishData(buf,len);
        
        return result;

}


SYSERRORCODE_E OpenDoor ( uint8_t* msgBuf )
{
	SYSERRORCODE_E result = NO_ERR;
    uint8_t buf[MQTT_MAX_LEN] = {0};
    uint16_t len = 0;

    if(!msgBuf)
    {
        return STR_EMPTY_ERR;
    }

//    result = modifyJsonItem(msgBuf,"openStatus","1",1,buf);
    result = modifyJsonItem(packetBaseJson(msgBuf),"openStatus","1",1,buf);

    if(result != NO_ERR)
    {
        return result;
    }

    len = strlen((const char*)buf);

    log_d("OpenDoor len = %d,buf = %s\r\n",len,buf);

    PublishData(buf,len);
    
	return result;
}

SYSERRORCODE_E AbnormalAlarm ( uint8_t* msgBuf )
{
	SYSERRORCODE_E result = NO_ERR;
	return result;
}

SYSERRORCODE_E AddCardNo ( uint8_t* msgBuf )
{
	SYSERRORCODE_E result = NO_ERR;
	return result;
}

SYSERRORCODE_E DelCardNo ( uint8_t* msgBuf )
{
	SYSERRORCODE_E result = NO_ERR;
	return result;
}

SYSERRORCODE_E UpgradeDev ( uint8_t* msgBuf )
{
	SYSERRORCODE_E result = NO_ERR;
    uint8_t tmpUrl[256] = {0};
    
    if(!msgBuf)
    {
        return STR_EMPTY_ERR;
    }

    //1.保存URL
    strcpy((char *)tmpUrl,(const char*)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"softwareUrl",1));
    log_d("tmpUrl = %s\r\n",tmpUrl);
    
    ef_set_env("url", (const char*)GetJsonItem((const uint8_t *)tmpUrl,(const uint8_t *)"picUrl",0)); 

    //2.设置升级状态为待升级状态
    ef_set_env("up_status", "101700");
    
    //3.保存整个JSON数据
    ef_set_env("upData", (const char*)msgBuf);
    
    //4.设置标志位并重启
    SystemUpdate();
    
	return result;

}

SYSERRORCODE_E UpgradeAck ( uint8_t* msgBuf )
{
	SYSERRORCODE_E result = NO_ERR;
    uint8_t buf[MQTT_MAX_LEN] = {0};
    uint16_t len = 0;

    //读取升级数据并解析JSON包   

    result = upgradeDataPacket(buf);

    if(result != NO_ERR)
    {
        return result;
    }

    len = strlen((const char*)buf);

    log_d("OpenDoor len = %d,buf = %s\r\n",len,buf);

    PublishData(buf,len);
    
	return result;

}

SYSERRORCODE_E EnableDev ( uint8_t* msgBuf )
{
	SYSERRORCODE_E result = NO_ERR;
	return result;
}

SYSERRORCODE_E DisableDev ( uint8_t* msgBuf )
{
	SYSERRORCODE_E result = NO_ERR;
	return result;
}

SYSERRORCODE_E SetDevParam ( uint8_t* msgBuf )
{
	SYSERRORCODE_E result = NO_ERR;
	return result;
}

SYSERRORCODE_E SetJudgeMode ( uint8_t* msgBuf )
{
	SYSERRORCODE_E result = NO_ERR;
	return result;
}

SYSERRORCODE_E GetDevInfo ( uint8_t* msgBuf )
{
	SYSERRORCODE_E result = NO_ERR;
    uint8_t buf[MQTT_MAX_LEN] = {0};
    uint8_t *identification;
    uint16_t len = 0;

    if(!msgBuf)
    {
        return STR_EMPTY_ERR;
    }

    result = PacketDeviceInfo(msgBuf,buf);


    if(result != NO_ERR)
    {
        return result;
    }

    len = strlen((const char*)buf);

    log_d("GetDevInfo len = %d,buf = %s\r\n",len,buf);

    PublishData(buf,len);
    
    my_free(identification);
    
	return result;

}



int PublishData(uint8_t *payload_out,uint16_t payload_out_len)
{
    
    #define DEVICE_PUBLISH		"/smartCloud/server/msg/device"		
    
	MQTTString topicString = MQTTString_initializer;
    
	uint32_t len = 0;
	int32_t rc = 0;
	unsigned char buf[MQTT_MAX_LEN];
	int buflen = sizeof(buf);

	unsigned short msgid = 1;
	int req_qos = 0;
	unsigned char retained = 0;  

    if(!payload_out)
    {
        return STR_EMPTY_ERR;
    }

    log_d("payload_out = %s,payload_out_len = %d\r\n",payload_out,payload_out_len);

   if(gConnectStatus == 1)
   { 
       topicString.cstring = DEVICE_PUBLISH;       //属性上报 发布       

       len = MQTTSerialize_publish((unsigned char*)buf, buflen, 0, req_qos, retained, msgid, topicString, payload_out, payload_out_len);//发布消息
       rc = transport_sendPacketBuffer(gMySock, (unsigned char*)buf, len);
       if(rc == len)                                                           //
           log_d("send PUBLISH Successfully\r\n");
       else
           log_d("send PUBLISH failed\r\n");     
      
   }
   else
   {
        log_d("MQTT Lost the connect!!!\r\n");
   }

   return len;
}


SYSERRORCODE_E GetTemplateParam ( uint8_t* msgBuf )
{
	SYSERRORCODE_E result = NO_ERR;
    uint8_t buf[MQTT_MAX_LEN] = {0};
    uint16_t len = 0;

    if(!msgBuf)
    {
        return STR_EMPTY_ERR;
    }
    
    result = modifyJsonItem(packetBaseJson(msgBuf),"status","1",1,buf);

    if(result != NO_ERR)
    {
        return result;
    }

    len = strlen((const char*)buf);

    log_d("GetParam len = %d,buf = %s\r\n",len,buf);

    PublishData(buf,len);
    
	return result;
}

//获服务器IP
static SYSERRORCODE_E GetServerIp ( uint8_t* msgBuf )
{
	SYSERRORCODE_E result = NO_ERR;
    uint8_t buf[MQTT_MAX_LEN] = {0};
    uint8_t ip[32] = {0};
    uint16_t len = 0;

    if(!msgBuf)
    {
        return STR_EMPTY_ERR;
    }

    //1.保存IP     
    strcpy((char *)ip,(const char *)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"ip",1));
    log_d("server ip = %s\r\n",ip[0]);

    //影响服务器
    result = modifyJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"status",(const uint8_t *)"1",1,buf);

    if(result != NO_ERR)
    {
        return result;
    }

    len = strlen((const char*)buf);

    PublishData(buf,len);
    
	return result;

}

//获取用户信息
static SYSERRORCODE_E GetUserInfo ( uint8_t* msgBuf )
{
	SYSERRORCODE_E result = NO_ERR;
    uint8_t buf[MQTT_MAX_LEN] = {0};
    uint8_t userID[16] = {0};
    uint8_t cardID[16] = {0};
    uint8_t value[255] = {0};
    uint8_t cardIDvalue[255] = {0};
    uint8_t tmp[128] = {0};
    uint16_t len = 0;

    if(!msgBuf)
    {
        return STR_EMPTY_ERR;
    }

    //1.保存以userID为key的表
    memset(userID,0x00,sizeof(userID));
    strcpy((char *)userID,(const char*)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"userId",1));
    log_d("userId = %s\r\n",userID);
    
    //1.1 保存以card id 为key的表
    memset(cardIDvalue,0x00,sizeof(cardIDvalue));    
    strcpy((char *)cardIDvalue,(const char*)userID);
    

    //2.保存卡号
    memset(value,0x00,sizeof(value));   
    strcpy((char *)cardID,(const char *)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"cardNo",1));
    strcpy((char *)value,(const char*)cardID);
    log_d("cardNo = %s\r\n",value);
    
    

    //3.保存楼层权限
    memset(tmp,0x00,sizeof(tmp));
    strcpy((char *)tmp,  (const char*)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"accessLayer",1));
    
    strcat((char *)value,(const char *)";");
    strcat((char *)value,(const char*)tmp);

    strcat((char *)cardIDvalue,(const char*)";");
    strcat((char *)cardIDvalue,(const char*)tmp);    
    log_d("accessLayer = %s\r\n",value);

    //4.保存默认楼层
    memset(tmp,0x00,sizeof(tmp));
    strcpy((char *)tmp,  (const char*)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"defaultLayer",1));
    
    strcat((char *)value,(const char*)";");
    strcat((char *)value,(const char*)tmp);

    strcat((char *)cardIDvalue,(const char*)";");
    strcat((char *)cardIDvalue,(const char*)tmp);      
    log_d("defaultLayer = %s\r\n",value);

    //5.保存开始时间
    memset(tmp,0x00,sizeof(tmp));
    strcpy((char *)tmp,  (const char*)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"startTime",1));
    
    strcat((char *)value,(const char*)";");
    strcat((char *)value,(const char*)tmp);

    strcat((char *)cardIDvalue,(const char*)";");
    strcat((char *)cardIDvalue,(const char*)tmp); 
    log_d("startTime = %s\r\n",value);

    //6.保存结束时间
    memset((char *)tmp,0x00,sizeof(tmp));
    strcpy((char *)tmp,  (const char*)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"endTime",1));
    
    strcat((char *)value,(const char*)";");
    strcat((char *)value,(const char*)tmp);

    strcat((char *)cardIDvalue,(const char*)";");
    strcat((char *)cardIDvalue,(const char*)tmp); 
    log_d("endTime = %s\r\n",value);    

    //写记录
    if((ef_set_env((const char*)userID, (const char*)value) == EF_NO_ERR) && (ef_set_env((const char*)cardID, (const char*)cardIDvalue) == EF_NO_ERR))
    {        
        //影响服务器
        result = modifyJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"status","1",1,buf);
        result = modifyJsonItem((const uint8_t *)buf,(const uint8_t *)"reason","success",1,buf);
    }
    else
    {
        //影响服务器
        result = modifyJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"status","0",1,buf);
        result = modifyJsonItem((const uint8_t *)buf,(const uint8_t *)"reason","add record error",1,buf);
    }
    

    if(result != NO_ERR)
    {
        return result;
    }

    len = strlen((const char*)buf);

    PublishData(buf,len);
    
	return result;

}

//远程呼梯
static SYSERRORCODE_E RemoteOptDev ( uint8_t* msgBuf )
{
    SYSERRORCODE_E result = NO_ERR;
    uint8_t buf[MQTT_MAX_LEN] = {0};
    uint8_t accessLayer[4] = {0};
    uint16_t len = 0;
    
    if(!msgBuf)
    {
        return STR_EMPTY_ERR;
    }

    
    strcpy((char *)accessLayer,(const char*)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"accessLayer",1));

    result = modifyJsonItem(msgBuf,"status","1",1,buf);

    if(result != NO_ERR)
    {
        return result;
    }
    

    //这里需要发消息到消息队列，进行呼梯
    SendToQueue(accessLayer,strlen((const char*)accessLayer),AUTH_MODE_REMOTE);

    len = strlen((const char*)buf);

    log_d("RemoteOptDev len = %d,buf = %s\r\n",len,buf);

    PublishData(buf,len); 
    
    return result;

}


//删除用户信息
static SYSERRORCODE_E ClearUserInof ( uint8_t* msgBuf )
{
    SYSERRORCODE_E result = NO_ERR;
    uint8_t buf[MQTT_MAX_LEN] = {0};
    uint16_t len = 0;

    if(!msgBuf)
    {
        return STR_EMPTY_ERR;
    }

    result = modifyJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"status","1",1,buf);

    if(result != NO_ERR)
    {
        return result;
    }

    len = strlen((const char*)buf);

    log_d("ClearUserInof len = %d,buf = %s\r\n",len,buf);

    PublishData(buf,len);


    //清空用户信息
    // ef_env_set_default();

    
    return result;

}

//添加单个用户
static SYSERRORCODE_E AddSingleUser( uint8_t* msgBuf )
{
    SYSERRORCODE_E result = NO_ERR;
    uint8_t buf[MQTT_MAX_LEN] = {0};
    uint8_t userID[16] = {0};
    uint8_t cardID[16] = {0};
    uint8_t value[255] = {0};
    uint8_t cardIDvalue[255] = {0};
    uint16_t len = 0;
    

    if(!msgBuf)
    {
        return STR_EMPTY_ERR;
    }

    //1.保存以userID为key的表
    memset(userID,0x00,sizeof(userID));
    strcpy((char *)userID,(const char*)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"userId",1));
    log_d("userId = %s\r\n",userID);

    //2.保存卡号
    memset(value,0x00,sizeof(value));   
    strcpy((char *)cardID,(const char *)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"cardNo",1));
    if(strlen((char *)cardID) == 0)
    {
        strcat((char *)value,"00000000");
    }
    else
    {
        strcat((char *)value,(const char*)cardID);
    }
    log_d("cardNo = %s\r\n",cardID); 
    
    //3.保存楼层权限
    strcat((char *)value,(const char *)";");
    strcat((char *)value,  (const char*)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"accessLayer",1));    
    log_d("accessLayer = %s\r\n",value);

    //4.保存默认楼层
    strcat((char *)value,(const char *)";");
    strcat((char *)value,  (const char*)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"defaultLayer",1));    
    log_d("accessLayer = %s\r\n",value);

    //5.保存开始时间
    strcat((char *)value,(const char *)";");
    strcat((char *)value,  (const char*)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"startTime",1));    
    log_d("accessLayer = %s\r\n",value);    

    //6.保存结束时间
    strcat((char *)value,(const char *)";");
    strcat((char *)value,  (const char*)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"endTime",1));    
    log_d("accessLayer = %s\r\n",value);    

    //写记录
    if(ef_set_env((const char*)userID, (const char*)value) == EF_NO_ERR)
    {        
        //影响服务器
        result = modifyJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"status","1",1,buf);
        result = modifyJsonItem((const uint8_t *)buf,(const uint8_t *)"commandCode","3004",1,buf);
    }
    else
    {
        //影响服务器
        result = modifyJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"status","0",1,buf);
        result = modifyJsonItem((const uint8_t *)buf,(const uint8_t *)"commandCode","3004",1,buf);        
        result = modifyJsonItem((const uint8_t *)buf,(const uint8_t *)"reason","add record error",1,buf);
    }    

    if(result != NO_ERR)
    {
        return result;
    }
    
    len = strlen((const char*)buf);

    log_d("AddSingleUser len = %d,buf = %s\r\n",len,buf);

    PublishData(buf,len);
    
    return result;

}

//解除绑定
static SYSERRORCODE_E UnbindDev( uint8_t* msgBuf )
{
    SYSERRORCODE_E result = NO_ERR;
    uint8_t buf[MQTT_MAX_LEN] = {0};
    uint8_t type[4] = {0};
    uint16_t len = 0;

    if(!msgBuf)
    {
        return STR_EMPTY_ERR;
    }

    strcpy((char *)type,(const char*)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"type",1));

    result = modifyJsonItem(msgBuf,"status","1",1,buf);

    if(result != NO_ERR)
    {
        return result;
    }

    //这里需要发消息到消息队列，解除绑定

    if(memcmp(type,"0",1) == 0)
    {
        SendToQueue(type,strlen((const char*)type),AUTH_MODE_UNBIND);
    }
    else if(memcmp(type,"1",1) == 0)
    {
        SendToQueue(type,strlen((const char*)type),AUTH_MODE_BIND);
    } 
    
    len = strlen((const char*)buf);

    log_d("UnbindDev len = %d,buf = %s\r\n",len,buf);

    PublishData(buf,len);

    return result;

}

static SYSERRORCODE_E SendToQueue(uint8_t *buf,int len,uint8_t authMode)
{
    SYSERRORCODE_E result = NO_ERR;

    READER_BUFF_T *ptQR; 
    /* 初始化结构体指针 */
    ptQR = &gReaderMsg;

	/* 清零 */
    ptQR->authMode = authMode; 
    ptQR->dataLen = 0;
    memset(ptQR->data,0x00,sizeof(ptQR->data)); 

    ptQR->dataLen = len;                
    memcpy(ptQR->data,buf,len);
    
    /* 使用消息队列实现指针变量的传递 */
    if(xQueueSend(xTransQueue,              /* 消息队列句柄 */
                 (void *) &ptQR,   /* 发送指针变量recv_buf的地址 */
                 (TickType_t)50) != pdPASS )
    {
        DBG("the queue is full!\r\n");                
        xQueueReset(xTransQueue);
    } 
    else
    {
        dbh("QR",(char *)buf,len);
        log_d("buf = %s,len = %d\r\n",buf,len);
    } 


    return result;
}

