/*
*********************************************************************************************************
*
*	模块名称 : 低功耗定时器驱动
*	文件名称 : bsp_lptim_pwm.c
*	版    本 : V1.0
*	说    明 : 利用STM32H7内部LPTIM输出PWM
*	修改记录 :
*		版本号  日期        作者     说明
*		V1.0    2018-12-12 Eric2013  正式发布
*
*	Copyright (C), 2018-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#include "bsp.h"


/*
	LPTIM输入输出所复用的GPIO:
	LPTIM1_IN1   PD12   PG12
	LPTIM1_IN2   PH2    PE1
	LPTIM1_OUT   PG13
	LPTIM1_OUT   PD13
	LPTIM1_ETR   PG14   PE0

	LPTIM2_IN1   PB10  PD12
	LPTIM2_IN2   PD11
	LPTIM2_OUT   PB13
	LPTIM2_ETR   PB11  PE0

	LPTIM3_OUT   PA1
	LPTIM4_OUT   PA2
	LPTIM5_OUT   PA3
*/

/* 选择LPTIM的时钟源 */
#define LPTIM_CLOCK_SOURCE_LSE     /* LSE 时钟32768Hz */
//#define LPTIM_CLOCK_SOURCE_LSI   /* LSI 时钟约32KHz */ 
//#define LPTIM_CLOCK_SOURCE_PCLK  /* PCLK 时钟100MHz */ 

/*
*********************************************************************************************************
*	函 数 名: bsp_InitLPTIMOutPWM
*	功能说明: LPTIM1时钟默认选择的LSE，而PWM输出使用的PD13引脚，频率1024Hz。
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_InitLPTIMOutPWM(void)
{
    LPTIM_HandleTypeDef        LptimHandle = {0};	
	RCC_PeriphCLKInitTypeDef   RCC_PeriphCLKInitStruct = {0};
	GPIO_InitTypeDef   		   GPIO_InitStruct = {0};

	/* ## - 1 - 使能LPTIM时钟和GPIO时钟 ####################################### */
	__HAL_RCC_LPTIM1_CLK_ENABLE();

	__HAL_RCC_GPIOD_CLK_ENABLE();

	/* ## - 2 - 配置PD13做PWM输出 ############################################ */	
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF1_LPTIM1;
	GPIO_InitStruct.Pin = GPIO_PIN_13;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

	/* ## - 3 - 配置LPTIM时钟，可以选择LSE，LSI或者PCLK ######################## */		
#if defined (LPTIM_CLOCK_SOURCE_LSE)
	{
		RCC_OscInitTypeDef RCC_OscInitStruct = {0};

		RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE;
		RCC_OscInitStruct.LSEState = RCC_LSE_ON;
		RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;

		if (HAL_RCC_OscConfig(&RCC_OscInitStruct)!= HAL_OK)
		{
			Error_Handler(__FILE__, __LINE__);		
		}
		
		RCC_PeriphCLKInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LPTIM1;
		RCC_PeriphCLKInitStruct.Lptim1ClockSelection = RCC_LPTIM1CLKSOURCE_LSE;
		HAL_RCCEx_PeriphCLKConfig(&RCC_PeriphCLKInitStruct);
	}
#elif defined (LPTIM_CLOCK_SOURCE_LSI)
	{
		RCC_OscInitTypeDef RCC_OscInitStruct = {0};

		RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI;
		RCC_OscInitStruct.LSIState = RCC_LSI_ON;
		RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;

		if (HAL_RCC_OscConfig(&RCC_OscInitStruct)!= HAL_OK)
		{
			Error_Handler(__FILE__, __LINE__);		
		}
		
		RCC_PeriphCLKInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LPTIM1;
		RCC_PeriphCLKInitStruct.Lptim1ClockSelection = RCC_LPTIM1CLKSOURCE_LSI;
		HAL_RCCEx_PeriphCLKConfig(&RCC_PeriphCLKInitStruct);
	}
#elif defined (LPTIM_CLOCK_SOURCE_PCLK)
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

        因为APB1 prescaler != 1, 所以 APB1上的TIMxCLK = APB1 x 2 = 200MHz; 不含这个总线下的LPTIM1
        因为APB2 prescaler != 1, 所以 APB2上的TIMxCLK = APB2 x 2 = 200MHz;
        APB4上面的TIMxCLK没有分频，所以就是100MHz;

        APB1 定时器有 TIM2, TIM3 ,TIM4, TIM5, TIM6, TIM7, TIM12, TIM13, TIM14，LPTIM1
        APB2 定时器有 TIM1, TIM8 , TIM15, TIM16，TIM17

        APB4 定时器有 LPTIM2，LPTIM3，LPTIM4，LPTIM5
	----------------------------------------------------------------------- */
	RCC_PeriphCLKInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LPTIM1;
	RCC_PeriphCLKInitStruct.Lptim1ClockSelection = RCC_LPTIM1CLKSOURCE_D2PCLK1;
	HAL_RCCEx_PeriphCLKConfig(&RCC_PeriphCLKInitStruct);
#else
	#error Please select the LPTIM Clock source inside the bsp_lptim_pwm.c file
#endif

	/* ## - 4 - 配置LPTIM ######################################################## */		
	LptimHandle.Instance = LPTIM1;

	LptimHandle.Init.Clock.Source    = LPTIM_CLOCKSOURCE_APBCLOCK_LPOSC; /* 对应寄存器CKSEL，选择内部时钟源 */
	LptimHandle.Init.Clock.Prescaler = LPTIM_PRESCALER_DIV1;        /* 设置LPTIM时钟分频 */
	LptimHandle.Init.CounterSource   = LPTIM_COUNTERSOURCE_INTERNAL;/* LPTIM计数器对内部时钟源计数 */
	LptimHandle.Init.Trigger.Source  = LPTIM_TRIGSOURCE_SOFTWARE;   /* 软件触发 */ 
	LptimHandle.Init.OutputPolarity  = LPTIM_OUTPUTPOLARITY_HIGH;   /* 计数器计数到比较寄存器和ARR自动重载寄存器之间数值，输出高电平 */
	LptimHandle.Init.UpdateMode      = LPTIM_UPDATE_IMMEDIATE;      /* 比较寄存器和ARR自动重载寄存器选择更改后立即更新 */ 
	LptimHandle.Init.Input1Source    = LPTIM_INPUT1SOURCE_GPIO;     /* 外部输入1，本配置未使用 */
	LptimHandle.Init.Input2Source    = LPTIM_INPUT2SOURCE_GPIO;     /* 外部输入2，本配置未使用 */

	if (HAL_LPTIM_Init(&LptimHandle) != HAL_OK)
	{
		Error_Handler(__FILE__, __LINE__);
	}
    

	/* ## - 5 - 启动LPTIM的PWM模式 ######################################################## */	
	/*
	   ARR是自动重装寄存器，对应函数HAL_LPTIM_PWM_Start的第2个参数
	   Compare是比较寄存器，对应函数HAL_LPTIM_PWM_Start的第3个参数

	   ---------------------
	   LSE = 32768Hz
	   分频设置为LPTIM_PRESCALER_DIV1，即未分频
	   ARR自动重载寄存器 = 31
	   那么PWM频率 = LSE / （ARR + 1） = 32768Hz / (31 + 1) = 1024Hz
	
	   占空比 = 1 - (Comprare + 1)/ (ARR + 1)
	          = 1 - (15 + 1)/(31 + 1)
	          = 50%
	
	   占空比这里为什么要1减操作，而不是直接的(Comprare + 1)/ (ARR + 1)，这是因为前面的配置中
	   计数器计数到比较寄存器和ARR自动重载寄存器之间数值，输出高电平。
	*/
	if (HAL_LPTIM_PWM_Start(&LptimHandle, 31, 15) != HAL_OK)
	{
		Error_Handler(__FILE__, __LINE__);
	}
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
