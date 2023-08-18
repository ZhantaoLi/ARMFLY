/*
*********************************************************************************************************
*
*	ģ������ : ������ģ��
*	�ļ����� : main.c
*	��    �� : V1.0
*	˵    �� : RS485���ͨѶ��ʹ�õ�USART3��
*              ʵ��Ŀ�ģ�
*                1. ѧϰRS485���ͨѶ��ʵ�֡�
*              ʵ�����ݣ�
*                1. ����ͨ�ž���϶̣�SP3485EоƬ��ȱʡδ���ĵ��費��Ҫ���ϣ���ҿ��Ը�����Ҫ���ϵ��������ԡ�
*                2. ������֧�ֶ��485�ڵ㣬����Ҫ�������豸�ʹ��豸�����нڵ����ش˳��򼴿ɡ�
*                3. �������485-A�������ӵ�һ��485-B�������ӵ�һ�𣬾������ӿ�����Doc�ļ����еĽ�ͼ��
*              ʵ�������
*                1. ���¿������ϵ�K1������LED1���ɿ�Ϩ��LED1��ͬʱ��ӡ�����¼�������1��485�����ϵ�������
*                   ��������ͬ�Ķ�����
*                2. ���¿������ϵ�K2��������50ms���Զ���װ��ʱ����ÿ��50ms��תLED2������485�����ϵ�������
*                   ���巢�Ͱ���K2������Ϣ���Ӷ�Ҳʵ��ÿ��50ms��תLED2��
*                3. ���¿������ϵ�K3������ֹͣK2����������50ms�Զ����ض�ʱ����485�����ϵ���������������ͬ
*                   �Ķ�����
*                4. ���¿������ϵ�ҡ�ˣ��������ң�OK��5�֣�����ͨ������1��ӡҡ���¼���485�����ϵ�������
*                   ��������ͬ�Ķ�����
*              ע�����
*                1. RS485 PHY�ķ���ʹ���õ�����PB11������ǰ��Ҫ������J5��������ñ�̽ӵ�PB11�ˡ�
*                2. ��ʵ���Ƽ�ʹ�ô������SecureCRT�鿴��ӡ��Ϣ��������115200������λ8����żУ��λ�ޣ�ֹͣλ1��
*                3. ��ؽ��༭��������������TAB����Ϊ4���Ķ����ļ���Ҫ��������ʾ�����롣
*
*	�޸ļ�¼ :
*		�汾��   ����         ����        ˵��
*		V1.0    2018-12-12   Eric2013     1. CMSIS����汾 V5.4.0
*                                         2. HAL��汾 V1.3.0
*
*	Copyright (C), 2018-2030, ���������� www.armfly.com
*
*********************************************************************************************************
*/	
#include "bsp.h"			/* �ײ�Ӳ������ */



/* ���������������̷������� */
#define EXAMPLE_NAME	"V7-RS485���ͨ��"
#define EXAMPLE_DATE	"2018-12-12"
#define DEMO_VER		"1.0"

static void PrintfLogo(void);
static void PrintfHelp(void);

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
	uint8_t ucDataTravel;	/* ���ͱ��� */
	uint8_t ucDataRec;	    /* ���ձ��� */
    

	bsp_Init();		/* Ӳ����ʼ�� */
	
	PrintfLogo();	/* ��ӡ�������ƺͰ汾����Ϣ */
	PrintfHelp();	/* ��ӡ������ʾ */

	/* ����������ѭ���� */
	while (1)
	{
		bsp_Idle();		/* ���������bsp.c�ļ����û������޸��������ʵ��CPU���ߺ�ι�� */
		
		/* ��ȡ����������ͨ��485���߷��������� */
		if(comGetChar(COM3, &ucDataRec))
		{
			switch (ucDataRec)
			{
				case KEY_DOWN_K1:			/* ���K1��������Ϣ */
					bsp_LedOn(1);
					printf("K1������, LED1����\r\n");
					break;

				case KEY_UP_K1:		  		/* ���K1���ͷ���Ϣ */
					bsp_LedOff(1);
					printf("K1������, LED1Ϩ��\r\n");
					break;

				case KEY_DOWN_K2:			/* ���K2��������Ϣ */
					bsp_LedToggle(2);
					break;

				case JOY_DOWN_U:			/* ���ҡ��UP������ */
					printf("ҡ���ϼ�����\r\n");
					break;

				case JOY_DOWN_D:			/* ���ҡ��DOWN������ */
					printf("ҡ���¼�����\r\n");
					break;

				case JOY_DOWN_L:			/* ���ҡ��LEFT������ */
					printf("ҡ���������\r\n");
					break;

				case JOY_DOWN_R:			/* ���ҡ��RIGHT������ */
					printf("ҡ���Ҽ�����\r\n");
					break;

				case JOY_DOWN_OK:			/* ���ҡ��OK������ */
					printf("ҡ��OK������\r\n");
					break;

				case JOY_UP_OK:				/* ���ҡ��OK������ */
					printf("ҡ��OK������\r\n");
					break;

				default:
					/* �����ļ�ֵ������ */
					break;
			}
		}

		/* �ж϶�ʱ����ʱʱ�� */
		if (bsp_CheckTimer(0))	
		{
			/* ÿ��50ms ����һ�� */  
			bsp_LedToggle(2);
			/* �����������巢�Ͱ���K2���µ���Ϣ */
			ucDataTravel = KEY_DOWN_K2;
			comSendChar(COM3, ucDataTravel);
		}

		/* �����˲��ͼ���ɺ�̨systick�жϷ������ʵ�֣�����ֻ��Ҫ����bsp_GetKey��ȡ��ֵ���ɡ� */
		ucKeyCode = bsp_GetKey();	/* ��ȡ��ֵ, �޼�����ʱ���� KEY_NONE = 0 */
		if (ucKeyCode != KEY_NONE)
		{
			switch (ucKeyCode)
			{
				case KEY_DOWN_K1:			/* K1������ */
					ucDataTravel = KEY_DOWN_K1;
					comSendChar(COM3, ucDataTravel);
					bsp_LedOn(1);
					printf("K1������, LED1����\r\n");
					break;

				case KEY_UP_K1:				/* K1������ */
					ucDataTravel = KEY_UP_K1;
					comSendChar(COM3, ucDataTravel);
					bsp_LedOff(1);
					printf("K1������, LED1Ϩ��\r\n");
					break;

				case KEY_DOWN_K2:			/* K2������ */
					bsp_StartAutoTimer(0, 50);	/* ����1��50ms���Զ���װ�Ķ�ʱ�� */
					break;

				case KEY_DOWN_K3:			/* K3������ */
					bsp_StopTimer(0);       /* ֹͣ�Զ���װ�Ķ�ʱ�� */
					break;

				case JOY_DOWN_U:			/* ҡ��UP������ */
					ucDataTravel = JOY_DOWN_U;
					comSendChar(COM3, ucDataTravel);
					printf("ҡ���ϼ�����\r\n");
					break;

				case JOY_DOWN_D:			/* ҡ��DOWN������ */
					ucDataTravel = JOY_DOWN_D;
					comSendChar(COM3, ucDataTravel);
					printf("ҡ���¼�����\r\n");
					break;

				case JOY_DOWN_L:			/* ҡ��LEFT������ */
					ucDataTravel = JOY_DOWN_L;
					comSendChar(COM3, ucDataTravel);
					printf("ҡ���������\r\n");
					break;

				case JOY_DOWN_R:			/* ҡ��RIGHT������ */
					ucDataTravel = JOY_DOWN_R;
					comSendChar(COM3, ucDataTravel);
					printf("ҡ���Ҽ�����\r\n");
					break;

				case JOY_DOWN_OK:			/* ҡ��OK������ */
					ucDataTravel = JOY_DOWN_OK;
					comSendChar(COM3, ucDataTravel);
					printf("ҡ��OK������\r\n");
					break;

				case JOY_UP_OK:				/* ҡ��OK������ */
					ucDataTravel = JOY_UP_OK;
					comSendChar(COM3, ucDataTravel);
					printf("ҡ��OK������\r\n");
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
*	�� �� ��: PrintfHelp
*	����˵��: ��ӡ������ʾ
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void PrintfHelp(void)
{
	printf("������ʾ, 485�����ϵ���������������ͬ�Ķ���:\r\n");
	printf("1. K1�������µ���LED1���ɿ�Ϩ��LED1��ͬʱ���ڴ�ӡ��Ϣ\r\n");
	printf("2. K2������������50ms���Զ���װ��ʱ����ÿ50ms��תLED2\r\n");
	printf("3. K3�������£�ֹͣK2����������50ms�Զ����ض�ʱ��\r\n");
	printf("4. ҡ�ˣ��������ң�OK��5�֣����£���ͨ������1��ӡҡ���¼�\r\n");
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
	printf("* ��������   : %s\r\n", EXAMPLE_NAME);	/* ��ӡ�������� */
	printf("* ���̰汾   : %s\r\n", DEMO_VER);		/* ��ӡ���̰汾 */
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
