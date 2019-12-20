/**********************************************************************************************************
** 文件名		:mqtt_app.c
** 作者			:maxlicheng<licheng.chn@outlook.com>
** 作者github	:https://github.com/maxlicheng
** 作者博客		:https://www.maxlicheng.com/	
** 生成日期		:2018-08-08
** 描述			:mqtt服务程序
************************************************************************************************************/
#include "mqtt_app.h"
#include "MQTTPacket.h"
#include "transport.h"
#include "FreeRTOS.h"
#include "task.h"
#include "malloc.h"
#include <string.h>
#include <stdio.h>
#include "cJSON.h"
#include "ini.h"

#define LOG_TAG    "MQTTAPP"
#include "elog.h"


#define send_duration	20	//温度发送周期（ms）

float temp = 0;
float humid = 0;

static uint32_t send_cnt = 0;
static uint32_t recv_cnt = 0;



//static int MqttDataParseAndSend(uint8_t *recvData,int (*func)(uint8_t *));
static void MqttDataParseAndSend(uint8_t *recvData);
static int MqttGetUpgradeUrl(uint8_t *jsonData);
static int MqttSendDataToHost(uint8_t *jsonData);

    

int gConnectStatus = 0;
int	mysock = 0;



void mqtt_thread(void)
{
	MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
	MQTTString receivedTopic;
	MQTTString topicString = MQTTString_initializer;
	
	int32_t rc = 0;
	unsigned char buf[1024];
	int buflen = sizeof(buf);
	
	
	int payloadlen_in;
	unsigned char* payload_in;
	unsigned short msgid = 1;
	int subcount;
	int granted_qos =0;
	unsigned char sessionPresent, connack_rc;
	unsigned short submsgid;
	uint32_t len = 0;
	int req_qos = 0;
	unsigned char dup;
	int qos;
	unsigned char retained = 0;

	uint8_t connect_flag = 0;		//连接标志
	uint8_t msgtypes = CONNECT;		//消息状态初始化
	uint8_t t=0;
	
	log_d("socket connect to server\r\n");
	mysock = transport_open((char *)HOST_NAME,HOST_PORT);
	log_d("Sending to hostname %s port %d\r\n", HOST_NAME, HOST_PORT);
	
	len = MQTTSerialize_disconnect((unsigned char*)buf,buflen);
	rc = transport_sendPacketBuffer(mysock, (uint8_t *)buf, len);
	if(rc == len)															//
		log_d("send DISCONNECT Successfully\r\n");
	else
		log_d("send DISCONNECT failed\r\n"); 
    
	vTaskDelay(2500);
	
	log_d("socket connect to server\r\n");
	mysock = transport_open((char *)HOST_NAME,HOST_PORT);
	log_d("Sending to hostname %s port %d\r\n", HOST_NAME, HOST_PORT);


	data.clientID.cstring = CLIENT_ID;                   //随机
	data.keepAliveInterval = KEEPLIVE_TIME;         //保持活跃
	data.username.cstring = USER_NAME;              //用户名
	data.password.cstring = PASSWORD;               //秘钥
	data.MQTTVersion = MQTT_VERSION;                //3表示3.1版本，4表示3.11版本
	data.cleansession = 1;
    
	unsigned char payload_out[200];
	int payload_out_len = 0;

    //获取当前滴答，作为心跳包起始时间
	uint32_t curtick  =	 xTaskGetTickCount();
	uint32_t sendtick =  xTaskGetTickCount();
    
	while(1)
	{
		if(( xTaskGetTickCount() - curtick) >(data.keepAliveInterval*200))		//每秒200次tick
		{
			if(msgtypes == 0)
			{                
				curtick =  xTaskGetTickCount();
				msgtypes = PINGREQ;
                log_d("send heartbeat!!  set msgtypes = %d \r\n",msgtypes);
			}

		}
//		if(connect_flag == 1)
//		{
//			if((xTaskGetTickCount() - sendtick) >= (send_duration*200))
//			{
//                log_d("send PUBLISH!!  get msgtypes = %d \r\n",msgtypes);
//                
//				sendtick = xTaskGetTickCount();
//                
//				taskENTER_CRITICAL();	//进入临界区(无法被中断打断)
//				temp =10.0;
//				taskEXIT_CRITICAL();		//退出临界区(可以被中断打断)
//				
//				humid = 54.8+rand()%10+1;
				//sprintf((char*)payload_out,"{\"params\":{\"CurrentTemperature\":+%0.1f,\"RelativeHumidity\":%0.1f},\"method\":\"thing.event.property.post\"}",temp, humid);
//                sprintf((char*)payload_out,"{\"commandCode\":\"1000\",\"data\":{\"CurrentTemperature\":\"%0.1f\",\"RelativeHumidity\":\"%0.1f\"},\"dev\":\"arm test\"}",temp, humid);

//				payload_out_len = strlen((char*)payload_out);
//				topicString.cstring = DEVICE_PUBLISH;		//属性上报 发布
//				log_d("send PUBLISH buff = %s\r\n",payload_out);
//				len = MQTTSerialize_publish((unsigned char*)buf, buflen, 0, req_qos, retained, msgid, topicString, payload_out, payload_out_len);//发布消息
//				rc = transport_sendPacketBuffer(mysock, (unsigned char*)buf, len);
//				if(rc == len)															//
//					log_d("the %dth send PUBLISH Successfully\r\n",send_cnt++);
//				else
//					log_d("send PUBLISH failed\r\n");  
//				log_d("send temp(%0.1f)&humid(%0.1f) !\r\n",temp, humid);
//			}
//		}
		switch(msgtypes)
		{

            //连接服务端 客户端请求连接服务端
            case CONNECT:	len = MQTTSerialize_connect((unsigned char*)buf, buflen, &data); 						//获取数据组长度		发送连接信息     
							rc = transport_sendPacketBuffer(mysock, (unsigned char*)buf, len);		//发送 返回发送数组长度
							if(rc == len)															//
								log_d("send CONNECT Successfully\r\n");
							else
								log_d("send CONNECT failed\r\n");               
							log_d("step = %d,MQTT concet to server!\r\n",CONNECT);
							msgtypes = 0;
							break;
           //确认连接请求
			case CONNACK:   if(MQTTDeserialize_connack(&sessionPresent, &connack_rc, (unsigned char*)buf, buflen) != 1 || connack_rc != 0)	//收到回执
							{
								log_d("Unable to connect, return code %d\r\n", connack_rc);		//回执不一致，连接失败
							}
							else
							{
								log_d("step = %d,MQTT is concet OK!\r\n",CONNACK);									//连接成功
								connect_flag = 1;
                                gConnectStatus = 1;
							}
							msgtypes = SUBSCRIBE;													//连接成功 执行 订阅 操作
							break;
            //订阅主题 客户端订阅请求
			case SUBSCRIBE: topicString.cstring = DEVICE_SUBSCRIBE;
							len = MQTTSerialize_subscribe((unsigned char*)buf, buflen, 0, msgid, 1, &topicString, &req_qos);
							rc = transport_sendPacketBuffer(mysock, (unsigned char*)buf, len);
							if(rc == len)
								log_d("send SUBSCRIBE Successfully\r\n");
							else
							{
								log_d("send SUBSCRIBE failed\r\n"); 
								t++;
								if(t >= 10)
								{
									t = 0;
									msgtypes = CONNECT;
								}
								else
									msgtypes = SUBSCRIBE;
								break;
							}
							log_d("step = %d,client subscribe:[%s]\r\n",SUBSCRIBE,topicString.cstring);
							msgtypes = 0;
							break;
            //订阅确认 订阅请求报文确认
			case SUBACK: 	rc = MQTTDeserialize_suback(&submsgid, 1, &subcount, &granted_qos, (unsigned char*)buf, buflen);	//有回执  QoS                                                     
							log_d("step = %d,granted qos is %d\r\n",SUBACK, granted_qos);         								//打印 Qos                                                       
							msgtypes = 0;
							break;
            //发布消息
			case PUBLISH:	rc = MQTTDeserialize_publish(&dup, &qos, &retained, &msgid, &receivedTopic,&payload_in, &payloadlen_in, (unsigned char*)buf, buflen);	//读取服务器推送信息
							log_d("step = %d,message arrived : %s,len= %d\r\n",PUBLISH,payload_in,strlen(payload_in));
                            MqttDataParseAndSend(payload_in);
							msgtypes = 0;
							break;
            //发布确认 QoS	1消息发布收到确认
			case PUBACK:    log_d("step = %d,PUBACK!\r\n",PUBACK);					//发布成功
							msgtypes = 0;
							break;
            //发布收到 QoS2 第一步
			case PUBREC:    log_d("step = %d,PUBREC!\r\n",PUBREC);     				//just for qos2
							break;
            //发布释放 QoS2 第二步
			case PUBREL:    log_d("step = %d,PUBREL!\r\n",PUBREL);        			//just for qos2
							break;
            //发布完成 QoS2 第三步                            
			case PUBCOMP:   log_d("step = %d,PUBCOMP!\r\n",PUBCOMP);        			//just for qos2
							break;
            //心跳请求
			case PINGREQ:   len = MQTTSerialize_pingreq((unsigned char*)buf, buflen);							//心跳
							rc = transport_sendPacketBuffer(mysock, (unsigned char*)buf, len);
							if(rc == len)
								log_d("send PINGREQ Successfully\r\n");
							else
								log_d("send PINGREQ failed\r\n");       
								log_d("step = %d,time to ping mqtt server to take alive!\r\n",PINGREQ);
								msgtypes = 0;
							break;
            //心跳响应
			case PINGRESP:	log_d("step = %d,mqtt server Pong\r\n",PINGRESP);  			//心跳回执，服务有响应                                                     
							msgtypes = 0;
							break;
			default:
							break;

		}
		memset(buf,0,buflen);
		rc=MQTTPacket_read((unsigned char*)buf, buflen, transport_getdata);//轮询，读MQTT返回数据，
		log_d("MQTTPacket_read = %d\r\n",rc);		
		if(rc >0)//如果有数据，进入相应状态。
		{
			msgtypes = rc;
			log_d("MQTT is get recv: msgtypes = %d\r\n",msgtypes);
		}
	}
	transport_close(mysock);
    log_d("mqtt thread exit.\r\n");
}



/*
// C prototype : void HexToStr(BYTE *pbDest, BYTE *pbSrc, int nLen)
// parameter(s): [OUT] pbDest - 存放目标字符串
// [IN] pbSrc - 输入16进制数的起始地址
// [IN] nLen - 16进制数的字节数
// return value: 
// remarks : 将16进制数转化为字符串
*/
void HexToStr(uint8_t *pbDest, uint8_t *pbSrc, int nLen)
{
	char ddl,ddh;
	int i;

	for (i=0; i<nLen; i++)
	{
		ddh = 48 + pbSrc[i] / 16;
		ddl = 48 + pbSrc[i] % 16;
		if (ddh > 57) ddh = ddh + 7;
		if (ddl > 57) ddl = ddl + 7;
		pbDest[i*2] = ddh;
		pbDest[i*2+1] = ddl;
	}

	pbDest[nLen*2] = '\0';
}


//这里可以使用表驱动法，把每个操作放在结构体数组中，目前暂只作升级和远程开门
static void MqttDataParseAndSend(uint8_t *recvData)
{
    int cmd = 0;
    cJSON *json , *json_params, *json_cmd, *json_sw;
    json = cJSON_Parse((char *)recvData);         //解析数据包
    if (!json)  
    {  
        log_d("Error before: [%s]\r\n",cJSON_GetErrorPtr());  
    } 
    else
    {
        json_cmd = cJSON_GetObjectItem(json , "commandCode"); 
        if(json_cmd->type == cJSON_String)
        {
            log_d("commandCode:%s\r\n", json_cmd->valuestring);  
        }

        json_params = cJSON_GetObjectItem( json , "data" );

        cmd = atoi(json_cmd->valuestring);

        log_d("the cmd id =  %d\r\n",cmd);

        switch(cmd)
        {
            case 201:    
                    json_sw = cJSON_GetObjectItem( json_params , "identification" );
                    MqttSendDataToHost(json_sw->valuestring);
                    break;
            case 1016:
                    json_sw = cJSON_GetObjectItem( json_params , "softwareUrl" );
                    MqttGetUpgradeUrl(json_sw->valuestring);
            
                    ef_set_env("upData",recvData);
            
                    //设置标志位并重启
                    SystemUpdate();
                    
                    break;
			default:
				    break; 
        }         
    }  
    cJSON_Delete(json);
    cJSON_Delete(json_params);
    cJSON_Delete(json_cmd);
    cJSON_Delete(json_sw);     
    
}

//static int SaveUpNackParam(uint8_t *jsonData)
//{
//    int cmd = 0;
//    cJSON *jsonObj,*dataObj;    
//    cJSON *root , *json_params, *json_cmd, *json_sw;
//    cJSON *json_id,*json_status,*json_deviceCode,*json_productionModel,*json_version,*json_softwareFirmware,*json_versionType,*json_commandCode;
//    char *up_status,*TxdBuf;

//    jsonObj=cJSON_CreateObject(); // 创建dataobj对象，返回值为cJSON指针

//    if (!jsonObj)  
//    {  
//        log_d("jsonObj Error before: [%s]\r\n",cJSON_GetErrorPtr());  
//    } 
//    cJSON_AddItemToObject (jsonObj,"data",dataObj); 
//        
//    root = cJSON_Parse((char *)jsonData);         //解析数据包
//    if (!root)  
//    {  
//        log_d("Error before: [%s]\r\n",cJSON_GetErrorPtr());  
//    } 
//    else
//    {
//        log_d("jsonData = %s\r\n",jsonData);
//                
//        
//        json_cmd = cJSON_GetObjectItem(root , "commandCode"); 
//        json_deviceCode = cJSON_GetObjectItem( root , "deviceCode" );
//        if(json_cmd->type == cJSON_String)
//        {
//            log_d("commandCode:%s\r\n", json_cmd->valuestring);  
//        }

//        json_params = cJSON_GetObjectItem( root , "data" );        
//        json_id = cJSON_GetObjectItem( json_params , "id" );
//        json_status = cJSON_GetObjectItem( json_params , "status" );
//        json_productionModel = cJSON_GetObjectItem( json_params , "productionModel" );
//        json_version = cJSON_GetObjectItem( json_params , "version" );
//        json_softwareFirmware = cJSON_GetObjectItem( json_params , "softwareFirmware" );
//        json_versionType = cJSON_GetObjectItem( json_params , "versionType" );


//        cJSON_AddStringToObject(jsonObj,"commandCode",json_cmd->valuestring);
//         log_d("json_cmd->string = %s\r\n",json_cmd->valuestring);
//         
//        cJSON_AddStringToObject(jsonObj,"deviceCode",json_deviceCode->valuestring);  
//         log_d("json_deviceCode->string = %s\r\n",json_deviceCode->valuestring); 

//         
//         log_d("json_productionModel->string = %s\r\n",json_productionModel->valuestring);         
//        cJSON_AddStringToObject(dataObj,"productionModel",json_productionModel->valuestring);

//        log_d("json_version->string = %s\r\n",json_version->valuestring);
//        cJSON_AddStringToObject(dataObj,"version",json_version->valuestring);
//        
//        log_d("json_softwareFirmware->string = %s\r\n",json_softwareFirmware->valuestring);
//        cJSON_AddStringToObject(dataObj,"softwareFirmware",json_softwareFirmware->valuestring);

//        
//        log_d("json_versionType->string = %s\r\n",json_versionType->valuestring);
//        cJSON_AddStringToObject(dataObj,"versionType",json_versionType->valuestring); 
//        
//        log_d("json_id->valueint = %d\r\n",json_id->valueint);         
//        cJSON_AddNumberToObject(dataObj,"id",json_id->valueint);   
//        
//        up_status = ef_get_env("up_status");

//        log_d("up_status = %s\r\n",up_status);

//        if(memcmp(up_status,"101711",6) == 0)
//            cJSON_AddStringToObject(dataObj,"status","1");
//        else
//            cJSON_AddStringToObject(dataObj,"status","2");

//        TxdBuf = cJSON_PrintUnformatted(root); 
//        
//        if(TxdBuf == NULL)
//        {
//            return 0;
//        }          


//        ef_set_env("upData",TxdBuf);        
//        
//        log_d("send json data = %s\r\n",TxdBuf);

//        cJSON_Delete(jsonObj);
//        cJSON_Delete(root);
//        cJSON_Delete(dataObj);

//        my_free(up_status);
//        my_free(TxdBuf); 
//    }

//    

//}



static int MqttSendDataToHost(uint8_t *jsonData)
{
	MQTTString topicString = MQTTString_initializer;
    
	uint32_t len = 0;
	int32_t rc = 0;
	unsigned char buf[1024];
	int buflen = sizeof(buf);
	unsigned char payload_out[200];
	int payload_out_len = 0;

	unsigned short msgid = 1;
	int req_qos = 0;
	unsigned char retained = 0;  

   if(gConnectStatus == 1)
   {
       sprintf((char*)payload_out,"{\"commandCode\":\"201\",\"data\":{\"identification\":\"%s\",\"openStatus\":\"1\"},\"deviceCode\":\"3E51E8848A4C00863617\"}",jsonData);
 
       payload_out_len = strlen((char*)payload_out);
       topicString.cstring = DEVICE_PUBLISH;       //属性上报 发布
       log_d("send PUBLISH buff = %s\r\n",payload_out);
       len = MQTTSerialize_publish((unsigned char*)buf, buflen, 0, req_qos, retained, msgid, topicString, payload_out, payload_out_len);//发布消息
       rc = transport_sendPacketBuffer(mysock, (unsigned char*)buf, len);
       if(rc == len)                                                           //
           log_d("send PUBLISH Successfully\r\n");
       else
           log_d("send PUBLISH failed\r\n");     
      
   }

   return 0;
}


static int MqttGetUpgradeUrl(uint8_t *jsonData)
{

    cJSON *json , *json_url;
    json = cJSON_Parse((char *)jsonData);         //解析数据包
    if (!json)  
    {  
        log_d("Error before: [%s]\r\n",cJSON_GetErrorPtr());  
    } 
    else
    {
        json_url = cJSON_GetObjectItem(json , "picUrl"); 
        if(json_url->type == cJSON_String)
        {
            log_d("picUrl:%s\r\n", json_url->valuestring);  

            ef_set_env("url", json_url->valuestring);
            ef_set_env("up_status", "101700");
        }        
    }  

    
    cJSON_Delete(json);
    cJSON_Delete(json_url);   
    
    return 0;
}



