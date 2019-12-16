/******************************************************************************

                  版权所有 (C), 2013-2023, 深圳博思高科技有限公司

 ******************************************************************************
  文 件 名   : main.c
  版 本 号   : 初稿
  作    者   : 张舵
  生成日期   : 2019年7月9日
  最近修改   :
  功能描述   : 主程序模块
  函数列表   :
  修改历史   :
  1.日    期   : 2019年7月9日
    作    者   : 张舵
    修改内容   : 创建文件

******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "def.h"
#include "mqtt_app.h"

#define LOG_TAG    "main"
#include "elog.h"

/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
//任务优先级   



#define LED_TASK_PRIO	    ( tskIDLE_PRIORITY)
#define HANDSHAKE_TASK_PRIO	( tskIDLE_PRIORITY)
#define READER_TASK_PRIO	( tskIDLE_PRIORITY + 1)
#define QR_TASK_PRIO	    ( tskIDLE_PRIORITY + 1)
#define KEY_TASK_PRIO	    ( tskIDLE_PRIORITY + 2)
#define DISPLAY_TASK_PRIO	( tskIDLE_PRIORITY + 2)
#define MQTT_TASK_PRIO	    ( tskIDLE_PRIORITY + 3)
#define CMD_TASK_PRIO		( tskIDLE_PRIORITY + 4)
#define START_TASK_PRIO		( tskIDLE_PRIORITY + 5)




#define LED_STK_SIZE 		(1024)
#define CMD_STK_SIZE 		(1024*1)
#define START_STK_SIZE 	    (512)
#define QR_STK_SIZE 		(1024)
#define READER_STK_SIZE     (1024)
#define HANDSHAKE_STK_SIZE  (1024)
#define KEY_STK_SIZE        (1024)
#define MQTT_STK_SIZE        (1024)
#define DISPLAY_STK_SIZE        (1024)





//事件标志
#define TASK_BIT_0	 (1 << 0)
#define TASK_BIT_1	 (1 << 1)
#define TASK_BIT_2	 (1 << 2)
#define TASK_BIT_3	 (1 << 3)
#define TASK_BIT_4	 (1 << 4)
#define TASK_BIT_5	 (1 << 5)
#define TASK_BIT_6	 (1 << 6)
#define TASK_BIT_7	 (1 << 7)
#define TASK_BIT_8	 (1 << 8)

//读取电机状态最大次数
#define READ_MOTOR_STATUS_TIMES 20


#define TASK_BIT_ALL ( TASK_BIT_0 | TASK_BIT_1 | TASK_BIT_2 |TASK_BIT_3|TASK_BIT_4|TASK_BIT_5|TASK_BIT_6|TASK_BIT_7|TASK_BIT_8)

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/
//任务句柄
static TaskHandle_t xHandleTaskLed = NULL;      //LED灯
static TaskHandle_t xHandleTaskCmd = NULL;      //android通讯
static TaskHandle_t xHandleTaskReader = NULL;   //韦根读卡器
static TaskHandle_t xHandleTaskQr = NULL;       //二维码读头
static TaskHandle_t xHandleTaskStart = NULL;    //看门狗
static TaskHandle_t xHandleTaskHandShake = NULL;    // 握手
static TaskHandle_t xHandleTaskKey = NULL;      //B门按键
static TaskHandle_t xHandleTaskMqtt = NULL;      //MQTT 测试
static TaskHandle_t xHandleTaskDisplay = NULL;      //数码管



//事件句柄
static EventGroupHandle_t xCreatedEventGroup = NULL;


//可以做为脱机模式情况下，红外，读卡器，二维码来开门
//脱机模式，判定读卡器的编码范围，以及二维码的计算规则
#ifdef USEQUEUE
#define MONITOR_TASK_PRIO	( tskIDLE_PRIORITY + 4)
#define MONITOR_STK_SIZE   (1024)
static TaskHandle_t xHandleTaskMonitor = NULL;    //监控任务
static void vTaskMonitor(void *pvParameters);
static QueueHandle_t xTransQueue = NULL;
#endif





/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/

//任务函数
static void vTaskLed(void *pvParameters);
static void vTaskKey(void *pvParameters);
static void vTaskReader(void *pvParameters);
static void vTaskQR(void *pvParameters);
static void vTaskStart(void *pvParameters);
//上送开机次数
static void vTaskHandShake(void *pvParameters);
static void vTaskMqttTest(void *pvParameters);
static void vTaskDisplay(void *pvParameters);




static void AppTaskCreate(void);
static void AppObjCreate (void);
static void App_Printf(char *format, ...);
static void EasyLogInit(void);




int main(void)
{   
    //硬件初始化
    bsp_Init();  

    EasyLogInit();  
    
	mymem_init(SRAMIN);								//初始化内部内存池
	mymem_init(SRAMEX);								//初始化外部内存池
	mymem_init(SRAMCCM);	  					    //初始化CCM内存池

	while(lwip_comm_init() != 0) //lwip初始化
	{
        log_d("lwip init error!\r\n");
		delay_ms(1200);
	}

    log_d("lwip init success!\r\n");

	/* 创建任务通信机制 */
	AppObjCreate();

	/* 创建任务 */
	AppTaskCreate();
    
    /* 启动调度，开始执行任务 */
    vTaskStartScheduler();
    
}

/*
*********************************************************************************************************
*	函 数 名: AppTaskCreate
*	功能说明: 创建应用任务
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void AppTaskCreate (void)
{

#if LWIP_DHCP
        lwip_comm_dhcp_creat();                             //创建DHCP任务
#endif


    //跟android握手
    xTaskCreate((TaskFunction_t )vTaskHandShake,
                (const char*    )"vHandShake",       
                (uint16_t       )HANDSHAKE_STK_SIZE, 
                (void*          )NULL,              
                (UBaseType_t    )HANDSHAKE_TASK_PRIO,    
                (TaskHandle_t*  )&xHandleTaskHandShake);  

    //创建LED任务
    xTaskCreate((TaskFunction_t )vTaskLed,         
                (const char*    )"vTaskLed",       
                (uint16_t       )LED_STK_SIZE, 
                (void*          )NULL,              
                (UBaseType_t    )LED_TASK_PRIO,    
                (TaskHandle_t*  )&xHandleTaskLed);                   

    //数码管
    xTaskCreate((TaskFunction_t )vTaskDisplay,
                (const char*    )"vTaskDisplay",       
                (uint16_t       )DISPLAY_STK_SIZE, 
                (void*          )NULL,              
                (UBaseType_t    )DISPLAY_TASK_PRIO,    
                (TaskHandle_t*  )&xHandleTaskDisplay);  
    
    //创建电机信息返回任务
//    xTaskCreate((TaskFunction_t )vTaskMortorToHost,     
//                (const char*    )"vTMTHost",   
//                (uint16_t       )MOTOR_STK_SIZE, 
//                (void*          )NULL,
//                (UBaseType_t    )MOTOR_TASK_PRIO,
//                (TaskHandle_t*  )&xHandleTaskMotor);     

    //跟android通讯串口数据解析
//    xTaskCreate((TaskFunction_t )vTaskMsgPro,     
//                (const char*    )"cmd",   
//                (uint16_t       )CMD_STK_SIZE, 
//                (void*          )NULL,
//                (UBaseType_t    )CMD_TASK_PRIO,
//                (TaskHandle_t*  )&xHandleTaskCmd);      

    //红外传感器状态上送
    xTaskCreate((TaskFunction_t )vTaskMqttTest,     
                (const char*    )"vMqttTest",   
                (uint16_t       )MQTT_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )MQTT_TASK_PRIO,
                (TaskHandle_t*  )&xHandleTaskMqtt);    


    //全高门电机状态返回
//    xTaskCreate((TaskFunction_t )vTaskRs485,     
//                (const char*    )"vRs485",   
//                (uint16_t       )RS485_STK_SIZE, 
//                (void*          )NULL,
//                (UBaseType_t    )RS485_TASK_PRIO,
//                (TaskHandle_t*  )&xHandleTaskRs485);  

    //韦根读卡器
//    xTaskCreate((TaskFunction_t )vTaskReader,     
//                (const char*    )"vReader",   
//                (uint16_t       )READER_STK_SIZE, 
//                (void*          )NULL,
//                (UBaseType_t    )READER_TASK_PRIO,
//                (TaskHandle_t*  )&xHandleTaskReader);    

    //二维码扫码模块
//    xTaskCreate((TaskFunction_t )vTaskQR,     
//                (const char*    )"vTaskQR",   
//                (uint16_t       )QR_STK_SIZE, 
//                (void*          )NULL,
//                (UBaseType_t    )QR_TASK_PRIO,
//                (TaskHandle_t*  )&xHandleTaskQr);      

    //B门按键
    xTaskCreate((TaskFunction_t )vTaskKey,         
                (const char*    )"vTaskKey",       
                (uint16_t       )KEY_STK_SIZE, 
                (void*          )NULL,              
                (UBaseType_t    )KEY_TASK_PRIO,    
                (TaskHandle_t*  )&xHandleTaskKey);   

    #ifdef USEQUEUE
    //监控线程
    xTaskCreate((TaskFunction_t )vTaskMonitor,     
                (const char*    )"vTaskMonitor",   
                (uint16_t       )MONITOR_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )MONITOR_TASK_PRIO,
                (TaskHandle_t*  )&xHandleTaskMonitor);
    #endif

    //看门狗
//	xTaskCreate((TaskFunction_t )vTaskStart,     		/* 任务函数  */
//                (const char*    )"vTaskStart",   		/* 任务名    */
//                (uint16_t       )START_STK_SIZE,        /* 任务栈大小，单位word，也就是4字节 */
//                (void*          )NULL,           		/* 任务参数  */
//                (UBaseType_t    )START_TASK_PRIO,       /* 任务优先级*/
//                (TaskHandle_t*  )&xHandleTaskStart );   /* 任务句柄  */                

}


/*
*********************************************************************************************************
*	函 数 名: AppObjCreate
*	功能说明: 创建任务通信机制
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void AppObjCreate (void)
{
	/* 创建事件标志组 */
	xCreatedEventGroup = xEventGroupCreate();
	
	if(xCreatedEventGroup == NULL)
    {
        /* 没有创建成功，用户可以在这里加入创建失败的处理机制 */
        App_Printf("创建事件标志组失败\r\n");
    }

	/* 创建互斥信号量 */
    gxMutex = xSemaphoreCreateMutex();
	
	if(gxMutex == NULL)
    {
        /* 没有创建成功，用户可以在这里加入创建失败的处理机制 */
        App_Printf("创建互斥信号量失败\r\n");
    }    

    //创建二值信号量
//    gBinarySem_Handle = xSemaphoreCreateBinary();

//    if(gBinarySem_Handle == NULL)
//    {
//        App_Printf("创建二值信号量失败\r\n");
//    }

    #ifdef USEQUEUE
    /* 创建消息队列 */
    xTransQueue = xQueueCreate((UBaseType_t ) QUEUE_LEN,/* 消息队列的长度 */
                              (UBaseType_t ) sizeof(QUEUE_TO_HOST_T *));/* 消息的大小 */
    if(xTransQueue == NULL)
    {
        App_Printf("创建xTransQueue消息队列失败!\r\n");
    }	
    #endif

}



/*
*********************************************************************************************************
*	函 数 名: vTaskStart
*	功能说明: 启动任务，等待所有任务发事件标志过来。
*	形    参: pvParameters 是在创建该任务时传递的形参
*	返 回 值: 无
*   优 先 级: 4  
*********************************************************************************************************
*/
static void vTaskStart(void *pvParameters)
{
	EventBits_t uxBits;
	const TickType_t xTicksToWait = 100 / portTICK_PERIOD_MS; /* 最大延迟1000ms */   
    
	/* 
	  开始执行启动任务主函数前使能独立看门狗。
	  设置LSI是32分频，下面函数参数范围0-0xFFF，分别代表最小值1ms和最大值4095ms
	  下面设置的是4s，如果4s内没有喂狗，系统复位。
	*/
	bsp_InitIwdg(4000);
	
	/* 打印系统开机状态，方便查看系统是否复位 */
	App_Printf("=====================================================\r\n");
	App_Printf("系统开机执行\r\n");
	App_Printf("=====================================================\r\n");
	
    while(1)
    {   
        
		/* 等待所有任务发来事件标志 */
		uxBits = xEventGroupWaitBits(xCreatedEventGroup, /* 事件标志组句柄 */
							         TASK_BIT_ALL,       /* 等待TASK_BIT_ALL被设置 */
							         pdTRUE,             /* 退出前TASK_BIT_ALL被清除，这里是TASK_BIT_ALL都被设置才表示“退出”*/
							         pdTRUE,             /* 设置为pdTRUE表示等待TASK_BIT_ALL都被设置*/
							         xTicksToWait); 	 /* 等待延迟时间 */
		
		if((uxBits & TASK_BIT_ALL) == TASK_BIT_ALL)
		{  
		    IWDG_Feed(); //喂狗			
		}
	    else
		{
			/* 基本是每xTicksToWait进来一次 */
			/* 通过变量uxBits简单的可以在此处检测那个任务长期没有发来运行标志 */

            //时序原因，值不太准确，需要更精准的方法

//            if((uxBits & TASK_BIT_0) != 0x01)
//            {
//                DBG("BIT_0 vTaskLed error = %02x,%02x   %02x \r\n",(uxBits & TASK_BIT_0),uxBits,TASK_BIT_0);
//            }

//            if((uxBits & TASK_BIT_1) != 0x02)
//            {
//                DBG("BIT_1 vTaskMotorToHost error = %02x, %02x   %02x \r\n",(uxBits & TASK_BIT_1),uxBits,TASK_BIT_1);
//            }

//            if((uxBits & TASK_BIT_2) != 0x04)
//            {
//                DBG("BIT_2 vTaskMsgPro error = %02x, %02x   %02x \r\n",(uxBits & TASK_BIT_2),uxBits,TASK_BIT_2);
//            }
//            
//            if((uxBits & TASK_BIT_3) != 0x08)
//            {
//                DBG("BIT_3 vTaskInfrared error = %02x, %02x   %02x \r\n",(uxBits & TASK_BIT_3),uxBits,TASK_BIT_3);
//            }

//            if((uxBits & TASK_BIT_4) != 0x10)
//            {
//                DBG("BIT_4 vTaskReader error = %02x,%02x   ,%02x \r\n",(uxBits & TASK_BIT_4),uxBits,TASK_BIT_4);
//            }

//            if((uxBits & TASK_BIT_5) != 0x20)
//            {
//                DBG("BIT_5 vTaskQR error = %02x,%02x   ,%02x \r\n",(uxBits & TASK_BIT_5),uxBits,TASK_BIT_5);
//            }       

//            if((uxBits & TASK_BIT_6) != 0x40)
//            {
//                DBG("BIT_6 vTaskRs485 error = %02x,%02x   ,%02x \r\n",(uxBits & TASK_BIT_6),uxBits,TASK_BIT_6);
//            }  

//            if((uxBits & TASK_BIT_7) != 0x80)
//            {
//                DBG("BIT_7 vTaskKey error = %02x,%02x   ,%02x \r\n",(uxBits & TASK_BIT_7),uxBits,TASK_BIT_7);
//            } 

//            if((uxBits & (TASK_BIT_8>>8)) != 0x01)
//            {
//                DBG("BIT_8 vTaskQueryMotor error = %04x,%02x   ,%02x \r\n",(uxBits & (TASK_BIT_8>>8)),uxBits,TASK_BIT_8);
//            } 

            
		}
    }
}




//LED任务函数 
void vTaskLed(void *pvParameters)
{       
    uint8_t i = 0;
    BEEP = 1;
    vTaskDelay(300);
    BEEP = 0;
    
    while(1)
    {  
        if(i == 250)
        {
            i = 100;
        }
        i+=20;
        bsp_SetTIMOutPWM(GPIOA, GPIO_Pin_8, TIM1, 1, 100, ((i) * 10000) /255);

        LED1=!LED1;  
        LED2=!LED2; 
        LED3=!LED3; 
        LED4=!LED4; 
        

        
		/* 发送事件标志，表示任务正常运行 */        
		xEventGroupSetBits(xCreatedEventGroup, TASK_BIT_0);  
        vTaskDelay(100);     
    }
}   

void vTaskKey(void *pvParameters)
{
    
	uint8_t ucKeyCode;
	uint8_t pcWriteBuffer[500];

    uint16_t crc_value = 0;

    uint8_t cm4[] = { 0x02,0x7B,0x22,0x63,0x6D,0x64,0x22,0x3A,0x22,0x75,0x70,0x64,0x61,0x74,0x65,0x22,0x2C,0x22,0x76,0x61,0x6C,0x75,0x65,0x22,0x3A,0x7B,0x22,0x75,0x70,0x64,0x61,0x74,0x65,0x22,0x3A,0x22,0x41,0x37,0x22,0x7D,0x2C,0x22,0x64,0x61,0x74,0x61,0x22,0x3A,0x22,0x30,0x30,0x22,0x7D,0x03 };

    
    
    while(1)
    {
        ucKeyCode = bsp_Key_Scan(0);      
		
		if (ucKeyCode != KEY_NONE)
		{
            //dbg("ucKeyCode = %d\r\n",ucKeyCode);
              
			switch (ucKeyCode)
			{
				/* K1键按下 打印任务执行情况 */
				case KEY_SET_PRES:	             
					App_Printf("=================================================\r\n");
					App_Printf("任务名      任务状态 优先级   剩余栈 任务序号\r\n");
					vTaskList((char *)&pcWriteBuffer);
					App_Printf("%s\r\n", pcWriteBuffer);
                    
					App_Printf("\r\n任务名       运行计数         使用率\r\n");
					vTaskGetRunTimeStats((char *)&pcWriteBuffer);
					App_Printf("%s\r\n", pcWriteBuffer);                    
					break;				
				/* K2键按下，打印串口操作命令 */
				case KEY_RR_PRES:
                
                    log_a("KEY_DOWN_K2\r\n");

			
					break;
				case KEY_LL_PRES:

                    log_i("KEY_DOWN_K3\r\n");  
					break;
				case KEY_OK_PRES:    
       
                    log_w("KEY_DOWN_K4\r\n");
                    crc_value = CRC16_Modbus(cm4, 54);
                    log_v("hi = %02x, lo = %02x\r\n", crc_value>>8, crc_value & 0xff);

					break;                
				
				/* 其他的键值不处理 */
				default:   
				log_e("KEY_default\r\n");
					break;
			}
		}

        /* 发送事件标志，表示任务正常运行 */
//		xEventGroupSetBits(xCreatedEventGroup, TASK_BIT_2);
		
		vTaskDelay(20);
	}   

}


void vTaskMqttTest(void *pvParameters)
{


    mqtt_thread();

    while(1)
    {
        vTaskDelay(500);        
    }

}


static void vTaskDisplay(void *pvParameters)
{
    while(1)
    {
        bsp_HC595Show(1,2,3);
        vTaskDelay(1000);
        bsp_HC595Show(4,5,6);
        vTaskDelay(1000);
        bsp_HC595Show(7,8,9);
        vTaskDelay(1000);
        bsp_HC595Show('a','b','c');
        vTaskDelay(1000);
        bsp_HC595Show('d','e','f');     
        vTaskDelay(1000);
        bsp_HC595Show('a',0,1);
        vTaskDelay(1000);
        bsp_HC595Show('d',3,4);   
        vTaskDelay(1000);
        bsp_HC595Show(1,0,1);  
        vTaskDelay(1000);
    }  

}



void vTaskReader(void *pvParameters)
{ 
    uint32_t CardID = 0;
    uint8_t dat[4] = {0};
    
//    uint32_t FunState = 0;
//    char *IcReaderState;
    #ifdef USEQUEUE
    QUEUE_TO_HOST_T *ptReaderToHost; 
    ptReaderToHost = &gQueueToHost;
    #endif
//    IcReaderState = ef_get_env("ICSTATE");
//    assert_param(IcReaderState);
//    FunState = atol(IcReaderState);
    
    while(1)
    {

//        if(FunState != 0x00)
        {
            CardID = bsp_WeiGenScanf();

            if(CardID != 0)
            {
                memset(dat,0x00,sizeof(dat));            
                
    			dat[0] = CardID>>24;
    			dat[1] = CardID>>16;
    			dat[2] = CardID>>8;
    			dat[3] = CardID&0XFF;    

                #ifdef USEQUEUE
                ptReaderToHost->cmd = WGREADER;
                memcpy(ptReaderToHost->data,dat,4);

    			/* 使用消息队列实现指针变量的传递 */
    			if(xQueueSend(xTransQueue,              /* 消息队列句柄 */
    						 (void *) &ptReaderToHost,   /* 发送结构体指针变量ptQueueToHost的地址 */
    						 (TickType_t)10) != pdPASS )
    			{
                    DBG("向xTransQueue发送数据失败，即使等待了10个时钟节拍\r\n");                
                } 
                else
                {
                    dbh("WGREADER",(char *)dat,4);
                }
                #endif
                
                send_to_host(WGREADER,dat,4);
            }  
        }


		/* 发送事件标志，表示任务正常运行 */        
		xEventGroupSetBits(xCreatedEventGroup, TASK_BIT_4);        
        
        vTaskDelay(100);
        
    }

}   


void vTaskQR(void *pvParameters)
{ 
    uint8_t recv_buf[256] = {0};
    uint16_t len = 0;  
    

    
    while(1)
    {   
           memset(recv_buf,0x00,sizeof(recv_buf));
           len = comRecvBuff(COM3,recv_buf,sizeof(recv_buf));

           if(len > 0  && recv_buf[len-1] == 0x0A && recv_buf[len-2] == 0x0D)
           {
                DBG("QR = %s\r\n",recv_buf);
                SendAsciiCodeToHost(QRREADER,NO_ERR,recv_buf);
           }
    

		/* 发送事件标志，表示任务正常运行 */        
		xEventGroupSetBits(xCreatedEventGroup, TASK_BIT_5);  
        vTaskDelay(500);        
    }
}   


void vTaskHandShake(void *pvParameters)
{
    uint32_t i_boot_times = NULL;
    char *c_old_boot_times, c_new_boot_times[12] = {0};
    uint8_t bcdbuf[6] = {0};

    /* get the boot count number from Env */
    c_old_boot_times = ef_get_env("boot_times");
    assert_param(c_old_boot_times);
    i_boot_times = atol(c_old_boot_times);
    
    /* boot count +1 */
    i_boot_times ++;

    /* interger to string */
    sprintf(c_new_boot_times,"%012ld", i_boot_times);
    
    /* set and store the boot count number to Env */
    ef_set_env("boot_times", c_new_boot_times);    

    asc2bcd(bcdbuf,(uint8_t *)c_new_boot_times , 12, 0);

    send_to_host(HANDSHAKE,bcdbuf,6);  
    
    vTaskDelete( NULL ); //删除自己


//    uint32_t i_boot_times = NULL;
//    char *c_old_boot_times, c_new_boot_times[12] = {0};
//    

//    g500usCount = 1000*10;

//    c_old_boot_times = ef_get_env("boot_times");

//    DBG("1.c_old_boot_times = %s\r\n",c_old_boot_times);

//    while(1)
//    {
//        if(g500usCount == 0)
//        {
//            break;
//        }
//        /* get the boot count number from Env */
//        c_old_boot_times = ef_get_env("boot_times");
//        assert_param(c_old_boot_times);
//        i_boot_times = atol(c_old_boot_times);
//        
//        /* boot count +1 */
//        i_boot_times ++;

//        /* interger to string */
//        sprintf(c_new_boot_times,"%012ld", i_boot_times);

//        /* set and store the boot count number to Env */
//        ef_set_env("boot_times", c_new_boot_times);   


//    }

//    c_old_boot_times = ef_get_env("boot_times");

//    DBG("2.c_old_boot_times = %s\r\n",c_old_boot_times);    

//    vTaskDelete( NULL ); //删除自己
}



/*
*********************************************************************************************************
*	函 数 名: App_Printf
*	功能说明: 线程安全的printf方式		  			  
*	形    参: 同printf的参数。
*             在C中，当无法列出传递函数的所有实参的类型和数目时,可以用省略号指定参数表
*	返 回 值: 无
*********************************************************************************************************
*/
static void  App_Printf(char *format, ...)
{
    char  buf_str[512 + 1];
    va_list   v_args;


    va_start(v_args, format);
   (void)vsnprintf((char       *)&buf_str[0],
                   (size_t      ) sizeof(buf_str),
                   (char const *) format,
                                  v_args);
    va_end(v_args);

	/* 互斥信号量 */
	xSemaphoreTake(gxMutex, portMAX_DELAY);

    printf("%s", buf_str);

   	xSemaphoreGive(gxMutex);
}



#ifdef USEQUEUE
static void vTaskMonitor(void *pvParameters)
{
  BaseType_t xReturn = pdTRUE;/* 定义一个创建信息返回值，默认为pdTRUE */
  QUEUE_TO_HOST_T *ptMsg;
  const TickType_t xMaxBlockTime = pdMS_TO_TICKS(200); /* 设置最大等待时间为200ms */  

  while (1)
  {
    xReturn = xQueueReceive( xTransQueue,    /* 消息队列的句柄 */
                             (void *)&ptMsg,  /*这里获取的是结构体的地址 */
                             xMaxBlockTime); /* 设置阻塞时间 */
    if(pdPASS == xReturn)
    {
//        DBG("ptMsg->cmd = %02x\r\n", ptMsg->cmd);
//        dbh("ptMsg->data ", (char *)ptMsg->data,QUEUE_BUF_LEN);

        switch (ptMsg->cmd)
        {
            case GETSENSOR:
                 DBG("红外数据\r\n");
                break;
             case CONTROLMOTOR:
                 DBG("A门电机数据\r\n");
                break;
            case DOOR_B:
                 DBG("B门电机数据\r\n");
                break;
            case WGREADER:
                 DBG("读卡器数据\r\n");
                break;            
            
        }
    }    

  }    
}
#endif


static void EasyLogInit(void)
{
    /* initialize EasyLogger */
     elog_init();
     /* set EasyLogger log format */
     elog_set_fmt(ELOG_LVL_ASSERT, ELOG_FMT_ALL);
     elog_set_fmt(ELOG_LVL_ERROR, ELOG_FMT_LVL | ELOG_FMT_TAG );
     elog_set_fmt(ELOG_LVL_WARN, ELOG_FMT_LVL | ELOG_FMT_TAG );
     elog_set_fmt(ELOG_LVL_INFO, ELOG_FMT_LVL | ELOG_FMT_TAG );
     elog_set_fmt(ELOG_LVL_DEBUG, ELOG_FMT_ALL & ~ELOG_FMT_TIME);
     elog_set_fmt(ELOG_LVL_VERBOSE, ELOG_FMT_ALL & ~ELOG_FMT_TIME);

     
     /* start EasyLogger */
     elog_start();  
}



