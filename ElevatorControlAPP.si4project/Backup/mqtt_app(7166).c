/**********************************************************************************************************
** �ļ���		:mqtt_app.c
** ����			:maxlicheng<licheng.chn@outlook.com>
** ����github	:https://github.com/maxlicheng
** ���߲���		:https://www.maxlicheng.com/	
** ��������		:2018-08-08
** ����			:mqtt�������
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

//static int MqttDataParseAndSend(uint8_t *recvData,int (*func)(uint8_t *));
static void MqttDataParseAndSend(uint8_t *recvData);
static int MqttGetUpgradeUrl(uint8_t *jsonData);
static int MqttSendDataToHost(uint8_t *jsonData);

    

int gConnectStatus = 0;
int	gMySock = 0;



void mqtt_thread(void)
{
	MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
	MQTTString receivedTopic;
	MQTTString topicString = MQTTString_initializer;
	
	int32_t rc = 0;
	unsigned char buf[MQTT_MAX_LEN];
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

	uint8_t connect_flag = 0;		//���ӱ�־
	uint8_t msgtypes = CONNECT;		//��Ϣ״̬��ʼ��
	uint8_t t=0;
	
	log_d("socket connect to server\r\n");
	gMySock = transport_open((char *)HOST_NAME,HOST_PORT);
	log_d("1.Sending to hostname %s port %d,gMySock = %d\r\n", HOST_NAME, HOST_PORT,gMySock);
	
	len = MQTTSerialize_disconnect((unsigned char*)buf,buflen);
	rc = transport_sendPacketBuffer(gMySock, (uint8_t *)buf, len);
	if(rc == len)															//
		log_d("send DISCONNECT Successfully\r\n");
	else
		log_d("send DISCONNECT failed\r\n"); 
    
	vTaskDelay(2500);
	
	log_d("socket connect to server\r\n");
	gMySock = transport_open((char *)HOST_NAME,HOST_PORT);
	log_d("2.Sending to hostname %s port %d,gMySock = %d\r\n", HOST_NAME, HOST_PORT,gMySock);

	data.clientID.cstring = CLIENT_ID;              //���
	data.keepAliveInterval = KEEPLIVE_TIME;         //���ֻ�Ծ
	data.username.cstring = USER_NAME;              //�û���
	data.password.cstring = PASSWORD;               //��Կ
	data.MQTTVersion = MQTT_VERSION;                //3��ʾ3.1�汾��4��ʾ3.11�汾
	data.cleansession = 1;    


    //��ȡ��ǰ�δ���Ϊ��������ʼʱ��
	uint32_t curtick  =	 xTaskGetTickCount();
	uint32_t sendtick =  xTaskGetTickCount();
    
	while(1)
	{
		if(( xTaskGetTickCount() - curtick) >(data.keepAliveInterval*200))		//ÿ��200��tick
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
//				taskENTER_CRITICAL();	//�����ٽ���(�޷����жϴ��)
//				temp =10.0;
//				taskEXIT_CRITICAL();		//�˳��ٽ���(���Ա��жϴ��)
//				
//				humid = 54.8+rand()%10+1;
				//sprintf((char*)payload_out,"{\"params\":{\"CurrentTemperature\":+%0.1f,\"RelativeHumidity\":%0.1f},\"method\":\"thing.event.property.post\"}",temp, humid);
//                sprintf((char*)payload_out,"{\"commandCode\":\"1000\",\"data\":{\"CurrentTemperature\":\"%0.1f\",\"RelativeHumidity\":\"%0.1f\"},\"dev\":\"arm test\"}",temp, humid);

//				payload_out_len = strlen((char*)payload_out);
//				topicString.cstring = DEVICE_PUBLISH;		//�����ϱ� ����
//				log_d("send PUBLISH buff = %s\r\n",payload_out);
//				len = MQTTSerialize_publish((unsigned char*)buf, buflen, 0, req_qos, retained, msgid, topicString, payload_out, payload_out_len);//������Ϣ
//				rc = transport_sendPacketBuffer(gMySock, (unsigned char*)buf, len);
//				if(rc == len)															//
//					log_d("the %dth send PUBLISH Successfully\r\n",send_cnt++);
//				else
//					log_d("send PUBLISH failed\r\n");  
//				log_d("send temp(%0.1f)&humid(%0.1f) !\r\n",temp, humid);
//			}
//		}
		switch(msgtypes)
		{

            //���ӷ���� �ͻ����������ӷ����
            case CONNECT:	len = MQTTSerialize_connect((unsigned char*)buf, buflen, &data); 						//��ȡ�����鳤��		����������Ϣ     
							rc = transport_sendPacketBuffer(gMySock, (unsigned char*)buf, len);		//���� ���ط������鳤��
							if(rc == len)															//
								log_d("send CONNECT Successfully\r\n");
							else
								log_d("send CONNECT failed\r\n");               
							log_d("step = %d,MQTT concet to server!\r\n",CONNECT);
							msgtypes = 0;
							break;
           //ȷ����������
			case CONNACK:   if(MQTTDeserialize_connack(&sessionPresent, &connack_rc, (unsigned char*)buf, buflen) != 1 || connack_rc != 0)	//�յ���ִ
							{
								log_d("Unable to connect, return code %d\r\n", connack_rc);		//��ִ��һ�£�����ʧ��
							}
							else
							{
								log_d("step = %d,MQTT is concet OK!\r\n",CONNACK);									//���ӳɹ�
								connect_flag = 1;
                                gConnectStatus = 1;
							}
							msgtypes = SUBSCRIBE;													//���ӳɹ� ִ�� ���� ����
							break;
            //�������� �ͻ��˶�������
			case SUBSCRIBE: topicString.cstring = DEVICE_SUBSCRIBE;
							len = MQTTSerialize_subscribe((unsigned char*)buf, buflen, 0, msgid, 1, &topicString, &req_qos);
							rc = transport_sendPacketBuffer(gMySock, (unsigned char*)buf, len);
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
            //����ȷ�� ����������ȷ��
			case SUBACK: 	rc = MQTTDeserialize_suback(&submsgid, 1, &subcount, &granted_qos, (unsigned char*)buf, buflen);	//�л�ִ  QoS                                                     
							log_d("step = %d,granted qos is %d\r\n",SUBACK, granted_qos);         								//��ӡ Qos                                                       
							msgtypes = 0;
							break;
            //������Ϣ
			case PUBLISH:	rc = MQTTDeserialize_publish(&dup, &qos, &retained, &msgid, &receivedTopic,&payload_in, &payloadlen_in, (unsigned char*)buf, buflen);	//��ȡ������������Ϣ
							log_d("step = %d,message arrived : %s,len= %d\r\n",PUBLISH,payload_in,strlen(payload_in));
//                            MqttDataParseAndSend(payload_in);

                            //����������ִ�У����Ƿ��͵���Ϣ�����У��ڶ���Ϣ������ִ�У�
                            //���˸о�����Ϣ�����л�õ�
                            exec_proc(GetJsonItem(payload_in,CMD_ID,0),payload_in);
							msgtypes = 0;
							break;
            //����ȷ�� QoS	1��Ϣ�����յ�ȷ��
			case PUBACK:    log_d("step = %d,PUBACK!\r\n",PUBACK);					//�����ɹ�
							msgtypes = 0;
							break;
            //�����յ� QoS2 ��һ��
			case PUBREC:    log_d("step = %d,PUBREC!\r\n",PUBREC);     				//just for qos2
							break;
            //�����ͷ� QoS2 �ڶ���
			case PUBREL:    log_d("step = %d,PUBREL!\r\n",PUBREL);        			//just for qos2
							break;
            //������� QoS2 ������                            
			case PUBCOMP:   log_d("step = %d,PUBCOMP!\r\n",PUBCOMP);        			//just for qos2
							break;
            //��������
			case PINGREQ:   len = MQTTSerialize_pingreq((unsigned char*)buf, buflen);							//����
							rc = transport_sendPacketBuffer(gMySock, (unsigned char*)buf, len);
							if(rc == len)
								log_d("send PINGREQ Successfully\r\n");
							else
								log_d("send PINGREQ failed\r\n");       
								log_d("step = %d,time to ping mqtt server to take alive!\r\n",PINGREQ);
								msgtypes = 0;
							break;
            //������Ӧ
			case PINGRESP:	log_d("step = %d,mqtt server Pong\r\n",PINGRESP);  			//������ִ����������Ӧ                                                     
							msgtypes = 0;
							break;
			default:
							break;

		}
		memset(buf,0,buflen);
		rc=MQTTPacket_read((unsigned char*)buf, buflen, transport_getdata);//��ѯ����MQTT�������ݣ�
		log_d("MQTTPacket_read = %d\r\n",rc);		
		if(rc >0)//��������ݣ�������Ӧ״̬��
		{
			msgtypes = rc;
			log_d("MQTT is get recv: msgtypes = %d\r\n",msgtypes);
		}
	}
	transport_close(gMySock);
    log_d("mqtt thread exit.\r\n");
}


//�������ʹ�ñ�����������ÿ���������ڽṹ�������У�Ŀǰ��ֻ��������Զ�̿���
static void MqttDataParseAndSend(uint8_t *recvData)
{
    int cmd = 0;
    cJSON *json , *json_params, *json_cmd, *json_sw;
    json = cJSON_Parse((char *)recvData);         //�������ݰ�
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
            
                    //���ñ�־λ������
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


int PublishData(uint8_t *payload_out,uint16_t payload_out_len)
{
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

   if(gConnectStatus == 1)
   { 
       topicString.cstring = DEVICE_PUBLISH;       //�����ϱ� ����       

       len = MQTTSerialize_publish((unsigned char*)buf, buflen, 0, req_qos, retained, msgid, topicString, payload_out, payload_out_len);//������Ϣ
       rc = transport_sendPacketBuffer(gMySock, (unsigned char*)buf, len);
       if(rc == len)                                                           //
           log_d("send PUBLISH Successfully\r\n");
       else
           log_d("send PUBLISH failed\r\n");     
      
   }

   return len;
}




static int MqttSendDataToHost(uint8_t *jsonData)
{
	MQTTString topicString = MQTTString_initializer;
    
	uint32_t len = 0;
	int32_t rc = 0;
	unsigned char buf[MQTT_MAX_LEN];
	int buflen = sizeof(buf);
	unsigned char payload_out[MQTT_MAX_LEN];
	int payload_out_len = 0;

	unsigned short msgid = 1;
	int req_qos = 0;
	unsigned char retained = 0;  

   if(gConnectStatus == 1)
   {
       sprintf((char*)payload_out,"{\"commandCode\":\"201\",\"data\":{\"identification\":\"%s\",\"openStatus\":\"1\"},\"deviceCode\":\"3E51E8848A4C00863617\"}",jsonData);
 
       payload_out_len = strlen((char*)payload_out);
       topicString.cstring = DEVICE_PUBLISH;       //�����ϱ� ����
       log_d("send PUBLISH buff = %s\r\n",payload_out);
       len = MQTTSerialize_publish((unsigned char*)buf, buflen, 0, req_qos, retained, msgid, topicString, payload_out, payload_out_len);//������Ϣ
       rc = transport_sendPacketBuffer(gMySock, (unsigned char*)buf, len);
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
    json = cJSON_Parse((char *)jsonData);         //�������ݰ�
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


