/*
*********************************************************************************************************
*
*	模块名称 : AD7606数据采集模块【原创】
*	文件名称 : bsp_fmcdma_ad7606.c
*	版    本 : V1.0
*	说    明 : AD7606 FMC DMA采集方式，支持双缓冲
*
*	修改记录 :
*		版本号  日期        作者     说明
*		V1.0    2020-05-24 armfly  正式发布
*
*	Copyright (C), 2020-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/
#include "bsp.h"



/*
	STM32-V7开发板 + AD7606模块， 控制采集的IO:
	
	PC6/TIM3_CH1/TIM8_CH1     ----> AD7606_CONVST  (和摄像头复用),  输出PWM方波，作为ADC启动信号
	PE5/DCMI_D6/AD7606_BUSY   <---- AD7606_BUSY    , CPU在BUSY中断服务程序中读取采集结果
*/
/* CONVST 启动ADC转换的GPIO = PC6 */
#define CONVST_RCC_GPIO_CLK_ENABLE	__HAL_RCC_GPIOC_CLK_ENABLE
#define CONVST_TIM8_CLK_ENABLE      __HAL_RCC_TIM8_CLK_ENABLE
#define CONVST_RCC_GPIO_CLK_DISBALE	__HAL_RCC_GPIOC_CLK_DISABLE
#define CONVST_TIM8_CLK_DISABLE     __HAL_RCC_TIM8_CLK_DISABLE
#define CONVST_GPIO		GPIOC
#define CONVST_PIN		GPIO_PIN_6
#define CONVST_AF		GPIO_AF3_TIM8
#define CONVST_TIMX		TIM8
#define CONVST_TIMCH	TIM_CHANNEL_1

/* FMC DMA */
#define TIMx_UP_DMA_STREAM_CLK_ENABLE  	__HAL_RCC_DMA2_CLK_ENABLE
#define TIMx_UP_DMA_STREAM_CLK_DISABLE  __HAL_RCC_DMA2_CLK_DISABLE
#define TIMx_UP_DMA_STREAM             DMA2_Stream1
#define TIMx_UP_DMA_REQUEST            DMA_REQUEST_TIM8_UP
#define TIMx_UP_DMA_IRQn               DMA2_Stream1_IRQn
#define TIMx_UP_DMA_IRQHandler         DMA2_Stream1_IRQHandler

/* 设置过采样的IO, 在扩展的74HC574上 */
#define OS0_1()		HC574_SetPin(AD7606_OS0, 1)
#define OS0_0()		HC574_SetPin(AD7606_OS0, 0)
#define OS1_1()		HC574_SetPin(AD7606_OS1, 1)
#define OS1_0()		HC574_SetPin(AD7606_OS1, 0)
#define OS2_1()		HC574_SetPin(AD7606_OS2, 1)
#define OS2_0()		HC574_SetPin(AD7606_OS2, 0)

/* 启动AD转换的GPIO : PC6 */
#define CONVST_1()		CONVST_GPIO->BSRR = CONVST_PIN
#define CONVST_0()		CONVST_GPIO->BSRR = ((uint32_t)CONVST_PIN << 16U)

/* 设置输入量程的GPIO, 在扩展的74HC574上 */
#define RANGE_1()	HC574_SetPin(AD7606_RANGE, 1)
#define RANGE_0()	HC574_SetPin(AD7606_RANGE, 0)

/* AD7606复位口线, 在扩展的74HC574上 */
#define RESET_1()	HC574_SetPin(AD7606_RESET, 1)
#define RESET_0()	HC574_SetPin(AD7606_RESET, 0)

/* AD7606 FSMC总线地址，只能读，无需写 */
#define AD7606_BASE    	0x60003000

static DMA_HandleTypeDef TIMDMA = {0};
static TIM_HandleTypeDef TimHandle = {0};


/* 8路同步采集，每次采集16字节数据，防止DMA突发方式1KB边界问题，即每次采集不要有跨边界的情况 */
#define AD7606_BUFSIZE        16
__align(16) int16_t g_sAd7606Buf[AD7606_BUFSIZE];   

AD7606_VAR_T g_tAD7606;		/* 定义1个全局变量，保存一些参数 */

static void AD7606_CtrlLinesConfig(void);
static void AD7606_FSMCConfig(void);
static void AD7606_SetTIMOutPWM(TIM_TypeDef* TIMx, uint32_t _ulFreq);

/*
*********************************************************************************************************
*	函 数 名: bsp_InitAD7606
*	功能说明: 配置连接外部SRAM的GPIO和FSMC
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_InitAD7606(void)
{
	AD7606_CtrlLinesConfig();
	AD7606_FSMCConfig();

	AD7606_SetOS(AD_OS_NO);		/* 无过采样 */
	AD7606_SetInputRange(0);	/* 0表示输入量程为正负5V, 1表示正负10V */
	AD7606_Reset();				/* 复位 */
	CONVST_1();					/* 启动转换的GPIO，平时设置为高 */
}

/*
*********************************************************************************************************
*	函 数 名: AD7606_CtrlLinesConfig
*	功能说明: 配置GPIO口线，FMC管脚设置为复用功能
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
/*
	安富莱STM32-H7开发板接线方法：4片74HC574挂在FMC 32位总线上。1个地址端口可以扩展出32个IO
	PD0/FMC_D2
	PD1/FMC_D3
	PD4/FMC_NOE		---- 读控制信号，OE = Output Enable ， N 表示低有效
	PD5/FMC_NWE		-XX- 写控制信号，AD7606 只有读，无写信号
	PD8/FMC_D13
	PD9/FMC_D14
	PD10/FMC_D15
	PD14/FMC_D0
	PD15/FMC_D1

	PE7/FMC_D4
	PE8/FMC_D5
	PE9/FMC_D6
	PE10/FMC_D7
	PE11/FMC_D8
	PE12/FMC_D9
	PE13/FMC_D10
	PE14/FMC_D11
	PE15/FMC_D12
	
	PG0/FMC_A10		--- 和主片选FMC_NE2一起译码
	PG1/FMC_A11		--- 和主片选FMC_NE2一起译码
	PD7/FMC_NE1		--- 主片选（OLED, 74HC574, DM9000, AD7606）	

	 +-------------------+------------------+
	 +   32-bits Mode: D31-D16              +
	 +-------------------+------------------+
	 | PH8 <-> FMC_D16   | PI0 <-> FMC_D24  |
	 | PH9 <-> FMC_D17   | PI1 <-> FMC_D25  |
	 | PH10 <-> FMC_D18  | PI2 <-> FMC_D26  |
	 | PH11 <-> FMC_D19  | PI3 <-> FMC_D27  |
	 | PH12 <-> FMC_D20  | PI6 <-> FMC_D28  |
	 | PH13 <-> FMC_D21  | PI7 <-> FMC_D29  |
	 | PH14 <-> FMC_D22  | PI9 <-> FMC_D30  |
	 | PH15 <-> FMC_D23  | PI10 <-> FMC_D31 |
	 +------------------+-------------------+
*/

/* 
	控制AD7606参数的其他IO分配在扩展的74HC574上
	X13 - AD7606_OS0
	X14 - AD7606_OS1
	X15 - AD7606_OS2
	X24 - AD7606_RESET
	X25 - AD7606_RAGE	
	
	PE5 - AD7606_BUSY
*/
static void AD7606_CtrlLinesConfig(void)
{
	/* bsp_fm_io 已配置fmc，bsp_InitExtIO();
	   此处可以不必重复配置 
	*/

	GPIO_InitTypeDef gpio_init_structure;

	/* 使能 GPIO时钟 */
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOE_CLK_ENABLE();
	__HAL_RCC_GPIOF_CLK_ENABLE();
	__HAL_RCC_GPIOG_CLK_ENABLE();
	__HAL_RCC_GPIOH_CLK_ENABLE();
	__HAL_RCC_GPIOI_CLK_ENABLE();

	/* 使能FMC时钟 */
	__HAL_RCC_FMC_CLK_ENABLE();

	/* 设置 GPIOD 相关的IO为复用推挽输出 */
	gpio_init_structure.Mode = GPIO_MODE_AF_PP;
	gpio_init_structure.Pull = GPIO_PULLUP;
	gpio_init_structure.Speed = GPIO_SPEED_FREQ_HIGH;
	gpio_init_structure.Alternate = GPIO_AF12_FMC;
	
	/* 配置GPIOD */
	gpio_init_structure.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_7 |
	                            GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_14 |
	                            GPIO_PIN_15;
	HAL_GPIO_Init(GPIOD, &gpio_init_structure);

	/* 配置GPIOE */
	gpio_init_structure.Pin = GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 |
	                            GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 |
	                            GPIO_PIN_15;
	HAL_GPIO_Init(GPIOE, &gpio_init_structure);

	/* 配置GPIOG */
	gpio_init_structure.Pin = GPIO_PIN_0 | GPIO_PIN_1;
	HAL_GPIO_Init(GPIOG, &gpio_init_structure);
	
	/* 配置GPIOH */
	gpio_init_structure.Pin = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12
						| GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
	HAL_GPIO_Init(GPIOH, &gpio_init_structure);

	/* 配置GPIOI */
	gpio_init_structure.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_6
						| GPIO_PIN_7 | GPIO_PIN_9 | GPIO_PIN_10;
	HAL_GPIO_Init(GPIOI, &gpio_init_structure);
	
	/* CONVST 启动ADC转换的GPIO = PC6 */
	{
		GPIO_InitTypeDef   GPIO_InitStructure;
		CONVST_RCC_GPIO_CLK_ENABLE();

		/* 配置PC6 */
		GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;		/* 设置推挽输出 */
		GPIO_InitStructure.Pull = GPIO_NOPULL;				/* 上下拉电阻不使能 */
		GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_MEDIUM;  /* GPIO速度等级 */	

		GPIO_InitStructure.Pin = CONVST_PIN;	
		HAL_GPIO_Init(CONVST_GPIO, &GPIO_InitStructure);	
	}
}

/*
*********************************************************************************************************
*	函 数 名: AD7606_FSMCConfig
*	功能说明: 配置FSMC并口访问时序
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void AD7606_FSMCConfig(void)
{
	/* 
	   DM9000，扩展IO，OLED和AD7606公用一个FMC配置，如果都开启，请以FMC速度最慢的为准。
	   从而保证所有外设都可以正常工作。
	*/
	SRAM_HandleTypeDef hsram = {0};
	FMC_NORSRAM_TimingTypeDef SRAM_Timing = {0};
		
	/*
		AD7606规格书要求(3.3V时，通信电平Vdriver)：RD读信号低电平脉冲宽度最短21ns，对应FMC的DataSetupTime
		CS片选和RD读信号独立方式的高电平脉冲最短宽度15ns。
		CS片选和RD读信号并联方式的高电平脉冲最短宽度22ns。
		这里将22ns作为最小值更合理些，对应FMC的AddressSetupTime
	
		5-x-5-x-x-x  : RD高持续25ns， 低电平持续25ns. 读取8路样本数据到内存差不多就是400ns。
	*/
	hsram.Instance  = FMC_NORSRAM_DEVICE;
	hsram.Extended  = FMC_NORSRAM_EXTENDED_DEVICE;
	
	/* FMC使用的HCLK3，主频200MHz，1个FMC时钟周期就是5ns */
	SRAM_Timing.AddressSetupTime       = 5;  /* 5*5ns=25ns，地址建立时间，范围0 -15个FMC时钟周期个数 */
	SRAM_Timing.AddressHoldTime        = 2;  /* 地址保持时间，配置为模式A时，用不到此参数 范围1 -15个时钟周期个数 */
	SRAM_Timing.DataSetupTime          = 5;  /* 5*5ns=25ns，数据建立时间，范围1 -255个时钟周期个数 */
	SRAM_Timing.BusTurnAroundDuration  = 1;  /* 此配置用不到这个参数 */
	SRAM_Timing.CLKDivision            = 2;  /* 此配置用不到这个参数 */
	SRAM_Timing.DataLatency            = 2;  /* 此配置用不到这个参数 */
	SRAM_Timing.AccessMode             = FMC_ACCESS_MODE_A; /* 配置为模式A */

	hsram.Init.NSBank             = FMC_NORSRAM_BANK1;              /* 使用的BANK1，即使用的片选FMC_NE1 */
	hsram.Init.DataAddressMux     = FMC_DATA_ADDRESS_MUX_DISABLE;   /* 禁止地址数据复用 */
	hsram.Init.MemoryType         = FMC_MEMORY_TYPE_SRAM;           /* 存储器类型SRAM */
	hsram.Init.MemoryDataWidth    = FMC_NORSRAM_MEM_BUS_WIDTH_32;	/* 32位总线宽度 */
	hsram.Init.BurstAccessMode    = FMC_BURST_ACCESS_MODE_DISABLE;  /* 关闭突发模式 */
	hsram.Init.WaitSignalPolarity = FMC_WAIT_SIGNAL_POLARITY_LOW;   /* 用于设置等待信号的极性，关闭突发模式，此参数无效 */
	hsram.Init.WaitSignalActive   = FMC_WAIT_TIMING_BEFORE_WS;      /* 关闭突发模式，此参数无效 */
	hsram.Init.WriteOperation     = FMC_WRITE_OPERATION_ENABLE;     /* 用于使能或者禁止写保护 */
	hsram.Init.WaitSignal         = FMC_WAIT_SIGNAL_DISABLE;        /* 关闭突发模式，此参数无效 */
	hsram.Init.ExtendedMode       = FMC_EXTENDED_MODE_DISABLE;      /* 禁止扩展模式 */
	hsram.Init.AsynchronousWait   = FMC_ASYNCHRONOUS_WAIT_DISABLE;  /* 用于异步传输期间，使能或者禁止等待信号，这里选择关闭 */
	hsram.Init.WriteBurst         = FMC_WRITE_BURST_DISABLE;        /* 禁止写突发 */
	hsram.Init.ContinuousClock    = FMC_CONTINUOUS_CLOCK_SYNC_ONLY; /* 仅同步模式才做时钟输出 */
    hsram.Init.WriteFifo          = FMC_WRITE_FIFO_ENABLE;          /* 使能写FIFO */

	/* 初始化SRAM控制器 */
	if (HAL_SRAM_Init(&hsram, &SRAM_Timing, &SRAM_Timing) != HAL_OK)
	{
		/* 初始化错误 */
		Error_Handler(__FILE__, __LINE__);
	}	
}

/*
*********************************************************************************************************
*	函 数 名: AD7606_SetOS
*	功能说明: 配置AD7606数字滤波器，也就设置过采样倍率。
*			  通过设置 AD7606_OS0、OS1、OS2口线的电平组合状态决定过采样倍率。
*			  启动AD转换之后，AD7606内部自动实现剩余样本的采集，然后求平均值输出。
*
*			  过采样倍率越高，转换时间越长。
*			  0、无过采样时，AD转换时间 = 3.45us - 4.15us
*			  1、2倍过采样时 = 7.87us - 9.1us
*			  2、4倍过采样时 = 16.05us - 18.8us
*			  3、8倍过采样时 = 33us - 39us
*			  4、16倍过采样时 = 66us - 78us
*			  5、32倍过采样时 = 133us - 158us
*			  6、64倍过采样时 = 257us - 315us
*
*	形    参: _ucOS : 过采样倍率, 0 - 6
*	返 回 值: 无
*********************************************************************************************************
*/
void AD7606_SetOS(uint8_t _ucOS)
{
	g_tAD7606.ucOS = _ucOS;
	switch (_ucOS)
	{
		case AD_OS_X2:
			OS2_0();
			OS1_0();
			OS0_1();
			break;

		case AD_OS_X4:
			OS2_0();
			OS1_1();
			OS0_0();
			break;

		case AD_OS_X8:
			OS2_0();
			OS1_1();
			OS0_1();
			break;

		case AD_OS_X16:
			OS2_1();
			OS1_0();
			OS0_0();
			break;

		case AD_OS_X32:
			OS2_1();
			OS1_0();
			OS0_1();
			break;

		case AD_OS_X64:
			OS2_1();
			OS1_1();
			OS0_0();
			break;

		case AD_OS_NO:
		default:
			g_tAD7606.ucOS = AD_OS_NO;
			OS2_0();
			OS1_0();
			OS0_0();
			break;
	}
}

/*
*********************************************************************************************************
*	函 数 名: AD7606_SetInputRange
*	功能说明: 配置AD7606模拟信号输入量程。
*	形    参: _ucRange : 0 表示正负5V   1表示正负10V
*	返 回 值: 无
*********************************************************************************************************
*/
void AD7606_SetInputRange(uint8_t _ucRange)
{
	if (_ucRange == 0)
	{
		g_tAD7606.ucRange = 0;
		RANGE_0();	/* 设置为正负5V */
	}
	else
	{
		g_tAD7606.ucRange = 1;
		RANGE_1();	/* 设置为正负10V */
	}
}

/*
*********************************************************************************************************
*	函 数 名: AD7606_Reset
*	功能说明: 硬件复位AD7606，复位之后恢复到正常工作状态。
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void AD7606_Reset(void)
{
	RESET_0();	/* 退出复位状态 */

	RESET_1();	/* 进入复位状态 */
	RESET_1();	/* 仅用于延迟。 RESET复位高电平脉冲宽度最小50ns。 */
	RESET_1();
	RESET_1();

	RESET_0();	/* 退出复位状态 */
}

/*
*********************************************************************************************************
*	函 数 名: AD7606_StartConvst
*	功能说明: 启动1次ADC转换
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void AD7606_StartConvst(void)
{
	/* page 7：  CONVST 高电平脉冲宽度和低电平脉冲宽度最短 25ns */
	/* CONVST平时为高 */
	CONVST_0();
	CONVST_0();
	CONVST_0();

	CONVST_1();
}

/*
*********************************************************************************************************
*	函 数 名: AD7606_StartRecord
*	功能说明: 开始采集
*	形    参: _ulFreq : 采样频率，单位Hz，	1k，2k，5k，10k，20K，50k，100k，200k
*	返 回 值: 无
*********************************************************************************************************
*/
void AD7606_StartRecord(uint32_t _ulFreq)
{
	AD7606_StopRecord();

	AD7606_Reset();					/* 复位硬件 */
	AD7606_StartConvst();			/* 启动采样，避免第1组数据全0的问题 */

	/* 配置PC6为TIM8_CH1功能 */
	AD7606_SetTIMOutPWM(CONVST_TIMX, _ulFreq);
}

/*
*********************************************************************************************************
*	函 数 名: AD7606_StopRecord
*	功能说明: 停止采集定时器
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void AD7606_StopRecord(void)
{
	/* 配置PC6 输出低电平，关闭TIM */
	HAL_GPIO_DeInit(CONVST_GPIO, CONVST_PIN);
	CONVST_TIM8_CLK_DISABLE();
	TIMx_UP_DMA_STREAM_CLK_DISABLE();
	
	/* CONVST 启动ADC转换的GPIO = PC6 */
	{
		GPIO_InitTypeDef   GPIO_InitStructure;
		CONVST_RCC_GPIO_CLK_ENABLE();

		/* 配置PC6 */
		GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;		/* 设置推挽输出 */
		GPIO_InitStructure.Pull = GPIO_NOPULL;				/* 上下拉电阻不使能 */
		GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_MEDIUM;  /* GPIO速度等级 */	

		GPIO_InitStructure.Pin = CONVST_PIN;	
		HAL_GPIO_Init(CONVST_GPIO, &GPIO_InitStructure);	
	}
	
	CONVST_1();			/* 启动转换的GPIO平时设置为高 */	
}

/*
*********************************************************************************************************
*	函 数 名: AD7606_SetTIMOutPWM
*	功能说明: 用于设置定时UP更新触发DMA传输，并设置定时器PWM作为AD7606的触发时钟
*	形    参: TIMx : TIM1 - TIM17
*			  _ulFreq : PWM信号频率，单位Hz，范围1Hz - 200KHz
*	返 回 值: 无
*********************************************************************************************************
*/
/* DMA传输完成回调函数，弱定义 */
__weak void AD7606_DmaCplCb(DMA_HandleTypeDef *hdma)
{
	
}

/* DMA半传输完成回调函数，弱定义 */
__weak void AD7606_DmaHalfCplCb(DMA_HandleTypeDef *hdma)
{
	
}

/* DMA中断服务程序 */
void TIMx_UP_DMA_IRQHandler(void)
{
	HAL_DMA_IRQHandler(&TIMDMA);
}

static void AD7606_SetTIMOutPWM(TIM_TypeDef* TIMx, uint32_t _ulFreq)
{
	TIM_OC_InitTypeDef sConfig = {0};	
	GPIO_InitTypeDef   GPIO_InitStruct;
	uint16_t usPeriod;
	uint16_t usPrescaler;
	uint32_t uiTIMxCLK;
	uint32_t pulse;

	
	/* 配置时钟 */
	CONVST_RCC_GPIO_CLK_ENABLE();
	CONVST_TIM8_CLK_ENABLE();
	TIMx_UP_DMA_STREAM_CLK_ENABLE();
	
	/* 配置引脚 */
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.Alternate = CONVST_AF;
	GPIO_InitStruct.Pin = CONVST_PIN;
	HAL_GPIO_Init(CONVST_GPIO, &GPIO_InitStruct);
	
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

        因为APB1 prescaler != 1, 所以 APB1上的TIMxCLK = APB1 x 2 = 200MHz;
        因为APB2 prescaler != 1, 所以 APB2上的TIMxCLK = APB2 x 2 = 200MHz;
        APB4上面的TIMxCLK没有分频，所以就是100MHz;

        APB1 定时器有 TIM2, TIM3 ,TIM4, TIM5, TIM6, TIM7, TIM12, TIM13, TIM14，LPTIM1
        APB2 定时器有 TIM1, TIM8 , TIM15, TIM16，TIM17

        APB4 定时器有 LPTIM2，LPTIM3，LPTIM4，LPTIM5

	----------------------------------------------------------------------- */
	if ((TIMx == TIM1) || (TIMx == TIM8) || (TIMx == TIM15) || (TIMx == TIM16) || (TIMx == TIM17))
	{
		/* APB2 定时器时钟 = 200M */
		uiTIMxCLK = SystemCoreClock / 2;
	}
	else	
	{
		/* APB1 定时器 = 200M */
		uiTIMxCLK = SystemCoreClock / 2;
	}

	if (_ulFreq < 100)
	{
		usPrescaler = 10000 - 1;							/* 分频比 = 10000 */
		usPeriod =  (uiTIMxCLK / 10000) / _ulFreq  - 1;		/* 自动重装的值， usPeriod最小值200, 单位50us */
		pulse = usPeriod;                               	/* 设置低电平时间50us，注意usPeriod已经进行了减1操作 */
	} 
	else if (_ulFreq < 3000)
	{
		usPrescaler = 100 - 1;							/* 分频比 = 100 */
		usPeriod =  (uiTIMxCLK / 100) / _ulFreq  - 1;	/* 自动重装的值， usPeriod最小值666，单位500ns */
		pulse = usPeriod-1;                           	/* 设置低电平时间1us，注意usPeriod已经进行了减1操作 */
	}
	else	/* 大于4K的频率，无需分频 */
	{
		usPrescaler = 0;								/* 分频比 = 1 */
		usPeriod = uiTIMxCLK / _ulFreq - 1;				/* 自动重装的值， usPeriod最小值1000，单位5ns */
		pulse = usPeriod - 199;              			/* 设置低电平时间1us，注意usPeriod已经进行了减1操作 */
	}
    
	/*  PWM频率 = TIMxCLK / usPrescaler + 1）/usPeriod + 1）*/
	TimHandle.Instance = TIMx;
	TimHandle.Init.Prescaler         = usPrescaler;         /* 用于设置定时器分频 */
	TimHandle.Init.Period            = usPeriod;            /* 用于设置定时器周期 */
	TimHandle.Init.ClockDivision     = 0;                   /* 用于指示定时器时钟 (CK_INT) 频率与死区发生器以及数字滤波器（ETR、 TIx）
	                                                           所使用的死区及采样时钟 (tDTS) 之间的分频比*/
	TimHandle.Init.CounterMode       = TIM_COUNTERMODE_UP;  /* 用于设置计数模式，向上计数模式 */
	TimHandle.Init.RepetitionCounter = 0;                   /* 用于设置重复计数器，仅 TIM1 和 TIM8 有，其它定时器没有 */
	TimHandle.Init.AutoReloadPreload = 0;                   /* 用于设置定时器的 ARR 自动重装寄存器是更新事件产生时写入有效 */
	
	if (HAL_TIM_PWM_DeInit(&TimHandle) != HAL_OK)
	{
		Error_Handler(__FILE__, __LINE__);		
	}
	
	if (HAL_TIM_PWM_Init(&TimHandle) != HAL_OK)
	{
		Error_Handler(__FILE__, __LINE__);
	}

	/* 配置定时器PWM输出通道 */
	sConfig.OCMode       = TIM_OCMODE_PWM1;         /* 配置输出比较模式 */
	sConfig.OCPolarity   = TIM_OCPOLARITY_HIGH;     /* 设置输出高电平有效 */
	sConfig.OCFastMode   = TIM_OCFAST_DISABLE;      /* 关闭快速输出模式 */
	sConfig.OCNPolarity  = TIM_OCNPOLARITY_HIGH;    /* 配置互补输出高电平有效 */
	sConfig.OCIdleState  = TIM_OCIDLESTATE_SET;     /* 空闲状态时，设置输出比较引脚为高电平 */
	sConfig.OCNIdleState = TIM_OCNIDLESTATE_RESET;  /* 空闲状态时，设置互补输出比较引脚为低电平 */

	/* 占空比 */
	sConfig.Pulse = pulse;
	if (HAL_TIM_PWM_ConfigChannel(&TimHandle, &sConfig, CONVST_TIMCH) != HAL_OK)
	{
		Error_Handler(__FILE__, __LINE__);
	}
	
	/* 使能定时器中断  */
	__HAL_TIM_ENABLE_DMA(&TimHandle, TIM_DMA_UPDATE);
	
	/* 启动PWM输出 */
	if (HAL_TIM_PWM_Start(&TimHandle, CONVST_TIMCH) != HAL_OK)
	{
		Error_Handler(__FILE__, __LINE__);
	}
	
	/* 定时器UP更新触发DMA传输 */		
	TIMDMA.Instance                 = TIMx_UP_DMA_STREAM;      /* 例化使用的DMA数据流 */
	TIMDMA.Init.FIFOMode            = DMA_FIFOMODE_ENABLE;     /* 使能FIFO*/
	TIMDMA.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL; /* 用于设置阀值 */
	TIMDMA.Init.MemBurst            = DMA_MBURST_INC8;	       /* 用于存储器突发 */
	TIMDMA.Init.PeriphBurst         = DMA_PBURST_INC8;	       /* 用于外设突发 */
	TIMDMA.Init.Request             = TIMx_UP_DMA_REQUEST;     /* 请求类型 */  
	TIMDMA.Init.Direction           = DMA_PERIPH_TO_MEMORY;    /* 传输方向是从外设到存储器 */  
	TIMDMA.Init.PeriphInc           = DMA_PINC_DISABLE;        /* 外设地址自增禁止 */ 
	TIMDMA.Init.MemInc              = DMA_MINC_ENABLE;         /* 存储器地址自增使能 */  
	TIMDMA.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD; /* 外设数据传输位宽选择半字，即16bit */ 
	TIMDMA.Init.MemDataAlignment    = DMA_MDATAALIGN_HALFWORD; /* 存储器数据传输位宽选择半字，即16bit */    
	TIMDMA.Init.Mode                = DMA_CIRCULAR; 		   /* 循环模式 */
	TIMDMA.Init.Priority            = DMA_PRIORITY_LOW;        /* 优先级低 */
	
	 /* 复位DMA */
	if(HAL_DMA_DeInit(&TIMDMA) != HAL_OK)
	{
		Error_Handler(__FILE__, __LINE__);     
	}
	
	 /* 初始化DMA */
	if(HAL_DMA_Init(&TIMDMA) != HAL_OK)
	{
		Error_Handler(__FILE__, __LINE__);     
	}
	
	/* 关联DMA句柄到TIM */
	//__HAL_LINKDMA(&TimHandle, hdma[TIM_DMA_ID_UPDATE], TIMDMA);	
	
	/* 配置DMA中断 */
	HAL_NVIC_SetPriority(TIMx_UP_DMA_IRQn, 1, 0);
	HAL_NVIC_EnableIRQ(TIMx_UP_DMA_IRQn);
	
	/* 注册半传输完成中断和传输完成中断 */
	HAL_DMA_RegisterCallback(&TIMDMA, HAL_DMA_XFER_CPLT_CB_ID, AD7606_DmaCplCb);
	HAL_DMA_RegisterCallback(&TIMDMA, HAL_DMA_XFER_HALFCPLT_CB_ID, AD7606_DmaHalfCplCb);
	
	/* 启动DMA传输 */
	HAL_DMA_Start_IT(&TIMDMA, (uint32_t)AD7606_BASE, (uint32_t)g_sAd7606Buf, AD7606_BUFSIZE);
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
