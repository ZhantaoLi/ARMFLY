/*
*********************************************************************************************************
*
*	ģ������ : TFTҺ����ʾ������ģ��
*	�ļ����� : bsp_tft_lcd.c
*	��    �� : V4.8
*	˵    �� : 	֧��3.0�� 3.5�� 4.3�� 5.0�� 7.0����ʾģ��.
*			  	3.0���֧�ֵ�LCD�ڲ�����оƬ�ͺ���: SPFD5420A��OTM4001A��R61509V
*				3.5���LCD��������ILI9488�� 4.3���ϵ���RA8875����
*	�޸ļ�¼ :
*		�汾��  ����       ����    ˵��
*		v1.0    2011-08-21 armfly  ST�̼���汾 V3.5.0�汾��
*					a) ȡ�����ʼĴ����Ľṹ�壬ֱ�Ӷ���
*		V2.0    2011-10-16 armfly  ����R61509V������ʵ��ͼ����ʾ����
*		V2.1    2012-07-06 armfly  ����RA8875������֧��4.3����
*		V2.2    2012-07-13 armfly  �Ľ�LCD_DispStr������֧��12�����ַ�;�޸�LCD_DrawRect,�����һ����������
*		V2.3    2012-08-08 armfly  ���ײ�оƬ�Ĵ���������صĺ����ŵ��������ļ���֧��RA8875
*   	V3.0    2013-05-20 ����ͼ��ṹ; �޸�	LCD_DrawIconActive  �޸�DispStr����֧���ı�͸��
*		V3.1    2013-06-12 ���LCD_DispStr()����BUG�������Ƕ�ֿ��к��ָ�������256���������ѭ����
*		V3.2    2013-06-28 ����Label�ؼ�, ����ʾ�ַ�����֮ǰ��ʱ���Զ�������������
*		V3.3    2013-06-29 FSMC��ʼ��ʱ������ʱ��дʱ��Ͷ�ʱ��ֿ����á� LCD_FSMCConfig ������
*		V3.4    2013-07-06 ������ʾ32λ��Alphaͼ��ĺ��� LCD_DrawIcon32
*		V3.5    2013-07-24 ������ʾ32λ��AlphaͼƬ�ĺ��� LCD_DrawBmp32
*		V3.6    2013-07-30 �޸� DispEdit() ֧��12�����ֶ���
*		V3.7    2014-09-06 �޸� LCD_InitHard() ͬʱ֧�� RA8875-SPI�ӿں�8080�ӿ�
*		V3.8    2014-09-15 �������ɺ���:
*					��1�� LCD_DispStrEx() �����Զ������Զ���׵���ʾ�ַ�������
*					��2�� LCD_GetStrWidth() �����ַ��������ؿ��
*		V3.9    2014-10-18
*					(1) ���� LCD_ButtonTouchDown() LCD_ButtonTouchRelease �жϴ������겢�ػ水ť
*					(2) ����3.5��LCD����
*					(3) ���� LCD_SetDirection() ������������ʾ�����򣨺��� ������̬�л���
*		V4.0   2015-04-04 
*				(1) ��ť���༭��ؼ�����RA8875���壬��Ƕ�ֿ��RA8875�ֿ�ͳһ���롣����������� 
*				    FC_RA8875_16, FC_RA8875_24,	FC_RA8875_32
*				(2) FONT_T�ṹ���ԱFontCode�������� uint16_t �޸�Ϊ FONT_CODE_Eö�٣����ڱ��������;
*				(3) �޸� LCD_DispStrEx(), �����������������������_LCD_ReadAsciiDot(), _LCD_ReadHZDot()
*				(4) LCD_DispStr() �����򻯣�ֱ�ӵ��� LCD_DispStrEx() ʵ�֡�
*				(5) LCD_DispStrEx() ����֧�� RA8875���塣
*				(6) LCD_ButtonTouchDown() ���Ӱ�����ʾ��
*		V4.1   2015-04-18 
*				(1) ���RA885 ASCII����Ŀ�ȱ�LCD_DispStrEx() ��������֧��RA8875 ASCII�䳤��ȼ��㡣
*				(2) ��� LCD_HardReset(��������֧��LCD��λ��GPIO���ƵĲ�Ʒ��STM32-V5 ����ҪGPIO���ơ�
*		V4.2   2015-07-23
*				(1) ��Ӻ���LCD_InitButton()
*				(2) h�ļ���ʹ�ܰ�����ʾ�� #define BUTTON_BEEP()	BEEP_KeyTone();
*		V4.3   2016-01-30 
*				(1) ��LCD�ײ�Ӳ�����ʺ���������ȥ���ŵ� bsp_tft_port.c �ļ���
*				(2) ���ӽṹ�� BTN_PARAM_T�� ����������ʹ��forѭ����ʽ��ʼ����ť������Ȳ���
*				(3) ��ť�Ľṹ�壬���Ӱ�ť����Ա����
*		V4.4	2016-02-03
*				(1) ����Բ�Ǿ��οؼ���TPannel
*				(2) ���Ӻ��� LCD_InitGroupBox();
*				(3) LCD_DispStrEx(). ���֧��RA8875���屳��͸���� ���� RA8875_SetTextTransp����
*				(4) LCD_DrawGroupBox(), ֧������͸��ģʽ���������߷�2�λ���
*				(5) ���� LCD_InitLabel()����
*				(6) ���� LCD_InitEdit() ����
*				(7) ���� LCD_PannelClick(), LCD_LabelClick()����
*		V4.5	2016-04-22
*				(1) �޸� LCD_DispStrEx������֧����Ƕ24,32�����ַ��ͺ��֣���������������������֧��SPI Flash
*		V4.6	2016-04-24
*				(1) �޸� LCD_ButtonTouchRelease() �����ⴥ��̧��ʱ��ť��΢��������
*				(2)	�޸� �ؼ�������
*		V4.7    2016-07-02
*				����4��RA8876��Ч����غ���
*					����������Ч���󻬡��һ�����ˢ����
*						LCD_BeginDrawAll()
*						LCD_EndDrawAll()
*					��������ˢ����
*						LCD_BeginDrawWin()		
*						LCD_EndDrawWin()
*		V4.8	2017-06-17
*				(1) ֧�ְ�ť���ֻ�����ʾ.
*		V4.9	2018-03-10 
*				(1) ���� 62*40 ASCII����0-9
*				(2) ���� 96*40 ASCII����0-9
*				(3) �޸� _LCD_ReadHZDot()������֧���ֶ��庺��ͼ��
*
*	Copyright (C), 2015-2020, ���������� www.armfly.com
*
*********************************************************************************************************
*/

/*
	��ֲ����
	1. ����PWM����ĺ�����Ҫ���ġ�  LCD_SetPwmBackLight
	2. FSMC���õ�Ƭѡ��Ҫ����	LCD_FSMCConfig
*/
#include "fonts.h"
#include "bsp.h"

#include "math.h"

static void LCD_DispStrEx0(uint16_t _usX, uint16_t _usY, char *_ptr, FONT_T *_tFont, uint16_t _Width,
	uint8_t _Align);
static uint8_t SeachStr_a(char *_ptr);
static void _LCD_ReadSmallDot(uint8_t _code, uint8_t _fontcode, uint8_t *_pBuf);

/*
*********************************************************************************************************
*	�� �� ��: LCD_BeginDrawAll
*	����˵��: ��ʼ��Ч����������ʱ�����ڴ��ַָ���3���ڴ档
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void LCD_BeginDrawAll(void)
{
	uint16_t width;
	
	if (g_LcdDirection > 1)			/* ���� */
	{
		width = g_LcdHeight;
	}
	else							/* ���� */
	{
		width = g_LcdWidth;
	}
    
    (void)width;
	
	if (g_ChipID == IC_8876)
	{
		#if 0
		Canvas_Image_Start_address(width * 2 * 2);		/* �Ƚ�����д����3���ڴ��У�д����ڽ����ݸ��Ƶ���1���2���ڴ��� */
		g_Drawing = 1;				/* ��Ч��ͼ */
		#endif
	}
	else
	{
		;
	}
}

/*
*********************************************************************************************************
*	�� �� ��: LCD_EndDrawAll
*	����˵��: �����ڽ�����֧��3������Ч
*	��    ��: _slide ��0-2   0��ʾ����������ˢ���� 1��ʾ�󻬶� 2��ʾ�һ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void LCD_EndDrawAll(uint8_t _slide)
{
	uint16_t width, height;	
		
	
	if (g_LcdDirection > 1)			/* ���� */
	{
		width = g_LcdHeight;
		height = g_LcdWidth;
	}
	else							/* ���� */
	{
		width = g_LcdWidth;
		height = g_LcdHeight;
	}
    
    (void)width;
	(void)height;

	if (g_ChipID == IC_8876)
	{
	 #if 0
		uint16_t i;
		uint32_t SAddr, DAddr;
		
		if (_slide == 0)			/* ������ */
		{
			SAddr = (width * 2) * 2;				/* Դ��ַ����3���ڴ棩 */
			DAddr = (width * 2) * g_Interface;		/* Ŀ�ĵ�ַ */
			
			/* ����3���ڴ�����ݸ��Ƶ��������ڴ棨��1�飩 */
			BTE_Memory_Copy(SAddr, CanvasWidth, 0, 0, SAddr, CanvasWidth, 0, 0, DAddr,CanvasWidth,0,0,12, width, height);
			
			Canvas_Image_Start_address(DAddr);				/* ���ڴ�д���ַ��ԭ���ӽ�����ʼ��ַ */
		}
		else if (_slide == 1)		/* �󻬶� */
		{
			if (g_Interface == 0)					/* ��ǰ��ʾ��1���ڴ� */
			{
				SAddr = 0;							/* Դ��ַ����1���ڴ棩 */
				DAddr = (width * 2) * 1;			/* Ŀ�ĵ�ַ����2���ڴ棩 */
				
				/* ����1���ڴ�����ݸ��Ƶ���2�� */
				BTE_Memory_Copy(SAddr, CanvasWidth, 0, 0, SAddr, CanvasWidth, 0, 0, DAddr,CanvasWidth,0,0,12, width, height);
				Main_Image_Start_Address(DAddr);			/* ���Դ�ָ���2���ڴ��ַ */
				
				SAddr = (width * 2) * 2;			/* Դ��ַ����3���ڴ棩 */
				DAddr = 0;							/* Ŀ�ĵ�ַ����1���ڴ棩 */
				BTE_Memory_Copy(SAddr, CanvasWidth, 0, 0, SAddr, CanvasWidth, 0, 0, DAddr,CanvasWidth,0,0,12, width, height);
			}
			else											/* ��ǰ��ʾ��2���ڴ� */
			{
				SAddr = (width * 2) * 2;			/* Դ��ַ����3���ڴ棩 */
				DAddr = 0;							/* Ŀ�ĵ�ַ����1���ڴ棩 */
				BTE_Memory_Copy(SAddr, CanvasWidth, 0, 0, SAddr, CanvasWidth, 0, 0, DAddr,CanvasWidth,0,0,12, width, height);
			}
			
			/* ��ʼ��������1���ڴ� <- ��2���ڴ� */
			for (i = width; i > 0; i--)
			{
				Main_Image_Start_Address(i * 2);
				bsp_DelayUS(500);
			}
			
			Canvas_Image_Start_address(DAddr);				/* ���ڴ�д���ַ��ԭ����������ʼ��ַ */
			g_Interface = 0;		/* ���յ���ʾ�ڴ�Ϊ��1���ڴ� */
		}
		else if (_slide == 2)		/* �һ��� */
		{
			if (g_Interface == 0)					/* ��ǰ��ʾ��1���ڴ� */
			{
				SAddr = (width * 2) * 2;			/* Դ��ַ����3���ڴ棩 */
				DAddr = (width * 2) * 1;			/* Ŀ�ĵ�ַ����2���ڴ棩 */
				BTE_Memory_Copy(SAddr, CanvasWidth, 0, 0, SAddr, CanvasWidth, 0, 0, DAddr,CanvasWidth,0,0,12, width, height);
			}
			else									/* ��ǰ��ʾ��2���ڴ� */
			{
				SAddr = (width * 2) * 1;			/* Դ��ַ����2���ڴ棩 */
				DAddr = 0;							/* Ŀ�ĵ�ַ����1���ڴ棩 */
				BTE_Memory_Copy(SAddr, CanvasWidth, 0, 0, SAddr, CanvasWidth, 0, 0, DAddr,CanvasWidth,0,0,12, width, height);
		
				Main_Image_Start_Address(DAddr);			/* ���Դ�ָ���1���ڴ��ַ */
				
				SAddr = (width * 2) * 2;			/* Դ��ַ����3���ڴ棩 */
				DAddr = (width * 2) * 1;			/* Ŀ�ĵ�ַ����2���ڴ棩 */
				BTE_Memory_Copy(SAddr, CanvasWidth, 0, 0, SAddr, CanvasWidth, 0, 0, DAddr,CanvasWidth,0,0,12, width, height);
			}
			
			/* ��ʼ��������1���ڴ� -> ��2���ڴ� */
			for (i = 0; i <= width; i++)
			{
				Main_Image_Start_Address(i * 2);			/* �����浽�ӽ��� */
				bsp_DelayUS(500);
			}
			
			Canvas_Image_Start_address(DAddr);				/* ���ڴ�д���ַ��ԭ���ӽ�����ʼ��ַ */
			g_Interface = 1;		/* ���յ���ʾ�ڴ�Ϊ��2���ڴ� */
		}
		g_Drawing = 0; 			/* ��Ч��ͼ���� */
		#endif
	}
	else
	{
		;
	}
}

/*
*********************************************************************************************************
*	�� �� ��: LCD_BeginDrawWin
*	����˵��: �������ִ��ڡ������������Ƶ���ʾ�ڴ��
*	��    ��: _usColor : ���ڵ�ɫ
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void LCD_BeginDrawWin(uint16_t _usColor)
{
	uint16_t width, height;
	
	
	if (g_LcdDirection > 1)			/* ���� */
	{
		width = g_LcdHeight;
		height = g_LcdWidth;
	}
	else							/* ���� */
	{
		width = g_LcdWidth;
		height = g_LcdHeight;
	}
    
    
	
	if (g_ChipID == IC_8876)
	{
	#if 0
        uint32_t MemCopyAddr;
		MemCopyAddr = (width * 2) * 2;			/* �������ڴ���ʼ��ַ */
		Canvas_Image_Start_address(MemCopyAddr);			/* ���ڴ������ʼ��ַ����Ϊ�������ڴ� */
		BTE_Solid_Fill(0, CanvasWidth, width * 2, 0, _usColor, width, height); /* ����3���ڴ����� */	
		#endif
	}
	else
	{
		;
	}
    
    (void)width;
	(void)height;
}

/*
*********************************************************************************************************
*	�� �� ��: LCD_EndDrawWin
*	����˵��: �������ִ��ڽ����������������Ƶ���ʾ�ڴ��
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void LCD_EndDrawWin(uint16_t _usX, uint16_t _usY, uint16_t _Width, uint16_t _Height)
{
	uint16_t width; //height;
	uint16_t temp;   
	
	if (g_LcdDirection > 1)			/* ���� */
	{		
		temp = _usX;
		_usX = _usY;
		_usY = temp;
		
		temp = _Width;
		_Width = _Height;
		_Height = temp;

		width = g_LcdHeight;
//		height = g_LcdWidth;
	}
	else							/* ���� */
	{
		width = g_LcdWidth;
//		height = g_LcdHeight;
	}
    
	
	if (g_ChipID == IC_8876)
	{
		#if 0
        uint32_t SAddr, DAddr;
        
		SAddr = (width * 2) * 2;			/* �������ڴ���ʼ��ַ */
	
		if (g_Interface == 0)					/* ��ǰ��ʾ��1���ڴ� */
		{
			DAddr = 0;						/* Ŀ���ַ����1���ڴ���ʼ��ַ */
			/* ��Դ�ڴ�����ݸ��Ƶ�Ŀ���ڴ��� */
			BTE_Memory_Copy(SAddr, CanvasWidth,_usX,_usY, SAddr, CanvasWidth,_usX,_usY, DAddr,CanvasWidth,_usX,_usY,12, _Width, _Height);	
			Canvas_Image_Start_address(DAddr);				/* �����ڴ��ַ�лص�ԭ�������� */
		}
		else
		{
			DAddr = (width * 2) * 1;		/* Ŀ���ַ����2���ڴ���ʼ��ַ */
			/* ��Դ�ڴ�����ݸ��Ƶ�Ŀ���ڴ��� */
			BTE_Memory_Copy(SAddr, CanvasWidth,_usX,_usY, SAddr, CanvasWidth,_usX,_usY, DAddr,CanvasWidth,_usX,_usY,12, _Width, _Height);	
			Canvas_Image_Start_address(DAddr);				/* �����ڴ��ַ�лص�ԭ�������� */
		}
		#endif
	}
	else
	{
		;
	}
    
    (void)width;
}

/*
*********************************************************************************************************
*	�� �� ��: LCD_DispStr
*	����˵��: ��LCDָ�����꣨���Ͻǣ���ʾһ���ַ���
*	��    ��:
*		_usX : X����
*		_usY : Y����
*		_ptr  : �ַ���ָ��
*		_tFont : ����ṹ�壬������ɫ������ɫ(֧��͸��)��������롢���ּ��Ȳ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void LCD_DispStr(uint16_t _usX, uint16_t _usY, char *_ptr, FONT_T *_tFont)
{
	LCD_DispStrEx0(_usX, _usY, _ptr, _tFont, 0, 0);
}


/*
*********************************************************************************************************
*	�� �� ��: LCD_DispStrEx
*	����˵��: ��LCDָ�����꣨���Ͻǣ���ʾһ���ַ����� ��ǿ�ͺ�����֧����\��\�Ҷ��룬֧�ֶ��������� ֧�ֻ���
*	��    ��:
*		_usX : X����
*		_usY : Y����
*		_ptr  : �ַ���ָ��
*		_tFont : ����ṹ�壬������ɫ������ɫ(֧��͸��)��������롢���ּ��Ȳ���������ָ��RA8875�ֿ���ʾ����
*		_Width : �ַ�����ʾ����Ŀ��. 0 ��ʾ�������������򣬴�ʱ_Align��Ч
*		_Align :�ַ�������ʾ����Ķ��뷽ʽ��
*				ALIGN_LEFT = 0,
*				ALIGN_CENTER = 1,
*				ALIGN_RIGHT = 2
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void LCD_DispStrEx(uint16_t _usX, uint16_t _usY, char *_ptr, FONT_T *_tFont, uint16_t _Width,
	uint8_t _Align)
{
	uint16_t i = 0;
	char str_buf[128] = {0};
	uint16_t len;
	uint16_t x, y;
	uint8_t ch;
	
	len = 0;
	x = _usX;
	y = _usY;
	str_buf[0] = 0;

	for (i = 0; i < 1024; i++)
	{
		ch = _ptr[i];
		
		if (ch == 0)
		{
			LCD_DispStrEx0(x, y, str_buf, _tFont, _Width, _Align);
			break;
		}
		else if (ch == '\r')	/* ����ָ�����2���ַ���ʾ��ֱ��ࣨ��λ���أ� ֧���ص� */
		{
			uint8_t cap;
			
			LCD_DispStrEx0(x, y, str_buf, _tFont, _Width, _Align);
			
			len = 0;
			
			x = _usX;
			
			cap = (_ptr[i + 1] - '0') * 10 + _ptr[i + 2] - '0';		/* ��� */
			y += cap;
			i += 2;
		}
		else if (ch == '\t')	/* ����ָ�����8���ַ���ʾ X1, Y2, X2,  Y2 00 99 02 02 */
		{
			uint16_t x1, x2, y1, y2;
			
			x1 = _usX + (_ptr[i + 1] - '0') * 10 + (_ptr[i + 2] - '0');
			y1 = _usY + (_ptr[i + 3] - '0') * 10 + (_ptr[i + 4] - '0');				
			x2 = _usX + (_ptr[i + 5] - '0') * 10 + (_ptr[i + 6] - '0');
			y2 = _usY + (_ptr[i + 7] - '0') * 10 + (_ptr[i + 8] - '0');
			LCD_DrawLine(x1, y1, x2, y2, _tFont->FrontColor);
						
			i += 8;
		}
		else
		{			
			if (len < sizeof(str_buf) - 1)
			{
				str_buf[len++] = ch;
				
				str_buf[len] = 0;
			}
		}
	}
}

/*
*********************************************************************************************************
*	�� �� ��: LCD_GetFontWidth
*	����˵��: ��ȡ����Ŀ�ȣ����ص�λ)
*	��    ��:
*		_tFont : ����ṹ�壬������ɫ������ɫ(֧��͸��)��������롢���ּ��Ȳ���
*	�� �� ֵ: ����Ŀ�ȣ����ص�λ)
*********************************************************************************************************
*/
uint16_t LCD_GetFontWidth(FONT_T *_tFont)
{
	uint16_t font_width = 16;

	switch (_tFont->FontCode)
	{
		case FC_ST_12:
			font_width = 12;
			break;

		case FC_ST_16:
		case FC_RA8875_16:			
			font_width = 16;
			break;
			
		case FC_RA8875_24:			
		case FC_ST_24:
			font_width = 24;
			break;
			
		case FC_ST_32:
		case FC_RA8875_32:	
			font_width = 32;
			break;			

		case FC_ST_62X40:
			font_width = 40;
			break;	
		
		case FC_ST_96X40:
			font_width = 40;
			break;			
	}
	return font_width;
}

/*
*********************************************************************************************************
*	�� �� ��: LCD_GetFontHeight
*	����˵��: ��ȡ����ĸ߶ȣ����ص�λ)
*	��    ��:
*		_tFont : ����ṹ�壬������ɫ������ɫ(֧��͸��)��������롢���ּ��Ȳ���
*	�� �� ֵ: ����Ŀ�ȣ����ص�λ)
*********************************************************************************************************
*/
uint16_t LCD_GetFontHeight(FONT_T *_tFont)
{
	uint16_t height = 16;

	switch (_tFont->FontCode)
	{
		case FC_ST_12:
			height = 12;
			break;

		case FC_ST_16:
		case FC_RA8875_16:			
			height = 16;
			break;
			
		case FC_RA8875_24:			
		case FC_ST_24:
			height = 24;
			break;
			
		case FC_ST_32:
		case FC_RA8875_32:	
			height = 32;
			break;	
		
		case FC_ST_62X40:
			height = 62;
			break;		

		case FC_ST_96X40:
			height = 96;
			break;		
	}
	return height;
}

/*
*********************************************************************************************************
*	�� �� ��: LCD_GetStrWidth
*	����˵��: �����ַ������(���ص�λ)
*	��    ��:
*		_ptr  : �ַ���ָ��
*		_tFont : ����ṹ�壬������ɫ������ɫ(֧��͸��)��������롢���ּ��Ȳ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
uint16_t LCD_GetStrWidth(char *_ptr, FONT_T *_tFont)
{
	char *p = _ptr;
	uint16_t width = 0;
	uint8_t code1, code2;
	uint16_t font_width;
	uint16_t m;
	uint16_t address;
	uint8_t a_flag = 0;

	font_width = LCD_GetFontWidth(_tFont);

	while (*p != 0)
	{
		code1 = *p;	/* ��ȡ�ַ������ݣ� �����ݿ�����ascii���룬Ҳ���ܺ��ִ���ĸ��ֽ� */
		if (code1 < 0x80)	/* ASCII */
		{
			if (code1 == '\a')
			{
				a_flag = 1;
				p++;
				code1 = *p;
			}
			else
			{
				a_flag = 0;
			}
			
			switch(_tFont->FontCode)
			{
				case FC_RA8875_16:
					font_width = g_RA8875_Ascii16_width[code1 - 0x20];
					break;
				
				case FC_RA8875_24:
					if (a_flag == 0)
					{
						font_width = g_RA8875_Ascii24_width[code1 - 0x20];
					}
					else
					{
						{
							m = 0;
							while(1)
							{
							   address = m * (72 + 2);
							   m++;
							   if (code1 == g_Ascii24_VarWidth[address + 0])
							   {
								  font_width = g_Ascii24_VarWidth[address + 1];
								  break;
							   }
							   else if ((g_Ascii24_VarWidth[address + 0] == 0xFF) && (g_Ascii24_VarWidth[address + 1] == 0xFF))
							   {
	//							  /* �ֿ�������ϣ�δ�ҵ��������ȫFF */
	//							  memset(g_Ascii32_VarWidth, 0xFF, 128);
								  break;
							   }	   
						   }
						}
					}
					break;
				
				case FC_RA8875_32:
					if (a_flag == 0)
					{
						font_width = g_RA8875_Ascii32_width[code1 - 0x20];
					}
					else
					{
						{
							m = 0;
							while(1)
							{
							   address = m * (128 + 2);
							   m++;
							   if (code1 == g_Ascii32_VarWidth[address + 0])
							   {
								  font_width = g_Ascii32_VarWidth[address + 1];
								  break;
							   }
							   else if ((g_Ascii32_VarWidth[address + 0] == 0xFF) && (g_Ascii32_VarWidth[address + 1] == 0xFF))
							   {
	//							  /* �ֿ�������ϣ�δ�ҵ��������ȫFF */
	//							  memset(g_Ascii32_VarWidth, 0xFF, 128);
								  break;
							   }	   
						   }
						}
					}
					break;
				
				case FC_ST_12:
					font_width = 6;
					break;

				case FC_ST_16:		
					font_width = 8;
					break;
					
				case FC_ST_24:			
					font_width = 12;
					break;
					
				case FC_ST_32:
					font_width = 16;
					break;

				case FC_ST_62X40:
					//����������⴦��
					if (code1 == 0x5E)
					{
						font_width = 28;
					}
					else
					{
						font_width = 40;
					}
					break;

				case FC_ST_96X40:
					//����������⴦��
					if (code1 == 0x5E)
					{
						font_width = 28;
					}
					else
					{
						font_width = 40;
					}
					break;
									
				default:
					font_width = 8;
					break;					
			}
			
		}
		else	/* ���� */
		{
			code2 = *++p;
			if (code2 == 0)
			{
				break;
			}
			font_width = LCD_GetFontWidth(_tFont);
			
		}
		width += (font_width + _tFont->Space);
		p++;
	}

	return width;
}

/*
*********************************************************************************************************
*	�� �� ��: _LCD_ReadSmallDot
*	����˵��: ��ȡ1��С�����ַ��ĵ�������
*	��    ��:
*		_code : ASCII�ַ��ı��룬1�ֽڡ�1-128
*		_fontcode ���������
*		_pBuf : ��Ŷ������ַ���������
*	�� �� ֵ: ���ֿ��
*********************************************************************************************************
*/
static void _LCD_ReadSmallDot(uint8_t _code, uint8_t _fontcode, uint8_t *_pBuf)
{
#ifdef USE_SMALL_FONT	/* ʹ��CPU �ڲ�Flash С�ֿ� */
	const uint8_t *pAscDot;
	uint32_t font_bytes = 0;
	uint16_t m;
	uint16_t address;
	uint8_t fAllHz = 0;	/* 1��ʾ��������Ƕȫ����ASCII�ַ��� */
	
	pAscDot = 0;
	switch (_fontcode)
	{
		case FC_ST_12:		/* 12���� */
			font_bytes = 24 / 2;
			pAscDot = g_Ascii12;	
			fAllHz = 1;
			break;
		
		case FC_ST_16:
			/* ȱʡ��16���� */
			font_bytes = 32 / 2;
			pAscDot = g_Ascii16;
			fAllHz = 1;
			break;
		
		case FC_ST_24:
			font_bytes = 48;
			pAscDot = g_Ascii24;
			break;
		
		case FC_ST_32:
			font_bytes = 64;
			pAscDot = g_Ascii32;
			break;

		case FC_ST_62X40:
			font_bytes = 310;
			pAscDot = g_Ascii62x40;
			break;	
		
		case FC_ST_96X40:
			font_bytes = 480;
			pAscDot = g_Ascii96x40;
			break;

		case FC_RA8875_24:
			font_bytes = 72;
			pAscDot = g_Ascii24_VarWidth;
			fAllHz = 2;
			break;
		
		case FC_RA8875_32:
			font_bytes = 128;
			pAscDot = g_Ascii32_VarWidth;
			fAllHz = 2;
			break;
		
		default:
			return;
	}	

	if (fAllHz == 1)	/* ��Ƕȫ��ASCII�ַ����� */
	{
		/* ��CPU�ڲ�Flash�е�ascii�ַ������Ƶ�buf */
		memcpy(_pBuf, &pAscDot[_code * (font_bytes)], (font_bytes));		
	}
	else if (fAllHz == 2)
	{
		m = 0;
		while(1)
		{
		   address = m * (font_bytes + 2);
		   m++;
		   if (_code == pAscDot[address + 0])
		   {
			  address += 2;
			  memcpy(_pBuf, &pAscDot[address], font_bytes);
			  break;
		   }
		   else if ((pAscDot[address + 0] == 0xFF) && (pAscDot[address + 1] == 0xFF))
		   {
			  /* �ֿ�������ϣ�δ�ҵ��������ȫFF */
			  memset(_pBuf, 0xFF, font_bytes);
			  break;
		   }	   
	   }
	}
	else	/* ��Ƕ�����ַ�����ģ�������ֽ���ASCII�� */
	{
		m = 0;
		while(1)
		{
		   address = m * (font_bytes + 1);
		   m++;
		   if (_code == pAscDot[address + 0])
		   {
			  address += 1;
			  memcpy(_pBuf, &pAscDot[address], font_bytes);
			  break;
		   }
		   else if ((pAscDot[address + 0] == 0xFF) && (pAscDot[address + 1] == 0xFF))
		   {
			  /* �ֿ�������ϣ�δ�ҵ��������ȫFF */
			  memset(_pBuf, 0xFF, font_bytes);
			  break;
		   }	   
	   }
   }
#else	/* ��ȫ�ֿ� */
	uint32_t pAscDot;
	uint8_t font_bytes = 0;

	pAscDot = 0;
	switch (_fontcode)
	{
		case FC_ST_12:		/* 12���� */
			font_bytes = 12;
			#if 0
				pAscDot = ASC12_ADDR;	/* �ֿ�оƬ��16�����ַ����ÿ�,�ʻ�ϸ�ˣ������Ƿǵȿ����� */
			#else
				pAscDot = (uint32_t)&g_Ascii12[' ' * 12];	/* ʹ��CPU��Ƕ��16�����ַ� */
			#endif			
			break;
		
		case FC_ST_16:
			font_bytes = 16;
			#if 0
				pAscDot = ASC16_ADDR;	/* �ֿ�оƬ��16�����ַ����ÿ�,�ʻ�ϸ�ˣ������Ƿǵȿ����� */
			#else
				pAscDot = (uint32_t)&g_Ascii16[' ' * 16];	/* ʹ��CPU��Ƕ��16�����ַ� */
			#endif
			break;
		
		case FC_ST_24:
			font_bytes = 48;
			pAscDot = ASC24_ADDR;
			break;
		
		case FC_ST_32:
			font_bytes = 64;
			pAscDot = ASC32_ADDR;
			break;
		
		default:
			return;
	}			
	if (_code >=0x20 && _code <= 0x7E)
	{		
		pAscDot = ((uint32_t)_code - 0x20)*font_bytes + pAscDot;
	}

#ifdef USE_NOR_FONT		/* NOR Flashȫ�ֿ� */
	/* ��CPU�ڲ�Flash�е�ascii�ַ������Ƶ�buf */
	memcpy(_pBuf, (char *)pAscDot, font_bytes);	
#endif

#ifdef USE_SPI_FONT		/* ���� Flashȫ�ֿ� */
	if (_fontcode == FC_ST_12 || _fontcode == FC_ST_16)
	{
		memcpy(_pBuf, (char *)pAscDot, font_bytes);	
	}
	else
	{
		/* �ֿ�оƬ��12�����16�����ַ����ÿ�,�ʻ�ϸ�ˣ������Ƿǵȿ����� */
		sf_ReadBuffer(_pBuf, pAscDot, font_bytes);
	}
#endif
	
#endif
}

/*
*********************************************************************************************************
*	�� �� ��: _LCD_ReadAsciiDot
*	����˵��: ��ȡ1��ASCII�ַ��ĵ�������
*	��    ��:
*		_code : ASCII�ַ��ı��룬1�ֽڡ�1-128
*		_fontcode ���������
*		_pBuf : ��Ŷ������ַ���������
*	�� �� ֵ: ���ֿ��
*********************************************************************************************************
*/
static void _LCD_ReadAsciiDot(uint8_t _code, uint8_t _fontcode, uint8_t *_pBuf)
{
#ifdef USE_SMALL_FONT	/* ʹ��CPU �ڲ�Flash С�ֿ� */
	const uint8_t *pAscDot;
	uint32_t font_bytes = 0;
	uint16_t m;
	uint16_t address;
	uint8_t fAllHz = 0;	/* 1��ʾ��������Ƕȫ����ASCII�ַ��� */

	pAscDot = 0;
	switch (_fontcode)
	{
		case FC_ST_12:		/* 12���� */
			font_bytes = 24 / 2;
			pAscDot = g_Ascii12;	
			fAllHz = 1;
			break;
		
		case FC_ST_16:
			/* ȱʡ��16���� */
			font_bytes = 32 / 2;
			pAscDot = g_Ascii16;
			fAllHz = 1;
			break;
		
		case FC_ST_24:
			font_bytes = 48;
			pAscDot = g_Ascii24;
			break;
		
		case FC_ST_32:
			font_bytes = 64;
			pAscDot = g_Ascii32;
			break;

		case FC_ST_62X40:
			font_bytes = 310;
			pAscDot = g_Ascii62x40;
			break;	
		
		case FC_ST_96X40:
			font_bytes = 480;
			pAscDot = g_Ascii96x40;
			break;
		
		default:
			return;
	}	

	if (fAllHz == 1)	/* ��Ƕȫ��ASCII�ַ����� */
	{
		/* ��CPU�ڲ�Flash�е�ascii�ַ������Ƶ�buf */
		memcpy(_pBuf, &pAscDot[_code * (font_bytes)], (font_bytes));		
	}
	else	/* ��Ƕ�����ַ�����ģ�������ֽ���ASCII�� */
	{
		m = 0;
		while(1)
		{
		   address = m * (font_bytes + 1);
		   m++;
		   if (_code == pAscDot[address + 0])
		   {
			  address += 1;
			  memcpy(_pBuf, &pAscDot[address], font_bytes);
			  break;
		   }
		   else if ((pAscDot[address + 0] == 0xFF) && (pAscDot[address + 1] == 0xFF))
		   {
			  /* �ֿ�������ϣ�δ�ҵ��������ȫFF */
			  memset(_pBuf, 0xFF, font_bytes);
			  break;
		   }	   
	   }
   }
#else	/* ��ȫ�ֿ� */
	uint32_t pAscDot;
	uint8_t font_bytes = 0;

	pAscDot = 0;
	switch (_fontcode)
	{
		case FC_ST_12:		/* 12���� */
			font_bytes = 12;
			#if 0
				pAscDot = ASC12_ADDR;	/* �ֿ�оƬ��16�����ַ����ÿ�,�ʻ�ϸ�ˣ������Ƿǵȿ����� */
			#else
				pAscDot = (uint32_t)&g_Ascii12[' ' * 12];	/* ʹ��CPU��Ƕ��16�����ַ� */
			#endif			
			break;
		
		case FC_ST_16:
			font_bytes = 16;
			#if 0
				pAscDot = ASC16_ADDR;	/* �ֿ�оƬ��16�����ַ����ÿ�,�ʻ�ϸ�ˣ������Ƿǵȿ����� */
			#else
				pAscDot = (uint32_t)&g_Ascii16[' ' * 16];	/* ʹ��CPU��Ƕ��16�����ַ� */
			#endif
			break;
		
		case FC_ST_24:
			font_bytes = 48;
			pAscDot = ASC24_ADDR;
			break;
		
		case FC_ST_32:
			font_bytes = 64;
			pAscDot = ASC32_ADDR;
			break;
		
		default:
			return;
	}			
	if (_code >=0x20 && _code <= 0x7E)
	{		
		pAscDot = ((uint32_t)_code - 0x20)*font_bytes + pAscDot;
	}

#ifdef USE_NOR_FONT		/* NOR Flashȫ�ֿ� */
	/* ��CPU�ڲ�Flash�е�ascii�ַ������Ƶ�buf */
	memcpy(_pBuf, (char *)pAscDot, font_bytes);	
#endif

#ifdef USE_SPI_FONT		/* ���� Flashȫ�ֿ� */
	if (_fontcode == FC_ST_12 || _fontcode == FC_ST_16)
	{
		memcpy(_pBuf, (char *)pAscDot, font_bytes);	
	}
	else
	{
		/* �ֿ�оƬ��12�����16�����ַ����ÿ�,�ʻ�ϸ�ˣ������Ƿǵȿ����� */
		sf_ReadBuffer(_pBuf, pAscDot, font_bytes);
	}
#endif
	
#endif
}

/*
*********************************************************************************************************
*	�� �� ��: _LCD_ReadHZDot
*	����˵��: ��ȡ1�����ֵĵ�������
*	��    ��:
*		_code1, _cod2 : ��������. GB2312����
*		_fontcode ���������
*		_pBuf : ��Ŷ������ַ���������
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void _LCD_ReadHZDot(uint8_t _code1, uint8_t _code2,  uint8_t _fontcode, uint8_t *_pBuf)
{
#ifdef USE_SMALL_FONT	/* ʹ��CPU �ڲ�Flash С�ֿ� */
	uint8_t *pDot;
	uint8_t font_bytes = 0;
	uint32_t address;
	uint16_t m;

	pDot = 0;	/* �������ڱ���澯 */
	switch (_fontcode)
	{
		case FC_ST_12:		/* 12���� */
			font_bytes = 24;
			pDot = (uint8_t *)g_Hz12;	
			break;
		
		case FC_ST_16:
			font_bytes = 32;
			pDot = (uint8_t *)g_Hz16;
			break;

		case FC_ST_24:
			font_bytes = 72;
			pDot = (uint8_t *)g_Hz24;
			break;			
			
		case FC_ST_32:	
			font_bytes = 128;
			pDot = (uint8_t *)g_Hz32;
			break;						
		
		default:
			return;
	}	

	m = 0;
	while(1)
	{
		address = m * (font_bytes + 2);
		m++;
		if ((_code1 == pDot[address + 0]) && (_code2 == pDot[address + 1]))
		{
			address += 2;
			memcpy(_pBuf, &pDot[address], font_bytes);
			break;
		}
		else if ((pDot[address + 0] == 0xFF) && (pDot[address + 1] == 0xFF))
		{
			/* �ֿ�������ϣ�δ�ҵ��������ȫFF */
			memset(_pBuf, 0xFF, font_bytes);
			break;
		}
	}
#else	/* ��ȫ�ֿ� */
	uint32_t offset = 0;
	uint8_t font_bytes = 0;
		
	switch (_fontcode)
	{
		case FC_ST_12:		/* 12���� */
			font_bytes = 24;
			offset = HZK12_ADDR;	
			break;
		
		case FC_ST_16:
			font_bytes = 32;
			offset = HZK16_ADDR;
			break;

		case FC_ST_24:
			font_bytes = 72;
			offset = HZK24_ADDR;
			break;			
			
		case FC_ST_32:	
			font_bytes = 128;
			offset = HZK32_ADDR;
			break;						
		
		default:
			return;
	}			

	/* �˴���Ҫ�����ֿ��ļ����λ�ý����޸� 
		GB2312��Χ�� 0xA1A1 - 0xFEFE
		���к��ַ�Χ : 0xB0A1 - 0xF7FE
	
		GBK ��Χ�� 0x8140 - 0xFEFE 
	
		�������Զ��庺�ֱ����GBK��GB2312����ռ䣺 0x8000 - 0x813F ��319����		
	*/
	if (_code1 >=0xA1 && _code1 <= 0xA9 && _code2 >=0xA1)
	{
		offset += ((_code1 - 0xA1) * 94 + (_code2 - 0xA1)) * font_bytes;
	}
	else if (_code1 >=0xB0 && _code1 <= 0xF7 && _code2 >=0xA1)
	{
		offset += ((_code1 - 0xB0) * 94 + (_code2 - 0xA1) + 846) * font_bytes;
	}
	else	/* 2018-03-13 �����Զ��庺�ֱ��룬����ʵ������ͼ����� */
	{
		uint16_t code16;
		uint8_t *pDot;
		uint32_t address;
		uint16_t m;		
		
		code16 = _code1 * 256 + _code2;
		if (code16 >= 0x8000 && code16 <= 0x813F)	/* �Զ��庺�ֵ��󣬹̶�ʹ��CPUƬ�ڲ�С�ֿ� */
		{
			pDot = 0;	/* �������ڱ���澯 */
			switch (_fontcode)
			{
				case FC_ST_12:		/* 12���� */
					font_bytes = 24;
					pDot = (uint8_t *)g_Hz12;	
					break;
				
				case FC_ST_16:
					font_bytes = 32;
					pDot = (uint8_t *)g_Hz16;
					break;

				case FC_ST_24:
					font_bytes = 72;
					pDot = (uint8_t *)g_Hz24;
					break;			
					
				case FC_ST_32:	
					font_bytes = 128;
					pDot = (uint8_t *)g_Hz32;
					break;						
				
				default:
					break;
			}	
			
			m = 0;
			while(1)
			{
				address = m * (font_bytes + 2);
				m++;
				if ((_code1 == pDot[address + 0]) && (_code2 == pDot[address + 1]))
				{
					address += 2;
					memcpy(_pBuf, &pDot[address], font_bytes);
					break;
				}
				else if ((pDot[address + 0] == 0xFF) && (pDot[address + 1] == 0xFF))
				{
					/* �ֿ�������ϣ�δ�ҵ��������ȫFF */
					memset(_pBuf, 0xFF, font_bytes);
					break;
				}
			}	
			return;
		}
	}

#ifdef USE_NOR_FONT		/* NOR Flashȫ�ֿ� */
	/* ��CPU�ڲ�Flash�е�ascii�ַ������Ƶ�buf */
	memcpy(_pBuf, (char *)offset, font_bytes);	
#endif

#ifdef USE_SPI_FONT		/* NOR Flashȫ�ֿ� */
	sf_ReadBuffer(_pBuf, offset, font_bytes);
#endif
	
#endif
}

/*
*********************************************************************************************************
*	�� �� ��: SeachStr_a
*	����˵��: ����һ���ַ����Ƿ��С�\a��
*	��    ��:
*		_ptr  : �ַ���ָ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static uint8_t SeachStr_a(char *_ptr)
{
	uint16_t j = 0;
	uint8_t a_flag = 0;
	
	while(_ptr[j] != 0)
	{
		if (_ptr[j] == '\a')
		{
			a_flag = 1;
			break;
		}
		j++;
	}
	
	return a_flag;
}

/*
*********************************************************************************************************
*	�� �� ��: LCD_DispStrEx0
*	����˵��: ��LCDָ�����꣨���Ͻǣ���ʾһ���ַ����� ��ǿ�ͺ�����֧����\��\�Ҷ��룬֧�ֶ���������
*	��    ��:
*		_usX : X����
*		_usY : Y����
*		_ptr  : �ַ���ָ��
*		_tFont : ����ṹ�壬������ɫ������ɫ(֧��͸��)��������롢���ּ��Ȳ���������ָ��RA8875�ֿ���ʾ����
*		_Width : �ַ�����ʾ����Ŀ��. 0 ��ʾ�������������򣬴�ʱ_Align��Ч
*		_Align :�ַ�������ʾ����Ķ��뷽ʽ��
*				ALIGN_LEFT = 0,
*				ALIGN_CENTER = 1,
*				ALIGN_RIGHT = 2
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void LCD_DispStrEx0(uint16_t _usX, uint16_t _usY, char *_ptr, FONT_T *_tFont, uint16_t _Width,
	uint8_t _Align)
{
	uint32_t i;
	uint8_t code1;
	uint8_t code2;
	//uint8_t buf[32 * 32 / 8];	/* ���֧��32������ */
	uint8_t buf[96 * 40 / 8];	/* ���֧��96x40�����ַ� */
	uint8_t width;
	uint16_t m;
	uint8_t font_width = 0;
	uint8_t font_height = 0;
	uint16_t x, y;
	uint16_t offset;
	uint16_t str_width;	/* �ַ���ʵ�ʿ��  */
	uint8_t ra8875_use = 0;
	uint8_t ra8875_font_code = 0;
	uint16_t address;
	uint8_t a_flag = 0;
	uint8_t RA8875_flag = 0;
	
	uint8_t line_bytes;
	uint8_t asc_bytes = 0;
	uint8_t hz_bytes = 0;

	switch (_tFont->FontCode)
	{
		case FC_ST_12:		/* 12���� */
			font_height = 12;
			font_width = 12;
			asc_bytes = 1;
			hz_bytes = 2;
			break;
		
		case FC_ST_16:
			font_height = 16;
			font_width = 16;
			asc_bytes = 1;
			hz_bytes = 2;
			break;

		case FC_ST_24:
			font_height = 24;
			font_width = 24;
			asc_bytes = 2;
			hz_bytes = 3;
			break;
						
		case FC_ST_32:	
			font_height = 32;
			font_width = 32;
			asc_bytes = 2;
			hz_bytes = 4;
			break;		

		case FC_ST_62X40:	
			font_height = 62;
			font_width = 80;
			asc_bytes = 5;
			hz_bytes = 10;
			break;			

		case FC_ST_96X40:	
			font_height = 96;
			font_width = 80;
			asc_bytes = 5;
			hz_bytes = 10;
			break;		
		
		case FC_RA8875_16:
			ra8875_font_code = RA_FONT_16;
			ra8875_use = 1;	/* ��ʾ��RA8875�ֿ� */
			break;
			
		case FC_RA8875_24:
			ra8875_font_code = RA_FONT_24;
			a_flag = SeachStr_a(_ptr);	/* �����ַ������Ƿ���'\a' */
			if (a_flag == 0)	/**/
			{
				ra8875_use = 1;	/* ��ʾ��RA8875�ֿ� */
			}
			else
			{
				ra8875_use = 0;	/* ��ʾ����RA8875�ֿ� */
				font_height = 24;
				font_width = 24;
				asc_bytes = 3;
				hz_bytes = 3;
			}
			break;
						
		case FC_RA8875_32:
			ra8875_font_code = RA_FONT_32;
			a_flag = SeachStr_a(_ptr);
			if (a_flag == 0)
			{
				ra8875_use = 1;	/* ��ʾ��RA8875�ֿ� */
			}
			else
			{
				ra8875_use = 0;	/* ��ʾ����RA8875�ֿ� */
				font_height = 32;
				font_width = 32;
				asc_bytes = 4;
				hz_bytes = 4;
			}
			break;
	}
	
	str_width = LCD_GetStrWidth(_ptr, _tFont);	/* �����ַ���ʵ�ʿ��(RA8875�ڲ�ASCII������Ϊ�䳤 */
	offset = 0;
	if (_Width > str_width)
	{
		if (_Align == ALIGN_RIGHT)	/* �Ҷ��� */
		{
			offset = _Width - str_width;
		}
		else if (_Align == ALIGN_CENTER)	/* ����� */
		{
			offset = (_Width - str_width) / 2;
		}
		else	/* ����� ALIGN_LEFT */
		{
			;
		}
	}

	/* ������ɫ, �м������ұ߶���  */
	if (offset > 0)
	{
		if (_tFont->BackColor != CL_MASK)	/* ͸��ɫ */
		{
			LCD_Fill_Rect(_usX, _usY, LCD_GetFontHeight(_tFont), offset,  _tFont->BackColor);	
		}
		_usX += offset;
		
	}
	
	/* �Ҳ����ɫ */
	if (_Width > str_width)
	{
		if (_tFont->BackColor != CL_MASK)	/* ͸��ɫ */
		{
			LCD_Fill_Rect(_usX + str_width, _usY, LCD_GetFontHeight(_tFont), _Width - str_width - offset,  _tFont->BackColor);
		}
	}
	
	if (ra8875_use == 1)	/* ʹ��RA8875��ҵ��ֿ�оƬ */
	{
		if (g_ChipID == IC_8875)
		{
			if (_tFont->BackColor == CL_MASK)	/* ͸��ɫ */
			{
				RA8875_SetTextTransp(1);
			}
			RA8875_SetFrontColor(_tFont->FrontColor);			/* ��������ǰ��ɫ */
			RA8875_SetBackColor(_tFont->BackColor);				/* �������屳��ɫ */
			RA8875_SetFont(ra8875_font_code, 0, _tFont->Space);	/* ������룬�м�࣬�ּ�� */
			RA8875_DispStr(_usX, _usY, _ptr);
			if (_tFont->BackColor == CL_MASK)	/* ͸��ɫ */
			{
				RA8875_SetTextTransp(0);
			}	
		}
		else if (g_ChipID == IC_8876)
		{
		  #if 0
			if (_tFont->BackColor == CL_MASK)	/* ͸��ɫ */
			{
				Font_Background_select_Original_Canvas();		/* ��ʾ����ԭ������ɫ */	
			}
			RA8876_SetFont(ra8875_font_code, 0, _tFont->Space);	/* ������룬�м�࣬�ּ�� */
			RA8876_DispStr(0, _usX, _usY, _tFont->FrontColor, _tFont->BackColor, _ptr);		/* ��һ���βα�ʾRA8876ѡ�д���flash1 */
			if (_tFont->BackColor == CL_MASK)	/* ͸��ɫ */
			{
				Font_Background_select_Color();					/* ��ʾ����ѡ�����ɫ */
			}
		  #endif
		}
	}
	else	/* ʹ��CPU�ڲ��ֿ�. ������Ϣ��CPU��ȡ */
	{
		/* ��ʼѭ�������ַ� */
		while (*_ptr != 0)
		{
			code1 = *_ptr;	/* ��ȡ�ַ������ݣ� �����ݿ�����ascii���룬Ҳ���ܺ��ִ���ĸ��ֽ� */
				
			if (code1 < 0x80)
			{
				if (a_flag == 0)
				{
					RA8875_flag = 0;
					/* ��ascii�ַ������Ƶ�buf */
					_LCD_ReadAsciiDot(code1, _tFont->FontCode, buf);	/* ��ȡASCII�ַ����� */
					
					//����������⴦��,�����ȹ���
					if (_tFont->FontCode == FC_ST_62X40 || _tFont->FontCode == FC_ST_96X40)
					{
						if (code1 == 0x5E)
						{
							width = 28;
						}
						else
						{
							width = font_width / 2;
						}	
					}
					else
					{
						width = font_width / 2;
					}
					
					
					line_bytes = asc_bytes;
				}	
				else
				{
					if (code1 == '\a')
					{
						RA8875_flag = 0;
						_ptr++;
						code1 = *_ptr;
						if (_tFont->FontCode == FC_RA8875_32)
						{
							m = 0;
							while(1)
							{
							   address = m * (128 + 2);
							   m++;
							   if (code1 == g_Ascii32_VarWidth[address + 0])
							   {
								  font_width = g_Ascii32_VarWidth[address + 1];
								  break;
							   }
							   else if ((g_Ascii32_VarWidth[address + 0] == 0xFF) && (g_Ascii32_VarWidth[address + 1] == 0xFF))
							   {
	//							  /* �ֿ�������ϣ�δ�ҵ��������ȫFF */
	//							  memset(g_Ascii32_VarWidth, 0xFF, 128);
								  break;
							   }	   
							}
						}
						else if (_tFont->FontCode == FC_RA8875_24)
						{
							m = 0;
							while(1)
							{
							   address = m * (72 + 2);
							   m++;
							   if (code1 == g_Ascii24_VarWidth[address + 0])
							   {
								  font_width = g_Ascii24_VarWidth[address + 1];
								  break;
							   }
							   else if ((g_Ascii24_VarWidth[address + 0] == 0xFF) && (g_Ascii24_VarWidth[address + 1] == 0xFF))
							   {
	//							  /* �ֿ�������ϣ�δ�ҵ��������ȫFF */
	//							  memset(g_Ascii32_VarWidth, 0xFF, 128);
								  break;
							   }	   
						   }
						}	
						_LCD_ReadSmallDot(code1, _tFont->FontCode, buf);
						
						width = font_width;
					
						line_bytes = asc_bytes;
					}
					else
					{
						RA8875_flag = 1;
						if (_tFont->FontCode == FC_RA8875_32)
						{
							font_width = g_RA8875_Ascii32_width[code1 - 0x20];
						}
						else if (_tFont->FontCode == FC_RA8875_24)
						{
							font_width = g_RA8875_Ascii24_width[code1 - 0x20];
						}
						width = font_width;
						line_bytes = asc_bytes;
					}
				}
			}
			else
			{
				RA8875_flag = 0;
				code2 = *++_ptr;
				if (code2 == 0)
				{
					break;
				}
				/* ��1�����ֵĵ��� */
				_LCD_ReadHZDot(code1, code2, _tFont->FontCode, buf);
				width = font_width;
				line_bytes = hz_bytes;
			}
			
			y = _usY;
			if (RA8875_flag == 0)
			{
				/* ��ʼˢLCD */
				for (m = 0; m < font_height; m++)	/* �ַ��߶� */
				{
					x = _usX;
					for (i = 0; i < width; i++)	/* �ַ���� */
					{
						if ((buf[m * line_bytes + i / 8] & (0x80 >> (i % 8 ))) != 0x00)
						{
							LCD_PutPixel(x, y, _tFont->FrontColor);	/* ����������ɫΪ����ɫ */
						}
						else
						{
							if (_tFont->BackColor != CL_MASK)	/* ͸��ɫ */
							{
								LCD_PutPixel(x, y, _tFont->BackColor);	/* ����������ɫΪ���ֱ���ɫ */
							}
						}
		
						x++;
					}
					
					for (i = 0; i < _tFont->Space; i++)	/* �ַ���� */
					{
						if (_tFont->BackColor != CL_MASK)	/* ͸��ɫ */
						{						
							/* ������ֵ�ɫ��_tFont->usBackColor�������ּ����ڵ���Ŀ�ȣ���ô��Ҫ������֮�����(��ʱδʵ��) */
							LCD_PutPixel(x + i, y, _tFont->BackColor);	/* ����������ɫΪ���ֱ���ɫ */
						}
					}
					y++;					
				}
			}
			else
			{
				if (_tFont->BackColor == CL_MASK)	/* ͸��ɫ */
				{
					RA8875_SetTextTransp(1);
				}
				RA8875_SetFrontColor(_tFont->FrontColor);			/* ��������ǰ��ɫ */
				RA8875_SetBackColor(_tFont->BackColor);				/* �������屳��ɫ */
				RA8875_SetFont(ra8875_font_code, 0, _tFont->Space);	/* ������룬�м�࣬�ּ�� */
				RA8875_DispStr(_usX, _usY, (char *)&code1);
				if (_tFont->BackColor == CL_MASK)	/* ͸��ɫ */
				{
					RA8875_SetTextTransp(0);
				}	
			}
	
			_usX += width + _tFont->Space;	/* �е�ַ���� */
			_ptr++;			/* ָ����һ���ַ� */
		}
	}
}

/*
*********************************************************************************************************
*	�� �� ��: LCD_DrawPoints
*	����˵��: ���� Bresenham �㷨������һ��㣬������Щ�����������������ڲ�����ʾ��
*	��    ��:
*			x, y     : ��������
*			_usColor : ��ɫ
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void LCD_DrawPoints(uint16_t *x, uint16_t *y, uint16_t _usSize, uint16_t _usColor)
{
	uint16_t i;

	for (i = 0 ; i < _usSize - 1; i++)
	{
		LCD_DrawLine(x[i], y[i], x[i + 1], y[i + 1], _usColor);
	}
}

/*
*********************************************************************************************************
*	�� �� ��: LCD_DrawWin
*	����˵��: ��LCD�ϻ���һ������
*	��    ��: �ṹ��ָ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void LCD_DrawWin(WIN_T *_pWin)
{
#if 1
	uint16_t TitleHegiht;

	TitleHegiht = 20;

	/* ���ƴ������ */
	LCD_DrawRect(_pWin->Left, _pWin->Top, _pWin->Height, _pWin->Width, WIN_BORDER_COLOR);
	LCD_DrawRect(_pWin->Left + 1, _pWin->Top + 1, _pWin->Height - 2, _pWin->Width - 2, WIN_BORDER_COLOR);

	/* ���ڱ����� */
	LCD_Fill_Rect(_pWin->Left + 2, _pWin->Top + 2, TitleHegiht, _pWin->Width - 4, WIN_TITLE_COLOR);

	/* ������� */
	LCD_Fill_Rect(_pWin->Left + 2, _pWin->Top + TitleHegiht + 2, _pWin->Height - 4 - TitleHegiht,
		_pWin->Width - 4, WIN_BODY_COLOR);

	LCD_DispStr(_pWin->Left + 3, _pWin->Top + 2, _pWin->pCaption, _pWin->Font);
#else
	if (g_ChipID == IC_8875)
	{
		uint16_t TitleHegiht;

		TitleHegiht = 28;

		/* ���ƴ������ */
		RA8875_DrawRect(_pWin->Left, _pWin->Top, _pWin->Height, _pWin->Width, WIN_BORDER_COLOR);
		RA8875_DrawRect(_pWin->Left + 1, _pWin->Top + 1, _pWin->Height - 2, _pWin->Width - 2, WIN_BORDER_COLOR);

		/* ���ڱ����� */
		RA8875_FillRect(_pWin->Left + 2, _pWin->Top + 2, TitleHegiht, _pWin->Width - 4, WIN_TITLE_COLOR);

		/* ������� */
		RA8875_FillRect(_pWin->Left + 2, _pWin->Top + TitleHegiht + 2, _pWin->Height - 4 - TitleHegiht, _pWin->Width - 4, WIN_BODY_COLOR);

		//RA8875_SetFont(_pWin->Font.FontCode, 0, 0);
		RA8875_SetFont(RA_FONT_24, 0, 0);

		RA8875_SetBackColor(WIN_TITLE_COLOR);
		RA8875_SetFrontColor(WIN_CAPTION_COLOR);
		RA8875_DispStr(_pWin->Left + 3, _pWin->Top + 2, _pWin->Caption);
	}
	else
	{
		;
	}
#endif
}


/*
*********************************************************************************************************
*	�� �� ��: LCD_DrawIcon
*	����˵��: ��LCD�ϻ���һ��ͼ�꣬�Ľ��Զ���Ϊ����
*	��    ��: _pIcon : ͼ��ṹ
*			  _tFont : ��������
*			  _ucFocusMode : ����ģʽ��0 ��ʾ����ͼ��  1��ʾѡ�е�ͼ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void LCD_DrawIcon(const ICON_T *_tIcon, FONT_T *_tFont, uint8_t _ucFocusMode)
{
	const uint16_t *p;
	uint16_t usNewRGB;
	uint16_t x, y;		/* ���ڼ�¼�����ڵ�������� */

	p = _tIcon->pBmp;
	for (y = 0; y < _tIcon->Height; y++)
	{
		for (x = 0; x < _tIcon->Width; x++)
		{
			usNewRGB = *p++;	/* ��ȡͼ�����ɫֵ��ָ���1 */
			/* ��ͼ���4��ֱ���и�Ϊ���ǣ��������Ǳ���ͼ�� */
			if ((y == 0 && (x < 6 || x > _tIcon->Width - 7)) ||
				(y == 1 && (x < 4 || x > _tIcon->Width - 5)) ||
				(y == 2 && (x < 3 || x > _tIcon->Width - 4)) ||
				(y == 3 && (x < 2 || x > _tIcon->Width - 3)) ||
				(y == 4 && (x < 1 || x > _tIcon->Width - 2)) ||
				(y == 5 && (x < 1 || x > _tIcon->Width - 2))	||

				(y == _tIcon->Height - 1 && (x < 6 || x > _tIcon->Width - 7)) ||
				(y == _tIcon->Height - 2 && (x < 4 || x > _tIcon->Width - 5)) ||
				(y == _tIcon->Height - 3 && (x < 3 || x > _tIcon->Width - 4)) ||
				(y == _tIcon->Height - 4 && (x < 2 || x > _tIcon->Width - 3)) ||
				(y == _tIcon->Height - 5 && (x < 1 || x > _tIcon->Width - 2)) ||
				(y == _tIcon->Height - 6 && (x < 1 || x > _tIcon->Width - 2))
				)
			{
				;
			}
			else
			{
				if (_ucFocusMode != 0)	/* 1��ʾѡ�е�ͼ�� */
				{
					/* ����ԭʼ���ص����ȣ�ʵ��ͼ�걻����ѡ�е�Ч�� */
					uint16_t R,G,B;
					uint16_t bright = 15;

					/* rrrr rggg gggb bbbb */
					R = (usNewRGB & 0xF800) >> 11;
					G = (usNewRGB & 0x07E0) >> 5;
					B =  usNewRGB & 0x001F;
					if (R > bright)
					{
						R -= bright;
					}
					else
					{
						R = 0;
					}
					if (G > 2 * bright)
					{
						G -= 2 * bright;
					}
					else
					{
						G = 0;
					}
					if (B > bright)
					{
						B -= bright;
					}
					else
					{
						B = 0;
					}
					usNewRGB = (R << 11) + (G << 5) + B;
				}

				LCD_PutPixel(x + _tIcon->Left, y + _tIcon->Top, usNewRGB);
			}
		}
	}

	/* ����ͼ���µ����� */
	{
		uint16_t len;
		uint16_t width;

		len = strlen(_tIcon->Text);

		if  (len == 0)
		{
			return;	/* ���ͼ���ı�����Ϊ0������ʾ */
		}

		/* �����ı����ܿ�� */
		if (_tFont->FontCode == FC_ST_12)		/* 12���� */
		{
			width = 6 * (len + _tFont->Space);
		}
		else	/* FC_ST_16 */
		{
			width = 8 * (len + _tFont->Space);
		}


		/* ˮƽ���� */
		x = (_tIcon->Left + _tIcon->Width / 2) - width / 2;
		y = _tIcon->Top + _tIcon->Height + 2;
		LCD_DispStr(x, y, (char *)_tIcon->Text, _tFont);
	}
}

/*
*********************************************************************************************************
*	�� �� ��: LCD_Blend565
*	����˵��: ������͸���� ��ɫ���
*	��    ��: src : ԭʼ����
*			  dst : ��ϵ���ɫ
*			  alpha : ͸���� 0-32
*	�� �� ֵ: ��
*********************************************************************************************************
*/
uint16_t LCD_Blend565(uint16_t src, uint16_t dst, uint8_t alpha)
{
	uint32_t src2;
	uint32_t dst2;

	src2 = ((src << 16) |src) & 0x07E0F81F;
	dst2 = ((dst << 16) | dst) & 0x07E0F81F;
	dst2 = ((((dst2 - src2) * alpha) >> 5) + src2) & 0x07E0F81F;
	return (dst2 >> 16) | dst2;
}

/*
*********************************************************************************************************
*	�� �� ��: LCD_DrawIcon32
*	����˵��: ��LCD�ϻ���һ��ͼ��, ����͸����Ϣ��λͼ(32λ�� RGBA). ͼ���´�����
*	��    ��: _pIcon : ͼ��ṹ
*			  _tFont : ��������
*			  _ucFocusMode : ����ģʽ��0 ��ʾ����ͼ��  1��ʾѡ�е�ͼ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void LCD_DrawIcon32(const ICON_T *_tIcon, FONT_T *_tFont, uint8_t _ucFocusMode)
{
	const uint8_t *p;
	uint16_t usOldRGB, usNewRGB;
	int16_t x, y;		/* ���ڼ�¼�����ڵ�������� */
	uint8_t R1,G1,B1,A;	/* ������ɫ�ʷ��� */
	uint8_t R0,G0,B0;	/* ������ɫ�ʷ��� */

	p = (const uint8_t *)_tIcon->pBmp;
	p += 54;		/* ֱ��ָ��ͼ�������� */

	/* ����BMPλͼ���򣬴������ң���������ɨ�� */
	for (y = _tIcon->Height - 1; y >= 0; y--)
	{
		for (x = 0; x < _tIcon->Width; x++)
		{
			B1 = *p++;
			G1 = *p++;
			R1 = *p++;
			A = *p++;	/* Alpha ֵ(͸����)��0-255, 0��ʾ͸����1��ʾ��͸��, �м�ֵ��ʾ͸���� */

			if (A == 0x00)	/* ��Ҫ͸��,��ʾ���� */
			{
				;	/* ����ˢ�±��� */
			}
			else if (A == 0xFF)	/* ��ȫ��͸���� ��ʾ������ */
			{
				usNewRGB = RGB(R1, G1, B1);
				if (_ucFocusMode == 1)
				{
					usNewRGB = LCD_Blend565(usNewRGB, CL_YELLOW, 10);
				}
				LCD_PutPixel(x + _tIcon->Left, y + _tIcon->Top, usNewRGB);
			}
			else 	/* ��͸�� */
			{
				/* ���㹫ʽ�� ʵ����ʾ��ɫ = ǰ����ɫ * Alpha / 255 + ������ɫ * (255-Alpha) / 255 */
				usOldRGB = LCD_GetPixel(x + _tIcon->Left, y + _tIcon->Top);
				
				//usOldRGB = 0xFFFF;
				R0 = RGB565_R(usOldRGB);
				G0 = RGB565_G(usOldRGB);
				B0 = RGB565_B(usOldRGB);

				R1 = (R1 * A) / 255 + R0 * (255 - A) / 255;
				G1 = (G1 * A) / 255 + G0 * (255 - A) / 255;
				B1 = (B1 * A) / 255 + B0 * (255 - A) / 255;
				usNewRGB = RGB(R1, G1, B1);
				if (_ucFocusMode == 1)
				{
					usNewRGB = LCD_Blend565(usNewRGB, CL_YELLOW, 10);
				}
				LCD_PutPixel(x + _tIcon->Left, y + _tIcon->Top, usNewRGB);
				
				LCD_PutPixel(x + _tIcon->Left, y + _tIcon->Top, usNewRGB);
			}
		}
	}

	/* ����ͼ���µ����� */
	{
		uint16_t len;
		uint16_t width;

		len = strlen(_tIcon->Text);

		if  (len == 0)
		{
			return;	/* ���ͼ���ı�����Ϊ0������ʾ */
		}

		/* �����ı����ܿ�� */
		if (_tFont->FontCode == FC_ST_12)		/* 12���� */
		{
			width = 6 * (len + _tFont->Space);
		}
		else	/* FC_ST_16 */
		{
			width = 8 * (len + _tFont->Space);
		}


		/* ˮƽ���� */
		x = (_tIcon->Left + _tIcon->Width / 2) - width / 2;
		y = _tIcon->Top + _tIcon->Height + 2;
		LCD_DispStr(x, y, (char *)_tIcon->Text, _tFont);
	}
}

/*
*********************************************************************************************************
*	�� �� ��: LCD_DrawBmp32
*	����˵��: ��LCD�ϻ���һ��32λ��BMPͼ, ����͸����Ϣ��λͼ(32λ�� RGBA)
*	��    ��: _usX, _usY : ��ʾ����
*			  _usHeight, _usWidth : ͼƬ�߶ȺͿ��
*			  _pBmp : ͼƬ���ݣ���BMP�ļ�ͷ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void LCD_DrawBmp32(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth, uint8_t *_pBmp)
{
	const uint8_t *p;
	uint16_t usOldRGB, usNewRGB;
	int16_t x, y;		/* ���ڼ�¼�����ڵ�������� */
	uint8_t R1,G1,B1,A;	/* ������ɫ�ʷ��� */
	uint8_t R0,G0,B0;	/* ������ɫ�ʷ��� */

	p = (const uint8_t *)_pBmp;
	p += 54;		/* ֱ��ָ��ͼ�������� */

	/* ����BMPλͼ���򣬴������ң���������ɨ�� */
	for (y = _usHeight - 1; y >= 0; y--)
	{
		for (x = 0; x < _usWidth; x++)
		{
			B1 = *p++;
			G1 = *p++;
			R1 = *p++;
			A = *p++;	/* Alpha ֵ(͸����)��0-255, 0��ʾ͸����1��ʾ��͸��, �м�ֵ��ʾ͸���� */

			if (A == 0x00)	/* ��Ҫ͸��,��ʾ���� */
			{
				;	/* ����ˢ�±��� */
			}
			else if (A == 0xFF)	/* ��ȫ��͸���� ��ʾ������ */
			{
				usNewRGB = RGB(R1, G1, B1);
				//if (_ucFocusMode == 1)
				//{
				//	usNewRGB = Blend565(usNewRGB, CL_YELLOW, 10);
				//}
				LCD_PutPixel(x + _usX, y + _usY, usNewRGB);
			}
			else 	/* ��͸�� */
			{
				/* ���㹫ʽ�� ʵ����ʾ��ɫ = ǰ����ɫ * Alpha / 255 + ������ɫ * (255-Alpha) / 255 */
				usOldRGB = LCD_GetPixel(x + _usX, y + _usY);
				R0 = RGB565_R(usOldRGB);
				G0 = RGB565_G(usOldRGB);
				B0 = RGB565_B(usOldRGB);

				R1 = (R1 * A) / 255 + R0 * (255 - A) / 255;
				G1 = (G1 * A) / 255 + G0 * (255 - A) / 255;
				B1 = (B1 * A) / 255 + B0 * (255 - A) / 255;
				usNewRGB = RGB(R1, G1, B1);
				//if (_ucFocusMode == 1)
				//{
				//	usNewRGB = Blend565(usNewRGB, CL_YELLOW, 10);
				//}
				LCD_PutPixel(x + _usX, y + _usY, usNewRGB);
			}
		}
	}
}

/*
*********************************************************************************************************
*	�� �� ��: LCD_DrawLabel
*	����˵��: ����һ���ı���ǩ
*	��    ��: _pLabel : Label�ṹ��ָ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void LCD_InitLabel(LABEL_T *_pLabel, uint16_t _x, uint16_t _y, uint16_t _h, uint16_t _w, 
	char *_Text, FONT_T *_tFont)
{
	_pLabel->Left = _x;
	_pLabel->Top = _y;
	_pLabel->Height = _h;
	_pLabel->Width = _w;
	_pLabel->pCaption = _Text;
	_pLabel->Font = _tFont;
	
	_pLabel->MaxLen = 0;
}

/*
*********************************************************************************************************
*	�� �� ��: LCD_DrawLabel
*	����˵��: ����һ���ı���ǩ
*	��    ��: �ṹ��ָ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void LCD_DrawLabel(LABEL_T *_pLabel)
{
#if 1
	char dispbuf[256];
	uint16_t i;
	uint16_t NewLen;

	NewLen = strlen(_pLabel->pCaption);

	if (NewLen > _pLabel->MaxLen)
	{
		LCD_DispStr(_pLabel->Left, _pLabel->Top, _pLabel->pCaption, _pLabel->Font);
		_pLabel->MaxLen = NewLen;
	}
	else
	{
		for (i = 0; i < NewLen; i++)
		{
			dispbuf[i] = _pLabel->pCaption[i];
		}
		for (; i < _pLabel->MaxLen; i++)
		{
			dispbuf[i] = ' ';		/* ĩβ���ո� */
		}
		dispbuf[i] = 0;
		LCD_DispStr(_pLabel->Left, _pLabel->Top, dispbuf, _pLabel->Font);
	}
#else
	if (g_ChipID == IC_8875)
	{
		RA8875_SetFont(_pLabel->Font->FontCode, 0, 0);	/* ����32�������壬�м��=0���ּ��=0 */

		RA8875_SetBackColor(_pLabel->Font->BackColor);
		RA8875_SetFrontColor(_pLabel->Font->FrontColor);

		RA8875_DispStr(_pLabel->Left, _pLabel->Top, _pLabel->Caption);
	}
	else
	{

	}
#endif
}

/*
*********************************************************************************************************
*	�� �� ��: LCD_DrawCheckBox
*	����˵��: ����һ������
*	��    ��: �ṹ��ָ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void LCD_DrawCheckBox(CHECK_T *_pCheckBox)
{
#if 1
	uint16_t x, y;

	/* Ŀǰֻ����16�����ֵĴ�С */

	/* ������� */
	x = _pCheckBox->Left;
	LCD_DrawRect(x, _pCheckBox->Top, CHECK_BOX_H, CHECK_BOX_W, CHECK_BOX_BORDER_COLOR);
	LCD_DrawRect(x + 1, _pCheckBox->Top + 1, CHECK_BOX_H - 2, CHECK_BOX_W - 2, CHECK_BOX_BORDER_COLOR);
	LCD_Fill_Rect(x + 2, _pCheckBox->Top + 2, CHECK_BOX_H - 4, CHECK_BOX_W - 4, CHECK_BOX_BACK_COLOR);

	/* �����ı���ǩ */
	x = _pCheckBox->Left + CHECK_BOX_W + 2;
	y = _pCheckBox->Top + CHECK_BOX_H / 2 - 8;
	LCD_DispStr(x, y, _pCheckBox->pCaption, _pCheckBox->Font);

	if (_pCheckBox->Checked)
	{
		FONT_T font;

	    font.FontCode = FC_ST_16;
		font.BackColor = CL_MASK;
		font.FrontColor = CHECK_BOX_CHECKED_COLOR;	/* ������ɫ */
		font.Space = 0;
		x = _pCheckBox->Left;
		LCD_DispStr(x + 3, _pCheckBox->Top + 3, "��", &font);
	}
#else
	if (g_ChipID == IC_8875)
	{
		uint16_t x;

		RA8875_SetFont(_pCheckBox->Font.FontCode, 0, 0);	/* ����32�������壬�м��=0���ּ��=0 */

		/* ���Ʊ�ǩ */
		//RA8875_SetBackColor(_pCheckBox->Font.BackColor);
		RA8875_SetBackColor(WIN_BODY_COLOR);
		RA8875_SetFrontColor(_pCheckBox->Font.FrontColor);
		RA8875_DispStr(_pCheckBox->Left, _pCheckBox->Top, _pCheckBox->Caption);

		/* ������� */
		x = _pCheckBox->Left + _pCheckBox->Width - CHECK_BOX_W;
		RA8875_DrawRect(x, _pCheckBox->Top, CHECK_BOX_H, CHECK_BOX_W, CHECK_BOX_BORDER_COLOR);
		RA8875_DrawRect(x + 1, _pCheckBox->Top + 1, CHECK_BOX_H - 2, CHECK_BOX_W - 2, CHECK_BOX_BORDER_COLOR);
		RA8875_FillRect(x + 2, _pCheckBox->Top + 2, CHECK_BOX_H - 4, CHECK_BOX_W - 4, CHECK_BOX_BACK_COLOR);

		if (_pCheckBox->Checked)
		{
			RA8875_SetBackColor(CHECK_BOX_BACK_COLOR);
			RA8875_SetFrontColor(CL_RED);
			RA8875_DispStr(x + 3, _pCheckBox->Top + 3, "��");
		}
	}
	else
	{

	}
#endif

}

/*
*********************************************************************************************************
*	�� �� ��: LCD_InitEdit
*	����˵��: ��ʼ���༭�����
*	��    ��: _pBox �����
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void LCD_InitEdit(EDIT_T *_pEdit, uint16_t _x, uint16_t _y, uint16_t _h, uint16_t _w,
	char  *pCaption, FONT_T *Font)
{
	_pEdit->Left = _x;
	_pEdit->Top = _y;
	_pEdit->Height = _h;
	_pEdit->Width = _w;
	_pEdit->pCaption = pCaption;
	_pEdit->Font = Font;	
	_pEdit->Focus = 0;
}

/*
*********************************************************************************************************
*	�� �� ��: LCD_DrawEdit
*	����˵��: ��LCD�ϻ���һ���༭��
*	��    ��: _pEdit �༭��ṹ��ָ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void LCD_DrawEdit(EDIT_T *_pEdit)
{
	uint16_t len, x, y;
	
	/* ��XP���ƽ��༭�� */
	if (_pEdit->Focus == 0)
	{
		LCD_DrawRect(_pEdit->Left, _pEdit->Top, _pEdit->Height, _pEdit->Width, EDIT_BORDER_COLOR);
		LCD_Fill_Rect(_pEdit->Left + 1, _pEdit->Top + 1, _pEdit->Height - 2, _pEdit->Width - 2, EDIT_BACK_COLOR);
	}
	else
	{
		LCD_DrawRect(_pEdit->Left, _pEdit->Top, _pEdit->Height, _pEdit->Width, EDIT_BORDER_COLOR2);
		LCD_Fill_Rect(_pEdit->Left + 1, _pEdit->Top + 1, _pEdit->Height - 2, _pEdit->Width - 2, EDIT_BACK_COLOR2);
	}	
	
	if (_pEdit->pCaption > 0)
	{
		for (len = 0; len < 32; len++)
		{
			_pEdit->Text[len] = _pEdit->pCaption[len];
			
			if (_pEdit->pCaption[len] == 0)
			{
				break;
			}
		}
		_pEdit->Text[32] = 0;
		
		//_pEdit->pCaption = 0;
	}
	
	/* ���־��� */
	len = LCD_GetStrWidth(_pEdit->Text,  _pEdit->Font);
	x = _pEdit->Left +  (_pEdit->Width - len) / 2;
	y = _pEdit->Top + (_pEdit->Height - LCD_GetFontHeight(_pEdit->Font)) / 2;

	LCD_DispStr(x, y, _pEdit->Text, _pEdit->Font);
}

/*
*********************************************************************************************************
*	�� �� ��: LCD_EditTouchDown
*	����˵��: �жϰ�ť�Ƿ񱻰���. ��鴥�������Ƿ��ڰ�ť�ķ�Χ֮�ڡ����ػ水ť��
*	��    ��:  _edit : �༭�����
*			  _usX, _usY: ��������
*	�� �� ֵ: 1 ��ʾ�ڷ�Χ��
*********************************************************************************************************
*/
uint8_t LCD_EditTouchDown(EDIT_T *_Edit, uint16_t _usX, uint16_t _usY)
{
	if ((_usX > _Edit->Left) && (_usX < _Edit->Left + _Edit->Width)
		&& (_usY > _Edit->Top) && (_usY < _Edit->Top + _Edit->Height))
	{
		BUTTON_BEEP();	/* ������ʾ�� bsp_tft_lcd.h �ļ���ͷ����ʹ�ܺ͹ر� */
		_Edit->Focus = 1;
		LCD_DrawEdit(_Edit);
		return 1;
	}
	else
	{
		return 0;
	}
}

/*
*********************************************************************************************************
*	�� �� ��: LCD_EditTouchRelease
*	����˵��: �༭���˳��༭״̬���ػ�
*	��    ��:  _Edit : �༭�����
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void LCD_EditRefresh(EDIT_T *_Edit)
{
	_Edit->Focus = 0;
	LCD_DrawEdit(_Edit);
}

/*
*********************************************************************************************************
*	�� �� ��: LCD_InitGroupBox
*	����˵��: ��ʼ����������
*	��    ��: _pBox �����
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void LCD_InitGroupBox(GROUP_T *_pBox, uint16_t _x, uint16_t _y, uint16_t _h, uint16_t _w,
	char  *pCaption, FONT_T *Font)
{
	_pBox->Left = _x;
	_pBox->Top = _y;
	_pBox->Height = _h;
	_pBox->Width = _w;
	_pBox->pCaption = pCaption;
	_pBox->Font = Font;
}

/*
*********************************************************************************************************
*	�� �� ��: LCD_DrawGroupBox
*	����˵��: ��LCD�ϻ���һ�������
*	��    ��: _pBox �����
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void LCD_DrawGroupBox(GROUP_T *_pBox)
{
	uint16_t x, y;
	uint16_t x1,y1;		/* �������Ͻ� */
	uint16_t x2, y2;	/* �������½� */
	uint16_t len;

	
	len = LCD_GetStrWidth(_pBox->pCaption, _pBox->Font);	/* �ַ������ܿ�� */
	
	/* ����Ӱ�� */
	//LCD_DrawRect(_pBox->Left + 1, _pBox->Top + 5, _pBox->Height, _pBox->Width - 1, CL_BOX_BORDER2);
	x1 = _pBox->Left + 1;
	y1 = _pBox->Top + 5;
	x2 = _pBox->Left + 1 + _pBox->Width - 2;
	y2 = _pBox->Top + 5 + _pBox->Height - 1;
	
	LCD_DrawLine(x1, y1, x1 + 6, y1, CL_BOX_BORDER2);	/* ��1 */
	LCD_DrawLine(x1 + 8 + len + 1, y1, x2, y1, CL_BOX_BORDER2);	/* ��2 */	
	LCD_DrawLine(x1, y2, x2, y2, CL_BOX_BORDER2);	/* �� */
	LCD_DrawLine(x1, y1, x1, y2, CL_BOX_BORDER2);	/* �� */
	LCD_DrawLine(x2, y1, x2, y2, CL_BOX_BORDER2);	/* �� */	

	/* �������� */
	//LCD_DrawRect(_pBox->Left, _pBox->Top + 4, _pBox->Height, _pBox->Width - 1, CL_BOX_BORDER1);
	x1 = _pBox->Left;
	y1 = _pBox->Top + 4;
	x2 = _pBox->Left + _pBox->Width - 2;
	y2 = _pBox->Top + 4 + _pBox->Height - 1;
	
	LCD_DrawLine(x1, y1, x1 + 6, y1, CL_BOX_BORDER1);	/* ��1 */
	LCD_DrawLine(x1 + 9 + len + 1, y1, x2, y1, CL_BOX_BORDER1);	/* ��2 */	
	LCD_DrawLine(x1, y2, x2, y2, CL_BOX_BORDER1);	/* �� */
	LCD_DrawLine(x1, y1, x1, y2, CL_BOX_BORDER1);	/* �� */
	LCD_DrawLine(x2, y1, x2, y2, CL_BOX_BORDER1);	/* �� */		

	/* ��ʾ�������⣨���������Ͻǣ� */
	x = _pBox->Left + 9;
	y = _pBox->Top;
	LCD_DispStr(x, y, _pBox->pCaption, _pBox->Font);
}

/*
*********************************************************************************************************
*	�� �� ��: LCD_DispControl
*	����˵��: ���ƿؼ�
*	��    ��: _pControl �ؼ�ָ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void LCD_DispControl(void *_pControl)
{
	uint8_t id;

	id = *(uint8_t *)_pControl;	/* ��ȡID */

	switch (id)
	{
		case ID_ICON:
			//void LCD_DrawIcon(const ICON_T *_tIcon, FONT_T *_tFont, uint8_t _ucFocusMode);
			break;

		case ID_WIN:
			LCD_DrawWin((WIN_T *)_pControl);
			break;

		case ID_LABEL:
			LCD_DrawLabel((LABEL_T *)_pControl);
			break;

		case ID_BUTTON:
			LCD_DrawButton((BUTTON_T *)_pControl);
			break;

		case ID_CHECK:
			LCD_DrawCheckBox((CHECK_T *)_pControl);
			break;

		case ID_EDIT:
			LCD_DrawEdit((EDIT_T *)_pControl);
			break;

		case ID_GROUP:
			LCD_DrawGroupBox((GROUP_T *)_pControl);
			break;
	}
}

/*
*********************************************************************************************************
*	�� �� ��: LCD_InitButton
*	����˵��: ��ʼ����ť�ṹ���Ա��
*	��    ��:  _x, _y : ����
*			  _h, _w : �߶ȺͿ��
*			  _pCaption : ��ť����
*			  _pFont : ��ť����
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void LCD_InitButton(BUTTON_T *_btn, uint16_t _x, uint16_t _y, uint16_t _h, uint16_t _w, char *_pCaption, FONT_T *_pFont)
{
	_btn->Left = _x;
	_btn->Top = _y;
	_btn->Height = _h;
	_btn->Width = _w;
	_btn->pCaption = _pCaption;	
	_btn->Font = _pFont;
	_btn->Focus = 0;
}

/*
*********************************************************************************************************
*	�� �� ��: LCD_DrawButton2
*	����˵��: ��LCD�ϻ���һ����ť������emwin��ť
*	��    ��:
*			_usX, _usY : ͼƬ������
*			_usHeight  : ͼƬ�߶�
*			_usWidth   : ͼƬ���
*			_ptr       : ͼƬ����ָ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void LCD_DrawButton(BUTTON_T *_pBtn)
{
	uint16_t x, y;	
	uint8_t muti_line = 0;
		
	{
		uint16_t i;
		
		for (i = 0; i < 1024; i++)
		{
			if (_pBtn->pCaption[i] == '\r' || _pBtn->pCaption[i] == '\t')
			{
				muti_line = 1;
				break;
			}
			if (_pBtn->pCaption[i] == 0)
			{
				break;
			}
		}
	}
	x = _pBtn->Left;		
	if (muti_line == 0)
	{
		y = _pBtn->Top + (_pBtn->Height - LCD_GetFontHeight(_pBtn->Font)) / 2;	/* �����ı���ֱ���� */
	}
	else
	{
		y = _pBtn->Top;		/* �����ı�,��ֱ����Ӷ�����ʼ */
	}	
		
	if (g_ChipID == IC_8875)
	{
		uint8_t Arc = 5;
				
		if (_pBtn->Focus == 0)
		{
			RA8875_DrawRoundRect(_pBtn->Left, _pBtn->Top, _pBtn->Height, _pBtn->Width, Arc,  BTN_BORDER_COLOR1);
			RA8875_FillRoundRect(_pBtn->Left + 1, _pBtn->Top + 1, _pBtn->Height - 2, _pBtn->Width - 2, Arc,  BTN_BODY_COLOR1);			
			LCD_Fill_Rect(_pBtn->Left + Arc, _pBtn->Top + 1, _pBtn->Height / 2, _pBtn->Width - 2 * Arc, BTN_SHADOW_COLOR1);	/* ����Ӱ�Ա�ɫ */
		}
		else
		{
			RA8875_DrawRoundRect(_pBtn->Left, _pBtn->Top, _pBtn->Height, _pBtn->Width, Arc,  BTN_BORDER_COLOR2);
			RA8875_FillRoundRect(_pBtn->Left + 1, _pBtn->Top + 1, _pBtn->Height - 2, _pBtn->Width - 2, Arc, BTN_BODY_COLOR2);			
			LCD_Fill_Rect(_pBtn->Left + Arc, _pBtn->Top + 1, _pBtn->Height / 2, _pBtn->Width - 2 * Arc, BTN_SHADOW_COLOR2);	/* ����Ӱ�Ա�ɫ */
		}

		RA8875_SetTextTransp(1);		
		LCD_DispStrEx(x, y, _pBtn->pCaption, _pBtn->Font, _pBtn->Width, ALIGN_CENTER);
		RA8875_SetTextTransp(0);
	}
	else if (g_ChipID == IC_8876)
	{
		#if 0
		uint8_t Arc = 5;
		
		if (_pBtn->Focus == 0)
		{
			RA8876_DrawRoundRect(_pBtn->Left, _pBtn->Top, _pBtn->Height, _pBtn->Width, Arc,  BTN_BORDER_COLOR1);
			RA8876_FillRoundRect(_pBtn->Left + 1, _pBtn->Top + 1, _pBtn->Height - 2, _pBtn->Width - 2, Arc,  BTN_BODY_COLOR1);			
			LCD_Fill_Rect(_pBtn->Left + Arc, _pBtn->Top + 1, _pBtn->Height / 2, _pBtn->Width - 2 * Arc, BTN_SHADOW_COLOR1);	/* ����Ӱ�Ա�ɫ */
		}
		else
		{
			RA8876_DrawRoundRect(_pBtn->Left, _pBtn->Top, _pBtn->Height, _pBtn->Width, Arc,  BTN_BORDER_COLOR2);
			RA8876_FillRoundRect(_pBtn->Left + 1, _pBtn->Top + 1, _pBtn->Height - 2, _pBtn->Width - 2, Arc, BTN_BODY_COLOR2);			
			LCD_Fill_Rect(_pBtn->Left + Arc, _pBtn->Top + 1, _pBtn->Height / 2, _pBtn->Width - 2 * Arc, BTN_SHADOW_COLOR2);	/* ����Ӱ�Ա�ɫ */
		}
	
		RA8876_SetTextTransp(1);
		LCD_DispStrEx(x, y, _pBtn->pCaption, _pBtn->Font, _pBtn->Width, ALIGN_CENTER);
		RA8876_SetTextTransp(0);
		#endif
	}
	else
	{
		uint8_t Arc = 5;
			
		if (_pBtn->Focus == 0)
		{
			SOFT_FillRoundRect(_pBtn->Left, _pBtn->Top, _pBtn->Height, _pBtn->Width, Arc,  BTN_BODY_COLOR1);			
			SOFT_DrawRoundRect(_pBtn->Left, _pBtn->Top, _pBtn->Height, _pBtn->Width, Arc,  BTN_BORDER_COLOR1);											
			LCD_Fill_Rect(_pBtn->Left + Arc, _pBtn->Top + 1, _pBtn->Height / 2, _pBtn->Width - 2 * Arc, BTN_SHADOW_COLOR1);	/* ����Ӱ�Ա�ɫ */
		}
		else
		{
			SOFT_FillRoundRect(_pBtn->Left, _pBtn->Top, _pBtn->Height, _pBtn->Width, Arc,  BTN_BODY_COLOR2);			
			SOFT_DrawRoundRect(_pBtn->Left, _pBtn->Top, _pBtn->Height, _pBtn->Width, Arc,  BTN_BORDER_COLOR2);	
			LCD_Fill_Rect(_pBtn->Left + Arc, _pBtn->Top + 1, _pBtn->Height / 2, _pBtn->Width - 2 * Arc, BTN_SHADOW_COLOR2);	/* ����Ӱ�Ա�ɫ */
		}

		LCD_DispStrEx(x, y, _pBtn->pCaption, _pBtn->Font, _pBtn->Width, ALIGN_CENTER);
	}
}

/*
*********************************************************************************************************
*	�� �� ��: LCD_ButtonTouchDown
*	����˵��: �жϰ�ť�Ƿ񱻰���. ��鴥�������Ƿ��ڰ�ť�ķ�Χ֮�ڡ����ػ水ť��
*	��    ��:  _btn : ��ť����
*			  _usX, _usY: ��������
*	�� �� ֵ: 1 ��ʾ�ڷ�Χ��
*********************************************************************************************************
*/
uint8_t LCD_ButtonTouchDown(BUTTON_T *_btn, uint16_t _usX, uint16_t _usY)
{
	if ((_usX > _btn->Left) && (_usX < _btn->Left + _btn->Width)
		&& (_usY > _btn->Top) && (_usY < _btn->Top + _btn->Height))
	{
		BUTTON_BEEP();	/* ������ʾ�� bsp_tft_lcd.h �ļ���ͷ����ʹ�ܺ͹ر� */
		_btn->Focus = 1;
		LCD_DrawButton(_btn);
		return 1;
	}
	else
	{
		return 0;
	}
}

/*
*********************************************************************************************************
*	�� �� ��: LCD_ButtonTouchRelease
*	����˵��: �жϰ�ť�Ƿ񱻴����ͷ�. ���ػ水ť���ڴ����ͷ��¼��б����á�
*	��    ��:  _btn : ��ť����
*			  _usX, _usY: ��������
*	�� �� ֵ: 1 ��ʾ�ڷ�Χ��
*********************************************************************************************************
*/
uint8_t LCD_ButtonTouchRelease(BUTTON_T *_btn, uint16_t _usX, uint16_t _usY)
{
	/* 2016-04-24 �������� */
	if (_btn->Focus != 0)
	{
		_btn->Focus = 0;
		LCD_DrawButton(_btn);
	}
	
	if ((_usX > _btn->Left) && (_usX < _btn->Left + _btn->Width)
		&& (_usY > _btn->Top) && (_usY < _btn->Top + _btn->Height))
	{

		return 1;
	}
	else
	{
		return 0;
	}
}

/*
*********************************************************************************************************
*	�� �� ��: LCD_DrawBmpButton
*	����˵��: ��LCD�ϻ���һ��ͼƬ��ť
*	��    ��:
*			_usX, _usY : ͼƬ������
*			_usHeight  : ͼƬ�߶�
*			_usWidth   : ͼƬ���
*			_ptr       : ͼƬ����ָ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void LCD_DrawBmpButton(BMP_BUTTON_T *_pBtn)
{
	if (_pBtn->Focus == 1)
	{
		RA8875_DispBmpInFlash(_pBtn->Left, _pBtn->Top, _pBtn->Height, _pBtn->Width, _pBtn->Pic2);
	}
	else
	{
		RA8875_DispBmpInFlash(_pBtn->Left, _pBtn->Top, _pBtn->Height, _pBtn->Width, _pBtn->Pic1);
	}
}

/*
*********************************************************************************************************
*	�� �� ��: LCD_BmpButtonTouchDown
*	����˵��: �ж�ͼƬ��ť��ť�Ƿ񱻰���. ��鴥�������Ƿ��ڰ�ť�ķ�Χ֮�ڡ����ػ水ť��
*	��    ��:  _btn : ��ť����
*			  _usX, _usY: ��������
*	�� �� ֵ: 1 ��ʾ�ڷ�Χ��
*********************************************************************************************************
*/
uint8_t LCD_BmpButtonTouchDown(BMP_BUTTON_T *_btn, uint16_t _usX, uint16_t _usY)
{
	if ((_usX > _btn->Left) && (_usX < _btn->Left + _btn->Width)
		&& (_usY > _btn->Top) && (_usY < _btn->Top + _btn->Height))
	{
		BUTTON_BEEP();	/* ������ʾ�� bsp_tft_lcd.h �ļ���ͷ����ʹ�ܺ͹ر� */
		_btn->Focus = 1;
		LCD_DrawBmpButton(_btn);
		return 1;
	}
	else
	{
		return 0;
	}
}

/*
*********************************************************************************************************
*	�� �� ��: LCD_BmpButtonTouchRelease
*	����˵��: �ж�ͼƬ��ť�Ƿ񱻴����ͷ�. ���ػ水ť���ڴ����ͷ��¼��б����á�
*	��    ��:  _btn : ��ť����
*			  _usX, _usY: ��������
*	�� �� ֵ: 1 ��ʾ�ڷ�Χ��
*********************************************************************************************************
*/
uint8_t LCD_BmpButtonTouchRelease(BMP_BUTTON_T *_btn, uint16_t _usX, uint16_t _usY)
{
	_btn->Focus = 0;
	LCD_DrawBmpButton(_btn);
	
	if ((_usX > _btn->Left) && (_usX < _btn->Left + _btn->Width)
		&& (_usY > _btn->Top) && (_usY < _btn->Top + _btn->Height))
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

/*
*********************************************************************************************************
*	�� �� ��: LCD_SelectTouchDown
*	����˵��: ѡ��ҪУ׼�Ĳ���
*	��    ��:  _slt : ѡ�ж���
*			  _usX, _usY: ��������
*	�� �� ֵ: 1 ��ʾ�ڷ�Χ��
*********************************************************************************************************
*/
uint8_t LCD_SelectTouchDown(SELECT_T *_slt, uint16_t _usX, uint16_t _usY)
{
	if ((_usX > _slt->Left) && (_usX < _slt->Left + _slt->Width)
		&& (_usY > _slt->Top) && (_usY < _slt->Top + _slt->Height))
	{
		BUTTON_BEEP();
		return 1;
	}
	else
	{
		return 0;
	}
}

/*
*********************************************************************************************************
*	�� �� ��: LCD_InitPannel
*	����˵��: ��ʼ�����ṹ���Ա��
*	��    ��: _panl : pannel ����
*			  _x, _y : ����
*			  _h, _w : �߶ȺͿ��
*			  _pCaption : ��ť����
*			  _pFont : ��ť����
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void LCD_InitPannel(PANNEL_T *_pnl, uint16_t _x, uint16_t _y, uint16_t _h, uint16_t _w, uint16_t _arc, uint16_t _color)
{
	_pnl->Left = _x;
	_pnl->Top = _y;
	_pnl->Height = _h;
	_pnl->Width = _w;
	_pnl->Arc = _arc;	
	_pnl->Color = _color;
}

/*
*********************************************************************************************************
*	�� �� ��: LCD_DrawPannel
*	����˵��: ��LCD�ϻ���һ�����
*	��    ��: 
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void LCD_DrawPannel(PANNEL_T *_pnl)
{
	if (g_ChipID == IC_8875)
	{
		/* ����һ��Բ�Ǿ��Σ�����ɫ */
		RA8875_FillRoundRect(_pnl->Left, _pnl->Top, _pnl->Height, _pnl->Width, _pnl->Arc, _pnl->Color);
	}
	else
	{
		;	
	}
}

/*
*********************************************************************************************************
*	�� �� ��: LCD_PannelClick
*	����˵��: �ж�Pannel�Ƿ񱻵��. ��鴥�������Ƿ��ڰ�ť�ķ�Χ֮�ڡ�
*	��    ��:  _obj : PANNEL����
*			  _usX, _usY: ��������
*	�� �� ֵ: 1 ��ʾ�ڷ�Χ�� 0��ʾ����
*********************************************************************************************************
*/
uint8_t LCD_PannelClick(PANNEL_T *_obj, uint16_t _usX, uint16_t _usY)
{
	if ((_usX > _obj->Left) && (_usX < _obj->Left + _obj->Width)
		&& (_usY > _obj->Top) && (_usY < _obj->Top + _obj->Height))
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

/*
*********************************************************************************************************
*	�� �� ��: LCD_LabelClick
*	����˵��: �ж�Label�Ƿ񱻵��. ��鴥�������Ƿ��ڰ�ť�ķ�Χ֮�ڡ�
*	��    ��:  _obj : PANNEL����
*			  _usX, _usY: ��������
*	�� �� ֵ: 1 ��ʾ�ڷ�Χ�� 0��ʾ����
*********************************************************************************************************
*/
uint8_t LCD_LabelClick(LABEL_T *_obj, uint16_t _usX, uint16_t _usY)
{
	if ((_usX > _obj->Left) && (_usX < _obj->Left + _obj->Width)
		&& (_usY > _obj->Top) && (_usY < _obj->Top + _obj->Height))
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

/*
*********************************************************************************************************
*	�� �� ��: LCD_DrawArc
*	����˵��: ����һ��Բ�����ʿ�Ϊ1������
*	��    ��:
*			_usX,_usY  ��Բ�ĵ�����
*			_usRadius  ��Բ�İ뾶
*			_StartAng  : ��ʼ�Ƕ�
*			_EndAng	   : ��ֹ�Ƕ�
*			_usColor   : Բ����ɫ
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void LCD_DrawArc(uint16_t _usX, uint16_t _usY, uint16_t _usRadius, float _StartRnd, float _EndRnd, uint16_t _usColor)
{
	float CurX,CurY,rnd;
	
	rnd = _StartRnd;    
	while (rnd <= _EndRnd)        
	{
		CurX = _usRadius * cos(rnd);         
		CurY = _usRadius * sin(rnd);          
		LCD_PutPixel(_usX + (uint16_t)CurX,_usY - (uint16_t)CurY, _usColor);            
		rnd = rnd + 0.01f;         
	} 
}

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/

