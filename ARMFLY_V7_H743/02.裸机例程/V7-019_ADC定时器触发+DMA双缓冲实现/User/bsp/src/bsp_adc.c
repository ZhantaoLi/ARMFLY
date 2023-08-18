/*
*********************************************************************************************************
*
*	ģ������ : ADC����
*	�ļ����� : bsp_adc.c
*	��    �� : V1.0
*	˵    �� : ADC��ʱ������+DMA˫�����ʵ��
*              1. ����Ĭ���õ�AHBʱ�ӹ�ADCʹ�ã���ҿ���ͨ��bsp_adc.c�ļ���ͷ�궨���л���PLL2ר��ʱ�ӡ�
*              2��ʹ�õ�TIM1��OC1��ΪADC���ⲿ����Դ�������ٶ���100KHz����ADC�Ĳ�����Ҳ��100KHz��
*              3��ʹ��DMA�İ봫������жϺʹ�������ж�ʵ�����ݵ�˫������¡�
*              4���ɼ�����ʹ�õ�PC0�������ر�ע�⿪�����ϵ�Vref��ѹ��׼����ñ�̽ӵ�3.3V��
*
*	�޸ļ�¼ :
*		�汾��  ����        ����     ˵��
*		V1.0    2018-12-12 armfly  ��ʽ����
*
*	Copyright (C), 2018-2030, ���������� www.armfly.com
*
*********************************************************************************************************
*/
#include "bsp.h"



/* ѡ��ADC��ʱ��Դ */
#define ADC_CLOCK_SOURCE_AHB     /* ѡ��AHBʱ��Դ */
//#define ADC_CLOCK_SOURCE_PLL     /* ѡ��PLLʱ��Դ */


/* ����Cache���API��������32�ֽڶ��� */
#if defined ( __ICCARM__ )
#pragma location = 0x38000000
uint16_t ADCxValues[128];
#elif defined ( __CC_ARM )
ALIGN_32BYTES(__attribute__((section (".RAM_D3"))) uint16_t ADCxValues[128]);
#endif

__IO uint8_t s_DmaFlag = 0;  /* 1��ʾ��DMA�봫������жϣ�2��ʾ����DMA��������ж� */
DMA_HandleTypeDef   DmaHandle = {0};	

/*
*********************************************************************************************************
*	�� �� ��: TIM1_Config
*	����˵��: ����TIM1�����ڴ���ADC����ǰ���õ�100KHz����Ƶ��
*	��    ��: ��									  
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void TIM1_Config(void)
{
	TIM_HandleTypeDef  htim ={0};
	TIM_OC_InitTypeDef sConfig = {0};


	/* ʹ��ʱ�� */  
	__HAL_RCC_TIM1_CLK_ENABLE();
      
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

    /* ռ�ձ�50% */
    sConfig.Pulse = 1000;  
    if(HAL_TIM_OC_ConfigChannel(&htim, &sConfig, TIM_CHANNEL_1) != HAL_OK)
    {
		Error_Handler(__FILE__, __LINE__);
    }

    /* ����OC1 */
    if(HAL_TIM_OC_Start(&htim, TIM_CHANNEL_1) != HAL_OK)
    {
		Error_Handler(__FILE__, __LINE__);
    }
}

/*
*********************************************************************************************************
*	�� �� ��: bsp_InitADC
*	����˵��: ��ʼ��ADC
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void bsp_InitADC(void)
{
    ADC_HandleTypeDef   AdcHandle = {0};
	ADC_ChannelConfTypeDef   sConfig = {0};
	GPIO_InitTypeDef         GPIO_InitStruct;

	
  /* ## - 1 - ����ADC������ʱ�� ####################################### */
#if defined (ADC_CLOCK_SOURCE_PLL)
	/* ����PLL2ʱ��Ϊ��72MHz�������Ƶ����ADC���ʱ��36MHz */
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
  
  /* ʹ��AHBʱ�ӵĻ����������ã�Ĭ��ѡ��*/
  
#endif

	/* ## - 2 - ����ADC����ʹ�õ����� ####################################### */
	__HAL_RCC_GPIOC_CLK_ENABLE();

	GPIO_InitStruct.Pin = GPIO_PIN_0;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	/* ## - 3 - ����ADC����ʹ�õ�ʱ�� ####################################### */
	__HAL_RCC_DMA1_CLK_ENABLE();
	DmaHandle.Instance                 = DMA1_Stream1;            /* ʹ�õ�DMA1 Stream1 */
	DmaHandle.Init.Request             = DMA_REQUEST_ADC1;  	  /* �������Ͳ���DMA_REQUEST_ADC1 */  
	DmaHandle.Init.Direction           = DMA_PERIPH_TO_MEMORY;    /* ���䷽���Ǵ����赽�洢�� */  
	DmaHandle.Init.PeriphInc           = DMA_PINC_DISABLE;        /* �����ַ������ֹ */ 
	DmaHandle.Init.MemInc              = DMA_MINC_ENABLE;         /* �洢����ַ����ʹ�� */  
	DmaHandle.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;  /* �������ݴ���λ��ѡ����֣���16bit */     
	DmaHandle.Init.MemDataAlignment    = DMA_MDATAALIGN_HALFWORD;  /* �洢�����ݴ���λ��ѡ����֣���16bit */    
	DmaHandle.Init.Mode                = DMA_CIRCULAR;            /* ѭ��ģʽ */   
	DmaHandle.Init.Priority            = DMA_PRIORITY_LOW;        /* ���ȼ��� */  
	DmaHandle.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;    /* ��ֹFIFO*/
	DmaHandle.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL; /* ��ֹFIFO��λ�������ã��������÷�ֵ */
	DmaHandle.Init.MemBurst            = DMA_MBURST_SINGLE;       /* ��ֹFIFO��λ�������ã����ڴ洢��ͻ�� */
	DmaHandle.Init.PeriphBurst         = DMA_PBURST_SINGLE;       /* ��ֹFIFO��λ�������ã���������ͻ�� */
 
    /* ��ʼ��DMA */
    if(HAL_DMA_Init(&DmaHandle) != HAL_OK)
    {
		Error_Handler(__FILE__, __LINE__);     
    }
    
    /* ����DMA1 Stream1���ж� */
    HAL_NVIC_SetPriority(DMA1_Stream1_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(DMA1_Stream1_IRQn);
    
    /* ����ADC�����DMA��� */
	__HAL_LINKDMA(&AdcHandle, DMA_Handle, DmaHandle);

	/* ## - 4 - ����ADC ########################################################### */
	__HAL_RCC_ADC12_CLK_ENABLE();
	AdcHandle.Instance = ADC1;
	
#if defined (ADC_CLOCK_SOURCE_PLL)
	AdcHandle.Init.ClockPrescaler        = ADC_CLOCK_ASYNC_DIV2;          /* ����PLL�첽ʱ�ӣ�2��Ƶ����72MHz/2 = 36MHz */
#elif defined (ADC_CLOCK_SOURCE_AHB)
	AdcHandle.Init.ClockPrescaler        = ADC_CLOCK_SYNC_PCLK_DIV4;      /* ����AHBͬ��ʱ�ӣ�4��Ƶ����200MHz/4 = 50MHz */
#endif
	AdcHandle.Init.Resolution            = ADC_RESOLUTION_16B;            /* 16λ�ֱ��� */
	AdcHandle.Init.ScanConvMode          = ADC_SCAN_DISABLE;              /* ��ֹɨ�裬��Ϊ������һ��ͨ�� */
	AdcHandle.Init.EOCSelection          = ADC_EOC_SINGLE_CONV;           /* EOCת��������־ */
	AdcHandle.Init.LowPowerAutoWait      = DISABLE;                       /* ��ֹ�͹����Զ��ӳ����� */
	AdcHandle.Init.ContinuousConvMode    = DISABLE;                       /* ��ֹ�Զ�ת�������õĶ�ʱ������ת�� */
	AdcHandle.Init.NbrOfConversion       = 1;                             /* ʹ����1��ת��ͨ�� */
	AdcHandle.Init.DiscontinuousConvMode = DISABLE;                       /* ��ֹ������ģʽ */
	AdcHandle.Init.NbrOfDiscConversion   = 1;                             /* ��ֹ������ģʽ�󣬴˲������ԣ���λ���������ò�����������ͨ���� */
	AdcHandle.Init.ExternalTrigConv      = ADC_EXTERNALTRIG_T1_CC1;            /* ��ʱ��1��CC1���� */
	AdcHandle.Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_RISING;    /* �����ش��� */
	AdcHandle.Init.ConversionDataManagement = ADC_CONVERSIONDATA_DMA_CIRCULAR; /* DMAѭ��ģʽ����ADCת�������� */
	AdcHandle.Init.BoostMode                = ENABLE;                          /* ADCʱ�ӳ���20MHz�Ļ���ʹ��boost */
	AdcHandle.Init.Overrun                  = ADC_OVR_DATA_OVERWRITTEN;        /* ADCת������Ļ�������ADC�����ݼĴ��� */
	AdcHandle.Init.OversamplingMode         = DISABLE;                         /* ��ֹ������ */

    /* ��ʼ��ADC */
	if (HAL_ADC_Init(&AdcHandle) != HAL_OK)
	{
		Error_Handler(__FILE__, __LINE__);
	}
  
	/* У׼ADC������ƫ��У׼ */
	if (HAL_ADCEx_Calibration_Start(&AdcHandle, ADC_CALIB_OFFSET, ADC_SINGLE_ENDED) != HAL_OK)
	{
		Error_Handler(__FILE__, __LINE__);
	}
  
	/* ����ADCͨ��  */
	sConfig.Channel      = ADC_CHANNEL_10;              /* ����ʹ�õ�ADCͨ�� */
	sConfig.Rank         = ADC_REGULAR_RANK_1;          /* ����������ĵ�1�� */
	sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;     /* �������� */
	sConfig.SingleDiff   = ADC_SINGLE_ENDED;            /* �������� */
	sConfig.OffsetNumber = ADC_OFFSET_NONE;             /* ��ƫ�� */ 
	sConfig.Offset = 0;                                 /* ��ƫ�Ƶ�����£��˲������� */

	if (HAL_ADC_ConfigChannel(&AdcHandle, &sConfig) != HAL_OK)
	{
		Error_Handler(__FILE__, __LINE__);
	}
  
	/* ## - 5 - ����ADC�Ķ�ʱ������ ####################################### */
	TIM1_Config();
  
	/* ## - 6 - ����ADC��DMA��ʽ���� ####################################### */
	if (HAL_ADC_Start_DMA(&AdcHandle, (uint32_t *)ADCxValues, 128) != HAL_OK)
	{
		Error_Handler(__FILE__, __LINE__);
	}
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
		
        HC574_TogglePin(GPIO_PIN_23);
		
		/*
		   1��ʹ�ô˺���Ҫ�ر�ע�⣬��1��������ַҪ32�ֽڶ��룬��2������Ҫ��32�ֽڵ���������
		   2�����봫������жϣ���ǰDMA����ʹ�û�������ǰ�벿�֣��û����Բ�����벿�֡�
		*/
		SCB_InvalidateDCache_by_Addr((uint32_t *)(&ADCxValues[64]), 128);
		
		s_DmaFlag = 2;
		
		/* �����־ */
		DMA1->LIFCR = DMA_FLAG_TCIF1_5;
	}

	/* �봫������ж� */    
	if((DMA1->LISR & DMA_FLAG_HTIF1_5) != RESET)
	{
		/*
		   1��ʹ�ô˺���Ҫ�ر�ע�⣬��1��������ַҪ32�ֽڶ��룬��2������Ҫ��32�ֽڵ���������
		   2������봫������жϣ���ǰDMA����ʹ�û������ĺ�벿�֣��û����Բ���ǰ�벿�֡�
		*/
		SCB_InvalidateDCache_by_Addr((uint32_t *)(&ADCxValues[0]), 128);
		
		s_DmaFlag = 1;
        
		/* �����־ */
		DMA1->LIFCR = DMA_FLAG_HTIF1_5;
	}

	/* ��������ж� */
	if((DMA1->LISR & DMA_FLAG_TEIF1_5) != RESET)
	{
		/* �����־ */
		DMA1->LIFCR = DMA_FLAG_TEIF1_5;
	}

	/* ֱ��ģʽ�����ж� */
	if((DMA1->LISR & DMA_FLAG_DMEIF1_5) != RESET)
	{
		/* �����־ */
		DMA1->LIFCR = DMA_FLAG_DMEIF1_5;
	}
}

/*
*********************************************************************************************************
*	�� �� ��: bsp_GetAdcValues
*	����˵��: ��ȡADC�����ݲ���ӡ
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void bsp_GetAdcValues(void)
{
	uint32_t values;
	float  temp;
	
	/* ��ǰDMA�����Ǻ������壬��ȡǰ��������ǰ4����ֵ��ƽ�� */
	if(s_DmaFlag == 1)
	{
		DISABLE_INT();
		s_DmaFlag = 0;
		values = (ADCxValues[0] + ADCxValues[1] + ADCxValues[2] + ADCxValues[3])/4;

		ENABLE_INT();
	}
	/* ��ǰDMA�����Ǻ�ǰ�����壬��ȡ���������ǰ4����ֵ��ƽ�� */
	else if(s_DmaFlag == 2)
	{
		DISABLE_INT();
		s_DmaFlag = 0;
		values = (ADCxValues[64] + ADCxValues[65] + ADCxValues[66] + ADCxValues[67])/4;
		ENABLE_INT();
	}
	
	/* ��ӡ�����Ĵ���ֵ */
	temp = values *3.3 / 65536;
	
	printf("ADCxValues = %d, %5.3f\r\n", values, temp);
}

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
