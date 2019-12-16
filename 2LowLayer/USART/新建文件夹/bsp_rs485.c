#include "bsp_rs485.h"	 
#include "delay.h"
#include "bsp_time.h"


//接收缓存区 	
static uint8_t RS485_RX_BUF[RS485_MAX_LEN];  	//接收缓冲,最大255个字节.
//接收到的数据长度
static uint8_t RS485_RX_CNT=0;  




void UART5_IRQHandler(void)
{
	uint8_t res;	  

	if(USART_GetITStatus(UART5, USART_IT_RXNE) != RESET)//接收到数据
	{	 	
        
	    res =USART_ReceiveData(UART5);//;读取接收到的数据UART5->DR

        USART_ClearITPendingBit(UART5, USART_IT_RXNE);  // 清除UART5的接收中断 ； 
		if(RS485_RX_CNT < RS485_MAX_LEN)
		{
			RS485_RX_BUF[RS485_RX_CNT]=res;		//记录接收到的值
			RS485_RX_CNT++;						//接收数据增加1 
		} 
	}  											 
} 


								 
//初始化IO 串口2
//bound:波特率	  
void bsp_RS485_Init(uint32_t bound)
{  	 
	
    GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_AHB1PeriphClockCmd(USART5_GPIO_RX_CLK|USART5_GPIO_TX_CLK|RCC_RS485_TXEN,ENABLE); //使能GPIO时钟
	RCC_APB1PeriphClockCmd(USART5_CLK,ENABLE);//使能UART5时钟
	
  //串口5引脚复用映射
	GPIO_PinAFConfig(USART5_GPIO_TX_PORT,USART5_TX_SOURCE,USART5_AF); //GPIOC12复用为UART5
	GPIO_PinAFConfig(USART5_GPIO_RX_PORT,USART5_RX_SOURCE,USART5_AF); //GPIOD2复用为UART5
	
	//UART5    
    GPIO_InitStructure.GPIO_Pin = USART5_TX_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用功能
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	//速度100MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //推挽复用输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //上拉
	GPIO_Init(USART5_GPIO_TX_PORT,&GPIO_InitStructure); //初始化


    GPIO_InitStructure.GPIO_Pin = USART5_RX_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用功能
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	//速度100MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //推挽复用输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //上拉
	GPIO_Init(USART5_GPIO_RX_PORT,&GPIO_InitStructure); //初始化
	
	
	//PG0推挽输出，485模式控制  
    GPIO_InitStructure.GPIO_Pin = PIN_RS485_TXEN; //GPIOG0
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	//速度100MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //推挽输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //上拉
	GPIO_Init(PORT_RS485_TXEN,&GPIO_InitStructure); //初始化PE2
	

   //UART5 初始化设置
	USART_InitStructure.USART_BaudRate = bound;//波特率设置
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
    USART_Init(UART5, &USART_InitStructure); //初始化串口2
	
    USART_Cmd(UART5, ENABLE);  //使能串口 2
	
	USART_ClearFlag(UART5, USART_FLAG_TC);
	

	USART_ITConfig(UART5, USART_IT_RXNE, ENABLE);//开启接受中断

	//Usart2 NVIC 配置
    NVIC_InitStructure.NVIC_IRQChannel = UART5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
	
	RS485_TX_EN=0;				//默认为接收模式	
}

//RS485发送len个字节.
//buf:发送区首地址
//len:发送的字节数(为了和本代码的接收匹配,这里建议不要超过64个字节)
void bsp_RS485_Send_Data(uint8_t *buf,uint16_t len)
{
	uint8_t t;
	RS485_TX_EN=1;			//设置为发送模式
	
  	for(t=0;t<len;t++)		//循环发送数据
	{
	    while(USART_GetFlagStatus(UART5,USART_FLAG_TC)==RESET); //等待发送结束		
        USART_SendData(UART5,buf[t]); //发送数据
	}	
    
	while(USART_GetFlagStatus(UART5,USART_FLAG_TC)==RESET); //等待发送结束		
	
	RS485_RX_CNT=0;	  
	RS485_TX_EN=0;				//设置为接收模式	
}


//RS485查询接收到的数据
//buf:接收缓存首地址
//len:读到的数据长度
void bsp_RS485_Receive_Data(uint8_t *buf,uint16_t *len)
{
	uint8_t rxlen=RS485_RX_CNT;
	uint8_t i=0;
	*len=0;				//默认为0
	delay_ms(10);		//等待10ms,连续超过10ms没有接收到一个数据,则认为接收结束    
    
	if(rxlen==RS485_RX_CNT&&rxlen)//接收到了数据,且接收完成了
	{
		for(i=0;i<rxlen;i++)
		{
			buf[i]=RS485_RX_BUF[i];	
		}		
		*len=RS485_RX_CNT;	//记录本次数据长度
		RS485_RX_CNT=0;		//清零
	}
}

//static uint8_t RS485_RecvOne (uint8_t *Str)
//{
//	if (RecvTop2 == RS485_RX_CNT) return 0;//read nothing

//	*Str = RS485_RX_BUF[RecvTop2];
//	RecvTop2++;

//	if (RecvTop2 >= RS485_MAX_LEN) RecvTop2 = 0;

//	return 1;//read one


//}


//uint16_t bsp_RS485_Receive_Data (uint8_t *buf,uint16_t len,uint32_t timeout_MilliSeconds)
//{
//	u16 RecvLen = 0;
//	u8 tmp[1] = {0};

//	if (len == 0) return 0;

//	g1msTimerUART2 = timeout_MilliSeconds;

//	while (1)
//	{
//		if (g1msTimerUART2 == 0) return RecvLen;

//		if (RS485_RecvOne (tmp) == 1) 
//		{
//			buf[RecvLen++] = tmp[0];
//		}

//		if (RecvLen >= len) return RecvLen;
//	}
//	
//}






