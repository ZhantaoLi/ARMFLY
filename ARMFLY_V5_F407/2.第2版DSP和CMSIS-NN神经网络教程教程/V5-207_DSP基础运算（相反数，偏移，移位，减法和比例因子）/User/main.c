/*
*********************************************************************************************************
*
*	ģ������ : ������ģ��
*	�ļ����� : main.c
*	��    �� : V1.0
*	˵    �� : DSP�������㣨�෴����ƫ�ƣ���λ�������ͱ������ӣ�
*              ʵ��Ŀ�ģ�
*                1. ѧϰ�������㣨�෴����ƫ�ƣ���λ�������ͱ������ӣ�
*              ʵ�����ݣ� 
*                1. ����һ���Զ���װ�����ʱ����ÿ100ms��תһ��LED2��
*                2. ���°���K1, DSP���෴�����㡣
*                3. ���°���K2, DSP��ƫ�����㡣
*                4. ���°���K3, DSP����λ���㡣
*                5. ����ҡ��OK��, DSP��������㡣
*                6. ����ҡ���ϼ�, DSP�����������㡣		
*              ע�����
*                1. ��ʵ���Ƽ�ʹ�ô������SecureCRT�鿴��ӡ��Ϣ��������115200������λ8����żУ��λ�ޣ�ֹͣλ1��
*                2. ��ؽ��༭��������������TAB����Ϊ4���Ķ����ļ���Ҫ��������ʾ�����롣
*
*	�޸ļ�¼ :
*		�汾��   ����         ����        ˵��
*		V1.0    2019-10-05   Eric2013     1. CMSIS����汾 V5.6.0
*                                         2. HAL��汾 V2.4.0
*
*	Copyright (C), 2018-2030, ���������� www.armfly.com
*
*********************************************************************************************************
*/	
#include "bsp.h"			 /* �ײ�Ӳ������ */
#include "arm_math.h"



/* ���������������̷������� */
#define EXAMPLE_NAME	"V5-�������㣨�෴����ƫ�ƣ���λ�������ͱ������ӣ�"
#define EXAMPLE_DATE	"2019-10-05"
#define DEMO_VER		"1.0"

/* �������ļ��ڵ��õĺ������� */
static void PrintfLogo(void);
static void PrintfHelp(void);
static void DSP_Negate(void);
static void DSP_Offset(void);
static void DSP_Shift(void);
static void DSP_Sub(void);
static void DSP_Scale(void);


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
				case KEY_DOWN_K1:			/* K1�����£����෴�� */
					DSP_Negate();
					break;

				case KEY_DOWN_K2:			/* K2������, ��ƫ�� */
					DSP_Offset();
					break;

				case KEY_DOWN_K3:			/* K3�����£�����λ */
					DSP_Shift();
					break;
	
				case JOY_DOWN_OK:	        /* ҡ��OK�����£������ */
					DSP_Sub();
					break;
				
				case JOY_DOWN_U:	        /* ҡ���ϼ����£���������Ӽ��� */
					DSP_Scale();
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
*	�� �� ��: DSP_Negate
*	����˵��: ���෴��
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void DSP_Negate(void)
{
	float32_t pSrc = 0.0f;
    float32_t pDst;
	
	q31_t pSrc1 = 0;
	q31_t pDst1;
	
	q15_t pSrc2 = 0;
	q15_t pDst2;
	
	q7_t pSrc3 = 0; 
	q7_t pDst3;
	
	/*���෴��*********************************/	
	pSrc -= 1.23f;
	arm_negate_f32(&pSrc, &pDst, 1);
	printf("arm_negate_f32 = %f\r\n", pDst);

	pSrc1 -= 1;
	arm_negate_q31(&pSrc1, &pDst1, 1);
	printf("arm_negate_q31 = %d\r\n", pDst1);

	pSrc2 -= 1;
	arm_negate_q15(&pSrc2, &pDst2, 1);
	printf("arm_negate_q15 = %d\r\n", pDst2);

	pSrc3 += 1; 
	arm_negate_q7(&pSrc3, &pDst3, 1);
	printf("arm_negate_q7 = %d\r\n", pDst3);
	printf("***********************************\r\n");
}

/*
*********************************************************************************************************
*	�� �� ��: DSP_Offset
*	����˵��: ƫ��
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void DSP_Offset(void)
{
	float32_t   pSrcA = 0.0f;
	float32_t   Offset = 0.0f;  
	float32_t   pDst;  
	
	q31_t  pSrcA1 = 0;  
	q31_t  Offset1 = 0;  
	q31_t  pDst1;  

	q15_t  pSrcA2 = 0;  
	q15_t  Offset2 = 0;  
	q15_t  pDst2; 

	q7_t  pSrcA3 = 0; 
	q7_t  Offset3 = 0;  
	q7_t  pDst3;  

	/*��ƫ��*********************************/		
	Offset--;
	arm_offset_f32(&pSrcA, Offset, &pDst, 1);
	printf("arm_offset_f32 = %f\r\n", pDst);

	Offset1--;
	arm_offset_q31(&pSrcA1, Offset1, &pDst1, 1);
	printf("arm_offset_q31 = %d\r\n", pDst1);

	Offset2--;
	arm_offset_q15(&pSrcA2, Offset2, &pDst2, 1);
	printf("arm_offset_q15 = %d\r\n", pDst2);

	Offset3--;
	arm_offset_q7(&pSrcA3, Offset3, &pDst3, 1);
	printf("arm_offset_q7 = %d\r\n", pDst3);
	printf("***********************************\r\n");
}

/*
*********************************************************************************************************
*	�� �� ��: DSP_Shift
*	����˵��: ��λ
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void DSP_Shift(void)
{
	q31_t  pSrcA1 = 0x88886666;  
	q31_t  pDst1;  

	q15_t  pSrcA2 = 0x8866;  
	q15_t  pDst2; 

	q7_t  pSrcA3 = 0x86; 
	q7_t  pDst3;  


	/*����λ*********************************/	
	arm_shift_q31(&pSrcA1, 3, &pDst1, 1);
	printf("arm_shift_q31 = %8x\r\n", pDst1);

	arm_shift_q15(&pSrcA2, -3, &pDst2, 1);
	printf("arm_shift_q15 = %4x\r\n", pDst2);

	arm_shift_q7(&pSrcA3, 3, &pDst3, 1);
	printf("arm_shift_q7 = %2x\r\n", pDst3);
	printf("***********************************\r\n");
}

/*
*********************************************************************************************************
*	�� �� ��: DSP_Sub
*	����˵��: ����
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void DSP_Sub(void)
{
	float32_t   pSrcA[5] = {1.0f,1.0f,1.0f,1.0f,1.0f};
	float32_t   pSrcB[5] = {1.0f,1.0f,1.0f,1.0f,1.0f};  
	float32_t   pDst[5];  
	
	q31_t  pSrcA1[5] = {1,1,1,1,1};  
	q31_t  pSrcB1[5] = {1,1,1,1,1};  
	q31_t  pDst1[5];   

	q15_t  pSrcA2[5] = {1,1,1,1,1};  
	q15_t  pSrcB2[5] = {1,1,1,1,1};  
	q15_t  pDst2[5];   

	q7_t  pSrcA3[5] = {0x70,1,1,1,1}; 
	q7_t  pSrcB3[5] = {0x7f,1,1,1,1};  
	q7_t  pDst3[5];  


	/*�����*********************************/	
	pSrcA[0] += 1.1f;
	arm_sub_f32(pSrcA, pSrcB, pDst, 5);
	printf("arm_sub_f32 = %f\r\n", pDst[0]);
	
	pSrcA1[0] += 1;
	arm_sub_q31(pSrcA1, pSrcB1, pDst1, 5);
	printf("arm_sub_q31 = %d\r\n", pDst1[0]);

	pSrcA2[0] += 1;
	arm_sub_q15(pSrcA2, pSrcB2, pDst2, 5);
	printf("arm_sub_q15 = %d\r\n", pDst2[0]);

	pSrcA3[0] += 1;
	arm_sub_q7(pSrcA3, pSrcB3, pDst3, 5);
	printf("arm_sub_q7 = %d\r\n", pDst3[0]);
	printf("***********************************\r\n");
}

/*
*********************************************************************************************************
*	�� �� ��: DSP_Scale
*	����˵��: ��������
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void DSP_Scale(void)
{
	float32_t   pSrcA[5] = {1.0f,1.0f,1.0f,1.0f,1.0f};
	float32_t   scale = 0.0f;  
	float32_t   pDst[5];  
	
	q31_t  pSrcA1[5] = {0x6fffffff,1,1,1,1};  
	q31_t  scale1 = 0x6fffffff;  
	q31_t  pDst1[5];   

	q15_t  pSrcA2[5] = {0x6fff,1,1,1,1};  
	q15_t  scale2 = 0x6fff;  
	q15_t  pDst2[5];   

	q7_t  pSrcA3[5] = {0x70,1,1,1,1}; 
	q7_t  scale3 = 0x6f;  
	q7_t pDst3[5];  

	/*��������Ӽ���*********************************/	
	scale += 0.1f;
	arm_scale_f32(pSrcA, scale, pDst, 5);
	printf("arm_scale_f32 = %f\r\n", pDst[0]);
	
	scale1 += 1;
	arm_scale_q31(pSrcA1, scale1, 0, pDst1, 5);
	printf("arm_scale_q31 = %x\r\n", pDst1[0]);

	scale2 += 1;
	arm_scale_q15(pSrcA2, scale2, 0, pDst2, 5);
	printf("arm_scale_q15 = %x\r\n", pDst2[0]);

	scale3 += 1;
	arm_scale_q7(pSrcA3, scale3, 0, pDst3, 5);
	printf("arm_scale_q7 = %x\r\n", pDst3[0]);
	printf("***********************************\r\n");
}

/*
*********************************************************************************************************
*	�� �� ��: PrintfHelp
*	����˵��: ��ʾ������ʾ�˵�
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void PrintfHelp(void)
{
	printf("������ʾ:\r\n");
	printf("1. ����һ���Զ���װ�����ʱ����ÿ100ms��תһ��LED2\r\n");
	printf("2. ���°���K1, DSP���෴������\r\n");
	printf("3. ���°���K2, DSP��ƫ������\r\n");
	printf("4. ���°���K3, DSP����λ����\r\n");
	printf("5. ����ҡ��OK��, DSP���������\r\n");
	printf("6. ����ҡ���ϼ�, DSP������������\r\n");		
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
