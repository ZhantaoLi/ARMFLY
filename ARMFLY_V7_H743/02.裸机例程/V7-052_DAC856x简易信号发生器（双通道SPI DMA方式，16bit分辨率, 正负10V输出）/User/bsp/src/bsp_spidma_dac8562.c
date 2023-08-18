/*
*********************************************************************************************************
*
*	模块名称 : DAC8562/8563 驱动模块(双通道带16位DAC)【原创】
*	文件名称 : bsp_spidam_dac8562.c
*	版    本 : V1.0
*	说    明 : DAC8562/8563模块和CPU之间采用SPI接口，本驱动程序支持硬件SPI DMA方式。
*
*              特别注意，这个文件是独立的SPI DMA驱动，要使用SPI1 NSS引脚硬件方式驱动DAC8562的片选。
*              使用了此文件就不能再使用bsp_spi_bus.c共用SPI驱动文件，因为这个文件是软件方式的片选控制。
*
*	修改记录 :
*		版本号  日期         作者     说明
*		V1.0    2020-04-02  armfly  正式发布
*
*	Copyright (C), 2020-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/
#include "bsp.h"



/*
	DAC8562模块可以直接插到STM32-V7开发板CN19排母(2*4P 2.54mm)接口上
	DAC8562/8563模块：

	GND   ------  GND
	VCC   ------  3.3V

	SYNC  ------  PG10/SPI1_NSS
	SCLK  ------  PB3/SPI1_SCK
	DIN   ------  PB5/SPI1_MOSI

		  ------  PB4/SPI3_MISO               --- DAC无读出需求
	CLR   ------  PE4/NRF24L01_IRQ	          --- 推荐接GND。
	LDAC  ------  扩展IO/NRF24L01_CE/DAC1_OUT --- 必须接地或发指令前设0V

	DAC8562基本特性:
	1、供电2.7 - 5V;  【本例使用3.3V】
	2、参考电压2.5V，使用内部参考

	对SPI的时钟速度要求: 高达50MHz， 速度很快.
	SCLK下降沿读取数据, 每次传送24bit数据， 高位先传
*/

/* CLR */	
#define CLR_CLK_ENABLE() 	__HAL_RCC_GPIOE_CLK_ENABLE()
#define CLR_GPIO			GPIOE
#define CLR_PIN				GPIO_PIN_4
#define CLR_1()				CLR_GPIO->BSRR = CLR_PIN
#define CLR_0()				CLR_GPIO->BSRR = ((uint32_t)CLR_PIN << 16U)

/* LDAC 使用扩展IO */	
#define LDAC_1()			HC574_SetPin(NRF24L01_CE, 1);
#define LDAC_0()			HC574_SetPin(NRF24L01_CE, 0);

/* 定义电压和DAC值间的关系。 两点校准 x是dac y 是电压 0.1mV */
#define X1	0
#define Y1  -100000

#define X2	65535
#define Y2  100000

#define	SPI_BUFFER_SIZE		(4 * 1024)	


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

/* SYNC, 也就是CS片选 */	
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
*	                                           变量
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
*	函 数 名: HAL_SPI_MspInit和HAL_SPI_MspDeInit
*	功能说明: 用于SPI底层的初始化和复位初始化
*	形    参: SPI_HandleTypeDef 类型指针变量
*	返 回 值: 无
*********************************************************************************************************
*/
void HAL_SPI_MspInit(SPI_HandleTypeDef *_hspi)
{
	GPIO_InitTypeDef  GPIO_InitStruct;
		
	/* SPI和GPIO时钟 */
	SPIx_SCK_CLK_ENABLE();
	SPIx_MISO_CLK_ENABLE();
	SPIx_MOSI_CLK_ENABLE();
	SPIx_NSS_CLK_ENABLE();
	SPIx_CLK_ENABLE();
	
	/* 使能DMA时钟 */
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
  

	/* SPI DMA发送配置 */		
	hdma_tx.Instance                 = SPIx_TX_DMA_STREAM;      /* 例化使用的DMA数据流 */
	hdma_tx.Init.FIFOMode            = DMA_FIFOMODE_ENABLE;     /* 使能FIFO*/
	hdma_tx.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL; /* 用于设置阀值， 如果禁止FIFO此位不起作用*/
	hdma_tx.Init.MemBurst            = DMA_MBURST_SINGLE;	    /* 用于存储器突发，如果禁止FIFO此位不起作用*/
	hdma_tx.Init.PeriphBurst         = DMA_PBURST_SINGLE;	    /* 用于外设突发，禁止FIFO此位不起作用 */
	hdma_tx.Init.Request             = SPIx_TX_DMA_REQUEST;     /* 请求类型 */  
	hdma_tx.Init.Direction           = DMA_MEMORY_TO_PERIPH;    /* 传输方向是从存储器到外设 */  
	hdma_tx.Init.PeriphInc           = DMA_PINC_DISABLE;        /* 外设地址自增禁止 */ 
	hdma_tx.Init.MemInc              = DMA_MINC_ENABLE;         /* 存储器地址自增使能 */  
	hdma_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;     /* 外设数据传输位宽选择字节，即8bit */ 
	hdma_tx.Init.MemDataAlignment    = DMA_MDATAALIGN_WORD;     /* 存储器数据传输位宽选择字节，即8bit */    
	hdma_tx.Init.Mode                = DMA_NORMAL;              /* 正常模式 */
	hdma_tx.Init.Priority            = DMA_PRIORITY_LOW;        /* 优先级低 */
	
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
	

	/* 配置DMA发送中断 */
	HAL_NVIC_SetPriority(SPIx_DMA_TX_IRQn, 1, 0);
	HAL_NVIC_EnableIRQ(SPIx_DMA_TX_IRQn);
	
	/* 配置SPI中断 */
	HAL_NVIC_SetPriority(SPIx_IRQn, 1, 0);
	HAL_NVIC_EnableIRQ(SPIx_IRQn);
}

void HAL_SPI_MspDeInit(SPI_HandleTypeDef *hspi)
{
	/*##-1- 复位SPI */
	SPIx_FORCE_RESET();
	SPIx_RELEASE_RESET();

	/*##-2- 复位硬件配置 */
	HAL_GPIO_DeInit(SPIx_SCK_GPIO, SPIx_SCK_PIN);
	HAL_GPIO_DeInit(SPIx_MISO_GPIO, SPIx_MISO_PIN);
	HAL_GPIO_DeInit(SPIx_MOSI_GPIO, SPIx_MOSI_PIN);
	HAL_GPIO_DeInit(SPIx_NSS_GPIO, SPIx_NSS_PIN);

	/*##-3- 禁止DMA TX */
	HAL_DMA_DeInit(&hdma_tx);

	/*##-4- 禁止DMA NVIC */
	HAL_NVIC_DisableIRQ(SPIx_DMA_TX_IRQn);

	/*##-5- 禁止SPI NVIC */
	HAL_NVIC_DisableIRQ(SPIx_IRQn);
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
	
	/* 设置SPI参数 */
	hspi.Instance               = SPIx;                   		/* 例化SPI */
	hspi.Init.BaudRatePrescaler = _BaudRatePrescaler;     		/* 设置波特率 */
	hspi.Init.Direction         = SPI_DIRECTION_2LINES_TXONLY;  /* 全双工 */
	hspi.Init.CLKPhase          = _CLKPhase;             		/* 配置时钟相位 */
	hspi.Init.CLKPolarity       = _CLKPolarity;           		/* 配置时钟极性 */
	hspi.Init.DataSize          = SPI_DATASIZE_24BIT;      	 	/* 设置数据宽度 */
	hspi.Init.FirstBit          = SPI_FIRSTBIT_MSB;         	/* 数据传输先传高位 */
	hspi.Init.TIMode            = SPI_TIMODE_DISABLE;     		/* 禁止TI模式  */
	hspi.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLE; 	/* 禁止CRC */
	hspi.Init.CRCPolynomial     = 7;                       		/* 禁止CRC后，此位无效 */
	hspi.Init.CRCLength         = SPI_CRC_LENGTH_8BIT;     		/* 禁止CRC后，此位无效 */
	hspi.Init.FifoThreshold     = SPI_FIFO_THRESHOLD_05DATA;	/* 设置FIFO大小是一个数据项 */
	
	hspi.Init.NSS         = SPI_NSS_HARD_OUTPUT;         		/* 使用软件方式管理片选引脚 */
	hspi.Init.NSSPMode    = SPI_NSS_PULSE_ENABLE;    			/* 使能脉冲输出 */
	hspi.Init.NSSPolarity = SPI_NSS_POLARITY_LOW;               /* 低电平有效 */
	hspi.Init.MasterSSIdleness        = SPI_MASTER_SS_IDLENESS_00CYCLE;        /* MSS, 插入到NSS有效边沿和第一个数据开始之间的额外延迟，单位SPI时钟周期个数 */
	hspi.Init.MasterInterDataIdleness = SPI_MASTER_INTERDATA_IDLENESS_10CYCLE; /* MIDI, 两个连续数据帧之间插入的最小时间延迟，单位SPI时钟周期个数 */
	
	hspi.Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_ENABLE; /* 禁止SPI后，SPI相关引脚保持当前状态 */  
	hspi.Init.Mode 			 	= SPI_MODE_MASTER;          	   /* SPI工作在主控模式 */

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
*	函 数 名: LPTIM_Config
*	功能说明: 配置LPTIM，用于触发DMAMUX的请求发生器
*	形    参: _ulFreq，推荐频率范围是2KHz - 1MHz
*                      最低的触发频率 100MHz / 65535 = 1525Hz
*                      最低的触发频率 100MHz / 2 = 50MHz
*	返 回 值: 无
*********************************************************************************************************
*/
#if 0
LPTIM_HandleTypeDef  LptimHandle = {0};
static void LPTIM_Config(uint32_t _ulFreq)
{
	uint16_t usPeriod;
	uint32_t uiTIMxCLK;
    RCC_PeriphCLKInitTypeDef  PeriphClkInitStruct;

    
    /*##-1- 配置LPTIM1使用PCLK时钟 ##################################################*/
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LPTIM1;
    PeriphClkInitStruct.Lptim2ClockSelection = RCC_LPTIM1CLKSOURCE_PCLK1;
    HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);  

	
	/*-----------------------------------------------------------------------
		bsp.c 文件中 void SystemClock_Config(void) 函数对时钟的配置如下: 

        System Clock source       = PLL (HSE)
        SYSCLK(Hz)                = 400000000 (CPU Clock)
        HCLK(Hz)                  = 200000000 (AXI and AHBs Clock)
        AHB Prescaler             = 2
        D1 APB3 Prescaler         = 2 (APB3 Clock  100MHz)
        D2 APB1 Prescaler         = 2 (APB1 Clock  100MHz)
        D2 APB2 Prescaler         = 2 (APB2 Clock  100MHz)
        D3 APB4 Prescaler         = 2 (APB4 Clock  100MHz)

        因为APB1 prescaler != 1, 所以 APB1上的TIMxCLK = APB1 x 2 = 200MHz; 不含这个总线下的LPTIM1
        因为APB2 prescaler != 1, 所以 APB2上的TIMxCLK = APB2 x 2 = 200MHz;
        APB4上面的TIMxCLK没有分频，所以就是100MHz;

        APB1 定时器有 TIM2, TIM3 ,TIM4, TIM5, TIM6, TIM7, TIM12, TIM13, TIM14，LPTIM1
        APB2 定时器有 TIM1, TIM8 , TIM15, TIM16，TIM17

        APB4 定时器有 LPTIM2，LPTIM3，LPTIM4，LPTIM5
	----------------------------------------------------------------------- */
	
    /*##-2- 使能LPTIM1时钟并配置 ####################################################*/
    __HAL_RCC_LPTIM1_CLK_ENABLE();
	
    LptimHandle.Instance                           = LPTIM1;
    LptimHandle.Init.CounterSource                 = LPTIM_COUNTERSOURCE_INTERNAL;     /* LPTIM计数器对内部时钟源计数 */
    LptimHandle.Init.UpdateMode                    = LPTIM_UPDATE_ENDOFPERIOD;         /* 比较寄存器和ARR自动重载寄存器选择更改后立即更新 */ 
    LptimHandle.Init.OutputPolarity                = LPTIM_OUTPUTPOLARITY_HIGH;        /* 计数器计数到比较寄存器和ARR自动重载寄存器之间数值，输出高电平 */
    LptimHandle.Init.Clock.Source                  = LPTIM_CLOCKSOURCE_APBCLOCK_LPOSC; /* 对应寄存器CKSEL，选择内部时钟源 */
    LptimHandle.Init.Clock.Prescaler               = LPTIM_PRESCALER_DIV1;             /* 设置LPTIM时钟分频 */
    LptimHandle.Init.Trigger.Source                = LPTIM_TRIGSOURCE_SOFTWARE;        /* 软件触发 */ 
    LptimHandle.Init.Trigger.ActiveEdge            = LPTIM_ACTIVEEDGE_RISING;
    LptimHandle.Init.Trigger.SampleTime            = LPTIM_TRIGSAMPLETIME_DIRECTTRANSITION;
	LptimHandle.Init.UltraLowPowerClock.Polarity   = LPTIM_CLOCKPOLARITY_RISING;       
    LptimHandle.Init.UltraLowPowerClock.SampleTime = LPTIM_CLOCKSAMPLETIME_DIRECTTRANSITION;

    /*##-3- 初始化LPTIM1 ##########################################################*/
    if(HAL_LPTIM_Init(&LptimHandle) != HAL_OK)
    {
        Error_Handler(__FILE__, __LINE__);
    }
	
	/* ## - 4 - 启动LPTIM1的PWM模式，但不使用输出引脚，仅用于DMAMUX的触发 */	
	/*
	   ARR是自动重装寄存器，对应函数HAL_LPTIM_PWM_Start的第2个参数
	   Compare是比较寄存器，对应函数HAL_LPTIM_PWM_Start的第3个参数

	   ---------------------
	   分频设置为LPTIM_PRESCALER_DIV1，即未分频
	   那么PWM频率 = LPTIM1 / （ARR + 1）
	   占空比 = 1 - (Comprare + 1)/ (ARR + 1)
	
	   占空比这里为什么要1减操作，而不是直接的(Comprare + 1)/ (ARR + 1)，这是因为前面的配置中
	   计数器计数到比较寄存器和ARR自动重载寄存器之间数值，输出高电平。
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
*	函 数 名: TIM12_Config
*	功能说明: 配置TIM12，用于触发DMAMUX的请求发生器
*	形    参: _ulFreq  触发频率，推荐范围100Hz - 1MHz							  
*	返 回 值: 无
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
	
	
  	/* 使能时钟 */  
  	__HAL_RCC_TIM12_CLK_ENABLE();
      
	/*-----------------------------------------------------------------------
		bsp.c 文件中 void SystemClock_Config(void) 函数对时钟的配置如下: 

        System Clock source       = PLL (HSE)
        SYSCLK(Hz)                = 400000000 (CPU Clock)
        HCLK(Hz)                  = 200000000 (AXI and AHBs Clock)
        AHB Prescaler             = 2
        D1 APB3 Prescaler         = 2 (APB3 Clock  100MHz)
        D2 APB1 Prescaler         = 2 (APB1 Clock  100MHz)
        D2 APB2 Prescaler         = 2 (APB2 Clock  100MHz)
        D3 APB4 Prescaler         = 2 (APB4 Clock  100MHz)

        因为APB1 prescaler != 1, 所以 APB1上的TIMxCLK = APB1 x 2 = 200MHz; 不含这个总线下的LPTIM1
        因为APB2 prescaler != 1, 所以 APB2上的TIMxCLK = APB2 x 2 = 200MHz;
        APB4上面的TIMxCLK没有分频，所以就是100MHz;

        APB1 定时器有 TIM2, TIM3 ,TIM4, TIM5, TIM6, TIM7, TIM12, TIM13, TIM14，LPTIM1
        APB2 定时器有 TIM1, TIM8 , TIM15, TIM16，TIM17

        APB4 定时器有 LPTIM2，LPTIM3，LPTIM4，LPTIM5
	----------------------------------------------------------------------- */
	uiTIMxCLK = SystemCoreClock / 2;
	
	if (_ulFreq < 100)
	{
		usPrescaler = 10000 - 1;					/* 分频比 = 10000 */
		usPeriod =  (uiTIMxCLK / 10000) / _ulFreq  - 1;		/* 自动重装的值 */
	}
	else if (_ulFreq < 3000)
	{
		usPrescaler = 100 - 1;					/* 分频比 = 100 */
		usPeriod =  (uiTIMxCLK / 100) / _ulFreq  - 1;		/* 自动重装的值 */
	}
	else	/* 大于4K的频率，无需分频 */
	{
		usPrescaler = 0;					/* 分频比 = 1 */
		usPeriod = uiTIMxCLK / _ulFreq - 1;	/* 自动重装的值 */
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
    sConfig.Pulse = usPeriod / 2;     /* 占空比50% */
    if(HAL_TIM_OC_ConfigChannel(&htim, &sConfig, TIM_CHANNEL_1) != HAL_OK)
    {
		Error_Handler(__FILE__, __LINE__);
    }

    /* 启动OC1 */
    if(HAL_TIM_OC_Start(&htim, TIM_CHANNEL_1) != HAL_OK)
    {
		Error_Handler(__FILE__, __LINE__);
    }
 
    /* TIM12的TRGO用于触发DMAMUX的请求发生器 */
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_OC1REF;
    sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	
	HAL_TIMEx_MasterConfigSynchronization(&htim, &sMasterConfig);
}
#endif

/*
*********************************************************************************************************
*	函 数 名: bsp_spiDamStart
*	功能说明: 启动SPI DMA传输
*	形    参: _ulFreq 范围推荐100Hz-1MHz
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_spiDamStart(uint32_t _ulFreq)
{
	/* 设置模式，要切换到DMA CIRCULAR模式 */
	s_SpiDmaMode = 1;
	
	bsp_InitSPIParam(SPI_BAUDRATEPRESCALER_4, SPI_PHASE_2EDGE, SPI_POLARITY_LOW);
	
	/* 使能DMA时钟 */
	DMAx_CLK_ENABLE();      

	/* SPI DMA发送配置 */		
	hdma_tx.Instance                 = SPIx_TX_DMA_STREAM;      /* 例化使用的DMA数据流 */
	hdma_tx.Init.FIFOMode            = DMA_FIFOMODE_ENABLE;     /* 使能FIFO*/
	hdma_tx.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL; /* 用于设置阀值， 如果禁止FIFO此位不起作用*/
	hdma_tx.Init.MemBurst            = DMA_MBURST_SINGLE;	    /* 用于存储器突发，如果禁止FIFO此位不起作用*/
	hdma_tx.Init.PeriphBurst         = DMA_PBURST_SINGLE;	    /* 用于外设突发，禁止FIFO此位不起作用 */
	hdma_tx.Init.Request             = SPIx_TX_DMA_REQUEST;     /* 请求类型 */  
	hdma_tx.Init.Direction           = DMA_MEMORY_TO_PERIPH;    /* 传输方向是从存储器到外设 */  
	hdma_tx.Init.PeriphInc           = DMA_PINC_DISABLE;        /* 外设地址自增禁止 */ 
	hdma_tx.Init.MemInc              = DMA_MINC_ENABLE;         /* 存储器地址自增使能 */  
	hdma_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;     /* 外设数据传输位宽选择字节，即8bit */ 
	hdma_tx.Init.MemDataAlignment    = DMA_MDATAALIGN_WORD;     /* 存储器数据传输位宽选择字节，即8bit */    
	hdma_tx.Init.Mode                = DMA_CIRCULAR;            /* 正常模式 */
	hdma_tx.Init.Priority            = DMA_PRIORITY_LOW;        /* 优先级低 */

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
	__HAL_LINKDMA(&hspi, hdmatx, hdma_tx);	


	/* 关闭DMA发送中断 */
	HAL_NVIC_SetPriority(SPIx_DMA_TX_IRQn, 1, 0);
	HAL_NVIC_DisableIRQ(SPIx_DMA_TX_IRQn);
	
	/* 关闭SPI中断 */
	HAL_NVIC_SetPriority(SPIx_IRQn, 1, 0);
	HAL_NVIC_DisableIRQ(SPIx_IRQn);

	/* 同步触发配置 */
	dmamux_syncParams.EventEnable   = ENABLE; 							
	dmamux_syncParams.SyncPolarity  = HAL_DMAMUX_SYNC_RISING;          
	dmamux_syncParams.RequestNumber = 1;                   
	dmamux_syncParams.SyncSignalID  = HAL_DMAMUX1_SYNC_TIM12_TRGO; /* HAL_DMAMUX1_SYNC_TIM12_TRGO HAL_DMAMUX1_SYNC_LPTIM1_OUT*/
	dmamux_syncParams.SyncEnable    = ENABLE;    
	
	HAL_DMAEx_ConfigMuxSync(&hdma_tx, &dmamux_syncParams);
	
	//LPTIM_Config(_ulFreq);
	
	TIM12_Config(_ulFreq);
	
	/* 启动DMA传输 */
	if(HAL_SPI_Transmit_DMA(&hspi, (uint8_t*)g_spiTxBuf, g_spiLen/4)!= HAL_OK)	
	{
		Error_Handler(__FILE__, __LINE__);
	}
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
		Error_Handler(__FILE__, __LINE__);
	}
	
	/* 之前用的是SPI DMA CIRCULAR模式，要切换回来 */
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
*	函 数 名: HAL_SPI_TxRxCpltCallback，HAL_SPI_ErrorCallback
*	功能说明: SPI数据传输完成回调和传输错误回调
*	形    参: SPI_HandleTypeDef 类型指针变量
*	返 回 值: 无
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
*	函 数 名: SPIx_IRQHandler，SPIx_DMA_TX_IRQHandler
*	功能说明: 中断服务程序
*	形    参: 无
*	返 回 值: 无
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
*	函 数 名: bsp_InitDAC8562
*	功能说明: 配置GPIO并初始化DAC8562寄存器
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_InitDAC8562(void)
{
	/* 配置GPIO */
	GPIO_InitTypeDef GPIO_InitStruct;

	s_SpiDmaMode = 0;  
	
	/*##-1- 配置SPI DMA ############################################################*/
	bsp_InitSPIParam(SPI_BAUDRATEPRESCALER_4, SPI_PHASE_2EDGE, SPI_POLARITY_LOW);
	
	/*##-2- 配置CLR引脚 ############################################################*/
	CLR_CLK_ENABLE();
	
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;		/* 设置推挽输出 */
	GPIO_InitStruct.Pull = GPIO_NOPULL;				/* 上下拉电阻不使能 */
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM; /* GPIO速度等级 */	

	GPIO_InitStruct.Pin = CLR_PIN;	
	HAL_GPIO_Init(CLR_GPIO, &GPIO_InitStruct);			

	CLR_0();		/* CLR接GND可靠一些，CLR是下降沿触发 */
	LDAC_0();		/* 不用异步更新模式，此引脚接GND */
	
	/*##-3- 配置DAC8562 ############################################################*/
	/* Power up DAC-A and DAC-B */
	DAC8562_WriteCmd((4 << 19) | (0 << 16) | (3 << 0));

	/* LDAC pin inactive for DAC-B and DAC-A  不使用LDAC引脚更新数据 */
	DAC8562_WriteCmd((6 << 19) | (0 << 16) | (3 << 0));

	/* 复位2个DAC到中间值, 输出0V */
	DAC8562_SetDacData(0, 32767);
	DAC8562_SetDacData(1, 32767);

	/* 选择内部参考并复位2个DAC的增益=2 （复位时，内部参考是禁止的) */
	DAC8562_WriteCmd((7 << 19) | (0 << 16) | (1 << 0));
}

/*
*********************************************************************************************************
*	函 数 名: DAC8562_SetDacDataDMA
*	功能说明: DAC8562数据发送，DMA方式
*	形    参: _ch         1表示通道1输出
*                         2表示通道2输出
*                         3表示通道1和2都输出
*                         4表示通道1和2都输出，并且附加一个控制命令，有效防止传输错误时恢复。
*             _pbufch1    通道1数据缓冲地址
*             _pbufch2    通道2数据缓冲地址
*             _sizech1    通道1数据大小
*             _sizech2    通道2数据大小
*             _ulFreq     触发频率，推荐范围100Hz- 1MHz，注意这个参数是触发频率，并不是波形周期。
*                         这里触发一次，SPI DMA传输一次24bit数据。
*	返 回 值: 无
*********************************************************************************************************
*/
void DAC8562_SetDacDataDMA(uint8_t _ch, uint16_t *_pbufch1, uint16_t *_pbufch2, uint32_t _sizech1, uint32_t _sizech2, uint32_t _ulFreq)
{
	uint32_t i;
	uint32_t _cmd;
	
	g_spiLen = 0;
	
	switch (_ch)
	{
		/* 通道1数据发送 */
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
	
		/* 通道2数据发送 */
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

		/* 通道1和2混合发送 */			
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
			
		/* 插入关键命令，防止传输错误 */			
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
			
			/* 数据填充完毕后，插入关键命令，数据输出过程中被8256误识别为命令处理*/
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
*	函 数 名: DAC8562_WriteCmd
*	功能说明: 向SPI总线发送24个bit数据。
*	形    参: _cmd : 数据
*	返 回 值: 无
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
*	函 数 名: DAC8562_SetDacData
*	功能说明: 设置DAC输出，并立即更新。
*	形    参: _ch, 通道, 0 , 1
*		     _data : 数据
*	返 回 值: 无
*********************************************************************************************************
*/
void DAC8562_SetDacData(uint8_t _ch, uint16_t _dac)
{
	if (_ch == 0)
	{
		/* 写数据到DAC-A寄存器并更新DAC-A */
		DAC8562_WriteCmd((3 << 19) | (0 << 16) | (_dac << 0));
	}
	else if (_ch == 1)
	{
		/* 写数据到DAC-B寄存器并更新DAC-B */
		DAC8562_WriteCmd((3 << 19) | (1 << 16) | (_dac << 0));
	}
}

/*
*********************************************************************************************************
*	函 数 名: DAC8562_DacToVoltage
*	功能说明: 将DAC值换算为电压值，单位0.1mV
*	形    参: _dac  16位DAC字
*	返 回 值: 电压。单位0.1mV
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
*	函 数 名: DAC8562_VoltageToDac
*	功能说明: 将电压值转换为DAC置
*	形    参: _volt 电压，单位0.1mV
*	返 回 值: 16位DAC字
*********************************************************************************************************
*/
uint32_t DAC8562_VoltageToDac(int32_t _volt)
{
	/* CaculTwoPoint(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x);*/
	return CaculTwoPoint(Y1, X1, Y2, X2, _volt);
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
