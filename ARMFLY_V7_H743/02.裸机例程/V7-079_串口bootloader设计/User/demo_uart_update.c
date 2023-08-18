/*
*********************************************************************************************************
*
*	ģ������ : ���ڷ�ʽ�̼�����
*	�ļ����� : demo_uart_update.c
*	��    �� : V1.0
*	˵    �� : ���ڷ�ʽ�̼�����
*
*	�޸ļ�¼ :
*		�汾��   ����         ����        ˵��
*		V1.0    2022-06-15   Eric2013    ��ʽ����
*
*	Copyright (C), 2022-2030, ���������� www.armfly.com
*
*********************************************************************************************************
*/
#include "bsp.h"


/*
*********************************************************************************************************
*	                                        ����
*********************************************************************************************************
*/
/* �������ļ��ڵ��õĺ������� */
static void JumpToApp(void);

/*
*********************************************************************************************************
*	                                       �궨��
*********************************************************************************************************
*/
#define AppAddr  0x08100000    /* APP��ַ */


/*
*********************************************************************************************************
*	                                       ����
*********************************************************************************************************
*/
__IO uint32_t uwCRCValue;
__IO uint32_t uwExpectedCRCValue;
__IO uint32_t uwAppSize;

uint8_t buf[1024];
uint32_t RecCount = 0;
uint32_t RecCount0 = 0;
uint32_t RecSize = 0;
uint8_t RecCplt = 0;
uint32_t filesize = 0;

/*
*********************************************************************************************************
*	�� �� ��: DemoUartUpdate
*	����˵��: ������¼
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void DemoUartUpdate(void)
{
	uint8_t cmd;
	uint8_t ucStatus = 0;  /* ״̬����־ */
	uint32_t SectorCount = 0;
	uint32_t SectorRemain = 0;
	uint32_t i;
    uint32_t TotalSize = 0;
	uint8_t ucState;
	
	
	bsp_StartAutoTimer(0, 500);	/* ����1��500ms���Զ���װ�Ķ�ʱ�� */
	
	while (1)
	{
		/* �ж϶�ʱ����ʱʱ�� */
		if (bsp_CheckTimer(0))	
		{            
			/* ÿ��500ms ����һ�� */  
			bsp_LedToggle(2);
		}

		if (comGetChar(COM1, &cmd))	/* �Ӵ��ڶ���һ���ַ�(��������ʽ) */
		{
			switch (ucStatus)
			{
				case 0:
					/* ��ʼ����̼����� **************/
					if(cmd == '$')
					{
						RecCplt = 0;
						ucStatus = 1;        
					}
					
					/* ����������� **************/
					if(cmd == '#')
					{
						RecCount = 0;
						RecCplt = 1;
						JumpToApp();
					}
					
					/* ���չ̼���С���� */
					if(cmd == '*')
					{
						ucStatus = 3; 
					}
					break;
					
				/* ����ÿ֡�����ֽ�����Ĭ�����õ�224�ֽ� */
				case 1:
					RecSize = cmd;
					ucStatus = 2; 
					break;
					   
				/* �����յ������ݱ�̵��ڲ�Flash */
				case 2:
					buf[RecCount0] = cmd;
                                       
					/* ���չ�224������ */
					if(RecCount0 == (RecSize - 1))
					{
						ucStatus = 0;
						RecCount0 = 0;
						
						/* ����ڲ�Flash, */
						ucState = bsp_WriteCpuFlash((uint32_t)(AppAddr + TotalSize),  (uint8_t *)buf, RecSize);
						TotalSize += RecSize;
						
						/* ������ط�0����ʾ���ʧ�� */
						if(ucState != 0)
						{
							/* ����0x60����ʾ���ʧ�� */
							comSendChar(COM1, 0x60);
						}
						
						/* ����0x30����ʾ��̳ɹ� */
						comSendChar(COM1, 0x30);
					}
					else
					{
						RecCount++;
						RecCount0++;
					}
					break;
					
				/* ���ݽ��յ����ļ���С��������Ӧ��С������ */
				case 3:
					buf[RecCount0] = cmd;
                                       
					if(RecCount0 == 3)
					{
						ucStatus = 0;
						RecCount0 = 0;
						filesize = buf[0] + (buf[1] << 8) + (buf[2] << 16) + (buf[3] << 24);
						uwAppSize = filesize;
						SectorCount = filesize/(128*1024);
						SectorRemain = filesize%(128*1024);	
						
						for(i = 0; i < SectorCount; i++)
						{
							bsp_EraseCpuFlash((uint32_t)(AppAddr + i*128*1024));
						}
						
						if(SectorRemain)
						{
							bsp_EraseCpuFlash((uint32_t)(AppAddr + i*128*1024));
						}
						
						/* ����0x30����ʾ�����ɹ� */
						comSendChar(COM1, 0x30);
					}
					else
					{
						RecCount0++;
					}
					break;
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

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
