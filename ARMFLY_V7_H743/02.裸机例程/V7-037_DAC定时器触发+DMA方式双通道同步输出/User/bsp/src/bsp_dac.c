/*
*********************************************************************************************************
*
*	ģ������ : DAC����
*	�ļ����� : bsp_dac.c
*	��    �� : V1.0
*	˵    �� : DAC��ʱ������+DMA��ʽ˫ͨ��ͬ�����
*	�޸ļ�¼ :
*		�汾��   ����        ����     ˵��
*		V1.0    2019-06-01  armfly   ��ʽ����
*
*	Copyright (C), 2018-2030, ���������� www.armfly.com
*
*********************************************************************************************************
*/
#include "bsp.h"



/* ����Cache���API��������32�ֽڶ��� */
#if defined ( __ICCARM__ )
#pragma location = ".RAM_D3"  
ALIGN_32BYTES(uint16_t g_usWaveBuff[64]);

#elif defined ( __CC_ARM )
ALIGN_32BYTES(__attribute__((section (".RAM_D3"))) uint16_t g_usWaveBuff[64]);
#endif


/*
*********************************************************************************************************
*										����
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
*	�� �� ��: bsp_InitDAC
*	����˵��: DAC��ʼ��
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void bsp_InitDAC(void)
{   
    uint8_t i;

	/* һ�����ڵķ��� */
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
*	�� �� ��: TIM6_Config
*	����˵��: ���ö�ʱ��6�����ڴ���DAC��
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void TIM6_Config(void)
{
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
	
	TIM6 ���������� = TIM6CLK / ��Period + 1��/��Prescaler + 1��
	�������µ����ã����������ǣ�
	TIM6CLK /��Period + 1��/��Prescaler + 1��
	= 200MHz /��30+1��/��0+1��
	�� 6.45MHz
	----------------------------------------------------------------------- */
	TIM_MasterConfigTypeDef sMasterConfig;
	
		/* TIM6 ʱ��ʹ�� */
	__HAL_RCC_TIM6_CLK_ENABLE();

	/* ���ö�ʱ������ */
	htim.Instance = TIM6;

	htim.Init.Period            = 30;
	htim.Init.Prescaler         = 0;
	htim.Init.ClockDivision     = 0;
	htim.Init.CounterMode       = TIM_COUNTERMODE_UP;
	htim.Init.RepetitionCounter = 0;
	HAL_TIM_Base_Init(&htim);

	/* TIM6 TRGO ѡ�� */
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;

	HAL_TIMEx_MasterConfigSynchronization(&htim, &sMasterConfig);

	/* ʹ�ܶ�ʱ�� */
	HAL_TIM_Base_Start(&htim);
}

/*
*********************************************************************************************************
*	�� �� ��: DAC_WaveConfig
*	����˵��: DACͨ��1��ͨ��2�������
*             ���溯��TIM6_Config�ѽ���TIM6��������ó�6.45MHz������DMA�Ļ����С��64,��ô���������Ƶ�ʾ���
*             �������� = TIM6��������/64 = 6.45MHz / 64 �� 100KHz
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void DAC_WaveConfig(void)
{
	GPIO_InitTypeDef          GPIO_InitStruct;
	

	/*##-1- ʹ��ʱ�� #################################*/
    __HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_DAC12_CLK_ENABLE();
	__HAL_RCC_DMA1_CLK_ENABLE();

	/*##-2- ����GPIO ##########################################*/
	GPIO_InitStruct.Pin = GPIO_PIN_4;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    GPIO_InitStruct.Pin = GPIO_PIN_5;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  
	/*##-3- ��ʼ��DAC���� ######################################*/
    DacHandle.Instance = DAC1;
  
	HAL_DAC_DeInit(&DacHandle);	
    
	if (HAL_DAC_Init(&DacHandle) != HAL_OK)
	{
		Error_Handler(__FILE__, __LINE__);
	}

	/*##-4- ����DACͨ��1��ͨ��2 ######################################*/
	sConfig.DAC_SampleAndHold = DAC_SAMPLEANDHOLD_DISABLE;        /* �رղ�������ģʽ�����ģʽ��Ҫ���ڵ͹��� */
	sConfig.DAC_Trigger = DAC_TRIGGER_T6_TRGO;                    /* ���ö�ʱ��6���� */
	sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;           /* ʹ��������� */
	sConfig.DAC_ConnectOnChipPeripheral = DAC_CHIPCONNECT_DISABLE;/* ����DAC���ӵ�Ƭ������ */
	sConfig.DAC_UserTrimming = DAC_TRIMMING_FACTORY;              /* ʹ�ó���У׼ */

	if (HAL_DAC_ConfigChannel(&DacHandle, &sConfig, DAC_CHANNEL_1) != HAL_OK)
	{
		Error_Handler(__FILE__, __LINE__);
	}
    
    if (HAL_DAC_ConfigChannel(&DacHandle, &sConfig, DAC_CHANNEL_2) != HAL_OK)
	{
		Error_Handler(__FILE__, __LINE__);
	}
    
	/*##-5- ����ͨ��1��DMA ##########################################*/
	hdma_dac1.Instance = DMA1_Stream0;              /* ʹ�õ�DAM1 Stream0 */
	hdma_dac1.Init.Request  = DMA_REQUEST_DAC1;     /* DAC����DMA���� */
	hdma_dac1.Init.Direction = DMA_MEMORY_TO_PERIPH;/* �洢�������� */
	hdma_dac1.Init.PeriphInc = DMA_PINC_DISABLE;    /* �����ַ��ֹ���� */
	hdma_dac1.Init.MemInc = DMA_MINC_ENABLE;        /* �洢����ַ���� */
	hdma_dac1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD; /* ����������ݿ�ȣ����� */
	hdma_dac1.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;    /* �洢���������ݿ�ȣ����� */
	hdma_dac1.Init.Mode = DMA_CIRCULAR;                           /* ѭ��ģʽ */
	hdma_dac1.Init.Priority = DMA_PRIORITY_HIGH;                  /* ���ȼ��� */

	HAL_DMA_Init(&hdma_dac1);

	/* ����DMA�����DAC����� */
	__HAL_LINKDMA(&DacHandle, DMA_Handle1, hdma_dac1);

	/* ����DAC DMA */
	if (HAL_DAC_Start_DMA(&DacHandle, DAC_CHANNEL_1, (uint32_t *)g_usWaveBuff, 64, DAC_ALIGN_12B_R) != HAL_OK)
	{
		Error_Handler(__FILE__, __LINE__);
	} 
 
	/*##-6- ����ͨ��2��DMA ##########################################*/	
    hdma_dac2.Instance = DMA1_Stream1;                        /* ʹ�õ�DAM1 Stream1 */
	hdma_dac2.Init.Request  = DMA_REQUEST_DAC2;               /* DAC����DMA���� */
	hdma_dac2.Init.Direction = DMA_MEMORY_TO_PERIPH;          /* �洢�������� */
	hdma_dac2.Init.PeriphInc = DMA_PINC_DISABLE;              /* �����ַ��ֹ���� */
	hdma_dac2.Init.MemInc = DMA_MINC_ENABLE;                  /* �洢����ַ���� */
	hdma_dac2.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;/* ����������ݿ�ȣ����� */
	hdma_dac2.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;/* �洢���������ݿ�ȣ����� */
	hdma_dac2.Init.Mode = DMA_CIRCULAR;                       /* ѭ��ģʽ */
	hdma_dac2.Init.Priority = DMA_PRIORITY_HIGH;              /* ���ȼ��� */

	HAL_DMA_Init(&hdma_dac2);

	/* ����DMA�����DAC����� */
	__HAL_LINKDMA(&DacHandle, DMA_Handle2, hdma_dac2);

	/* ����DAC DMA */    
	if (HAL_DAC_Start_DMA(&DacHandle, DAC_CHANNEL_2, (uint32_t *)g_usWaveBuff, 64, DAC_ALIGN_12B_R) != HAL_OK)
	{
		Error_Handler(__FILE__, __LINE__);
	} 
}

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
