/*
*********************************************************************************************************
*
*	模块名称 : ADC驱动
*	文件名称 : bsp_adc.c
*	版    本 : V1.0
*	说    明 : ADC定时器触发+DMA双缓冲的实现
*              1. 例子默认用的AHB时钟供ADC使用，大家可以通过bsp_adc.c文件开头宏定义切换到PLL2专用时钟。
*              2、使用的TIM1的OC1作为ADC的外部触发源，触发速度是100KHz，即ADC的采样率也是100KHz。
*              3、使用DMA的半传输完成中断和传输完成中断实现数据的双缓冲更新。
*              4、采集引脚使用的PC0，另外特别注意开发板上的Vref稳压基准跳线帽短接的3.3V。
*
*	修改记录 :
*		版本号  日期        作者     说明
*		V1.0    2018-12-12 armfly  正式发布
*
*	Copyright (C), 2018-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/
#include "bsp.h"



/* 选择ADC的时钟源 */
#define ADC_CLOCK_SOURCE_AHB     /* 选择AHB时钟源 */
//#define ADC_CLOCK_SOURCE_PLL     /* 选择PLL时钟源 */


/* 方便Cache类的API操作，做32字节对齐 */
#if defined ( __ICCARM__ )
#pragma location = 0x38000000
uint16_t ADCxValues[128];
#elif defined ( __CC_ARM )
ALIGN_32BYTES(__attribute__((section (".RAM_D3"))) uint16_t ADCxValues[128]);
#endif

__IO uint8_t s_DmaFlag = 0;  /* 1表示进DMA半传输完成中断，2表示进入DMA传输完成中断 */
DMA_HandleTypeDef   DmaHandle = {0};	

/*
*********************************************************************************************************
*	函 数 名: TIM1_Config
*	功能说明: 配置TIM1，用于触发ADC，当前配置的100KHz触发频率
*	形    参: 无									  
*	返 回 值: 无
*********************************************************************************************************
*/
static void TIM1_Config(void)
{
	TIM_HandleTypeDef  htim ={0};
	TIM_OC_InitTypeDef sConfig = {0};


	/* 使能时钟 */  
	__HAL_RCC_TIM1_CLK_ENABLE();
      
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

    TIM12CLK = 200MHz/(Period + 1) / (Prescaler + 1) = 200MHz / 2000 / 1 = 100KHz
	----------------------------------------------------------------------- */  
    HAL_TIM_Base_DeInit(&htim);
    
    htim.Instance = TIM1;
	htim.Init.Period            = 1999;
	htim.Init.Prescaler         = 0;
	htim.Init.ClockDivision     = 0;
	htim.Init.CounterMode       = TIM_COUNTERMODE_UP;
	htim.Init.RepetitionCounter = 0;
	HAL_TIM_Base_Init(&htim);
    
    sConfig.OCMode     = TIM_OCMODE_PWM1;
    sConfig.OCPolarity = TIM_OCPOLARITY_LOW;

    /* 占空比50% */
    sConfig.Pulse = 1000;  
    if(HAL_TIM_OC_ConfigChannel(&htim, &sConfig, TIM_CHANNEL_1) != HAL_OK)
    {
		Error_Handler(__FILE__, __LINE__);
    }

    /* 启动OC1 */
    if(HAL_TIM_OC_Start(&htim, TIM_CHANNEL_1) != HAL_OK)
    {
		Error_Handler(__FILE__, __LINE__);
    }
}

/*
*********************************************************************************************************
*	函 数 名: bsp_InitADC
*	功能说明: 初始化ADC
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_InitADC(void)
{
    ADC_HandleTypeDef   AdcHandle = {0};
	ADC_ChannelConfTypeDef   sConfig = {0};
	GPIO_InitTypeDef         GPIO_InitStruct;

	
  /* ## - 1 - 配置ADC采样的时钟 ####################################### */
#if defined (ADC_CLOCK_SOURCE_PLL)
	/* 配置PLL2时钟为的72MHz，方便分频产生ADC最高时钟36MHz */
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
	PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_ADC;
	PeriphClkInitStruct.PLL2.PLL2M = 25;
	PeriphClkInitStruct.PLL2.PLL2N = 504;
	PeriphClkInitStruct.PLL2.PLL2P = 7;
	PeriphClkInitStruct.PLL2.PLL2Q = 7;
	PeriphClkInitStruct.PLL2.PLL2R = 7;
	PeriphClkInitStruct.PLL2.PLL2RGE = RCC_PLL2VCIRANGE_0;
	PeriphClkInitStruct.PLL2.PLL2VCOSEL = RCC_PLL2VCOWIDE;
	PeriphClkInitStruct.PLL2.PLL2FRACN = 0;
	PeriphClkInitStruct.AdcClockSelection = RCC_ADCCLKSOURCE_PLL2;
	if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
	{
		Error_Handler(__FILE__, __LINE__);  
	}
#elif defined (ADC_CLOCK_SOURCE_AHB)
  
  /* 使用AHB时钟的话，无需配置，默认选择*/
  
#endif

	/* ## - 2 - 配置ADC采样使用的引脚 ####################################### */
	__HAL_RCC_GPIOC_CLK_ENABLE();

	GPIO_InitStruct.Pin = GPIO_PIN_0;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	/* ## - 3 - 配置ADC采样使用的时钟 ####################################### */
	__HAL_RCC_DMA1_CLK_ENABLE();
	DmaHandle.Instance                 = DMA1_Stream1;            /* 使用的DMA1 Stream1 */
	DmaHandle.Init.Request             = DMA_REQUEST_ADC1;  	  /* 请求类型采用DMA_REQUEST_ADC1 */  
	DmaHandle.Init.Direction           = DMA_PERIPH_TO_MEMORY;    /* 传输方向是从外设到存储器 */  
	DmaHandle.Init.PeriphInc           = DMA_PINC_DISABLE;        /* 外设地址自增禁止 */ 
	DmaHandle.Init.MemInc              = DMA_MINC_ENABLE;         /* 存储器地址自增使能 */  
	DmaHandle.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;  /* 外设数据传输位宽选择半字，即16bit */     
	DmaHandle.Init.MemDataAlignment    = DMA_MDATAALIGN_HALFWORD;  /* 存储器数据传输位宽选择半字，即16bit */    
	DmaHandle.Init.Mode                = DMA_CIRCULAR;            /* 循环模式 */   
	DmaHandle.Init.Priority            = DMA_PRIORITY_LOW;        /* 优先级低 */  
	DmaHandle.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;    /* 禁止FIFO*/
	DmaHandle.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL; /* 禁止FIFO此位不起作用，用于设置阀值 */
	DmaHandle.Init.MemBurst            = DMA_MBURST_SINGLE;       /* 禁止FIFO此位不起作用，用于存储器突发 */
	DmaHandle.Init.PeriphBurst         = DMA_PBURST_SINGLE;       /* 禁止FIFO此位不起作用，用于外设突发 */
 
    /* 初始化DMA */
    if(HAL_DMA_Init(&DmaHandle) != HAL_OK)
    {
		Error_Handler(__FILE__, __LINE__);     
    }
    
    /* 开启DMA1 Stream1的中断 */
    HAL_NVIC_SetPriority(DMA1_Stream1_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(DMA1_Stream1_IRQn);
    
    /* 关联ADC句柄和DMA句柄 */
	__HAL_LINKDMA(&AdcHandle, DMA_Handle, DmaHandle);

	/* ## - 4 - 配置ADC ########################################################### */
	__HAL_RCC_ADC12_CLK_ENABLE();
	AdcHandle.Instance = ADC1;
	
#if defined (ADC_CLOCK_SOURCE_PLL)
	AdcHandle.Init.ClockPrescaler        = ADC_CLOCK_ASYNC_DIV2;          /* 采用PLL异步时钟，2分频，即72MHz/2 = 36MHz */
#elif defined (ADC_CLOCK_SOURCE_AHB)
	AdcHandle.Init.ClockPrescaler        = ADC_CLOCK_SYNC_PCLK_DIV4;      /* 采用AHB同步时钟，4分频，即200MHz/4 = 50MHz */
#endif
	AdcHandle.Init.Resolution            = ADC_RESOLUTION_16B;            /* 16位分辨率 */
	AdcHandle.Init.ScanConvMode          = ADC_SCAN_DISABLE;              /* 禁止扫描，因为仅开了一个通道 */
	AdcHandle.Init.EOCSelection          = ADC_EOC_SINGLE_CONV;           /* EOC转换结束标志 */
	AdcHandle.Init.LowPowerAutoWait      = DISABLE;                       /* 禁止低功耗自动延迟特性 */
	AdcHandle.Init.ContinuousConvMode    = DISABLE;                       /* 禁止自动转换，采用的定时器触发转换 */
	AdcHandle.Init.NbrOfConversion       = 1;                             /* 使用了1个转换通道 */
	AdcHandle.Init.DiscontinuousConvMode = DISABLE;                       /* 禁止不连续模式 */
	AdcHandle.Init.NbrOfDiscConversion   = 1;                             /* 禁止不连续模式后，此参数忽略，此位是用来配置不连续子组中通道数 */
	AdcHandle.Init.ExternalTrigConv      = ADC_EXTERNALTRIG_T1_CC1;            /* 定时器1的CC1触发 */
	AdcHandle.Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_RISING;    /* 上升沿触发 */
	AdcHandle.Init.ConversionDataManagement = ADC_CONVERSIONDATA_DMA_CIRCULAR; /* DMA循环模式接收ADC转换的数据 */
	AdcHandle.Init.BoostMode                = ENABLE;                          /* ADC时钟超过20MHz的话，使能boost */
	AdcHandle.Init.Overrun                  = ADC_OVR_DATA_OVERWRITTEN;        /* ADC转换溢出的话，覆盖ADC的数据寄存器 */
	AdcHandle.Init.OversamplingMode         = DISABLE;                         /* 禁止过采样 */

    /* 初始化ADC */
	if (HAL_ADC_Init(&AdcHandle) != HAL_OK)
	{
		Error_Handler(__FILE__, __LINE__);
	}
  
	/* 校准ADC，采用偏移校准 */
	if (HAL_ADCEx_Calibration_Start(&AdcHandle, ADC_CALIB_OFFSET, ADC_SINGLE_ENDED) != HAL_OK)
	{
		Error_Handler(__FILE__, __LINE__);
	}
  
	/* 配置ADC通道  */
	sConfig.Channel      = ADC_CHANNEL_10;              /* 配置使用的ADC通道 */
	sConfig.Rank         = ADC_REGULAR_RANK_1;          /* 采样序列里的第1个 */
	sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;     /* 采样周期 */
	sConfig.SingleDiff   = ADC_SINGLE_ENDED;            /* 单端输入 */
	sConfig.OffsetNumber = ADC_OFFSET_NONE;             /* 无偏移 */ 
	sConfig.Offset = 0;                                 /* 无偏移的情况下，此参数忽略 */

	if (HAL_ADC_ConfigChannel(&AdcHandle, &sConfig) != HAL_OK)
	{
		Error_Handler(__FILE__, __LINE__);
	}
  
	/* ## - 5 - 配置ADC的定时器触发 ####################################### */
	TIM1_Config();
  
	/* ## - 6 - 启动ADC的DMA方式传输 ####################################### */
	if (HAL_ADC_Start_DMA(&AdcHandle, (uint32_t *)ADCxValues, 128) != HAL_OK)
	{
		Error_Handler(__FILE__, __LINE__);
	}
}

/*
*********************************************************************************************************
*	函 数 名: DMA1_Stream1_IRQHandler
*	功能说明: DMA1 Stream1中断服务程序
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void DMA1_Stream1_IRQHandler(void)
{
	/* 传输完成中断 */
	if((DMA1->LISR & DMA_FLAG_TCIF1_5) != RESET)
	{
		
        HC574_TogglePin(GPIO_PIN_23);
		
		/*
		   1、使用此函数要特别注意，第1个参数地址要32字节对齐，第2个参数要是32字节的整数倍。
		   2、进入传输完成中断，当前DMA正在使用缓冲区的前半部分，用户可以操作后半部分。
		*/
		SCB_InvalidateDCache_by_Addr((uint32_t *)(&ADCxValues[64]), 128);
		
		s_DmaFlag = 2;
		
		/* 清除标志 */
		DMA1->LIFCR = DMA_FLAG_TCIF1_5;
	}

	/* 半传输完成中断 */    
	if((DMA1->LISR & DMA_FLAG_HTIF1_5) != RESET)
	{
		/*
		   1、使用此函数要特别注意，第1个参数地址要32字节对齐，第2个参数要是32字节的整数倍。
		   2、进入半传输完成中断，当前DMA正在使用缓冲区的后半部分，用户可以操作前半部分。
		*/
		SCB_InvalidateDCache_by_Addr((uint32_t *)(&ADCxValues[0]), 128);
		
		s_DmaFlag = 1;
        
		/* 清除标志 */
		DMA1->LIFCR = DMA_FLAG_HTIF1_5;
	}

	/* 传输错误中断 */
	if((DMA1->LISR & DMA_FLAG_TEIF1_5) != RESET)
	{
		/* 清除标志 */
		DMA1->LIFCR = DMA_FLAG_TEIF1_5;
	}

	/* 直接模式错误中断 */
	if((DMA1->LISR & DMA_FLAG_DMEIF1_5) != RESET)
	{
		/* 清除标志 */
		DMA1->LIFCR = DMA_FLAG_DMEIF1_5;
	}
}

/*
*********************************************************************************************************
*	函 数 名: bsp_GetAdcValues
*	功能说明: 获取ADC的数据并打印
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_GetAdcValues(void)
{
	uint32_t values;
	float  temp;
	
	/* 当前DMA操作是后半个缓冲，读取前半个缓冲的前4个数值求平均 */
	if(s_DmaFlag == 1)
	{
		DISABLE_INT();
		s_DmaFlag = 0;
		values = (ADCxValues[0] + ADCxValues[1] + ADCxValues[2] + ADCxValues[3])/4;

		ENABLE_INT();
	}
	/* 当前DMA操作是后前个缓冲，读取后半个缓冲的前4个数值求平均 */
	else if(s_DmaFlag == 2)
	{
		DISABLE_INT();
		s_DmaFlag = 0;
		values = (ADCxValues[64] + ADCxValues[65] + ADCxValues[66] + ADCxValues[67])/4;
		ENABLE_INT();
	}
	
	/* 打印读出的串口值 */
	temp = values *3.3 / 65536;
	
	printf("ADCxValues = %d, %5.3f\r\n", values, temp);
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
