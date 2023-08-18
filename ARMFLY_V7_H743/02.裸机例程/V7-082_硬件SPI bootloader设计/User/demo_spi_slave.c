/*
*********************************************************************************************************
*
*	ģ������ : SPI �ӻ�
*	�ļ����� : demo_spi_slave.c
*	��    �� : V1.0
*	˵    �� : SPI �ӻ�
*     
*	�޸ļ�¼ :
*		�汾��  ����        ����     ˵��
*		V1.0    2022-04-08 Eric2013  ��ʽ����
*
*	Copyright (C), 2020-2030, ���������� www.armfly.com
*
*********************************************************************************************************
*/
#include "demo_spi_slave.h"
#include "bsp.h"


/* �������ļ��ڵ��õĺ������� */
static void sfDispMenu(void);
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
*	�� �� ��: DemoSpiSlave
*	����˵��: SPI �ӻ�ͨ��
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void DemoSpiSlave(void)
{
	uint32_t SectorCount = 0;
	uint32_t SectorRemain = 0;
	uint32_t i;
    uint32_t TotalSize = 0;
	uint8_t ucState;
	
    
    /***************����SPI FlashƬѡ��������ֹӰ�� ***************/
    {
        GPIO_InitTypeDef gpio_init;

        /* ��GPIOʱ�� */
        __HAL_RCC_GPIOD_CLK_ENABLE();
        
        gpio_init.Mode = GPIO_MODE_OUTPUT_PP;	/* ����������� */
        gpio_init.Pull = GPIO_NOPULL;			/* ���������費ʹ�� */
        gpio_init.Speed = GPIO_SPEED_HIGH;  	/* GPIO�ٶȵȼ� */	
        gpio_init.Pin = GPIO_PIN_13;	
        HAL_GPIO_Init(GPIOD, &gpio_init);

        GPIOD->BSRR = GPIO_PIN_13;
    }
    
	sfDispMenu();		/* ��ӡ������ʾ */
	
	bsp_StartAutoTimer(0, 100);	/* ����1��100ms���Զ���װ�Ķ�ʱ�� */
    
    /* �ϵ��׼�������������� */
	g_spiTxBuf[69] = 0x30;
    g_spiLen = 70;
    bsp_spiTransfer();
    
	while(1)
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
			/* ���ݹ̼���С������������ ************/
			if(g_spiRxBuf[0] == '*')
			{
				/* ��ȡ�ļ���С */
				filesize = g_spiRxBuf[1] + (g_spiRxBuf[2] << 8) + (g_spiRxBuf[3] << 16) + (g_spiRxBuf[4] << 24);
				uwAppSize = filesize;
				for(int i = 0; i < 69; i++)
				{
					printf("%x ", g_spiRxBuf[i]);
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
				/* �ϵ��׼�������������� */
				g_spiTxBuf[69] = 0x30;
				g_spiLen = 70;
				bsp_spiTransfer();
			}
			
			/* ����������� **************/
			if(g_spiRxBuf[0]  == '#')
			{
				JumpToApp();
			}
			
			/* ��ʼ����̼����� **************/
			if(g_spiRxBuf[0]  == '$')
			{					   
				/* �������ݸ��� */
				RecSize = g_spiRxBuf[1];
				
				/* ����ڲ�Flash, */
				ucState = bsp_WriteCpuFlash((uint32_t)(AppAddr + TotalSize),  (uint8_t *)&g_spiRxBuf[2], RecSize);
				TotalSize += RecSize;
				printf("=====%d\r\n", TotalSize);
				
				/* ������ط�0����ʾ���ʧ�� */
				if(ucState != 0)
				{
					/* ����0x60����ʾ���ʧ�� */
					g_spiTxBuf[69] = 0x60;
				}
				else
				{
					g_spiTxBuf[69] = 0x30;
				}
				
				/* ����0x30����ʾ�����ɹ� */
				/* �ϵ��׼�������������� */
				g_spiLen = 70;
				bsp_spiTransfer();
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
*	�� �� ��: sfDispMenu
*	����˵��: ��ʾ������ʾ�˵�
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void sfDispMenu(void)
{
	printf("SPI�ӻ�����\r\n");
	printf("�ȴ�������������\r\n");
}

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
