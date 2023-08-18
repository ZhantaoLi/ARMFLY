/*
*********************************************************************************************************
*
*	ģ������ : TFTҺ����ʾ���ײ�Ӳ�������ӿ�
*	�ļ����� : bsp_tft_port.c
*	��    �� : V1.1
*	˵    �� : �ײ�Ӳ�����������漰�ؼ����ơ�
*	�޸ļ�¼ :
*		�汾��  ����        ����    ˵��
*		v1.0    2015-08-21 armfly  ����Ӳ��������صĺ����� bsp_tft_lcd.c �Ƶ����ļ�
*		V1.1	2016-04-23 armfly  
*
*	Copyright (C), 2015-2020, ���������� www.armfly.com
*
*********************************************************************************************************
*/

#include "bsp.h"

/* ����3����������Ҫ����ʹ����ͬʱ֧�ֲ�ͬ���� */
uint16_t g_ChipID = IC_4001;		/* ����оƬID */
uint16_t g_LcdHeight = 240;			/* ��ʾ���ֱ���-�߶� */
uint16_t g_LcdWidth = 400;			/* ��ʾ���ֱ���-��� */
uint8_t s_ucBright;					/* �������Ȳ��� */
uint8_t g_LcdDirection;				/* ��ʾ����.0��1��2��3 */

static void LCD_CtrlLinesConfig(void);
static void LCD_FSMCConfig(void);
static void LCD_HardReset(void);

void SOFT1_DrawCircle(uint16_t x0, uint16_t y0, uint16_t r, uint16_t _usColor);
void SOFT_DrawQuterCircle(uint16_t _usX, uint16_t _usY, uint16_t _usRadius, uint16_t _usColor, uint8_t _ucMode);

LCD_DEV_T g_tLCD;

/*
*********************************************************************************************************
*	�� �� ��: LCD_InitHard
*	����˵��: ��ʼ��LCD
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void LCD_InitHard(void)
{
	uint32_t id;

	/* ����LCD���ƿ���GPIO */
	LCD_CtrlLinesConfig();

	/* ����FSMC�ӿڣ��������� */
	LCD_FSMCConfig();

	LCD_HardReset();	/* Ӳ����λ ��STM32-V5 ���裩���������GPIO����LCD��λ�Ĳ�Ʒ */
	
	/* FSMC���ú������ӳٲ��ܷ��������豸  */
	bsp_DelayMS(200);
	
	g_RA8875_IF = RA_HARD_8080_16;	
	id = RA8875_ReadReg(0);
	
	if (id == 0x75)
	{
		g_ChipID = IC_8875;
		RA8875_InitHard();			/* ��ʼ��RA8875оƬ */
	}
	else			
	{
		g_ChipID = IC_8876;
		//RA8876_InitHard();			/* ��ʼ��RA8876оƬ */
		//bsp_InitRA8876Flash();		/* ��ʼ��RA8876 SPI flash */
	}

	
	if (g_ChipID == IC_8875)
	{
		g_tLCD.DispOn = RA8875_DispOn;
		g_tLCD.DispOff = RA8875_DispOff;
		g_tLCD.ClrScr = RA8875_ClrScr;
		g_tLCD.PutPixel = RA8875_PutPixel;
		g_tLCD.GetPixel = RA8875_GetPixel;
		g_tLCD.DrawLine = RA8875_DrawLine;
		g_tLCD.DrawRect = RA8875_DrawRect;
		g_tLCD.DrawCircle = RA8875_DrawCircle;
		g_tLCD.DrawBMP = RA8875_DrawBMP;
		g_tLCD.FillRect = RA8875_FillRect;
		g_tLCD.FillCircle = RA8875_FillCircle;	
		g_tLCD.DrawHColorLine = RA8875_DrawHColorLine;		
	}
	
	if (g_ChipID == IC_8876)
	{
		//LCD_ClrScr(CL_BLACK);	  /* ��������ʾȫ�� */
	}
	else
	{
		LCD_SetDirection(0);
		//LCD_ClrScr(CL_BLUE);	 /* ��������ʾȫ�� */
		//LCD_SetBackLight(255); /* �򿪱��⣬����Ϊȱʡ���� */
	}
}

/*
*********************************************************************************************************
*	�� �� ��: LCD_HardReset
*	����˵��: Ӳ����λ. ��Ը�λ������GPIO���ƵĲ�Ʒ��
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void LCD_HardReset(void)
{
#if 0	
	GPIO_InitTypeDef GPIO_InitStructure;

	/* ʹ�� GPIOʱ�� */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	
	/* ���ñ���GPIOΪ�������ģʽ */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_ResetBits(GPIOB, GPIO_Pin_1);
	bsp_DelayMS(20);
	GPIO_SetBits(GPIOB, GPIO_Pin_1);
#endif	
}

/*
*********************************************************************************************************
*	�� �� ��: LCD_GetChipDescribe
*	����˵��: ��ȡLCD����оƬ���������ţ�������ʾ
*	��    ��: char *_str : �������ַ�������˻�����
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void LCD_GetChipDescribe(char *_str)
{
	switch (g_ChipID)
	{
		case IC_5420:
			strcpy(_str, CHIP_STR_5420);
			break;

		case IC_4001:
			strcpy(_str, CHIP_STR_4001);
			break;

		case IC_61509:
			strcpy(_str, CHIP_STR_61509);
			break;

		case IC_8875:
			strcpy(_str, CHIP_STR_8875);
			break;

		case IC_8876:
			strcpy(_str, CHIP_STR_8875);
			break;		

		case IC_9488:
			strcpy(_str, CHIP_STR_9488);
			break;

		default:
			strcpy(_str, "Unknow");
			break;
	}
}

/*
*********************************************************************************************************
*	�� �� ��: LCD_GetHeight
*	����˵��: ��ȡLCD�ֱ���֮�߶�
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
uint16_t LCD_GetHeight(void)
{
	return g_LcdHeight;
}

/*
*********************************************************************************************************
*	�� �� ��: LCD_GetWidth
*	����˵��: ��ȡLCD�ֱ���֮���
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
uint16_t LCD_GetWidth(void)
{
	return g_LcdWidth;
}

/*
*********************************************************************************************************
*	�� �� ��: LCD_DispOn
*	����˵��: ����ʾ
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void LCD_DispOn(void)
{
	g_tLCD.DispOn();
}

/*
*********************************************************************************************************
*	�� �� ��: LCD_DispOff
*	����˵��: �ر���ʾ
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void LCD_DispOff(void)
{
	g_tLCD.DispOn();
}

/*
*********************************************************************************************************
*	�� �� ��: LCD_ClrScr
*	����˵��: �����������ɫֵ����
*	��    ��: _usColor : ����ɫ
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void LCD_ClrScr(uint16_t _usColor)
{
	g_tLCD.ClrScr(_usColor);
}

/*
*********************************************************************************************************
*	�� �� ��: LCD_PutPixel
*	����˵��: ��1������
*	��    ��:
*			_usX,_usY : ��������
*			_usColor  : ������ɫ
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void LCD_PutPixel(uint16_t _usX, uint16_t _usY, uint16_t _usColor)
{
	g_tLCD.PutPixel(_usX, _usY, _usColor);
}

/*
*********************************************************************************************************
*	�� �� ��: LCD_GetPixel
*	����˵��: ��ȡ1������
*	��    ��:
*			_usX,_usY : ��������
*			_usColor  : ������ɫ
*	�� �� ֵ: RGB��ɫֵ
*********************************************************************************************************
*/
uint16_t LCD_GetPixel(uint16_t _usX, uint16_t _usY)
{
	return g_tLCD.GetPixel(_usX, _usY);
}

/*
*********************************************************************************************************
*	�� �� ��: LCD_DrawLine
*	����˵��: ���� Bresenham �㷨����2��仭һ��ֱ�ߡ�
*	��    ��:
*			_usX1, _usY1 : ��ʼ������
*			_usX2, _usY2 : ��ֹ������
*			_usColor     : ��ɫ
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void LCD_DrawLine(uint16_t _usX1 , uint16_t _usY1 , uint16_t _usX2 , uint16_t _usY2 , uint16_t _usColor)
{
	g_tLCD.DrawLine(_usX1 , _usY1 , _usX2, _usY2 , _usColor);
}

/*
*********************************************************************************************************
*	�� �� ��: LCD_DrawRect
*	����˵��: ����ˮƽ���õľ��Ρ�
*	��    ��:
*			_usX,_usY: �������Ͻǵ�����
*			_usHeight : ���εĸ߶�
*			_usWidth  : ���εĿ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void LCD_DrawRect(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth, uint16_t _usColor)
{
	g_tLCD.DrawRect(_usX, _usY, _usHeight, _usWidth, _usColor);
}

/*
*********************************************************************************************************
*	�� �� ��: LCD_Fill_Rect
*	����˵��: ��һ����ɫֵ���һ�����Ρ���emWin ����ͬ������ LCD_FillRect����˼����»������֡�
*	��    ��:
*			_usX,_usY: �������Ͻǵ�����
*			_usHeight : ���εĸ߶�
*			_usWidth  : ���εĿ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void LCD_Fill_Rect(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth, uint16_t _usColor)
{
	g_tLCD.FillRect(_usX, _usY, _usHeight, _usWidth, _usColor);
}

/*
*********************************************************************************************************
*	�� �� ��: LCD_DrawCircle
*	����˵��: ����һ��Բ���ʿ�Ϊ1������
*	��    ��:
*			_usX,_usY  : Բ�ĵ�����
*			_usRadius  : Բ�İ뾶
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void LCD_DrawCircle(uint16_t _usX, uint16_t _usY, uint16_t _usRadius, uint16_t _usColor)
{
	g_tLCD.DrawCircle(_usX, _usY, _usRadius, _usColor);
}

/*
*********************************************************************************************************
*	�� �� ��: LCD_FillCircle
*	����˵��: ���һ��Բ
*	��    ��:
*			_usX,_usY  : Բ�ĵ�����
*			_usRadius  : Բ�İ뾶
*			_usColor   : ������ɫ
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void _LCD_FillCircle(uint16_t _usX, uint16_t _usY, uint16_t _usRadius, uint16_t _usColor)
{
	g_tLCD.FillCircle(_usX, _usY, _usRadius, _usColor);
}

/*
*********************************************************************************************************
*	�� �� ��: LCD_DrawBMP
*	����˵��: ��LCD����ʾһ��BMPλͼ��λͼ����ɨ�����: �����ң����ϵ���
*	��    ��:
*			_usX, _usY : ͼƬ������
*			_usHeight  : ͼƬ�߶�
*			_usWidth   : ͼƬ���
*			_ptr       : ͼƬ����ָ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void LCD_DrawBMP(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth, uint16_t *_ptr)
{
	g_tLCD.DrawBMP(_usX, _usY, _usHeight, _usWidth, _ptr);
}

/*
*********************************************************************************************************
*	�� �� ��: LCD_CtrlLinesConfig
*	����˵��: ����LCD���ƿ��ߣ�FSMC�ܽ�����Ϊ���ù���
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
/*
	������STM32-V5��������߷���:

	PD0/FSMC_D2
	PD1/FSMC_D3
	PD4/FSMC_NOE		--- �������źţ�OE = Output Enable �� N ��ʾ����Ч
	PD5/FSMC_NWE		--- д�����źţ�WE = Output Enable �� N ��ʾ����Ч
	PD8/FSMC_D13
	PD9/FSMC_D14
	PD10/FSMC_D15
	PD13/FSMC_A18		--- ��ַ RS
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

	PI3/TP_INT			--- ����оƬ�ж� ������RA8875������RA8875������ж�)  ������δʹ��Ӳ���ж�

	---- ������ TFT LCD�ӿ������ź� ��FSMCģʽ��ʹ�ã�----
	PD3/LCD_BUSY		--- ����оƬæ       ��RA8875����RA8875оƬ��æ�ź�)
	PF6/LCD_PWM			--- LCD����PWM����  ��RA8875������˽ţ�������RA8875����)

	PI10/TP_NCS			--- ����оƬ��Ƭѡ		(RA8875������SPI�ӿڴ���оƬ��
	PB3/SPI3_SCK		--- ����оƬSPIʱ��		(RA8875������SPI�ӿڴ���оƬ��
	PB4/SPI3_MISO		--- ����оƬSPI������MISO(RA8875������SPI�ӿڴ���оƬ��
	PB5/SPI3_MOSI		--- ����оƬSPI������MOSI(RA8875������SPI�ӿڴ���оƬ��
*/
static void LCD_CtrlLinesConfig(void)
{
	GPIO_InitTypeDef gpio_init_structure;

	/* ʹ�� GPIOʱ�� */
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOE_CLK_ENABLE();
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
	                            GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_13 | GPIO_PIN_14 |
	                            GPIO_PIN_15;
	HAL_GPIO_Init(GPIOD, &gpio_init_structure);

	/* ����GPIOE */
	gpio_init_structure.Pin = GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 |
	                          GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
	HAL_GPIO_Init(GPIOE, &gpio_init_structure);

	/* ����GPIOG */
	gpio_init_structure.Pin = GPIO_PIN_12;
	HAL_GPIO_Init(GPIOG, &gpio_init_structure);
}

/*
*********************************************************************************************************
*	�� �� ��: LCD_FSMCConfig
*	����˵��: ����FSMC���ڷ���ʱ��
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void LCD_FSMCConfig(void)
{
/* 
	   TFT-LCD��OLED��AD7606����һ��FMC���ã����������������FMC�ٶ�������Ϊ׼��
	   �Ӷ���֤�������趼��������������
	*/
	SRAM_HandleTypeDef hsram = {0};
	FMC_NORSRAM_TimingTypeDef SRAM_TimingRead = {0};
	FMC_NORSRAM_TimingTypeDef SRAM_TimingWrite = {0};
		
	hsram.Instance  = FMC_NORSRAM_DEVICE;
	hsram.Extended  = FMC_NORSRAM_EXTENDED_DEVICE;
	
	/* FMCʹ�õ�HCLK����Ƶ168MHz��1��FMCʱ�����ھ���5.95ns */
	SRAM_TimingRead.AddressSetupTime       = 4;  /* 4*5.95ns����ַ����ʱ�䣬��Χ0 -15��FMCʱ�����ڸ��� */
	SRAM_TimingRead.AddressHoldTime        = 0;  /* ��ַ����ʱ�䣬����ΪģʽAʱ���ò����˲��� ��Χ1 -15��ʱ�����ڸ��� */
//	SRAM_TimingRead.DataSetupTime          = 8;  /* 6*5.95ns�����ݱ���ʱ�䣬��Χ1 -255��ʱ�����ڸ��� */
	SRAM_TimingRead.DataSetupTime          = 6;  /* 6*5.95ns�����ݱ���ʱ�䣬��Χ1 -255��ʱ�����ڸ��� */
	SRAM_TimingRead.BusTurnAroundDuration  = 1;  /* �����������ݶ�д��ʱ����*/
	SRAM_TimingRead.CLKDivision            = 0;  /* �������ò���������� */
	SRAM_TimingRead.DataLatency            = 0;  /* �������ò���������� */
	SRAM_TimingRead.AccessMode             = FSMC_ACCESS_MODE_A; /* ����ΪģʽA */
	
	SRAM_TimingWrite.AddressSetupTime       = 4;  /* 4*5.95ns����ַ����ʱ�䣬��Χ0 -15��FMCʱ�����ڸ��� */
	SRAM_TimingWrite.AddressHoldTime        = 0;  /* ��ַ����ʱ�䣬����ΪģʽAʱ���ò����˲��� ��Χ1 -15��ʱ�����ڸ��� */
	SRAM_TimingWrite.DataSetupTime          = 6;  /* 8*5.95ns�����ݱ���ʱ�䣬��Χ1 -255��ʱ�����ڸ��� */
	SRAM_TimingWrite.BusTurnAroundDuration  = 1;  /* �����������ݶ�д��ʱ����*/
	SRAM_TimingWrite.CLKDivision            = 0;  /* �������ò���������� */
	SRAM_TimingWrite.DataLatency            = 0;  /* �������ò���������� */
	SRAM_TimingWrite.AccessMode             = FSMC_ACCESS_MODE_A; /* ����ΪģʽA */

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
	if (HAL_SRAM_Init(&hsram, &SRAM_TimingRead, &SRAM_TimingWrite) != HAL_OK)
	{
		/* ��ʼ������ */
		Error_Handler(__FILE__, __LINE__);
	}	
}


/*
*********************************************************************************************************
*	�� �� ��: LCD_SetPwmBackLight
*	����˵��: ��ʼ������LCD�������GPIO,����ΪPWMģʽ��
*			���رձ���ʱ����CPU IO����Ϊ��������ģʽ���Ƽ�����Ϊ������������������͵�ƽ)����TIM3�ر� ʡ��
*	��    ��:  _bright ���ȣ�0����255������
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void LCD_SetPwmBackLight(uint8_t _bright)
{
	#if 0	/* P02 */
		bsp_SetTIMOutPWM(GPIOC, GPIO_Pin_6, TIM3, 1, 100, (_bright * 10000) /255);
	#else
		bsp_SetTIMOutPWM(GPIOF, GPIO_PIN_6, TIM10, 1, 100, (_bright * 10000) /255);
	#endif
}

/*
*********************************************************************************************************
*	�� �� ��: LCD_SetBackLight
*	����˵��: ��ʼ������LCD�������GPIO,����ΪPWMģʽ��
*			���رձ���ʱ����CPU IO����Ϊ��������ģʽ���Ƽ�����Ϊ������������������͵�ƽ)����TIM3�ر� ʡ��
*	��    ��: _bright ���ȣ�0����255������
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void LCD_SetBackLight(uint8_t _bright)
{
	s_ucBright =  _bright;	/* ���汳��ֵ */

	if (g_ChipID == IC_8875)
	{
		RA8875_SetBackLight(_bright);
	}
	else if (g_ChipID == IC_8876)
	{
		//RA8876_SetBackLight(_bright);
	}
	else
	{
		LCD_SetPwmBackLight(_bright);
	}
}

/*
*********************************************************************************************************
*	�� �� ��: LCD_GetBackLight
*	����˵��: ��ñ������Ȳ���
*	��    ��: ��
*	�� �� ֵ: �������Ȳ���
*********************************************************************************************************
*/
uint8_t LCD_GetBackLight(void)
{
	return s_ucBright;
}
/*
*********************************************************************************************************
*	�� �� ��: LCD_SetDirection
*	����˵��: ������ʾ����ʾ���򣨺��� ������
*	��    ��: ��ʾ������� 0 ��������, 1=����180�ȷ�ת, 2=����, 3=����180�ȷ�ת
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void LCD_SetDirection(uint8_t _dir)
{
	g_LcdDirection =  _dir;		/* ������ȫ�ֱ��� */

	if (g_ChipID == IC_8875)
	{
		RA8875_SetDirection(_dir);
	}
	else if (g_ChipID == IC_8876)
	{
		//RA8876_SetDirection(_dir);
	}
	else
	{
		//ILI9488_SetDirection(_dir);
	}
}

/*
*********************************************************************************************************
*	�� �� ��: SOFT_DrawLine
*	����˵��: ���� Bresenham �㷨����2��仭һ��ֱ�ߡ�
*	��    ��:
*			_usX1, _usY1 ����ʼ������
*			_usX2, _usY2 ����ֹ��Y����
*			_usColor     ����ɫ
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void SOFT_DrawLine(uint16_t _usX1 , uint16_t _usY1 , uint16_t _usX2 , uint16_t _usY2 , uint16_t _usColor)
{
	int32_t dx , dy ;
	int32_t tx , ty ;
	int32_t inc1 , inc2 ;
	int32_t d , iTag ;
	int32_t x , y ;

	/* ���� Bresenham �㷨����2��仭һ��ֱ�� */

	g_tLCD.PutPixel(_usX1 , _usY1 , _usColor);

	/* ��������غϣ���������Ķ�����*/
	if ( _usX1 == _usX2 && _usY1 == _usY2 )
	{
		return;
	}
	
	/* ���ƴ�ֱ�� */
	if (_usX1 == _usX2)
	{
		if (_usY2 > _usY1)
		{
			for (y = _usY1; y <= _usY2; y++)
			{
				g_tLCD.PutPixel(_usX1, y, _usColor);
			}
		}
		else
		{
			for (y = _usY2; y <= _usY1; y++)
			{
				g_tLCD.PutPixel(_usX1, y, _usColor);
			}			
		}
	}
	
	/* ����ˮƽ�� */
	if (_usY1 == _usY2)
	{
		if (_usX2 > _usX1)
		{
			for (x = _usX1; x <= _usX2; x++)
			{
				g_tLCD.PutPixel(x, _usY1, _usColor);
			}
		}
		else
		{
			for (x = _usX2; x <= _usX1; x++)
			{
				g_tLCD.PutPixel(x, _usY1, _usColor);
			}			
		}
	}	

	iTag = 0 ;
	/* dx = abs ( _usX2 - _usX1 ); */
	if (_usX2 >= _usX1)
	{
		dx = _usX2 - _usX1;
	}
	else
	{
		dx = _usX1 - _usX2;
	}

	/* dy = abs ( _usY2 - _usY1 ); */
	if (_usY2 >= _usY1)
	{
		dy = _usY2 - _usY1;
	}
	else
	{
		dy = _usY1 - _usY2;
	}

	if ( dx < dy )   /*���dyΪ�Ƴ������򽻻��ݺ����ꡣ*/
	{
		uint16_t temp;

		iTag = 1 ;
		temp = _usX1; _usX1 = _usY1; _usY1 = temp;
		temp = _usX2; _usX2 = _usY2; _usY2 = temp;
		temp = dx; dx = dy; dy = temp;
	}
	tx = _usX2 > _usX1 ? 1 : -1 ;    /* ȷ������1���Ǽ�1 */
	ty = _usY2 > _usY1 ? 1 : -1 ;
	x = _usX1 ;
	y = _usY1 ;
	inc1 = 2 * dy ;
	inc2 = 2 * ( dy - dx );
	d = inc1 - dx ;
	while ( x != _usX2 )     /* ѭ������ */
	{
		if ( d < 0 )
		{
			d += inc1 ;
		}
		else
		{
			y += ty ;
			d += inc2 ;
		}
		if ( iTag )
		{
			g_tLCD.PutPixel( y , x , _usColor) ;
		}
		else
		{
			g_tLCD.PutPixel( x , y , _usColor) ;
		}
		x += tx ;
	}
}

/*
*********************************************************************************************************
*	�� �� ��: SOFT_DrawCircle
*	����˵��: ����һ��Բ���ʿ�Ϊ1������, ʹ������㷨���ƣ�û������IC��Ӳ������
*	��    ��:
*			_usX,_usY  : Բ�ĵ�����
*			_usRadius  : Բ�İ뾶
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void SOFT_DrawCircle(uint16_t _usX, uint16_t _usY, uint16_t _usRadius, uint16_t _usColor)
{
	int32_t  D;			/* Decision Variable */
	uint32_t  CurX;		/* ��ǰ X ֵ */
	uint32_t  CurY;		/* ��ǰ Y ֵ */

	D = 3 - (_usRadius << 1);
	
	CurX = 0;
	CurY = _usRadius;

	while (CurX <= CurY)
	{
		g_tLCD.PutPixel(_usX + CurX, _usY + CurY, _usColor);
		g_tLCD.PutPixel(_usX + CurX, _usY - CurY, _usColor);
		g_tLCD.PutPixel(_usX - CurX, _usY + CurY, _usColor);
		g_tLCD.PutPixel(_usX - CurX, _usY - CurY, _usColor);
		g_tLCD.PutPixel(_usX + CurY, _usY + CurX, _usColor);
		g_tLCD.PutPixel(_usX + CurY, _usY - CurX, _usColor);
		g_tLCD.PutPixel(_usX - CurY, _usY + CurX, _usColor);
		g_tLCD.PutPixel(_usX - CurY, _usY - CurX, _usColor);

		if (D < 0)
		{
			D += (CurX << 2) + 6;
		}
		else
		{
			D += ((CurX - CurY) << 2) + 10;
			CurY--;
		}
		CurX++;
	}
}

/*
*********************************************************************************************************
*	�� �� ��: SOFT_DrawQuterCircle
*	����˵��: ����һ��1/4Բ���ʿ�Ϊ1������, ʹ������㷨����
*	��    ��:
*			_usX,_usY  : Բ�ĵ�����
*			_usRadius  : Բ�İ뾶
*			_ucMode    : 0 ��ʾ���Ͻ�1/4Բ 1��ʾ���Ͻ�  2��ʾ���½� 3��ʾ���½�
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void SOFT_DrawQuterCircle(uint16_t _usX, uint16_t _usY, uint16_t _usRadius, uint16_t _usColor, uint8_t _ucMode)
{
	int32_t  D;			/* Decision Variable */
	uint32_t  CurX;		/* ��ǰ X ֵ */
	uint32_t  CurY;		/* ��ǰ Y ֵ */

	D = 3 - (_usRadius << 1);
	
	CurX = 0;
	CurY = _usRadius;

	while (CurX <= CurY)
	{	
		if (_ucMode == 0)
		{
			g_tLCD.PutPixel(_usX - CurY, _usY - CurX, _usColor);   // �� -> ��
			g_tLCD.PutPixel(_usX - CurX, _usY - CurY, _usColor);   // �� -> ��
		}
		else if (_ucMode == 1)
		{
			g_tLCD.PutPixel(_usX + CurX, _usY - CurY, _usColor);	// �� -> ��
			g_tLCD.PutPixel(_usX + CurY, _usY - CurX, _usColor);	// �� -> ��	
		}
		else if (_ucMode == 2)
		{
			g_tLCD.PutPixel(_usX + CurX, _usY + CurY, _usColor);	// �� -> ��
			g_tLCD.PutPixel(_usX + CurY, _usY + CurX, _usColor);	// �� -> ��
		}
		else if (_ucMode == 3)
		{			
			g_tLCD.PutPixel(_usX - CurX, _usY + CurY, _usColor);	// �� -> ��
			g_tLCD.PutPixel(_usX - CurY, _usY + CurX, _usColor);    // �� -> ��
		}
		
		if (D < 0)
		{
			D += (CurX << 2) + 6;
		}
		else
		{
			D += ((CurX - CurY) << 2) + 10;
			CurY--;
		}
		CurX++;
	}
}

/*
*********************************************************************************************************
*	�� �� ��: SOFT_FillCircle
*	����˵��: ���һ��Բ������㷨ʵ�֡�
*	��    ��:
*			_usX,_usY  : Բ�ĵ�����
*			_usRadius  : Բ�İ뾶
*			_usColor   : ������ɫ
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void SOFT_FillCircle(uint16_t _usX, uint16_t _usY, uint16_t _usRadius, uint16_t _usColor)
{
	int32_t  D;
	uint32_t  CurX;		/* ��ǰ X ֵ */
	uint32_t  CurY;		/* ��ǰ Y ֵ */

	D = 3 - (_usRadius << 1);
	CurX = 0;
	CurY = _usRadius;

	while (CurX <= CurY)
	{			
		g_tLCD.DrawLine(_usX + CurX, _usY + CurY, _usX - CurX, _usY + CurY, _usColor);
		g_tLCD.DrawLine(_usX + CurX, _usY - CurY, _usX - CurX, _usY - CurY, _usColor);
		g_tLCD.DrawLine(_usX + CurY, _usY + CurX, _usX - CurY, _usY + CurX, _usColor);
		g_tLCD.DrawLine(_usX + CurY, _usY - CurX, _usX - CurY, _usY - CurX, _usColor);  	

		if (D < 0)
		{
			D += (CurX << 2) + 6;
		}
		else
		{
			D += ((CurX - CurY) << 2) + 10;
			CurY--;
		}
		CurX++;
	}
}

/*
*********************************************************************************************************
*	�� �� ��: SOFT_FillQuterCircle
*	����˵��: ���һ��1/4Բ������㷨ʵ�֡�
*	��    ��:
*			_usX,_usY  : Բ�ĵ�����
*			_usRadius  : Բ�İ뾶
*			_usColor   : ������ɫ
*			_ucMode    : 0 ��ʾ���Ͻ�1/4Բ 1��ʾ���Ͻ�  2��ʾ���½� 3��ʾ���½�
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void SOFT_FillQuterCircle(uint16_t _usX, uint16_t _usY, uint16_t _usRadius, uint16_t _usColor, uint8_t _ucMode)
{
	int32_t  D;
	uint32_t  CurX;		/* ��ǰ X ֵ */
	uint32_t  CurY;		/* ��ǰ Y ֵ */

	D = 3 - (_usRadius << 1);
	CurX = 0;
	CurY = _usRadius;

	while (CurX <= CurY)
	{			
		if (_ucMode == 0)
		{
			g_tLCD.DrawLine(_usX - CurY, _usY - CurX, _usX, _usY - CurX, _usColor);   // �� -> ��
			g_tLCD.DrawLine(_usX - CurX, _usY - CurY, _usX, _usY - CurY, _usColor);   // �� -> ��
		}
		else if (_ucMode == 1)
		{
			g_tLCD.DrawLine(_usX + CurX, _usY - CurY, _usX, _usY - CurY, _usColor);	// �� -> ��
			g_tLCD.DrawLine(_usX + CurY, _usY - CurX, _usX, _usY - CurX, _usColor);	// �� -> ��	
		}
		else if (_ucMode == 2)
		{
			g_tLCD.DrawLine(_usX + CurX, _usY + CurY, _usX, _usY + CurY, _usColor);	// �� -> ��
			g_tLCD.DrawLine(_usX + CurY, _usY + CurX, _usX, _usY + CurX, _usColor);	// �� -> ��
		}
		else if (_ucMode == 3)
		{			
			g_tLCD.DrawLine(_usX - CurX, _usY + CurY, _usX, _usY + CurY, _usColor);	// �� -> ��
			g_tLCD.DrawLine(_usX - CurY, _usY + CurX, _usX, _usY + CurX, _usColor);    // �� -> ��
		}		
		
		if (D < 0)
		{
			D += (CurX << 2) + 6;
		}
		else
		{
			D += ((CurX - CurY) << 2) + 10;
			CurY--;
		}
		CurX++;
	}
}

/*
*********************************************************************************************************
*	�� �� ��: SOFT_DrawRoundRect
*	����˵��: ����Բ�Ǿ����������ʿ��1����
*	��    ��:
*			_usX,_usY:�������Ͻǵ�����
*			_usHeight :���εĸ߶�
*			_usWidth  :���εĿ��
*			_usArc    :Բ�ǵĻ��뾶
*			_usColor  :��ɫ
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void SOFT_DrawRoundRect(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth, 
	uint16_t _usRadius, uint16_t _usColor)
{
	if (_usHeight < 2 *_usRadius)
	{
		_usHeight = 2 *_usRadius;
	}

	if (_usWidth < 2 *_usRadius)
	{
		_usWidth = 2 *_usRadius;
	}	
	
	SOFT_DrawQuterCircle(_usX + _usRadius, _usY + _usRadius, _usRadius, _usColor, 0);	/* ���ϽǵĻ� */
	g_tLCD.DrawLine(_usX + _usRadius, _usY, _usX + _usWidth - _usRadius - 1, _usY, _usColor);
	
	SOFT_DrawQuterCircle(_usX + _usWidth - _usRadius - 1, _usY + _usRadius, _usRadius, _usColor, 1);	/* ���ϽǵĻ� */
	g_tLCD.DrawLine(_usX + _usWidth - 1, _usY + _usRadius, _usX + _usWidth - 1, _usY + _usHeight  - _usRadius - 1, _usColor);
	
	SOFT_DrawQuterCircle(_usX + _usWidth - _usRadius - 1, _usY + _usHeight - _usRadius - 1, _usRadius, _usColor, 2);	/* ���½ǵĻ� */
	g_tLCD.DrawLine(_usX + _usRadius, _usY + _usHeight - 1, _usX + _usWidth - _usRadius - 1, _usY + _usHeight - 1, _usColor);
			
	SOFT_DrawQuterCircle(_usX + _usRadius,  _usY + _usHeight - _usRadius - 1, _usRadius, _usColor, 3);	/* ���½ǵĻ� */
	g_tLCD.DrawLine(_usX, _usY + _usRadius, _usX,  _usY + _usHeight - _usRadius - 1, _usColor);
}


/*
*********************************************************************************************************
*	�� �� ��: SOFT_FillRoundRect
*	����˵��: ���Բ�Ǿ���
*	��    ��:
*			_usX,_usY:�������Ͻǵ�����
*			_usHeight :���εĸ߶�
*			_usWidth  :���εĿ��
*			_usArc    :Բ�ǵĻ��뾶
*			_usColor  :��ɫ
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void SOFT_FillRoundRect(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth, 
	uint16_t _usRadius, uint16_t _usColor)
{
	if (_usHeight < 2 *_usRadius)
	{
		_usHeight = 2 *_usRadius;
	}

	if (_usWidth < 2 *_usRadius)
	{
		_usWidth = 2 *_usRadius;
	}	
	
	SOFT_FillQuterCircle(_usX + _usRadius, _usY + _usRadius, _usRadius, _usColor, 0);	/* ���ϽǵĻ� */

	g_tLCD.FillRect(_usX + _usRadius + 1,  _usY,  _usRadius + 1, _usWidth - 2 * _usRadius - 2, _usColor);
	
	SOFT_FillQuterCircle(_usX + _usWidth - _usRadius - 1, _usY + _usRadius, _usRadius, _usColor, 1);	/* ���ϽǵĻ� */

	g_tLCD.FillRect(_usX, _usY + _usRadius, _usHeight - 2 * _usRadius, _usWidth, _usColor);

	SOFT_FillQuterCircle(_usX + _usWidth - _usRadius - 1, _usY + _usHeight - _usRadius - 1, _usRadius, _usColor, 2);	/* ���½ǵĻ� */

	g_tLCD.FillRect(_usX + _usRadius + 1,  _usY + _usHeight - _usRadius - 1,  _usRadius + 1, _usWidth - 2 * _usRadius - 2, _usColor);	

	SOFT_FillQuterCircle(_usX + _usRadius,  _usY + _usHeight - _usRadius - 1, _usRadius, _usColor, 3);	/* ���½ǵĻ� */
}


/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
