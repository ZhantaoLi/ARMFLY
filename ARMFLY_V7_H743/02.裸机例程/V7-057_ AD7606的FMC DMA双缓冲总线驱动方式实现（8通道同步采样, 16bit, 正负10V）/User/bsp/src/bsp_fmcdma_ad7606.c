/*
*********************************************************************************************************
*
*	ģ������ : AD7606���ݲɼ�ģ�顾ԭ����
*	�ļ����� : bsp_fmcdma_ad7606.c
*	��    �� : V1.0
*	˵    �� : AD7606 FMC DMA�ɼ���ʽ��֧��˫����
*
*	�޸ļ�¼ :
*		�汾��  ����        ����     ˵��
*		V1.0    2020-05-24 armfly  ��ʽ����
*
*	Copyright (C), 2020-2030, ���������� www.armfly.com
*
*********************************************************************************************************
*/
#include "bsp.h"



/*
	STM32-V7������ + AD7606ģ�飬 ���Ʋɼ���IO:
	
	PC6/TIM3_CH1/TIM8_CH1     ----> AD7606_CONVST  (������ͷ����),  ���PWM��������ΪADC�����ź�
	PE5/DCMI_D6/AD7606_BUSY   <---- AD7606_BUSY    , CPU��BUSY�жϷ�������ж�ȡ�ɼ����
*/
/* CONVST ����ADCת����GPIO = PC6 */
#define CONVST_RCC_GPIO_CLK_ENABLE	__HAL_RCC_GPIOC_CLK_ENABLE
#define CONVST_TIM8_CLK_ENABLE      __HAL_RCC_TIM8_CLK_ENABLE
#define CONVST_RCC_GPIO_CLK_DISBALE	__HAL_RCC_GPIOC_CLK_DISABLE
#define CONVST_TIM8_CLK_DISABLE     __HAL_RCC_TIM8_CLK_DISABLE
#define CONVST_GPIO		GPIOC
#define CONVST_PIN		GPIO_PIN_6
#define CONVST_AF		GPIO_AF3_TIM8
#define CONVST_TIMX		TIM8
#define CONVST_TIMCH	TIM_CHANNEL_1

/* FMC DMA */
#define TIMx_UP_DMA_STREAM_CLK_ENABLE  	__HAL_RCC_DMA2_CLK_ENABLE
#define TIMx_UP_DMA_STREAM_CLK_DISABLE  __HAL_RCC_DMA2_CLK_DISABLE
#define TIMx_UP_DMA_STREAM             DMA2_Stream1
#define TIMx_UP_DMA_REQUEST            DMA_REQUEST_TIM8_UP
#define TIMx_UP_DMA_IRQn               DMA2_Stream1_IRQn
#define TIMx_UP_DMA_IRQHandler         DMA2_Stream1_IRQHandler

/* ���ù�������IO, ����չ��74HC574�� */
#define OS0_1()		HC574_SetPin(AD7606_OS0, 1)
#define OS0_0()		HC574_SetPin(AD7606_OS0, 0)
#define OS1_1()		HC574_SetPin(AD7606_OS1, 1)
#define OS1_0()		HC574_SetPin(AD7606_OS1, 0)
#define OS2_1()		HC574_SetPin(AD7606_OS2, 1)
#define OS2_0()		HC574_SetPin(AD7606_OS2, 0)

/* ����ADת����GPIO : PC6 */
#define CONVST_1()		CONVST_GPIO->BSRR = CONVST_PIN
#define CONVST_0()		CONVST_GPIO->BSRR = ((uint32_t)CONVST_PIN << 16U)

/* �����������̵�GPIO, ����չ��74HC574�� */
#define RANGE_1()	HC574_SetPin(AD7606_RANGE, 1)
#define RANGE_0()	HC574_SetPin(AD7606_RANGE, 0)

/* AD7606��λ����, ����չ��74HC574�� */
#define RESET_1()	HC574_SetPin(AD7606_RESET, 1)
#define RESET_0()	HC574_SetPin(AD7606_RESET, 0)

/* AD7606 FSMC���ߵ�ַ��ֻ�ܶ�������д */
#define AD7606_BASE    	0x60003000

static DMA_HandleTypeDef TIMDMA = {0};
static TIM_HandleTypeDef TimHandle = {0};


/* 8·ͬ���ɼ���ÿ�βɼ�16�ֽ����ݣ���ֹDMAͻ����ʽ1KB�߽����⣬��ÿ�βɼ���Ҫ�п�߽����� */
#define AD7606_BUFSIZE        16
__align(16) int16_t g_sAd7606Buf[AD7606_BUFSIZE];   

AD7606_VAR_T g_tAD7606;		/* ����1��ȫ�ֱ���������һЩ���� */

static void AD7606_CtrlLinesConfig(void);
static void AD7606_FSMCConfig(void);
static void AD7606_SetTIMOutPWM(TIM_TypeDef* TIMx, uint32_t _ulFreq);

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
	������STM32-H7��������߷�����4Ƭ74HC574����FMC 32λ�����ϡ�1����ַ�˿ڿ�����չ��32��IO
	PD0/FMC_D2
	PD1/FMC_D3
	PD4/FMC_NOE		---- �������źţ�OE = Output Enable �� N ��ʾ����Ч
	PD5/FMC_NWE		-XX- д�����źţ�AD7606 ֻ�ж�����д�ź�
	PD8/FMC_D13
	PD9/FMC_D14
	PD10/FMC_D15
	PD14/FMC_D0
	PD15/FMC_D1

	PE7/FMC_D4
	PE8/FMC_D5
	PE9/FMC_D6
	PE10/FMC_D7
	PE11/FMC_D8
	PE12/FMC_D9
	PE13/FMC_D10
	PE14/FMC_D11
	PE15/FMC_D12
	
	PG0/FMC_A10		--- ����ƬѡFMC_NE2һ������
	PG1/FMC_A11		--- ����ƬѡFMC_NE2һ������
	PD7/FMC_NE1		--- ��Ƭѡ��OLED, 74HC574, DM9000, AD7606��	

	 +-------------------+------------------+
	 +   32-bits Mode: D31-D16              +
	 +-------------------+------------------+
	 | PH8 <-> FMC_D16   | PI0 <-> FMC_D24  |
	 | PH9 <-> FMC_D17   | PI1 <-> FMC_D25  |
	 | PH10 <-> FMC_D18  | PI2 <-> FMC_D26  |
	 | PH11 <-> FMC_D19  | PI3 <-> FMC_D27  |
	 | PH12 <-> FMC_D20  | PI6 <-> FMC_D28  |
	 | PH13 <-> FMC_D21  | PI7 <-> FMC_D29  |
	 | PH14 <-> FMC_D22  | PI9 <-> FMC_D30  |
	 | PH15 <-> FMC_D23  | PI10 <-> FMC_D31 |
	 +------------------+-------------------+
*/

/* 
	����AD7606����������IO��������չ��74HC574��
	X13 - AD7606_OS0
	X14 - AD7606_OS1
	X15 - AD7606_OS2
	X24 - AD7606_RESET
	X25 - AD7606_RAGE	
	
	PE5 - AD7606_BUSY
*/
static void AD7606_CtrlLinesConfig(void)
{
	/* bsp_fm_io ������fmc��bsp_InitExtIO();
	   �˴����Բ����ظ����� 
	*/

	GPIO_InitTypeDef gpio_init_structure;

	/* ʹ�� GPIOʱ�� */
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOE_CLK_ENABLE();
	__HAL_RCC_GPIOF_CLK_ENABLE();
	__HAL_RCC_GPIOG_CLK_ENABLE();
	__HAL_RCC_GPIOH_CLK_ENABLE();
	__HAL_RCC_GPIOI_CLK_ENABLE();

	/* ʹ��FMCʱ�� */
	__HAL_RCC_FMC_CLK_ENABLE();

	/* ���� GPIOD ��ص�IOΪ����������� */
	gpio_init_structure.Mode = GPIO_MODE_AF_PP;
	gpio_init_structure.Pull = GPIO_PULLUP;
	gpio_init_structure.Speed = GPIO_SPEED_FREQ_HIGH;
	gpio_init_structure.Alternate = GPIO_AF12_FMC;
	
	/* ����GPIOD */
	gpio_init_structure.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_7 |
	                            GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_14 |
	                            GPIO_PIN_15;
	HAL_GPIO_Init(GPIOD, &gpio_init_structure);

	/* ����GPIOE */
	gpio_init_structure.Pin = GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 |
	                            GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 |
	                            GPIO_PIN_15;
	HAL_GPIO_Init(GPIOE, &gpio_init_structure);

	/* ����GPIOG */
	gpio_init_structure.Pin = GPIO_PIN_0 | GPIO_PIN_1;
	HAL_GPIO_Init(GPIOG, &gpio_init_structure);
	
	/* ����GPIOH */
	gpio_init_structure.Pin = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12
						| GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
	HAL_GPIO_Init(GPIOH, &gpio_init_structure);

	/* ����GPIOI */
	gpio_init_structure.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_6
						| GPIO_PIN_7 | GPIO_PIN_9 | GPIO_PIN_10;
	HAL_GPIO_Init(GPIOI, &gpio_init_structure);
	
	/* CONVST ����ADCת����GPIO = PC6 */
	{
		GPIO_InitTypeDef   GPIO_InitStructure;
		CONVST_RCC_GPIO_CLK_ENABLE();

		/* ����PC6 */
		GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;		/* ����������� */
		GPIO_InitStructure.Pull = GPIO_NOPULL;				/* ���������費ʹ�� */
		GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_MEDIUM;  /* GPIO�ٶȵȼ� */	

		GPIO_InitStructure.Pin = CONVST_PIN;	
		HAL_GPIO_Init(CONVST_GPIO, &GPIO_InitStructure);	
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
	   DM9000����չIO��OLED��AD7606����һ��FMC���ã����������������FMC�ٶ�������Ϊ׼��
	   �Ӷ���֤�������趼��������������
	*/
	SRAM_HandleTypeDef hsram = {0};
	FMC_NORSRAM_TimingTypeDef SRAM_Timing = {0};
		
	/*
		AD7606�����Ҫ��(3.3Vʱ��ͨ�ŵ�ƽVdriver)��RD���źŵ͵�ƽ���������21ns����ӦFMC��DataSetupTime
		CSƬѡ��RD���źŶ�����ʽ�ĸߵ�ƽ������̿��15ns��
		CSƬѡ��RD���źŲ�����ʽ�ĸߵ�ƽ������̿��22ns��
		���ｫ22ns��Ϊ��Сֵ������Щ����ӦFMC��AddressSetupTime
	
		5-x-5-x-x-x  : RD�߳���25ns�� �͵�ƽ����25ns. ��ȡ8·�������ݵ��ڴ������400ns��
	*/
	hsram.Instance  = FMC_NORSRAM_DEVICE;
	hsram.Extended  = FMC_NORSRAM_EXTENDED_DEVICE;
	
	/* FMCʹ�õ�HCLK3����Ƶ200MHz��1��FMCʱ�����ھ���5ns */
	SRAM_Timing.AddressSetupTime       = 5;  /* 5*5ns=25ns����ַ����ʱ�䣬��Χ0 -15��FMCʱ�����ڸ��� */
	SRAM_Timing.AddressHoldTime        = 2;  /* ��ַ����ʱ�䣬����ΪģʽAʱ���ò����˲��� ��Χ1 -15��ʱ�����ڸ��� */
	SRAM_Timing.DataSetupTime          = 5;  /* 5*5ns=25ns�����ݽ���ʱ�䣬��Χ1 -255��ʱ�����ڸ��� */
	SRAM_Timing.BusTurnAroundDuration  = 1;  /* �������ò���������� */
	SRAM_Timing.CLKDivision            = 2;  /* �������ò���������� */
	SRAM_Timing.DataLatency            = 2;  /* �������ò���������� */
	SRAM_Timing.AccessMode             = FMC_ACCESS_MODE_A; /* ����ΪģʽA */

	hsram.Init.NSBank             = FMC_NORSRAM_BANK1;              /* ʹ�õ�BANK1����ʹ�õ�ƬѡFMC_NE1 */
	hsram.Init.DataAddressMux     = FMC_DATA_ADDRESS_MUX_DISABLE;   /* ��ֹ��ַ���ݸ��� */
	hsram.Init.MemoryType         = FMC_MEMORY_TYPE_SRAM;           /* �洢������SRAM */
	hsram.Init.MemoryDataWidth    = FMC_NORSRAM_MEM_BUS_WIDTH_32;	/* 32λ���߿�� */
	hsram.Init.BurstAccessMode    = FMC_BURST_ACCESS_MODE_DISABLE;  /* �ر�ͻ��ģʽ */
	hsram.Init.WaitSignalPolarity = FMC_WAIT_SIGNAL_POLARITY_LOW;   /* �������õȴ��źŵļ��ԣ��ر�ͻ��ģʽ���˲�����Ч */
	hsram.Init.WaitSignalActive   = FMC_WAIT_TIMING_BEFORE_WS;      /* �ر�ͻ��ģʽ���˲�����Ч */
	hsram.Init.WriteOperation     = FMC_WRITE_OPERATION_ENABLE;     /* ����ʹ�ܻ��߽�ֹд���� */
	hsram.Init.WaitSignal         = FMC_WAIT_SIGNAL_DISABLE;        /* �ر�ͻ��ģʽ���˲�����Ч */
	hsram.Init.ExtendedMode       = FMC_EXTENDED_MODE_DISABLE;      /* ��ֹ��չģʽ */
	hsram.Init.AsynchronousWait   = FMC_ASYNCHRONOUS_WAIT_DISABLE;  /* �����첽�����ڼ䣬ʹ�ܻ��߽�ֹ�ȴ��źţ�����ѡ��ر� */
	hsram.Init.WriteBurst         = FMC_WRITE_BURST_DISABLE;        /* ��ֹдͻ�� */
	hsram.Init.ContinuousClock    = FMC_CONTINUOUS_CLOCK_SYNC_ONLY; /* ��ͬ��ģʽ����ʱ����� */
    hsram.Init.WriteFifo          = FMC_WRITE_FIFO_ENABLE;          /* ʹ��дFIFO */

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
*	�� �� ��: AD7606_StartRecord
*	����˵��: ��ʼ�ɼ�
*	��    ��: _ulFreq : ����Ƶ�ʣ���λHz��	1k��2k��5k��10k��20K��50k��100k��200k
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void AD7606_StartRecord(uint32_t _ulFreq)
{
	AD7606_StopRecord();

	AD7606_Reset();					/* ��λӲ�� */
	AD7606_StartConvst();			/* ���������������1������ȫ0������ */

	/* ����PC6ΪTIM8_CH1���� */
	AD7606_SetTIMOutPWM(CONVST_TIMX, _ulFreq);
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
	/* ����PC6 ����͵�ƽ���ر�TIM */
	HAL_GPIO_DeInit(CONVST_GPIO, CONVST_PIN);
	CONVST_TIM8_CLK_DISABLE();
	TIMx_UP_DMA_STREAM_CLK_DISABLE();
	
	/* CONVST ����ADCת����GPIO = PC6 */
	{
		GPIO_InitTypeDef   GPIO_InitStructure;
		CONVST_RCC_GPIO_CLK_ENABLE();

		/* ����PC6 */
		GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;		/* ����������� */
		GPIO_InitStructure.Pull = GPIO_NOPULL;				/* ���������費ʹ�� */
		GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_MEDIUM;  /* GPIO�ٶȵȼ� */	

		GPIO_InitStructure.Pin = CONVST_PIN;	
		HAL_GPIO_Init(CONVST_GPIO, &GPIO_InitStructure);	
	}
	
	CONVST_1();			/* ����ת����GPIOƽʱ����Ϊ�� */	
}

/*
*********************************************************************************************************
*	�� �� ��: AD7606_SetTIMOutPWM
*	����˵��: �������ö�ʱUP���´���DMA���䣬�����ö�ʱ��PWM��ΪAD7606�Ĵ���ʱ��
*	��    ��: TIMx : TIM1 - TIM17
*			  _ulFreq : PWM�ź�Ƶ�ʣ���λHz����Χ1Hz - 200KHz
*	�� �� ֵ: ��
*********************************************************************************************************
*/
/* DMA������ɻص������������� */
__weak void AD7606_DmaCplCb(DMA_HandleTypeDef *hdma)
{
	
}

/* DMA�봫����ɻص������������� */
__weak void AD7606_DmaHalfCplCb(DMA_HandleTypeDef *hdma)
{
	
}

/* DMA�жϷ������ */
void TIMx_UP_DMA_IRQHandler(void)
{
	HAL_DMA_IRQHandler(&TIMDMA);
}

static void AD7606_SetTIMOutPWM(TIM_TypeDef* TIMx, uint32_t _ulFreq)
{
	TIM_OC_InitTypeDef sConfig = {0};	
	GPIO_InitTypeDef   GPIO_InitStruct;
	uint16_t usPeriod;
	uint16_t usPrescaler;
	uint32_t uiTIMxCLK;
	uint32_t pulse;

	
	/* ����ʱ�� */
	CONVST_RCC_GPIO_CLK_ENABLE();
	CONVST_TIM8_CLK_ENABLE();
	TIMx_UP_DMA_STREAM_CLK_ENABLE();
	
	/* �������� */
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.Alternate = CONVST_AF;
	GPIO_InitStruct.Pin = CONVST_PIN;
	HAL_GPIO_Init(CONVST_GPIO, &GPIO_InitStruct);
	
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

	----------------------------------------------------------------------- */
	if ((TIMx == TIM1) || (TIMx == TIM8) || (TIMx == TIM15) || (TIMx == TIM16) || (TIMx == TIM17))
	{
		/* APB2 ��ʱ��ʱ�� = 200M */
		uiTIMxCLK = SystemCoreClock / 2;
	}
	else	
	{
		/* APB1 ��ʱ�� = 200M */
		uiTIMxCLK = SystemCoreClock / 2;
	}

	if (_ulFreq < 100)
	{
		usPrescaler = 10000 - 1;							/* ��Ƶ�� = 10000 */
		usPeriod =  (uiTIMxCLK / 10000) / _ulFreq  - 1;		/* �Զ���װ��ֵ�� usPeriod��Сֵ200, ��λ50us */
		pulse = usPeriod;                               	/* ���õ͵�ƽʱ��50us��ע��usPeriod�Ѿ������˼�1���� */
	} 
	else if (_ulFreq < 3000)
	{
		usPrescaler = 100 - 1;							/* ��Ƶ�� = 100 */
		usPeriod =  (uiTIMxCLK / 100) / _ulFreq  - 1;	/* �Զ���װ��ֵ�� usPeriod��Сֵ666����λ500ns */
		pulse = usPeriod-1;                           	/* ���õ͵�ƽʱ��1us��ע��usPeriod�Ѿ������˼�1���� */
	}
	else	/* ����4K��Ƶ�ʣ������Ƶ */
	{
		usPrescaler = 0;								/* ��Ƶ�� = 1 */
		usPeriod = uiTIMxCLK / _ulFreq - 1;				/* �Զ���װ��ֵ�� usPeriod��Сֵ1000����λ5ns */
		pulse = usPeriod - 199;              			/* ���õ͵�ƽʱ��1us��ע��usPeriod�Ѿ������˼�1���� */
	}
    
	/*  PWMƵ�� = TIMxCLK / usPrescaler + 1��/usPeriod + 1��*/
	TimHandle.Instance = TIMx;
	TimHandle.Init.Prescaler         = usPrescaler;         /* �������ö�ʱ����Ƶ */
	TimHandle.Init.Period            = usPeriod;            /* �������ö�ʱ������ */
	TimHandle.Init.ClockDivision     = 0;                   /* ����ָʾ��ʱ��ʱ�� (CK_INT) Ƶ���������������Լ������˲�����ETR�� TIx��
	                                                           ��ʹ�õ�����������ʱ�� (tDTS) ֮��ķ�Ƶ��*/
	TimHandle.Init.CounterMode       = TIM_COUNTERMODE_UP;  /* �������ü���ģʽ�����ϼ���ģʽ */
	TimHandle.Init.RepetitionCounter = 0;                   /* ���������ظ����������� TIM1 �� TIM8 �У�������ʱ��û�� */
	TimHandle.Init.AutoReloadPreload = 0;                   /* �������ö�ʱ���� ARR �Զ���װ�Ĵ����Ǹ����¼�����ʱд����Ч */
	
	if (HAL_TIM_PWM_DeInit(&TimHandle) != HAL_OK)
	{
		Error_Handler(__FILE__, __LINE__);		
	}
	
	if (HAL_TIM_PWM_Init(&TimHandle) != HAL_OK)
	{
		Error_Handler(__FILE__, __LINE__);
	}

	/* ���ö�ʱ��PWM���ͨ�� */
	sConfig.OCMode       = TIM_OCMODE_PWM1;         /* ��������Ƚ�ģʽ */
	sConfig.OCPolarity   = TIM_OCPOLARITY_HIGH;     /* ��������ߵ�ƽ��Ч */
	sConfig.OCFastMode   = TIM_OCFAST_DISABLE;      /* �رտ������ģʽ */
	sConfig.OCNPolarity  = TIM_OCNPOLARITY_HIGH;    /* ���û�������ߵ�ƽ��Ч */
	sConfig.OCIdleState  = TIM_OCIDLESTATE_SET;     /* ����״̬ʱ����������Ƚ�����Ϊ�ߵ�ƽ */
	sConfig.OCNIdleState = TIM_OCNIDLESTATE_RESET;  /* ����״̬ʱ�����û�������Ƚ�����Ϊ�͵�ƽ */

	/* ռ�ձ� */
	sConfig.Pulse = pulse;
	if (HAL_TIM_PWM_ConfigChannel(&TimHandle, &sConfig, CONVST_TIMCH) != HAL_OK)
	{
		Error_Handler(__FILE__, __LINE__);
	}
	
	/* ʹ�ܶ�ʱ���ж�  */
	__HAL_TIM_ENABLE_DMA(&TimHandle, TIM_DMA_UPDATE);
	
	/* ����PWM��� */
	if (HAL_TIM_PWM_Start(&TimHandle, CONVST_TIMCH) != HAL_OK)
	{
		Error_Handler(__FILE__, __LINE__);
	}
	
	/* ��ʱ��UP���´���DMA���� */		
	TIMDMA.Instance                 = TIMx_UP_DMA_STREAM;      /* ����ʹ�õ�DMA������ */
	TIMDMA.Init.FIFOMode            = DMA_FIFOMODE_ENABLE;     /* ʹ��FIFO*/
	TIMDMA.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL; /* �������÷�ֵ */
	TIMDMA.Init.MemBurst            = DMA_MBURST_INC8;	       /* ���ڴ洢��ͻ�� */
	TIMDMA.Init.PeriphBurst         = DMA_PBURST_INC8;	       /* ��������ͻ�� */
	TIMDMA.Init.Request             = TIMx_UP_DMA_REQUEST;     /* �������� */  
	TIMDMA.Init.Direction           = DMA_PERIPH_TO_MEMORY;    /* ���䷽���Ǵ����赽�洢�� */  
	TIMDMA.Init.PeriphInc           = DMA_PINC_DISABLE;        /* �����ַ������ֹ */ 
	TIMDMA.Init.MemInc              = DMA_MINC_ENABLE;         /* �洢����ַ����ʹ�� */  
	TIMDMA.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD; /* �������ݴ���λ��ѡ����֣���16bit */ 
	TIMDMA.Init.MemDataAlignment    = DMA_MDATAALIGN_HALFWORD; /* �洢�����ݴ���λ��ѡ����֣���16bit */    
	TIMDMA.Init.Mode                = DMA_CIRCULAR; 		   /* ѭ��ģʽ */
	TIMDMA.Init.Priority            = DMA_PRIORITY_LOW;        /* ���ȼ��� */
	
	 /* ��λDMA */
	if(HAL_DMA_DeInit(&TIMDMA) != HAL_OK)
	{
		Error_Handler(__FILE__, __LINE__);     
	}
	
	 /* ��ʼ��DMA */
	if(HAL_DMA_Init(&TIMDMA) != HAL_OK)
	{
		Error_Handler(__FILE__, __LINE__);     
	}
	
	/* ����DMA�����TIM */
	//__HAL_LINKDMA(&TimHandle, hdma[TIM_DMA_ID_UPDATE], TIMDMA);	
	
	/* ����DMA�ж� */
	HAL_NVIC_SetPriority(TIMx_UP_DMA_IRQn, 1, 0);
	HAL_NVIC_EnableIRQ(TIMx_UP_DMA_IRQn);
	
	/* ע��봫������жϺʹ�������ж� */
	HAL_DMA_RegisterCallback(&TIMDMA, HAL_DMA_XFER_CPLT_CB_ID, AD7606_DmaCplCb);
	HAL_DMA_RegisterCallback(&TIMDMA, HAL_DMA_XFER_HALFCPLT_CB_ID, AD7606_DmaHalfCplCb);
	
	/* ����DMA���� */
	HAL_DMA_Start_IT(&TIMDMA, (uint32_t)AD7606_BASE, (uint32_t)g_sAd7606Buf, AD7606_BUFSIZE);
}

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
