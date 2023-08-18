/*
*********************************************************************************************************
*
*	ģ������ : ������ģ��
*	�ļ����� : main.c
*	��    �� : V1.0
*	˵    �� : �ռ��������EventRecorder��ʹ�á�
*              ʵ��Ŀ�ģ�
*                1. ѧϰ�ռ��������EventRecorder��ʹ�á�
*              ʵ�����ݣ�
*                1�������Զ���װ�����ʱ��0��ÿ200ms��תһ��LED2������bsp_DelayMS(5)��bsp_DelayMS(30)
*                   ��ʵ��ִ��ʱ�䡣
*                2������һ��Ƶ��Ϊ500Hz�Ķ�ʱ���жϣ�����������жϵ�ʵ��ִ��ʱ�䡣
*                3��printf���ض���ʹ��EventRecoderʵ�֡�             
*              ʵ�������
*                1��K1�����£�����EventRecord2��EventRecord4��EventRecordData���ܡ�
*                2��K2�����£����ڴ�ӡ��Ϣ��
*                3��K3�����£����ڴ�ӡ��Ϣ��
*              ע�����
*                1. ��ʵ���Ƽ�ʹ�ô������SecureCRT�鿴��ӡ��Ϣ��������115200������λ8����żУ��λ�ޣ�ֹͣλ1��
*                2. ��ؽ��༭��������������TAB����Ϊ4���Ķ����ļ���Ҫ��������ʾ�����롣
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
#define EXAMPLE_NAME	"V7-�ռ��������EventRecorder��ʹ��"
#define EXAMPLE_DATE	"2018-12-12"
#define DEMO_VER		"1.0"

static void PrintfLogo(void);
static void PrintfHelp(void);
extern void vEventRecorderTest(void);
uint8_t s_ucBuf[10] = "armfly";

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
	uint32_t t0 = 0, t1 = 0, t2 = 0, t3 = 0, t4 = 0;
	
	bsp_Init();		/* Ӳ����ʼ�� */
	
	PrintfLogo();	/* ��ӡ�������ƺͰ汾����Ϣ */
	PrintfHelp();	/* ��ӡ������ʾ */

	bsp_StartAutoTimer(0, 200);	/* ����1��200ms���Զ���װ�Ķ�ʱ�� */

	/* �����ж����� */
	vEventRecorderTest();
	
	/* ����������ѭ���� */
	while (1)
	{
		bsp_Idle();		/* ���������bsp.c�ļ����û������޸��������ʵ��CPU���ߺ�ι�� */

		/* �ж϶�ʱ����ʱʱ�� */
		if (bsp_CheckTimer(0))	
		{
			bsp_LedToggle(2);
			
			EventStartA(0);
			EventStopA(0);
			
			EventStartA(1);
			bsp_DelayMS(5);
			EventStopA(1);
			
			EventStartA(2);
			bsp_DelayMS(30);
			EventStopA(2);
			
			t0++;
			EventStartAv(3, t0, t0);
			bsp_DelayMS(30);
			EventStopAv(3, t0, t0);
		}

		/* �����˲��ͼ���ɺ�̨systick�жϷ������ʵ�֣�����ֻ��Ҫ����bsp_GetKey��ȡ��ֵ���ɡ� */
		ucKeyCode = bsp_GetKey();	/* ��ȡ��ֵ, �޼�����ʱ���� KEY_NONE = 0 */
		if (ucKeyCode != KEY_NONE)
		{
			switch (ucKeyCode)
			{
				case KEY_DOWN_K1:			/* K1������ */
					t1 += 1;
					t2 += 2;
					EventRecord2(1+EventLevelAPI, t1, t2);
					t3 += 3;
					t4 += 4;
					EventRecord4(2+EventLevelOp, t1, t2, t3, t4);
					EventRecordData(3+EventLevelOp, s_ucBuf, sizeof(s_ucBuf));
					break;

				case KEY_DOWN_K2:			/* K2������ */
					printf("K2��������\r\n");
					break;

				case KEY_DOWN_K3:			/* K3������ */
					printf("K3��������\r\n");
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
	printf("������ʾ:\r\n");
	printf("1. K1�����£�����EventRecord2��EventRecord4��EventRecordData����\r\n");
	printf("2. K2�����£����ڴ�ӡ��\r\n");
	printf("3. K3�����£����ڴ�ӡ��3\r\n");	
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
