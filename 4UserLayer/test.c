/******************************************************************************

                  版权所有 (C), 2013-2023, 深圳博思高科技有限公司

 ******************************************************************************
  文 件 名   : test.c
  版 本 号   : 初稿
  作    者   : 张舵
  生成日期   : 2019年5月28日
  最近修改   :
  功能描述   : 功能模块测试
  函数列表   :
  修改历史   :
  1.日    期   : 2019年5月28日
    作    者   : 张舵
    修改内容   : 创建文件

******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "test.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bsp_flash.h"
#include "malloc.h"
#include "tool.h"


#define LOG_TAG    "TEST"
#include "elog.h"


void FlashTest(void)
{
    uint8_t data[4096] = {0};    
    sfud_err result = SFUD_SUCCESS;    
    size_t size = sizeof(data);
    uint32_t addr = 4096;
    size_t i;

    const sfud_flash * myFlash = sfud_get_device_table() + 0;

    
    /* prepare write data */
    for (i = 0; i < size; i++) {
        data[i] = i;
    }
    /* erase test */
    result = sfud_erase(myFlash, addr, size);
    if (result == SFUD_SUCCESS) {
        printf("Erase the %s flash data finish. Start from 0x%08X, size is %ld.\r\n", myFlash->name, addr,
                size);
    } else {
        printf("Erase the %s flash data failed.\r\n", myFlash->name);
        return;
    }
    /* write test */
    result = sfud_write(myFlash, addr, size, data);
    if (result == SFUD_SUCCESS) {
        printf("Write the %s flash data finish. Start from 0x%08X, size is %ld.\r\n", myFlash->name, addr,
                size);
    } else {
        printf("Write the %s flash data failed.\r\n", myFlash->name);
        return;
    }
    /* read test */
    result = sfud_read(myFlash, addr, size, data);
    if (result == SFUD_SUCCESS) {
        printf("Read the %s flash data success. Start from 0x%08X, size is %ld. The data is:\r\n", myFlash->name, addr,
                size);
        printf("Offset (h) 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F\r\n");
        for (i = 0; i < size; i++) {
            if (i % 16 == 0) {
                printf("[%08X] ", addr + i);
            }
            printf("%02X ", data[i]);
            if (((i + 1) % 16 == 0) || i == size - 1) {
                printf("\r\n");
            }
        }
        printf("\r\n");
    } else {
        printf("Read the %s flash data failed.\r\n", myFlash->name);
    }
    
    /* data check */
    for (i = 0; i < size; i++) {
        if (data[i] != i % 256) {
            printf("Read and check write data has an error. Write the %s flash data failed.\r\n", myFlash->name);
			break;
        }
    }

    if (i == size) {
        printf("The %s flash test is success.\r\n", myFlash->name);
    }    
}


void test_env(void)
{
    uint16_t i = 0;
    uint8_t kkk[12+1] = {0};
    uint8_t value[12+1] = {0};
    uint32_t curtick  =  xTaskGetTickCount();

   printf("start write env\r\n");

    
    for(i=1001;i<=2001;i++)
    {
        sprintf(kkk,"%012d",i);       
        sprintf(value,"%012d",2001-i);

        
        ef_set_env(kkk, value);

        memset(kkk,0x00,sizeof(kkk));
        memset(value,0x00,sizeof(value));
    }
    
    printf("end write env,calcRunTime = %d\r\n",xTaskGetTickCount()-curtick);
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

/* Create a bunch of objects as demonstration. */
static void create_objects(void)
{
    /* declare a few. */
    cJSON *root = NULL;
    cJSON *led = NULL; 
    cJSON *motor = NULL; 
    
    /* Here we construct some JSON standards, from the JSON site. */

    /* Our "Video" datatype: */
    root = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "sensor", cJSON_CreateString("01"));
    cJSON_AddItemToObject(root, "led", led = cJSON_CreateObject());
    cJSON_AddStringToObject(led, "RRD", "01");
    cJSON_AddStringToObject(led, "RGN", "00");
    cJSON_AddStringToObject(led, "LRD", "01");
    cJSON_AddStringToObject (led, "LGN", "00");
    cJSON_AddItemToObject(root, "motor", motor = cJSON_CreateObject());
    cJSON_AddStringToObject(motor, "close", "0106080C00018A69");


    /* Print to text */
    if (print_preallocated(root) != 0) {
        cJSON_Delete(root);
        exit(EXIT_FAILURE);
    }
    cJSON_Delete(root);
    my_free(led);
    my_free(motor);

}




int CJSON_CDECL json_test(void)
{
    /* print the version */
    printf("Version: %s\n", cJSON_Version());

    /* Now some samplecode for building objects concisely: */
    create_objects();

    return 0;
}


void addJsonTest(void)
{
    unsigned char payload_out[200];
    sprintf((char*)payload_out,"{\"commandCode\":\"201\",\"data\":{\"identification\":\"%s\",\"openStatus\":\"1\"},\"deviceCode\":\"3E51E8848A4C00863617\"}","134");


    log_d("payload_out = %s\r\n",payload_out);
    cJSON *json , *json_url,*json_new;
    json = cJSON_Parse((char *)payload_out);         //解析数据包
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
            
        }        
    }  

    json_new = cJSON_GetObjectItem( json , "data" ); 

    cJSON_AddStringToObject(json,"surge","test");
    cJSON_AddStringToObject(json,"111324","22222");
    cJSON_AddStringToObject(json_new,"442342","te11111st");
    cJSON_AddStringToObject(json_new,"42413125","222");

    if (print_preallocated(json) != 0) 
    {
        cJSON_Delete(json);
        exit(EXIT_FAILURE);
    }
    cJSON_Delete(json);

    

}


#define TRANSHEAD_START_ADD 0x1E0000


//uint8_t SPI_Flash_Test(void)
//{
//	unsigned char buff[6] = {0xA5,0xA6,0xA7,0xA8,0xA9,0xA5,};
//	unsigned char rx_buff[6] = {0};
//	char ret = 0;
//	unsigned char times = 50;
//	unsigned int addr = TRANSHEAD_START_ADD;	    
//    
//	while(times)
//	{
//		bsp_FLASH_Write (buff, addr, 6);
//        dbh("wx_buff",buff,6);
//        
//		//再读出来，对比是否一致
//		memset(rx_buff,0x00,sizeof(rx_buff));
//		bsp_FLASH_Read (rx_buff, addr, 6);

//        dbh("rx_buff",rx_buff,6);
//        
//		ret = memcmp(buff,rx_buff,sizeof(buff));
//		
//		if(ret == 0)
//		{
//			break;
//		}

//		if(ret != 0 && times == 1)
//		{
//			return 1;
//		}

//		times--;
//	}
//	
//	return 0;	    
//}


//void spi_flash_demo(void)
//{
//	
//    unsigned char buff[1024] = {0};
//	unsigned char rx_buff[1024] = {0};
//	char ret = 0;
//	unsigned char times = 1;
//	unsigned int addr = TRANSHEAD_START_ADD;	
//    uint16_t i = 0;

//	/* 检测串行Flash OK */
//	printf("检测到串行Flash, ID = %08X, 型号: %s \r\n", g_tSF.ChipID , g_tSF.ChipName);
//	printf("    容量 : %dM字节, 扇区大小 : %d字节\r\n", g_tSF.TotalSize/(1024*1024), g_tSF.PageSize);    

//	
//	for(i=0;i<1024;i++)
//	{
//		buff[i] = i;
//	}	
//    
//    
//	while(times)
//	{
//		bsp_sf_WriteBuffer (buff, addr, 1024);
//        dbh("wx_buff",buff,1024);
//        
//		//再读出来，对比是否一致
//		memset(rx_buff,0x00,sizeof(rx_buff));
//		bsp_sf_ReadBuffer (rx_buff, addr, 1024);

//        dbh("rx_buff",rx_buff,1024);
//        
//		ret = memcmp(buff,rx_buff,sizeof(buff));
//		
//		if(ret == 0)
//		{
//			break;
//		}

//		if(ret != 0 && times == 1)
//		{
//			return ;
//		}

//		times--;
//	}
//    
//}



void calcRunTime(void)
{
    uint32_t curtick  =  xTaskGetTickCount();
    uint16_t i = 0;
    uint8_t kkk[12+1] = {0};

    char *value;

    value = ef_get_env("000000900654");

    printf("read value = %s,calcRunTime = %d\r\n",value,xTaskGetTickCount()-curtick);   
    
     for(i=1000;i>=1;i--)
     {
         sprintf(kkk,"%012d",i);
         curtick  =  xTaskGetTickCount();
         value = ef_get_env(kkk);
         printf("read value = %s,calcRunTime = %d\r\n",value,xTaskGetTickCount()-curtick);
         memset(kkk,0x00,sizeof(kkk));
     }   
}


