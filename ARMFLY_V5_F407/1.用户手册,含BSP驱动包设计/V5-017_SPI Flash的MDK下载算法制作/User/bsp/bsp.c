/*
*********************************************************************************************************
*
*	ģ������ : BSPģ��(For STM32F407)
*	�ļ����� : bsp.c
*	��    �� : V1.0
*	˵    �� : ����Ӳ���ײ�������������ļ���ÿ��c�ļ����� #include "bsp.h" ���������е���������ģ�顣
*			   bsp = Borad surport packet �弶֧�ְ�
*	�޸ļ�¼ :
*		�汾��  ����         ����       ˵��
*		V1.0    2018-07-29  Eric2013   ��ʽ����
*
*	Copyright (C), 2018-2030, ���������� www.armfly.com
*
*********************************************************************************************************
*/
#include "bsp.h"



/*
*********************************************************************************************************
*	�� �� ��: HAL_InitTick
*	����˵��: �ض��򣬲�ʹ��
*	��    ��: TickPriority
*	�� �� ֵ: ��
*********************************************************************************************************
*/
HAL_StatusTypeDef HAL_InitTick(uint32_t TickPriority)
{
	return HAL_OK;
}

/*
*********************************************************************************************************
*	�� �� ��: SystemClock_Config
*	����˵��: ��ʼ��ϵͳʱ��
*            	System Clock source            = PLL (HSE)
*            	SYSCLK(Hz)                     = 168000000 (CPU Clock)
*            	HCLK = SYSCLK / 1              = 168000000 (AHB1Periph)
*            	PCLK2 = HCLK / 2               = 84000000  (APB2Periph)
*            	PCLK1 = HCLK / 4               = 42000000  (APB1Periph)
*            	HSE Frequency(Hz)              = 25000000
*           	PLL_M                          = 25
*            	PLL_N                          = 336
*            	PLL_P                          = 2
*            	PLL_Q                          = 4
*            	VDD(V)                         = 3.3
*            	Flash Latency(WS)              = 5
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
int SystemClock_Config(void)
{
	RCC_ClkInitTypeDef RCC_ClkInitStruct;
	RCC_OscInitTypeDef RCC_OscInitStruct;

	
	/* оƬ�ڲ���LDO��ѹ������ĵ�ѹ��Χ��ѡ�õ�PWR_REGULATOR_VOLTAGE_SCALE1 */
	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	/* ʹ��HSE����ѡ��HSE��ΪPLLʱ��Դ */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 25;
	RCC_OscInitStruct.PLL.PLLN = 336;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 4;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	{
        return 1;
	}

	/* 
       ѡ��PLL�������Ϊϵͳʱ��
		HCLK = SYSCLK / 1  (AHB1Periph)
		PCLK2 = HCLK / 2   (APB2Periph)
		PCLK1 = HCLK / 4   (APB1Periph)
    */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
								  |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

	/* �˺��������SystemCoreClock������������HAL_InitTick */
	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
	{
        return 1;
	}

    /* ʹ��SYSʱ�Ӻ�IO���� */
	__HAL_RCC_SYSCFG_CLK_ENABLE() ;

	HAL_EnableCompensationCell();
    
    return 0;
}

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
