/******************************************************************************

                  ��Ȩ���� (C), 2013-2023, ���ڲ�˼�߿Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : comm.c
  �� �� ��   : ����
  ��    ��   : �Ŷ�
  ��������   : 2019��6��18��
  ����޸�   :
  ��������   : ��������ָ��
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2019��6��18��
    ��    ��   : �Ŷ�
    �޸�����   : �����ļ�

******************************************************************************/

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include "comm.h"
#include "tool.h"
#include "bsp_led.h"
#include "malloc.h"
#include "ini.h"
#include "bsp_uart_fifo.h"
#include "version.h"
#include "errorcode.h"



/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
#define DIM(x)  (sizeof(x)/sizeof(x[0]))


/*----------------------------------------------*
 * ��������                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/
static SYSERRORCODE_E OpenDoor ( char* msgBuf ); //����
static SYSERRORCODE_E AbnormalAlarm ( char* msgBuf ); //Զ�̱���
static SYSERRORCODE_E AddCardNo ( char* msgBuf ); //���ӿ���
static SYSERRORCODE_E DelCardNo ( char* msgBuf ); //ɾ������
static SYSERRORCODE_E UpgradeDev ( char* msgBuf ); //���豸��������
static SYSERRORCODE_E UpgradeAck ( char* msgBuf ); //����Ӧ��
static SYSERRORCODE_E EnableDev ( char* msgBuf ); //�����豸
static SYSERRORCODE_E DisableDev ( char* msgBuf ); //�ر��豸
static SYSERRORCODE_E SetDevParam ( char* msgBuf ); //���ò���
static SYSERRORCODE_E SetJudgeMode ( char* msgBuf ); //����ʶ��ģʽ
static SYSERRORCODE_E GetDevInfo ( char* msgBuf ); //��ȡ�豸��Ϣ



typedef SYSERRORCODE_E ( *cmd_fun ) ( char* msgBuf ); //��ȡSN�Ļص�

typedef struct
{
	const char* cmd_id;            /* ����id */
	cmd_fun  fun_ptr;     /* ����ָ�� */
} CMD_HANDLE_T;

CMD_HANDLE_T CmdList[] =
{
	{"201",  OpenDoor},
	{"1006", AbnormalAlarm},
	{"1012", AddCardNo},
	{"1013", DelCardNo},
	{"1016", UpgradeDev},
	{"1017", UpgradeAck},
	{"1021", EnableDev},
	{"1023", SetDevParam},
	{"1024", SetJudgeMode},
	{"1026", GetDevInfo}
};


SYSERRORCODE_E exec_proc ( char* cmd_id, char* msg_buf )
{

	SYSERRORCODE_E result = NO_ERR;
	int i = 0;

	for ( i = 0; i < DIM ( CmdList ); i++ )
	{
		if ( 0 == strcmp ( CmdList[i].cmd_id, cmd_id ) )
		{
			CmdList[i].fun_ptr ( msg_buf );
			return ;
		}
	}
	printf ( "invalid id %s\n", cmd_id );



	return result;
}



SYSERRORCODE_E OpenDoor ( char* msgBuf )
{
	SYSERRORCODE_E result = NO_ERR;
	return result;

}

SYSERRORCODE_E AbnormalAlarm ( char* msgBuf )
{
	SYSERRORCODE_E result = NO_ERR;
	return result;

}

SYSERRORCODE_E AddCardNo ( char* msgBuf )
{
	SYSERRORCODE_E result = NO_ERR;
	return result;

}

SYSERRORCODE_E DelCardNo ( char* msgBuf )
{
	SYSERRORCODE_E result = NO_ERR;
	return result;

}

SYSERRORCODE_E UpgradeDev ( char* msgBuf )
{
	SYSERRORCODE_E result = NO_ERR;
	return result;

}

SYSERRORCODE_E UpgradeAck ( char* msgBuf )
{
	SYSERRORCODE_E result = NO_ERR;
	return result;

}

SYSERRORCODE_E EnableDev ( char* msgBuf )
{
	SYSERRORCODE_E result = NO_ERR;
	return result;

}

SYSERRORCODE_E DisableDev ( char* msgBuf )
{
	SYSERRORCODE_E result = NO_ERR;
	return result;

}

SYSERRORCODE_E SetDevParam ( char* msgBuf )
{
	SYSERRORCODE_E result = NO_ERR;
	return result;

}

SYSERRORCODE_E SetJudgeMode ( char* msgBuf )
{
	SYSERRORCODE_E result = NO_ERR;
	return result;

}

SYSERRORCODE_E GetDevInfo ( char* msgBuf )
{
	SYSERRORCODE_E result = NO_ERR;
	return result;

}




