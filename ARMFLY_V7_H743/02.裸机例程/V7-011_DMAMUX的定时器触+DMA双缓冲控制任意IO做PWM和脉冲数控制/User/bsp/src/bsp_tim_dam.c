/*
*********************************************************************************************************
*
*	ģ������ : ��ʱ������DMA
*	�ļ����� : bsp_tim_dma.c
*	��    �� : V1.0
*	˵    �� : DMAMUX�Ķ�ʱ����+DMA˫�����������IO��PWM������������
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
*	�� �� ��: TIM12_Config
*	����˵��: ����TIM12�����ڴ���DMAMUX����������
*	��    ��: _Mode 
*             0 ��ʾ����Ϊ100KHz����Ƶ��,����Ϊ100KHz����Ƶ�ʣ����DMAMUX����Ϊ�����ش�������ô���PWMƵ
*               ����50KHz��˫������100KHz��
*			  1 ��ʾ����Ϊ10KHz����Ƶ�ʣ����DMAMUX����Ϊ�����ش�������ô���PWMƵ����5KHz��˫������10KHz��									  
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void TIM12_Config(uint8_t _Mode)
{
    TIM_HandleTypeDef  htim ={0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    TIM_OC_InitTypeDef sConfig = {0};
    uint32_t Period[2] = {1999, 19999};
    uint32_t Pulse[2]  = {999, 9999};

  	/* ʹ��ʱ�� */  
  	__HAL_RCC_TIM12_CLK_ENABLE();
      
    /*-----------------------------------------------------------------------
		bsp.c �ļ��� void SystemClock_Config(void) ������ʱ�ӵ���������: 

        System Clock source       = PLL (HSE)
        SYSCLK(Hz)                = 400000000 (CPU Clock)
        HCLK(Hz)                  = 200000000 (AXI and AHBs Clock)
        AHB Prescaler             = 2
        D1 APB3 Prescaler         = 2 (APB3 Clock  100MHz)
        D2 APB1 Prescaler         = 2 (APB1 Clock  100MHz)
        D2 APB2 Prescaler         = 2 (APB2 Clock  100MHz)
        D3 APB4 Prescaler         = 2 (APB4 Clock  100MHz)

        ��ΪAPB1 prescaler != 1, ���� APB1�ϵ�TIMxCLK = APB1 x 2 = 200MHz;
        ��ΪAPB2 prescaler != 1, ���� APB2�ϵ�TIMxCLK = APB2 x 2 = 200MHz;
        APB4�����TIMxCLKû�з�Ƶ�����Ծ���100MHz;

        APB1 ��ʱ���� TIM2, TIM3 ,TIM4, TIM5, TIM6, TIM7, TIM12, TIM13, TIM14��LPTIM1
        APB2 ��ʱ���� TIM1, TIM8 , TIM15, TIM16��TIM17

        APB4 ��ʱ���� LPTIM2��LPTIM3��LPTIM4��LPTIM5

    TIM12CLK = 200MHz/(Period + 1) / (Prescaler + 1)
    ����bsp_InitTimDMA1��DMAMUX1ѡ����ǵ����ش�����ÿ��ʱ�ӿ��Դ������Ρ�
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

    /* ռ�ձ�50% */
    sConfig.Pulse = Pulse[_Mode];  
    if(HAL_TIM_OC_ConfigChannel(&htim, &sConfig, TIM_CHANNEL_1) != HAL_OK)
    {
		Error_Handler(__FILE__, __LINE__);
    }

    /* ����OC1 */
    if(HAL_TIM_OC_Start(&htim, TIM_CHANNEL_1) != HAL_OK)
    {
		Error_Handler(__FILE__, __LINE__);
    }
    
    /* TIM12��TRGO���ڴ���DMAMUX���������� */
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_OC1REF;
    sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;

	HAL_TIMEx_MasterConfigSynchronization(&htim, &sMasterConfig);
}

/*
*********************************************************************************************************
*	�� �� ��: bsp_InitTimDMA
*	����˵��: ����DMAMUX�Ķ�ʱ����+DMA˫�����������IO��PWM������������
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void bsp_InitTimDMA(void)
{
    GPIO_InitTypeDef  GPIO_InitStruct;
    DMA_HandleTypeDef DMA_Handle = {0};
    HAL_DMA_MuxRequestGeneratorConfigTypeDef dmamux_ReqGenParams = {0};

    /*##-1- ����PB1����PWM��� ##################################################*/
    __HAL_RCC_GPIOB_CLK_ENABLE();
    GPIO_InitStruct.Pin = GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  
    /*##-2- ʹ��DMA1ʱ�Ӳ����� ##################################################*/
    __HAL_RCC_DMA1_CLK_ENABLE();
    DMA_Handle.Instance                 = DMA1_Stream1;            /* ʹ�õ�DMA1 Stream1 */
    DMA_Handle.Init.Request             = DMA_REQUEST_GENERATOR0;  /* �������Ͳ��õ�DMAMUX��������ͨ��0 */  
    DMA_Handle.Init.Direction           = DMA_MEMORY_TO_PERIPH;    /* ���䷽���ǴӴ洢�������� */  
    DMA_Handle.Init.PeriphInc           = DMA_PINC_DISABLE;        /* �����ַ������ֹ */ 
    DMA_Handle.Init.MemInc              = DMA_MINC_ENABLE;         /* �洢����ַ����ʹ�� */  
    DMA_Handle.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;     /* �������ݴ���λ��ѡ���֣���32bit */     
    DMA_Handle.Init.MemDataAlignment    = DMA_MDATAALIGN_WORD;     /* �洢�����ݴ���λ��ѡ���֣���32bit */    
    DMA_Handle.Init.Mode                = DMA_CIRCULAR;            /* ѭ��ģʽ */   
    DMA_Handle.Init.Priority            = DMA_PRIORITY_LOW;        /* ���ȼ��� */  
    DMA_Handle.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;    /* ��ֹFIFO*/
    DMA_Handle.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL; /* ��ֹFIFO��λ�������ã��������÷�ֵ */
    DMA_Handle.Init.MemBurst            = DMA_MBURST_SINGLE;       /* ��ֹFIFO��λ�������ã����ڴ洢��ͻ�� */
    DMA_Handle.Init.PeriphBurst         = DMA_PBURST_SINGLE;       /* ��ֹFIFO��λ�������ã���������ͻ�� */
 
    /* ��ʼ��DMA */
    if(HAL_DMA_Init(&DMA_Handle) != HAL_OK)
    {
		Error_Handler(__FILE__, __LINE__);     
    }

    /* ����DMA1 Stream1���ж� */
    HAL_NVIC_SetPriority(DMA1_Stream1_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(DMA1_Stream1_IRQn); 

    
    /*##-4- ����DMAMUX ###########################################################*/
    dmamux_ReqGenParams.SignalID  = HAL_DMAMUX1_REQ_GEN_TIM12_TRGO;         /* ���󴥷���ѡ��LPTIM2_OUT */
    dmamux_ReqGenParams.Polarity  = HAL_DMAMUX_REQ_GEN_RISING;              /* �����ش���  */
    dmamux_ReqGenParams.RequestNumber = 1;                                  /* �����󣬴������1��DMA���� */

    HAL_DMAEx_ConfigMuxRequestGenerator(&DMA_Handle, &dmamux_ReqGenParams); /* ����DMAMUX */
    HAL_DMAEx_EnableMuxRequestGenerator (&DMA_Handle);                      /* ʹ��DMAMUX�������� */   
 
    
    /*##-4- ����DMA˫���崫�� ################################################*/
    /*
        1���˺����Ὺ��DMA��TC��TE��DME�ж�
        2������û������˻ص�����DMA_Handle.XferHalfCpltCallback����ô����HAL_DMA_Init�Ὺ���봫������жϡ�
        3������û�ʹ����DMAMUX��ͬ��ģʽ���˺����Ὺ��ͬ������жϡ�
        4������û�ʹ����DMAMUX�������������˺����Ὺʼ������������жϡ�
    */
    HAL_DMAEx_MultiBufferStart_IT(&DMA_Handle, (uint32_t)IO_Toggle, (uint32_t)&GPIOB->BSRRL,(uint32_t)IO_Toggle1, 8);
    
    /* �ò������жϿ���ֱ�ӹر� */
    //DMA1_Stream1->CR &= ~DMA_IT_DME; 
    //DMA1_Stream1->CR &= ~DMA_IT_TE;
    //DMAMUX1_RequestGenerator0->RGCR &= ~DMAMUX_RGxCR_OIE;
    
    TIM12_Config(0);
}

/*
*********************************************************************************************************
*	�� �� ��: DMA1_Stream1_IRQHandler
*	����˵��: DMA1 Stream1�жϷ������
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void DMA1_Stream1_IRQHandler(void)
{
	/* ��������ж� */
	if((DMA1->LISR & DMA_FLAG_TCIF1_5) != RESET)
	{
		/* �����־ */
		DMA1->LIFCR = DMA_FLAG_TCIF1_5;

		/* ��ǰʹ�õĻ���0 */
		if((DMA1_Stream1->CR & DMA_SxCR_CT) == RESET)
		{
			/*
				1����ǰ����ʹ�û���0����ʱ���Զ�̬�޸Ļ���1�����ݡ�
				   ���绺����0��IO_Toggle��������1��IO_Toggle1����ô��ʱ�Ϳ����޸�IO_Toggle1��
				2���������ڵ�SRAM���Ѿ�ͨ��MPU����ΪWTģʽ�����±���IO_Toggle������д�롣
				3��������MPU�Ļ���Ҳ����ͨ��Cahce�ĺ���SCB_CleanDCache_by_Addr��Clean������
			*/
			
		}
		/* ��ǰʹ�õĻ���1 */
		else
		{
			 /*
			   1����ǰ����ʹ�û���1����ʱ���Զ�̬�޸Ļ���0�����ݡ�
				  ���绺����0��IO_Toggle��������1��IO_Toggle1����ô��ʱ�Ϳ����޸�IO_Toggle��
			   2���������ڵ�SRAM���Ѿ�ͨ��MPU����ΪWTģʽ�����±���IO_Toggle������д�롣
			   3��������MPU�Ļ���Ҳ����ͨ��Cahce�ĺ���SCB_CleanDCache_by_Addr��Clean������
			*/

		}
	}

	/* �봫������ж� */    
	if((DMA1->LISR & DMA_FLAG_HTIF1_5) != RESET)
	{
		/* �����־ */
		DMA1->LISR = DMA_FLAG_HTIF1_5;
	}

	/* ��������ж� */
	if((DMA1->LISR & DMA_FLAG_TEIF1_5) != RESET)
	{
		/* �����־ */
		DMA1->LISR = DMA_FLAG_TEIF1_5;
	}

	/* ֱ��ģʽ�����ж� */
	if((DMA1->LISR & DMA_FLAG_DMEIF1_5) != RESET)
	{
		/* �����־ */
		DMA1->LISR = DMA_FLAG_DMEIF1_5;
	}
}

/*
*********************************************************************************************************
*	�� �� ��: DMAMUX1_OVR_IRQHandler
*	����˵��: DMAMUX���жϷ�������������ڴ������������������
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void DMAMUX1_OVR_IRQHandler(void)
{
    if((DMAMUX1_RequestGenStatus->RGSR & DMAMUX_RGSR_OF0) != RESET)
    {
       /* �ر�����ж� */
       DMAMUX1_RequestGenerator0->RGCR &= ~DMAMUX_RGxCR_OIE;
       
       /* �����־ */
       DMAMUX1_RequestGenStatus->RGCFR = DMAMUX_RGSR_OF0;
    }
}

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
