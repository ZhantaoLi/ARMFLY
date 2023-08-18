/*
*********************************************************************************************************
*
*	ģ������ : SPI��������
*	�ļ����� : bsp_spi_bus.c
*	��    �� : V1.3
*	˵    �� : SPI���ߵײ��������ṩSPI���á��շ����ݡ����豸����SPI����.
*	�޸ļ�¼ :
*		�汾��  ����        ����    ˵��
*       v1.0    2014-10-24 armfly   �װ档������FLASH��TSC2046��VS1053��AD7705��ADS1256��SPI�豸������
*									���շ����ݵĺ������л��ܷ��ࡣ�������ͬ�ٶȵ��豸��Ĺ������⡣
*		V1.1	2015-02-25 armfly   Ӳ��SPIʱ��û�п���GPIOBʱ�ӣ��ѽ����
*		V1.2	2015-07-23 armfly   �޸� bsp_SPI_Init() ���������ӿ���SPIʱ�ӵ���䡣�淶Ӳ��SPI�����SPI�ĺ궨�塣
*		V1.3	2020-03-14 Eric2013 ����STM32H7��
*
*	Copyright (C), 2020-2030, ���������� www.armfly.com
*
*********************************************************************************************************
*/

#include "bsp.h"

/*
	������STM32-V7��������߷���
	PB3/SPI3_SCK/SPI1_SCK
	PB4/SPI3_MISO/SPI1_MISO
	PB5/SPI3_MOSI/SPI1_MOSI	
*/


/*
*********************************************************************************************************
*	                             ѡ��DMA���жϻ��߲�ѯ��ʽ
*********************************************************************************************************
*/
#define USE_SPI_DMA    /* DMA��ʽ  */
//#define USE_SPI_INT    /* �жϷ�ʽ */
//#define USE_SPI_POLL   /* ��ѯ��ʽ */


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

/* CSƬѡ */	
#define SPIx_NSS_CLK_ENABLE() 			__HAL_RCC_GPIOG_CLK_ENABLE()
#define SPIx_NSS_GPIO					GPIOG
#define SPIx_NSS_PIN					GPIO_PIN_10
#define SPIx_NSS_AF						GPIO_AF5_SPI1

#define SPIx_TX_DMA_STREAM               DMA2_Stream3
#define SPIx_RX_DMA_STREAM               DMA2_Stream2

#define SPIx_TX_DMA_REQUEST              DMA_REQUEST_SPI1_TX
#define SPIx_RX_DMA_REQUEST              DMA_REQUEST_SPI1_RX

#define SPIx_DMA_TX_IRQn                 DMA2_Stream3_IRQn
#define SPIx_DMA_RX_IRQn                 DMA2_Stream2_IRQn

#define SPIx_DMA_TX_IRQHandler           DMA2_Stream3_IRQHandler
#define SPIx_DMA_RX_IRQHandler           DMA2_Stream2_IRQHandler

#define SPIx_IRQn                        SPI1_IRQn
#define SPIx_IRQHandler                  SPI1_IRQHandler



/*
*********************************************************************************************************
*	                                           ����
*********************************************************************************************************
*/
static SPI_HandleTypeDef hspi = {0};
static DMA_HandleTypeDef hdma_tx;
static DMA_HandleTypeDef hdma_rx;
static uint32_t s_BaudRatePrescaler;
static uint32_t s_CLKPhase;
static uint32_t s_CLKPolarity;
uint32_t g_spiLen;	
uint8_t  g_spi_busy; /* SPIæ״̬��0��ʾ��æ��1��ʾæ */
__IO uint32_t wTransferState = TRANSFER_WAIT;


/* ��ѯģʽ */
#if defined (USE_SPI_POLL)

uint8_t g_spiTxBuf[SPI_BUFFER_SIZE];  
uint8_t g_spiRxBuf[SPI_BUFFER_SIZE];

/* �ж�ģʽ */
#elif defined (USE_SPI_INT)

uint8_t g_spiTxBuf[SPI_BUFFER_SIZE];   
uint8_t g_spiRxBuf[SPI_BUFFER_SIZE];

/* DMAģʽʹ�õ�SRAM4 */
#elif defined (USE_SPI_DMA)
    #if defined ( __CC_ARM )    /* IAR *******/
        __attribute__((section (".RAM_D3"))) uint8_t g_spiTxBuf[SPI_BUFFER_SIZE];   
        __attribute__((section (".RAM_D3"))) uint8_t g_spiRxBuf[SPI_BUFFER_SIZE];
    #elif defined (__ICCARM__)   /* MDK ********/
        #pragma location = ".RAM_D3"
        uint8_t g_spiTxBuf[SPI_BUFFER_SIZE];   
        #pragma location = ".RAM_D3"
        uint8_t g_spiRxBuf[SPI_BUFFER_SIZE];
    #endif
#endif


/*
*********************************************************************************************************
*	�� �� ��: bsp_InitSPIBus
*	����˵��: ����SPI���ߡ�
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void bsp_InitSPIBus(void)
{	
	g_spi_busy = 0;
	
	bsp_InitSPIParam(SPI_BAUDRATEPRESCALER_4, SPI_PHASE_1EDGE, SPI_POLARITY_LOW);
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
	/* ���ִ��Ч�ʣ�ֻ����SPIӲ�����������仯ʱ����ִ��HAL_Init */
	if (s_BaudRatePrescaler == _BaudRatePrescaler && s_CLKPhase == _CLKPhase && s_CLKPolarity == _CLKPolarity)
	{		
		return;
	}

	s_BaudRatePrescaler = _BaudRatePrescaler;	
	s_CLKPhase = _CLKPhase;
	s_CLKPolarity = _CLKPolarity;
	
	/* ����SPI���� */
	hspi.Instance               = SPIx;                   /* ����SPI */
	hspi.Init.BaudRatePrescaler = _BaudRatePrescaler;     /* ���ò����� */
	hspi.Init.Direction         = SPI_DIRECTION_2LINES;   /* ȫ˫�� */
	hspi.Init.CLKPhase          = _CLKPhase;              /* ����ʱ����λ */
	hspi.Init.CLKPolarity       = _CLKPolarity;           /* ����ʱ�Ӽ��� */
	hspi.Init.DataSize          = SPI_DATASIZE_8BIT;      /* �������ݿ�� */
	hspi.Init.FirstBit          = SPI_FIRSTBIT_MSB;       /* ���ݴ����ȴ���λ */
	hspi.Init.TIMode            = SPI_TIMODE_DISABLE;     /* ��ֹTIģʽ  */
	hspi.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLE; /* ��ֹCRC */
	hspi.Init.CRCPolynomial     = 7;                       /* ��ֹCRC�󣬴�λ��Ч */
	hspi.Init.CRCLength         = SPI_CRC_LENGTH_8BIT;     /* ��ֹCRC�󣬴�λ��Ч */
	hspi.Init.NSS               = SPI_NSS_SOFT;               /* ʹ�������ʽ����Ƭѡ���� */
	hspi.Init.FifoThreshold     = SPI_FIFO_THRESHOLD_16DATA;  /* ����FIFO��С��һ�������� */
	hspi.Init.NSSPMode          = SPI_NSS_PULSE_DISABLE;      /* ��ֹ������� */
	hspi.Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_ENABLE; /* ��ֹSPI��SPI������ű��ֵ�ǰ״̬ */  
	hspi.Init.Mode 			 	= SPI_MODE_SLAVE;                /* SPI����������ģʽ */

//	/* ����SPI���� */
//	hspi.Instance               = SPIx;                   		/* ����SPI */
//	hspi.Init.BaudRatePrescaler = _BaudRatePrescaler;     		/* ���ò����� */
//	hspi.Init.Direction         = SPI_DIRECTION_2LINES;         /* ȫ˫�� */
//	hspi.Init.CLKPhase          = _CLKPhase;             		/* ����ʱ����λ */
//	hspi.Init.CLKPolarity       = _CLKPolarity;           		/* ����ʱ�Ӽ��� */
//	hspi.Init.DataSize          = SPI_DATASIZE_8BIT;      	 	/* �������ݿ�� */
//	hspi.Init.FirstBit          = SPI_FIRSTBIT_MSB;         	/* ���ݴ����ȴ���λ */
//	hspi.Init.TIMode            = SPI_TIMODE_DISABLE;     		/* ��ֹTIģʽ  */
//	hspi.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLE; 	/* ��ֹCRC */
//	hspi.Init.CRCPolynomial     = 7;                       		/* ��ֹCRC�󣬴�λ��Ч */
//	hspi.Init.CRCLength         = SPI_CRC_LENGTH_8BIT;     		/* ��ֹCRC�󣬴�λ��Ч */
//	hspi.Init.FifoThreshold     = SPI_FIFO_THRESHOLD_16DATA;	/* ����FIFO��С��һ�������� */
//	
//	hspi.Init.NSS         = SPI_NSS_HARD_OUTPUT;         		/* ʹ�������ʽ����Ƭѡ���� */
//	hspi.Init.NSSPMode    = SPI_NSS_PULSE_ENABLE;    			/* ʹ��������� */
//	hspi.Init.NSSPolarity = SPI_NSS_POLARITY_LOW;               /* �͵�ƽ��Ч */
//	hspi.Init.MasterSSIdleness        = SPI_MASTER_SS_IDLENESS_00CYCLE;        /* MSS, ���뵽NSS��Ч���غ͵�һ�����ݿ�ʼ֮��Ķ����ӳ٣���λSPIʱ�����ڸ��� */
//	hspi.Init.MasterInterDataIdleness = SPI_MASTER_INTERDATA_IDLENESS_10CYCLE; /* MIDI, ������������֮֡��������Сʱ���ӳ٣���λSPIʱ�����ڸ��� */
//	
//	hspi.Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_ENABLE; /* ��ֹSPI��SPI������ű��ֵ�ǰ״̬ */  
//	hspi.Init.Mode 			 	= SPI_MODE_SLAVE;          	   /* SPI����������ģʽ */

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
*	�� �� ��: bsp_InitSPIParam
*	����˵��: ����SPI����ʱ�ӣ�GPIO���жϣ�DMA��
*	��    ��: SPI_HandleTypeDef ����ָ�����
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void HAL_SPI_MspInit(SPI_HandleTypeDef *_hspi)
{
	/* ���� SPI����GPIO : SCK MOSI MISO */
	{
		GPIO_InitTypeDef  GPIO_InitStruct;
			
		/* SPI��GPIPʱ�� */
		SPIx_SCK_CLK_ENABLE();
		SPIx_MISO_CLK_ENABLE();
		SPIx_MOSI_CLK_ENABLE();
		SPIx_CLK_ENABLE();

		/* SPI SCK */
		GPIO_InitStruct.Pin       = SPIx_SCK_PIN;
		GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull      = GPIO_PULLUP;
		GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
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
	}

	/* ����DMA��NVIC */
	#ifdef USE_SPI_DMA
	{
		/* ʹ��DMAʱ�� */
		DMAx_CLK_ENABLE();      

		/* SPI DMA�������� */		
		hdma_tx.Instance                 = SPIx_TX_DMA_STREAM;      /* ����ʹ�õ�DMA������ */
		hdma_tx.Init.FIFOMode            = DMA_FIFOMODE_ENABLE;     /* ��ֹFIFO*/
		hdma_tx.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL; /* ��ֹFIFO��λ�������ã��������÷�ֵ */
		hdma_tx.Init.MemBurst            = DMA_MBURST_SINGLE;	    /* ��ֹFIFO��λ�������ã����ڴ洢��ͻ�� */
		hdma_tx.Init.PeriphBurst         = DMA_PBURST_SINGLE;	    /* ��ֹFIFO��λ�������ã���������ͻ�� */
		hdma_tx.Init.Request             = SPIx_TX_DMA_REQUEST;     /* �������� */  
		hdma_tx.Init.Direction           = DMA_MEMORY_TO_PERIPH;    /* ���䷽���ǴӴ洢�������� */  
		hdma_tx.Init.PeriphInc           = DMA_PINC_DISABLE;        /* �����ַ������ֹ */ 
		hdma_tx.Init.MemInc              = DMA_MINC_ENABLE;         /* �洢����ַ����ʹ�� */  
		hdma_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;     /* �������ݴ���λ��ѡ���ֽڣ���8bit */ 
		hdma_tx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;     /* �洢�����ݴ���λ��ѡ���ֽڣ���8bit */    
		hdma_tx.Init.Mode                = DMA_NORMAL;              /* ����ģʽ */
		hdma_tx.Init.Priority            = DMA_PRIORITY_HIGH;        /* ���ȼ��� */

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

		/* SPI DMA�������� */	
		hdma_rx.Instance                 = SPIx_RX_DMA_STREAM;     /* ����ʹ�õ�DMA������ */
		hdma_rx.Init.FIFOMode            = DMA_FIFOMODE_ENABLE;    /* FIFO*/
		hdma_rx.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;/* ��ֹFIFO��λ�������ã��������÷�ֵ */
		hdma_rx.Init.MemBurst            = DMA_MBURST_SINGLE;	   /* ��ֹFIFO��λ�������ã����ڴ洢��ͻ�� */
		hdma_rx.Init.PeriphBurst         = DMA_PBURST_SINGLE;	   /* ��ֹFIFO��λ�������ã���������ͻ�� */
		hdma_rx.Init.Request             = SPIx_RX_DMA_REQUEST;    /* �������� */  
		hdma_rx.Init.Direction           = DMA_PERIPH_TO_MEMORY;   /* ���䷽������赽�洢�� */  
		hdma_rx.Init.PeriphInc           = DMA_PINC_DISABLE;       /* �����ַ������ֹ */   
		hdma_rx.Init.MemInc              = DMA_MINC_ENABLE;        /* �洢����ַ����ʹ�� */   
		hdma_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;    /* �������ݴ���λ��ѡ���ֽڣ���8bit */ 
		hdma_rx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;    /* �洢�����ݴ���λ��ѡ���ֽڣ���8bit */   
		hdma_rx.Init.Mode                = DMA_NORMAL;             /* ����ģʽ */
		hdma_rx.Init.Priority            = DMA_PRIORITY_LOW;      /* ���ȼ��� */

		 /* ��λDMA */
		if(HAL_DMA_DeInit(&hdma_rx) != HAL_OK)
		{
			Error_Handler(__FILE__, __LINE__);     
		}
		
		 /* ��ʼ��DMA */
		if(HAL_DMA_Init(&hdma_rx) != HAL_OK)
		{
			Error_Handler(__FILE__, __LINE__);     
		}

		/* ����DMA�����SPI */
	   __HAL_LINKDMA(_hspi, hdmarx, hdma_rx);	

		/* ����DMA�����ж� */
		HAL_NVIC_SetPriority(SPIx_DMA_TX_IRQn, 1, 0);
		HAL_NVIC_EnableIRQ(SPIx_DMA_TX_IRQn);
		
		/* ����DMA�����ж� */
		HAL_NVIC_SetPriority(SPIx_DMA_RX_IRQn, 1, 0);
		HAL_NVIC_EnableIRQ(SPIx_DMA_RX_IRQn);
		
		/* ����SPI�ж� */
		HAL_NVIC_SetPriority(SPIx_IRQn, 1, 0);
		HAL_NVIC_EnableIRQ(SPIx_IRQn);
	}
	#endif
	
	#ifdef USE_SPI_INT
		/* ����SPI�ж����ȼ���ʹ���ж� */
		HAL_NVIC_SetPriority(SPIx_IRQn, 1, 0);
		HAL_NVIC_EnableIRQ(SPIx_IRQn);
	#endif
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
		return;
	}
	
	/* DMA��ʽ���� */
#ifdef USE_SPI_DMA
	wTransferState = TRANSFER_WAIT;
	
    if(HAL_SPI_TransmitReceive_DMA(&hspi, (uint8_t*)g_spiTxBuf, (uint8_t *)g_spiRxBuf, g_spiLen) != HAL_OK)	
    {
        Error_Handler(__FILE__, __LINE__);
    }
	
//	while (wTransferState == TRANSFER_WAIT)
//	{
//		;
//	}
#endif

	/* �жϷ�ʽ���� */	
#ifdef USE_SPI_INT
	wTransferState = TRANSFER_WAIT;

    if(HAL_SPI_TransmitReceive_IT(&hspi, (uint8_t*)g_spiTxBuf, (uint8_t *)g_spiRxBuf, g_spiLen) != HAL_OK)	
    {
        Error_Handler(__FILE__, __LINE__);
    }
	
//	while (wTransferState == TRANSFER_WAIT)
//	{
//		;
//	}
#endif

	/* ��ѯ��ʽ���� */	
#ifdef USE_SPI_POLL
	if(HAL_SPI_TransmitReceive(&hspi, (uint8_t*)g_spiTxBuf, (uint8_t *)g_spiRxBuf, g_spiLen, 1000000) != HAL_OK)	
	{
		Error_Handler(__FILE__, __LINE__);
	}	
#endif
}

/*
*********************************************************************************************************
*	�� �� ��: HAL_SPI_TxRxCpltCallback��HAL_SPI_ErrorCallback
*	����˵��: SPI���ݴ�����ɻص��ʹ������ص�
*	��    ��: SPI_HandleTypeDef ����ָ�����
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
	wTransferState = TRANSFER_COMPLETE;
}


void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi)
{
	wTransferState = TRANSFER_ERROR;
}

/*
*********************************************************************************************************
*	�� �� ��: bsp_SpiBusEnter
*	����˵��: ռ��SPI����
*	��    ��: ��
*	�� �� ֵ: 0 ��ʾ��æ  1��ʾæ
*********************************************************************************************************
*/
void bsp_SpiBusEnter(void)
{
	g_spi_busy = 1;
}

/*
*********************************************************************************************************
*	�� �� ��: bsp_SpiBusExit
*	����˵��: �ͷ�ռ�õ�SPI����
*	��    ��: ��
*	�� �� ֵ: 0 ��ʾ��æ  1��ʾæ
*********************************************************************************************************
*/
void bsp_SpiBusExit(void)
{
	g_spi_busy = 0;
}

/*
*********************************************************************************************************
*	�� �� ��: bsp_SpiBusBusy
*	����˵��: �ж�SPI����æ�������Ǽ������SPIоƬ��Ƭѡ�ź��Ƿ�Ϊ1
*	��    ��: ��
*	�� �� ֵ: 0 ��ʾ��æ  1��ʾæ
*********************************************************************************************************
*/
uint8_t bsp_SpiBusBusy(void)
{
	return g_spi_busy;
}

/*
*********************************************************************************************************
*	�� �� ��: SPIx_IRQHandler��SPIx_DMA_RX_IRQHandler��SPIx_DMA_TX_IRQHandler
*	����˵��: �жϷ������
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
#ifdef USE_SPI_INT
	void SPIx_IRQHandler(void)
	{
		HAL_SPI_IRQHandler(&hspi);
	}	
#endif

#ifdef USE_SPI_DMA
	void SPIx_DMA_RX_IRQHandler(void)
	{
		HAL_DMA_IRQHandler(hspi.hdmarx);
	}

	void SPIx_DMA_TX_IRQHandler(void)
	{
		HAL_DMA_IRQHandler(hspi.hdmatx);
	}
	
	void SPIx_IRQHandler(void)
	{
		HAL_SPI_IRQHandler(&hspi);
	}	
#endif
	
/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
