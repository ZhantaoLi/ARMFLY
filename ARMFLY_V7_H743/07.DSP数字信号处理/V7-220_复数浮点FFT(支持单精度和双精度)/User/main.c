/*
*********************************************************************************************************
*
*	ģ������ : ������ģ��
*	�ļ����� : main.c
*	��    �� : V1.0
*	˵    �� : ��ֲ��������FFT��֧�ֵ����ȸ����˫���ȸ���
*              ʵ��Ŀ�ģ�
*                1. ѧϰ��������FFT��֧�ֵ����ȸ����˫���ȸ���
*              ʵ�����ݣ� 
*                1. ����һ���Զ���װ�����ʱ����ÿ100ms��תһ��LED2��
*                2.	���°���K1�����ڴ�ӡ1024�㸴��������FFT�ķ�Ƶ��Ӧ����Ƶ��Ӧ��
*                3.	���°���K2�����ڴ�ӡ1024�㸴��˫����FFT�ķ�Ƶ��Ӧ����Ƶ��Ӧ��
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
*		V1.0    2021-05-22   Eric2013     1. CMSIS����汾 V5.7.0
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
#define EXAMPLE_NAME	"V7-��������FTT��֧�ֵ����Ⱥ�˫���ȣ�"
#define EXAMPLE_DATE	"2021-05-22"
#define DEMO_VER		"1.0"


/* �������ļ��ڵ��õĺ������� */
static void PrintfLogo(void);
static void PrintfHelp(void);
static void arm_cfft_f32_app(void);
static void arm_cfft_f64_app(void);


/* ���� */
uint32_t ifftFlag = 0; 
uint32_t doBitReverse = 1; 

/* ������������ */
#define TEST_LENGTH_SAMPLES 1024 
static float32_t testOutput_f32[TEST_LENGTH_SAMPLES*2]; 
static float32_t testInput_f32[TEST_LENGTH_SAMPLES*2];
static float32_t Phase_f32[TEST_LENGTH_SAMPLES*2]; /* ��λ*/ 

static float64_t testOutput_f64[TEST_LENGTH_SAMPLES*2]; 
static float64_t testInput_f64[TEST_LENGTH_SAMPLES*2];
static float64_t Phase_f64[TEST_LENGTH_SAMPLES*2]; /* ��λ*/ 

/*
*********************************************************************************************************
*	�� �� ��: PowerPhaseRadians_f32
*	����˵��: ����λ
*	��    �Σ�_ptr  ��λ��ַ����ʵ�����鲿
*             _phase �����λ����λ�Ƕ��ƣ���Χ(-180, 180]
*             _usFFTPoints  ����������ÿ������������float32_t��ֵ
*             _uiCmpValue  �Ƚ�ֵ����Ҫ�����λ����ֵ
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void PowerPhaseRadians_f32(float32_t *_ptr, float32_t *_phase, uint16_t _usFFTPoints, float32_t _uiCmpValue)		
{
	float32_t lX, lY;
	uint16_t i;
	float32_t phase;
	float32_t mag;
	
	
	for (i=0; i <_usFFTPoints; i++)
	{
		lX= _ptr[2*i];  	  /* ʵ�� */
		lY= _ptr[2*i + 1];    /* �鲿 */ 
		
 		phase = atan2f(lY, lX);    		  				 /* atan2���Ľ����Χ��(-pi, pi], ������ */
		arm_sqrt_f32((float32_t)(lX*lX+ lY*lY), &mag);   /* ��ģ */
		
		if(_uiCmpValue > mag)
		{
			Phase_f32[i] = 0;			
		}
		else
		{
			Phase_f32[i] = phase* 180.0f/3.1415926f;   /* �����Ľ���ɻ���ת��Ϊ�Ƕ� */
		}
	}
}

/*
*********************************************************************************************************
*	�� �� ��: PowerPhaseRadians_f64
*	����˵��: ����λ
*	��    �Σ�_ptr  ��λ��ַ����ʵ�����鲿
*             _phase �����λ����λ�Ƕ��ƣ���Χ(-180, 180]
*             _usFFTPoints  ����������ÿ������������float64_t��ֵ
*             _uiCmpValue  �Ƚ�ֵ����Ҫ�����λ����ֵ
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void PowerPhaseRadians_f64(float64_t *_ptr, float64_t *_phase, uint16_t _usFFTPoints, float64_t _uiCmpValue)		
{
	float64_t lX, lY;
	uint16_t i;
	float64_t phase;
	float64_t mag;
	
	
	for (i=0; i <_usFFTPoints; i++)
	{
		lX= _ptr[2*i];  	  /* ʵ�� */
		lY= _ptr[2*i + 1];    /* �鲿 */ 
		
 		phase = atan2(lY, lX);      /* atan2���Ľ����Χ��(-pi, pi], ������ */
		mag = sqrt(lX*lX+ lY*lY);   /* ��ģ */
		
		if(_uiCmpValue > mag)
		{
			Phase_f64[i] = 0;			
		}
		else
		{
			Phase_f64[i] = phase* 180.0/3.1415926;  /* �����Ľ���ɻ���ת��Ϊ�Ƕ� */
		}
	}
}

/*
*********************************************************************************************************
*	�� �� ��: arm_cfft_f32_app
*	����˵��: ���ú���arm_cfft_f32�����Ƶ����Ƶ
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void arm_cfft_f32_app(void)
{
	uint16_t i;
	
    ifftFlag = 0; 
    doBitReverse = 1; 
	
	/* ����ʵ�����鲿��ʵ�����鲿..... ��˳��洢���� */
	for(i=0; i<TEST_LENGTH_SAMPLES; i++)
	{
		/* ��������ֱ��������50Hz���Ҳ���ɣ����β�����1024����ʼ��λ60�� */
		testInput_f32[i*2] = 1 + cos(2*3.1415926f*50*i/1024 + 3.1415926f/3);
		testInput_f32[i*2+1] = 0;
	}
	
	/* CFFT�任 */ 
	arm_cfft_f32(&arm_cfft_sR_f32_len1024, testInput_f32, ifftFlag, doBitReverse);

	/* ���ģֵ  */ 
	arm_cmplx_mag_f32(testInput_f32, testOutput_f32, TEST_LENGTH_SAMPLES);
	

	printf("=========================================\r\n");	
	
	/* ����Ƶ */
	PowerPhaseRadians_f32(testInput_f32, Phase_f32, TEST_LENGTH_SAMPLES, 0.5f);
	
	/* ���ʵ�ʴ�С */
//	testOutput_f32[0] = testOutput_f32[0] / TEST_LENGTH_SAMPLES;
//	
//	for(i=1; i<TEST_LENGTH_SAMPLES; i++)
//	{
//		testOutput_f32[i] = testOutput_f32[i] /(TEST_LENGTH_SAMPLES/2);
//	}
	
	/* ���ڴ�ӡ����ģֵ */
	for(i=0; i<TEST_LENGTH_SAMPLES; i++)
	{
		printf("%f, %f\r\n", testOutput_f32[i], Phase_f32[i]);
	}
}

/*
*********************************************************************************************************
*	�� �� ��: arm_cfft_f64_app
*	����˵��: ���ú���arm_cfft_f64�����Ƶ����Ƶ
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void arm_cfft_f64_app(void)
{
	uint16_t i;
	float64_t lX,lY;
	
    ifftFlag = 0; 
    doBitReverse = 1; 
	
	/* ����ʵ�����鲿��ʵ�����鲿..... ��˳��洢���� */
	for(i=0; i<TEST_LENGTH_SAMPLES; i++)
	{
		/* ��������ֱ��������50Hz���Ҳ���ɣ����β�����1024����ʼ��λ60�� */
		testInput_f64[i*2] = 1 + cos(2*3.1415926*50*i/1024 + 3.1415926/3);
		testInput_f64[i*2+1] = 0;
	}
	
	/* CFFT�任 */ 
	arm_cfft_f64(&arm_cfft_sR_f64_len1024, testInput_f64, ifftFlag, doBitReverse);

	/* ���ģֵ  */ 
	for (i =0; i < TEST_LENGTH_SAMPLES; i++)
	{
 		lX = testInput_f64[2*i];            /* ʵ��*/
		lY = testInput_f64[2*i+1];          /* �鲿 */  
		testOutput_f64[i] = sqrt(lX*lX+ lY*lY);   /* ��ģ */
	}
	
	printf("=========================================\r\n");	
	
	/* ����Ƶ */
	PowerPhaseRadians_f64(testInput_f64, Phase_f64, TEST_LENGTH_SAMPLES, 0.5);
	
	
	/* ���ڴ�ӡ����ģֵ */
	for(i=0; i<TEST_LENGTH_SAMPLES; i++)
	{
		printf("%.11f, %.11f\r\n", testOutput_f64[i], Phase_f64[i]);
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
					arm_cfft_f32_app();
					break;
				
				case KEY_DOWN_K2:		    /* K2������ */
					arm_cfft_f64_app();
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
	printf("2. ���°���K1�����ڴ�ӡ1024�㸴��������FFT�ķ�Ƶ��Ӧ����Ƶ��Ӧ\r\n");
	printf("3. ���°���K2�����ڴ�ӡ1024�㸴��˫����FFT�ķ�Ƶ��Ӧ����Ƶ��Ӧ\r\n");
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
