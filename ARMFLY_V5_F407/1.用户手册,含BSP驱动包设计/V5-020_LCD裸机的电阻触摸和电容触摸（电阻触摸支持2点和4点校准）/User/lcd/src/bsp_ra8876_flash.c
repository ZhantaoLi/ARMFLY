/*
*********************************************************************************************************
*
*	ģ������ : RA8876оƬ��ҵĴ���Flash����ģ��
*	�ļ����� : bsp_ra8876_flash.c
*	��    �� : V1.0
*	˵    �� : ����RA8875��ҵĴ���Flash ���ֿ�оƬ��ͼ��оƬ����֧�� SST25VF016B��MX25L1606E ��
*			   W25Q64BVSSIG�� ͨ��TFT��ʾ�ӿ���SPI���ߺ�PWM���߿���7�������ϵĴ���Flash��
*				����ע�� RA8875����֧����Ҵ���Flash��д�������������Ӷ���ĵ��ӿ��ص�·����ʵ�֡�
*	�޸ļ�¼ :
*		�汾��  ����       ����    ˵��
*		V1.0    2012-06-25 armfly  �����װ�
*
*	Copyright (C), 2015-2020, ���������� www.armfly.com
*
*********************************************************************************************************
*/
#include "bsp.h"

#define CMD_AAI       0xAD  	/* AAI �������ָ��(FOR SST25VF016B) */
#define CMD_DISWR	  0x04		/* ��ֹд, �˳�AAI״̬ */
#define CMD_EWRSR	  0x50		/* ����д״̬�Ĵ��������� */
#define CMD_WRSR      0x01  	/* д״̬�Ĵ������� */
#define CMD_WREN      0x06		/* дʹ������ */
#define CMD_READ      0x03  	/* ������������ */
#define CMD_RDSR      0x05		/* ��״̬�Ĵ������� */
#define CMD_RDID      0x9F		/* ������ID���� */
#define CMD_SE        0x20		/* ������������ */
#define CMD_BE        0xC7		/* ������������ */
#define DUMMY_BYTE    0xA5		/* ���������Ϊ����ֵ�����ڶ����� */

#define WIP_FLAG      0x01		/* ״̬�Ĵ����е����ڱ�̱�־��WIP) */

/*
	PWM�����õ�ѡ��
	PWM = 1  ���ģʽ֧��STM32��дRA8875��ҵĴ���Flash
	PWM = 0 ������������ģʽ����RA8875 DMA��ȡ��ҵĴ���Flash
*/
#define	RCC_CS			(RCC_AHB1Periph_GPIOI | RCC_AHB1Periph_GPIOF)

/* Ƭѡ1  */
#define W25_CS1_GPIO	GPIOI
#define W25_CS1_PIN		GPIO_Pin_10

/* Ƭѡ2 */
#define PWM_CS2_GPIO	GPIOF
#define PWM_CS2_PIN		GPIO_Pin_6

/* ����Ƭѡ1 */
#define W25_CS1_0()     W25_CS1_GPIO->BSRRH = W25_CS1_PIN
#define W25_CS1_1()     W25_CS1_GPIO->BSRRL = W25_CS1_PIN

/* ����Ƭѡ2 */
#define PWM_CS2_0()     PWM_CS2_GPIO->BSRRH = PWM_CS2_PIN
#define PWM_CS2_1()     PWM_CS2_GPIO->BSRRL = PWM_CS2_PIN

static void RA8876_W25_SetCS(uint8_t _Value);
static uint32_t RA8876_w25_ReadID(void);
static void RA8876_w25_WaitForWriteEnd(void);
static void RA8876_w25_WriteStatus(uint8_t _ucValue);
static void RA8876_w25_ConfigGPIO(void);
static void RA8876_w25_WriteEnable(void);
static void RA8876_w25_ReadInfo(void);

uint8_t s_Ra8876CS = 0;			/* RA8876Ƭѡ 1��ʾѡ�е�һƬspi flash�� 2��ʾѡ�еڶ�Ƭspi flash */

/*
*********************************************************************************************************
*	�� �� ��: bsp_InitRA8876Flash
*	����˵��: ��ʼ������FlashӲ���ӿڣ�����STM32��SPIʱ�ӡ�GPIO)
*	��    ��:  ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void bsp_InitRA8876Flash(void)
{
	RA8876_w25_ConfigGPIO();	/* 1.����Ƭѡ1��2 */
	/* ����SPIӲ���������ڷ��ʴ���Flash */
	bsp_CfgSPIForW25();			/* 2.����SPI���Ź��� */

	/* ʶ����FLASH�ͺ� */
	RA8876_w25_CtrlByMCU();		/* (������ִ��RA8876_w25_CtrlByMCU()�л�SPI����Ȩ)  */
	bsp_DelayMS(10);			/* 2016-04-24 ���ֲ�����ʾģ�鲻�ӳ٣����´���Flash IDʶ����� */
	s_Ra8876CS = 1;				/* Ĭ��Ϊ��һƬspi flash */
	RA8876_w25_ReadInfo();		/* �Զ�ʶ��оƬ�ͺ�,RA8876����Ƭ����flash ����Ƭѡ������ */
	RA8876_w25_CtrlByRA8876();	/* ��flash����Ȩ����RA8876 */
}

/*
*********************************************************************************************************
*	�� �� ��: RA8876_w25_ConfigGPIO
*	����˵��: ����GPIO�� ������ SCK  MOSI  MISO �����SPI���ߡ�
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void RA8876_w25_ConfigGPIO(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* ʹ��GPIO ʱ�� */
	RCC_AHB1PeriphClockCmd(RCC_CS, ENABLE);

	/* ����Ƭѡ1����Ϊ�������ģʽ */
	W25_CS1_1();		/* Ƭѡ�øߣ���ѡ�� */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Pin = W25_CS1_PIN;
	GPIO_Init(W25_CS1_GPIO, &GPIO_InitStructure);
	
	/* ����Ƭѡ2����Ϊ�������ģʽ */
	PWM_CS2_1();
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Pin = PWM_CS2_PIN;
	GPIO_Init(PWM_CS2_GPIO, &GPIO_InitStructure);
}

/*
*********************************************************************************************************
*	�� �� ��: RA8876_w25_CtrlByMCU
*	����˵��: ����Flash����Ȩ����MCU ��STM32��
*	��    ��:  ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void RA8876_w25_CtrlByMCU(void)
{
	uint8_t temp;

	/* RA8876 GPIOD[3] = 0 */
	RA8876_CmdWrite(0xF6);
	temp = RA8876_DataRead();
	temp &= cClrb3;
	RA8876_DataWrite(temp);
	
	/* RA8876 �� SPI ���� */
	Disable_SFlash_SPI();
}

/*
*********************************************************************************************************
*	�� �� ��: w25_CtrlByRA8875
*	����˵��: ����Flash����Ȩ����RA8875
*	��    ��:  ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void RA8876_w25_CtrlByRA8876(void)
{
	uint8_t temp;

	/* RA8876 GPIOD[3] = 0 */
	RA8876_CmdWrite(0xF6);
	temp = RA8876_DataRead();
	temp |= cSetb3;
	RA8876_DataWrite(temp);
	
	/* RA8876 ���� SPI ���� */
	Enable_SFlash_SPI();
}


/*
*********************************************************************************************************
*	�� �� ��: RA8876_W25_CS_1
*	����˵��: RA8876��ƬFlash����ѡ��
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void RA8876_W25_SetCS(uint8_t _Value)
{
	if (_Value == 0)			/* ѡ������һ��Ƭѡ */
	{
		if (s_Ra8876CS == 1)			/* ѡ�е�һƬflash */
		{
			W25_CS1_0();
			PWM_CS2_1();	
		}
		else							/* ѡ�еڶ�Ƭflash */
		{
			W25_CS1_1();
			PWM_CS2_0();	
		}
	}
	else if (_Value == 1)		/* Ƭѡȫ����1 */
	{
		W25_CS1_1();
		PWM_CS2_1();	
	}
}

/*
*********************************************************************************************************
*	�� �� ��: RA8876_w25_SelectChip
*	����˵��: ѡ�񼴽�������оƬ
*	��    ��: _idex = FONT_CHIP ��ʾ�ֿ�оƬ;  idex = BMP_CHIP ��ʾͼ��оƬ
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void RA8876_w25_SelectChip(uint8_t _idex)
{
/* 	
	д�ֿ⣬Ƭѡѡ�� ����flash1
	дͼ�⣬Ƭѡѡ�� ����flash2
*/	
	if (_idex == FONT_CHIP)
	{
		/* ����Ϊ��1��spi flash */
		s_Ra8876CS = 1;
	}
	else	/* BMPͼƬоƬ */
	{
		/* ����Ϊ��2��spi flash */	
		s_Ra8876CS = 2;
	}

	RA8876_w25_ReadInfo();		/* �Զ�ʶ��оƬ�ͺ� */
	
	RA8876_W25_SetCS(0);				/* �����ʽ��ʹ�ܴ���FlashƬѡ */
	bsp_spiWrite1(CMD_DISWR);			/* ���ͽ�ֹд�������,��ʹ�����д���� */
	RA8876_W25_SetCS(1);					/* �����ʽ�����ܴ���FlashƬѡ */

	RA8876_w25_WaitForWriteEnd();		/* �ȴ�����Flash�ڲ�������� */

	RA8876_w25_WriteStatus(0);			/* �������BLOCK��д���� */
}

/*
*********************************************************************************************************
*	�� �� ��: RA8876_w25_ReadInfo
*	����˵��: ��ȡ����ID,�������������
*	��    ��:  ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void RA8876_w25_ReadInfo(void)
{
	/* �Զ�ʶ����Flash�ͺ� */
	{
		g_tW25.ChipID = RA8876_w25_ReadID();	/* оƬID */

		switch (g_tW25.ChipID)
		{
			case SST25VF016B:
				g_tW25.TotalSize = 2 * 1024 * 1024;	/* ������ = 2M */
				g_tW25.PageSize = 4 * 1024;			/* ҳ���С = 4K */
				break;

			case MX25L1606E:
				g_tW25.TotalSize = 2 * 1024 * 1024;	/* ������ = 2M */
				g_tW25.PageSize = 4 * 1024;			/* ҳ���С = 4K */
				break;

			case W25Q64BV:
				g_tW25.TotalSize = 8 * 1024 * 1024;	/* ������ = 8M */
				g_tW25.PageSize = 4 * 1024;			/* ҳ���С = 4K */
				break;

			case W25Q128:
				g_tW25.TotalSize = 16 * 1024 * 1024;	/* ������ = 16M */
				g_tW25.PageSize = 4 * 1024;			/* ҳ���С = 4K */
				break;

			default:		/* ��ͨ�ֿⲻ֧��ID��ȡ */
				g_tW25.TotalSize = 2 * 1024 * 1024;
				g_tW25.PageSize = 4 * 1024;
				break;
		}
	}
}

/*
*********************************************************************************************************
*	�� �� ��: w25_WriteEnable
*	����˵��: ����������дʹ������
*	��    ��:  ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void RA8876_w25_WriteEnable(void)
{
	RA8876_W25_SetCS(0);									/* ʹ��Ƭѡ */
	bsp_spiWrite1(CMD_WREN);								/* �������� */
	RA8876_W25_SetCS(1);									/* ����Ƭѡ */
}

/*
*********************************************************************************************************
*	�� �� ��: RA8876_w25_ReadID
*	����˵��: ��ȡ����ID
*	��    ��:  ��
*	�� �� ֵ: 32bit������ID (���8bit��0����ЧIDλ��Ϊ24bit��
*********************************************************************************************************
*/
static uint32_t RA8876_w25_ReadID(void)
{
	uint32_t uiID;
	uint8_t id1, id2, id3;

	RA8876_W25_SetCS(0);						/* ѡ����Ӧ��spi flash */
	bsp_spiWrite1(CMD_RDID);				/* ���Ͷ�ID���� */
	id1 = bsp_spiRead1();					/* ��ID�ĵ�1���ֽ� */
	id2 = bsp_spiRead1();					/* ��ID�ĵ�2���ֽ� */
	id3 = bsp_spiRead1();					/* ��ID�ĵ�3���ֽ� */
	RA8876_W25_SetCS(1);						/* ѡ����Ӧ��spi flash */
	
	uiID = ((uint32_t)id1 << 16) | ((uint32_t)id2 << 8) | id3;

	return uiID;
}

/*
*********************************************************************************************************
*	�� �� ��: RA8876_w25_WaitForWriteEnd
*	����˵��: ����ѭ����ѯ�ķ�ʽ�ȴ������ڲ�д�������
*	��    ��:  ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void RA8876_w25_WaitForWriteEnd()
{
	RA8876_W25_SetCS(0);								/* ʹ��Ƭѡ_cs */
	bsp_spiWrite1(CMD_RDSR);						/* ������� ��״̬�Ĵ��� */
	while((bsp_spiRead1() & WIP_FLAG) == SET);		/* �ж�״̬�Ĵ�����æ��־λ */
	RA8876_W25_SetCS(1);									/* ����Ƭѡ */
}

/*
*********************************************************************************************************
*	�� �� ��: RA8876_w25_WriteStatus
*	����˵��: д״̬�Ĵ���
*	��    ��:  _ucValue : ״̬�Ĵ�����ֵ
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void RA8876_w25_WriteStatus(uint8_t _ucValue)
{

	if (g_tW25.ChipID == SST25VF016B)
	{
		/* ��1������ʹ��д״̬�Ĵ��� */
		RA8876_W25_SetCS(0);									/* ʹ��Ƭѡ_cs */
		bsp_spiWrite1(CMD_EWRSR);							/* ������� ����д״̬�Ĵ��� */
		RA8876_W25_SetCS(1);										/* ����Ƭѡ */

		/* ��2������д״̬�Ĵ��� */
		RA8876_W25_SetCS(0);								/* ʹ��Ƭѡ */
		bsp_spiWrite1(CMD_WRSR);							/* ������� д״̬�Ĵ��� */
		bsp_spiWrite1(_ucValue);							/* �������ݣ�״̬�Ĵ�����ֵ */
		RA8876_W25_SetCS(1);											/* ����Ƭѡ */
	}
	else
	{
		RA8876_W25_SetCS(0);										/* ʹ��Ƭѡ */
		bsp_spiWrite1(CMD_WRSR);							/* ������� д״̬�Ĵ��� */
		bsp_spiWrite1(_ucValue);							/* �������ݣ�״̬�Ĵ�����ֵ */
		RA8876_W25_SetCS(1);									/* ����Ƭѡ */
	}
}

/*
*********************************************************************************************************
*	�� �� ��: w25_EraseSector
*	����˵��: ����ָ��������
*	��    ��:  _uiSectorAddr : ������ַ
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void RA8876_w25_EraseSector(uint32_t _uiSectorAddr)
{
	RA8876_w25_WriteEnable();								/* ����дʹ������ */

	/* ������������ */
	RA8876_W25_SetCS(0);									/* ʹ��Ƭѡ */
	bsp_spiWrite1(CMD_SE);								/* ���Ͳ������� */
	bsp_spiWrite1((_uiSectorAddr & 0xFF0000) >> 16);	/* ����������ַ�ĸ�8bit */
	bsp_spiWrite1((_uiSectorAddr & 0xFF00) >> 8);		/* ����������ַ�м�8bit */
	bsp_spiWrite1(_uiSectorAddr & 0xFF);				/* ����������ַ��8bit */
	RA8876_W25_SetCS(1);									/* ����Ƭѡ */

	RA8876_w25_WaitForWriteEnd();							/* �ȴ�����Flash�ڲ�д������� */
}

/*
*********************************************************************************************************
*	�� �� ��: w25_EraseChip
*	����˵��: ��������оƬ
*	��    ��:  ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void RA8876_w25_EraseChip(void)
{
	RA8876_w25_WriteEnable();								/* ����дʹ������ */

	/* ������������ */
	RA8876_W25_SetCS(0);									/* ʹ��Ƭѡ */
	bsp_spiWrite1(CMD_BE);							/* ������Ƭ�������� */
	RA8876_W25_SetCS(1);									/* ����Ƭѡ */

	RA8876_w25_WaitForWriteEnd();							/* �ȴ�����Flash�ڲ�д������� */
}

/*
*********************************************************************************************************
*	�� �� ��: RA8876_w25_WritePage
*	����˵��: ��һ��page��д�������ֽڡ��ֽڸ������ܳ���ҳ���С��4K)
*	��    ��:  	_pBuf : ����Դ��������
*				_uiWriteAddr ��Ŀ�������׵�ַ
*				_usSize �����ݸ��������ܳ���ҳ���С
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void RA8876_w25_WritePage(uint8_t * _pBuf, uint32_t _uiWriteAddr, uint16_t _usSize)
{
	uint32_t i, j;

	if (g_tW25.ChipID == SST25VF016B)
	{
		/* AAIָ��Ҫ��������ݸ�����ż�� */
		if ((_usSize < 2) && (_usSize % 2))
		{
			return ;
		}

		RA8876_w25_WriteEnable();								/* ����дʹ������ */

		RA8876_W25_SetCS(0);									/* ʹ��Ƭѡ */
		bsp_spiWrite1(CMD_AAI);							/* ����AAI����(��ַ�Զ����ӱ��) */
		bsp_spiWrite1((_uiWriteAddr & 0xFF0000) >> 16);	/* ����������ַ�ĸ�8bit */
		bsp_spiWrite1((_uiWriteAddr & 0xFF00) >> 8);		/* ����������ַ�м�8bit */
		bsp_spiWrite1(_uiWriteAddr & 0xFF);				/* ����������ַ��8bit */
		bsp_spiWrite1(*_pBuf++);							/* ���͵�1������ */
		bsp_spiWrite1(*_pBuf++);							/* ���͵�2������ */
		RA8876_W25_SetCS(1);									/* ����Ƭѡ */

		RA8876_w25_WaitForWriteEnd();							/* �ȴ�����Flash�ڲ�д������� */

		_usSize -= 2;									/* ����ʣ���ֽ��� */

		for (i = 0; i < _usSize / 2; i++)
		{
			RA8876_W25_SetCS(0);								/* ʹ��Ƭѡ */
			bsp_spiWrite1(CMD_AAI);						/* ����AAI����(��ַ�Զ����ӱ��) */
			bsp_spiWrite1(*_pBuf++);						/* �������� */
			bsp_spiWrite1(*_pBuf++);						/* �������� */
			RA8876_W25_SetCS(1);								/* ����Ƭѡ */
			RA8876_w25_WaitForWriteEnd();						/* �ȴ�����Flash�ڲ�д������� */
		}

		/* ����д����״̬ */
		RA8876_W25_SetCS(0);
		bsp_spiWrite1(CMD_DISWR);
		RA8876_W25_SetCS(1);

		RA8876_w25_WaitForWriteEnd();							/* �ȴ�����Flash�ڲ�д������� */
	}
	else	/* for MX25L1606E �� W25Q64BV */
	{
		for (j = 0; j < _usSize / 256; j++)
		{
			RA8876_w25_WriteEnable();								/* ����дʹ������ */

			RA8876_W25_SetCS(0);									/* ʹ��Ƭѡ */
			bsp_spiWrite1(0x02);								/* ����AAI����(��ַ�Զ����ӱ��) */
			bsp_spiWrite1((_uiWriteAddr & 0xFF0000) >> 16);	/* ����������ַ�ĸ�8bit */
			bsp_spiWrite1((_uiWriteAddr & 0xFF00) >> 8);		/* ����������ַ�м�8bit */
			bsp_spiWrite1(_uiWriteAddr & 0xFF);				/* ����������ַ��8bit */

			for (i = 0; i < 256; i++)
			{
				bsp_spiWrite1(*_pBuf++);					/* �������� */
			}

			RA8876_W25_SetCS(1);								/* ��ֹƬѡ */

			RA8876_w25_WaitForWriteEnd();						/* �ȴ�����Flash�ڲ�д������� */

			_uiWriteAddr += 256;
		}

		/* ����д����״̬ */
		RA8876_W25_SetCS(0);
		bsp_spiWrite1(CMD_DISWR);
		RA8876_W25_SetCS(1);

		RA8876_w25_WaitForWriteEnd();							/* �ȴ�����Flash�ڲ�д������� */
	}
}

/*
*********************************************************************************************************
*	�� �� ��: RA8876_w25_ReadBuffer
*	����˵��: ������ȡ�����ֽڡ��ֽڸ������ܳ���оƬ������
*	��    ��:  	_pBuf : ����Դ��������
*				_uiReadAddr ���׵�ַ
*				_usSize �����ݸ���, ���Դ���PAGE_SIZE,���ǲ��ܳ���оƬ������
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void RA8876_w25_ReadBuffer(uint8_t * _pBuf, uint32_t _uiReadAddr, uint32_t _uiSize)
{
	/* �����ȡ�����ݳ���Ϊ0���߳�������Flash��ַ�ռ䣬��ֱ�ӷ��� */
	if ((_uiSize == 0) ||(_uiReadAddr + _uiSize) > g_tW25.TotalSize)
	{
		return;
	}

	/* ������������ */
	RA8876_W25_SetCS(0);									/* ʹ��Ƭѡ */
	bsp_spiWrite1(CMD_READ);							/* ���Ͷ����� */
	bsp_spiWrite1((_uiReadAddr & 0xFF0000) >> 16);	/* ����������ַ�ĸ�8bit */
	bsp_spiWrite1((_uiReadAddr & 0xFF00) >> 8);		/* ����������ַ�м�8bit */
	bsp_spiWrite1(_uiReadAddr & 0xFF);				/* ����������ַ��8bit */
	while (_uiSize--)
	{
		*_pBuf++ = bsp_spiRead1();			/* ��һ���ֽڲ��洢��pBuf�������ָ���Լ�1 */
	}
	RA8876_W25_SetCS(1);									/* ����Ƭѡ */
}

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
