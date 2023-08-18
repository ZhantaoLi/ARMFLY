/*
*********************************************************************************************************
*
*	模块名称 : SPI总线驱动
*	文件名称 : bsp_spi_bus.c
*	版    本 : V1.3
*	说    明 : SPI总线底层驱动。提供SPI配置、收发数据、多设备共享SPI功能.
*	修改记录 :
*		版本号  日期        作者    说明
*       v1.0    2014-10-24 armfly   首版。将串行FLASH、TSC2046、VS1053、AD7705、ADS1256等SPI设备的配置
*									和收发数据的函数进行汇总分类。并解决不同速度的设备间的共享问题。
*		V1.1	2015-02-25 armfly   硬件SPI时，没有开启GPIOB时钟，已解决。
*		V1.2	2015-07-23 armfly   修改 bsp_SPI_Init() 函数，增加开关SPI时钟的语句。规范硬件SPI和软件SPI的宏定义。
*		V1.3	2020-03-14 Eric2013 适配STM32H7。
*
*	Copyright (C), 2020-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#include "bsp.h"

/*
	安富莱STM32-V7开发板口线分配
	PB3/SPI3_SCK/SPI1_SCK
	PB4/SPI3_MISO/SPI1_MISO
	PB5/SPI3_MOSI/SPI1_MOSI	
*/


/*
*********************************************************************************************************
*	                             选择DMA，中断或者查询方式
*********************************************************************************************************
*/
#define USE_SPI_DMA    /* DMA方式  */
//#define USE_SPI_INT    /* 中断方式 */
//#define USE_SPI_POLL   /* 查询方式 */


/*
*********************************************************************************************************
*	                            时钟，引脚，DMA，中断等宏定义
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

/* CS片选 */	
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
*	                                           变量
*********************************************************************************************************
*/
static SPI_HandleTypeDef hspi = {0};
static DMA_HandleTypeDef hdma_tx;
static DMA_HandleTypeDef hdma_rx;
static uint32_t s_BaudRatePrescaler;
static uint32_t s_CLKPhase;
static uint32_t s_CLKPolarity;
uint32_t g_spiLen;	
uint8_t  g_spi_busy; /* SPI忙状态，0表示不忙，1表示忙 */
__IO uint32_t wTransferState = TRANSFER_WAIT;


/* 查询模式 */
#if defined (USE_SPI_POLL)

uint8_t g_spiTxBuf[SPI_BUFFER_SIZE];  
uint8_t g_spiRxBuf[SPI_BUFFER_SIZE];

/* 中断模式 */
#elif defined (USE_SPI_INT)

uint8_t g_spiTxBuf[SPI_BUFFER_SIZE];   
uint8_t g_spiRxBuf[SPI_BUFFER_SIZE];

/* DMA模式使用的SRAM4 */
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
*	函 数 名: bsp_InitSPIBus
*	功能说明: 配置SPI总线。
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_InitSPIBus(void)
{	
	g_spi_busy = 0;
	
	bsp_InitSPIParam(SPI_BAUDRATEPRESCALER_4, SPI_PHASE_1EDGE, SPI_POLARITY_LOW);
}

/*
*********************************************************************************************************
*	函 数 名: bsp_InitSPIParam
*	功能说明: 配置SPI总线参数，时钟分频，时钟相位和时钟极性。
*	形    参: _BaudRatePrescaler  SPI总线时钟分频设置，支持的参数如下：
*                                 SPI_BAUDRATEPRESCALER_2    2分频
*                                 SPI_BAUDRATEPRESCALER_4    4分频
*                                 SPI_BAUDRATEPRESCALER_8    8分频
*                                 SPI_BAUDRATEPRESCALER_16   16分频
*                                 SPI_BAUDRATEPRESCALER_32   32分频
*                                 SPI_BAUDRATEPRESCALER_64   64分频
*                                 SPI_BAUDRATEPRESCALER_128  128分频
*                                 SPI_BAUDRATEPRESCALER_256  256分频
*                                                        
*             _CLKPhase           时钟相位，支持的参数如下：
*                                 SPI_PHASE_1EDGE     SCK引脚的第1个边沿捕获传输的第1个数据
*                                 SPI_PHASE_2EDGE     SCK引脚的第2个边沿捕获传输的第1个数据
*                                 
*             _CLKPolarity        时钟极性，支持的参数如下：
*                                 SPI_POLARITY_LOW    SCK引脚在空闲状态处于低电平
*                                 SPI_POLARITY_HIGH   SCK引脚在空闲状态处于高电平
*
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_InitSPIParam(uint32_t _BaudRatePrescaler, uint32_t _CLKPhase, uint32_t _CLKPolarity)
{
	/* 提高执行效率，只有在SPI硬件参数发生变化时，才执行HAL_Init */
	if (s_BaudRatePrescaler == _BaudRatePrescaler && s_CLKPhase == _CLKPhase && s_CLKPolarity == _CLKPolarity)
	{		
		return;
	}

	s_BaudRatePrescaler = _BaudRatePrescaler;	
	s_CLKPhase = _CLKPhase;
	s_CLKPolarity = _CLKPolarity;
	
	/* 设置SPI参数 */
	hspi.Instance               = SPIx;                   /* 例化SPI */
	hspi.Init.BaudRatePrescaler = _BaudRatePrescaler;     /* 设置波特率 */
	hspi.Init.Direction         = SPI_DIRECTION_2LINES;   /* 全双工 */
	hspi.Init.CLKPhase          = _CLKPhase;              /* 配置时钟相位 */
	hspi.Init.CLKPolarity       = _CLKPolarity;           /* 配置时钟极性 */
	hspi.Init.DataSize          = SPI_DATASIZE_8BIT;      /* 设置数据宽度 */
	hspi.Init.FirstBit          = SPI_FIRSTBIT_MSB;       /* 数据传输先传高位 */
	hspi.Init.TIMode            = SPI_TIMODE_DISABLE;     /* 禁止TI模式  */
	hspi.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLE; /* 禁止CRC */
	hspi.Init.CRCPolynomial     = 7;                       /* 禁止CRC后，此位无效 */
	hspi.Init.CRCLength         = SPI_CRC_LENGTH_8BIT;     /* 禁止CRC后，此位无效 */
	hspi.Init.NSS               = SPI_NSS_SOFT;               /* 使用软件方式管理片选引脚 */
	hspi.Init.FifoThreshold     = SPI_FIFO_THRESHOLD_16DATA;  /* 设置FIFO大小是一个数据项 */
	hspi.Init.NSSPMode          = SPI_NSS_PULSE_DISABLE;      /* 禁止脉冲输出 */
	hspi.Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_ENABLE; /* 禁止SPI后，SPI相关引脚保持当前状态 */  
	hspi.Init.Mode 			 	= SPI_MODE_SLAVE;                /* SPI工作在主控模式 */

//	/* 设置SPI参数 */
//	hspi.Instance               = SPIx;                   		/* 例化SPI */
//	hspi.Init.BaudRatePrescaler = _BaudRatePrescaler;     		/* 设置波特率 */
//	hspi.Init.Direction         = SPI_DIRECTION_2LINES;         /* 全双工 */
//	hspi.Init.CLKPhase          = _CLKPhase;             		/* 配置时钟相位 */
//	hspi.Init.CLKPolarity       = _CLKPolarity;           		/* 配置时钟极性 */
//	hspi.Init.DataSize          = SPI_DATASIZE_8BIT;      	 	/* 设置数据宽度 */
//	hspi.Init.FirstBit          = SPI_FIRSTBIT_MSB;         	/* 数据传输先传高位 */
//	hspi.Init.TIMode            = SPI_TIMODE_DISABLE;     		/* 禁止TI模式  */
//	hspi.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLE; 	/* 禁止CRC */
//	hspi.Init.CRCPolynomial     = 7;                       		/* 禁止CRC后，此位无效 */
//	hspi.Init.CRCLength         = SPI_CRC_LENGTH_8BIT;     		/* 禁止CRC后，此位无效 */
//	hspi.Init.FifoThreshold     = SPI_FIFO_THRESHOLD_16DATA;	/* 设置FIFO大小是一个数据项 */
//	
//	hspi.Init.NSS         = SPI_NSS_HARD_OUTPUT;         		/* 使用软件方式管理片选引脚 */
//	hspi.Init.NSSPMode    = SPI_NSS_PULSE_ENABLE;    			/* 使能脉冲输出 */
//	hspi.Init.NSSPolarity = SPI_NSS_POLARITY_LOW;               /* 低电平有效 */
//	hspi.Init.MasterSSIdleness        = SPI_MASTER_SS_IDLENESS_00CYCLE;        /* MSS, 插入到NSS有效边沿和第一个数据开始之间的额外延迟，单位SPI时钟周期个数 */
//	hspi.Init.MasterInterDataIdleness = SPI_MASTER_INTERDATA_IDLENESS_10CYCLE; /* MIDI, 两个连续数据帧之间插入的最小时间延迟，单位SPI时钟周期个数 */
//	
//	hspi.Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_ENABLE; /* 禁止SPI后，SPI相关引脚保持当前状态 */  
//	hspi.Init.Mode 			 	= SPI_MODE_SLAVE;          	   /* SPI工作在主控模式 */

	/* 复位配置 */
	if (HAL_SPI_DeInit(&hspi) != HAL_OK)
	{
		Error_Handler(__FILE__, __LINE__);
	}	

	/* 初始化配置 */
	if (HAL_SPI_Init(&hspi) != HAL_OK)
	{
		Error_Handler(__FILE__, __LINE__);
	}	
}

/*
*********************************************************************************************************
*	函 数 名: bsp_InitSPIParam
*	功能说明: 配置SPI总线时钟，GPIO，中断，DMA等
*	形    参: SPI_HandleTypeDef 类型指针变量
*	返 回 值: 无
*********************************************************************************************************
*/
void HAL_SPI_MspInit(SPI_HandleTypeDef *_hspi)
{
	/* 配置 SPI总线GPIO : SCK MOSI MISO */
	{
		GPIO_InitTypeDef  GPIO_InitStruct;
			
		/* SPI和GPIP时钟 */
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

	/* 配置DMA和NVIC */
	#ifdef USE_SPI_DMA
	{
		/* 使能DMA时钟 */
		DMAx_CLK_ENABLE();      

		/* SPI DMA发送配置 */		
		hdma_tx.Instance                 = SPIx_TX_DMA_STREAM;      /* 例化使用的DMA数据流 */
		hdma_tx.Init.FIFOMode            = DMA_FIFOMODE_ENABLE;     /* 禁止FIFO*/
		hdma_tx.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL; /* 禁止FIFO此位不起作用，用于设置阀值 */
		hdma_tx.Init.MemBurst            = DMA_MBURST_SINGLE;	    /* 禁止FIFO此位不起作用，用于存储器突发 */
		hdma_tx.Init.PeriphBurst         = DMA_PBURST_SINGLE;	    /* 禁止FIFO此位不起作用，用于外设突发 */
		hdma_tx.Init.Request             = SPIx_TX_DMA_REQUEST;     /* 请求类型 */  
		hdma_tx.Init.Direction           = DMA_MEMORY_TO_PERIPH;    /* 传输方向是从存储器到外设 */  
		hdma_tx.Init.PeriphInc           = DMA_PINC_DISABLE;        /* 外设地址自增禁止 */ 
		hdma_tx.Init.MemInc              = DMA_MINC_ENABLE;         /* 存储器地址自增使能 */  
		hdma_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;     /* 外设数据传输位宽选择字节，即8bit */ 
		hdma_tx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;     /* 存储器数据传输位宽选择字节，即8bit */    
		hdma_tx.Init.Mode                = DMA_NORMAL;              /* 正常模式 */
		hdma_tx.Init.Priority            = DMA_PRIORITY_HIGH;        /* 优先级低 */

		 /* 复位DMA */
		if(HAL_DMA_DeInit(&hdma_tx) != HAL_OK)
		{
			Error_Handler(__FILE__, __LINE__);     
		}
		
		 /* 初始化DMA */
		if(HAL_DMA_Init(&hdma_tx) != HAL_OK)
		{
			Error_Handler(__FILE__, __LINE__);     
		}

		/* 关联DMA句柄到SPI */
		__HAL_LINKDMA(_hspi, hdmatx, hdma_tx);	

		/* SPI DMA接收配置 */	
		hdma_rx.Instance                 = SPIx_RX_DMA_STREAM;     /* 例化使用的DMA数据流 */
		hdma_rx.Init.FIFOMode            = DMA_FIFOMODE_ENABLE;    /* FIFO*/
		hdma_rx.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;/* 禁止FIFO此位不起作用，用于设置阀值 */
		hdma_rx.Init.MemBurst            = DMA_MBURST_SINGLE;	   /* 禁止FIFO此位不起作用，用于存储器突发 */
		hdma_rx.Init.PeriphBurst         = DMA_PBURST_SINGLE;	   /* 禁止FIFO此位不起作用，用于外设突发 */
		hdma_rx.Init.Request             = SPIx_RX_DMA_REQUEST;    /* 请求类型 */  
		hdma_rx.Init.Direction           = DMA_PERIPH_TO_MEMORY;   /* 传输方向从外设到存储器 */  
		hdma_rx.Init.PeriphInc           = DMA_PINC_DISABLE;       /* 外设地址自增禁止 */   
		hdma_rx.Init.MemInc              = DMA_MINC_ENABLE;        /* 存储器地址自增使能 */   
		hdma_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;    /* 外设数据传输位宽选择字节，即8bit */ 
		hdma_rx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;    /* 存储器数据传输位宽选择字节，即8bit */   
		hdma_rx.Init.Mode                = DMA_NORMAL;             /* 正常模式 */
		hdma_rx.Init.Priority            = DMA_PRIORITY_LOW;      /* 优先级低 */

		 /* 复位DMA */
		if(HAL_DMA_DeInit(&hdma_rx) != HAL_OK)
		{
			Error_Handler(__FILE__, __LINE__);     
		}
		
		 /* 初始化DMA */
		if(HAL_DMA_Init(&hdma_rx) != HAL_OK)
		{
			Error_Handler(__FILE__, __LINE__);     
		}

		/* 关联DMA句柄到SPI */
	   __HAL_LINKDMA(_hspi, hdmarx, hdma_rx);	

		/* 配置DMA发送中断 */
		HAL_NVIC_SetPriority(SPIx_DMA_TX_IRQn, 1, 0);
		HAL_NVIC_EnableIRQ(SPIx_DMA_TX_IRQn);
		
		/* 配置DMA接收中断 */
		HAL_NVIC_SetPriority(SPIx_DMA_RX_IRQn, 1, 0);
		HAL_NVIC_EnableIRQ(SPIx_DMA_RX_IRQn);
		
		/* 配置SPI中断 */
		HAL_NVIC_SetPriority(SPIx_IRQn, 1, 0);
		HAL_NVIC_EnableIRQ(SPIx_IRQn);
	}
	#endif
	
	#ifdef USE_SPI_INT
		/* 配置SPI中断优先级并使能中断 */
		HAL_NVIC_SetPriority(SPIx_IRQn, 1, 0);
		HAL_NVIC_EnableIRQ(SPIx_IRQn);
	#endif
}
	
/*
*********************************************************************************************************
*	函 数 名: bsp_spiTransfer
*	功能说明: 启动数据传输
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_spiTransfer(void)
{
	if (g_spiLen > SPI_BUFFER_SIZE)
	{
		return;
	}
	
	/* DMA方式传输 */
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

	/* 中断方式传输 */	
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

	/* 查询方式传输 */	
#ifdef USE_SPI_POLL
	if(HAL_SPI_TransmitReceive(&hspi, (uint8_t*)g_spiTxBuf, (uint8_t *)g_spiRxBuf, g_spiLen, 1000000) != HAL_OK)	
	{
		Error_Handler(__FILE__, __LINE__);
	}	
#endif
}

/*
*********************************************************************************************************
*	函 数 名: HAL_SPI_TxRxCpltCallback，HAL_SPI_ErrorCallback
*	功能说明: SPI数据传输完成回调和传输错误回调
*	形    参: SPI_HandleTypeDef 类型指针变量
*	返 回 值: 无
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
*	函 数 名: bsp_SpiBusEnter
*	功能说明: 占用SPI总线
*	形    参: 无
*	返 回 值: 0 表示不忙  1表示忙
*********************************************************************************************************
*/
void bsp_SpiBusEnter(void)
{
	g_spi_busy = 1;
}

/*
*********************************************************************************************************
*	函 数 名: bsp_SpiBusExit
*	功能说明: 释放占用的SPI总线
*	形    参: 无
*	返 回 值: 0 表示不忙  1表示忙
*********************************************************************************************************
*/
void bsp_SpiBusExit(void)
{
	g_spi_busy = 0;
}

/*
*********************************************************************************************************
*	函 数 名: bsp_SpiBusBusy
*	功能说明: 判断SPI总线忙，方法是检测其他SPI芯片的片选信号是否为1
*	形    参: 无
*	返 回 值: 0 表示不忙  1表示忙
*********************************************************************************************************
*/
uint8_t bsp_SpiBusBusy(void)
{
	return g_spi_busy;
}

/*
*********************************************************************************************************
*	函 数 名: SPIx_IRQHandler，SPIx_DMA_RX_IRQHandler，SPIx_DMA_TX_IRQHandler
*	功能说明: 中断服务程序
*	形    参: 无
*	返 回 值: 无
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
	
/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
