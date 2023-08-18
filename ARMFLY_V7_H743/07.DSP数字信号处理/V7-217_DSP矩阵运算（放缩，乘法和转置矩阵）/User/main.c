/*
*********************************************************************************************************
*
*	ģ������ : ������ģ��
*	�ļ����� : main.c
*	��    �� : V1.0
*	˵    �� : DSP��������(�������˷���ת��)
*              ʵ��Ŀ�ģ�
*                1. ѧϰDSP��������(�������˷���ת��)
*              ʵ�����ݣ� 
*                1. ����һ���Զ���װ�����ʱ����ÿ100ms��תһ��LED2��
*                2.	���°���K1�����ڴ���DSP_MatScale��������ݡ�
*				 3.	���°���K2�����ڴ���DSP_MatMult��������ݡ�
*                4.	���°���K3�����ڴ���DSP_MatTrans��������ݡ�
*              ע�����
*                1. ��ʵ���Ƽ�ʹ�ô������SecureCRT�鿴��ӡ��Ϣ��������115200������λ8����żУ��λ�ޣ�ֹͣλ1��
*                2. ��ؽ��༭��������������TAB����Ϊ4���Ķ����ļ���Ҫ��������ʾ�����롣
*
*	�޸ļ�¼ :
*		�汾��   ����         ����        ˵��
*		V1.0    2020-03-29   Eric2013     1. CMSIS����汾 V5.6.0
*                                         2. HAL��汾 V1.3.0
*
*	Copyright (C), 2020-2030, ���������� www.armfly.com
*
*********************************************************************************************************
*/	
#include "bsp.h"			 /* �ײ�Ӳ������ */
#include "arm_math.h"



/* ���������������̷������� */
#define EXAMPLE_NAME	"V7-DSP��������(�������˷���ת�þ���)"
#define EXAMPLE_DATE	"2020-03-29"
#define DEMO_VER		"1.0"


/* �������ļ��ڵ��õĺ������� */
static void PrintfLogo(void);
static void PrintfHelp(void);
static void DSP_MatScale(void);
static void DSP_MatMult(void);
static void DSP_MatTrans(void);


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
				case KEY_DOWN_K1:		    /* ���°���K1�����ڴ���DSP_MatScale��������� */
				    DSP_MatScale();
					break;
					
				case KEY_DOWN_K2:			/* ���°���K2�����ڴ���DSP_MatMult��������� */
					DSP_MatMult();
					break;

				case KEY_DOWN_K3:			/* ���°���K3�����ڴ���DSP_MatTrans��������� */
					DSP_MatTrans();
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
*	�� �� ��: DSP_MatScale
*	����˵��: �������
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void DSP_MatScale(void)
{
	uint8_t i;
	
	/****����������******************************************************************/
	float32_t pDataA[9] = {1.1f, 1.1f, 2.1f, 2.1f, 3.1f, 3.1f, 4.1f, 4.1f, 5.1f};
	float32_t scale = 1.1f;
	float32_t pDataDst[9];
	
	arm_matrix_instance_f32 pSrcA; //3��3������
	arm_matrix_instance_f32 pDst;
	
	/****������Q31����******************************************************************/
	q31_t pDataA1[9] = {1, 1, 2, 2, 3, 3, 4, 4, 5};
	q31_t scaleFract = 10;
    int32_t shift = 0;
	q31_t pDataDst1[9];
	
	arm_matrix_instance_q31 pSrcA1; //3��3������
	arm_matrix_instance_q31 pDst1;
	
	/****������Q15����******************************************************************/
	q15_t pDataA2[9] = {1, 1, 2, 2, 3, 3, 4, 4, 5};
	q15_t scaleFract1 = 10;
    int32_t shift1 = 0;
	q15_t pDataDst2[9];
	
	arm_matrix_instance_q15 pSrcA2; //3��3������
	arm_matrix_instance_q15 pDst2;
	
	/****������***********************************************************************/
	pSrcA.numCols = 3;
	pSrcA.numRows = 3;
	pSrcA.pData = pDataA;

	pDst.numCols = 3;
	pDst.numRows = 3;
	pDst.pData = pDataDst;
	
	printf("****������******************************************\r\n");
	arm_mat_scale_f32(&pSrcA, scale, &pDst);
	for(i = 0; i < 9; i++)
	{
		printf("pDataDst[%d] = %f\r\n", i, pDataDst[i]);
	}
	
	/****������Q31***********************************************************************/
	pSrcA1.numCols = 3;
	pSrcA1.numRows = 3;
	pSrcA1.pData = pDataA1;
	
	pDst1.numCols = 3;
	pDst1.numRows = 3;
	pDst1.pData = pDataDst1;
	
	printf("****������Q31******************************************\r\n");
	arm_mat_scale_q31(&pSrcA1, scaleFract, shift, &pDst1);
	for(i = 0; i < 9; i++)
	{
		printf("pDataDst1[%d] = %d\r\n", i, pDataDst1[i]);
	}
	
	/****������Q15***********************************************************************/
	pSrcA2.numCols = 3;
	pSrcA2.numRows = 3;
	pSrcA2.pData = pDataA2;
	
	pDst2.numCols = 3;
	pDst2.numRows = 3;
	pDst2.pData = pDataDst2;
	
	printf("****������Q15******************************************\r\n");
	arm_mat_scale_q15(&pSrcA2, scaleFract1, shift1, &pDst2);
	for(i = 0; i < 9; i++)
	{
		printf("pDataDst2[%d] = %d\r\n", i, pDataDst2[i]);
	}
}

/*
*********************************************************************************************************
*	�� �� ��: DSP_MatMult
*	����˵��: ����˷�
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void DSP_MatMult(void)
{
	uint8_t i;
	
	/****����������******************************************************************/
	float32_t pDataA[9] = {1.1f, 1.1f, 2.1f, 2.1f, 3.1f, 3.1f, 4.1f, 4.1f, 5.1f};
	float32_t pDataB[9] = {1.1f, 1.1f, 2.1f, 2.1f, 3.1f, 3.1f, 4.1f, 4.1f, 5.1f};
	float32_t pDataDst[9];
	
	arm_matrix_instance_f32 pSrcA; //3��3������
	arm_matrix_instance_f32 pSrcB; //3��3������
	arm_matrix_instance_f32 pDst;
	
	/****������Q31����******************************************************************/
	q31_t pDataA1[9] = {1, 1, 2, 2, 3, 3, 4, 4, 5};
	q31_t pDataB1[9] = {1, 1, 2, 2, 3, 3, 4, 4, 5};
	q31_t pDataDst1[9];
	
	arm_matrix_instance_q31 pSrcA1; //3��3������
	arm_matrix_instance_q31 pSrcB1; //3��3������
	arm_matrix_instance_q31 pDst1;
	
	/****������Q15����******************************************************************/
	q15_t pDataA2[9] = {1, 1, 2, 2, 3, 3, 4, 4, 5};
	q15_t pDataB2[9] = {1, 1, 2, 2, 3, 3, 4, 4, 5};
	q15_t pDataDst2[9];
	
	arm_matrix_instance_q15 pSrcA2; //3��3������
	arm_matrix_instance_q15 pSrcB2; //3��3������
	arm_matrix_instance_q15 pDst2;
	q15_t pState[9];
	
	/****������***********************************************************************/
	pSrcA.numCols = 3;
	pSrcA.numRows = 3;
	pSrcA.pData = pDataA;
	
	pSrcB.numCols = 3;
	pSrcB.numRows = 3;
	pSrcB.pData = pDataB;
	
	pDst.numCols = 3;
	pDst.numRows = 3;
	pDst.pData = pDataDst;
	
	printf("****������******************************************\r\n");
	arm_mat_mult_f32(&pSrcA, &pSrcB, &pDst);
	for(i = 0; i < 9; i++)
	{
		printf("pDataDst[%d] = %f\r\n", i, pDataDst[i]);
	}
	
	/****������Q31***********************************************************************/
	pSrcA1.numCols = 3;
	pSrcA1.numRows = 3;
	pSrcA1.pData = pDataA1;
	
	pSrcB1.numCols = 3;
	pSrcB1.numRows = 3;
	pSrcB1.pData = pDataB1;
	
	pDst1.numCols = 3;
	pDst1.numRows = 3;
	pDst1.pData = pDataDst1;
	
	printf("****������Q31******************************************\r\n");
	arm_mat_mult_q31(&pSrcA1, &pSrcB1, &pDst1);
	arm_mat_mult_fast_q31(&pSrcA1, &pSrcB1, &pDst1);
	for(i = 0; i < 9; i++)
	{
		printf("pDataDst1[%d] = %d\r\n", i, pDataDst1[i]);
	}
	
	/****������Q15***********************************************************************/
	pSrcA2.numCols = 3;
	pSrcA2.numRows = 3;
	pSrcA2.pData = pDataA2;
	
	pSrcB2.numCols = 3;
	pSrcB2.numRows = 3;
	pSrcB2.pData = pDataB2;
	
	pDst2.numCols = 3;
	pDst2.numRows = 3;
	pDst2.pData = pDataDst2;
	
	printf("****������Q15******************************************\r\n");
	
	arm_mat_mult_q15(&pSrcA2, &pSrcB2, &pDst2, pState);
	arm_mat_mult_fast_q15(&pSrcA2, &pSrcB2, &pDst2, pState);
	for(i = 0; i < 9; i++)
	{
		printf("pDataDst2[%d] = %d\r\n", i, pDataDst2[i]);
	}
}

/*
*********************************************************************************************************
*	�� �� ��: DSP_MatTrans
*	����˵��: �������ݳ�ʼ��
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void DSP_MatTrans(void)
{
	uint8_t i;

	/****����������******************************************************************/
	float32_t pDataA[9] = {1.1f, 1.1f, 2.1f, 2.1f, 3.1f, 3.1f, 4.1f, 4.1f, 5.1f};
	float32_t pDataDst[9];
	
	arm_matrix_instance_f32 pSrcA; //3��3������
	arm_matrix_instance_f32 pDst;
	
	/****������Q31����******************************************************************/
	q31_t pDataA1[9] = {1, 1, 2, 2, 3, 3, 4, 4, 5};
	q31_t pDataDst1[9];
	
	arm_matrix_instance_q31 pSrcA1; //3��3������
	arm_matrix_instance_q31 pDst1;
	
	/****������Q15����******************************************************************/
	q15_t pDataA2[9] = {1, 1, 2, 2, 3, 3, 4, 4, 5};
	q15_t pDataDst2[9];
	
	arm_matrix_instance_q15 pSrcA2; //3��3������
	arm_matrix_instance_q15 pDst2;
	
	/****������***********************************************************************/
	pSrcA.numCols = 3;
	pSrcA.numRows = 3;
	pSrcA.pData = pDataA;

	pDst.numCols = 3;
	pDst.numRows = 3;
	pDst.pData = pDataDst;
	
	printf("****������******************************************\r\n");
	arm_mat_trans_f32(&pSrcA, &pDst);
	for(i = 0; i < 9; i++)
	{
		printf("pDataDst[%d] = %f\r\n", i, pDataDst[i]);
	}
	
	/****������Q31***********************************************************************/
	pSrcA1.numCols = 3;
	pSrcA1.numRows = 3;
	pSrcA1.pData = pDataA1;
	
	pDst1.numCols = 3;
	pDst1.numRows = 3;
	pDst1.pData = pDataDst1;
	
	printf("****������Q31******************************************\r\n");
	arm_mat_trans_q31(&pSrcA1, &pDst1);
	for(i = 0; i < 9; i++)
	{
		printf("pDataDst1[%d] = %d\r\n", i, pDataDst1[i]);
	}
	
	/****������Q15***********************************************************************/
	pSrcA2.numCols = 3;
	pSrcA2.numRows = 3;
	pSrcA2.pData = pDataA2;
	
	pDst2.numCols = 3;
	pDst2.numRows = 3;
	pDst2.pData = pDataDst2;
	
	printf("****������Q15******************************************\r\n");
	arm_mat_trans_q15(&pSrcA2, &pDst2);
	for(i = 0; i < 9; i++)
	{
		printf("pDataDst2[%d] = %d\r\n", i, pDataDst2[i]);
	}
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
	printf("2. ���°���K1�����ڴ���DSP_MatScale���������\r\n");
	printf("3. ���°���K2�����ڴ���DSP_MatMult���������\r\n");
	printf("4. ���°���K3�����ڴ���DSP_MatTrans���������\r\n");
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
