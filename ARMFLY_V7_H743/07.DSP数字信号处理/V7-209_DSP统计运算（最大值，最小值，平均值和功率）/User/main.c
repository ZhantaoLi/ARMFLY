/*
*********************************************************************************************************
*
*	ģ������ : ������ģ��
*	�ļ����� : main.c
*	��    �� : V1.0
*	˵    �� : DSPͳ�����㣨���ֵ����Сֵ��ƽ��ֵ�͹��ʣ�
*              ʵ��Ŀ�ģ�
*                1. ѧϰDSPͳ�����㣨���ֵ����Сֵ��ƽ��ֵ�͹��ʣ�
*              ʵ�����ݣ� 
*                1. ����һ���Զ���װ�����ʱ����ÿ100ms��תһ��LED2��
*                2. ���°���K1, DSP�����ֵ��
*                3. ���°���K2, DSP����Сֵ��
*                4. ���°���K3, DSP��ƽ��ֵ��
*                5. ����ҡ��OK��, DSP���ʡ�
*              ע�����
*                1. ��ʵ���Ƽ�ʹ�ô������SecureCRT�鿴��ӡ��Ϣ��������115200������λ8����żУ��λ�ޣ�ֹͣλ1��
*                2. ��ؽ��༭��������������TAB����Ϊ4���Ķ����ļ���Ҫ��������ʾ�����롣
*
*	�޸ļ�¼ :
*		�汾��   ����         ����        ˵��
*		V1.0    2019-10-25   Eric2013     1. CMSIS����汾 V5.6.0
*                                         2. HAL��汾 V1.3.0
*
*	Copyright (C), 2018-2030, ���������� www.armfly.com
*
*********************************************************************************************************
*/	
#include "bsp.h"			 /* �ײ�Ӳ������ */
#include "arm_math.h"



/* ���������������̷������� */
#define EXAMPLE_NAME	"V7-DSPͳ�����㣨���ֵ����Сֵ��ƽ��ֵ�͹��ʣ�"
#define EXAMPLE_DATE	"2019-10-25"
#define DEMO_VER		"1.0"

/* �������ļ��ڵ��õĺ������� */
static void PrintfLogo(void);
static void PrintfHelp(void);
static void DSP_Max(void);
static void DSP_Min(void);
static void DSP_Mean(void);
static void DSP_Power(void);


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
	

	bsp_Init();		/* Ӳ����ʼ�� */
	PrintfLogo();	/* ��ӡ������Ϣ������1 */

	PrintfHelp();	/* ��ӡ������ʾ��Ϣ */
	

	bsp_StartAutoTimer(0, 100);	/* ����1��100ms���Զ���װ�Ķ�ʱ�� */

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

		ucKeyCode = bsp_GetKey();	/* ��ȡ��ֵ, �޼�����ʱ���� KEY_NONE = 0 */
		if (ucKeyCode != KEY_NONE)
		{
			switch (ucKeyCode)
			{
				case KEY_DOWN_K1:			/* K1�����£������ֵ */
					DSP_Max();
					break;

				case KEY_DOWN_K2:			/* K2������, ��Сֵ */
					DSP_Min();
					break;

				case KEY_DOWN_K3:			/* K3�����£���ƽ���� */
					DSP_Mean();
					break;
				
				case JOY_DOWN_OK:            /* ҡ���ϼ�������  */         
					DSP_Power();
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
*	�� �� ��: DSP_Max
*	����˵��: �����ֵ
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void DSP_Max(void)
{
	float32_t pSrc[10] = {0.6948f, 0.3171f, 0.9502f, 0.0344f, 0.4387f, 0.3816f, 0.7655f, 0.7952f, 0.1869f, 0.4898f};
	float32_t pResult;
	uint32_t pIndex;
	
	q31_t pSrc1[10];
	q31_t pResult1;
	
	q15_t pSrc2[10];
	q15_t pResult2;
	
	q7_t pSrc3[10];
	q7_t pResult3;
	
	arm_max_f32(pSrc, 10, &pResult, &pIndex);
	printf("arm_max_f32 : pResult = %f  pIndex = %d\r\n", pResult, pIndex);
	
	/*****************************************************************/
	for(pIndex = 0; pIndex < 10; pIndex++)
	{
		 pSrc1[pIndex] = rand();
	}
	arm_max_q31(pSrc1, 10, &pResult1, &pIndex);
	printf("arm_max_q31 : pResult = %d  pIndex = %d\r\n", pResult1, pIndex);
	
	/*****************************************************************/
	for(pIndex = 0; pIndex < 10; pIndex++)
	{
		 pSrc2[pIndex] = rand()%32768;
	}
	arm_max_q15(pSrc2, 10, &pResult2, &pIndex);
	printf("arm_max_q15 : pResult = %d  pIndex = %d\r\n", pResult2, pIndex);
	
	/*****************************************************************/
	for(pIndex = 0; pIndex < 10; pIndex++)
	{
		 pSrc3[pIndex] = rand()%128;
	}
	arm_max_q7(pSrc3, 10, &pResult3, &pIndex);
	printf("arm_max_q7 : pResult = %d  pIndex = %d\r\n", pResult3, pIndex);
	printf("******************************************************************\r\n");
}

/*
*********************************************************************************************************
*	�� �� ��: DSP_Min
*	����˵��: ����Сֵ
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void DSP_Min(void)
{
	float32_t pSrc[10] = {0.6948f, 0.3171f, 0.9502f, 0.0344f, 0.4387f, 0.3816f, 0.7655f, 0.7952f, 0.1869f, 0.4898f};
	float32_t pResult;
	uint32_t pIndex;
	
	q31_t pSrc1[10];
	q31_t pResult1;
	
	q15_t pSrc2[10];
	q15_t pResult2;
	
	q7_t pSrc3[10];
	q7_t pResult3;
	
	arm_min_f32(pSrc, 10, &pResult, &pIndex);
	printf("arm_min_f32 : pResult = %f  pIndex = %d\r\n", pResult, pIndex);
	
	/*****************************************************************/
	for(pIndex = 0; pIndex < 10; pIndex++)
	{
		 pSrc1[pIndex] = rand();
	}
	arm_min_q31(pSrc1, 10, &pResult1, &pIndex);
	printf("arm_min_q31 : pResult = %d  pIndex = %d\r\n", pResult1, pIndex);
	
	/*****************************************************************/
	for(pIndex = 0; pIndex < 10; pIndex++)
	{
		 pSrc2[pIndex] = rand()%32768;
	}
	arm_min_q15(pSrc2, 10, &pResult2, &pIndex);
	printf("arm_min_q15 : pResult = %d  pIndex = %d\r\n", pResult2, pIndex);
	
	/*****************************************************************/
	for(pIndex = 0; pIndex < 10; pIndex++)
	{
		 pSrc3[pIndex] = rand()%128;
	}
	arm_min_q7(pSrc3, 10, &pResult3, &pIndex);
	printf("arm_min_q7 : pResult = %d  pIndex = %d\r\n", pResult3, pIndex);
	printf("******************************************************************\r\n");
}

/*
*********************************************************************************************************
*	�� �� ��: DSP_Mean
*	����˵��: ��ƽ��
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void DSP_Mean(void)
{
	float32_t pSrc[10] = {0.6948f, 0.3171f, 0.9502f, 0.0344f, 0.4387f, 0.3816f, 0.7655f, 0.7952f, 0.1869f, 0.4898f};
	float32_t pResult;
	uint32_t pIndex;
	
	q31_t pSrc1[10];
	q31_t pResult1;
	
	q15_t pSrc2[10];
	q15_t pResult2;
	
	q7_t pSrc3[10];
	q7_t pResult3;
	
	arm_mean_f32(pSrc, 10, &pResult);
	printf("arm_mean_f32 : pResult = %f\r\n", pResult);
	
	/*****************************************************************/
	for(pIndex = 0; pIndex < 10; pIndex++)
	{
		 pSrc1[pIndex] = rand();
	}
	arm_mean_q31(pSrc1, 10, &pResult1);
	printf("arm_mean_q31 : pResult = %d\r\n", pResult1);
	
	/*****************************************************************/
	for(pIndex = 0; pIndex < 10; pIndex++)
	{
		 pSrc2[pIndex] = rand()%32768;
	}
	arm_mean_q15(pSrc2, 10, &pResult2);
	printf("arm_mean_q15 : pResult = %d\r\n", pResult2);
	
	/*****************************************************************/
	for(pIndex = 0; pIndex < 10; pIndex++)
	{
		 pSrc3[pIndex] = rand()%128;
	}
	arm_mean_q7(pSrc3, 10, &pResult3);
	printf("arm_mean_q7 : pResult = %d\r\n", pResult3);
	printf("******************************************************************\r\n");
}

/*
*********************************************************************************************************
*	�� �� ��: DSP_Power
*	����˵��: ����
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void DSP_Power(void)
{
	float32_t pSrc[10] = {0.6948f, 0.3171f, 0.9502f, 0.0344f, 0.4387f, 0.3816f, 0.7655f, 0.7952f, 0.1869f, 0.4898f};
	float32_t pResult;
	uint32_t pIndex;
	
	q31_t pSrc1[10];
	q63_t pResult1;
	
	q15_t pSrc2[10];
	q63_t pResult2;
	
	q7_t pSrc3[10];
	q31_t pResult3;
	
	arm_power_f32(pSrc, 10, &pResult);
	printf("arm_power_f32 : pResult = %f\r\n", pResult);
	
	/*****************************************************************/
	for(pIndex = 0; pIndex < 10; pIndex++)
	{
		 pSrc1[pIndex] = rand();
	}
	arm_power_q31(pSrc1, 10, &pResult1);
	printf("arm_power_q31 : pResult = %lld\r\n", pResult1);
	
	/*****************************************************************/
	for(pIndex = 0; pIndex < 10; pIndex++)
	{
		 pSrc2[pIndex] = rand()%32768;
	}
	arm_power_q15(pSrc2, 10, &pResult2);
	printf("arm_power_q15 : pResult = %lld\r\n", pResult2);
	
	/*****************************************************************/
	for(pIndex = 0; pIndex < 10; pIndex++)
	{
		 pSrc3[pIndex] = rand()%128;
	}
	arm_power_q7(pSrc3, 10, &pResult3);
	printf("arm_power_q7 : pResult = %d\r\n", pResult3);
	printf("******************************************************************\r\n");
}

/*
*********************************************************************************************************
*	�� �� ��: PrintfHelp
*	����˵��: ��ʾ������ʾ�˵�
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void PrintfHelp(void)
{
	printf("������ʾ:\r\n");
	printf("1. ����һ���Զ���װ�����ʱ����ÿ100ms��תһ��LED2\r\n");
	printf("2. ���°���K1, DSP�����ֵ\r\n");
	printf("3. ���°���K2, DSP����Сֵ\r\n");
	printf("4. ���°���K3, DSP��ƽ��ֵ\r\n");		
	printf("5. ����ҡ��OK��, DSP����\r\n");
	
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
