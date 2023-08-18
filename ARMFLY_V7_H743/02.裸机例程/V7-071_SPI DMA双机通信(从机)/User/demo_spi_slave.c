/*
*********************************************************************************************************
*
*	ģ������ : SPI �ӻ�
*	�ļ����� : demo_spi_slave.c
*	��    �� : V1.0
*	˵    �� : SPI �ӻ�
*                _________________________                        _____________________________
*                |           ______________|                      |______________________       |
*                |          |     SPI1     |                      |        SPI1          |      |
*                |          |              |                      |                      |      |
*                |          |     CLK(PB3) |______________________|(PB3)CLK              |      |
*                |          |              |                      |                      |      |
*                |          |    MISO(PB4) |______________________|(PB4)MISO             |      |
*                |          |              |                      |                      |      |
*                |          |    MOSI(PB5) |______________________|(PB5)MOSI             |      |
*                |          |              |                      |                      |      |
*                |          |    NSS(PG10) |______________________|(PG10)NSS             |      |
*                |          |______________|                      |______________________|      |
*                |                         |                      |                             |
*                |                         |                      |                             |
*                |                         |                      |                             |
*                |                      GND|______________________|GND                          |
*                |                         |                      |                             |
*                |_STM32H7 Master _________|                      |_STM32H7 Slave ______________|
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
*	�� �� ��: DemoSpiSlave
*	����˵��: SPI �ӻ�ͨ��
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void DemoSpiSlave(void)
{
    uint8_t count = 0;
    
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
    g_spiTxBuf[0] = count++;
    g_spiTxBuf[1] = count++;
    g_spiTxBuf[2] = count++;
    g_spiTxBuf[3] = count++;
    g_spiLen = 4;
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
            printf("SPI�ӻ��������� = %d,%d,%d,%d\r\n", g_spiTxBuf[0], g_spiTxBuf[1], g_spiTxBuf[2], g_spiTxBuf[3]);
            printf("SPI�ӻ��������� = %d,%d,%d,%d\r\n", g_spiRxBuf[0], g_spiRxBuf[1], g_spiRxBuf[2], g_spiRxBuf[3]);
            g_spiTxBuf[0] = count++;
            g_spiTxBuf[1] = count++;
            g_spiTxBuf[2] = count++;
            g_spiTxBuf[3] = count++;
            g_spiLen = 4;
            bsp_spiTransfer();
        }
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
