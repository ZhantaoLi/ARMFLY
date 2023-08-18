/*
*********************************************************************************************************
*
*	模块名称 : 定时器触发DMA
*	文件名称 : bsp_tim_dma.c
*	版    本 : V1.0
*	说    明 : DMAMUX的定时器触+DMA控制任意IO做PWM和脉冲数控制
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



LPTIM_HandleTypeDef  LptimHandle = {0};

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
#endif

/*
*********************************************************************************************************
*	函 数 名: LPTIM_Config
*	功能说明: 配置LPTIM，用于触发DMAMUX的请求发生器
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void LPTIM_Config(void)
{
    RCC_PeriphCLKInitTypeDef  PeriphClkInitStruct;

    
    /*##-1- 配置LPTIM2使用PCLK时钟 ##################################################*/
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LPTIM2;
    PeriphClkInitStruct.Lptim2ClockSelection = RCC_LPTIM2CLKSOURCE_D3PCLK1;
    HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);  


    /*##-2- 使能LPTIM2时钟并配置 ####################################################*/
    __HAL_RCC_LPTIM2_CLK_ENABLE();

    LptimHandle.Instance                           = LPTIM2;
    LptimHandle.Init.CounterSource                 = LPTIM_COUNTERSOURCE_INTERNAL;
    LptimHandle.Init.UpdateMode                    = LPTIM_UPDATE_ENDOFPERIOD;
    LptimHandle.Init.OutputPolarity                = LPTIM_OUTPUTPOLARITY_HIGH;
    LptimHandle.Init.Clock.Source                  = LPTIM_CLOCKSOURCE_APBCLOCK_LPOSC;
    LptimHandle.Init.Clock.Prescaler               = LPTIM_PRESCALER_DIV1;
    LptimHandle.Init.UltraLowPowerClock.Polarity   = LPTIM_CLOCKPOLARITY_RISING;
    LptimHandle.Init.UltraLowPowerClock.SampleTime = LPTIM_CLOCKSAMPLETIME_DIRECTTRANSITION;
    LptimHandle.Init.Trigger.Source                = LPTIM_TRIGSOURCE_SOFTWARE;
    LptimHandle.Init.Trigger.ActiveEdge            = LPTIM_ACTIVEEDGE_RISING;
    LptimHandle.Init.Trigger.SampleTime            = LPTIM_TRIGSAMPLETIME_DIRECTTRANSITION;

    /*##-3- 初始化LPTIM2 ##########################################################*/
    if(HAL_LPTIM_Init(&LptimHandle) != HAL_OK)
    {
        Error_Handler(__FILE__, __LINE__);
    }

    /*##-4- 启动LPTIM2的PWM模式，但使用输出引脚，仅用于DMAMUX的触发 ##############*/
    /* LPTIM2的时钟主频是100MHz，这里配置触发是100MHz / (10000 - 1 + 1) = 10KHz */
    if (HAL_LPTIM_PWM_Start(&LptimHandle, 10000-1, 5000 - 1) != HAL_OK)
    {
        Error_Handler(__FILE__, __LINE__);
    }  
}

/*
*********************************************************************************************************
*	函 数 名: bsp_InitTimBDMA
*	功能说明: 配置DMAMUX的定时器触+DMA控制任意IO做PWM和脉冲数控制
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_InitTimBDMA(void)
{
    GPIO_InitTypeDef  GPIO_InitStruct;
    DMA_HandleTypeDef DMA_Handle = {0};
    HAL_DMA_MuxRequestGeneratorConfigTypeDef dmamux_ReqGenParams ={0};

    
     /*##-1- 配置PB1用于PWM输出######################################*/ 
    __HAL_RCC_GPIOB_CLK_ENABLE();
      
    GPIO_InitStruct.Pin = GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    
  
    /*##-2- 配置DMA ##################################################*/
    __HAL_RCC_BDMA_CLK_ENABLE();

    DMA_Handle.Instance                 = BDMA_Channel0;           /* 使用的BDMA通道0 */
    DMA_Handle.Init.Request             = BDMA_REQUEST_GENERATOR0; /* 请求类型采用的DMAMUX请求发生器通道0 */  
    DMA_Handle.Init.Direction           = DMA_MEMORY_TO_PERIPH;    /* 传输方向是从存储器到外设 */  
    DMA_Handle.Init.PeriphInc           = DMA_PINC_DISABLE;        /* 外设地址自增禁止 */  
    DMA_Handle.Init.MemInc              = DMA_MINC_ENABLE;         /* 存储器地址自增使能 */  
    DMA_Handle.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;     /* 外设数据传输位宽选择字，即32bit */     
    DMA_Handle.Init.MemDataAlignment    = DMA_MDATAALIGN_WORD;     /* 存储器数据传输位宽选择字，即32bit */    
    DMA_Handle.Init.Mode                = DMA_CIRCULAR;            /* 循环模式 */   
    DMA_Handle.Init.Priority            = DMA_PRIORITY_LOW;        /* 优先级低 */  
    DMA_Handle.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;    /* BDMA不支持FIFO */ 
    DMA_Handle.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL; /* BDMA不支持FIFO阀值设置 */ 
    DMA_Handle.Init.MemBurst            = DMA_MBURST_SINGLE;       /* BDMA不支持存储器突发 */ 
    DMA_Handle.Init.PeriphBurst         = DMA_PBURST_SINGLE;       /* BDMA不支持外设突发 */ 
    
    HAL_DMA_Init(&DMA_Handle);

    /* 开启BDMA Channel0的中断 */
    HAL_NVIC_SetPriority(BDMA_Channel0_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(BDMA_Channel0_IRQn); 

    /*##-3- 配置DMAMUX #########################################################*/
    dmamux_ReqGenParams.SignalID  = HAL_DMAMUX2_REQ_GEN_LPTIM2_OUT;         /* 请求触发器选择LPTIM2_OUT */
    dmamux_ReqGenParams.Polarity  = HAL_DMAMUX_REQ_GEN_RISING_FALLING;      /* LPTIM2输出的上升沿和下降沿均可触发  */
    dmamux_ReqGenParams.RequestNumber = 1;                                  /* 触发后，传输进行1次DMA传输 */

    HAL_DMAEx_ConfigMuxRequestGenerator(&DMA_Handle, &dmamux_ReqGenParams); /* 配置DMAMUX */
    
    HAL_DMAEx_EnableMuxRequestGenerator (&DMA_Handle);                      /* 使能DMAMUX请求发生器 */        
      
    /*##-4- 启动DMA传输 ################################################*/
    HAL_DMA_Start_IT(&DMA_Handle, (uint32_t)IO_Toggle, (uint32_t)&GPIOB->BSRRL, 8);
    
    /* 
       默认情况下，用户通过注册回调函数DMA_Handle.XferHalfCpltCallback，然后函数HAL_DMA_Init会开启半传输完成中断，
       由于这里没有使用HAL库默认的中断管理函数HAL_DMA_IRQHandler，直接手动开启。
    */
    BDMA_Channel0->CCR |= BDMA_CCR_HTIE;
    
    LPTIM_Config(); /* 配置LPTIM触发DMAMUX */
}

/*
*********************************************************************************************************
*	函 数 名: BDMA_Channel0_IRQHandler
*	功能说明: BDMA通道0
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void BDMA_Channel0_IRQHandler(void)
{
    /* 传输完成中断 */
    if((BDMA->ISR & BDMA_FLAG_TC0) != RESET)
    {
        BDMA->IFCR = BDMA_FLAG_TC0;

        /*
           1、传输完成开始使用DMA缓冲区的前半部分，此时可以动态修改后半部分数据
              比如缓冲区大小是IO_Toggle[0] 到 IO_Toggle[7]
              那么此时可以修改IO_Toggle[4] 到 IO_Toggle[7]
           2、变量所在的SRAM区已经通过MPU配置为WT模式，更新变量IO_Toggle会立即写入。
           3、不配置MPU的话，也可以通过Cahce的函数SCB_CleanDCache_by_Addr做Clean操作。
        */
    }
 
    /* 半传输完成中断 */    
    if((BDMA->ISR & BDMA_FLAG_HT0) != RESET)
    {
        BDMA->IFCR = BDMA_FLAG_HT0;
        
        /*
           1、半传输完成开始使用DMA缓冲区的后半部分，此时可以动态修改前半部分数据
              比如缓冲区大小是IO_Toggle[0] 到 IO_Toggle[7]
              那么此时可以修改IO_Toggle[0] 到 IO_Toggle[3]
           2、变量所在的SRAM区已经通过MPU配置为WT模式，更新变量IO_Toggle会立即写入。
           3、不配置MPU的话，也可以通过Cahce的函数SCB_CleanDCache_by_Addr做Clean操作。
        */
    }
 
    /* 传输错误中断 */
    if((BDMA->ISR & BDMA_FLAG_TE0) != RESET)
    {
        BDMA->IFCR = BDMA_FLAG_TE0;
    }
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
