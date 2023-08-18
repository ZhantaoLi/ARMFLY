/*
*********************************************************************************************************
*
*	ģ������ : SPI ����
*	�ļ����� : demo_spi_master.c
*	��    �� : V1.0
*	˵    �� : SPI ����
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
#include "demo_spi_master.h"
#include "bsp.h"


static void sfDispMenu(void);

/*
*********************************************************************************************************
*	�� �� ��: DemoSpiMaster
*	����˵��: SPI ����ͨ��
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void DemoSpiMaster(void)
{
	uint8_t count = 0;
    uint8_t ucKeyCode;		/* �������� */
    
    /***************����SPI FlashƬѡ��������ֹӰ�� ***************/
    {
        GPIO_InitTypeDef gpio_init;

        /* ��GPIOʱ�� */
        __HAL_RCC_GPIOD_CLK_ENABLE();
        
        gpio_init.Mode = GPIO_MODE_OUTPUT_PP;	
        gpio_init.Pull = GPIO_NOPULL;		
        gpio_init.Speed = GPIO_SPEED_HIGH;  	
        gpio_init.Pin = GPIO_PIN_13;	
        HAL_GPIO_Init(GPIOD, &gpio_init);

        GPIOD->BSRR = GPIO_PIN_13;
    }
    
	sfDispMenu();		/* ��ӡ������ʾ */
	
	bsp_StartAutoTimer(0, 100);	/* ����1��100ms���Զ���װ�Ķ�ʱ�� */
	
	while(1)
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
				case KEY_DOWN_K1:			/* K1�����£��������ݸ��ӻ�*/
                    g_spiTxBuf[0] = count++;
                    g_spiTxBuf[1] = count++;
                    g_spiTxBuf[2] = count++;
                    g_spiTxBuf[3] = count++;
                    g_spiLen = 4;
                    printf("SPI�����������ݣ�%d,%d,%d,%d\r\n", g_spiTxBuf[0],g_spiTxBuf[1],g_spiTxBuf[2],g_spiTxBuf[3]);
                    bsp_spiTransfer();
                    printf("SPI�����������ݣ�%d,%d,%d,%d\r\n", g_spiRxBuf[0],g_spiRxBuf[1],g_spiRxBuf[2],g_spiRxBuf[3]);
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
*	�� �� ��: sfDispMenu
*	����˵��: ��ʾ������ʾ�˵�
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void sfDispMenu(void)
{
	printf("SPI��������\r\n");
	printf("K1�������£�SPI��������SPIȫ˫��ͨ��\r\n");
}

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
