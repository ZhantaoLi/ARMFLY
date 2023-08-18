/*
*********************************************************************************************************
*
*	模块名称 : 高分辨率定时器
*	文件名称 : bsp_hrtim_pwm.c
*	版    本 : V1.0
*	说    明 : 高分辨率定时器HRTIM的PWM输出
*
*	修改记录 :
*		版本号   日期        作者      说明
*		V1.0    2020-02-01  Eric2013  正式发布
*
*	Copyright (C), 2020-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/
#include "bsp.h"



/*
	HRTIM使用输入输出引脚整理：
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
*	                                   宏定义和变量
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
*	函 数 名: bsp_SetHRTIMOutPWM
*	功能说明: 配置HRTIM的TIMER D输出两路PWM，周期都是100KHz，PA11引脚输出占空比50%，PA12引脚输出的占空比25%。
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_SetHRTIMOutPWM(void)
{
	GPIO_InitTypeDef   GPIO_InitStruct;
	RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

	/*##-1- 使能时钟 ################################################*/
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_HRTIM1_CLK_ENABLE();

	
	/*##-2- 配置HRTIM的主频时钟 ######################################*/
	PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_HRTIM1;
	/*
		HRTIM时钟有两种选择：
	    1、使用CPU主频时钟，本程序配置的是400MHz，对应参数RCC_HRTIM1CLK_CPUCLK。
	    2、使用通用定时器时钟，本程序是200MHz，对应参数RCC_HRTIM1CLK_TIMCLK。
	*/
	PeriphClkInitStruct.Hrtim1ClockSelection = RCC_HRTIM1CLK_CPUCLK;
	if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
	{
        Error_Handler(__FILE__, __LINE__);
	}

	/*##-3- 配置HRTIM的TIMER D使用的两个输出通道引脚PA11和PA12 #######*/	
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

	GPIO_InitStruct.Alternate = GPIO_AF2_HRTIM1;
	GPIO_InitStruct.Pin = GPIO_PIN_11;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	GPIO_InitStruct.Alternate = GPIO_AF2_HRTIM1;
	GPIO_InitStruct.Pin = GPIO_PIN_12;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	/*##-4- 初始化HRTIM ###################################################*/	
	HrtimHandle.Instance = HRTIM1;  /* 例化，使用的HRTIM1 */
	HrtimHandle.Init.HRTIMInterruptResquests = HRTIM_IT_NONE; /* 用于配置支持的中断请求，当前配置无中断 */
	HrtimHandle.Init.SyncOptions = HRTIM_SYNCOPTION_NONE;     /* 配置HRTIM作为Master，发送同步信号，或者作为Slave，接收同步信号，当前配置没有做同步功能 */
	
	HAL_HRTIM_Init(&HrtimHandle);
    
	/*##-5- 配置HRTIM的TIMER D 时基 #########################################*/	
	/*
		PWM的频率 = 400MHz / HRTIM_TIMD_PERIOD
	              = 400000000 / 4000
				  = 100KHz
	*/
	sConfig_time_base.Mode = HRTIM_MODE_CONTINUOUS; /* 连续工作模式 */
	sConfig_time_base.Period = HRTIM_TIMD_PERIOD;   /* 设置周期 */
	sConfig_time_base.PrescalerRatio = HRTIM_PRESCALERRATIO_DIV1; /* 设置HRTIM分频，当前设置的1分频，也就是不分频 */
	sConfig_time_base.RepetitionCounter = 0;        /* 设置重复计数器为0，即不做重复计数 */
		  
	HAL_HRTIM_TimeBaseConfig(&HrtimHandle, HRTIM_TIMERINDEX_TIMER_D, &sConfig_time_base);

	/*##-6- HRTIM的TIMER D配置 #############################################*/	
	sConfig_timerD.DMARequests = HRTIM_TIM_DMA_NONE;         /* 不使用DMA */    
	sConfig_timerD.HalfModeEnable = HRTIM_HALFMODE_DISABLED; /* 关闭HALF模式 */
	sConfig_timerD.StartOnSync = HRTIM_SYNCSTART_DISABLED;   /* 设置同步输入端接收到上升沿信号后，不启动定时器 */
	sConfig_timerD.ResetOnSync = HRTIM_SYNCRESET_DISABLED;   /* 设置同步输入端接收到上升沿信号后，不复位定时器 */
	sConfig_timerD.DACSynchro = HRTIM_DACSYNC_NONE;			 /* 不使用DAC同步事件 */
	sConfig_timerD.PreloadEnable = HRTIM_PRELOAD_ENABLED;	 /* 使能寄存器预加载 */
	sConfig_timerD.UpdateGating = HRTIM_UPDATEGATING_INDEPENDENT;      /* 独立更新，与DMA突发传输完成无关 */
	sConfig_timerD.BurstMode = HRTIM_TIMERBURSTMODE_MAINTAINCLOCK;     /* 在突发模式下，定时器正常运行 */
	sConfig_timerD.RepetitionUpdate = HRTIM_UPDATEONREPETITION_ENABLED;/* 设置重计数器事件可以触发寄存器更新 */
	sConfig_timerD.ResetUpdate = HRTIM_TIMUPDATEONRESET_DISABLED;	   /* 当HRTIM TIMER的计数器复位时或者计数回滚到0时，不触发寄存器更新 */
	sConfig_timerD.InterruptRequests = HRTIM_TIM_IT_NONE;              /* 不使用中断 */
	sConfig_timerD.PushPull = HRTIM_TIMPUSHPULLMODE_DISABLED;          /* 不开启推挽模式 */
	sConfig_timerD.FaultEnable = HRTIM_TIMFAULTENABLE_NONE;            /* 不使用HRTIM TIMER的Fault通道 */
	sConfig_timerD.FaultLock = HRTIM_TIMFAULTLOCK_READWRITE;           /* 不开启HRTIM TIMER的异常使能状态写保护 */
	sConfig_timerD.DeadTimeInsertion = HRTIM_TIMDEADTIMEINSERTION_DISABLED;/* 不开启死区时间插入 */
	sConfig_timerD.DelayedProtectionMode = HRTIM_TIMER_D_E_DELAYEDPROTECTION_DISABLED;/* 不开启HRTIM TIMER的延迟保护模式 */
	sConfig_timerD.UpdateTrigger= HRTIM_TIMUPDATETRIGGER_NONE;         /* Master或者TIMER（A到E）更新时，不同步更新寄存器 */
	sConfig_timerD.ResetTrigger = HRTIM_TIMRESETTRIGGER_NONE;          /* 无复位触发 */

	HAL_HRTIM_WaveformTimerConfig(&HrtimHandle, HRTIM_TIMERINDEX_TIMER_D, &sConfig_timerD);

	/*##-7- 配置HRTIM的TIMER D的比较输出 ###########################*/	
	sConfig_compare.AutoDelayedMode = HRTIM_AUTODELAYEDMODE_REGULAR; /* 这里使用标准模式，即未使用自动延迟 */
	sConfig_compare.AutoDelayedTimeout = 0;                          /* 由于前面的参数未使用自动延迟模式，此参数无作用 */
	/*
	    设置定时器比较单元的比较值：
		最小值要大于等于3个HRTIM时钟周期。
		最大值要小于等于0xFFFF – 1
	*/
	sConfig_compare.CompareValue = HRTIM_TIMD_PERIOD / 2;  /* 占空比50% */
	HAL_HRTIM_WaveformCompareConfig(&HrtimHandle, HRTIM_TIMERINDEX_TIMER_D, HRTIM_COMPAREUNIT_1, &sConfig_compare);
	sConfig_compare.CompareValue = HRTIM_TIMD_PERIOD / 4;  /* 占空比25% */
	HAL_HRTIM_WaveformCompareConfig(&HrtimHandle, HRTIM_TIMERINDEX_TIMER_D, HRTIM_COMPAREUNIT_2, &sConfig_compare);


	/*##-8- 定时器输出配置 ##########################################*/	
	sConfig_output_config.Polarity = HRTIM_OUTPUTPOLARITY_LOW;    /* 设置定时器输出极性 */
	sConfig_output_config.SetSource = HRTIM_OUTPUTRESET_TIMCMP1;  /* 定时器比较事件1可以将输出置位 */
	sConfig_output_config.ResetSource = HRTIM_OUTPUTSET_TIMPER;   /* 定时器周期性更新事件可以将输出清零 */
	sConfig_output_config.IdleMode = HRTIM_OUTPUTIDLEMODE_NONE;   /* 输出不受突发模式影响 */
	sConfig_output_config.IdleLevel = HRTIM_OUTPUTIDLELEVEL_INACTIVE; /* 设置空闲状态输出低电平 */
	sConfig_output_config.FaultLevel = HRTIM_OUTPUTFAULTLEVEL_NONE;   /* 输出不受异常输入影响 */
	sConfig_output_config.ChopperModeEnable = HRTIM_OUTPUTCHOPPERMODE_DISABLED; /* 关闭Chopper模式 */
	sConfig_output_config.BurstModeEntryDelayed = HRTIM_OUTPUTBURSTMODEENTRY_REGULAR; /* 设置从突发模式切换到空闲模式，不插入死区时间 */

	HAL_HRTIM_WaveformOutputConfig(&HrtimHandle, HRTIM_TIMERINDEX_TIMER_D, HRTIM_OUTPUT_TD1, &sConfig_output_config);
	
	sConfig_output_config.SetSource = HRTIM_OUTPUTRESET_TIMCMP2;  /* 定时器比较事件2可以将输出置位 */    
	HAL_HRTIM_WaveformOutputConfig(&HrtimHandle, HRTIM_TIMERINDEX_TIMER_D, HRTIM_OUTPUT_TD2, &sConfig_output_config);
	
	/*##-9- 启动PWM输出 #############################################*/
	if (HAL_HRTIM_WaveformOutputStart(&HrtimHandle,  HRTIM_OUTPUT_TD1 + HRTIM_OUTPUT_TD2) != HAL_OK)
	{
        Error_Handler(__FILE__, __LINE__);
	}

	/*##-10- 启动计数器 #############################################*/	
	if (HAL_HRTIM_WaveformCounterStart(&HrtimHandle, HRTIM_TIMERID_TIMER_D) != HAL_OK)
	{
        Error_Handler(__FILE__, __LINE__);
	}
	
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
