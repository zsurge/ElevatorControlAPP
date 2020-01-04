/******************************************************************************

                  版权所有 (C), 2013-2023, 深圳博思高科技有限公司

 ******************************************************************************
  文 件 名   : rs485_process.c
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

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "rs485_process.h"
#include "jsonUtils.h"

#define LOG_TAG    "RS485"
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
static SYSERRORCODE_E packetToElevator(LOCAL_USER_T *localUserData,uint8_t *buff);
static void calcFloor(uint8_t layer,uint8_t *src,uint8_t *outFloor);



void packetDefaultSendBuf(uint8_t *buf)
{
    uint8_t sendBuf[64] = {0};

    sendBuf[0] = CMD_STX;
    sendBuf[1] = bsp_dipswitch_read();
    sendBuf[MAX_SEND_LEN-1] = xorCRC(sendBuf,MAX_SEND_LEN-2);

    memcpy(buf,sendBuf,MAX_SEND_LEN);
}


void packetSendBuf(READER_BUFF_T *pQueue,uint8_t *buf)
{
    uint8_t jsonBuf[JSON_ITEM_MAX_LEN] = {0};
    uint8_t sendBuf[64] = {0};
    uint16_t len = 0;
    uint16_t ret = 0;
    LOCAL_USER_T *localUserData= &gLoalUserData;
    
    sendBuf[0] = CMD_STX;
    sendBuf[1] = bsp_dipswitch_read();
    sendBuf[MAX_SEND_LEN-1] = xorCRC(sendBuf,MAX_SEND_LEN-2);

    memset(localUserData->cardNo,0x00,sizeof(localUserData->cardNo));
    memset(localUserData->userId,0x00,sizeof(localUserData->userId));
    memset(localUserData->accessLayer,0x00,sizeof(localUserData->accessLayer));
    memset(localUserData->startTime,0x00,sizeof(localUserData->startTime));
    memset(localUserData->endTime,0x00,sizeof(localUserData->endTime));
    localUserData->defaultLayer = 0;
    localUserData->authMode = 0;
    
    memcpy(buf,sendBuf,MAX_SEND_LEN);


    switch(pQueue->authMode)
    {
        case AUTH_MODE_CARD:
        case AUTH_MODE_QR:
            log_d("card or QR auth,pQueue->authMode = %d\r\n",pQueue->authMode);
            ret = authReader(pQueue,localUserData);    

            if(ret != NO_ERR)
            {
                log_d("reject access\r\n");
                return ;  //无权限
            }

            //1.发给电梯的数据
            packetToElevator(localUserData,buf);

            //2.发给服务器
            packetPayload(localUserData,jsonBuf); 

            len = strlen(jsonBuf);
            log_d("jsonBuf = %s,len = %d\r\n",jsonBuf,len);

            len = PublishData(jsonBuf,len);
            log_d("send = %d\r\n",len);            
            break;
        case AUTH_MODE_REMOTE:
            //直接发送目标楼层
            log_d("send desc floor\r\n");
            break;
        case AUTH_MODE_UNBIND:
            //直接发送停用设备指令
            log_d("send AUTH_MODE_UNBIND floor\r\n");
            break;
        case AUTH_MODE_BIND:
            //直接发送启动设置指令
            log_d("send AUTH_MODE_BIND floor\r\n");
            break;

        default:
            break;    
   }

}

SYSERRORCODE_E authReader(READER_BUFF_T *pQueue,LOCAL_USER_T *localUserData)
{
    SYSERRORCODE_E result = NO_ERR;
    char value[128] = {0};
    int val_len = 0;
    char *buf[6] = {0}; //存放分割后的子字符串 
    int num = 0;
    uint8_t key[8+1] = {0};    

    memset(key,0x00,sizeof(key));

    if(pQueue->authMode == AUTH_MODE_QR) 
    {
        //二维码
        strcpy((char *)key,(const char *)GetJsonItem((const uint8_t *)pQueue->data,(const uint8_t *)"ownerId",0));
    }
    else
    {
        //读卡，-2 是减掉0D 0A
        memcpy(key,pQueue->data+pQueue->dataLen-2-CARD_NO_LEN,CARD_NO_LEN);
    }
   
    log_d("key = %s\r\n",key);
    
    memset(value,0x00,sizeof(value));

    val_len = ef_get_env_blob(key, value, sizeof(value) , NULL);
   

    log_d("get env = %s,val_len = %d\r\n",value,val_len);

    if(val_len <= 0)
    {
        //未找到记录，无权限
        log_d("not find record\r\n");
        return NO_AUTHARITY_ERR;
    }

    split(value,";",buf,&num); //调用函数进行分割 
    log_d("num = %d\r\n",num);

    if(num != 5)
    {
        log_d("read record error\r\n");
        return READ_RECORD_ERR;       
    }

    localUserData->authMode = pQueue->authMode;    
    
    if(AUTH_MODE_QR == pQueue->authMode)
    {
        strcpy(localUserData->userId,key);
        
        strcpy(localUserData->cardNo,buf[0]);        
    }
    else
    {
        memcpy(localUserData->cardNo,key,CARD_NO_LEN);

        log_d("buf[0] = %s\r\n",buf[0]);
        strcpy(localUserData->userId,buf[0]);        
    }   

    //3867;0;0;2019-12-29;2029-12-31
    
    
    strcpy(localUserData->accessLayer,buf[1]);
    localUserData->defaultLayer = atoi(buf[2]);
    strcpy(localUserData->startTime,buf[3]);
    strcpy(localUserData->endTime,buf[4]);    



    log_d("localUserData->cardNo = %s\r\n",localUserData->cardNo);
    log_d("localUserData->userId = %s\r\n",localUserData->userId);
    log_d("localUserData->accessLayer = %s\r\n",localUserData->accessLayer);
    log_d("localUserData->defaultLayer = %d\r\n",localUserData->defaultLayer);    
    log_d("localUserData->startTime = %s\r\n",localUserData->startTime);        
    log_d("localUserData->endTime = %s\r\n",localUserData->endTime);        
    log_d("localUserData->authMode = %d\r\n",localUserData->authMode);

    return result;
}

SYSERRORCODE_E authRemote(READER_BUFF_T *pQueue,LOCAL_USER_T *localUserData)
{
    SYSERRORCODE_E result = NO_ERR;
    char value[128] = {0};
    int val_len = 0;
    char *buf[6] = {0}; //存放分割后的子字符串 
    int num = 0;
    uint8_t key[8+1] = {0};    

    memset(key,0x00,sizeof(key));   
    
    memset(value,0x00,sizeof(value));

    val_len = ef_get_env_blob(key, value, sizeof(value) , NULL);
   

    log_d("get env = %s,val_len = %d\r\n",value,val_len);

    if(val_len <= 0)
    {
        //未找到记录，无权限
        log_d("not find record\r\n");
        return NO_AUTHARITY_ERR;
    }

    split(value,";",buf,&num); //调用函数进行分割 
    log_d("num = %d\r\n",num);

    if(num != 5)
    {
        log_d("read record error\r\n");
        return READ_RECORD_ERR;       
    }

    localUserData->authMode = pQueue->authMode;    
    
    if(AUTH_MODE_QR == pQueue->authMode)
    {
        strcpy(localUserData->userId,key);
        
        strcpy(localUserData->cardNo,buf[0]);        
    }
    else
    {
        memcpy(localUserData->cardNo,key,CARD_NO_LEN);

        log_d("buf[0] = %s\r\n",buf[0]);
        strcpy(localUserData->userId,buf[0]);        
    }   

    //3867;0;0;2019-12-29;2029-12-31
    
    
    strcpy(localUserData->accessLayer,buf[1]);
    localUserData->defaultLayer = atoi(buf[2]);
    strcpy(localUserData->startTime,buf[3]);
    strcpy(localUserData->endTime,buf[4]);    



    log_d("localUserData->cardNo = %s\r\n",localUserData->cardNo);
    log_d("localUserData->userId = %s\r\n",localUserData->userId);
    log_d("localUserData->accessLayer = %s\r\n",localUserData->accessLayer);
    log_d("localUserData->defaultLayer = %d\r\n",localUserData->defaultLayer);    
    log_d("localUserData->startTime = %s\r\n",localUserData->startTime);        
    log_d("localUserData->endTime = %s\r\n",localUserData->endTime);        
    log_d("localUserData->authMode = %d\r\n",localUserData->authMode);

    return NO_ERR;

}

static SYSERRORCODE_E packetToElevator(LOCAL_USER_T *localUserData,uint8_t *buff)
{
    SYSERRORCODE_E result = NO_ERR;
    uint8_t oneLayer[8] = {0};
    uint8_t tmpBuf[MAX_SEND_LEN+1] = {0};
    char *authLayer[64] = {0}; //权限楼层，最多64层
    int num = 0;    
    uint8_t sendBuf[MAX_SEND_LEN+1] = {0};

    uint8_t floor = 0;

    uint8_t div = 0;
    uint8_t remainder = 0;
    uint8_t i = 0;

    
    log_d("localUserData->cardNo = %s\r\n",localUserData->cardNo);
    log_d("localUserData->userId = %s\r\n",localUserData->userId);
    log_d("localUserData->accessLayer = %s\r\n",localUserData->accessLayer);
    log_d("localUserData->defaultLayer = %d\r\n",localUserData->defaultLayer);    
    log_d("localUserData->startTime = %s\r\n",localUserData->startTime);        
    log_d("localUserData->endTime = %s\r\n",localUserData->endTime);        
    log_d("localUserData->authMode = %d\r\n",localUserData->authMode);

    

    split(localUserData->accessLayer,",",authLayer,&num); //调用函数进行分割 

    log_d("num = %d\r\n",num);

    memset(sendBuf,0x00,sizeof(sendBuf));
    
    if(num > 1)//多层权限
    {
        for(i=0;i<num;i++)
        {
            calcFloor(atoi(authLayer[i]),sendBuf,tmpBuf);
            memcpy(sendBuf,tmpBuf,MAX_SEND_LEN);
        }        
    }
    else    //单层权限，直接呼默认权限楼层
    {
        floor = atoi(authLayer[0]);

//        div = floor / 8;
//        remainder = floor / 8;

//        if(div != 0 && remainder != 0)
//        {            
//           SETBIT(tmpFloor,remainder-1);
//           sendBuf[div+8] = tmpFloor;
//        }
//        else if(div == 0 && remainder != 0)
//        {
//            SETBIT(tmpFloor,remainder-1);
//            sendBuf[div+8] = tmpFloor;
//        }
//        else if(div != 0 && remainder == 0)
//        {
//            SETBIT(tmpFloor,8-1);
//            sendBuf[div-1+8 ] = tmpFloor;
//        }        

        calcFloor(floor,sendBuf,tmpBuf);
        dbh("tmpBuf", tmpBuf, MAX_SEND_LEN);        
    }

    memset(sendBuf,0x00,sizeof(sendBuf));

    sendBuf[0] = CMD_STX;
    sendBuf[1] = bsp_dipswitch_read();
    memcpy(sendBuf+2,tmpBuf,MAX_SEND_LEN-5);        
    sendBuf[MAX_SEND_LEN-1] = xorCRC(sendBuf,MAX_SEND_LEN-2);        
    memcpy(buff,sendBuf,MAX_SEND_LEN);  

    dbh("sendBuf", sendBuf, MAX_SEND_LEN);
    
    return result;
}



static void calcFloor(uint8_t layer,uint8_t *src,uint8_t *outFloor)
{
    uint8_t div = 0;
    uint8_t remainder = 0;
    uint8_t floor = layer;
    uint8_t sendBuf[MAX_SEND_LEN+1] = {0};
    static uint8_t tmpFloor = 0;

    memcpy(sendBuf,src,MAX_SEND_LEN);
    
    div = floor / 8;
    remainder = floor / 8;

    log_d("div = %d,remain = %d\r\n",div,remainder);
    
//    if(div != 0 && remainder != 0)
//    {            
//       SETBIT(tmpFloor,remainder-1);
//       sendBuf[div+8] = tmpFloor;
//    }
//    else if(div == 0 && remainder != 0)
//    {
//        SETBIT(tmpFloor,remainder-1);
//        sendBuf[div+8] = tmpFloor;
//    }
//    else if(div != 0 && remainder == 0)
//    {
//        SETBIT(tmpFloor,8-1);
//        sendBuf[div-1+8 ] = tmpFloor;
//    }   

    if(div != 0 && remainder == 0)// 8,16,24
    {
        SETBIT(tmpFloor,8-1);
        sendBuf[div-1+8 ] = tmpFloor;
    } 
    else //1~7 ，非8层的倍数
    {
        SETBIT(tmpFloor,remainder-1);
        sendBuf[div+8] = tmpFloor;
    }


    memcpy(outFloor,sendBuf,MAX_SEND_LEN);

    tmpFloor = 0;
}





