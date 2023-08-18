/*
*********************************************************************************************************
*
*	ģ������ : ������ģ��
*	�ļ����� : main.c
*	��    �� : V1.0
*	˵    �� : DSP���ܺ��������ݿ������������͸���ת���㣩
*              ʵ��Ŀ�ģ�
*                1. ѧϰDSP���ܺ��������ݿ������������͸���ת���㣩
*              ʵ�����ݣ� 
*                1. ����һ���Զ���װ�����ʱ����ÿ100ms��תһ��LED2��
*                2. ���°���K1, ���ڴ�ӡ����DSP_Copy��������
*                3. ���°���K2, ���ڴ�ӡ����DSP_Fill��������
*                4. ���°���K3, ���ڴ�ӡ����DSP_FloatToFix��������
*              ע�����
*                1. ��ʵ���Ƽ�ʹ�ô������SecureCRT�鿴��ӡ��Ϣ��������115200������λ8����żУ��λ�ޣ�ֹͣλ1��
*                2. ��ؽ��༭��������������TAB����Ϊ4���Ķ����ļ���Ҫ��������ʾ�����롣
*
*	�޸ļ�¼ :
*		�汾��   ����         ����        ˵��
*		V1.0    2019-11-02   Eric2013     1. CMSIS����汾 V5.6.0
*                                         2. HAL��汾 V2.4.0
*
*	Copyright (C), 2018-2030, ���������� www.armfly.com
*
*********************************************************************************************************
*/	
#include "bsp.h"			 /* �ײ�Ӳ������ */
#include "arm_math.h"



/* ���������������̷������� */
#define EXAMPLE_NAME	"V5-DSP���ܺ��������ݿ������������͸���ת���㣩"
#define EXAMPLE_DATE	"2019-12-26"
#define DEMO_VER		"1.0"

/* �������ļ��ڵ��õĺ������� */
static void PrintfLogo(void);
static void PrintfHelp(void);
static void DSP_Copy(void);
static void DSP_Fill(void);
static void DSP_FloatToFix(void);


/*
*********************************************************************************************************
*	�� �� ��: main
*	����˵��: c�������
*	��    �Σ���
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
				case KEY_DOWN_K1:		    /* K1�����£����ݸ��� */
		 		    DSP_Copy();
					break;
					
				case KEY_DOWN_K2:			/* K2�����£�������� */
					DSP_Fill();
					break;

				case KEY_DOWN_K3:			/* K3�����£�����ת���� */
					DSP_FloatToFix();
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
*	�� �� ��: DSP_Copy
*	����˵��: ���ݿ���
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void DSP_Copy(void)
{
	float32_t pSrc[10] = {0.6557,  0.0357,  0.8491,  0.9340, 0.6787,  0.7577,  0.7431,  0.3922,  0.6555,  0.1712};
	float32_t pDst[10];
	uint32_t pIndex;
	
	q31_t pSrc1[10];
	q31_t pDst1[10];
	
	q15_t pSrc2[10];
	q15_t pDst2[10];
	
	q7_t pSrc3[10];
	q7_t pDst3[10];
	
	for(pIndex = 0; pIndex < 10; pIndex++)
	{
		printf("pSrc[%d] = %f\r\n", pIndex, pSrc[pIndex]);
	}
	arm_copy_f32(pSrc, pDst, 10);
	for(pIndex = 0; pIndex < 10; pIndex++)
	{
		printf("arm_copy_f32: pDst[%d] = %f\r\n", pIndex, pDst[pIndex]);
	}

	/*****************************************************************/
	for(pIndex = 0; pIndex < 10; pIndex++)
	{
		pSrc1[pIndex] = rand();
		printf("pSrc1[%d] = %d\r\n", pIndex, pSrc1[pIndex]);
	}
	arm_copy_q31(pSrc1, pDst1, 10);
	for(pIndex = 0; pIndex < 10; pIndex++)
	{
		printf("arm_copy_q31: pDst1[%d] = %d\r\n", pIndex, pDst1[pIndex]);
	}
	/*****************************************************************/
	for(pIndex = 0; pIndex < 10; pIndex++)
	{
		pSrc2[pIndex] = rand()%32768;
		printf("pSrc2[%d] = %d\r\n", pIndex, pSrc2[pIndex]);
	}
	arm_copy_q15(pSrc2, pDst2, 10);
	for(pIndex = 0; pIndex < 10; pIndex++)
	{
		printf("arm_copy_q15: pDst2[%d] = %d\r\n", pIndex, pDst2[pIndex]);
	}
	/*****************************************************************/
	for(pIndex = 0; pIndex < 10; pIndex++)
	{
		pSrc3[pIndex] = rand()%128;
		printf("pSrc3[%d] = %d\r\n", pIndex, pSrc3[pIndex]);
	}
	arm_copy_q7(pSrc3, pDst3, 10);
	for(pIndex = 0; pIndex < 10; pIndex++)
	{
		printf("arm_copy_q7: pDst3[%d] = %d\r\n", pIndex, pDst3[pIndex]);
	}
	/*****************************************************************/
	printf("******************************************************************\r\n");
}

/*
*********************************************************************************************************
*	�� �� ��: DSP_Fill
*	����˵��: �������
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void DSP_Fill(void)
{
	float32_t pDst[10];
	uint32_t pIndex;
	q31_t pDst1[10];
	q15_t pDst2[10];
	q7_t pDst3[10];
	

	arm_fill_f32(3.33f, pDst, 10);
	for(pIndex = 0; pIndex < 10; pIndex++)
	{
		printf("arm_fill_f32: pDst[%d] = %f\r\n", pIndex, pDst[pIndex]);
	}

	/*****************************************************************/
	arm_fill_q31(0x11111111, pDst1, 10);
	for(pIndex = 0; pIndex < 10; pIndex++)
	{
		printf("arm_fill_q31: pDst1[%d] = %x\r\n", pIndex, pDst1[pIndex]);
	}
	/*****************************************************************/
	arm_fill_q15(0x1111, pDst2, 10);
	for(pIndex = 0; pIndex < 10; pIndex++)
	{
		printf("arm_fill_q15: pDst2[%d] = %d\r\n", pIndex, pDst2[pIndex]);
	}
	/*****************************************************************/
	arm_fill_q7(0x11, pDst3, 10);
	for(pIndex = 0; pIndex < 10; pIndex++)
	{
		printf("arm_fill_q7: pDst3[%d] = %d\r\n", pIndex, pDst3[pIndex]);
	}
	/*****************************************************************/
	printf("******************************************************************\r\n");
}

/*
*********************************************************************************************************
*	�� �� ��: DSP_FloatToFix
*	����˵��: ������ת������
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void DSP_FloatToFix(void)
{
	float32_t pSrc[10] = {0.6557,  0.0357,  0.8491,  0.9340, 0.6787,  0.7577,  0.7431,  0.3922,  0.6555,  0.1712};
	uint32_t pIndex;
	q31_t pDst1[10];
	q15_t pDst2[10];
	q7_t pDst3[10];
	
	for(pIndex = 0; pIndex < 10; pIndex++)
	{
		printf("pSrc[%d] = %f\r\n", pIndex, pSrc[pIndex]);
	}
	
	/*****************************************************************/
	arm_float_to_q31(pSrc, pDst1, 10);
	for(pIndex = 0; pIndex < 10; pIndex++)
	{
		printf("arm_float_to_q31: pDst[%d] = %d\r\n", pIndex, pDst1[pIndex]);
	}
	
	/*****************************************************************/
	arm_float_to_q15(pSrc, pDst2, 10);
	for(pIndex = 0; pIndex < 10; pIndex++)
	{
		printf("arm_float_to_q15: pDst1[%d] = %d\r\n", pIndex, pDst2[pIndex]);
	}
	
	/*****************************************************************/
	arm_float_to_q7(pSrc, pDst3, 10);
	for(pIndex = 0; pIndex < 10; pIndex++)
	{
		printf("arm_float_to_q7: pDst2[%d] = %d\r\n", pIndex, pDst3[pIndex]);
	}
	/*****************************************************************/
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
	printf("2. ���°���K1, ���ݿ���\r\n");
	printf("3. ���°���K2, �������\r\n");
	printf("4. ���°���K3, ����ת����\r\n");
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
		
		CPU_Sn0 = *(__IO uint32_t*)(0x1FFF7A10);
		CPU_Sn1 = *(__IO uint32_t*)(0x1FFF7A10 + 4);
		CPU_Sn2 = *(__IO uint32_t*)(0x1FFF7A10 + 8);

		printf("\r\nCPU : STM32F407IGT6, LQFP176, ��Ƶ: %dMHz\r\n", SystemCoreClock / 1000000);
		printf("UID = %08X %08X %08X\n\r", CPU_Sn2, CPU_Sn1, CPU_Sn0);
	}

	printf("\n\r");
	printf("*************************************************************\n\r");
	printf("* ��������   : %s\r\n", EXAMPLE_NAME);	/* ��ӡ�������� */
	printf("* ���̰汾   : %s\r\n", DEMO_VER);		/* ��ӡ���̰汾 */
	printf("* ��������   : %s\r\n", EXAMPLE_DATE);	/* ��ӡ�������� */

	/* ��ӡST��HAL��汾 */
	printf("* HAL��汾  : V2.4.0 (STM32F407 HAL Driver)\r\n");
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
