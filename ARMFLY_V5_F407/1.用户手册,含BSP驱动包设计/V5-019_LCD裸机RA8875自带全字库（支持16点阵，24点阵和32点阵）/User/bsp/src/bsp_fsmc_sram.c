/*
*********************************************************************************************************
*
*	ģ������ : �ⲿSRAM����ģ��
*	�ļ����� : bsp_fsmc_sram.c
*	��    �� : V2.4
*	˵    �� : ������STM32-F4����������SRAMΪ IS61WV102416BLL-10TL  ����2M�ֽڣ�16Bit��10ns�ٶ�
*
*	�޸ļ�¼ :
*		�汾��  ����        ����     ˵��
*		V1.0    2013-02-01 armfly  ��ʽ����
*
*	Copyright (C), 2013-2014, ���������� www.armfly.com
*
*********************************************************************************************************
*/

#include "bsp.h"

/*
*********************************************************************************************************
*	�� �� ��: bsp_InitExtSRAM
*	����˵��: ���������ⲿSRAM��GPIO��FSMC
*	��    ��:  ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void bsp_InitExtSRAM(void)
{
	/* 
	    SRAM �� GPIO ��
		PD0/FSMC_D2
		PD1/FSMC_D3
		PD4/FSMC_NOE
		PD5/FSMC_NWE
		PD8/FSMC_D13
		PD9/FSMC_D14
		PD10/FSMC_D15
		PD11/FSMC_A16
		PD12/FSMC_A17
		PD13/FSMC_A18
		PD14/FSMC_D0
		PD15/FSMC_D1

		PE0/FSMC_NBL0
		PE1/FSMC_NBL1
		PE3/FSMC_A19
		PE4/FSMC_A20	-- ����Ƭѡ������
		PE5/FSMC_A21	-- ����Ƭѡ������
		PE7/FSMC_D4
		PE8/FSMC_D5
		PE9/FSMC_D6
		PE10/FSMC_D7
		PE11/FSMC_D8
		PE12/FSMC_D9
		PE13/FSMC_D10
		PE14/FSMC_D11
		PE15/FSMC_D12

		PF0/FSMC_A0
		PF1/FSMC_A1
		PF2/FSMC_A2
		PF3/FSMC_A3
		PF4/FSMC_A4
		PF5/FSMC_A5
		PF12/FSMC_A6
		PF13/FSMC_A7
		PF14/FSMC_A8
		PF15/FSMC_A9

		PG0/FSMC_A10
		PG1/FSMC_A11
		PG2/FSMC_A12
		PG3/FSMC_A13
		PG4/FSMC_A14
		PG5/FSMC_A15
		PG10/FSMC_NE3	--- Ƭѡ���ź�
	*/
	GPIO_InitTypeDef gpio_init_structure;
	SRAM_HandleTypeDef hsram = {0};
	FMC_NORSRAM_TimingTypeDef SRAM_TimingRead = {0};
	FMC_NORSRAM_TimingTypeDef SRAM_TimingWrite = {0};

	/* ʹ�� GPIOʱ�� */
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOE_CLK_ENABLE();
	__HAL_RCC_GPIOF_CLK_ENABLE();
	__HAL_RCC_GPIOG_CLK_ENABLE();

	/* ʹ��FMCʱ�� */
	__HAL_RCC_FSMC_CLK_ENABLE();

	/* ���� GPIOD ��ص�IOΪ����������� */
	gpio_init_structure.Mode = GPIO_MODE_AF_PP;
	gpio_init_structure.Pull = GPIO_PULLUP;
	gpio_init_structure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	gpio_init_structure.Alternate = GPIO_AF12_FSMC;
	
	/* ����GPIOD */
	gpio_init_structure.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_4 | GPIO_PIN_5  |
	                          GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 |
	                          GPIO_PIN_15;
	HAL_GPIO_Init(GPIOD, &gpio_init_structure);

	/* ����GPIOE */
	gpio_init_structure.Pin =  GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_3 |GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 |
	                          GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
	HAL_GPIO_Init(GPIOE, &gpio_init_structure);
	
	
	/* ����GPIOF */
	gpio_init_structure.Pin =  GPIO_PIN_0 | GPIO_PIN_1 |GPIO_PIN_2 | GPIO_PIN_3 |GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15; 
	HAL_GPIO_Init(GPIOF, &gpio_init_structure);

	/* ����GPIOG */
	gpio_init_structure.Pin = GPIO_PIN_0| GPIO_PIN_1 |GPIO_PIN_2 | GPIO_PIN_3 |GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_10 ;
	HAL_GPIO_Init(GPIOG, &gpio_init_structure);
	

//	/*-- FSMC Configuration ------------------------------------------------------*/
//	p.FSMC_AddressSetupTime = 3;		/* ����Ϊ2�����; 3���� */
//	p.FSMC_AddressHoldTime = 0;
//	p.FSMC_DataSetupTime = 2;			/* ����Ϊ1����2���� */
//	p.FSMC_BusTurnAroundDuration = 1;
//	p.FSMC_CLKDivision = 0;
//	p.FSMC_DataLatency = 0;
//	p.FSMC_AccessMode = FSMC_AccessMode_A;

//	FSMC_NORSRAMInitStructure.FSMC_Bank = FSMC_Bank1_NORSRAM3;
//	FSMC_NORSRAMInitStructure.FSMC_DataAddressMux = FSMC_DataAddressMux_Disable;
//	FSMC_NORSRAMInitStructure.FSMC_MemoryType = FSMC_MemoryType_SRAM;	// FSMC_MemoryType_PSRAM;
//	FSMC_NORSRAMInitStructure.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_16b;
//	FSMC_NORSRAMInitStructure.FSMC_BurstAccessMode = FSMC_BurstAccessMode_Disable;
//	FSMC_NORSRAMInitStructure.FSMC_AsynchronousWait = FSMC_AsynchronousWait_Disable;
//	FSMC_NORSRAMInitStructure.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low;
//	FSMC_NORSRAMInitStructure.FSMC_WrapMode = FSMC_WrapMode_Disable;
//	FSMC_NORSRAMInitStructure.FSMC_WaitSignalActive = FSMC_WaitSignalActive_BeforeWaitState;
//	FSMC_NORSRAMInitStructure.FSMC_WriteOperation = FSMC_WriteOperation_Enable;
//	FSMC_NORSRAMInitStructure.FSMC_WaitSignal = FSMC_WaitSignal_Disable;
//	FSMC_NORSRAMInitStructure.FSMC_ExtendedMode = FSMC_ExtendedMode_Disable;
//	FSMC_NORSRAMInitStructure.FSMC_WriteBurst = FSMC_WriteBurst_Disable;
//	FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct = &p;
//	FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct = &p;

//	FSMC_NORSRAMInit(&FSMC_NORSRAMInitStructure);

//	/*!< Enable FSMC Bank1_SRAM3 Bank */
//	FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM3, ENABLE);
	
	
	hsram.Instance  = FMC_NORSRAM_DEVICE;
	hsram.Extended  = FMC_NORSRAM_EXTENDED_DEVICE;
	
	/* FMCʹ�õ�HCLK����Ƶ168MHz��1��FMCʱ�����ھ���5.95ns */
	SRAM_TimingRead.AddressSetupTime       = 3;  /* 4*5.95ns����ַ����ʱ�䣬��Χ0 -15��FMCʱ�����ڸ��� */
	SRAM_TimingRead.AddressHoldTime        = 0;  /* ��ַ����ʱ�䣬����ΪģʽAʱ���ò����˲��� ��Χ1 -15��ʱ�����ڸ��� */
	SRAM_TimingRead.DataSetupTime          = 2;  /* 6*5.95ns�����ݱ���ʱ�䣬��Χ1 -255��ʱ�����ڸ��� */
	SRAM_TimingRead.BusTurnAroundDuration  = 1;  /* �����������ݶ�д��ʱ����*/
	SRAM_TimingRead.CLKDivision            = 0;  /* �������ò���������� */
	SRAM_TimingRead.DataLatency            = 0;  /* �������ò���������� */
	SRAM_TimingRead.AccessMode             = FSMC_ACCESS_MODE_A; /* ����ΪģʽA */
	
	hsram.Init.NSBank             = FSMC_NORSRAM_BANK3;              /* ʹ�õ�BANK4����ʹ�õ�ƬѡFSMC_NE4 */
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
	if (HAL_SRAM_Init(&hsram, &SRAM_TimingRead, &SRAM_TimingRead) != HAL_OK)
	{
		/* ��ʼ������ */
		Error_Handler(__FILE__, __LINE__);
	}	
}

/*
*********************************************************************************************************
*	�� �� ��: bsp_TestExtSRAM
*	����˵��: ɨ������ⲿSRAM
*	��    ��: ��
*	�� �� ֵ: 0 ��ʾ����ͨ���� ����0��ʾ����Ԫ�ĸ�����
*********************************************************************************************************
*/
uint8_t bsp_TestExtSRAM(void)
{
	uint32_t i;
	uint32_t *pSRAM;
	uint8_t *pBytes;
	uint32_t err;
	const uint8_t ByteBuf[4] = {0x55, 0xA5, 0x5A, 0xAA};

	/* дSRAM */
	pSRAM = (uint32_t *)EXT_SRAM_ADDR;
	for (i = 0; i < EXT_SRAM_SIZE / 4; i++)
	{
		*pSRAM++ = i;
	}

	/* ��SRAM */
	err = 0;
	pSRAM = (uint32_t *)EXT_SRAM_ADDR;
	for (i = 0; i < EXT_SRAM_SIZE / 4; i++)
	{
		if (*pSRAM++ != i)
		{
			err++;
		}
	}

	if (err >  0)
	{
		return  (4 * err);
	}

#if 0
	/* ��SRAM �������󷴲�д�� */
	pSRAM = (uint32_t *)EXT_SRAM_ADDR;
	for (i = 0; i < EXT_SRAM_SIZE / 4; i++)
	{
		*pSRAM = ~*pSRAM;
		pSRAM++;
	}

	/* �ٴαȽ�SRAM������ */
	err = 0;
	pSRAM = (uint32_t *)EXT_SRAM_ADDR;
	for (i = 0; i < EXT_SRAM_SIZE / 4; i++)
	{
		if (*pSRAM++ != (~i))
		{
			err++;
		}
	}

	if (err >  0)
	{
		return (4 * err);
	}
#endif

	/* ���԰��ֽڷ�ʽ����, Ŀ������֤ FSMC_NBL0 �� FSMC_NBL1 ���� */
	pBytes = (uint8_t *)EXT_SRAM_ADDR;
	for (i = 0; i < sizeof(ByteBuf); i++)
	{
		*pBytes++ = ByteBuf[i];
	}

	/* �Ƚ�SRAM������ */
	err = 0;
	pBytes = (uint8_t *)EXT_SRAM_ADDR;
	for (i = 0; i < sizeof(ByteBuf); i++)
	{
		if (*pBytes++ != ByteBuf[i])
		{
			err++;
		}
	}
	if (err >  0)
	{
		return err;
	}
	return 0;
}

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
