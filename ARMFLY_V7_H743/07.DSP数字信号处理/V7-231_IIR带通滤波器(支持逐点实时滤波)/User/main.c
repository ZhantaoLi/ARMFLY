/*
*********************************************************************************************************
*
*	ģ������ : ������ģ��
*	�ļ����� : main.c
*	��    �� : V1.0
*	˵    �� : IIR��ͨ�˲�����ʵ�֣�֧��ʵʱ�˲�
*              ʵ��Ŀ�ģ�
*                1.	ѧϰIIR��ͨ�˲�����ʵ�֣�֧��ʵʱ�˲�
*              ʵ�����ݣ� 
*                1. ����һ���Զ���װ�����ʱ����ÿ100ms��תһ��LED2��
*                2.	���°���K1����ӡԭʼ�������ݺ��˲���Ĳ������ݡ�
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
*		V1.0    2020-08-15   Eric2013     1. CMSIS����汾 V5.8.0
*                                         2. HAL��汾 V1.10.0
*
*	Copyright (C), 2020-2030, ���������� www.armfly.com
*
*********************************************************************************************************
*/	
#include "bsp.h"			 	/* �ײ�Ӳ������ */
#include "arm_math.h"
#include "arm_const_structs.h"


/* ���������������̷������� */
#define EXAMPLE_NAME	"IIR��ͨ�˲���"
#define EXAMPLE_DATE	"2021-08-15"
#define DEMO_VER		"1.0"


/* �������ļ��ڵ��õĺ������� */
static void PrintfLogo(void);
static void PrintfHelp(void);
static void arm_iir_f32_bp(void);


#define numStages  2                /* 2��IIR�˲��ĸ��� */
#define TEST_LENGTH_SAMPLES  400    /* �������� */
#define BLOCK_SIZE           1    	 /* ����һ��arm_biquad_cascade_df1_f32����Ĳ�������� */


uint32_t blockSize = BLOCK_SIZE;
uint32_t numBlocks = TEST_LENGTH_SAMPLES/BLOCK_SIZE;            /* ��Ҫ����arm_biquad_cascade_df1_f32�Ĵ��� */


static float32_t testInput_f32_50Hz_200Hz[TEST_LENGTH_SAMPLES]; /* ������ */
static float32_t testOutput[TEST_LENGTH_SAMPLES];               /* �˲������� */
static float32_t IIRStateF32[4*numStages];                      /* ״̬���� */
      
/* ������˹��ͨ�˲���ϵ��140Hz 400Hz*/                                                                                                                                         
const float32_t IIRCoeffs32BP[5*numStages] = {
	1.0f,  0.0f,  -1.0f,     -1.127651872054164616798743736580945551395f,  -0.470013145087532668853214090631809085608f,      
	1.0f,  0.0f,  -1.0f,     0.774953058046049081397654845204669982195f,  -0.367077500556684199750634434167295694351f                               
};                                              

/*
*********************************************************************************************************
*	�� �� ��: arm_iir_f32_bp
*	����˵��: ���ú���arm_iir_f32_hpʵ�ִ�ͨ�˲���
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void arm_iir_f32_bp(void)
{
	uint32_t i;
	arm_biquad_casd_df1_inst_f32 S;
	float32_t ScaleValue;
	float32_t  *inputF32, *outputF32;
	
	/* ��ʼ�������������ָ�� */
	inputF32 = &testInput_f32_50Hz_200Hz[0];
	outputF32 = &testOutput[0];
	
	
	/* ��ʼ�� */
	arm_biquad_cascade_df1_init_f32(&S, numStages, (float32_t *)&IIRCoeffs32BP[0], (float32_t *)&IIRStateF32[0]);
	
	
	/* ʵ��IIR�˲�������ÿ�δ���1���� */
	for(i=0; i < numBlocks; i++)
	{
		arm_biquad_cascade_df1_f32(&S, inputF32 + (i * blockSize),  outputF32 + (i * blockSize),  blockSize);
	}
	        
	/*����ϵ�� */
	ScaleValue = 0.558156585760773649163013487850548699498f * 0.558156585760773649163013487850548699498f; 
	
	/* ��ӡ�˲����� */
	for(i=0; i<TEST_LENGTH_SAMPLES; i++)
	{
		printf("%f, %f\r\n", testInput_f32_50Hz_200Hz[i], testOutput[i]*ScaleValue);
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
	uint16_t i;

	
	bsp_Init();		/* Ӳ����ʼ�� */
	PrintfLogo();	/* ��ӡ������Ϣ������1 */

	PrintfHelp();	/* ��ӡ������ʾ��Ϣ */
	
	for(i=0; i<TEST_LENGTH_SAMPLES; i++)
	{
		/* 50Hz���Ҳ�+200Hz���Ҳ���������1KHz */
		testInput_f32_50Hz_200Hz[i] = arm_sin_f32(2*3.1415926f*50*i/1000) + arm_sin_f32(2*3.1415926f*200*i/1000);
	}
	
	bsp_StartAutoTimer(0, 100);	/* ����1��100ms���Զ���װ�Ķ�ʱ�� */

	/* ����������ѭ���� */
	while (1)
	{
		bsp_Idle();		/* ���������bsp.c�ļ����û������޸��������ʵ��CPU���ߺ�ι�� */
		

		if (bsp_CheckTimer(0))	/* �ж϶�ʱ����ʱʱ�� */
		{
			/* ÿ��100ms ����һ�� */
			bsp_LedToggle(2);	/* ��תLED��״̬ */
		}
		
		ucKeyCode = bsp_GetKey();	/* ��ȡ��ֵ, �޼�����ʱ���� KEY_NONE = 0 */
		if (ucKeyCode != KEY_NONE)
		{
			switch (ucKeyCode)
			{
				case KEY_DOWN_K1:		    /* K1������ */
					arm_iir_f32_bp();
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
	printf("2. ���°���K1����ӡԭʼ�������ݺ��˲���Ĳ�������\r\n");
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

		printf("\r\nCPU : STM32H743XIH6, BGA240, ��Ƶ: %dMHz\r\n", 480);
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
