/*
*********************************************************************************************************
*
*	ģ������ : ������ģ��
*	�ļ����� : main.c
*	��    �� : V1.0
*	˵    �� : ��ֲʵ������FTT��任��֧�ֵ����Ⱥ�˫���ȣ�
*              ʵ��Ŀ�ģ�
*                1. ѧϰʵ������FFT��任��֧�ֵ����ȸ����˫���ȸ���
*              ʵ�����ݣ� 
*                1. ����һ���Զ���װ�����ʱ����ÿ100ms��תһ��LED2��
*                2.	���°���K1�����ڴ�ӡ1024��ʵ��������FFT��任��
*                3.	���°���K2�����ڴ�ӡ1024��ʵ��˫����FFT��任��
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
*		V1.0    2021-06-04   Eric2013     1. CMSIS����汾 V5.7.0
*                                         2. HAL��汾 V1.10.0
*
*	Copyright (C), 2021-2030, ���������� www.armfly.com
*
*********************************************************************************************************
*/	
#include "bsp.h"			 	/* �ײ�Ӳ������ */
#include "arm_math.h"
#include "arm_const_structs.h"


/* ���������������̷������� */
#define EXAMPLE_NAME	"V7-ʵ������FTT��任��֧�ֵ����Ⱥ�˫���ȣ�"
#define EXAMPLE_DATE	"2021-06-04"
#define DEMO_VER		"1.0"


/* �������ļ��ڵ��õĺ������� */
static void PrintfLogo(void);
static void PrintfHelp(void);
static void arm_rfft_f32_app(void);
static void arm_rfft_f64_app(void);


/* ���� */
uint32_t ifftFlag = 0; 
uint32_t fftSize = 0;

/* ������������ */
#define TEST_LENGTH_SAMPLES 1024 

static float32_t testOutput_f32[TEST_LENGTH_SAMPLES*2]; 
static float32_t testOutputIn_f32[TEST_LENGTH_SAMPLES*2]; 
static float32_t testInput_f32[TEST_LENGTH_SAMPLES*2];

static float64_t testOutput_f64[TEST_LENGTH_SAMPLES*2]; 
static float64_t testOutputIn_f64[TEST_LENGTH_SAMPLES*2]; 
static float64_t testInput_f64[TEST_LENGTH_SAMPLES*2];


/*
*********************************************************************************************************
*	�� �� ��: arm_rfft_f32_app
*	����˵��: ���ú���arm_rfft_fast_f32����FFT��任�����任
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void arm_rfft_f32_app(void)
{
	uint16_t i;
	arm_rfft_fast_instance_f32 S;
	
	
	/* ���任 */
    ifftFlag = 0; 
	
	/* ��ʼ���ṹ��S�еĲ��� */
 	arm_rfft_fast_init_f32(&S, TEST_LENGTH_SAMPLES);
	
	for(i=0; i<1024; i++)
	{
		/* ��������ֱ��������50Hz���Ҳ���ɣ����β�����1024����ʼ��λ60�� */
		testInput_f32[i] = 1 + cos(2*3.1415926f*50*i/1024 + 3.1415926f/3);
		testOutputIn_f32[i] = testInput_f32[i];
	}
	
	/* 1024��ʵ���п���FFT, testInput_f32���������ݣ�testOutput_f32����� */ 
	arm_rfft_fast_f32(&S, testInput_f32, testOutput_f32, ifftFlag);
	
	/* ��任 */
    ifftFlag = 1; 
	
	/* 1024��ʵ���п���FFT��任��testOutput_f32���������ݣ�testInput_f32��������� */ 
	arm_rfft_fast_f32(&S, testOutput_f32, testInput_f32, ifftFlag);

	printf("=========================================\r\n");	
	
	/* ���ڴ�ӡ��testOutputIn_f32ԭʼ�źţ�testInput_f32��任����ź� */
	for(i=0; i<TEST_LENGTH_SAMPLES; i++)
	{
		printf("%f, %f\r\n", testOutputIn_f32[i], testInput_f32[i]);
	}
}

/*
*********************************************************************************************************
*	�� �� ��: arm_rfft_f64_app
*	����˵��: ���ú���arm_rfft_fast_f64����FFT��任�����任
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void arm_rfft_f64_app(void)
{
	uint16_t i;
	arm_rfft_fast_instance_f64 S;
	
	
	/* ���任 */
    ifftFlag = 0; 
	
	/* ��ʼ���ṹ��S�еĲ��� */
 	arm_rfft_fast_init_f64(&S, TEST_LENGTH_SAMPLES);
	
	for(i=0; i<1024; i++)
	{
		/* ��������ֱ��������50Hz���Ҳ���ɣ����β�����1024����ʼ��λ60�� */
		testInput_f64[i] = 1 + cos(2*3.1415926*50*i/1024 + 3.1415926/3);
		testOutputIn_f64[i] = testInput_f64[i];
	}
	
	/* 1024��ʵ���п���FFT, testInput_f64���������ݣ�testOutput_f64����� */ 
	arm_rfft_fast_f64(&S, testInput_f64, testOutput_f64, ifftFlag);
	
	/* ��任 */
    ifftFlag = 1; 
	
	/* 1024��ʵ���п���FFT��任��testOutput_f64���������ݣ�testInput_f64��������� */ 
	arm_rfft_fast_f64(&S, testOutput_f64, testInput_f64, ifftFlag);
	
	printf("=========================================\r\n");	
	
	/* ���ڴ�ӡ��testOutputIn_f32ԭʼ�źţ�testInput_f32��任����ź� */
	for(i=0; i<TEST_LENGTH_SAMPLES; i++)
	{
		printf("%.11f, %.11f\r\n", testOutputIn_f64[i], testInput_f64[i]);
	}	
			
}

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
		

		if (bsp_CheckTimer(0))	/* �ж϶�ʱ����ʱʱ�� */
		{
			/* ÿ��100ms ����һ�� */
			bsp_LedToggle(4);	/* ��תLED2��״̬ */   
		}
		
		ucKeyCode = bsp_GetKey();	/* ��ȡ��ֵ, �޼�����ʱ���� KEY_NONE = 0 */
		if (ucKeyCode != KEY_NONE)
		{
			switch (ucKeyCode)
			{
				case KEY_DOWN_K1:		    /* K1������ */
					arm_rfft_f32_app();
					break;
				
				case KEY_DOWN_K2:		    /* K2������ */
					arm_rfft_f64_app();
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
*	����˵��: ��ʾ������ʾ�˵�
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void PrintfHelp(void)
{
	printf("1. ����һ���Զ���װ�����ʱ����ÿ100ms��תһ��LED2\r\n");
	printf("2. ���°���K1�����ڴ�ӡ1024��ʵ��������FFT����任\r\n");
	printf("3. ���°���K2�����ڴ�ӡ1024��ʵ��˫����FFT����任\r\n");
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
