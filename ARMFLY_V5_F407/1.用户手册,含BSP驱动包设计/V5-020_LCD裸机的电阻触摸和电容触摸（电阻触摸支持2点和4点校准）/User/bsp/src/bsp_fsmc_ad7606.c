/*
*********************************************************************************************************
*
*	模块名称 : AD7606数据采集模块
*	文件名称 : bsp_fsmc_ad7606.c
*	版    本 : V1.0
*	说    明 : AD7606挂在STM32的FMC总线上，本例子使用了TIM8作为硬件定时器，定时启动ADC转换。
*              AD7606的FMC驱动做了两种采集方式：
*              （1）软件查询方式，适合低速查询获取。
*              （2）FIFO工作模式，适合8路实时采集，支持最高采样率200Ksps。
*
*	修改记录 :
*		版本号  日期        作者     说明
*		V1.0    2020-05-01 armfly  正式发布
*
*	Copyright (C), 2020-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/
#include "bsp.h"



/*
	STM32-V6开发板 + AD7606模块， 控制采集的IO:
	
	PH9/DCMI_D0/AD7606_OS0			---> AD7606_OS0		OS2:OS0 选择数字滤波参数
	PH10/DCMI_D1/AD7606_OS1         ---> AD7606_OS1
	PH11/DCMI_D2/AD7606_OS2         ---> AD7606_OS2
	PH14/DCMI_D4/AD7606_RAGE        ---> AD7606_RAGE	输入模拟电压量程，正负5V或正负10V
	PI4/DCMI_D5/AD7606_RESET        ---> AD7606_RESET	复位
	PI6/DCMI_D6/AD7606_BUSY         ---> AD7606_BUSY	忙信号	(未使用)
	PH12/TIM5_CH3/DCMI_D3/AD7606_CONVST   ---> AD7606_CONVST	启动ADC转换 (CONVSTA 和 CONVSTB 已经并联)
*/

/* CONVST 启动ADC转换的GPIO = PH12 */
#define CONVST_RCC_GPIO_CLK_ENABLE	__HAL_RCC_GPIOH_CLK_ENABLE
#define CONVST_TIM8_CLK_DISABLE     __HAL_RCC_TIM5_CLK_DISABLE
#define CONVST_GPIO		GPIOH
#define CONVST_PIN		GPIO_PIN_12
#define CONVST_TIMX		TIM5
#define CONVST_TIMCH	3

/* 启动AD转换 */
#define CONVST_1()		CONVST_GPIO->BSRR = CONVST_PIN
#define CONVST_0()		CONVST_GPIO->BSRR = ((uint32_t)CONVST_PIN << 16U)

/* BUSY 转换完毕信号 = PI6 */
#define BUSY_RCC_GPIO_CLK_ENABLE __HAL_RCC_GPIOI_CLK_ENABLE
#define BUSY_GPIO		GPIOI
#define BUSY_PIN		GPIO_PIN_6
#define BUSY_IRQn		EXTI9_5_IRQn
#define BUSY_IRQHandler	EXTI9_5_IRQHandler

/* 设置过采样的GPIO: PH9 PH10 PH11 */
#define ALL_OS_GPIO_CLK_ENABLE() {	   \
	    __HAL_RCC_GPIOH_CLK_ENABLE();	\
	};

#define OS0_GPIO	GPIOH
#define OS0_PIN		GPIO_PIN_9
	
#define OS1_GPIO	GPIOH
#define OS1_PIN		GPIO_PIN_10
	
#define OS2_GPIO	GPIOH
#define OS2_PIN		GPIO_PIN_11

#define OS0_1()		OS0_GPIO->BSRR = OS0_PIN
#define OS0_0()		OS0_GPIO->BSRR= ((uint32_t)OS0_PIN << 16U)
#define OS1_1()		OS1_GPIO->BSRR = OS1_PIN
#define OS1_0()		OS1_GPIO->BSRR = ((uint32_t)OS1_PIN << 16U)
#define OS2_1()		OS2_GPIO->BSRR = OS2_PIN
#define OS2_0()		OS2_GPIO->BSRR = ((uint32_t)OS2_PIN << 16U)

/* 设置输入量程的GPIO : PH14  */
#define RANGE_RCC_GPIO_CLK_ENABLE __HAL_RCC_GPIOH_CLK_ENABLE
#define RANGE_GPIO		GPIOH
#define RANGE_PIN		GPIO_PIN_14
	
#define RANGE_1()	RANGE_GPIO->BSRR = RANGE_PIN
#define RANGE_0()	RANGE_GPIO->BSRR = ((uint32_t)RANGE_PIN << 16U)

/* AD7606复位口线 : PI4  */
#define RESET_RCC_GPIO_CLK_ENABLE __HAL_RCC_GPIOI_CLK_ENABLE
#define RESET_GPIO		GPIOI
#define RESET_PIN		GPIO_PIN_4

#define RESET_1()	RESET_GPIO->BSRR = RESET_PIN
#define RESET_0()	RESET_GPIO->BSRR = ((uint32_t)RESET_PIN << 16U)

/* AD7606 FSMC总线地址，只能读，无需写 */
#define AD7606_RESULT()	*(__IO uint16_t *)0x6C400000

AD7606_VAR_T g_tAD7606;		/* 定义1个全局变量，保存一些参数 */
AD7606_FIFO_T g_tAdcFifo;	/* 定义FIFO结构体变量 */

static void AD7606_CtrlLinesConfig(void);
static void AD7606_FSMCConfig(void);


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
	安富莱STM32-V5开发板接线方法：

	PD0/FSMC_D2
	PD1/FSMC_D3
	PD4/FSMC_NOE		--- 读控制信号，OE = Output Enable ， N 表示低有效
	PD5/FSMC_NWE		--- 写控制信号，AD7606 只有读，无写信号
	PD8/FSMC_D13
	PD9/FSMC_D14
	PD10/FSMC_D15

	PD14/FSMC_D0
	PD15/FSMC_D1

	PE4/FSMC_A20		--- 和主片选一起译码
	PE5/FSMC_A21		--- 和主片选一起译码
	PE7/FSMC_D4
	PE8/FSMC_D5
	PE9/FSMC_D6
	PE10/FSMC_D7
	PE11/FSMC_D8
	PE12/FSMC_D9
	PE13/FSMC_D10
	PE14/FSMC_D11
	PE15/FSMC_D12

	PG12/FSMC_NE4		--- 主片选（TFT, OLED 和 AD7606）

	其他的控制IO:

	PH9/DCMI_D0/AD7606_OS0			---> AD7606_OS0		OS2:OS0 选择数字滤波参数
	PH10/DCMI_D1/AD7606_OS1         ---> AD7606_OS1
	PH11/DCMI_D2/AD7606_OS2         ---> AD7606_OS2
	PH12/DCMI_D3/AD7606_CONVST      ---> AD7606_CONVST	启动ADC转换 (CONVSTA 和 CONVSTB 已经并联)
	PH14/DCMI_D4/AD7606_RAGE        ---> AD7606_RAGE	输入模拟电压量程，正负5V或正负10V
	PI4/DCMI_D5/AD7606_RESET        ---> AD7606_RESET	复位
	PI6/DCMI_D6/AD7606_BUSY         ---> AD7606_BUSY	忙信号	(未使用)
*/

static void AD7606_CtrlLinesConfig(void)
{

	GPIO_InitTypeDef gpio_init_structure;

	/* 使能 GPIO时钟 */
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOE_CLK_ENABLE();
	__HAL_RCC_GPIOF_CLK_ENABLE();
	__HAL_RCC_GPIOG_CLK_ENABLE();
	__HAL_RCC_GPIOH_CLK_ENABLE();
	__HAL_RCC_GPIOI_CLK_ENABLE();

	/* 使能FMC时钟 */
	__HAL_RCC_FSMC_CLK_ENABLE();

	/* 设置 GPIOD 相关的IO为复用推挽输出 */
	gpio_init_structure.Mode = GPIO_MODE_AF_PP;
	gpio_init_structure.Pull = GPIO_PULLUP;
	gpio_init_structure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	gpio_init_structure.Alternate = GPIO_AF12_FSMC;
	
	/* 配置GPIOD */
	gpio_init_structure.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_4 | GPIO_PIN_5  |
	                            GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_14 |
	                            GPIO_PIN_15;
	HAL_GPIO_Init(GPIOD, &gpio_init_structure);

	/* 配置GPIOE */
	gpio_init_structure.Pin = GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 |
	                          GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
	HAL_GPIO_Init(GPIOE, &gpio_init_structure);

	/* 配置GPIOG */
	gpio_init_structure.Pin = GPIO_PIN_12;
	HAL_GPIO_Init(GPIOG, &gpio_init_structure);


	/*	配置几个控制用的GPIO
		PH9/DCMI_D0/AD7606_OS0			---> AD7606_OS0		OS2:OS0 选择数字滤波参数
		PH10/DCMI_D1/AD7606_OS1         ---> AD7606_OS1
		PH11/DCMI_D2/AD7606_OS2         ---> AD7606_OS2
		PH12/DCMI_D3/AD7606_CONVST      ---> AD7606_CONVST	启动ADC转换
		PH14/DCMI_D4/AD7606_RAGE        ---> AD7606_RAGE	输入模拟电压量程，正负5V或正负10V
		PI4/DCMI_D5/AD7606_RESET        ---> AD7606_RESET	复位

		PI6/DCMI_D6/AD7606_BUSY			---> AD7606_BUSY    转换结束的信号
	*/
	
	/* 配置BUSY引脚，默认是普通IO状态 */
	{
		GPIO_InitTypeDef   GPIO_InitStructure;
	
		__HAL_RCC_SYSCFG_CLK_ENABLE();
				
		BUSY_RCC_GPIO_CLK_ENABLE();		/* 打开GPIO时钟 */

		/* BUSY信号，使用的PI6，用于转换完毕检测 */
		GPIO_InitStructure.Mode = GPIO_MODE_INPUT;   /* 设置推挽输出 */
		GPIO_InitStructure.Pull = GPIO_NOPULL;       /* 无上拉下拉 */
		GPIO_InitStructure.Pin = BUSY_PIN;           
		HAL_GPIO_Init(BUSY_GPIO, &GPIO_InitStructure);	
	}
	
	/* 其它IO */
	{
		GPIO_InitTypeDef   GPIO_InitStructure;
		ALL_OS_GPIO_CLK_ENABLE();
		RANGE_RCC_GPIO_CLK_ENABLE();
		RESET_RCC_GPIO_CLK_ENABLE();
		CONVST_RCC_GPIO_CLK_ENABLE();
		
		GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;		/* 设置推挽输出 */
		GPIO_InitStructure.Pull = GPIO_PULLUP;				/* 上下拉电阻不使能 */
		GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;    /* GPIO速度等级 */	

		GPIO_InitStructure.Pin = CONVST_PIN;	
		HAL_GPIO_Init(CONVST_GPIO, &GPIO_InitStructure);

		GPIO_InitStructure.Pin = OS0_PIN;	
		HAL_GPIO_Init(OS0_GPIO, &GPIO_InitStructure);	

		GPIO_InitStructure.Pin = OS1_PIN;	
		HAL_GPIO_Init(OS1_GPIO, &GPIO_InitStructure);	

		GPIO_InitStructure.Pin = OS2_PIN;	
		HAL_GPIO_Init(OS2_GPIO, &GPIO_InitStructure);	

		GPIO_InitStructure.Pin = RANGE_PIN;	
		HAL_GPIO_Init(RANGE_GPIO, &GPIO_InitStructure);	
		
		GPIO_InitStructure.Pin = RESET_PIN;	
		HAL_GPIO_Init(RESET_GPIO, &GPIO_InitStructure);	
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
	   TFT-LCD，OLED和AD7606公用一个FMC配置，如果都开启，请以FMC速度最慢的为准。
	   从而保证所有外设都可以正常工作。
	*/
	SRAM_HandleTypeDef hsram = {0};
	FMC_NORSRAM_TimingTypeDef SRAM_Timing = {0};
		
	/*
		AD7606规格书要求(3.3V时，通信电平Vdriver)：RD读信号低电平脉冲宽度最短21ns，对应FMC的DataSetupTime
		CS片选和RD读信号独立方式的高电平脉冲最短宽度15ns。
		CS片选和RD读信号并联方式的高电平脉冲最短宽度22ns。
		这里将22ns作为最小值更合理些，对应FMC的AddressSetupTime
	
		4-x-6-x-x-x  : RD高持续35.7ns，低电平持续23.8ns. 读取8路样本数据到内存差不多就是476ns。
	*/
	hsram.Instance  = FMC_NORSRAM_DEVICE;
	hsram.Extended  = FMC_NORSRAM_EXTENDED_DEVICE;
	
	/* FMC使用的HCLK，主频168MHz，1个FMC时钟周期就是5.95ns */
	SRAM_Timing.AddressSetupTime       = 4;  /* 4*5.95ns=23.8ns，地址建立时间，范围0 -15个FMC时钟周期个数 */
	SRAM_Timing.AddressHoldTime        = 0;  /* 地址保持时间，配置为模式A时，用不到此参数 范围1 -15个时钟周期个数 */
	SRAM_Timing.DataSetupTime          = 6;  /* 6*5.95ns=35.7ns，数据保持时间，范围1 -255个时钟周期个数 */
	SRAM_Timing.BusTurnAroundDuration  = 0;  /* 此配置用不到这个参数 */
	SRAM_Timing.CLKDivision            = 0;  /* 此配置用不到这个参数 */
	SRAM_Timing.DataLatency            = 0;  /* 此配置用不到这个参数 */
	SRAM_Timing.AccessMode             = FSMC_ACCESS_MODE_A; /* 配置为模式A */

	hsram.Init.NSBank             = FSMC_NORSRAM_BANK4;              /* 使用的BANK4，即使用的片选FSMC_NE4 */
	hsram.Init.DataAddressMux     = FSMC_DATA_ADDRESS_MUX_DISABLE;   /* 禁止地址数据复用 */
	hsram.Init.MemoryType         = FSMC_MEMORY_TYPE_SRAM;           /* 存储器类型SRAM */
	hsram.Init.MemoryDataWidth    = FSMC_NORSRAM_MEM_BUS_WIDTH_16;	/* 16位总线宽度 */
	hsram.Init.BurstAccessMode    = FSMC_BURST_ACCESS_MODE_DISABLE;  /* 关闭突发模式 */
	hsram.Init.WaitSignalPolarity = FSMC_WAIT_SIGNAL_POLARITY_LOW;   /* 用于设置等待信号的极性，关闭突发模式，此参数无效 */
	hsram.Init.WaitSignalActive   = FSMC_WAIT_TIMING_BEFORE_WS;      /* 关闭突发模式，此参数无效 */
	hsram.Init.WriteOperation     = FSMC_WRITE_OPERATION_ENABLE;     /* 用于使能或者禁止写保护 */
	hsram.Init.WaitSignal         = FSMC_WAIT_SIGNAL_DISABLE;        /* 关闭突发模式，此参数无效 */
	hsram.Init.ExtendedMode       = FSMC_EXTENDED_MODE_DISABLE;      /* 禁止扩展模式 */
	hsram.Init.AsynchronousWait   = FSMC_ASYNCHRONOUS_WAIT_DISABLE;  /* 用于异步传输期间，使能或者禁止等待信号，这里选择关闭 */
	hsram.Init.WriteBurst         = FSMC_WRITE_BURST_DISABLE;        /* 禁止写突发 */
	hsram.Init.ContinuousClock    = FSMC_CONTINUOUS_CLOCK_SYNC_ONLY; /* 仅同步模式才做时钟输出 */
    hsram.Init.WriteFifo          = FSMC_WRITE_FIFO_ENABLE;          /* 使能写FIFO */

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
*	函 数 名: AD7606_ReadNowAdc
*	功能说明: 读取8路采样结果。结果存储在全局变量 g_tAD7606
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
/* 弱定义，方便用户将采集的结果实时输出 */
__weak void AD7606_SEGGER_RTTOUT(void)
{
	
}

void AD7606_ReadNowAdc(void)
{
	g_tAD7606.sNowAdc[0] = AD7606_RESULT();	/* 读第1路样本 */
	g_tAD7606.sNowAdc[1] = AD7606_RESULT();	/* 读第2路样本 */
	g_tAD7606.sNowAdc[2] = AD7606_RESULT();	/* 读第3路样本 */
	g_tAD7606.sNowAdc[3] = AD7606_RESULT();	/* 读第4路样本 */
	g_tAD7606.sNowAdc[4] = AD7606_RESULT();	/* 读第5路样本 */
	g_tAD7606.sNowAdc[5] = AD7606_RESULT();	/* 读第6路样本 */
	g_tAD7606.sNowAdc[6] = AD7606_RESULT();	/* 读第7路样本 */
	g_tAD7606.sNowAdc[7] = AD7606_RESULT();	/* 读第8路样本 */

	AD7606_SEGGER_RTTOUT();
}

/*
*********************************************************************************************************
*		下面的函数用于定时采集模式。 TIM8硬件定时中断中读取ADC结果，存在全局FIFO
*
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*	函 数 名: AD7606_HasNewData
*	功能说明: 判断FIFO中是否有新数据
*	形    参:  无
*	返 回 值: 1 表示有，0表示暂无数据
*********************************************************************************************************
*/
uint8_t AD7606_HasNewData(void)
{
	if (g_tAdcFifo.usCount > 0)
	{
		return 1;
	}
	return 0;
}

/*
*********************************************************************************************************
*	函 数 名: AD7606_FifoFull
*	功能说明: 判断FIFO是否满
*	形    参: 无
*	返 回 值: 1 表示满，0表示未满
*********************************************************************************************************
*/
uint8_t AD7606_FifoFull(void)
{
	return g_tAdcFifo.ucFull;
}

/*
*********************************************************************************************************
*	函 数 名: AD7606_ReadFifo
*	功能说明: 从FIFO中读取一个ADC值
*	形    参:  _usReadAdc : 存放ADC结果的变量指针
*	返 回 值: 1 表示OK，0表示暂无数据
*********************************************************************************************************
*/
uint8_t AD7606_ReadFifo(uint16_t *_usReadAdc)
{
	if (AD7606_HasNewData())
	{
		*_usReadAdc = g_tAdcFifo.sBuf[g_tAdcFifo.usRead];
		if (++g_tAdcFifo.usRead >= ADC_FIFO_SIZE)
		{
			g_tAdcFifo.usRead = 0;
		}

		DISABLE_INT();
		if (g_tAdcFifo.usCount > 0)
		{
			g_tAdcFifo.usCount--;
		}
		ENABLE_INT();
		return 1;
	}
	return 0;
}

/*
*********************************************************************************************************
*	函 数 名: AD7606_StartRecord
*	功能说明: 开始采集
*	形    参: _ulFreq : 采样频率，单位Hz , 对于AD7606来说，范围1 - 200KHz
*	返 回 值: 无
*********************************************************************************************************
*/
void AD7606_StartRecord(uint32_t _ulFreq)
{
	AD7606_StopRecord();

	AD7606_Reset();					/* 复位硬件 */
	AD7606_StartConvst();			/* 启动采样，避免第1组数据全0的问题 */

	g_tAdcFifo.usRead = 0;			/* 必须在开启定时器之前清0 */
	g_tAdcFifo.usWrite = 0;
	g_tAdcFifo.usCount = 0;
	g_tAdcFifo.ucFull = 0;

	AD7606_EnterAutoMode(_ulFreq);
}

/*
*********************************************************************************************************
*	函 数 名: AD7606_EnterAutoMode
*	功能说明: 配置硬件工作在自动采集模式，结果存储在FIFO缓冲区。
*	形    参:  _ulFreq : 采样频率，单位Hz，	1k，2k，5k，10k，20K，50k，100k，200k
*	返 回 值: 无
*********************************************************************************************************
*/
void AD7606_EnterAutoMode(uint32_t _ulFreq)
{
	/* 配置PH12为TIM5_CH3功能，输出占空比50%的方波 */
	bsp_SetTIMOutPWM(CONVST_GPIO, CONVST_PIN, CONVST_TIMX,  CONVST_TIMCH, _ulFreq, 5000);
	
	/* 配置PI6, BUSY 作为中断输入口，下降沿触发 */
	{
		GPIO_InitTypeDef   GPIO_InitStructure;
	
		CONVST_RCC_GPIO_CLK_ENABLE();	/* 打开GPIO时钟 */
		__HAL_RCC_SYSCFG_CLK_ENABLE();

		GPIO_InitStructure.Mode = GPIO_MODE_IT_FALLING;	/* 中断下降沿触发 */
		GPIO_InitStructure.Pull = GPIO_NOPULL;
		GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_LOW;		
		GPIO_InitStructure.Pin = BUSY_PIN;
		HAL_GPIO_Init(BUSY_GPIO, &GPIO_InitStructure);	

		HAL_NVIC_SetPriority(BUSY_IRQn, 2, 0);
		HAL_NVIC_EnableIRQ(BUSY_IRQn);	
	}		
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
	/* 配置PH12 输出低电平，关闭TIM */
	bsp_SetTIMOutPWM(CONVST_GPIO, CONVST_PIN, CONVST_TIMX,  CONVST_TIMCH, 1000, 10000);
	HAL_GPIO_DeInit(CONVST_GPIO, CONVST_PIN);
	CONVST_TIM8_CLK_DISABLE();
	
	/* CONVST 启动ADC转换的GPIO = PH12*/
	{
		GPIO_InitTypeDef   GPIO_InitStructure;
		CONVST_RCC_GPIO_CLK_ENABLE();

		/* 配置PH12 */
		GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;		/* 设置推挽输出 */
		GPIO_InitStructure.Pull = GPIO_NOPULL;				/* 上下拉电阻不使能 */
		GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;    /* GPIO速度等级 */	

		GPIO_InitStructure.Pin = CONVST_PIN;	
		HAL_GPIO_Init(CONVST_GPIO, &GPIO_InitStructure);	
	}
	
	CONVST_1();			/* 启动转换的GPIO平时设置为高 */	
	
	/* 配置BUSY引脚，默认是普通IO状态 */
	{
		GPIO_InitTypeDef   GPIO_InitStructure;
	
		__HAL_RCC_SYSCFG_CLK_ENABLE();
		BUSY_RCC_GPIO_CLK_ENABLE();		/* 打开GPIO时钟 */

		HAL_GPIO_DeInit(BUSY_GPIO, BUSY_PIN);
		
		/* BUSY信号，使用的PI6，用于转换完毕检测 */
		GPIO_InitStructure.Mode = GPIO_MODE_INPUT;   /* 设置推挽输出 */
		GPIO_InitStructure.Pull = GPIO_NOPULL;       /* 无上拉下拉 */
		GPIO_InitStructure.Pin = BUSY_PIN;           
		HAL_GPIO_Init(BUSY_GPIO, &GPIO_InitStructure);	
		
		HAL_NVIC_DisableIRQ(BUSY_IRQn);
	}
}

/*
*********************************************************************************************************
*	函 数 名: AD7606_ISR
*	功能说明: 定时采集中断服务程序
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void AD7606_ISR(void)
{
	uint8_t i;

	AD7606_ReadNowAdc();

	for (i = 0; i < 8; i++)
	{
		g_tAdcFifo.sBuf[g_tAdcFifo.usWrite] = g_tAD7606.sNowAdc[i];
		if (++g_tAdcFifo.usWrite >= ADC_FIFO_SIZE)
		{
			g_tAdcFifo.usWrite = 0;
		}
		if (g_tAdcFifo.usCount < ADC_FIFO_SIZE)
		{
			g_tAdcFifo.usCount++;
		}
		else
		{
			g_tAdcFifo.ucFull = 1;		/* FIFO 满，主程序来不及处理数据 */
		}
	}
}

/*
*********************************************************************************************************
*	函 数 名: EXTI9_5_IRQHandler
*	功能说明: 外部中断服务程序。
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
#ifdef EXTI9_5_ISR_MOVE_OUT		/* bsp.h 中定义此行，表示本函数移到 stam32xx_it.c。 避免重复定义 */
void EXTI9_5_IRQHandler(void)
{
	HAL_GPIO_EXTI_IRQHandler(BUSY_PIN);
}

/*
*********************************************************************************************************
*	函 数 名: EXTI9_5_IRQHandler
*	功能说明: 外部中断服务程序入口, AD7606_BUSY 下降沿中断触发
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if (GPIO_Pin == BUSY_PIN)
	{
		AD7606_ISR();
	}
}
#endif

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
