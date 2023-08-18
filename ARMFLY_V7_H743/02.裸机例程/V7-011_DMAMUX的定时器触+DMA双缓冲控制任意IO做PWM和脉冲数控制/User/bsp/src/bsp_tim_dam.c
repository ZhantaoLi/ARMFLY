/*
*********************************************************************************************************
*
*	模块名称 : 定时器触发DMA
*	文件名称 : bsp_tim_dma.c
*	版    本 : V1.0
*	说    明 : DMAMUX的定时器触+DMA双缓冲控制任意IO做PWM和脉冲数控制
*
*	修改记录 :
*		版本号   日期        作者      说明
*		V1.0    2018-12-12  Eric2013  正式发布
*
*	Copyright (C), 2018-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/
#include "bsp.h"




/* 方便Cache类的API操作，做32字节对齐 */
#if defined ( __ICCARM__ )
#pragma location = 0x38000000
uint32_t IO_Toggle[8]  =
                      { 
                          0x00000002U,   
                          0x00020000U,  
                          0x00000002U,   
                          0x00020000U,   
                          0x00000002U,   
                          0x00020000U,   
                          0x00000002U,   
                          0x00020000U,  
                      };

#pragma location = 0x38000020
uint32_t IO_Toggle1[8]  =
                      { 
                          0x00000002U,   
                          0x00020000U,  
                          0x00000002U,   
                          0x00020000U,   
                          0x00000002U,   
                          0x00020000U,   
                          0x00000002U,   
                          0x00020000U,  
                      };

#elif defined ( __CC_ARM )
ALIGN_32BYTES(__attribute__((section (".RAM_D3"))) uint32_t IO_Toggle[8]) =
                                                      { 
                                                          0x00000002U,   
                                                          0x00020000U,  
                                                          0x00000002U,   
                                                          0x00020000U,   
                                                          0x00000002U,   
                                                          0x00020000U,   
                                                          0x00000002U,   
                                                          0x00020000U,  
                                                      };

ALIGN_32BYTES(__attribute__((section (".RAM_D3"))) uint32_t IO_Toggle1[8]) =
                                                      { 
                                                          0x00000002U,   
                                                          0x00020000U,  
                                                          0x00000002U,   
                                                          0x00020000U,   
                                                          0x00000002U,   
                                                          0x00020000U,   
                                                          0x00000002U,   
                                                          0x00020000U,  
                                                      };
#endif


/*
*********************************************************************************************************
*	函 数 名: TIM12_Config
*	功能说明: 配置TIM12，用于触发DMAMUX的请求发生器
*	形    参: _Mode 
*             0 表示配置为100KHz触发频率,配置为100KHz触发频率，如果DMAMUX配置为单边沿触发，那么输出PWM频
*               率是50KHz，双边沿是100KHz。
*			  1 表示配置为10KHz触发频率，如果DMAMUX配置为单边沿触发，那么输出PWM频率是5KHz，双边沿是10KHz。									  
*	返 回 值: 无
*********************************************************************************************************
*/
void TIM12_Config(uint8_t _Mode)
{
    TIM_HandleTypeDef  htim ={0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    TIM_OC_InitTypeDef sConfig = {0};
    uint32_t Period[2] = {1999, 19999};
    uint32_t Pulse[2]  = {999, 9999};

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

        因为APB1 prescaler != 1, 所以 APB1上的TIMxCLK = APB1 x 2 = 200MHz;
        因为APB2 prescaler != 1, 所以 APB2上的TIMxCLK = APB2 x 2 = 200MHz;
        APB4上面的TIMxCLK没有分频，所以就是100MHz;

        APB1 定时器有 TIM2, TIM3 ,TIM4, TIM5, TIM6, TIM7, TIM12, TIM13, TIM14，LPTIM1
        APB2 定时器有 TIM1, TIM8 , TIM15, TIM16，TIM17

        APB4 定时器有 LPTIM2，LPTIM3，LPTIM4，LPTIM5

    TIM12CLK = 200MHz/(Period + 1) / (Prescaler + 1)
    函数bsp_InitTimDMA1中DMAMUX1选择的是单边沿触发，每个时钟可以触发两次。
	----------------------------------------------------------------------- */  
    HAL_TIM_Base_DeInit(&htim);
    
    htim.Instance = TIM12;
	htim.Init.Period            = Period[_Mode];
	htim.Init.Prescaler         = 0;
	htim.Init.ClockDivision     = 0;
	htim.Init.CounterMode       = TIM_COUNTERMODE_UP;
	htim.Init.RepetitionCounter = 0;
	HAL_TIM_Base_Init(&htim);
    
    sConfig.OCMode     = TIM_OCMODE_PWM1;
    sConfig.OCPolarity = TIM_OCPOLARITY_LOW;

    /* 占空比50% */
    sConfig.Pulse = Pulse[_Mode];  
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

/*
*********************************************************************************************************
*	函 数 名: bsp_InitTimDMA
*	功能说明: 配置DMAMUX的定时器触+DMA双缓冲控制任意IO做PWM和脉冲数控制
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_InitTimDMA(void)
{
    GPIO_InitTypeDef  GPIO_InitStruct;
    DMA_HandleTypeDef DMA_Handle = {0};
    HAL_DMA_MuxRequestGeneratorConfigTypeDef dmamux_ReqGenParams = {0};

    /*##-1- 配置PB1用于PWM输出 ##################################################*/
    __HAL_RCC_GPIOB_CLK_ENABLE();
    GPIO_InitStruct.Pin = GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  
    /*##-2- 使能DMA1时钟并配置 ##################################################*/
    __HAL_RCC_DMA1_CLK_ENABLE();
    DMA_Handle.Instance                 = DMA1_Stream1;            /* 使用的DMA1 Stream1 */
    DMA_Handle.Init.Request             = DMA_REQUEST_GENERATOR0;  /* 请求类型采用的DMAMUX请求发生器通道0 */  
    DMA_Handle.Init.Direction           = DMA_MEMORY_TO_PERIPH;    /* 传输方向是从存储器到外设 */  
    DMA_Handle.Init.PeriphInc           = DMA_PINC_DISABLE;        /* 外设地址自增禁止 */ 
    DMA_Handle.Init.MemInc              = DMA_MINC_ENABLE;         /* 存储器地址自增使能 */  
    DMA_Handle.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;     /* 外设数据传输位宽选择字，即32bit */     
    DMA_Handle.Init.MemDataAlignment    = DMA_MDATAALIGN_WORD;     /* 存储器数据传输位宽选择字，即32bit */    
    DMA_Handle.Init.Mode                = DMA_CIRCULAR;            /* 循环模式 */   
    DMA_Handle.Init.Priority            = DMA_PRIORITY_LOW;        /* 优先级低 */  
    DMA_Handle.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;    /* 禁止FIFO*/
    DMA_Handle.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL; /* 禁止FIFO此位不起作用，用于设置阀值 */
    DMA_Handle.Init.MemBurst            = DMA_MBURST_SINGLE;       /* 禁止FIFO此位不起作用，用于存储器突发 */
    DMA_Handle.Init.PeriphBurst         = DMA_PBURST_SINGLE;       /* 禁止FIFO此位不起作用，用于外设突发 */
 
    /* 初始化DMA */
    if(HAL_DMA_Init(&DMA_Handle) != HAL_OK)
    {
		Error_Handler(__FILE__, __LINE__);     
    }

    /* 开启DMA1 Stream1的中断 */
    HAL_NVIC_SetPriority(DMA1_Stream1_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(DMA1_Stream1_IRQn); 

    
    /*##-4- 配置DMAMUX ###########################################################*/
    dmamux_ReqGenParams.SignalID  = HAL_DMAMUX1_REQ_GEN_TIM12_TRGO;         /* 请求触发器选择LPTIM2_OUT */
    dmamux_ReqGenParams.Polarity  = HAL_DMAMUX_REQ_GEN_RISING;              /* 上升沿触发  */
    dmamux_ReqGenParams.RequestNumber = 1;                                  /* 触发后，传输进行1次DMA传输 */

    HAL_DMAEx_ConfigMuxRequestGenerator(&DMA_Handle, &dmamux_ReqGenParams); /* 配置DMAMUX */
    HAL_DMAEx_EnableMuxRequestGenerator (&DMA_Handle);                      /* 使能DMAMUX请求发生器 */   
 
    
    /*##-4- 启动DMA双缓冲传输 ################################################*/
    /*
        1、此函数会开启DMA的TC，TE和DME中断
        2、如果用户配置了回调函数DMA_Handle.XferHalfCpltCallback，那么函数HAL_DMA_Init会开启半传输完成中断。
        3、如果用户使用了DMAMUX的同步模式，此函数会开启同步溢出中断。
        4、如果用户使用了DMAMUX的请求发生器，此函数会开始请求发生器溢出中断。
    */
    HAL_DMAEx_MultiBufferStart_IT(&DMA_Handle, (uint32_t)IO_Toggle, (uint32_t)&GPIOB->BSRRL,(uint32_t)IO_Toggle1, 8);
    
    /* 用不到的中断可以直接关闭 */
    //DMA1_Stream1->CR &= ~DMA_IT_DME; 
    //DMA1_Stream1->CR &= ~DMA_IT_TE;
    //DMAMUX1_RequestGenerator0->RGCR &= ~DMAMUX_RGxCR_OIE;
    
    TIM12_Config(0);
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
		/* 清除标志 */
		DMA1->LIFCR = DMA_FLAG_TCIF1_5;

		/* 当前使用的缓冲0 */
		if((DMA1_Stream1->CR & DMA_SxCR_CT) == RESET)
		{
			/*
				1、当前正在使用缓冲0，此时可以动态修改缓冲1的数据。
				   比如缓冲区0是IO_Toggle，缓冲区1是IO_Toggle1，那么此时就可以修改IO_Toggle1。
				2、变量所在的SRAM区已经通过MPU配置为WT模式，更新变量IO_Toggle会立即写入。
				3、不配置MPU的话，也可以通过Cahce的函数SCB_CleanDCache_by_Addr做Clean操作。
			*/
			
		}
		/* 当前使用的缓冲1 */
		else
		{
			 /*
			   1、当前正在使用缓冲1，此时可以动态修改缓冲0的数据。
				  比如缓冲区0是IO_Toggle，缓冲区1是IO_Toggle1，那么此时就可以修改IO_Toggle。
			   2、变量所在的SRAM区已经通过MPU配置为WT模式，更新变量IO_Toggle会立即写入。
			   3、不配置MPU的话，也可以通过Cahce的函数SCB_CleanDCache_by_Addr做Clean操作。
			*/

		}
	}

	/* 半传输完成中断 */    
	if((DMA1->LISR & DMA_FLAG_HTIF1_5) != RESET)
	{
		/* 清除标志 */
		DMA1->LISR = DMA_FLAG_HTIF1_5;
	}

	/* 传输错误中断 */
	if((DMA1->LISR & DMA_FLAG_TEIF1_5) != RESET)
	{
		/* 清除标志 */
		DMA1->LISR = DMA_FLAG_TEIF1_5;
	}

	/* 直接模式错误中断 */
	if((DMA1->LISR & DMA_FLAG_DMEIF1_5) != RESET)
	{
		/* 清除标志 */
		DMA1->LISR = DMA_FLAG_DMEIF1_5;
	}
}

/*
*********************************************************************************************************
*	函 数 名: DMAMUX1_OVR_IRQHandler
*	功能说明: DMAMUX的中断服务程序，这里用于处理请求发生器的溢出。
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void DMAMUX1_OVR_IRQHandler(void)
{
    if((DMAMUX1_RequestGenStatus->RGSR & DMAMUX_RGSR_OF0) != RESET)
    {
       /* 关闭溢出中断 */
       DMAMUX1_RequestGenerator0->RGCR &= ~DMAMUX_RGxCR_OIE;
       
       /* 清除标志 */
       DMAMUX1_RequestGenStatus->RGCFR = DMAMUX_RGSR_OF0;
    }
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
