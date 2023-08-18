/*
*********************************************************************************************************
*
*	ģ������ : AD7606���ݲɼ�ģ��
*	�ļ����� : bsp_fsmc_ad7606.c
*	��    �� : V1.0
*	˵    �� : AD7606����STM32��FMC�����ϣ�������ʹ����TIM8��ΪӲ����ʱ������ʱ����ADCת����
*              AD7606��FMC�����������ֲɼ���ʽ��
*              ��1�������ѯ��ʽ���ʺϵ��ٲ�ѯ��ȡ��
*              ��2��FIFO����ģʽ���ʺ�8·ʵʱ�ɼ���֧����߲�����200Ksps��
*
*	�޸ļ�¼ :
*		�汾��  ����        ����     ˵��
*		V1.0    2020-05-01 armfly  ��ʽ����
*
*	Copyright (C), 2020-2030, ���������� www.armfly.com
*
*********************************************************************************************************
*/
#include "bsp.h"



/*
	STM32-V6������ + AD7606ģ�飬 ���Ʋɼ���IO:
	
	PH9/DCMI_D0/AD7606_OS0			---> AD7606_OS0		OS2:OS0 ѡ�������˲�����
	PH10/DCMI_D1/AD7606_OS1         ---> AD7606_OS1
	PH11/DCMI_D2/AD7606_OS2         ---> AD7606_OS2
	PH14/DCMI_D4/AD7606_RAGE        ---> AD7606_RAGE	����ģ���ѹ���̣�����5V������10V
	PI4/DCMI_D5/AD7606_RESET        ---> AD7606_RESET	��λ
	PI6/DCMI_D6/AD7606_BUSY         ---> AD7606_BUSY	æ�ź�	(δʹ��)
	PH12/TIM5_CH3/DCMI_D3/AD7606_CONVST   ---> AD7606_CONVST	����ADCת�� (CONVSTA �� CONVSTB �Ѿ�����)
*/

/* CONVST ����ADCת����GPIO = PH12 */
#define CONVST_RCC_GPIO_CLK_ENABLE	__HAL_RCC_GPIOH_CLK_ENABLE
#define CONVST_TIM8_CLK_DISABLE     __HAL_RCC_TIM5_CLK_DISABLE
#define CONVST_GPIO		GPIOH
#define CONVST_PIN		GPIO_PIN_12
#define CONVST_TIMX		TIM5
#define CONVST_TIMCH	3

/* ����ADת�� */
#define CONVST_1()		CONVST_GPIO->BSRR = CONVST_PIN
#define CONVST_0()		CONVST_GPIO->BSRR = ((uint32_t)CONVST_PIN << 16U)

/* BUSY ת������ź� = PI6 */
#define BUSY_RCC_GPIO_CLK_ENABLE __HAL_RCC_GPIOI_CLK_ENABLE
#define BUSY_GPIO		GPIOI
#define BUSY_PIN		GPIO_PIN_6
#define BUSY_IRQn		EXTI9_5_IRQn
#define BUSY_IRQHandler	EXTI9_5_IRQHandler

/* ���ù�������GPIO: PH9 PH10 PH11 */
#define ALL_OS_GPIO_CLK_ENABLE() {	   \
	    __HAL_RCC_GPIOH_CLK_ENABLE();	\
	};

#define OS0_GPIO	GPIOH
#define OS0_PIN		GPIO_PIN_9
	
#define OS1_GPIO	GPIOH
#define OS1_PIN		GPIO_PIN_10
	
#define OS2_GPIO	GPIOH
#define OS2_PIN		GPIO_PIN_11

#define OS0_1()		OS0_GPIO->BSRR = OS0_PIN
#define OS0_0()		OS0_GPIO->BSRR= ((uint32_t)OS0_PIN << 16U)
#define OS1_1()		OS1_GPIO->BSRR = OS1_PIN
#define OS1_0()		OS1_GPIO->BSRR = ((uint32_t)OS1_PIN << 16U)
#define OS2_1()		OS2_GPIO->BSRR = OS2_PIN
#define OS2_0()		OS2_GPIO->BSRR = ((uint32_t)OS2_PIN << 16U)

/* �����������̵�GPIO : PH14  */
#define RANGE_RCC_GPIO_CLK_ENABLE __HAL_RCC_GPIOH_CLK_ENABLE
#define RANGE_GPIO		GPIOH
#define RANGE_PIN		GPIO_PIN_14
	
#define RANGE_1()	RANGE_GPIO->BSRR = RANGE_PIN
#define RANGE_0()	RANGE_GPIO->BSRR = ((uint32_t)RANGE_PIN << 16U)

/* AD7606��λ���� : PI4  */
#define RESET_RCC_GPIO_CLK_ENABLE __HAL_RCC_GPIOI_CLK_ENABLE
#define RESET_GPIO		GPIOI
#define RESET_PIN		GPIO_PIN_4

#define RESET_1()	RESET_GPIO->BSRR = RESET_PIN
#define RESET_0()	RESET_GPIO->BSRR = ((uint32_t)RESET_PIN << 16U)

/* AD7606 FSMC���ߵ�ַ��ֻ�ܶ�������д */
#define AD7606_RESULT()	*(__IO uint16_t *)0x6C400000

AD7606_VAR_T g_tAD7606;		/* ����1��ȫ�ֱ���������һЩ���� */
AD7606_FIFO_T g_tAdcFifo;	/* ����FIFO�ṹ����� */

static void AD7606_CtrlLinesConfig(void);
static void AD7606_FSMCConfig(void);


/*
*********************************************************************************************************
*	�� �� ��: bsp_InitAD7606
*	����˵��: ���������ⲿSRAM��GPIO��FSMC
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void bsp_InitAD7606(void)
{
	AD7606_CtrlLinesConfig();
	AD7606_FSMCConfig();

	AD7606_SetOS(AD_OS_NO);		/* �޹����� */
	AD7606_SetInputRange(0);	/* 0��ʾ��������Ϊ����5V, 1��ʾ����10V */
	AD7606_Reset();				/* ��λ */
	CONVST_1();					/* ����ת����GPIO��ƽʱ����Ϊ�� */
}

/*
*********************************************************************************************************
*	�� �� ��: AD7606_CtrlLinesConfig
*	����˵��: ����GPIO���ߣ�FMC�ܽ�����Ϊ���ù���
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
/*
	������STM32-V5��������߷�����

	PD0/FSMC_D2
	PD1/FSMC_D3
	PD4/FSMC_NOE		--- �������źţ�OE = Output Enable �� N ��ʾ����Ч
	PD5/FSMC_NWE		--- д�����źţ�AD7606 ֻ�ж�����д�ź�
	PD8/FSMC_D13
	PD9/FSMC_D14
	PD10/FSMC_D15

	PD14/FSMC_D0
	PD15/FSMC_D1

	PE4/FSMC_A20		--- ����Ƭѡһ������
	PE5/FSMC_A21		--- ����Ƭѡһ������
	PE7/FSMC_D4
	PE8/FSMC_D5
	PE9/FSMC_D6
	PE10/FSMC_D7
	PE11/FSMC_D8
	PE12/FSMC_D9
	PE13/FSMC_D10
	PE14/FSMC_D11
	PE15/FSMC_D12

	PG12/FSMC_NE4		--- ��Ƭѡ��TFT, OLED �� AD7606��

	�����Ŀ���IO:

	PH9/DCMI_D0/AD7606_OS0			---> AD7606_OS0		OS2:OS0 ѡ�������˲�����
	PH10/DCMI_D1/AD7606_OS1         ---> AD7606_OS1
	PH11/DCMI_D2/AD7606_OS2         ---> AD7606_OS2
	PH12/DCMI_D3/AD7606_CONVST      ---> AD7606_CONVST	����ADCת�� (CONVSTA �� CONVSTB �Ѿ�����)
	PH14/DCMI_D4/AD7606_RAGE        ---> AD7606_RAGE	����ģ���ѹ���̣�����5V������10V
	PI4/DCMI_D5/AD7606_RESET        ---> AD7606_RESET	��λ
	PI6/DCMI_D6/AD7606_BUSY         ---> AD7606_BUSY	æ�ź�	(δʹ��)
*/

static void AD7606_CtrlLinesConfig(void)
{

	GPIO_InitTypeDef gpio_init_structure;

	/* ʹ�� GPIOʱ�� */
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOE_CLK_ENABLE();
	__HAL_RCC_GPIOF_CLK_ENABLE();
	__HAL_RCC_GPIOG_CLK_ENABLE();
	__HAL_RCC_GPIOH_CLK_ENABLE();
	__HAL_RCC_GPIOI_CLK_ENABLE();

	/* ʹ��FMCʱ�� */
	__HAL_RCC_FSMC_CLK_ENABLE();

	/* ���� GPIOD ��ص�IOΪ����������� */
	gpio_init_structure.Mode = GPIO_MODE_AF_PP;
	gpio_init_structure.Pull = GPIO_PULLUP;
	gpio_init_structure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	gpio_init_structure.Alternate = GPIO_AF12_FSMC;
	
	/* ����GPIOD */
	gpio_init_structure.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_4 | GPIO_PIN_5  |
	                            GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_14 |
	                            GPIO_PIN_15;
	HAL_GPIO_Init(GPIOD, &gpio_init_structure);

	/* ����GPIOE */
	gpio_init_structure.Pin = GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 |
	                          GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
	HAL_GPIO_Init(GPIOE, &gpio_init_structure);

	/* ����GPIOG */
	gpio_init_structure.Pin = GPIO_PIN_12;
	HAL_GPIO_Init(GPIOG, &gpio_init_structure);


	/*	���ü��������õ�GPIO
		PH9/DCMI_D0/AD7606_OS0			---> AD7606_OS0		OS2:OS0 ѡ�������˲�����
		PH10/DCMI_D1/AD7606_OS1         ---> AD7606_OS1
		PH11/DCMI_D2/AD7606_OS2         ---> AD7606_OS2
		PH12/DCMI_D3/AD7606_CONVST      ---> AD7606_CONVST	����ADCת��
		PH14/DCMI_D4/AD7606_RAGE        ---> AD7606_RAGE	����ģ���ѹ���̣�����5V������10V
		PI4/DCMI_D5/AD7606_RESET        ---> AD7606_RESET	��λ

		PI6/DCMI_D6/AD7606_BUSY			---> AD7606_BUSY    ת���������ź�
	*/
	
	/* ����BUSY���ţ�Ĭ������ͨIO״̬ */
	{
		GPIO_InitTypeDef   GPIO_InitStructure;
	
		__HAL_RCC_SYSCFG_CLK_ENABLE();
				
		BUSY_RCC_GPIO_CLK_ENABLE();		/* ��GPIOʱ�� */

		/* BUSY�źţ�ʹ�õ�PI6������ת����ϼ�� */
		GPIO_InitStructure.Mode = GPIO_MODE_INPUT;   /* ����������� */
		GPIO_InitStructure.Pull = GPIO_NOPULL;       /* ���������� */
		GPIO_InitStructure.Pin = BUSY_PIN;           
		HAL_GPIO_Init(BUSY_GPIO, &GPIO_InitStructure);	
	}
	
	/* ����IO */
	{
		GPIO_InitTypeDef   GPIO_InitStructure;
		ALL_OS_GPIO_CLK_ENABLE();
		RANGE_RCC_GPIO_CLK_ENABLE();
		RESET_RCC_GPIO_CLK_ENABLE();
		CONVST_RCC_GPIO_CLK_ENABLE();
		
		GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;		/* ����������� */
		GPIO_InitStructure.Pull = GPIO_PULLUP;				/* ���������費ʹ�� */
		GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;    /* GPIO�ٶȵȼ� */	

		GPIO_InitStructure.Pin = CONVST_PIN;	
		HAL_GPIO_Init(CONVST_GPIO, &GPIO_InitStructure);

		GPIO_InitStructure.Pin = OS0_PIN;	
		HAL_GPIO_Init(OS0_GPIO, &GPIO_InitStructure);	

		GPIO_InitStructure.Pin = OS1_PIN;	
		HAL_GPIO_Init(OS1_GPIO, &GPIO_InitStructure);	

		GPIO_InitStructure.Pin = OS2_PIN;	
		HAL_GPIO_Init(OS2_GPIO, &GPIO_InitStructure);	

		GPIO_InitStructure.Pin = RANGE_PIN;	
		HAL_GPIO_Init(RANGE_GPIO, &GPIO_InitStructure);	
		
		GPIO_InitStructure.Pin = RESET_PIN;	
		HAL_GPIO_Init(RESET_GPIO, &GPIO_InitStructure);	
	}
}

/*
*********************************************************************************************************
*	�� �� ��: AD7606_FSMCConfig
*	����˵��: ����FSMC���ڷ���ʱ��
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void AD7606_FSMCConfig(void)
{
	/* 
	   TFT-LCD��OLED��AD7606����һ��FMC���ã����������������FMC�ٶ�������Ϊ׼��
	   �Ӷ���֤�������趼��������������
	*/
	SRAM_HandleTypeDef hsram = {0};
	FMC_NORSRAM_TimingTypeDef SRAM_Timing = {0};
		
	/*
		AD7606�����Ҫ��(3.3Vʱ��ͨ�ŵ�ƽVdriver)��RD���źŵ͵�ƽ���������21ns����ӦFMC��DataSetupTime
		CSƬѡ��RD���źŶ�����ʽ�ĸߵ�ƽ������̿��15ns��
		CSƬѡ��RD���źŲ�����ʽ�ĸߵ�ƽ������̿��22ns��
		���ｫ22ns��Ϊ��Сֵ������Щ����ӦFMC��AddressSetupTime
	
		4-x-6-x-x-x  : RD�߳���35.7ns���͵�ƽ����23.8ns. ��ȡ8·�������ݵ��ڴ������476ns��
	*/
	hsram.Instance  = FMC_NORSRAM_DEVICE;
	hsram.Extended  = FMC_NORSRAM_EXTENDED_DEVICE;
	
	/* FMCʹ�õ�HCLK����Ƶ168MHz��1��FMCʱ�����ھ���5.95ns */
	SRAM_Timing.AddressSetupTime       = 4;  /* 4*5.95ns=23.8ns����ַ����ʱ�䣬��Χ0 -15��FMCʱ�����ڸ��� */
	SRAM_Timing.AddressHoldTime        = 0;  /* ��ַ����ʱ�䣬����ΪģʽAʱ���ò����˲��� ��Χ1 -15��ʱ�����ڸ��� */
	SRAM_Timing.DataSetupTime          = 6;  /* 6*5.95ns=35.7ns�����ݱ���ʱ�䣬��Χ1 -255��ʱ�����ڸ��� */
	SRAM_Timing.BusTurnAroundDuration  = 0;  /* �������ò���������� */
	SRAM_Timing.CLKDivision            = 0;  /* �������ò���������� */
	SRAM_Timing.DataLatency            = 0;  /* �������ò���������� */
	SRAM_Timing.AccessMode             = FSMC_ACCESS_MODE_A; /* ����ΪģʽA */

	hsram.Init.NSBank             = FSMC_NORSRAM_BANK4;              /* ʹ�õ�BANK4����ʹ�õ�ƬѡFSMC_NE4 */
	hsram.Init.DataAddressMux     = FSMC_DATA_ADDRESS_MUX_DISABLE;   /* ��ֹ��ַ���ݸ��� */
	hsram.Init.MemoryType         = FSMC_MEMORY_TYPE_SRAM;           /* �洢������SRAM */
	hsram.Init.MemoryDataWidth    = FSMC_NORSRAM_MEM_BUS_WIDTH_16;	/* 16λ���߿�� */
	hsram.Init.BurstAccessMode    = FSMC_BURST_ACCESS_MODE_DISABLE;  /* �ر�ͻ��ģʽ */
	hsram.Init.WaitSignalPolarity = FSMC_WAIT_SIGNAL_POLARITY_LOW;   /* �������õȴ��źŵļ��ԣ��ر�ͻ��ģʽ���˲�����Ч */
	hsram.Init.WaitSignalActive   = FSMC_WAIT_TIMING_BEFORE_WS;      /* �ر�ͻ��ģʽ���˲�����Ч */
	hsram.Init.WriteOperation     = FSMC_WRITE_OPERATION_ENABLE;     /* ����ʹ�ܻ��߽�ֹд���� */
	hsram.Init.WaitSignal         = FSMC_WAIT_SIGNAL_DISABLE;        /* �ر�ͻ��ģʽ���˲�����Ч */
	hsram.Init.ExtendedMode       = FSMC_EXTENDED_MODE_DISABLE;      /* ��ֹ��չģʽ */
	hsram.Init.AsynchronousWait   = FSMC_ASYNCHRONOUS_WAIT_DISABLE;  /* �����첽�����ڼ䣬ʹ�ܻ��߽�ֹ�ȴ��źţ�����ѡ��ر� */
	hsram.Init.WriteBurst         = FSMC_WRITE_BURST_DISABLE;        /* ��ֹдͻ�� */
	hsram.Init.ContinuousClock    = FSMC_CONTINUOUS_CLOCK_SYNC_ONLY; /* ��ͬ��ģʽ����ʱ����� */
    hsram.Init.WriteFifo          = FSMC_WRITE_FIFO_ENABLE;          /* ʹ��дFIFO */

	/* ��ʼ��SRAM������ */
	if (HAL_SRAM_Init(&hsram, &SRAM_Timing, &SRAM_Timing) != HAL_OK)
	{
		/* ��ʼ������ */
		Error_Handler(__FILE__, __LINE__);
	}	
}

/*
*********************************************************************************************************
*	�� �� ��: AD7606_SetOS
*	����˵��: ����AD7606�����˲�����Ҳ�����ù��������ʡ�
*			  ͨ������ AD7606_OS0��OS1��OS2���ߵĵ�ƽ���״̬�������������ʡ�
*			  ����ADת��֮��AD7606�ڲ��Զ�ʵ��ʣ�������Ĳɼ���Ȼ����ƽ��ֵ�����
*
*			  ����������Խ�ߣ�ת��ʱ��Խ����
*			  0���޹�����ʱ��ADת��ʱ�� = 3.45us - 4.15us
*			  1��2��������ʱ = 7.87us - 9.1us
*			  2��4��������ʱ = 16.05us - 18.8us
*			  3��8��������ʱ = 33us - 39us
*			  4��16��������ʱ = 66us - 78us
*			  5��32��������ʱ = 133us - 158us
*			  6��64��������ʱ = 257us - 315us
*
*	��    ��: _ucOS : ����������, 0 - 6
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void AD7606_SetOS(uint8_t _ucOS)
{
	g_tAD7606.ucOS = _ucOS;
	switch (_ucOS)
	{
		case AD_OS_X2:
			OS2_0();
			OS1_0();
			OS0_1();
			break;

		case AD_OS_X4:
			OS2_0();
			OS1_1();
			OS0_0();
			break;

		case AD_OS_X8:
			OS2_0();
			OS1_1();
			OS0_1();
			break;

		case AD_OS_X16:
			OS2_1();
			OS1_0();
			OS0_0();
			break;

		case AD_OS_X32:
			OS2_1();
			OS1_0();
			OS0_1();
			break;

		case AD_OS_X64:
			OS2_1();
			OS1_1();
			OS0_0();
			break;

		case AD_OS_NO:
		default:
			g_tAD7606.ucOS = AD_OS_NO;
			OS2_0();
			OS1_0();
			OS0_0();
			break;
	}
}

/*
*********************************************************************************************************
*	�� �� ��: AD7606_SetInputRange
*	����˵��: ����AD7606ģ���ź��������̡�
*	��    ��: _ucRange : 0 ��ʾ����5V   1��ʾ����10V
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void AD7606_SetInputRange(uint8_t _ucRange)
{
	if (_ucRange == 0)
	{
		g_tAD7606.ucRange = 0;
		RANGE_0();	/* ����Ϊ����5V */
	}
	else
	{
		g_tAD7606.ucRange = 1;
		RANGE_1();	/* ����Ϊ����10V */
	}
}

/*
*********************************************************************************************************
*	�� �� ��: AD7606_Reset
*	����˵��: Ӳ����λAD7606����λ֮��ָ�����������״̬��
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void AD7606_Reset(void)
{
	RESET_0();	/* �˳���λ״̬ */

	RESET_1();	/* ���븴λ״̬ */
	RESET_1();	/* �������ӳ١� RESET��λ�ߵ�ƽ��������С50ns�� */
	RESET_1();
	RESET_1();

	RESET_0();	/* �˳���λ״̬ */
}

/*
*********************************************************************************************************
*	�� �� ��: AD7606_StartConvst
*	����˵��: ����1��ADCת��
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void AD7606_StartConvst(void)
{
	/* page 7��  CONVST �ߵ�ƽ�����Ⱥ͵͵�ƽ��������� 25ns */
	/* CONVSTƽʱΪ�� */
	CONVST_0();
	CONVST_0();
	CONVST_0();

	CONVST_1();
}

/*
*********************************************************************************************************
*	�� �� ��: AD7606_ReadNowAdc
*	����˵��: ��ȡ8·�������������洢��ȫ�ֱ��� g_tAD7606
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
/* �����壬�����û����ɼ��Ľ��ʵʱ��� */
__weak void AD7606_SEGGER_RTTOUT(void)
{
	
}

void AD7606_ReadNowAdc(void)
{
	g_tAD7606.sNowAdc[0] = AD7606_RESULT();	/* ����1·���� */
	g_tAD7606.sNowAdc[1] = AD7606_RESULT();	/* ����2·���� */
	g_tAD7606.sNowAdc[2] = AD7606_RESULT();	/* ����3·���� */
	g_tAD7606.sNowAdc[3] = AD7606_RESULT();	/* ����4·���� */
	g_tAD7606.sNowAdc[4] = AD7606_RESULT();	/* ����5·���� */
	g_tAD7606.sNowAdc[5] = AD7606_RESULT();	/* ����6·���� */
	g_tAD7606.sNowAdc[6] = AD7606_RESULT();	/* ����7·���� */
	g_tAD7606.sNowAdc[7] = AD7606_RESULT();	/* ����8·���� */

	AD7606_SEGGER_RTTOUT();
}

/*
*********************************************************************************************************
*		����ĺ������ڶ�ʱ�ɼ�ģʽ�� TIM8Ӳ����ʱ�ж��ж�ȡADC���������ȫ��FIFO
*
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*	�� �� ��: AD7606_HasNewData
*	����˵��: �ж�FIFO���Ƿ���������
*	��    ��:  ��
*	�� �� ֵ: 1 ��ʾ�У�0��ʾ��������
*********************************************************************************************************
*/
uint8_t AD7606_HasNewData(void)
{
	if (g_tAdcFifo.usCount > 0)
	{
		return 1;
	}
	return 0;
}

/*
*********************************************************************************************************
*	�� �� ��: AD7606_FifoFull
*	����˵��: �ж�FIFO�Ƿ���
*	��    ��: ��
*	�� �� ֵ: 1 ��ʾ����0��ʾδ��
*********************************************************************************************************
*/
uint8_t AD7606_FifoFull(void)
{
	return g_tAdcFifo.ucFull;
}

/*
*********************************************************************************************************
*	�� �� ��: AD7606_ReadFifo
*	����˵��: ��FIFO�ж�ȡһ��ADCֵ
*	��    ��:  _usReadAdc : ���ADC����ı���ָ��
*	�� �� ֵ: 1 ��ʾOK��0��ʾ��������
*********************************************************************************************************
*/
uint8_t AD7606_ReadFifo(uint16_t *_usReadAdc)
{
	if (AD7606_HasNewData())
	{
		*_usReadAdc = g_tAdcFifo.sBuf[g_tAdcFifo.usRead];
		if (++g_tAdcFifo.usRead >= ADC_FIFO_SIZE)
		{
			g_tAdcFifo.usRead = 0;
		}

		DISABLE_INT();
		if (g_tAdcFifo.usCount > 0)
		{
			g_tAdcFifo.usCount--;
		}
		ENABLE_INT();
		return 1;
	}
	return 0;
}

/*
*********************************************************************************************************
*	�� �� ��: AD7606_StartRecord
*	����˵��: ��ʼ�ɼ�
*	��    ��: _ulFreq : ����Ƶ�ʣ���λHz , ����AD7606��˵����Χ1 - 200KHz
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void AD7606_StartRecord(uint32_t _ulFreq)
{
	AD7606_StopRecord();

	AD7606_Reset();					/* ��λӲ�� */
	AD7606_StartConvst();			/* ���������������1������ȫ0������ */

	g_tAdcFifo.usRead = 0;			/* �����ڿ�����ʱ��֮ǰ��0 */
	g_tAdcFifo.usWrite = 0;
	g_tAdcFifo.usCount = 0;
	g_tAdcFifo.ucFull = 0;

	AD7606_EnterAutoMode(_ulFreq);
}

/*
*********************************************************************************************************
*	�� �� ��: AD7606_EnterAutoMode
*	����˵��: ����Ӳ���������Զ��ɼ�ģʽ������洢��FIFO��������
*	��    ��:  _ulFreq : ����Ƶ�ʣ���λHz��	1k��2k��5k��10k��20K��50k��100k��200k
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void AD7606_EnterAutoMode(uint32_t _ulFreq)
{
	/* ����PH12ΪTIM5_CH3���ܣ����ռ�ձ�50%�ķ��� */
	bsp_SetTIMOutPWM(CONVST_GPIO, CONVST_PIN, CONVST_TIMX,  CONVST_TIMCH, _ulFreq, 5000);
	
	/* ����PI6, BUSY ��Ϊ�ж�����ڣ��½��ش��� */
	{
		GPIO_InitTypeDef   GPIO_InitStructure;
	
		CONVST_RCC_GPIO_CLK_ENABLE();	/* ��GPIOʱ�� */
		__HAL_RCC_SYSCFG_CLK_ENABLE();

		GPIO_InitStructure.Mode = GPIO_MODE_IT_FALLING;	/* �ж��½��ش��� */
		GPIO_InitStructure.Pull = GPIO_NOPULL;
		GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_LOW;		
		GPIO_InitStructure.Pin = BUSY_PIN;
		HAL_GPIO_Init(BUSY_GPIO, &GPIO_InitStructure);	

		HAL_NVIC_SetPriority(BUSY_IRQn, 2, 0);
		HAL_NVIC_EnableIRQ(BUSY_IRQn);	
	}		
}

/*
*********************************************************************************************************
*	�� �� ��: AD7606_StopRecord
*	����˵��: ֹͣ�ɼ���ʱ��
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void AD7606_StopRecord(void)
{
	/* ����PH12 ����͵�ƽ���ر�TIM */
	bsp_SetTIMOutPWM(CONVST_GPIO, CONVST_PIN, CONVST_TIMX,  CONVST_TIMCH, 1000, 10000);
	HAL_GPIO_DeInit(CONVST_GPIO, CONVST_PIN);
	CONVST_TIM8_CLK_DISABLE();
	
	/* CONVST ����ADCת����GPIO = PH12*/
	{
		GPIO_InitTypeDef   GPIO_InitStructure;
		CONVST_RCC_GPIO_CLK_ENABLE();

		/* ����PH12 */
		GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;		/* ����������� */
		GPIO_InitStructure.Pull = GPIO_NOPULL;				/* ���������費ʹ�� */
		GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;    /* GPIO�ٶȵȼ� */	

		GPIO_InitStructure.Pin = CONVST_PIN;	
		HAL_GPIO_Init(CONVST_GPIO, &GPIO_InitStructure);	
	}
	
	CONVST_1();			/* ����ת����GPIOƽʱ����Ϊ�� */	
	
	/* ����BUSY���ţ�Ĭ������ͨIO״̬ */
	{
		GPIO_InitTypeDef   GPIO_InitStructure;
	
		__HAL_RCC_SYSCFG_CLK_ENABLE();
		BUSY_RCC_GPIO_CLK_ENABLE();		/* ��GPIOʱ�� */

		HAL_GPIO_DeInit(BUSY_GPIO, BUSY_PIN);
		
		/* BUSY�źţ�ʹ�õ�PI6������ת����ϼ�� */
		GPIO_InitStructure.Mode = GPIO_MODE_INPUT;   /* ����������� */
		GPIO_InitStructure.Pull = GPIO_NOPULL;       /* ���������� */
		GPIO_InitStructure.Pin = BUSY_PIN;           
		HAL_GPIO_Init(BUSY_GPIO, &GPIO_InitStructure);	
		
		HAL_NVIC_DisableIRQ(BUSY_IRQn);
	}
}

/*
*********************************************************************************************************
*	�� �� ��: AD7606_ISR
*	����˵��: ��ʱ�ɼ��жϷ������
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void AD7606_ISR(void)
{
	uint8_t i;

	AD7606_ReadNowAdc();

	for (i = 0; i < 8; i++)
	{
		g_tAdcFifo.sBuf[g_tAdcFifo.usWrite] = g_tAD7606.sNowAdc[i];
		if (++g_tAdcFifo.usWrite >= ADC_FIFO_SIZE)
		{
			g_tAdcFifo.usWrite = 0;
		}
		if (g_tAdcFifo.usCount < ADC_FIFO_SIZE)
		{
			g_tAdcFifo.usCount++;
		}
		else
		{
			g_tAdcFifo.ucFull = 1;		/* FIFO ������������������������ */
		}
	}
}

/*
*********************************************************************************************************
*	�� �� ��: EXTI9_5_IRQHandler
*	����˵��: �ⲿ�жϷ������
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
#ifdef EXTI9_5_ISR_MOVE_OUT		/* bsp.h �ж�����У���ʾ�������Ƶ� stam32xx_it.c�� �����ظ����� */
void EXTI9_5_IRQHandler(void)
{
	HAL_GPIO_EXTI_IRQHandler(BUSY_PIN);
}

/*
*********************************************************************************************************
*	�� �� ��: EXTI9_5_IRQHandler
*	����˵��: �ⲿ�жϷ���������, AD7606_BUSY �½����жϴ���
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if (GPIO_Pin == BUSY_PIN)
	{
		AD7606_ISR();
	}
}
#endif

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
