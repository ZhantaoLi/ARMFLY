/*
*********************************************************************************************************
*
*	ģ������ : NAND Flash����ģ��
*	�ļ����� : NAND_STM32F1xx.c
*	��    �� : V1.0
*	˵    �� : �ṩNAND Flash (HY27UF081G2A�� 8bit 128K�ֽ� ��ҳ)�ĵײ�ӿں�������������ԭ����
*              ����bsp_nand_flash.c�ļ��޸Ķ�����
*
*	�޸ļ�¼ :
*		�汾��  ����        ����       ˵��
*		V1.0    2019-07-01  Eric2013  ��ʽ����
*
*	Copyright (C), 2019-2030, ���������� www.armfly.com
*
*********************************************************************************************************
*/
#include "bsp.h"


/* ����PG6�ĳ�ʱ�жϺ�ʹ�� */
#define TIMEOUT_COUNT		400000000  
#define ENABLE_TIMEOUT		0


/*
	��NAND Flash �ṹ���塿
     ��������16x4�ֽڣ�ÿpage 2048�ֽڣ�ÿ512�ֽ�һ��������ÿ��������Ӧ16�Լ��ı�������

	 ÿ��PAGE���߼��ṹ��ǰ��512Bx4����������������16Bx4�Ǳ�����
	��������������������������������������������������������������������������������������������������������������������������������
	�� Main area  ���� Main area  ���� Main area  ����Main area   ���� Spare area ���� Spare area ���� Spare area ����Spare area  ��
	��            ����            ����            ����            ����            ����            ����            ����            ��
	��   512B     ����    512B    ����    512B    ����    512B    ����    16B     ����     16B    ����     16B    ����    16B     ��
	��������������������������������������������������������������������������������������������������������������������������������

	 ÿ16B�ı��������߼��ṹ����:(FlashFS�е����ã�
	������������������������������������������������������������������������������������������������������������������������������������������������������
	��LSN0  ����LSN1  ����LSN2  ����LSN3  ����COR ����BBM   ����ECC0  ����ECC1  ����ECC2����ECC3����ECC4����ECC5����  ECC6����  ECC7���� ECC8 ���� ECC9 ��
	��      ����      ����      ����      ����    ����      ����      ����      ����    ����    ����    ����    ����      ����      ����      ����      ��
	������������������������������������������������������������������������������������������������������������������������������������������������������

    - LSN0 ~ LSN3 : �߼�������(logical sector number) ��
	- COR         : �����𻵱�ǣ�data corrupted marker����
	- BBM         : �����־(Bad Block Marker)��
    - ECC0 ~ ECC9 : 512B����������ECCУ�顣

	K9F1G08U0A �� HY27UF081G2A �Ǽ��ݵġ�оƬ����ʱ�����̱�֤оƬ�ĵ�1�����Ǻÿ顣����ǻ��飬���ڸÿ�ĵ�1��PAGE�ĵ�1���ֽ�
	���ߵ�2��PAGE������1��PAGE�����޷����Ϊ0xFFʱ���ĵ�1���ֽ�д���0xFFֵ��������ֵ������ģ����ֱ���ж��Ƿ����0xFF���ɡ�

	ע�⣺������Щ����˵NAND Flash���̵�Ĭ�������ǽ������Ƕ��ڵ�1��PAGE�ĵ�6���ֽڴ������˵���Ǵ��󡣻������ڵ�6���ֽڽ���Բ���С������512�ֽڣ���NAND Flash
	���������е�NAND Flash���������׼������ڸ���NAND Flashʱ������ϸ�Ķ�оƬ�������ֲᡣ

*/

/* ����NAND Flash�������ַ���������Ӳ�������� */
#define Bank_NAND_ADDR     ((uint32_t)0x80000000)

/* �������NAND Flash�õ�3���� */
#define NAND_CMD_AREA		*(__IO uint8_t *)(Bank_NAND_ADDR | CMD_AREA)
#define NAND_ADDR_AREA		*(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA)
#define NAND_DATA_AREA		*(__IO uint8_t *)(Bank_NAND_ADDR | DATA_AREA)


#define BUSY_GPIO	GPIOD
#define BUSY_PIN	GPIO_PIN_6

/* ����ȴ���������������쳣, �˴�Ӧ���жϳ�ʱ*/
#define WAIT_BUSY()	{	\
	uint16_t k; 	\
	for (k = 0; k < 200; k++)	\
	{	\
		if ((BUSY_GPIO->IDR & BUSY_PIN) == 0) break;	\
	}	\
	for (k = 0; k < 2000; k++)	\
	{	\
		if ((BUSY_GPIO->IDR & BUSY_PIN) != 0) break;	\
	}	\
}

static uint32_t FMC_NAND_GetStatus(void);

static uint32_t FMC_NAND_EraseBlock(uint32_t _ulBlockNo);
static void bsp_FMC_NAND_Init(void);
static uint8_t FMC_NAND_Reset(void);


/**
  \fn          int32_t Driver_NANDx_GetDeviceBusy (uint32_t dev_num)
  \brief       NAND Driver GetDeviceBusy callback.
               Needs to be implemented by user.
  \param[in]   dev_num   Device number
  \return      1=busy, 0=not busy, or error
*/
int32_t Driver_NAND0_GetDeviceBusy (uint32_t dev_num)
{
	WAIT_BUSY();
	
	return 0;
}
/*
*********************************************************************************************************
*	�� �� ��: NAND_Init
*	����˵��: ��ʼ��NAND Flash�ӿ�
*	��    ��:  ��
*	�� �� ֵ: ִ�н����
*			  - NAND_FAIL ��ʾʧ��
*			  - NAND_OK ��ʾ�ɹ�
*********************************************************************************************************
*/
uint8_t NAND_Init(void)
{
	bsp_FMC_NAND_Init();		/* ����FSMC��GPIO����NAND Flash�ӿ� */

	FMC_NAND_Reset();			/* ͨ����λ���λNAND Flash����״̬ */

	return 0;
}

/*
*********************************************************************************************************
*	�� �� ��: bsp_FMC_NAND_Init
*	����˵��: ����FSMC��GPIO����NAND Flash�ӿڡ�������������ڶ�дnand flashǰ������һ�Ρ�
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void bsp_FMC_NAND_Init(void)
{
	/* --NAND Flash GPIOs ����  ------*/
	{
		/*
			PD0/FMC_D2
			PD1/FMC_D3
			PD4/FMC_NOE
			PD5/FMC_NWE
		
				PD7/FMC_NCE2  --�滻Ϊ PG9/FMC_NCE3
		
			PD11/FMC_A16/FMC_CLE
			PD12/FMC_A17/FMC_ALE
			PD14/FMC_D0
			PD15/FMC_D1

			PE7/FMC_D4
			PE8/FMC_D5
			PE9/FMC_D6
			PE10/FMC_D7

			PD6/FMC_NWAIT	(�������ò�ѯ��ʽ��æ���˿�����Ϊ��ͨGPIO���빦��ʹ��)
		*/
		GPIO_InitTypeDef gpio_init_structure;

		/* ʹ�� GPIOʱ�� */
		__HAL_RCC_GPIOD_CLK_ENABLE();
		__HAL_RCC_GPIOE_CLK_ENABLE();

		/* ʹ��FMCʱ�� */
		__HAL_RCC_FMC_CLK_ENABLE();

		/* ���� GPIOD ��ص�IOΪ����������� */
		gpio_init_structure.Mode = GPIO_MODE_AF_PP;
		gpio_init_structure.Pull = GPIO_PULLUP;
		gpio_init_structure.Speed = GPIO_SPEED_FREQ_MEDIUM;
		gpio_init_structure.Alternate = GPIO_AF12_FMC;
		
		/* ����GPIOD */
		gpio_init_structure.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_4 | GPIO_PIN_5 |
									GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_14 |
									GPIO_PIN_15;
		HAL_GPIO_Init(GPIOD, &gpio_init_structure);

		/* ����GPIOE */
		gpio_init_structure.Pin = GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10;
		HAL_GPIO_Init(GPIOE, &gpio_init_structure);
		
		/* ����GPIOG NAND Ƭѡ */
		gpio_init_structure.Pin = GPIO_PIN_9;
		HAL_GPIO_Init(GPIOG, &gpio_init_structure);		

		/* busy R/B ����Ϊ���� */
		gpio_init_structure.Mode = GPIO_MODE_INPUT;			/* �������� */
		gpio_init_structure.Pull = GPIO_NOPULL;				/* ���������費ʹ�� */
		gpio_init_structure.Speed = GPIO_SPEED_FREQ_MEDIUM;  	/* GPIO�ٶȵȼ� */
		gpio_init_structure.Pin = BUSY_PIN;	
		HAL_GPIO_Init(BUSY_GPIO, &gpio_init_structure);	
	}

	{
		NAND_HandleTypeDef hNand = {0};
		
		FMC_NAND_PCC_TimingTypeDef CommonSpaceTiming;
		FMC_NAND_PCC_TimingTypeDef AttributeSpaceTiming;
		
		hNand.Instance = FMC_NAND_DEVICE;									
		hNand.Init.NandBank = FMC_NAND_BANK3;						/* ����FSMC NAND BANK �� */
		hNand.Init.Waitfeature = FMC_NAND_PCC_WAIT_FEATURE_DISABLE;	/* ����ȴ�ʱ��ʹ��, ��ֹ */
		hNand.Init.MemoryDataWidth = FMC_NAND_PCC_MEM_BUS_WIDTH_8;	/* ���ݿ�� 8bit */
		hNand.Init.EccComputation = FMC_NAND_ECC_DISABLE;			/* ECC������;������� - ��ֹ */
		hNand.Init.ECCPageSize = FMC_NAND_ECC_PAGE_SIZE_2048BYTE;	/* ECC ҳ���С */
		hNand.Init.TCLRSetupTime = 0x03;							/* CLE�ͺ�RE��֮����ӳ٣�HCLK������ */	
		hNand.Init.TARSetupTime = 0x03;                             /* ALE�ͺ�RE��֮����ӳ٣�HCLK������ */
   
		/* 2-5-3-1 V6 OK   4-10-6-2 �쳣 */
		CommonSpaceTiming.SetupTime = 3;
		CommonSpaceTiming.WaitSetupTime = 5;
		CommonSpaceTiming.HoldSetupTime = 3;
		CommonSpaceTiming.HiZSetupTime = 3;
    
		AttributeSpaceTiming.SetupTime = 3;
		AttributeSpaceTiming.WaitSetupTime = 5;
		AttributeSpaceTiming.HoldSetupTime = 3;
		AttributeSpaceTiming.HiZSetupTime = 3;

		HAL_NAND_Init(&hNand, &CommonSpaceTiming, &AttributeSpaceTiming); 
	}
}

/*
*********************************************************************************************************
*	�� �� ��: FMC_NAND_Reset
*	����˵��: ��λNAND Flash
*	��    ��:  ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static uint8_t FMC_NAND_Reset(void)
{
	NAND_CMD_AREA = NAND_CMD_RESET;

	/* ������״̬ */
	if (FMC_NAND_GetStatus() == NAND_READY)
	{
		return NAND_OK;
	}

	return NAND_FAIL;
}

/*
*********************************************************************************************************
*	�� �� ��: FSMC_NAND_ReadStatus
*	����˵��: ʹ��Read statuc �����NAND Flash�ڲ�״̬
*	��    ��:  - Address: �������Ŀ��������ַ
*	�� �� ֵ: NAND����״̬�������¼���ֵ��
*             - NAND_BUSY: �ڲ���æ
*             - NAND_READY: �ڲ����У����Խ����²�����
*             - NAND_ERROR: ��ǰ������ִ��ʧ��
*********************************************************************************************************
*/
static uint32_t FMC_NAND_ReadStatus(void)
{
	uint8_t ucData;
	uint8_t ucStatus = NAND_BUSY;

	/* ��״̬���� */
	NAND_CMD_AREA = NAND_CMD_STATUS;
	ucData = *(__IO uint8_t *)(Bank_NAND_ADDR);

	if((ucData & NAND_ERROR) == NAND_ERROR)
	{
		ucStatus = NAND_ERROR;
	}
	else if((ucData & NAND_READY) == NAND_READY)
	{
		ucStatus = NAND_READY;
	}
	else
	{
		ucStatus = NAND_BUSY;
	}

	return (ucStatus);
}

/*
*********************************************************************************************************
*	�� �� ��: FSMC_NAND_GetStatus
*	����˵��: ��ȡNAND Flash����״̬
*	��    ��:  - Address: �������Ŀ��������ַ
*	�� �� ֵ: NAND����״̬�������¼���ֵ��
*             - NAND_TIMEOUT_ERROR  : ��ʱ����
*             - NAND_READY          : �����ɹ�
*             - NAND_ERROR: ��ǰ������ִ��ʧ��
*********************************************************************************************************
*/
static uint32_t FMC_NAND_GetStatus(void)
{
	uint32_t ulTimeout = 0x10000;
	uint32_t ucStatus = NAND_READY;

	ucStatus = FMC_NAND_ReadStatus();

	/* �ȴ�NAND������������ʱ����˳� */
	while ((ucStatus != NAND_READY) &&( ulTimeout != 0x00))
	{
		ucStatus = FMC_NAND_ReadStatus();
		if(ucStatus == NAND_ERROR)
		{
			/* ���ز���״̬ */
			return (ucStatus);
		}
		ulTimeout--;
	}

	if(ulTimeout == 0x00)
	{
		ucStatus =  NAND_TIMEOUT_ERROR;
	}

	/* ���ز���״̬ */
	return (ucStatus);
}

/*
*********************************************************************************************************
*	�� �� ��: NAND_ReadID
*	����˵��: ��NAND Flash��ID��ID�洢���β�ָ���Ľṹ������С�
*	��    ��:  ��
*	�� �� ֵ: 32bit��NAND Flash ID
*********************************************************************************************************
*/
uint32_t NAND_ReadID(void)
{
	uint32_t data = 0;

	/* �������� Command to the command area */
	NAND_CMD_AREA = 0x90;
	NAND_ADDR_AREA = 0x00;

	/* ˳���ȡNAND Flash��ID */
	data = *(__IO uint32_t *)(Bank_NAND_ADDR | DATA_AREA);
	data =  ((data << 24) & 0xFF000000) |
			((data << 8 ) & 0x00FF0000) |
			((data >> 8 ) & 0x0000FF00) |
			((data >> 24) & 0x000000FF) ;
	return data;
}

/*
*********************************************************************************************************
*	�� �� ��: FMC_NAND_EraseBlock
*	����˵��: ����NAND Flashһ���飨block��
*	��    ��:  - _ulBlockNo: ��ţ���ΧΪ��0 - 1023,   0-4095
*	�� �� ֵ: NAND����״̬�������¼���ֵ��
*             - NAND_TIMEOUT_ERROR  : ��ʱ����
*             - NAND_READY          : �����ɹ�
*********************************************************************************************************
*/
static uint32_t FMC_NAND_EraseBlock(uint32_t _ulBlockNo)
{
	uint8_t ucStatus;
	
	/* HY27UF081G2A  (128MB)
				  Bit7 Bit6 Bit5 Bit4 Bit3 Bit2 Bit1 Bit0
		��1�ֽڣ� A7   A6   A5   A4   A3   A2   A1   A0		(_usPageAddr ��bit7 - bit0)
		��2�ֽڣ� 0    0    0    0    A11  A10  A9   A8		(_usPageAddr ��bit11 - bit8, ��4bit������0)
		��3�ֽڣ� A19  A18  A17  A16  A15  A14  A13  A12    A18�����ǿ��
		��4�ֽڣ� A27  A26  A25  A24  A23  A22  A21  A20

		H27U4G8F2DTR (512MB)
				  Bit7 Bit6 Bit5 Bit4 Bit3 Bit2 Bit1 Bit0
		��1�ֽڣ� A7   A6   A5   A4   A3   A2   A1   A0		(_usPageAddr ��bit7 - bit0)
		��2�ֽڣ� 0    0    0    0    A11  A10  A9   A8		(_usPageAddr ��bit11 - bit8, ��4bit������0)
		��3�ֽڣ� A19  A18  A17  A16  A15  A14  A13  A12    A18�����ǿ��
		��4�ֽڣ� A27  A26  A25  A24  A23  A22  A21  A20
		��5�ֽڣ� A28  A29  A30  A31  0    0    0    0
	*/

	/* ���Ͳ������� */
	NAND_CMD_AREA = NAND_CMD_ERASE0;

	_ulBlockNo <<= 6;	/* ���ת��Ϊҳ��� */

	#if NAND_ADDR_5 == 0	/* 128MB�� */
		NAND_ADDR_AREA = _ulBlockNo;
		NAND_ADDR_AREA = _ulBlockNo >> 8;
	#else		/* 512MB�� */
		NAND_ADDR_AREA = _ulBlockNo;
		NAND_ADDR_AREA = _ulBlockNo >> 8;
		NAND_ADDR_AREA = _ulBlockNo >> 16;
	#endif

	NAND_CMD_AREA = NAND_CMD_ERASE1;

	/* ����״̬ */
	ucStatus = FMC_NAND_GetStatus();
	
	return (ucStatus);
}

/*
*********************************************************************************************************
*	�� �� ��: NAND_Format
*	����˵��: NAND Flash��ʽ�����������е����ݣ��ؽ�LUT
*	��    ��:  ��
*	�� �� ֵ: NAND_OK : �ɹ��� NAND_Fail ��ʧ�ܣ�һ���ǻ����������ർ�£�
*********************************************************************************************************
*/
uint8_t NAND_Format(void)
{
	uint16_t i;

	for (i = 0; i < NAND_BLOCK_COUNT; i++)
	{
		FMC_NAND_EraseBlock(i);
	}

	return NAND_OK;
}

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
