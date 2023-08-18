/*
*********************************************************************************************************
*
*	ģ������ : ��ʱ������DMA
*	�ļ����� : bsp_tim_dma.c
*	��    �� : V1.0
*	˵    �� : DMAMUX�Ķ�ʱ����+DMA��������IO��PWM������������
*
*	�޸ļ�¼ :
*		�汾��   ����        ����      ˵��
*		V1.0    2018-12-12  Eric2013  ��ʽ����
*
*	Copyright (C), 2018-2030, ���������� www.armfly.com
*
*********************************************************************************************************
*/
#include "bsp.h"



LPTIM_HandleTypeDef  LptimHandle = {0};

/* ����Cache���API��������32�ֽڶ��� */
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
*	�� �� ��: LPTIM_Config
*	����˵��: ����LPTIM�����ڴ���DMAMUX����������
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void LPTIM_Config(void)
{
    RCC_PeriphCLKInitTypeDef  PeriphClkInitStruct;

    
    /*##-1- ����LPTIM2ʹ��PCLKʱ�� ##################################################*/
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LPTIM2;
    PeriphClkInitStruct.Lptim2ClockSelection = RCC_LPTIM2CLKSOURCE_D3PCLK1;
    HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);  


    /*##-2- ʹ��LPTIM2ʱ�Ӳ����� ####################################################*/
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

    /*##-3- ��ʼ��LPTIM2 ##########################################################*/
    if(HAL_LPTIM_Init(&LptimHandle) != HAL_OK)
    {
        Error_Handler(__FILE__, __LINE__);
    }

    /*##-4- ����LPTIM2��PWMģʽ����ʹ��������ţ�������DMAMUX�Ĵ��� ##############*/
    /* LPTIM2��ʱ����Ƶ��100MHz���������ô�����100MHz / (10000 - 1 + 1) = 10KHz */
    if (HAL_LPTIM_PWM_Start(&LptimHandle, 10000-1, 5000 - 1) != HAL_OK)
    {
        Error_Handler(__FILE__, __LINE__);
    }  
}

/*
*********************************************************************************************************
*	�� �� ��: bsp_InitTimBDMA
*	����˵��: ����DMAMUX�Ķ�ʱ����+DMA��������IO��PWM������������
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void bsp_InitTimBDMA(void)
{
    GPIO_InitTypeDef  GPIO_InitStruct;
    DMA_HandleTypeDef DMA_Handle = {0};
    HAL_DMA_MuxRequestGeneratorConfigTypeDef dmamux_ReqGenParams ={0};

    
     /*##-1- ����PB1����PWM���######################################*/ 
    __HAL_RCC_GPIOB_CLK_ENABLE();
      
    GPIO_InitStruct.Pin = GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    
  
    /*##-2- ����DMA ##################################################*/
    __HAL_RCC_BDMA_CLK_ENABLE();

    DMA_Handle.Instance                 = BDMA_Channel0;           /* ʹ�õ�BDMAͨ��0 */
    DMA_Handle.Init.Request             = BDMA_REQUEST_GENERATOR0; /* �������Ͳ��õ�DMAMUX��������ͨ��0 */  
    DMA_Handle.Init.Direction           = DMA_MEMORY_TO_PERIPH;    /* ���䷽���ǴӴ洢�������� */  
    DMA_Handle.Init.PeriphInc           = DMA_PINC_DISABLE;        /* �����ַ������ֹ */  
    DMA_Handle.Init.MemInc              = DMA_MINC_ENABLE;         /* �洢����ַ����ʹ�� */  
    DMA_Handle.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;     /* �������ݴ���λ��ѡ���֣���32bit */     
    DMA_Handle.Init.MemDataAlignment    = DMA_MDATAALIGN_WORD;     /* �洢�����ݴ���λ��ѡ���֣���32bit */    
    DMA_Handle.Init.Mode                = DMA_CIRCULAR;            /* ѭ��ģʽ */   
    DMA_Handle.Init.Priority            = DMA_PRIORITY_LOW;        /* ���ȼ��� */  
    DMA_Handle.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;    /* BDMA��֧��FIFO */ 
    DMA_Handle.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL; /* BDMA��֧��FIFO��ֵ���� */ 
    DMA_Handle.Init.MemBurst            = DMA_MBURST_SINGLE;       /* BDMA��֧�ִ洢��ͻ�� */ 
    DMA_Handle.Init.PeriphBurst         = DMA_PBURST_SINGLE;       /* BDMA��֧������ͻ�� */ 
    
    HAL_DMA_Init(&DMA_Handle);

    /* ����BDMA Channel0���ж� */
    HAL_NVIC_SetPriority(BDMA_Channel0_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(BDMA_Channel0_IRQn); 

    /*##-3- ����DMAMUX #########################################################*/
    dmamux_ReqGenParams.SignalID  = HAL_DMAMUX2_REQ_GEN_LPTIM2_OUT;         /* ���󴥷���ѡ��LPTIM2_OUT */
    dmamux_ReqGenParams.Polarity  = HAL_DMAMUX_REQ_GEN_RISING_FALLING;      /* LPTIM2����������غ��½��ؾ��ɴ���  */
    dmamux_ReqGenParams.RequestNumber = 1;                                  /* �����󣬴������1��DMA���� */

    HAL_DMAEx_ConfigMuxRequestGenerator(&DMA_Handle, &dmamux_ReqGenParams); /* ����DMAMUX */
    
    HAL_DMAEx_EnableMuxRequestGenerator (&DMA_Handle);                      /* ʹ��DMAMUX�������� */        
      
    /*##-4- ����DMA���� ################################################*/
    HAL_DMA_Start_IT(&DMA_Handle, (uint32_t)IO_Toggle, (uint32_t)&GPIOB->BSRRL, 8);
    
    /* 
       Ĭ������£��û�ͨ��ע��ص�����DMA_Handle.XferHalfCpltCallback��Ȼ����HAL_DMA_Init�Ὺ���봫������жϣ�
       ��������û��ʹ��HAL��Ĭ�ϵ��жϹ�����HAL_DMA_IRQHandler��ֱ���ֶ�������
    */
    BDMA_Channel0->CCR |= BDMA_CCR_HTIE;
    
    LPTIM_Config(); /* ����LPTIM����DMAMUX */
}

/*
*********************************************************************************************************
*	�� �� ��: BDMA_Channel0_IRQHandler
*	����˵��: BDMAͨ��0
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void BDMA_Channel0_IRQHandler(void)
{
    /* ��������ж� */
    if((BDMA->ISR & BDMA_FLAG_TC0) != RESET)
    {
        BDMA->IFCR = BDMA_FLAG_TC0;

        /*
           1��������ɿ�ʼʹ��DMA��������ǰ�벿�֣���ʱ���Զ�̬�޸ĺ�벿������
              ���绺������С��IO_Toggle[0] �� IO_Toggle[7]
              ��ô��ʱ�����޸�IO_Toggle[4] �� IO_Toggle[7]
           2���������ڵ�SRAM���Ѿ�ͨ��MPU����ΪWTģʽ�����±���IO_Toggle������д�롣
           3��������MPU�Ļ���Ҳ����ͨ��Cahce�ĺ���SCB_CleanDCache_by_Addr��Clean������
        */
    }
 
    /* �봫������ж� */    
    if((BDMA->ISR & BDMA_FLAG_HT0) != RESET)
    {
        BDMA->IFCR = BDMA_FLAG_HT0;
        
        /*
           1���봫����ɿ�ʼʹ��DMA�������ĺ�벿�֣���ʱ���Զ�̬�޸�ǰ�벿������
              ���绺������С��IO_Toggle[0] �� IO_Toggle[7]
              ��ô��ʱ�����޸�IO_Toggle[0] �� IO_Toggle[3]
           2���������ڵ�SRAM���Ѿ�ͨ��MPU����ΪWTģʽ�����±���IO_Toggle������д�롣
           3��������MPU�Ļ���Ҳ����ͨ��Cahce�ĺ���SCB_CleanDCache_by_Addr��Clean������
        */
    }
 
    /* ��������ж� */
    if((BDMA->ISR & BDMA_FLAG_TE0) != RESET)
    {
        BDMA->IFCR = BDMA_FLAG_TE0;
    }
}

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
