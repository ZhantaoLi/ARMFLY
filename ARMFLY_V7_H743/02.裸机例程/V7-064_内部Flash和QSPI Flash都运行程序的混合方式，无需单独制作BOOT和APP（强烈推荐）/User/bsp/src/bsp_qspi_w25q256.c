/*
*********************************************************************************************************
*
*	ģ������ : W25Q256 QSPI����ģ��
*	�ļ����� : bsp_qspi_w25q256.c
*	��    �� : V1.0
*	˵    �� : ʹ��CPU��QSPI������������FLASH���ṩ�����Ķ�д����������4�߷�ʽ��MDMA����
*
*	�޸ļ�¼ :
*		�汾��  ����        ����     ˵��
*		V1.0    2020-11-01  armfly  ��ʽ����
*
*	Copyright (C), 2020-2030, ���������� www.armfly.com
*
*********************************************************************************************************
*/

#include "bsp.h"



/* 
    STM32-V7���������

	PG6/QUADSPI_BK1_NCS AF10
	PF10/QUADSPI_CLK	AF9
	PF8/QUADSPI_BK1_IO0 AF10
	PF9/QUADSPI_BK1_IO1 AF10
	PF7/QUADSPI_BK1_IO2 AF9
	PF6/QUADSPI_BK1_IO3 AF9

	W25Q256JV��512�飬ÿ����16��������ÿ������Sector��16ҳ��ÿҳ��256�ֽڣ�����32MB
*/

/* QSPI���ź�ʱ��������ú궨�� */
#define QSPI_CLK_ENABLE()              __HAL_RCC_QSPI_CLK_ENABLE()
#define QSPI_CLK_DISABLE()             __HAL_RCC_QSPI_CLK_DISABLE()
#define QSPI_CS_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOG_CLK_ENABLE()
#define QSPI_CLK_GPIO_CLK_ENABLE()     __HAL_RCC_GPIOF_CLK_ENABLE()
#define QSPI_BK1_D0_GPIO_CLK_ENABLE()  __HAL_RCC_GPIOF_CLK_ENABLE()
#define QSPI_BK1_D1_GPIO_CLK_ENABLE()  __HAL_RCC_GPIOF_CLK_ENABLE()
#define QSPI_BK1_D2_GPIO_CLK_ENABLE()  __HAL_RCC_GPIOF_CLK_ENABLE()
#define QSPI_BK1_D3_GPIO_CLK_ENABLE()  __HAL_RCC_GPIOF_CLK_ENABLE()

#define QSPI_MDMA_CLK_ENABLE()         __HAL_RCC_MDMA_CLK_ENABLE()
#define QSPI_FORCE_RESET()             __HAL_RCC_QSPI_FORCE_RESET()
#define QSPI_RELEASE_RESET()           __HAL_RCC_QSPI_RELEASE_RESET()

#define QSPI_CS_PIN                    GPIO_PIN_6
#define QSPI_CS_GPIO_PORT              GPIOG

#define QSPI_CLK_PIN                   GPIO_PIN_10
#define QSPI_CLK_GPIO_PORT             GPIOF

#define QSPI_BK1_D0_PIN                GPIO_PIN_8
#define QSPI_BK1_D0_GPIO_PORT          GPIOF

#define QSPI_BK1_D1_PIN                GPIO_PIN_9
#define QSPI_BK1_D1_GPIO_PORT          GPIOF

#define QSPI_BK1_D2_PIN                GPIO_PIN_7
#define QSPI_BK1_D2_GPIO_PORT          GPIOF

#define QSPI_BK1_D3_PIN                GPIO_PIN_6
#define QSPI_BK1_D3_GPIO_PORT          GPIOF


/* �����ļ����õ�ȫ�ֱ��� */
static QSPI_HandleTypeDef 	QSPIHandle;

/* �����ļ����õĺ��� */
static void QSPI_WriteEnable(QSPI_HandleTypeDef *hqspi);
static void QSPI_AutoPollingMemReady(QSPI_HandleTypeDef *hqspi);


/*
*********************************************************************************************************
*	�� �� ��: bsp_InitQSPI_W25Q256
*	����˵��: QSPI FlashӲ����ʼ�������û�������
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void bsp_InitQSPI_W25Q256(void)
{
	/* ��λQSPI */
	QSPIHandle.Instance = QUADSPI;
	if (HAL_QSPI_DeInit(&QSPIHandle) != HAL_OK)
	{
		Error_Handler(__FILE__, __LINE__);
	}
	
	/* ����ʱ���ٶȣ�QSPI clock = 200MHz / (ClockPrescaler+1) = 100MHz */
	QSPIHandle.Init.ClockPrescaler  = 1;  
	
	/* ����FIFO��ֵ����Χ1 - 32 */
	QSPIHandle.Init.FifoThreshold   = 32; 
	
	/* 
		QUADSPI��FLASH�����źź�����CLK���ڲŶ�FLASH���������ݲ�����
		���ⲿ�ź��ӳ�ʱ�����������Ƴ����ݲ�����
	*/
	QSPIHandle.Init.SampleShifting  = QSPI_SAMPLE_SHIFTING_HALFCYCLE; 
	
	/*Flash��С��2^(FlashSize + 1) = 2^25 = 32MB */
    //QSPI_FLASH_SIZE - 1; ��Ҫ����һ���������ڴ�ӳ�䷽λ���1����ַʱ�����쳣��
	QSPIHandle.Init.FlashSize       = QSPI_FLASH_SIZE; 
	
	/* ����֮���CSƬѡ���ٱ���2��ʱ�����ڵĸߵ�ƽ */
	QSPIHandle.Init.ChipSelectHighTime = QSPI_CS_HIGH_TIME_2_CYCLE;
	
	/*
	   MODE0: ��ʾƬѡ�źſ����ڼ䣬CLKʱ���ź��ǵ͵�ƽ
	   MODE3: ��ʾƬѡ�źſ����ڼ䣬CLKʱ���ź��Ǹߵ�ƽ
	*/
	QSPIHandle.Init.ClockMode = QSPI_CLOCK_MODE_0;
	
	/* QSPI������BANK������ʹ�õ�BANK1 */
	QSPIHandle.Init.FlashID   = QSPI_FLASH_ID_1;
	
	/* V7�������ʹ����BANK1�������ǽ�ֹ˫BANK */
	QSPIHandle.Init.DualFlash = QSPI_DUALFLASH_DISABLE;

	/* ��ʼ������QSPI */
	if (HAL_QSPI_Init(&QSPIHandle) != HAL_OK)
	{
		Error_Handler(__FILE__, __LINE__);
	}	
}	

/*
*********************************************************************************************************
*	�� �� ��: HAL_QSPI_MspInit
*	����˵��: QSPI�ײ��ʼ������HAL_QSPI_Init���õĵײ㺯��
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void HAL_QSPI_MspInit(QSPI_HandleTypeDef *hqspi)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	/* ʹ��QPSIʱ��  */
	QSPI_CLK_ENABLE();
	
	/* ��λʱ�ӽӿ� */
	QSPI_FORCE_RESET();
	QSPI_RELEASE_RESET();
	
	/* ʹ��GPIOʱ�� */
	QSPI_CS_GPIO_CLK_ENABLE();
	QSPI_CLK_GPIO_CLK_ENABLE();
	QSPI_BK1_D0_GPIO_CLK_ENABLE();
	QSPI_BK1_D1_GPIO_CLK_ENABLE();
	QSPI_BK1_D2_GPIO_CLK_ENABLE();
	QSPI_BK1_D3_GPIO_CLK_ENABLE(); 

	/* QSPI CS GPIO �������� */
	GPIO_InitStruct.Pin       = QSPI_CS_PIN;
	GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull      = GPIO_PULLUP;
	GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF10_QUADSPI;
	HAL_GPIO_Init(QSPI_CS_GPIO_PORT, &GPIO_InitStruct);
	
	/* QSPI CLK GPIO �������� */
	GPIO_InitStruct.Pin       = QSPI_CLK_PIN;
	GPIO_InitStruct.Pull      = GPIO_NOPULL;
	GPIO_InitStruct.Alternate = GPIO_AF9_QUADSPI;
	HAL_GPIO_Init(QSPI_CLK_GPIO_PORT, &GPIO_InitStruct);

	/* QSPI D0 GPIO pin �������� */
	GPIO_InitStruct.Pin       = QSPI_BK1_D0_PIN;
	GPIO_InitStruct.Alternate = GPIO_AF10_QUADSPI;
	HAL_GPIO_Init(QSPI_BK1_D0_GPIO_PORT, &GPIO_InitStruct);

	/* QSPI D1 GPIO �������� */
	GPIO_InitStruct.Pin       = QSPI_BK1_D1_PIN;
	GPIO_InitStruct.Alternate = GPIO_AF10_QUADSPI;
	HAL_GPIO_Init(QSPI_BK1_D1_GPIO_PORT, &GPIO_InitStruct);

	/* QSPI D2 GPIO �������� */
	GPIO_InitStruct.Pin       = QSPI_BK1_D2_PIN;
	GPIO_InitStruct.Alternate = GPIO_AF9_QUADSPI;
	HAL_GPIO_Init(QSPI_BK1_D2_GPIO_PORT, &GPIO_InitStruct);

	/* QSPI D3 GPIO �������� */
	GPIO_InitStruct.Pin       = QSPI_BK1_D3_PIN;
	GPIO_InitStruct.Alternate = GPIO_AF9_QUADSPI;
	HAL_GPIO_Init(QSPI_BK1_D3_GPIO_PORT, &GPIO_InitStruct);
}

/*
*********************************************************************************************************
*	�� �� ��: HAL_QSPI_MspDeInit
*	����˵��: QSPI�ײ㸴λ����HAL_QSPI_Init���õĵײ㺯��
*	��    ��: hqspi QSPI_HandleTypeDef���;��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void HAL_QSPI_MspDeInit(QSPI_HandleTypeDef *hqspi)
{
	/* ��λQSPI���� */
	HAL_GPIO_DeInit(QSPI_CS_GPIO_PORT, QSPI_CS_PIN);
	HAL_GPIO_DeInit(QSPI_CLK_GPIO_PORT, QSPI_CLK_PIN);
	HAL_GPIO_DeInit(QSPI_BK1_D0_GPIO_PORT, QSPI_BK1_D0_PIN);
	HAL_GPIO_DeInit(QSPI_BK1_D1_GPIO_PORT, QSPI_BK1_D1_PIN);
	HAL_GPIO_DeInit(QSPI_BK1_D2_GPIO_PORT, QSPI_BK1_D2_PIN);
	HAL_GPIO_DeInit(QSPI_BK1_D3_GPIO_PORT, QSPI_BK1_D3_PIN);

	/* ��λQSPI */
	QSPI_FORCE_RESET();
	QSPI_RELEASE_RESET();

	/* �ر�QSPIʱ�� */
	QSPI_CLK_DISABLE();
}

/*
*********************************************************************************************************
*	�� �� ��: QSPI_EraseSector
*	����˵��: ����ָ����������������С4KB
*	��    ��: _uiSectorAddr : ������ַ����4KBΪ��λ�ĵ�ַ������0��4096, 8192��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void QSPI_EraseSector(uint32_t _uiSectorAddr)
{
	QSPI_CommandTypeDef sCommand={0};

	/* дʹ�� */
	QSPI_WriteEnable(&QSPIHandle);	

	/* �������� */
	sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;    /* 1�߷�ʽ����ָ�� */
	sCommand.AddressSize       = QSPI_ADDRESS_32_BITS;       /* 32λ��ַ */
	sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;  /* �޽����ֽ� */
	sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;      /* W25Q256JV��֧��DDR */
	sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;  /* DDRģʽ����������ӳ� */
	sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;	 /* ÿ�δ��䶼��ָ�� */	
	
	/* �������� */
	sCommand.Instruction = SUBSECTOR_ERASE_4_BYTE_ADDR_CMD;   /* 32bit��ַ��ʽ�������������������С4KB*/       
	sCommand.AddressMode = QSPI_ADDRESS_1_LINE;  /* ��ַ������1�߷�ʽ */       
	sCommand.Address     = _uiSectorAddr;        /* �����׵�ַ����֤��4KB������ */    
	sCommand.DataMode    = QSPI_DATA_NONE;       /* ���跢������ */  
	sCommand.DummyCycles = 0;                    /* ��������� */  

	if (HAL_QSPI_Command(&QSPIHandle, &sCommand, 10000) != HAL_OK)
	{
		Error_Handler(__FILE__, __LINE__);
	}
	
	QSPI_AutoPollingMemReady(&QSPIHandle);	
}

/*
*********************************************************************************************************
*	�� �� ��: QSPI_EraseChip
*	����˵��: ����оƬ����
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void QSPI_EraseChip(void)
{
	QSPI_CommandTypeDef sCommand={0};

	/* дʹ�� */
	QSPI_WriteEnable(&QSPIHandle);	

	/* �������� */
	sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;    /* 1�߷�ʽ����ָ�� */
	sCommand.AddressSize       = QSPI_ADDRESS_32_BITS;       /* 32λ��ַ */
	sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;  /* �޽����ֽ� */
	sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;      /* W25Q256JV��֧��DDR */
	sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;  /* DDRģʽ����������ӳ� */
	sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;	 /* ÿ�δ��䶼��ָ�� */	
	
	/* �������� */
	sCommand.Instruction = BULK_ERASE_CMD;       /* ����оƬ��������*/       
	sCommand.AddressMode = QSPI_ADDRESS_1_LINE;  /* ��ַ������1�߷�ʽ */       
	sCommand.Address     = 0;                    /* ��ַ */    
	sCommand.DataMode    = QSPI_DATA_NONE;       /* ���跢������ */  
	sCommand.DummyCycles = 0;                    /* ��������� */  

	if (HAL_QSPI_Command(&QSPIHandle, &sCommand, 10000) != HAL_OK)
	{
		Error_Handler(__FILE__, __LINE__);
	}
	
	QSPI_AutoPollingMemReady(&QSPIHandle);	
}

/*
*********************************************************************************************************
*	�� �� ��: QSPI_WriteBuffer
*	����˵��: ҳ��̣�ҳ��С256�ֽڣ�����ҳ������д��
*	��    ��: _pBuf : ����Դ��������
*			  _uiWriteAddr ��Ŀ�������׵�ַ����ҳ�׵�ַ������0�� 256, 512�ȡ�
*			  _usWriteSize �����ݸ��������ܳ���ҳ���С����Χ1 - 256��
*	�� �� ֵ: 1:�ɹ��� 0��ʧ��
*********************************************************************************************************
*/
uint8_t QSPI_WriteBuffer(uint8_t *_pBuf, uint32_t _uiWriteAddr, uint16_t _usWriteSize)
{
	QSPI_CommandTypeDef sCommand={0};

	/* дʹ�� */
	QSPI_WriteEnable(&QSPIHandle);	
	
	/* �������� */
	sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;    /* 1�߷�ʽ����ָ�� */
	sCommand.AddressSize       = QSPI_ADDRESS_32_BITS;       /* 32λ��ַ */
	sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;  /* �޽����ֽ� */
	sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;      /* W25Q256JV��֧��DDR */
	sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;  /* DDRģʽ����������ӳ� */
	sCommand.SIOOMode          = QSPI_SIOO_INST_ONLY_FIRST_CMD;	 /* ������һ������ */	
	
	/* д�������� */
	sCommand.Instruction = QUAD_IN_FAST_PROG_4_BYTE_ADDR_CMD; /* 32bit��ַ��4�߿���д������ */
	sCommand.DummyCycles = 0;                    /* ����Ҫ������ */
	sCommand.AddressMode = QSPI_ADDRESS_1_LINE;  /* 4�ߵ�ַ��ʽ */
	sCommand.DataMode    = QSPI_DATA_4_LINES;    /* 4�����ݷ�ʽ */
	sCommand.NbData      = _usWriteSize;         /* д���ݴ�С */   
	sCommand.Address     = _uiWriteAddr;         /* д���ַ */
	
	if (HAL_QSPI_Command(&QSPIHandle, &sCommand, 10000) != HAL_OK)
	{
        //return 0;
		Error_Handler(__FILE__, __LINE__);
	}
	
	/* �������� */
	if (HAL_QSPI_Transmit(&QSPIHandle, _pBuf, 10000) != HAL_OK)
	{
        //return 0;
		Error_Handler(__FILE__, __LINE__);
        
	}
	
	QSPI_AutoPollingMemReady(&QSPIHandle);	
	
	return 1;
}

/*
*********************************************************************************************************
*	�� �� ��: QSPI_ReadBuffer
*	����˵��: ������ȡ�����ֽڣ��ֽڸ������ܳ���оƬ������
*	��    ��: _pBuf : ����Դ��������
*			  _uiReadAddr ����ʼ��ַ��
*			  _usSize �����ݸ���, ���Դ���PAGE_SIZE, ���ǲ��ܳ���оƬ��������
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void QSPI_ReadBuffer(uint8_t * _pBuf, uint32_t _uiReadAddr, uint32_t _uiSize)
{
	
	QSPI_CommandTypeDef sCommand = {0};
	
	
	/* �������� */
	sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;    	/* 1�߷�ʽ����ָ�� */
	sCommand.AddressSize       = QSPI_ADDRESS_32_BITS;      	/* 32λ��ַ */
	sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;  	/* �޽����ֽ� */
	sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;      	/* W25Q256JV��֧��DDR */
	sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;  	/* DDRģʽ����������ӳ� */
	sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;		/* ÿ�δ���Ҫ��ָ�� */	
 
	/* ��ȡ���� */
	sCommand.Instruction = QUAD_INOUT_FAST_READ_4_BYTE_ADDR_CMD; /* 32bit��ַ��4�߿��ٶ�ȡ���� */
	sCommand.DummyCycles = 6;                    /* ������ */
	sCommand.AddressMode = QSPI_ADDRESS_4_LINES; /* 4�ߵ�ַ */
	sCommand.DataMode    = QSPI_DATA_4_LINES;    /* 4������ */ 
	sCommand.NbData      = _uiSize;              /* ��ȡ�����ݴ�С */ 
	sCommand.Address     = _uiReadAddr;          /* ��ȡ���ݵ���ʼ��ַ */ 
	
	if (HAL_QSPI_Command(&QSPIHandle, &sCommand, 10000) != HAL_OK)
	{
		Error_Handler(__FILE__, __LINE__);
	}

	/* ��ȡ */
	if (HAL_QSPI_Receive(&QSPIHandle, _pBuf, 10000) != HAL_OK)
	{
		Error_Handler(__FILE__, __LINE__);
	}	
}

/*
*********************************************************************************************************
*	�� �� ��: QSPI_WriteEnable
*	����˵��: дʹ��
*	��    ��: hqspi  QSPI_HandleTypeDef�����
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void QSPI_WriteEnable(QSPI_HandleTypeDef *hqspi)
{
	QSPI_CommandTypeDef     sCommand = {0};
	
	/* �������� */
	sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;    /* 1�߷�ʽ����ָ�� */
	sCommand.AddressSize       = QSPI_ADDRESS_32_BITS;       /* 32λ��ַ */
	sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;  /* �޽����ֽ� */
	sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;      /* W25Q256JV��֧��DDR */
	sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;  /* DDRģʽ����������ӳ� */
	sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;	 /* ÿ�δ��䶼��ָ�� */
	
	/* дʹ�� */
	sCommand.Instruction       = WRITE_ENABLE_CMD;  /* дʹ��ָ�� */
	sCommand.AddressMode       = QSPI_ADDRESS_NONE; /* �����ַ */
	sCommand.DataMode          = QSPI_DATA_NONE;    /* �������� */
	sCommand.DummyCycles       = 0;                 /* ������  */

	if (HAL_QSPI_Command(&QSPIHandle, &sCommand, 10000) != HAL_OK)
	{
		Error_Handler(__FILE__, __LINE__);
	}	
}

/*
*********************************************************************************************************
*	�� �� ��: QSPI_AutoPollingMemReady
*	����˵��: �ȴ�QSPI Flash��������Ҫ����Flash������ҳ���ʱʹ��
*	��    ��: hqspi  QSPI_HandleTypeDef���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void QSPI_AutoPollingMemReady(QSPI_HandleTypeDef *hqspi)
{
	QSPI_CommandTypeDef     sCommand = {0};
	QSPI_AutoPollingTypeDef sConfig = {0};

	
	/* �������� */
	sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;    /* 1�߷�ʽ����ָ�� */
	sCommand.AddressSize       = QSPI_ADDRESS_32_BITS;       /* 32λ��ַ */
	sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;  /* �޽����ֽ� */
	sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;      /* W25Q256JV��֧��DDR */
	sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;  /* DDRģʽ����������ӳ� */
	sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;	 /* ÿ�δ��䶼��ָ�� */
	
	/* ��ȡ״̬*/
	sCommand.Instruction       = READ_STATUS_REG_CMD; /* ��ȡ״̬���� */
	sCommand.AddressMode       = QSPI_ADDRESS_NONE;   /* �����ַ */
	sCommand.DataMode          = QSPI_DATA_1_LINE;    /* 1������ */
	sCommand.DummyCycles       = 0;                   /* ��������� */

	/* ����λ���õ�bit0��ƥ��λ�ȴ�bit0Ϊ0�������ϲ�ѯ״̬�Ĵ���bit0���ȴ���Ϊ0 */
	sConfig.Mask            = 0x01;
	sConfig.Match           = 0x00;
	sConfig.MatchMode       = QSPI_MATCH_MODE_AND;
	sConfig.StatusBytesSize = 1;
	sConfig.Interval        = 0x10;
	sConfig.AutomaticStop   = QSPI_AUTOMATIC_STOP_ENABLE;

	if (HAL_QSPI_AutoPolling(&QSPIHandle, &sCommand, &sConfig, 10000) != HAL_OK)
	{
		Error_Handler(__FILE__, __LINE__);
	}
}

/*
*********************************************************************************************************
*    �� �� ��: QSPI_MemoryMapped
*    ����˵��: QSPI�ڴ�ӳ�䣬��ַ 0x90000000
*    ��    ��: ��
*    �� �� ֵ: ��
*********************************************************************************************************
*/
void QSPI_MemoryMapped(void)
{
    QSPI_CommandTypeDef s_command = {0};
    QSPI_MemoryMappedTypeDef s_mem_mapped_cfg = {0};

    /* �������� */
    s_command.InstructionMode = QSPI_INSTRUCTION_1_LINE;      /* 1�߷�ʽ����ָ�� */ 
    s_command.AddressSize = QSPI_ADDRESS_32_BITS;             /* 32λ��ַ */
    s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;  /* �޽����ֽ� */
    s_command.DdrMode = QSPI_DDR_MODE_DISABLE;                /* W25Q256JV��֧��DDR */
    s_command.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;   /* DDRģʽ����������ӳ� */
    s_command.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;            /* ÿ�δ��䶼��ָ�� */
    
    /* ȫ������4�� */
    s_command.Instruction = QUAD_INOUT_FAST_READ_4_BYTE_ADDR_CMD; /* ���ٶ�ȡ���� */
    s_command.AddressMode = QSPI_ADDRESS_4_LINES;                 /* 4����ַ�� */
    s_command.DataMode = QSPI_DATA_4_LINES;                       /* 4�������� */
    s_command.DummyCycles = 6;                                    /* ������ */

    /* �ر�������� */
    s_mem_mapped_cfg.TimeOutActivation = QSPI_TIMEOUT_COUNTER_DISABLE;
    s_mem_mapped_cfg.TimeOutPeriod = 0;

    if (HAL_QSPI_MemoryMapped(&QSPIHandle, &s_command, &s_mem_mapped_cfg) != HAL_OK)
    {
       Error_Handler(__FILE__, __LINE__);
    }
}

/*
*********************************************************************************************************
*	�� �� ��: sf_ReadID
*	����˵��: ��ȡ����ID
*	��    ��: ��
*	�� �� ֵ: 32bit������ID (���8bit��0����ЧIDλ��Ϊ24bit��
*********************************************************************************************************
*/
uint32_t QSPI_ReadID(void)
{
	uint32_t uiID;
	QSPI_CommandTypeDef s_command = {0};
	uint8_t buf[3];

	/* �������� */
	s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;    /* 1�߷�ʽ����ָ�� */
	s_command.AddressSize       = QSPI_ADDRESS_32_BITS;       /* 32λ��ַ */
	s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;  /* �޽����ֽ� */
	s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;      /* W25Q256JV��֧��DDR */
	s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;  /* DDRģʽ����������ӳ� */
	s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;	  /* ÿ�δ��䶼��ָ�� */
	
	/* ��ȡJEDEC ID */
	s_command.Instruction = READ_ID_CMD2;         /* ��ȡID���� */
	s_command.AddressMode = QSPI_ADDRESS_NONE;    /* 1�ߵ�ַ */
	s_command.DataMode = QSPI_DATA_1_LINE;        /* 1�ߵ�ַ */
	s_command.DummyCycles = 0;                    /* �޿����� */
	s_command.NbData = 3;                         /* ��ȡ�������� */

	if (HAL_QSPI_Command(&QSPIHandle, &s_command, 10000) != HAL_OK) 
	{
       Error_Handler(__FILE__, __LINE__);
	}

	if (HAL_QSPI_Receive(&QSPIHandle, buf, 10000) != HAL_OK) 
	{
       Error_Handler(__FILE__, __LINE__);
	}

	uiID = (buf[0] << 16) | (buf[1] << 8 ) | buf[2];

	return uiID;
}

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
