/*
*********************************************************************************************************
*
*	ģ������ : ������ģ��
*	�ļ����� : main.c
*	��    �� : V1.0
*	˵    �� : �ڲ�Flashģ��EEPROM��
*              ʵ��Ŀ�ģ�
*                1��ѧϰ�ڲ�Flashģ��EEPROM��
*              ʵ�����ݣ�
*                1��ʹ���ڲ�Flashģ��EEPROM����ظ��߱���Ҫʹ�õĴ洢�ռ䣬��ֹ����ռ�����˳���
*                2������ͬһ����ַ�ռ䣬��֧��һ�α��(���Ƽ����α�̣���ʹ�ǽ���Ӧbit����ֵ1���0)��
*                3��ֻ�ܶ��Ѿ������Ŀռ�����̣�����1��������128KB��
*                4��H7��Flash���ʱ����ر�֤Ҫ��̵ĵ�ַ��32�ֽڶ���ģ����˵�ַ��32����Ϊ0��
*                   ���ұ�̵����ݱ���32�ֽ�������������bsp_WriteCpuFlash���ֽ�������32�ֽ�������������Զ���0��                   
*              ʵ�������
*                1��K1�����£���8bit��16bit��32bit����д�뵽�ڲ�Flash��
*                2��K2�����£����ṹ������д�뵽�ڲ�Flash��
*              ע�����
*                1. ��ʵ���Ƽ�ʹ�ô������SecureCRT�鿴��ӡ��Ϣ��������115200������λ8����żУ��λ�ޣ�ֹͣλ1��
*                2. ��ؽ��༭��������������TAB����Ϊ4���Ķ����ļ���Ҫ��������ʾ�����롣
*
*	�޸ļ�¼ :
*		�汾��   ����         ����        ˵��
*		V1.0    2020-02-29   Eric2013     1. CMSIS����汾 V5.6.0
*                                         2. HAL��汾 V1.7.0
*
*	Copyright (C), 2020-2030, ���������� www.armfly.com
*
*********************************************************************************************************
*/	
#include "bsp.h"			 /* �ײ�Ӳ������ */



/* ���������������̷������� */
#define EXAMPLE_NAME	"V7-�ڲ�Flashģ��EEPROM"
#define EXAMPLE_DATE	"2020-02-29"
#define DEMO_VER		"1.0"

static void PrintfLogo(void);
static void PrintfHelp(void);


/* ȫ�ֲ��� */
typedef struct
{
	uint8_t   ParamVer;			
	uint16_t  ucBackLight;
	uint32_t  Baud485;
	float     ucRadioMode;		
}
PARAM_T;


/* 
   1����һ�������Ŀռ�Ԥ��������Ϊ�������������ǽ���2��������Ϊ��������
      Ĭ������²�Ҫ����1������������������Ϊ��1��������Ĭ�ϵ�boot������ַ��
   2��ͨ�����ֶ��巽ʽ���߱��������˿ռ��Ѿ���ռ�ã����ñ�������Ϊ����ռ��д����
*/
#if defined ( __CC_ARM )       /* MDK������ */
	const uint8_t para_flash_area[128*1024] __attribute__((at(0x08000000 + 128*1024)));
#elif defined ( __ICCARM__ )   /* IAR������ */
	#pragma location=0x08000000 + 128*1024
	const uint8_t para_flash_area[128*1024];
#endif

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
	uint8_t ucKeyCode;	/* �������� */
	uint8_t  ucTest, *ptr8;
	uint16_t uiTest, *ptr16;
	uint32_t ulTest, *ptr32;
	PARAM_T tPara, *paraptr;

	
	/* ��ʼ������ */
	tPara.Baud485 = 0x5555AAAA;
	tPara.ParamVer = 0x99;
	tPara.ucBackLight = 0x7788;
	tPara.ucRadioMode = 99.99f;
	
	
	bsp_Init();		/* Ӳ����ʼ�� */
	PrintfLogo();	/* ��ӡ�������ƺͰ汾����Ϣ */
	PrintfHelp();	/* ��ӡ������ʾ */
	
	bsp_StartAutoTimer(0, 100);	/* ����1��100ms���Զ���װ�Ķ�ʱ�� */
	while (1)
	{
		bsp_Idle();		/* ���������bsp.c�ļ����û������޸��������ʵ��CPU���ߺ�ι�� */

		/* �ж϶�ʱ����ʱʱ�� */
		if (bsp_CheckTimer(0))	
		{
			/* ÿ��100ms ����һ�� */  
			bsp_LedToggle(2);
		}

		/* �����˲��ͼ���ɺ�̨systick�жϷ������ʵ�֣�����ֻ��Ҫ����bsp_GetKey��ȡ��ֵ���ɡ� */
		ucKeyCode = bsp_GetKey();	/* ��ȡ��ֵ, �޼�����ʱ���� KEY_NONE = 0 */
		if (ucKeyCode != KEY_NONE)
		{
			switch (ucKeyCode)
			{
				case KEY_DOWN_K1:			/* K1�����£���8bit��16bit��32bit����д�뵽�ڲ�Flash */
					
					/*
						1������ͬһ����ַ�ռ䣬��֧��һ�α��(���Ƽ����α�̣���ʹ�ǽ���Ӧbit����ֵ1���0)��
				        2��ֻ�ܶ��Ѿ������Ŀռ�����̣�����1��������128KB��
				        3��H7��Flash���ʱ����ر�֤Ҫ��̵ĵ�ַ��32�ֽڶ���ģ����˵�ַ��32����Ϊ0�����ұ�̵����ݱ���32�ֽ���������
				           ����bsp_WriteCpuFlash���ֽ�������32�ֽ�������������Զ���0��
					*/
					/* �������� */
					bsp_EraseCpuFlash((uint32_t)para_flash_area);
				
					ucTest = 0xAA;
					uiTest = 0x55AA;
					ulTest = 0x11223344;
					
					/* ����д������ */
					bsp_WriteCpuFlash((uint32_t)para_flash_area + 32*0,  (uint8_t *)&ucTest, sizeof(ucTest));
					bsp_WriteCpuFlash((uint32_t)para_flash_area + 32*1,  (uint8_t *)&uiTest, sizeof(uiTest));
					bsp_WriteCpuFlash((uint32_t)para_flash_area + 32*2,  (uint8_t *)&ulTest, sizeof(ulTest));				
					
					/* �������ݲ���ӡ */
					ptr8  = (uint8_t  *)(para_flash_area + 32*0);
					ptr16 = (uint16_t *)(para_flash_area + 32*1);
					ptr32 = (uint32_t *)(para_flash_area + 32*2);
				
					printf("д�����ݣ�ucTest = %x, uiTest = %x, ulTest = %x\r\n", ucTest, uiTest, ulTest);
					printf("��ȡ���ݣ�ptr8 = %x, ptr16 = %x, ptr32 = %x\r\n", *ptr8, *ptr16, *ptr32);
					
					break;
				
				case KEY_DOWN_K2:			/* K2�����£� ���ṹ������д�뵽�ڲ�Flash */
					/* �������� */
					bsp_EraseCpuFlash((uint32_t)para_flash_area);

					/* ����д������ */
					bsp_WriteCpuFlash((uint32_t)para_flash_area,  (uint8_t *)&tPara, sizeof(tPara));			
					
					/* �������ݲ���ӡ */
					paraptr  = (PARAM_T  *)((uint32_t)para_flash_area);
				

					printf("д�����ݣ�Baud485=%x, ParamVer=%x, ucBackLight=%x, ucRadioMode=%f\r\n", 
																				tPara.Baud485,
																				tPara.ParamVer,
																				tPara.ucBackLight,
																				tPara.ucRadioMode);
				
					printf("��ȡ���ݣ�Baud485=%x, ParamVer=%x, ucBackLight=%x, ucRadioMode=%f\r\n", 
																				paraptr->Baud485,
																				paraptr->ParamVer,
																				paraptr->ucBackLight,
																				paraptr->ucRadioMode);
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
	printf("2. K1�����£���8bit��16bit��32bit����д�뵽�ڲ�Flash\r\n");
	printf("3. K2�����£����ṹ������д�뵽�ڲ�Flash\r\n");	
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
	printf("* HAL��汾  : V1.7.0 (STM32H7xx HAL Driver)\r\n");
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
