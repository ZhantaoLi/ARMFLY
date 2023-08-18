/*
*********************************************************************************************************
*
*	ģ������ : ������ģ��
*	�ļ����� : main.c
*	��    �� : V1.0
*	˵    �� : Matlab�Ĵ�������ͨ�š�
*              ʵ��Ŀ�ģ�
*                1��ѧϰmatlab�Ĵ�������ͨ�š�
*              ʵ�����ݣ�
*                1������һ���Զ���װ�����ʱ����ÿ100ms��תһ��LED2��
*                2�����������п����壬Ȼ������matlab��
*                3������matlab�������ݷ���ǰ������عرմ������֡�
*              ע�����
*                1����ʵ���Ƽ�ʹ�ô������SecureCRT�鿴��ӡ��Ϣ��������115200������λ8����żУ��λ�ޣ�ֹͣλ1��
*                2����ؽ��༭��������������TAB����Ϊ4���Ķ����ļ���Ҫ��������ʾ�����롣
*
*	�޸ļ�¼ :
*		�汾��   ����         ����        ˵��
*		V1.0    2019-09-07   Eric2013     1. CMSIS����汾 V5.4.0
*                                         2. HAL��汾 V1.3.0
*
*	Copyright (C), 2018-2030, ���������� www.armfly.com
*
*********************************************************************************************************
*/	
#include "bsp.h"			/* �ײ�Ӳ������ */



/* ���������������̷������� */
#define EXAMPLE_NAME	"V7-Matlab�Ĵ�������ͨ��"
#define EXAMPLE_DATE	"2019-09-07"
#define DEMO_VER		"1.0"

static void PrintfLogo(void);
static void PrintfHelp(void);
static void Serial_sendDataMATLAB(void);

/* ���巢�͸�matlab�����ݸ�ʽ  */
__packed typedef struct
{
	uint16_t data1;
	uint16_t data2;	
	uint16_t data3;	
	uint8_t  data4;	
	uint8_t  data5;
	uint8_t  data6;		
	uint8_t  data7;
}
SENDPARAM_T;

SENDPARAM_T g_SendData;

/*
*********************************************************************************************************
*	�� �� ��: main
*	����˵��: c�������
*	��    ��: ��
*	�� �� ֵ: �������(���账��)
*********************************************************************************************************
*/
int main(void)
{
	uint8_t ucKeyCode;		/* �������� */
	uint8_t read;
	
	bsp_Init();		/* Ӳ����ʼ�� */
	
	PrintfLogo();	/* ��ӡ�������ƺͰ汾����Ϣ */
	PrintfHelp();	/* ��ӡ������ʾ */

	bsp_StartAutoTimer(0, 50);  /* ����1��100ms���Զ���װ�Ķ�ʱ�� */
	
	/* ����������ѭ���� */
	while (1)
	{
		bsp_Idle();		/* ���������bsp.c�ļ����û������޸��������ʵ��CPU���ߺ�ι�� */

		/* �ж϶�ʱ����ʱʱ�� */
		if (bsp_CheckTimer(0))	
		{
			/* ÿ��100ms ����һ�� */  
			bsp_LedToggle(2);
		}
		
		if (comGetChar(COM1, &read))
		{
			/* ���յ�ͬ��֡'$'*/
			if(read == 13)
			{
				bsp_LedToggle(4);
				bsp_DelayMS(10);
				Serial_sendDataMATLAB();
			}
		}
		
		/* �����˲��ͼ���ɺ�̨systick�жϷ������ʵ�֣�����ֻ��Ҫ����bsp_GetKey��ȡ��ֵ���ɡ� */
		ucKeyCode = bsp_GetKey();	/* ��ȡ��ֵ, �޼�����ʱ���� KEY_NONE = 0 */
		if (ucKeyCode != KEY_NONE)
		{
			switch (ucKeyCode)
			{
				case KEY_DOWN_K1:			/* K1������ */
					printf("K1������\r\n");
					break;

				case KEY_UP_K1:				/* K1������ */
					break;

				case KEY_DOWN_K2:			/* K2������ */
					printf("K2������\r\n");
					break;

				case KEY_UP_K2:				/* K2������ */
					printf("K2������\r\n");
					break;

				case KEY_DOWN_K3:			/* K3������ */
					printf("K3������\r\n");
					break;

				case KEY_UP_K3:				/* K3������ */
					printf("K3������\r\n");
					break;
				
				default:
					/* �����ļ�ֵ������ */
					break;
			}
		
		}
	}
}

/*
*********************************************************************************************************
*	�� �� ��: Serial_sendDataMATLAB
*	����˵��: ���ʹ������ݸ�matlab
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void Serial_sendDataMATLAB(void)
{
	/* �ȷ�ͬ���ź�'$' */
	comSendChar(COM1, 13);
	
	/* �������ݣ�һ��10���ֽ� */
	g_SendData.data1 = rand()%65536;
	g_SendData.data2 = rand()%65536;
	g_SendData.data3 = rand()%65536;
	g_SendData.data4 = rand()%256;  
	g_SendData.data5 = rand()%256;  
	g_SendData.data6 = rand()%256;  
	g_SendData.data7 = rand()%256;  
	comSendBuf(COM1, (uint8_t *)&g_SendData, 10);
}

/*
*********************************************************************************************************
*	�� �� ��: PrintfHelp
*	����˵��: ��ӡ������ʾ
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void PrintfHelp(void)
{
	printf("������ʾ:\r\n");
	printf("1. ����һ���Զ���װ�����ʱ����ÿ100ms��תһ��LED2\r\n");
	printf("2. ���������п����壬Ȼ������matlab\r\n");
	printf("3. ����matlab�������ݷ���ǰ������عرմ�������\r\n");
}

/*
*********************************************************************************************************
*	�� �� ��: PrintfLogo
*	����˵��: ��ӡ�������ƺ����̷�������, ���ϴ����ߺ󣬴�PC���ĳ����ն�������Թ۲���
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void PrintfLogo(void)
{
	printf("*************************************************************\n\r");
	
	/* ���CPU ID */
	{
		uint32_t CPU_Sn0, CPU_Sn1, CPU_Sn2;
		
		CPU_Sn0 = *(__IO uint32_t*)(0x1FF1E800);
		CPU_Sn1 = *(__IO uint32_t*)(0x1FF1E800 + 4);
		CPU_Sn2 = *(__IO uint32_t*)(0x1FF1E800 + 8);

		printf("\r\nCPU : STM32H743XIH6, BGA240, ��Ƶ: %dMHz\r\n", SystemCoreClock / 1000000);
		printf("UID = %08X %08X %08X\n\r", CPU_Sn2, CPU_Sn1, CPU_Sn0);
	}

	printf("\n\r");
	printf("*************************************************************\n\r");
	printf("* ��������   : %s\n\r", EXAMPLE_NAME);	/* ��ӡ�������� */
	printf("* ���̰汾   : %s\n\r", DEMO_VER);		/* ��ӡ���̰汾 */
	printf("* ��������   : %s\r\n", EXAMPLE_DATE);	/* ��ӡ�������� */

	/* ��ӡST��HAL��汾 */
	printf("* HAL��汾  : V1.3.0 (STM32H7xx HAL Driver)\r\n");
	printf("* \r\n");	/* ��ӡһ�пո� */
	printf("* QQ    : 1295744630 \r\n");
	printf("* ����  : armfly\r\n");
	printf("* Email : armfly@qq.com \r\n");
	printf("* ΢�Ź��ں�: armfly_com \r\n");
	printf("* �Ա���: armfly.taobao.com\r\n");
	printf("* Copyright www.armfly.com ����������\r\n");
	printf("*************************************************************\n\r");
}

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
