/*
*********************************************************************************************************
*
*	模块名称 : DAC驱动
*	文件名称 : bsp_dac.c
*	版    本 : V1.0
*	说    明 : DAC定时器触发+DMA方式双通道同步输出
*	修改记录 :
*		版本号   日期        作者     说明
*		V1.0    2019-06-01  armfly   正式发布
*
*	Copyright (C), 2018-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/
#include "bsp.h"



/* 方便Cache类的API操作，做32字节对齐 */
#if defined ( __ICCARM__ )
#pragma location = ".RAM_D3"  
ALIGN_32BYTES(uint16_t g_usWaveBuff[64]);

#elif defined ( __CC_ARM )
ALIGN_32BYTES(__attribute__((section (".RAM_D3"))) uint16_t g_usWaveBuff[64]);
#endif


/*
*********************************************************************************************************
*										变量
*********************************************************************************************************
*/
static TIM_HandleTypeDef      htim;
static DMA_HandleTypeDef      hdma_dac1;
static DMA_HandleTypeDef      hdma_dac2;
static DAC_ChannelConfTypeDef sConfig;
static DAC_HandleTypeDef      DacHandle; 

void TIM6_Config(void);
void DAC_WaveConfig(void);	  

/*
*********************************************************************************************************
*	函 数 名: bsp_InitDAC
*	功能说明: DAC初始化
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_InitDAC(void)
{   
    uint8_t i;

	/* 一个周期的方波 */
	for(i =0; i < 32; i++)
	{
		g_usWaveBuff[i] = 0;
	}
	
	for(i =0; i < 32; i++)
	{
		g_usWaveBuff[i+32] = 4095;
	}
	
	DAC_WaveConfig();
	TIM6_Config(); 
}

/*
*********************************************************************************************************
*	函 数 名: TIM6_Config
*	功能说明: 配置定时器6，用于触发DAC。
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void TIM6_Config(void)
{
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
	
	TIM6 更新周期是 = TIM6CLK / （Period + 1）/（Prescaler + 1）
	根据如下的配置，更新周期是：
	TIM6CLK /（Period + 1）/（Prescaler + 1）
	= 200MHz /（30+1）/（0+1）
	≈ 6.45MHz
	----------------------------------------------------------------------- */
	TIM_MasterConfigTypeDef sMasterConfig;
	
		/* TIM6 时钟使能 */
	__HAL_RCC_TIM6_CLK_ENABLE();

	/* 配置定时器外设 */
	htim.Instance = TIM6;

	htim.Init.Period            = 30;
	htim.Init.Prescaler         = 0;
	htim.Init.ClockDivision     = 0;
	htim.Init.CounterMode       = TIM_COUNTERMODE_UP;
	htim.Init.RepetitionCounter = 0;
	HAL_TIM_Base_Init(&htim);

	/* TIM6 TRGO 选择 */
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;

	HAL_TIMEx_MasterConfigSynchronization(&htim, &sMasterConfig);

	/* 使能定时器 */
	HAL_TIM_Base_Start(&htim);
}

/*
*********************************************************************************************************
*	函 数 名: DAC_WaveConfig
*	功能说明: DAC通道1和通道2方波输出
*             上面函数TIM6_Config已将将TIM6的输出配置成6.45MHz，下面DMA的缓存大小是64,那么方波的输出频率就是
*             方波周期 = TIM6更新周期/64 = 6.45MHz / 64 ≈ 100KHz
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void DAC_WaveConfig(void)
{
	GPIO_InitTypeDef          GPIO_InitStruct;
	

	/*##-1- 使能时钟 #################################*/
    __HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_DAC12_CLK_ENABLE();
	__HAL_RCC_DMA1_CLK_ENABLE();

	/*##-2- 配置GPIO ##########################################*/
	GPIO_InitStruct.Pin = GPIO_PIN_4;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    GPIO_InitStruct.Pin = GPIO_PIN_5;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  
	/*##-3- 初始化DAC外设 ######################################*/
    DacHandle.Instance = DAC1;
  
	HAL_DAC_DeInit(&DacHandle);	
    
	if (HAL_DAC_Init(&DacHandle) != HAL_OK)
	{
		Error_Handler(__FILE__, __LINE__);
	}

	/*##-4- 配置DAC通道1和通道2 ######################################*/
	sConfig.DAC_SampleAndHold = DAC_SAMPLEANDHOLD_DISABLE;        /* 关闭采样保持模式，这个模式主要用于低功耗 */
	sConfig.DAC_Trigger = DAC_TRIGGER_T6_TRGO;                    /* 采用定时器6触发 */
	sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;           /* 使能输出缓冲 */
	sConfig.DAC_ConnectOnChipPeripheral = DAC_CHIPCONNECT_DISABLE;/* 不将DAC连接到片上外设 */
	sConfig.DAC_UserTrimming = DAC_TRIMMING_FACTORY;              /* 使用出厂校准 */

	if (HAL_DAC_ConfigChannel(&DacHandle, &sConfig, DAC_CHANNEL_1) != HAL_OK)
	{
		Error_Handler(__FILE__, __LINE__);
	}
    
    if (HAL_DAC_ConfigChannel(&DacHandle, &sConfig, DAC_CHANNEL_2) != HAL_OK)
	{
		Error_Handler(__FILE__, __LINE__);
	}
    
	/*##-5- 配置通道1的DMA ##########################################*/
	hdma_dac1.Instance = DMA1_Stream0;              /* 使用的DAM1 Stream0 */
	hdma_dac1.Init.Request  = DMA_REQUEST_DAC1;     /* DAC触发DMA传输 */
	hdma_dac1.Init.Direction = DMA_MEMORY_TO_PERIPH;/* 存储器到外设 */
	hdma_dac1.Init.PeriphInc = DMA_PINC_DISABLE;    /* 外设地址禁止自增 */
	hdma_dac1.Init.MemInc = DMA_MINC_ENABLE;        /* 存储器地址自增 */
	hdma_dac1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD; /* 外输操作数据宽度，半字 */
	hdma_dac1.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;    /* 存储器操作数据宽度，半字 */
	hdma_dac1.Init.Mode = DMA_CIRCULAR;                           /* 循环模式 */
	hdma_dac1.Init.Priority = DMA_PRIORITY_HIGH;                  /* 优先级高 */

	HAL_DMA_Init(&hdma_dac1);

	/* 关联DMA句柄到DAC句柄下 */
	__HAL_LINKDMA(&DacHandle, DMA_Handle1, hdma_dac1);

	/* 启动DAC DMA */
	if (HAL_DAC_Start_DMA(&DacHandle, DAC_CHANNEL_1, (uint32_t *)g_usWaveBuff, 64, DAC_ALIGN_12B_R) != HAL_OK)
	{
		Error_Handler(__FILE__, __LINE__);
	} 
 
	/*##-6- 配置通道2的DMA ##########################################*/	
    hdma_dac2.Instance = DMA1_Stream1;                        /* 使用的DAM1 Stream1 */
	hdma_dac2.Init.Request  = DMA_REQUEST_DAC2;               /* DAC触发DMA传输 */
	hdma_dac2.Init.Direction = DMA_MEMORY_TO_PERIPH;          /* 存储器到外设 */
	hdma_dac2.Init.PeriphInc = DMA_PINC_DISABLE;              /* 外设地址禁止自增 */
	hdma_dac2.Init.MemInc = DMA_MINC_ENABLE;                  /* 存储器地址自增 */
	hdma_dac2.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;/* 外输操作数据宽度，半字 */
	hdma_dac2.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;/* 存储器操作数据宽度，半字 */
	hdma_dac2.Init.Mode = DMA_CIRCULAR;                       /* 循环模式 */
	hdma_dac2.Init.Priority = DMA_PRIORITY_HIGH;              /* 优先级高 */

	HAL_DMA_Init(&hdma_dac2);

	/* 关联DMA句柄到DAC句柄下 */
	__HAL_LINKDMA(&DacHandle, DMA_Handle2, hdma_dac2);

	/* 启动DAC DMA */    
	if (HAL_DAC_Start_DMA(&DacHandle, DAC_CHANNEL_2, (uint32_t *)g_usWaveBuff, 64, DAC_ALIGN_12B_R) != HAL_OK)
	{
		Error_Handler(__FILE__, __LINE__);
	} 
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
