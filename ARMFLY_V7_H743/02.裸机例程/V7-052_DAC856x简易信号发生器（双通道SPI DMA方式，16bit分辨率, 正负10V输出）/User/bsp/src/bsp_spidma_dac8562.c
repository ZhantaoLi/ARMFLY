/*
*********************************************************************************************************
*
*	ģ������ : DAC8562/8563 ����ģ��(˫ͨ����16λDAC)��ԭ����
*	�ļ����� : bsp_spidam_dac8562.c
*	��    �� : V1.0
*	˵    �� : DAC8562/8563ģ���CPU֮�����SPI�ӿڣ�����������֧��Ӳ��SPI DMA��ʽ��
*
*              �ر�ע�⣬����ļ��Ƕ�����SPI DMA������Ҫʹ��SPI1 NSS����Ӳ����ʽ����DAC8562��Ƭѡ��
*              ʹ���˴��ļ��Ͳ�����ʹ��bsp_spi_bus.c����SPI�����ļ�����Ϊ����ļ��������ʽ��Ƭѡ���ơ�
*
*	�޸ļ�¼ :
*		�汾��  ����         ����     ˵��
*		V1.0    2020-04-02  armfly  ��ʽ����
*
*	Copyright (C), 2020-2030, ���������� www.armfly.com
*
*********************************************************************************************************
*/
#include "bsp.h"



/*
	DAC8562ģ�����ֱ�Ӳ嵽STM32-V7������CN19��ĸ(2*4P 2.54mm)�ӿ���
	DAC8562/8563ģ�飺

	GND   ------  GND
	VCC   ------  3.3V

	SYNC  ------  PG10/SPI1_NSS
	SCLK  ------  PB3/SPI1_SCK
	DIN   ------  PB5/SPI1_MOSI

		  ------  PB4/SPI3_MISO               --- DAC�޶�������
	CLR   ------  PE4/NRF24L01_IRQ	          --- �Ƽ���GND��
	LDAC  ------  ��չIO/NRF24L01_CE/DAC1_OUT --- ����ӵػ�ָ��ǰ��0V

	DAC8562��������:
	1������2.7 - 5V;  ������ʹ��3.3V��
	2���ο���ѹ2.5V��ʹ���ڲ��ο�

	��SPI��ʱ���ٶ�Ҫ��: �ߴ�50MHz�� �ٶȺܿ�.
	SCLK�½��ض�ȡ����, ÿ�δ���24bit���ݣ� ��λ�ȴ�
*/

/* CLR */	
#define CLR_CLK_ENABLE() 	__HAL_RCC_GPIOE_CLK_ENABLE()
#define CLR_GPIO			GPIOE
#define CLR_PIN				GPIO_PIN_4
#define CLR_1()				CLR_GPIO->BSRR = CLR_PIN
#define CLR_0()				CLR_GPIO->BSRR = ((uint32_t)CLR_PIN << 16U)

/* LDAC ʹ����չIO */	
#define LDAC_1()			HC574_SetPin(NRF24L01_CE, 1);
#define LDAC_0()			HC574_SetPin(NRF24L01_CE, 0);

/* �����ѹ��DACֵ��Ĺ�ϵ�� ����У׼ x��dac y �ǵ�ѹ 0.1mV */
#define X1	0
#define Y1  -100000

#define X2	65535
#define Y2  100000

#define	SPI_BUFFER_SIZE		(4 * 1024)	


/*
*********************************************************************************************************
*	                            ʱ�ӣ����ţ�DMA���жϵȺ궨��
*********************************************************************************************************
*/
#define SPIx							SPI1
#define SPIx_CLK_ENABLE()				__HAL_RCC_SPI1_CLK_ENABLE()
#define DMAx_CLK_ENABLE()				__HAL_RCC_DMA2_CLK_ENABLE()

#define SPIx_FORCE_RESET()				__HAL_RCC_SPI1_FORCE_RESET()
#define SPIx_RELEASE_RESET()			__HAL_RCC_SPI1_RELEASE_RESET()

/* SYNC, Ҳ����CSƬѡ */	
#define SPIx_NSS_CLK_ENABLE() 			__HAL_RCC_GPIOG_CLK_ENABLE()
#define SPIx_NSS_GPIO					GPIOG
#define SPIx_NSS_PIN					GPIO_PIN_10
#define SPIx_NSS_AF						GPIO_AF5_SPI1

#define SPIx_SCK_CLK_ENABLE()			__HAL_RCC_GPIOB_CLK_ENABLE()
#define SPIx_SCK_GPIO					GPIOB
#define SPIx_SCK_PIN					GPIO_PIN_3
#define SPIx_SCK_AF						GPIO_AF5_SPI1

#define SPIx_MISO_CLK_ENABLE()			__HAL_RCC_GPIOB_CLK_ENABLE()
#define SPIx_MISO_GPIO					GPIOB
#define SPIx_MISO_PIN 					GPIO_PIN_4
#define SPIx_MISO_AF					GPIO_AF5_SPI1

#define SPIx_MOSI_CLK_ENABLE()			__HAL_RCC_GPIOB_CLK_ENABLE()
#define SPIx_MOSI_GPIO					GPIOB
#define SPIx_MOSI_PIN 					GPIO_PIN_5
#define SPIx_MOSI_AF					GPIO_AF5_SPI1

#define SPIx_TX_DMA_STREAM              DMA2_Stream3
#define SPIx_RX_DMA_STREAM              DMA2_Stream2

#define SPIx_TX_DMA_REQUEST             DMA_REQUEST_SPI1_TX
#define SPIx_RX_DMA_REQUEST             DMA_REQUEST_SPI1_RX

#define SPIx_DMA_TX_IRQn                DMA2_Stream3_IRQn
#define SPIx_DMA_RX_IRQn                DMA2_Stream2_IRQn

#define SPIx_DMA_TX_IRQHandler          DMA2_Stream3_IRQHandler
#define SPIx_DMA_RX_IRQHandler          DMA2_Stream2_IRQHandler

#define SPIx_IRQn                       SPI1_IRQn
#define SPIx_IRQHandler                 SPI1_IRQHandler

enum {
	TRANSFER_WAIT,
	TRANSFER_COMPLETE,
	TRANSFER_ERROR
};

/*
*********************************************************************************************************
*	                                           ����
*********************************************************************************************************
*/
static SPI_HandleTypeDef hspi = {0};
static DMA_HandleTypeDef hdma_tx;
static HAL_DMA_MuxSyncConfigTypeDef dmamux_syncParams;
static uint32_t g_spiLen;	
static __IO uint32_t wTransferState = TRANSFER_WAIT;
static uint8_t s_SpiDmaMode = 0;


#if defined ( __CC_ARM )    /* IAR *******/
	__attribute__((section (".RAM_D3"))) uint8_t g_spiTxBuf[SPI_BUFFER_SIZE];   
	__attribute__((section (".RAM_D3"))) uint8_t g_spiRxBuf[SPI_BUFFER_SIZE];
#elif defined (__ICCARM__)   /* MDK ********/
	#pragma location = ".RAM_D3"
	uint8_t g_spiTxBuf[SPI_BUFFER_SIZE];   
	#pragma location = ".RAM_D3"
	uint8_t g_spiRxBuf[SPI_BUFFER_SIZE];
#endif


/*
*********************************************************************************************************
*	�� �� ��: HAL_SPI_MspInit��HAL_SPI_MspDeInit
*	����˵��: ����SPI�ײ�ĳ�ʼ���͸�λ��ʼ��
*	��    ��: SPI_HandleTypeDef ����ָ�����
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void HAL_SPI_MspInit(SPI_HandleTypeDef *_hspi)
{
	GPIO_InitTypeDef  GPIO_InitStruct;
		
	/* SPI��GPIOʱ�� */
	SPIx_SCK_CLK_ENABLE();
	SPIx_MISO_CLK_ENABLE();
	SPIx_MOSI_CLK_ENABLE();
	SPIx_NSS_CLK_ENABLE();
	SPIx_CLK_ENABLE();
	
	/* ʹ��DMAʱ�� */
	DMAx_CLK_ENABLE();    

	/* SPI SCK */
	GPIO_InitStruct.Pin       = SPIx_SCK_PIN;
	GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull      = GPIO_PULLUP;
	GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_MEDIUM; 
	GPIO_InitStruct.Alternate = SPIx_SCK_AF;
	HAL_GPIO_Init(SPIx_SCK_GPIO, &GPIO_InitStruct);

	/* SPI MISO */
	GPIO_InitStruct.Pin = SPIx_MISO_PIN;
	GPIO_InitStruct.Alternate = SPIx_MISO_AF;
	HAL_GPIO_Init(SPIx_MISO_GPIO, &GPIO_InitStruct);

	/* SPI MOSI */
	GPIO_InitStruct.Pin = SPIx_MOSI_PIN;
	GPIO_InitStruct.Alternate = SPIx_MOSI_AF;
	HAL_GPIO_Init(SPIx_MOSI_GPIO, &GPIO_InitStruct);
	
	/* SPI NSS */
	GPIO_InitStruct.Pin = SPIx_NSS_PIN;
	GPIO_InitStruct.Alternate = SPIx_NSS_AF;
	HAL_GPIO_Init(SPIx_NSS_GPIO, &GPIO_InitStruct);
  

	/* SPI DMA�������� */		
	hdma_tx.Instance                 = SPIx_TX_DMA_STREAM;      /* ����ʹ�õ�DMA������ */
	hdma_tx.Init.FIFOMode            = DMA_FIFOMODE_ENABLE;     /* ʹ��FIFO*/
	hdma_tx.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL; /* �������÷�ֵ�� �����ֹFIFO��λ��������*/
	hdma_tx.Init.MemBurst            = DMA_MBURST_SINGLE;	    /* ���ڴ洢��ͻ���������ֹFIFO��λ��������*/
	hdma_tx.Init.PeriphBurst         = DMA_PBURST_SINGLE;	    /* ��������ͻ������ֹFIFO��λ�������� */
	hdma_tx.Init.Request             = SPIx_TX_DMA_REQUEST;     /* �������� */  
	hdma_tx.Init.Direction           = DMA_MEMORY_TO_PERIPH;    /* ���䷽���ǴӴ洢�������� */  
	hdma_tx.Init.PeriphInc           = DMA_PINC_DISABLE;        /* �����ַ������ֹ */ 
	hdma_tx.Init.MemInc              = DMA_MINC_ENABLE;         /* �洢����ַ����ʹ�� */  
	hdma_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;     /* �������ݴ���λ��ѡ���ֽڣ���8bit */ 
	hdma_tx.Init.MemDataAlignment    = DMA_MDATAALIGN_WORD;     /* �洢�����ݴ���λ��ѡ���ֽڣ���8bit */    
	hdma_tx.Init.Mode                = DMA_NORMAL;              /* ����ģʽ */
	hdma_tx.Init.Priority            = DMA_PRIORITY_LOW;        /* ���ȼ��� */
	
	 /* ��λDMA */
	if(HAL_DMA_DeInit(&hdma_tx) != HAL_OK)
	{
		Error_Handler(__FILE__, __LINE__);     
	}
	
	 /* ��ʼ��DMA */
	if(HAL_DMA_Init(&hdma_tx) != HAL_OK)
	{
		Error_Handler(__FILE__, __LINE__);     
	}
	
	/* ����DMA�����SPI */
	__HAL_LINKDMA(_hspi, hdmatx, hdma_tx);	
	

	/* ����DMA�����ж� */
	HAL_NVIC_SetPriority(SPIx_DMA_TX_IRQn, 1, 0);
	HAL_NVIC_EnableIRQ(SPIx_DMA_TX_IRQn);
	
	/* ����SPI�ж� */
	HAL_NVIC_SetPriority(SPIx_IRQn, 1, 0);
	HAL_NVIC_EnableIRQ(SPIx_IRQn);
}

void HAL_SPI_MspDeInit(SPI_HandleTypeDef *hspi)
{
	/*##-1- ��λSPI */
	SPIx_FORCE_RESET();
	SPIx_RELEASE_RESET();

	/*##-2- ��λӲ������ */
	HAL_GPIO_DeInit(SPIx_SCK_GPIO, SPIx_SCK_PIN);
	HAL_GPIO_DeInit(SPIx_MISO_GPIO, SPIx_MISO_PIN);
	HAL_GPIO_DeInit(SPIx_MOSI_GPIO, SPIx_MOSI_PIN);
	HAL_GPIO_DeInit(SPIx_NSS_GPIO, SPIx_NSS_PIN);

	/*##-3- ��ֹDMA TX */
	HAL_DMA_DeInit(&hdma_tx);

	/*##-4- ��ֹDMA NVIC */
	HAL_NVIC_DisableIRQ(SPIx_DMA_TX_IRQn);

	/*##-5- ��ֹSPI NVIC */
	HAL_NVIC_DisableIRQ(SPIx_IRQn);
}

/*
*********************************************************************************************************
*	�� �� ��: bsp_InitSPIParam
*	����˵��: ����SPI���߲�����ʱ�ӷ�Ƶ��ʱ����λ��ʱ�Ӽ��ԡ�
*	��    ��: _BaudRatePrescaler  SPI����ʱ�ӷ�Ƶ���ã�֧�ֵĲ������£�
*                                 SPI_BAUDRATEPRESCALER_2    2��Ƶ
*                                 SPI_BAUDRATEPRESCALER_4    4��Ƶ
*                                 SPI_BAUDRATEPRESCALER_8    8��Ƶ
*                                 SPI_BAUDRATEPRESCALER_16   16��Ƶ
*                                 SPI_BAUDRATEPRESCALER_32   32��Ƶ
*                                 SPI_BAUDRATEPRESCALER_64   64��Ƶ
*                                 SPI_BAUDRATEPRESCALER_128  128��Ƶ
*                                 SPI_BAUDRATEPRESCALER_256  256��Ƶ
*                                                        
*             _CLKPhase           ʱ����λ��֧�ֵĲ������£�
*                                 SPI_PHASE_1EDGE     SCK���ŵĵ�1�����ز�����ĵ�1������
*                                 SPI_PHASE_2EDGE     SCK���ŵĵ�2�����ز�����ĵ�1������
*                                 
*             _CLKPolarity        ʱ�Ӽ��ԣ�֧�ֵĲ������£�
*                                 SPI_POLARITY_LOW    SCK�����ڿ���״̬���ڵ͵�ƽ
*                                 SPI_POLARITY_HIGH   SCK�����ڿ���״̬���ڸߵ�ƽ
*
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void bsp_InitSPIParam(uint32_t _BaudRatePrescaler, uint32_t _CLKPhase, uint32_t _CLKPolarity)
{
	
	/* ����SPI���� */
	hspi.Instance               = SPIx;                   		/* ����SPI */
	hspi.Init.BaudRatePrescaler = _BaudRatePrescaler;     		/* ���ò����� */
	hspi.Init.Direction         = SPI_DIRECTION_2LINES_TXONLY;  /* ȫ˫�� */
	hspi.Init.CLKPhase          = _CLKPhase;             		/* ����ʱ����λ */
	hspi.Init.CLKPolarity       = _CLKPolarity;           		/* ����ʱ�Ӽ��� */
	hspi.Init.DataSize          = SPI_DATASIZE_24BIT;      	 	/* �������ݿ�� */
	hspi.Init.FirstBit          = SPI_FIRSTBIT_MSB;         	/* ���ݴ����ȴ���λ */
	hspi.Init.TIMode            = SPI_TIMODE_DISABLE;     		/* ��ֹTIģʽ  */
	hspi.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLE; 	/* ��ֹCRC */
	hspi.Init.CRCPolynomial     = 7;                       		/* ��ֹCRC�󣬴�λ��Ч */
	hspi.Init.CRCLength         = SPI_CRC_LENGTH_8BIT;     		/* ��ֹCRC�󣬴�λ��Ч */
	hspi.Init.FifoThreshold     = SPI_FIFO_THRESHOLD_05DATA;	/* ����FIFO��С��һ�������� */
	
	hspi.Init.NSS         = SPI_NSS_HARD_OUTPUT;         		/* ʹ�������ʽ����Ƭѡ���� */
	hspi.Init.NSSPMode    = SPI_NSS_PULSE_ENABLE;    			/* ʹ��������� */
	hspi.Init.NSSPolarity = SPI_NSS_POLARITY_LOW;               /* �͵�ƽ��Ч */
	hspi.Init.MasterSSIdleness        = SPI_MASTER_SS_IDLENESS_00CYCLE;        /* MSS, ���뵽NSS��Ч���غ͵�һ�����ݿ�ʼ֮��Ķ����ӳ٣���λSPIʱ�����ڸ��� */
	hspi.Init.MasterInterDataIdleness = SPI_MASTER_INTERDATA_IDLENESS_10CYCLE; /* MIDI, ������������֮֡��������Сʱ���ӳ٣���λSPIʱ�����ڸ��� */
	
	hspi.Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_ENABLE; /* ��ֹSPI��SPI������ű��ֵ�ǰ״̬ */  
	hspi.Init.Mode 			 	= SPI_MODE_MASTER;          	   /* SPI����������ģʽ */

	/* ��λ���� */
	if (HAL_SPI_DeInit(&hspi) != HAL_OK)
	{
		Error_Handler(__FILE__, __LINE__);
	}	

	/* ��ʼ������ */
	if (HAL_SPI_Init(&hspi) != HAL_OK)
	{
		Error_Handler(__FILE__, __LINE__);
	}	
}

/*
*********************************************************************************************************
*	�� �� ��: LPTIM_Config
*	����˵��: ����LPTIM�����ڴ���DMAMUX����������
*	��    ��: _ulFreq���Ƽ�Ƶ�ʷ�Χ��2KHz - 1MHz
*                      ��͵Ĵ���Ƶ�� 100MHz / 65535 = 1525Hz
*                      ��͵Ĵ���Ƶ�� 100MHz / 2 = 50MHz
*	�� �� ֵ: ��
*********************************************************************************************************
*/
#if 0
LPTIM_HandleTypeDef  LptimHandle = {0};
static void LPTIM_Config(uint32_t _ulFreq)
{
	uint16_t usPeriod;
	uint32_t uiTIMxCLK;
    RCC_PeriphCLKInitTypeDef  PeriphClkInitStruct;

    
    /*##-1- ����LPTIM1ʹ��PCLKʱ�� ##################################################*/
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LPTIM1;
    PeriphClkInitStruct.Lptim2ClockSelection = RCC_LPTIM1CLKSOURCE_PCLK1;
    HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);  

	
	/*-----------------------------------------------------------------------
		bsp.c �ļ��� void SystemClock_Config(void) ������ʱ�ӵ���������: 

        System Clock source       = PLL (HSE)
        SYSCLK(Hz)                = 400000000 (CPU Clock)
        HCLK(Hz)                  = 200000000 (AXI and AHBs Clock)
        AHB Prescaler             = 2
        D1 APB3 Prescaler         = 2 (APB3 Clock  100MHz)
        D2 APB1 Prescaler         = 2 (APB1 Clock  100MHz)
        D2 APB2 Prescaler         = 2 (APB2 Clock  100MHz)
        D3 APB4 Prescaler         = 2 (APB4 Clock  100MHz)

        ��ΪAPB1 prescaler != 1, ���� APB1�ϵ�TIMxCLK = APB1 x 2 = 200MHz; ������������µ�LPTIM1
        ��ΪAPB2 prescaler != 1, ���� APB2�ϵ�TIMxCLK = APB2 x 2 = 200MHz;
        APB4�����TIMxCLKû�з�Ƶ�����Ծ���100MHz;

        APB1 ��ʱ���� TIM2, TIM3 ,TIM4, TIM5, TIM6, TIM7, TIM12, TIM13, TIM14��LPTIM1
        APB2 ��ʱ���� TIM1, TIM8 , TIM15, TIM16��TIM17

        APB4 ��ʱ���� LPTIM2��LPTIM3��LPTIM4��LPTIM5
	----------------------------------------------------------------------- */
	
    /*##-2- ʹ��LPTIM1ʱ�Ӳ����� ####################################################*/
    __HAL_RCC_LPTIM1_CLK_ENABLE();
	
    LptimHandle.Instance                           = LPTIM1;
    LptimHandle.Init.CounterSource                 = LPTIM_COUNTERSOURCE_INTERNAL;     /* LPTIM���������ڲ�ʱ��Դ���� */
    LptimHandle.Init.UpdateMode                    = LPTIM_UPDATE_ENDOFPERIOD;         /* �ȽϼĴ�����ARR�Զ����ؼĴ���ѡ����ĺ��������� */ 
    LptimHandle.Init.OutputPolarity                = LPTIM_OUTPUTPOLARITY_HIGH;        /* �������������ȽϼĴ�����ARR�Զ����ؼĴ���֮����ֵ������ߵ�ƽ */
    LptimHandle.Init.Clock.Source                  = LPTIM_CLOCKSOURCE_APBCLOCK_LPOSC; /* ��Ӧ�Ĵ���CKSEL��ѡ���ڲ�ʱ��Դ */
    LptimHandle.Init.Clock.Prescaler               = LPTIM_PRESCALER_DIV1;             /* ����LPTIMʱ�ӷ�Ƶ */
    LptimHandle.Init.Trigger.Source                = LPTIM_TRIGSOURCE_SOFTWARE;        /* ������� */ 
    LptimHandle.Init.Trigger.ActiveEdge            = LPTIM_ACTIVEEDGE_RISING;
    LptimHandle.Init.Trigger.SampleTime            = LPTIM_TRIGSAMPLETIME_DIRECTTRANSITION;
	LptimHandle.Init.UltraLowPowerClock.Polarity   = LPTIM_CLOCKPOLARITY_RISING;       
    LptimHandle.Init.UltraLowPowerClock.SampleTime = LPTIM_CLOCKSAMPLETIME_DIRECTTRANSITION;

    /*##-3- ��ʼ��LPTIM1 ##########################################################*/
    if(HAL_LPTIM_Init(&LptimHandle) != HAL_OK)
    {
        Error_Handler(__FILE__, __LINE__);
    }
	
	/* ## - 4 - ����LPTIM1��PWMģʽ������ʹ��������ţ�������DMAMUX�Ĵ��� */	
	/*
	   ARR���Զ���װ�Ĵ�������Ӧ����HAL_LPTIM_PWM_Start�ĵ�2������
	   Compare�ǱȽϼĴ�������Ӧ����HAL_LPTIM_PWM_Start�ĵ�3������

	   ---------------------
	   ��Ƶ����ΪLPTIM_PRESCALER_DIV1����δ��Ƶ
	   ��ôPWMƵ�� = LPTIM1 / ��ARR + 1��
	   ռ�ձ� = 1 - (Comprare + 1)/ (ARR + 1)
	
	   ռ�ձ�����ΪʲôҪ1��������������ֱ�ӵ�(Comprare + 1)/ (ARR + 1)��������Ϊǰ���������
	   �������������ȽϼĴ�����ARR�Զ����ؼĴ���֮����ֵ������ߵ�ƽ��
	*/
	uiTIMxCLK = SystemCoreClock / 4;
	usPeriod = uiTIMxCLK / _ulFreq - 1;
    if (HAL_LPTIM_PWM_Start(&LptimHandle, usPeriod, usPeriod/2) != HAL_OK)
    {
        Error_Handler(__FILE__, __LINE__);
    }  
}
#endif

/*
*********************************************************************************************************
*	�� �� ��: TIM12_Config
*	����˵��: ����TIM12�����ڴ���DMAMUX����������
*	��    ��: _ulFreq  ����Ƶ�ʣ��Ƽ���Χ100Hz - 1MHz							  
*	�� �� ֵ: ��
*********************************************************************************************************
*/   
#if 1
TIM_HandleTypeDef  htim ={0};
TIM_MasterConfigTypeDef sMasterConfig = {0};
TIM_OC_InitTypeDef sConfig = {0};
void TIM12_Config(uint32_t _ulFreq)
{
	uint16_t usPeriod;
	uint16_t usPrescaler;
	uint32_t uiTIMxCLK;
	
	
  	/* ʹ��ʱ�� */  
  	__HAL_RCC_TIM12_CLK_ENABLE();
      
	/*-----------------------------------------------------------------------
		bsp.c �ļ��� void SystemClock_Config(void) ������ʱ�ӵ���������: 

        System Clock source       = PLL (HSE)
        SYSCLK(Hz)                = 400000000 (CPU Clock)
        HCLK(Hz)                  = 200000000 (AXI and AHBs Clock)
        AHB Prescaler             = 2
        D1 APB3 Prescaler         = 2 (APB3 Clock  100MHz)
        D2 APB1 Prescaler         = 2 (APB1 Clock  100MHz)
        D2 APB2 Prescaler         = 2 (APB2 Clock  100MHz)
        D3 APB4 Prescaler         = 2 (APB4 Clock  100MHz)

        ��ΪAPB1 prescaler != 1, ���� APB1�ϵ�TIMxCLK = APB1 x 2 = 200MHz; ������������µ�LPTIM1
        ��ΪAPB2 prescaler != 1, ���� APB2�ϵ�TIMxCLK = APB2 x 2 = 200MHz;
        APB4�����TIMxCLKû�з�Ƶ�����Ծ���100MHz;

        APB1 ��ʱ���� TIM2, TIM3 ,TIM4, TIM5, TIM6, TIM7, TIM12, TIM13, TIM14��LPTIM1
        APB2 ��ʱ���� TIM1, TIM8 , TIM15, TIM16��TIM17

        APB4 ��ʱ���� LPTIM2��LPTIM3��LPTIM4��LPTIM5
	----------------------------------------------------------------------- */
	uiTIMxCLK = SystemCoreClock / 2;
	
	if (_ulFreq < 100)
	{
		usPrescaler = 10000 - 1;					/* ��Ƶ�� = 10000 */
		usPeriod =  (uiTIMxCLK / 10000) / _ulFreq  - 1;		/* �Զ���װ��ֵ */
	}
	else if (_ulFreq < 3000)
	{
		usPrescaler = 100 - 1;					/* ��Ƶ�� = 100 */
		usPeriod =  (uiTIMxCLK / 100) / _ulFreq  - 1;		/* �Զ���װ��ֵ */
	}
	else	/* ����4K��Ƶ�ʣ������Ƶ */
	{
		usPrescaler = 0;					/* ��Ƶ�� = 1 */
		usPeriod = uiTIMxCLK / _ulFreq - 1;	/* �Զ���װ��ֵ */
	}
	
    htim.Instance = TIM12;
	htim.Init.Period            = usPeriod;
	htim.Init.Prescaler         = usPrescaler;
	htim.Init.ClockDivision     = 0;
	htim.Init.CounterMode       = TIM_COUNTERMODE_UP;
	htim.Init.RepetitionCounter = 0;

	if(HAL_TIM_Base_DeInit(&htim) != HAL_OK)
	{
		Error_Handler(__FILE__, __LINE__);		
	}
	
	if(HAL_TIM_Base_Init(&htim) != HAL_OK)
	{
		Error_Handler(__FILE__, __LINE__);		
	}
 
    sConfig.OCMode     = TIM_OCMODE_PWM1;
    sConfig.OCPolarity = TIM_OCPOLARITY_LOW;
    sConfig.Pulse = usPeriod / 2;     /* ռ�ձ�50% */
    if(HAL_TIM_OC_ConfigChannel(&htim, &sConfig, TIM_CHANNEL_1) != HAL_OK)
    {
		Error_Handler(__FILE__, __LINE__);
    }

    /* ����OC1 */
    if(HAL_TIM_OC_Start(&htim, TIM_CHANNEL_1) != HAL_OK)
    {
		Error_Handler(__FILE__, __LINE__);
    }
 
    /* TIM12��TRGO���ڴ���DMAMUX���������� */
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_OC1REF;
    sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	
	HAL_TIMEx_MasterConfigSynchronization(&htim, &sMasterConfig);
}
#endif

/*
*********************************************************************************************************
*	�� �� ��: bsp_spiDamStart
*	����˵��: ����SPI DMA����
*	��    ��: _ulFreq ��Χ�Ƽ�100Hz-1MHz
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void bsp_spiDamStart(uint32_t _ulFreq)
{
	/* ����ģʽ��Ҫ�л���DMA CIRCULARģʽ */
	s_SpiDmaMode = 1;
	
	bsp_InitSPIParam(SPI_BAUDRATEPRESCALER_4, SPI_PHASE_2EDGE, SPI_POLARITY_LOW);
	
	/* ʹ��DMAʱ�� */
	DMAx_CLK_ENABLE();      

	/* SPI DMA�������� */		
	hdma_tx.Instance                 = SPIx_TX_DMA_STREAM;      /* ����ʹ�õ�DMA������ */
	hdma_tx.Init.FIFOMode            = DMA_FIFOMODE_ENABLE;     /* ʹ��FIFO*/
	hdma_tx.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL; /* �������÷�ֵ�� �����ֹFIFO��λ��������*/
	hdma_tx.Init.MemBurst            = DMA_MBURST_SINGLE;	    /* ���ڴ洢��ͻ���������ֹFIFO��λ��������*/
	hdma_tx.Init.PeriphBurst         = DMA_PBURST_SINGLE;	    /* ��������ͻ������ֹFIFO��λ�������� */
	hdma_tx.Init.Request             = SPIx_TX_DMA_REQUEST;     /* �������� */  
	hdma_tx.Init.Direction           = DMA_MEMORY_TO_PERIPH;    /* ���䷽���ǴӴ洢�������� */  
	hdma_tx.Init.PeriphInc           = DMA_PINC_DISABLE;        /* �����ַ������ֹ */ 
	hdma_tx.Init.MemInc              = DMA_MINC_ENABLE;         /* �洢����ַ����ʹ�� */  
	hdma_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;     /* �������ݴ���λ��ѡ���ֽڣ���8bit */ 
	hdma_tx.Init.MemDataAlignment    = DMA_MDATAALIGN_WORD;     /* �洢�����ݴ���λ��ѡ���ֽڣ���8bit */    
	hdma_tx.Init.Mode                = DMA_CIRCULAR;            /* ����ģʽ */
	hdma_tx.Init.Priority            = DMA_PRIORITY_LOW;        /* ���ȼ��� */

	 /* ��λDMA */
	if(HAL_DMA_DeInit(&hdma_tx) != HAL_OK)
	{
		Error_Handler(__FILE__, __LINE__);     
	}
	
	 /* ��ʼ��DMA */
	if(HAL_DMA_Init(&hdma_tx) != HAL_OK)
	{
		Error_Handler(__FILE__, __LINE__);     
	}

	/* ����DMA�����SPI */
	__HAL_LINKDMA(&hspi, hdmatx, hdma_tx);	


	/* �ر�DMA�����ж� */
	HAL_NVIC_SetPriority(SPIx_DMA_TX_IRQn, 1, 0);
	HAL_NVIC_DisableIRQ(SPIx_DMA_TX_IRQn);
	
	/* �ر�SPI�ж� */
	HAL_NVIC_SetPriority(SPIx_IRQn, 1, 0);
	HAL_NVIC_DisableIRQ(SPIx_IRQn);

	/* ͬ���������� */
	dmamux_syncParams.EventEnable   = ENABLE; 							
	dmamux_syncParams.SyncPolarity  = HAL_DMAMUX_SYNC_RISING;          
	dmamux_syncParams.RequestNumber = 1;                   
	dmamux_syncParams.SyncSignalID  = HAL_DMAMUX1_SYNC_TIM12_TRGO; /* HAL_DMAMUX1_SYNC_TIM12_TRGO HAL_DMAMUX1_SYNC_LPTIM1_OUT*/
	dmamux_syncParams.SyncEnable    = ENABLE;    
	
	HAL_DMAEx_ConfigMuxSync(&hdma_tx, &dmamux_syncParams);
	
	//LPTIM_Config(_ulFreq);
	
	TIM12_Config(_ulFreq);
	
	/* ����DMA���� */
	if(HAL_SPI_Transmit_DMA(&hspi, (uint8_t*)g_spiTxBuf, g_spiLen/4)!= HAL_OK)	
	{
		Error_Handler(__FILE__, __LINE__);
	}
}
	
/*
*********************************************************************************************************
*	�� �� ��: bsp_spiTransfer
*	����˵��: �������ݴ���
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void bsp_spiTransfer(void)
{
	if (g_spiLen > SPI_BUFFER_SIZE)
	{
		Error_Handler(__FILE__, __LINE__);
	}
	
	/* ֮ǰ�õ���SPI DMA CIRCULARģʽ��Ҫ�л����� */
	if(s_SpiDmaMode == 1)
	{
		s_SpiDmaMode = 0;
		bsp_InitSPIParam(SPI_BAUDRATEPRESCALER_4, SPI_PHASE_2EDGE, SPI_POLARITY_LOW);
	}

	wTransferState = TRANSFER_WAIT;
	
	if(HAL_SPI_Transmit_DMA(&hspi, (uint8_t*)g_spiTxBuf, g_spiLen/4)!= HAL_OK)	
	{
		Error_Handler(__FILE__, __LINE__);
	}
	
	while (wTransferState == TRANSFER_WAIT)
	{
		;
	}
}

/*
*********************************************************************************************************
*	�� �� ��: HAL_SPI_TxRxCpltCallback��HAL_SPI_ErrorCallback
*	����˵��: SPI���ݴ�����ɻص��ʹ������ص�
*	��    ��: SPI_HandleTypeDef ����ָ�����
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
	wTransferState = TRANSFER_COMPLETE;
}

void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi)
{
	wTransferState = TRANSFER_ERROR;
}

/*
*********************************************************************************************************
*	�� �� ��: SPIx_IRQHandler��SPIx_DMA_TX_IRQHandler
*	����˵��: �жϷ������
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void SPIx_DMA_TX_IRQHandler(void)
{
	HAL_DMA_IRQHandler(hspi.hdmatx);
}

void SPIx_IRQHandler(void)
{
	HAL_SPI_IRQHandler(&hspi);
}	

/*
*********************************************************************************************************
*	�� �� ��: bsp_InitDAC8562
*	����˵��: ����GPIO����ʼ��DAC8562�Ĵ���
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void bsp_InitDAC8562(void)
{
	/* ����GPIO */
	GPIO_InitTypeDef GPIO_InitStruct;

	s_SpiDmaMode = 0;  
	
	/*##-1- ����SPI DMA ############################################################*/
	bsp_InitSPIParam(SPI_BAUDRATEPRESCALER_4, SPI_PHASE_2EDGE, SPI_POLARITY_LOW);
	
	/*##-2- ����CLR���� ############################################################*/
	CLR_CLK_ENABLE();
	
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;		/* ����������� */
	GPIO_InitStruct.Pull = GPIO_NOPULL;				/* ���������費ʹ�� */
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM; /* GPIO�ٶȵȼ� */	

	GPIO_InitStruct.Pin = CLR_PIN;	
	HAL_GPIO_Init(CLR_GPIO, &GPIO_InitStruct);			

	CLR_0();		/* CLR��GND�ɿ�һЩ��CLR���½��ش��� */
	LDAC_0();		/* �����첽����ģʽ�������Ž�GND */
	
	/*##-3- ����DAC8562 ############################################################*/
	/* Power up DAC-A and DAC-B */
	DAC8562_WriteCmd((4 << 19) | (0 << 16) | (3 << 0));

	/* LDAC pin inactive for DAC-B and DAC-A  ��ʹ��LDAC���Ÿ������� */
	DAC8562_WriteCmd((6 << 19) | (0 << 16) | (3 << 0));

	/* ��λ2��DAC���м�ֵ, ���0V */
	DAC8562_SetDacData(0, 32767);
	DAC8562_SetDacData(1, 32767);

	/* ѡ���ڲ��ο�����λ2��DAC������=2 ����λʱ���ڲ��ο��ǽ�ֹ��) */
	DAC8562_WriteCmd((7 << 19) | (0 << 16) | (1 << 0));
}

/*
*********************************************************************************************************
*	�� �� ��: DAC8562_SetDacDataDMA
*	����˵��: DAC8562���ݷ��ͣ�DMA��ʽ
*	��    ��: _ch         1��ʾͨ��1���
*                         2��ʾͨ��2���
*                         3��ʾͨ��1��2�����
*                         4��ʾͨ��1��2����������Ҹ���һ�����������Ч��ֹ�������ʱ�ָ���
*             _pbufch1    ͨ��1���ݻ����ַ
*             _pbufch2    ͨ��2���ݻ����ַ
*             _sizech1    ͨ��1���ݴ�С
*             _sizech2    ͨ��2���ݴ�С
*             _ulFreq     ����Ƶ�ʣ��Ƽ���Χ100Hz- 1MHz��ע����������Ǵ���Ƶ�ʣ������ǲ������ڡ�
*                         ���ﴥ��һ�Σ�SPI DMA����һ��24bit���ݡ�
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void DAC8562_SetDacDataDMA(uint8_t _ch, uint16_t *_pbufch1, uint16_t *_pbufch2, uint32_t _sizech1, uint32_t _sizech2, uint32_t _ulFreq)
{
	uint32_t i;
	uint32_t _cmd;
	
	g_spiLen = 0;
	
	switch (_ch)
	{
		/* ͨ��1���ݷ��� */
		case 1:
			for(i = 0; i < _sizech1; i++)
			{
				_cmd = (3 << 19) | (0 << 16) | (_pbufch1[i] << 0);
				
				g_spiTxBuf[g_spiLen++] = (uint8_t)(_cmd);
				g_spiTxBuf[g_spiLen++] = (uint8_t)(_cmd >> 8);
				g_spiTxBuf[g_spiLen++] = (uint8_t)(_cmd >> 16);
				g_spiTxBuf[g_spiLen++] = 0xff;
			}
			break;
	
		/* ͨ��2���ݷ��� */
		case 2:
			for(i = 0; i < _sizech2; i++)
			{
				_cmd = (3 << 19) | (1 << 16) | (_pbufch2[i] << 0);
				
				g_spiTxBuf[g_spiLen++] = (uint8_t)(_cmd);
				g_spiTxBuf[g_spiLen++] = (uint8_t)(_cmd >> 8);
				g_spiTxBuf[g_spiLen++] = (uint8_t)(_cmd >> 16);
				g_spiTxBuf[g_spiLen++] = 0xff;
			}
			break;

		/* ͨ��1��2��Ϸ��� */			
		case 3:
			if(_sizech1 == _sizech2)
			{
				
				for(i = 0; i < _sizech1; i++)
				{
					_cmd = (3 << 19) | (0 << 16) | (_pbufch1[i] << 0);
					
					g_spiTxBuf[g_spiLen++] = (uint8_t)(_cmd);
					g_spiTxBuf[g_spiLen++] = (uint8_t)(_cmd >> 8);
					g_spiTxBuf[g_spiLen++] = (uint8_t)(_cmd >> 16);
					g_spiTxBuf[g_spiLen++] = 0xff;
					
					_cmd = (3 << 19) | (1 << 16) | (_pbufch2[i] << 0);
					
					g_spiTxBuf[g_spiLen++] = (uint8_t)(_cmd);
					g_spiTxBuf[g_spiLen++] = (uint8_t)(_cmd >> 8);
					g_spiTxBuf[g_spiLen++] = (uint8_t)(_cmd >> 16);
					g_spiTxBuf[g_spiLen++] = 0xff;
				}			
			}
			else
			{
				for(i = 0; i < _sizech1; i++)
				{
					_cmd = (3 << 19) | (0 << 16) | (_pbufch1[i] << 0);
					
					g_spiTxBuf[g_spiLen++] = (uint8_t)(_cmd);
					g_spiTxBuf[g_spiLen++] = (uint8_t)(_cmd >> 8);
					g_spiTxBuf[g_spiLen++] = (uint8_t)(_cmd >> 16);
					g_spiTxBuf[g_spiLen++] = 0xff;
				}
				for(i = 0; i < _sizech2; i++)
				{
					_cmd = (3 << 19) | (1 << 16) | (_pbufch2[i] << 0);
					
					g_spiTxBuf[g_spiLen++] = (uint8_t)(_cmd);
					g_spiTxBuf[g_spiLen++] = (uint8_t)(_cmd >> 8);
					g_spiTxBuf[g_spiLen++] = (uint8_t)(_cmd >> 16);
					g_spiTxBuf[g_spiLen++] = 0xff;
				}
			}
			break;
			
		/* ����ؼ������ֹ������� */			
		case 4:
			if(_sizech1 == _sizech2)
			{
				
				for(i = 0; i < _sizech1; i++)
				{
					_cmd = (3 << 19) | (0 << 16) | (_pbufch1[i] << 0);
					
					g_spiTxBuf[g_spiLen++] = (uint8_t)(_cmd);
					g_spiTxBuf[g_spiLen++] = (uint8_t)(_cmd >> 8);
					g_spiTxBuf[g_spiLen++] = (uint8_t)(_cmd >> 16);
					g_spiTxBuf[g_spiLen++] = 0xff;
					
					_cmd = (3 << 19) | (1 << 16) | (_pbufch2[i] << 0);
					
					g_spiTxBuf[g_spiLen++] = (uint8_t)(_cmd);
					g_spiTxBuf[g_spiLen++] = (uint8_t)(_cmd >> 8);
					g_spiTxBuf[g_spiLen++] = (uint8_t)(_cmd >> 16);
					g_spiTxBuf[g_spiLen++] = 0xff;
				}			
			}
			else
			{
				for(i = 0; i < _sizech1; i++)
				{
					_cmd = (3 << 19) | (0 << 16) | (_pbufch1[i] << 0);
					
					g_spiTxBuf[g_spiLen++] = (uint8_t)(_cmd);
					g_spiTxBuf[g_spiLen++] = (uint8_t)(_cmd >> 8);
					g_spiTxBuf[g_spiLen++] = (uint8_t)(_cmd >> 16);
					g_spiTxBuf[g_spiLen++] = 0xff;
				}
				for(i = 0; i < _sizech2; i++)
				{
					_cmd = (3 << 19) | (1 << 16) | (_pbufch2[i] << 0);
					
					g_spiTxBuf[g_spiLen++] = (uint8_t)(_cmd);
					g_spiTxBuf[g_spiLen++] = (uint8_t)(_cmd >> 8);
					g_spiTxBuf[g_spiLen++] = (uint8_t)(_cmd >> 16);
					g_spiTxBuf[g_spiLen++] = 0xff;
				}
			}
			
			/* ���������Ϻ󣬲���ؼ����������������б�8256��ʶ��Ϊ�����*/
			_cmd = (7 << 19) | (0 << 16) | (1 << 0);
			
			g_spiTxBuf[g_spiLen++] = (uint8_t)(_cmd);
			g_spiTxBuf[g_spiLen++] = (uint8_t)(_cmd >> 8);
			g_spiTxBuf[g_spiLen++] = (uint8_t)(_cmd >> 16);
			g_spiTxBuf[g_spiLen++] = 0xff;
			break;
		
		default:
			break;

	}
	
	bsp_spiDamStart(_ulFreq);
}

/*
*********************************************************************************************************
*	�� �� ��: DAC8562_WriteCmd
*	����˵��: ��SPI���߷���24��bit���ݡ�
*	��    ��: _cmd : ����
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void DAC8562_WriteCmd(uint32_t _cmd)
{
	g_spiLen = 0;
	g_spiTxBuf[g_spiLen++] = (_cmd);
	g_spiTxBuf[g_spiLen++] = (_cmd >> 8);
	g_spiTxBuf[g_spiLen++] = (_cmd >> 16);
	g_spiTxBuf[g_spiLen++] = 0;
	bsp_spiTransfer();		
}

/*
*********************************************************************************************************
*	�� �� ��: DAC8562_SetDacData
*	����˵��: ����DAC��������������¡�
*	��    ��: _ch, ͨ��, 0 , 1
*		     _data : ����
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void DAC8562_SetDacData(uint8_t _ch, uint16_t _dac)
{
	if (_ch == 0)
	{
		/* д���ݵ�DAC-A�Ĵ���������DAC-A */
		DAC8562_WriteCmd((3 << 19) | (0 << 16) | (_dac << 0));
	}
	else if (_ch == 1)
	{
		/* д���ݵ�DAC-B�Ĵ���������DAC-B */
		DAC8562_WriteCmd((3 << 19) | (1 << 16) | (_dac << 0));
	}
}

/*
*********************************************************************************************************
*	�� �� ��: DAC8562_DacToVoltage
*	����˵��: ��DACֵ����Ϊ��ѹֵ����λ0.1mV
*	��    ��: _dac  16λDAC��
*	�� �� ֵ: ��ѹ����λ0.1mV
*********************************************************************************************************
*/
int32_t DAC8562_DacToVoltage(uint16_t _dac)
{
	int32_t y;

	/* CaculTwoPoint(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x);*/
	y =  CaculTwoPoint(X1, Y1, X2, Y2, _dac);
	return y;
}

/*
*********************************************************************************************************
*	�� �� ��: DAC8562_VoltageToDac
*	����˵��: ����ѹֵת��ΪDAC��
*	��    ��: _volt ��ѹ����λ0.1mV
*	�� �� ֵ: 16λDAC��
*********************************************************************************************************
*/
uint32_t DAC8562_VoltageToDac(int32_t _volt)
{
	/* CaculTwoPoint(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x);*/
	return CaculTwoPoint(Y1, X1, Y2, X2, _volt);
}

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
