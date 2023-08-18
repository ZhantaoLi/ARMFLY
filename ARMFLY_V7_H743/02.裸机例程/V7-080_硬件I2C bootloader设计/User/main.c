/*
*********************************************************************************************************
*
*	ģ������ : ������ģ��
*	�ļ����� : main.c
*	��    �� : V1.0
*	˵    �� : ����Ӳ��I2C��bootloader���
*              ʵ��Ŀ�ģ�
*                1. ����Ӳ��I2C��bootloader���
*              ע�����
*                1. ��ӡ��ʽ��
*                   Ĭ��ʹ�ô��ڴ�ӡ������ʹ��SecureCRT����H7-TOOL��λ���������ֲ鿴��ӡ��Ϣ��
*                   ������115200������λ8����żУ��λ�ޣ�ֹͣλ1��
*                2. ��ؽ��༭��������������TAB����Ϊ4���Ķ����ļ���Ҫ��������ʾ�����롣
*
*	�޸ļ�¼ :
*		�汾��   ����         ����        ˵��
*		V1.0    2022-07-18  Eric2013     1. CMSIS����汾 V5.7.0
*                                         2. HAL��汾 V1.10.0
*
*	Copyright (C), 2020-2030, ���������� www.armfly.com
*
*********************************************************************************************************
*/	
#include "bsp.h"			 /* �ײ�Ӳ������ */


/* ���������������̷������� */
#define EXAMPLE_NAME	"V7-����Ӳ��I2C��bootloader���"
#define EXAMPLE_DATE	"2022-07-18"
#define DEMO_VER		"1.0"


/* �������ļ��ڵ��õĺ������� */
static void PrintfLogo(void);
static void PrintfHelp(void);

extern __IO uint32_t wTransferState;

/*
*********************************************************************************************************
*	                                       �궨��
*********************************************************************************************************
*/
#define AppAddr  0x08100000    /* APP��ַ */

__IO uint32_t uwCRCValue;
__IO uint32_t uwExpectedCRCValue;
__IO uint32_t uwAppSize;

uint8_t buf[1024];
uint32_t RecCount = 0;
uint32_t RecCount0 = 0;
uint32_t RecSize = 0;
uint8_t RecCplt = 0;
uint32_t filesize = 0;

static void JumpToApp(void);

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
	uint32_t SectorCount = 0;
	uint32_t SectorRemain = 0;
	uint32_t i;
    uint32_t TotalSize = 0;
	uint8_t ucState;
	

	bsp_Init();		/* Ӳ����ʼ�� */
	PrintfLogo();	/* ��ӡ������Ϣ������1 */
	PrintfHelp();	/* ��ӡ������ʾ��Ϣ */

	bsp_StartAutoTimer(0, 100);	/* ����1��100ms���Զ���װ�Ķ�ʱ�� */
	
	/* �״�ʹ�ã�������64�ֽڽ��� */
	g_i2cLen = 69;
	bsp_i2cReceive();
	
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
		
        if (wTransferState != TRANSFER_WAIT)
        {
			/* ����̼� */
			if(g_i2cRxBuf[0] == '*')
			{
				/* ��ȡ�ļ���С */
				filesize = g_i2cRxBuf[1] + (g_i2cRxBuf[2] << 8) + (g_i2cRxBuf[3] << 16) + (g_i2cRxBuf[4] << 24);
				uwAppSize = filesize;
				for(int i = 0; i < 69; i++)
				{
					printf("%x ", g_i2cRxBuf[i]);
				}
				
				/* �����ļ���Сִ�в��� */
				SectorCount = filesize/(128*1024);
				SectorRemain = filesize%(128*1024);	
				
				printf("filesize = %d\r\n", filesize);
				for(i = 0; i < SectorCount; i++)
				{
					bsp_EraseCpuFlash((uint32_t)(AppAddr + i*128*1024));
				}
				
				if(SectorRemain)
				{
					bsp_EraseCpuFlash((uint32_t)(AppAddr + i*128*1024));
				}
				
				/* ����0x30����ʾ�����ɹ� */
				g_i2cLen = 1;
				g_i2cTxBuf[0] = 0x30;
				bsp_i2cTransfer();
				
				/* ����ִ���´ν��� */
				g_i2cLen = 69;
				bsp_i2cReceive();
			}
			
			/* ����������� **************/
			if(g_i2cRxBuf[0]  == '#')
			{
				JumpToApp();
			}
	
			/* ��ʼ����̼����� **************/
			if(g_i2cRxBuf[0]  == '$')
			{					   
				/* �������ݸ��� */
				RecSize = g_i2cRxBuf[1];
				
				/* ����ڲ�Flash, */
				ucState = bsp_WriteCpuFlash((uint32_t)(AppAddr + TotalSize),  (uint8_t *)&g_i2cRxBuf[2], RecSize);
				TotalSize += RecSize;
				printf("=====%d\r\n", TotalSize);
				
				/* ������ط�0����ʾ���ʧ�� */
				if(ucState != 0)
				{
					/* ����0x60����ʾ���ʧ�� */
					g_i2cLen = 1;
					g_i2cTxBuf[0] = 0x60;
					bsp_i2cTransfer();
				}
				
				/* ����0x30����ʾ��̳ɹ� */  
				g_i2cLen = 1;
				g_i2cTxBuf[0] = 0x30;
				bsp_i2cTransfer();
				
				/* ����ִ���´ν��� */
				g_i2cLen = 69;
				bsp_i2cReceive();
			}
			
		}
	}
}

/*
*********************************************************************************************************
*	�� �� ��: JumpToApp
*	����˵��: ��ת��Ӧ��JumpToApp
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void JumpToApp(void)
{
	uint32_t i=0;
	void (*AppJump)(void);         /* ����һ������ָ�� */
    
    /* �ر�ȫ���ж� */
	DISABLE_INT(); 
    
    /* ��������ʱ�ӵ�Ĭ��״̬��ʹ��HSIʱ�� */
	HAL_RCC_DeInit();
    
	/* �رյδ�ʱ������λ��Ĭ��ֵ */
	SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL = 0;

	/* �ر������жϣ���������жϹ����־ */
	for (i = 0; i < 8; i++)
	{
		NVIC->ICER[i]=0xFFFFFFFF;
		NVIC->ICPR[i]=0xFFFFFFFF;
	}	

	/* ʹ��ȫ���ж� */
	ENABLE_INT();

	/* ��ת��Ӧ�ó����׵�ַ��MSP����ַ+4�Ǹ�λ�жϷ�������ַ */
	AppJump = (void (*)(void)) (*((uint32_t *) (AppAddr + 4)));

	/* ��������ջָ�� */
	__set_MSP(*(uint32_t *)AppAddr);
	
	/* ��RTOS���̣�����������Ҫ������Ϊ��Ȩ��ģʽ��ʹ��MSPָ�� */
	__set_CONTROL(0);

	/* ��ת��ϵͳBootLoader */
	AppJump(); 

	/* ��ת�ɹ��Ļ�������ִ�е�����û�������������Ӵ��� */
	while (1)
	{

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
	printf("1. I2C���������ַ�1����󣬴ӻ�����17���ַ������� \r\n");
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
