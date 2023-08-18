/*
*********************************************************************************************************
*
*	ģ������ : ������ģ��
*	�ļ����� : main.c
*	��    �� : V1.0
*	˵    �� : �ڲ�TCM��SRAM���ⲿSDRAM�������ڴ�ĳ�����ʹ�÷�ʽ��
*              ʵ��Ŀ�ģ�
*                1. ѧϰTCM��SRAM�������ڴ�ĳ�����ʹ�÷�ʽ��
*              ʵ�����ݣ�
*                1. �����Զ���װ�����ʱ��0��ÿ100ms��תһ��LED2��
*              ʵ�������
*                1. K1�����£�����AXI SRAM��
*                2. K2�����£�����D2���SRAM1��SRAM2��SRAM3��
*                3. K3�����£�����D3���SRAM4��
*                4. OK�����£�����SDRAM��        
*              ע�����
*                1. ��ʵ���Ƽ�ʹ�ô������SecureCRT�鿴��ӡ��Ϣ��������115200������λ8����żУ��λ�ޣ�ֹͣλ1��
*                2. ��ؽ��༭��������������TAB����Ϊ4���Ķ����ļ���Ҫ��������ʾ�����롣
*
*	�޸ļ�¼ :
*		�汾��   ����         ����        ˵��
*		V1.0    2020-07-08   Eric2013     1. CMSIS����汾 V5.4.0
*                                         2. HAL��汾 V1.3.0
*
*	Copyright (C), 2018-2030, ���������� www.armfly.com
*
*********************************************************************************************************
*/	
#include "bsp.h"			/* �ײ�Ӳ������ */



/* ���������������̷������� */
#define EXAMPLE_NAME	"V7-�ڲ�TCM��SRAM���ⲿSDRAM�������ڴ�ĳ�����ʹ�÷�ʽ"
#define EXAMPLE_DATE	"2020-07-08"
#define DEMO_VER		"1.0"

static void PrintfLogo(void);
static void PrintfHelp(void);


/* IAR ---------------------------------------------*/
#if defined ( __ICCARM__ )
/* ������512KB AXI SRAM����ı��� */
#pragma location = ".RAM_D1"  
uint32_t AXISRAMBuf[10];
#pragma location = ".RAM_D1"  
uint16_t AXISRAMCount;

/* ������128KB SRAM1(0x30000000) + 128KB SRAM2(0x30020000) + 32KB SRAM3(0x30040000)����ı��� */
#pragma location = ".RAM_D2" 
uint32_t D2SRAMBuf[10];
#pragma location = ".RAM_D2" 
uint16_t D2SRAMount;

/* ������64KB SRAM4(0x38000000)����ı��� */
#pragma location = ".RAM_D3"  
uint32_t D3SRAMBuf[10];
#pragma location = ".RAM_D3"  
uint16_t D3SRAMCount;

/* ������64KB SRAM4(0x38000000)����ı��� */
#pragma location = ".RAM_D3"  
uint32_t D3SRAMBuf[10];
#pragma location = ".RAM_D3"  
uint16_t D3SRAMCount;

/* ������32MB SDRAM(0xC0000000)����ı��� */
#pragma location = ".RAM_SDRAM"  
uint32_t SDRAMSRAMBuf[10];
#pragma location = ".RAM_SDRAM"  
uint16_t SDRAMSRAMCount;

/* MDK ----------------------------------------------*/
#elif defined ( __CC_ARM )

/* ������512KB AXI SRAM����ı��� */
__attribute__((section (".RAM_D1"))) uint32_t AXISRAMBuf[10];
__attribute__((section (".RAM_D1"))) uint16_t AXISRAMCount;

/* ������128KB SRAM1(0x30000000) + 128KB SRAM2(0x30020000) + 32KB SRAM3(0x30040000)����ı��� */
__attribute__((section (".RAM_D2"))) uint32_t D2SRAMBuf[10];
__attribute__((section (".RAM_D2"))) uint16_t D2SRAMount;

/* ������64KB SRAM4(0x38000000)����ı��� */
__attribute__((section (".RAM_D3"))) uint32_t D3SRAMBuf[10];
__attribute__((section (".RAM_D3"))) uint16_t D3SRAMCount;

/* ������32MB SDRAM(0xC0000000)����ı��� */
__attribute__((section (".RAM_SDRAM"),zero_init)) uint16_t SDRAMSRAMCount;
__attribute__((section (".RAM_SDRAM"),zero_init)) uint32_t SDRAMSRAMBuf[10];

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
	uint8_t ucKeyCode;		/* �������� */
    
    
	bsp_Init();		/* Ӳ����ʼ�� */
	
	PrintfLogo();	/* ��ӡ�������ƺͰ汾����Ϣ */
	PrintfHelp();	/* ��ӡ������ʾ */

	bsp_StartAutoTimer(0, 100);	/* ����1��100ms���Զ���װ�Ķ�ʱ�� */
	
	AXISRAMCount = 0;
	D2SRAMount = 0;
	D3SRAMCount = 0;
	SDRAMSRAMCount = 0;
	
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

		/* �����˲��ͼ���ɺ�̨systick�жϷ������ʵ�֣�����ֻ��Ҫ����bsp_GetKey��ȡ��ֵ���ɡ� */
		ucKeyCode = bsp_GetKey();	/* ��ȡ��ֵ, �޼�����ʱ���� KEY_NONE = 0 */
		if (ucKeyCode != KEY_NONE)
		{
			switch (ucKeyCode)
			{
				case KEY_DOWN_K1:			/* K1�����£�����AXI SRAM */
					AXISRAMBuf[0] = AXISRAMCount++;
					AXISRAMBuf[5] = AXISRAMCount++;
					AXISRAMBuf[9] = AXISRAMCount++;
					printf("K1������, AXISRAMBuf[0] = %d, AXISRAMBuf[5] = %d, AXISRAMBuf[9] = %d\r\n", 
																						AXISRAMBuf[0],
																						AXISRAMBuf[5],
																						AXISRAMBuf[9]);
					break;

				case KEY_DOWN_K2:			/* K2�����£�����D2���SRAM1��SRAM2��SRAM3 */
					D2SRAMBuf[0] = D2SRAMount++;
					D2SRAMBuf[5] = D2SRAMount++;
					D2SRAMBuf[9] = D2SRAMount++;
					printf("K2������, D2SRAMBuf[0] = %d, D2SRAMBuf[5] = %d, D2SRAMBuf[9] = %d\r\n", 
																						D2SRAMBuf[0],
																						D2SRAMBuf[5],
																						D2SRAMBuf[9]);
                    break;
				
				case KEY_DOWN_K3:			/* K3�����£�����D3���SRAM4 */			
                 	D3SRAMBuf[0] = D3SRAMCount++;
					D3SRAMBuf[5] = D3SRAMCount++;
					D3SRAMBuf[9] = D3SRAMCount++;
					printf("K3������, D3SRAMBuf[0] = %d, D3SRAMBuf[5] = %d, D3SRAMBuf[9] = %d\r\n", 
																						D3SRAMBuf[0],
																						D3SRAMBuf[5],
																						D3SRAMBuf[9]);
                  break;
				
				case JOY_DOWN_OK:			/* ҡ��OK�����£�����SDRAM */			
                 	SDRAMSRAMBuf[0] = SDRAMSRAMCount++;
					SDRAMSRAMBuf[5] = SDRAMSRAMCount++;
					SDRAMSRAMBuf[9] = SDRAMSRAMCount++;
					printf("K3������, SDRAMSRAMBuf[0] = %d, SDRAMSRAMBuf[5] = %d, SDRAMSRAMBuf[9] = %d\r\n", 
																						SDRAMSRAMBuf[0],
																						SDRAMSRAMBuf[5],
																						SDRAMSRAMBuf[9]);
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
	printf("1. K1�����£�����AXI SRAM\r\n");
	printf("2. K2�����£�����D2���SRAM1��SRAM2��SRAM3\r\n");
	printf("3. K3�����£�����D3���SRAM4\r\n");
	printf("4. OK�����£�����SDRAM\r\n");
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
