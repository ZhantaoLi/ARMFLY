/*
*********************************************************************************************************
*
*	ģ������ : ������ģ��
*	�ļ����� : main.c
*	��    �� : V1.0
*	˵    �� : �����岹ʵ��
*              ʵ��Ŀ�ģ�
*                1.	ѧϰ�����岹ʵ��
*              ʵ�����ݣ� 
*                1. ����һ���Զ���װ�����ʱ����ÿ100ms��תһ��LED2��
*                2.	K1�����£���Ȼ�����岹���ԡ�
*                3.	K2�����£����������岹���ԡ�
*              ע�����
*                1. ��ӡ��ʽ��
*                   ��ʽһ��
*                     Ĭ��ʹ�ô��ڴ�ӡ������ʹ��SecureCRT����H7-TOOL��λ���������ֲ鿴��ӡ��Ϣ��
*                     ������115200������λ8����żУ��λ�ޣ�ֹͣλ1��
*                   ��ʽ����
*                     ʹ��RTT��ӡ������ʹ��SEGGE RTT����H7-TOOL RTT��ӡ��
*                     MDK AC5��MDK AC6��IARͨ��ʹ��bsp.h�ļ��еĺ궨��Ϊ1����
*                     #define Enable_RTTViewer  1
*                2. ��ؽ��༭��������������TAB����Ϊ4���Ķ����ļ���Ҫ��������ʾ�����롣
*
*	�޸ļ�¼ :
*		�汾��   ����         ����        ˵��
*		V1.0    2021-10-31   Eric2013     1. CMSIS����汾 V5.8.0
*                                         2. HAL��汾 V1.10.0
*
*	Copyright (C), 2020-2030, ���������� www.armfly.com
*
*********************************************************************************************************
*/	
#include "bsp.h"			 /* �ײ�Ӳ������ */
#include "arm_math.h"



/* ���������������̷������� */
#define EXAMPLE_NAME	"V7-�����岹ʵ�֣��������˿��˳��"
#define EXAMPLE_DATE	"2021-10-31"
#define DEMO_VER		"1.0"

static void PrintfLogo(void);
static void PrintfHelp(void);


#define INPUT_TEST_LENGTH_SAMPLES 	128    /* �������ݸ��� */
#define OUT_TEST_LENGTH_SAMPLES 	1024   /* ������ݸ��� */

#define SpineTab (OUT_TEST_LENGTH_SAMPLES/INPUT_TEST_LENGTH_SAMPLES ) /* �岹ĩβ��8������ֵ��ʹ�� */


float32_t xn[INPUT_TEST_LENGTH_SAMPLES];   /* ��������x������ */
float32_t yn[INPUT_TEST_LENGTH_SAMPLES];   /* ��������y������ */

float32_t coeffs[3*(INPUT_TEST_LENGTH_SAMPLES - 1)];     /* �岹ϵ������ */  
float32_t tempBuffer[2 * INPUT_TEST_LENGTH_SAMPLES - 1]; /* �岹��ʱ���� */  

float32_t xnpos[OUT_TEST_LENGTH_SAMPLES];  /* �岹�����X������ֵ */
float32_t ynpos[OUT_TEST_LENGTH_SAMPLES];  /* �岹�����Y����ֵ */

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
	uint32_t i;
	uint32_t idx2;
	uint8_t ucKeyCode;	
	arm_spline_instance_f32 S;
	

	bsp_Init();		/* Ӳ����ʼ�� */
	PrintfLogo();	/* ��ӡ�������ƺͰ汾����Ϣ */
	PrintfHelp();	/* ��ӡ������ʾ */
	
	bsp_StartAutoTimer(0, 100);	/* ����1��100ms���Զ���װ�Ķ�ʱ�� */
	
	
	/* ԭʼx����ֵ��y����ֵ */
	for(i=0; i<INPUT_TEST_LENGTH_SAMPLES; i++)
	{
		xn[i] = i*SpineTab;
		yn[i] = 1 + cos(2*3.1415926*50*i/256 + 3.1415926/3);
	}
	
	/* �岹��X������ֵ���������Ҫ�û����õ� */
	for(i=0; i<OUT_TEST_LENGTH_SAMPLES; i++)
	{
		xnpos[i] = i;
	}
	
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
				case KEY_DOWN_K1:  /* K1�����£���Ȼ�����岹 */
					/* ������ʼ�� */
					arm_spline_init_f32(&S,
										ARM_SPLINE_NATURAL ,
										xn,
										yn,
										INPUT_TEST_LENGTH_SAMPLES,
										coeffs,
										tempBuffer);
					/* �������� */
					arm_spline_f32	(&S,
									 xnpos,
									 ynpos,
									 OUT_TEST_LENGTH_SAMPLES);

				
					/* ��ӡ������ */
					idx2 = 0;
					for (i = 0; i < OUT_TEST_LENGTH_SAMPLES-SpineTab; i++)
					{	
							if ((i % SpineTab) == 0)
							{
									printf("%f,%f\r\n", ynpos[i], yn[idx2++]);
							}
							else
							{
									printf("%f,\r\n", ynpos[i]);
							}
					}
					break;

				case KEY_DOWN_K2:			/* K2�����£������������岹 */
					/* ������ʼ�� */
					arm_spline_init_f32(&S,
										ARM_SPLINE_PARABOLIC_RUNOUT , 
										xn,
										yn,
										INPUT_TEST_LENGTH_SAMPLES,
										coeffs,
										tempBuffer);
					/* �������� */
					arm_spline_f32	(&S,
									 xnpos,
									 ynpos,
									 OUT_TEST_LENGTH_SAMPLES);

				
					/* ��ӡ������ */
					idx2 = 0;
					for (i = 0; i < OUT_TEST_LENGTH_SAMPLES-SpineTab; i++)
					{	
							if ((i % SpineTab) == 0)
							{
									printf("%f,%f\r\n", ynpos[i], yn[idx2++]);
							}
							else
							{
									printf("%f,\r\n", ynpos[i]);
							}
					}
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
	printf("1. �ϵ�������һ�������ʱ����ÿ100ms��תһ��LED2\r\n");
	printf("2. K1�����£���Ȼ�����岹����\r\n");
	printf("3. K2�����£����������岹����\r\n");
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
	printf("* HAL��汾  : V1.10.0 (STM32H7xx HAL Driver)\r\n");
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
