/*
*********************************************************************************************************
*
*	ģ������ : �߷ֱ��ʶ�ʱ��
*	�ļ����� : bsp_hrtim_pwm.c
*	��    �� : V1.0
*	˵    �� : �߷ֱ��ʶ�ʱ��HRTIM��PWM���
*
*	�޸ļ�¼ :
*		�汾��   ����        ����      ˵��
*		V1.0    2020-02-01  Eric2013  ��ʽ����
*
*	Copyright (C), 2020-2030, ���������� www.armfly.com
*
*********************************************************************************************************
*/
#include "bsp.h"



/*
	HRTIMʹ�����������������
	FTL = FAULT INPUT Lines
	PA15       HRTIM_FLT1
	PC11       HRTIM_FLT2
	PD4         HRTIM_FLT3
	PB3         HRTIM_FLT4
	PG10       HRTIM_FLT5

	EEV = EXTERN EVENT Lines
	PG13       HRTIM_EEV10
	PB7        HRTIM_EEV9
	PB6        HRTIM_EEV8
	PB5        HRTIM_EEV7
	PB4        HRTIM_EEV6
	PG12       HRTIM_EEV5
	PG11       HRTIM_EEV4
	PD5        HRTIM_EEV3
	PC12       HRTIM_EEV2
	PC10       HRTIM_EEV1

	PC6        HRTIM_CHA1  
	PC7        HRTIM_CHA2
	PC8        HRTIM_CHB1
	PA8        HRTIM_CHB2
	PA9        HRTIM_CHC1
	PA10       HRTIM_CHC2
	PA11       HRTIM_CHD1      
	PA12       HRTIM_CHD2
	PG6        HRTIM_CHE1
	PG7        HRTIM_CHE2

	PE0        HRTIM_SCIN
	PE1        HRTIM_SCOUT
	PB10       HRTIM_SCOUT
	PB11       HRTIM_SCIN
*/

/*
*********************************************************************************************************
*	                                   �궨��ͱ���
*********************************************************************************************************
*/
#define HRTIM_TIMD_PERIOD  4000

HRTIM_HandleTypeDef          HrtimHandle;
HRTIM_TimeBaseCfgTypeDef     sConfig_time_base;
HRTIM_TimerCfgTypeDef        sConfig_timerD;
HRTIM_OutputCfgTypeDef       sConfig_output_config;
HRTIM_CompareCfgTypeDef      sConfig_compare;


/*
*********************************************************************************************************
*	�� �� ��: bsp_SetHRTIMOutPWM
*	����˵��: ����HRTIM��TIMER D�����·PWM�����ڶ���100KHz��PA11�������ռ�ձ�50%��PA12���������ռ�ձ�25%��
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void bsp_SetHRTIMOutPWM(void)
{
	GPIO_InitTypeDef   GPIO_InitStruct;
	RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

	/*##-1- ʹ��ʱ�� ################################################*/
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_HRTIM1_CLK_ENABLE();

	
	/*##-2- ����HRTIM����Ƶʱ�� ######################################*/
	PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_HRTIM1;
	/*
		HRTIMʱ��������ѡ��
	    1��ʹ��CPU��Ƶʱ�ӣ����������õ���400MHz����Ӧ����RCC_HRTIM1CLK_CPUCLK��
	    2��ʹ��ͨ�ö�ʱ��ʱ�ӣ���������200MHz����Ӧ����RCC_HRTIM1CLK_TIMCLK��
	*/
	PeriphClkInitStruct.Hrtim1ClockSelection = RCC_HRTIM1CLK_CPUCLK;
	if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
	{
        Error_Handler(__FILE__, __LINE__);
	}

	/*##-3- ����HRTIM��TIMER Dʹ�õ��������ͨ������PA11��PA12 #######*/	
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

	GPIO_InitStruct.Alternate = GPIO_AF2_HRTIM1;
	GPIO_InitStruct.Pin = GPIO_PIN_11;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	GPIO_InitStruct.Alternate = GPIO_AF2_HRTIM1;
	GPIO_InitStruct.Pin = GPIO_PIN_12;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	/*##-4- ��ʼ��HRTIM ###################################################*/	
	HrtimHandle.Instance = HRTIM1;  /* ������ʹ�õ�HRTIM1 */
	HrtimHandle.Init.HRTIMInterruptResquests = HRTIM_IT_NONE; /* ��������֧�ֵ��ж����󣬵�ǰ�������ж� */
	HrtimHandle.Init.SyncOptions = HRTIM_SYNCOPTION_NONE;     /* ����HRTIM��ΪMaster������ͬ���źţ�������ΪSlave������ͬ���źţ���ǰ����û����ͬ������ */
	
	HAL_HRTIM_Init(&HrtimHandle);
    
	/*##-5- ����HRTIM��TIMER D ʱ�� #########################################*/	
	/*
		PWM��Ƶ�� = 400MHz / HRTIM_TIMD_PERIOD
	              = 400000000 / 4000
				  = 100KHz
	*/
	sConfig_time_base.Mode = HRTIM_MODE_CONTINUOUS; /* ��������ģʽ */
	sConfig_time_base.Period = HRTIM_TIMD_PERIOD;   /* �������� */
	sConfig_time_base.PrescalerRatio = HRTIM_PRESCALERRATIO_DIV1; /* ����HRTIM��Ƶ����ǰ���õ�1��Ƶ��Ҳ���ǲ���Ƶ */
	sConfig_time_base.RepetitionCounter = 0;        /* �����ظ�������Ϊ0���������ظ����� */
		  
	HAL_HRTIM_TimeBaseConfig(&HrtimHandle, HRTIM_TIMERINDEX_TIMER_D, &sConfig_time_base);

	/*##-6- HRTIM��TIMER D���� #############################################*/	
	sConfig_timerD.DMARequests = HRTIM_TIM_DMA_NONE;         /* ��ʹ��DMA */    
	sConfig_timerD.HalfModeEnable = HRTIM_HALFMODE_DISABLED; /* �ر�HALFģʽ */
	sConfig_timerD.StartOnSync = HRTIM_SYNCSTART_DISABLED;   /* ����ͬ������˽��յ��������źź󣬲�������ʱ�� */
	sConfig_timerD.ResetOnSync = HRTIM_SYNCRESET_DISABLED;   /* ����ͬ������˽��յ��������źź󣬲���λ��ʱ�� */
	sConfig_timerD.DACSynchro = HRTIM_DACSYNC_NONE;			 /* ��ʹ��DACͬ���¼� */
	sConfig_timerD.PreloadEnable = HRTIM_PRELOAD_ENABLED;	 /* ʹ�ܼĴ���Ԥ���� */
	sConfig_timerD.UpdateGating = HRTIM_UPDATEGATING_INDEPENDENT;      /* �������£���DMAͻ����������޹� */
	sConfig_timerD.BurstMode = HRTIM_TIMERBURSTMODE_MAINTAINCLOCK;     /* ��ͻ��ģʽ�£���ʱ���������� */
	sConfig_timerD.RepetitionUpdate = HRTIM_UPDATEONREPETITION_ENABLED;/* �����ؼ������¼����Դ����Ĵ������� */
	sConfig_timerD.ResetUpdate = HRTIM_TIMUPDATEONRESET_DISABLED;	   /* ��HRTIM TIMER�ļ�������λʱ���߼����ع���0ʱ���������Ĵ������� */
	sConfig_timerD.InterruptRequests = HRTIM_TIM_IT_NONE;              /* ��ʹ���ж� */
	sConfig_timerD.PushPull = HRTIM_TIMPUSHPULLMODE_DISABLED;          /* ����������ģʽ */
	sConfig_timerD.FaultEnable = HRTIM_TIMFAULTENABLE_NONE;            /* ��ʹ��HRTIM TIMER��Faultͨ�� */
	sConfig_timerD.FaultLock = HRTIM_TIMFAULTLOCK_READWRITE;           /* ������HRTIM TIMER���쳣ʹ��״̬д���� */
	sConfig_timerD.DeadTimeInsertion = HRTIM_TIMDEADTIMEINSERTION_DISABLED;/* ����������ʱ����� */
	sConfig_timerD.DelayedProtectionMode = HRTIM_TIMER_D_E_DELAYEDPROTECTION_DISABLED;/* ������HRTIM TIMER���ӳٱ���ģʽ */
	sConfig_timerD.UpdateTrigger= HRTIM_TIMUPDATETRIGGER_NONE;         /* Master����TIMER��A��E������ʱ����ͬ�����¼Ĵ��� */
	sConfig_timerD.ResetTrigger = HRTIM_TIMRESETTRIGGER_NONE;          /* �޸�λ���� */

	HAL_HRTIM_WaveformTimerConfig(&HrtimHandle, HRTIM_TIMERINDEX_TIMER_D, &sConfig_timerD);

	/*##-7- ����HRTIM��TIMER D�ıȽ���� ###########################*/	
	sConfig_compare.AutoDelayedMode = HRTIM_AUTODELAYEDMODE_REGULAR; /* ����ʹ�ñ�׼ģʽ����δʹ���Զ��ӳ� */
	sConfig_compare.AutoDelayedTimeout = 0;                          /* ����ǰ��Ĳ���δʹ���Զ��ӳ�ģʽ���˲��������� */
	/*
	    ���ö�ʱ���Ƚϵ�Ԫ�ıȽ�ֵ��
		��СֵҪ���ڵ���3��HRTIMʱ�����ڡ�
		���ֵҪС�ڵ���0xFFFF �C 1
	*/
	sConfig_compare.CompareValue = HRTIM_TIMD_PERIOD / 2;  /* ռ�ձ�50% */
	HAL_HRTIM_WaveformCompareConfig(&HrtimHandle, HRTIM_TIMERINDEX_TIMER_D, HRTIM_COMPAREUNIT_1, &sConfig_compare);
	sConfig_compare.CompareValue = HRTIM_TIMD_PERIOD / 4;  /* ռ�ձ�25% */
	HAL_HRTIM_WaveformCompareConfig(&HrtimHandle, HRTIM_TIMERINDEX_TIMER_D, HRTIM_COMPAREUNIT_2, &sConfig_compare);


	/*##-8- ��ʱ��������� ##########################################*/	
	sConfig_output_config.Polarity = HRTIM_OUTPUTPOLARITY_LOW;    /* ���ö�ʱ��������� */
	sConfig_output_config.SetSource = HRTIM_OUTPUTRESET_TIMCMP1;  /* ��ʱ���Ƚ��¼�1���Խ������λ */
	sConfig_output_config.ResetSource = HRTIM_OUTPUTSET_TIMPER;   /* ��ʱ�������Ը����¼����Խ�������� */
	sConfig_output_config.IdleMode = HRTIM_OUTPUTIDLEMODE_NONE;   /* �������ͻ��ģʽӰ�� */
	sConfig_output_config.IdleLevel = HRTIM_OUTPUTIDLELEVEL_INACTIVE; /* ���ÿ���״̬����͵�ƽ */
	sConfig_output_config.FaultLevel = HRTIM_OUTPUTFAULTLEVEL_NONE;   /* ��������쳣����Ӱ�� */
	sConfig_output_config.ChopperModeEnable = HRTIM_OUTPUTCHOPPERMODE_DISABLED; /* �ر�Chopperģʽ */
	sConfig_output_config.BurstModeEntryDelayed = HRTIM_OUTPUTBURSTMODEENTRY_REGULAR; /* ���ô�ͻ��ģʽ�л�������ģʽ������������ʱ�� */

	HAL_HRTIM_WaveformOutputConfig(&HrtimHandle, HRTIM_TIMERINDEX_TIMER_D, HRTIM_OUTPUT_TD1, &sConfig_output_config);
	
	sConfig_output_config.SetSource = HRTIM_OUTPUTRESET_TIMCMP2;  /* ��ʱ���Ƚ��¼�2���Խ������λ */    
	HAL_HRTIM_WaveformOutputConfig(&HrtimHandle, HRTIM_TIMERINDEX_TIMER_D, HRTIM_OUTPUT_TD2, &sConfig_output_config);
	
	/*##-9- ����PWM��� #############################################*/
	if (HAL_HRTIM_WaveformOutputStart(&HrtimHandle,  HRTIM_OUTPUT_TD1 + HRTIM_OUTPUT_TD2) != HAL_OK)
	{
        Error_Handler(__FILE__, __LINE__);
	}

	/*##-10- ���������� #############################################*/	
	if (HAL_HRTIM_WaveformCounterStart(&HrtimHandle, HRTIM_TIMERID_TIMER_D) != HAL_OK)
	{
        Error_Handler(__FILE__, __LINE__);
	}
	
}

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
