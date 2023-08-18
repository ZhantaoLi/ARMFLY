/*
*********************************************************************************************************
*
*	ģ������ : TFTҺ����ʾ������ģ��
*	�ļ����� : LCD_tft_lcd.h
*	��    �� : V2.0
*	˵    �� : ͷ�ļ�
*
*	Copyright (C), 2010-2011, ���������� www.armfly.com
*
*********************************************************************************************************
*/


#ifndef _BSP_TFT_LCD_H
#define _BSP_TFT_LCD_H

#define BUTTON_BEEP()	BEEP_KeyTone();	/* ������ʾ�� */
//#define BUTTON_BEEP()	/* �ް�����ʾ�� */

/*
	LCD ��ɫ���룬CL_��Color�ļ�д
	16Bit�ɸ�λ����λ�� RRRR RGGG GGGB BBBB

	�����RGB �꽫24λ��RGBֵת��Ϊ16λ��ʽ��
	����windows�Ļ��ʳ��򣬵���༭��ɫ��ѡ���Զ�����ɫ�����Ի�õ�RGBֵ��

	�Ƽ�ʹ������ȡɫ���������㿴���Ľ�����ɫ��
*/
#define RGB(R,G,B)	(((R >> 3) << 11) | ((G >> 2) << 5) | (B >> 3))	/* ��8λR,G,Bת��Ϊ 16λRGB565��ʽ */
#define RGB565_R(x)  ((x >> 8) & 0xF8)
#define RGB565_G(x)  ((x >> 3) & 0xFC)
#define RGB565_B(x)  ((x << 3) & 0xF8)
enum
{
	CL_WHITE        = RGB(255,255,255),	/* ��ɫ */
	CL_BLACK        = RGB(  0,  0,  0),	/* ��ɫ */
	CL_RED          = RGB(255,	0,  0),	/* ��ɫ */
	CL_GREEN        = RGB(  0,255,  0),	/* ��ɫ */
	CL_BLUE         = RGB(  0,	0,255),	/* ��ɫ */
	CL_YELLOW       = RGB(255,255,  0),	/* ��ɫ */

	CL_GREY			= RGB( 98, 98, 98), 	/* ���ɫ */
	CL_GREY1		= RGB( 150, 150, 150), 	/* ǳ��ɫ */
	CL_GREY2		= RGB( 180, 180, 180), 	/* ǳ��ɫ */
	CL_GREY3		= RGB( 200, 200, 200), 	/* ��ǳ��ɫ */
	CL_GREY4		= RGB( 230, 230, 230), 	/* ��ǳ��ɫ */

	CL_BUTTON_GREY	= RGB( 220, 220, 220), /* WINDOWS ��ť�����ɫ */

	CL_MAGENTA      = 0xF81F,	/* ����ɫ�����ɫ */
	CL_CYAN         = 0x7FFF,	/* ����ɫ����ɫ */

	CL_BLUE1        = RGB(  0,  0, 240),		/* ����ɫ */
	CL_BLUE2        = RGB(  0,  0, 128),		/* ����ɫ */
	CL_BLUE3        = RGB(  68, 68, 255),		/* ǳ��ɫ1 */
	CL_BLUE4        = RGB(  0, 64, 128),		/* ǳ��ɫ1 */

	/* UI ���� Windows�ؼ�����ɫ */
	CL_BTN_FACE		= RGB(236, 233, 216),	/* ��ť������ɫ(��) */
	
	CL_BTN_FONT		= CL_BLACK,				/* ��ť������ɫ���ڣ� */
	
	CL_BOX_BORDER1	= RGB(172, 168,153),	/* �����������ɫ */
	CL_BOX_BORDER2	= RGB(255, 255,255),	/* �������Ӱ����ɫ */


	CL_MASK			= 0x9999,	/* ��ɫ���룬�������ֱ���͸�� */
	
	
	
	/* ���� */
	CL_BLUE5        = RGB(70, 130, 180),		/* ��ɫ5(��ɫ) */
	CL_BLUE6        = RGB(0, 255, 255),		/* ��ɫ6(���ο�ɫ) */
	
	CL_GREY5		= RGB( 210, 220, 220), 	/* ��ť�� */
	CL_GREY6		= RGB( 224, 238, 238), 	/* ��ť�� */
	
	/* ������ʾ */
	CL_BLUE7		= RGB( 200, 255, 255), 	/* ��ť�� */
	CL_BLUE8		= RGB( 191, 238, 255), 	/* ��ť�� */
	CL_YELLOW2		= RGB( 255, 246, 143),  /* ����ɫ */
};

/* ���ֶ��뷽ʽ */
enum
{
	ALIGN_LEFT = 0,
	ALIGN_CENTER = 1,
	ALIGN_RIGHT = 2
};

/* �༭���� */
enum
{
	EDIT_BORDER_COLOR		= CL_BLUE2,		/* �༭���ĸ��ߵ���ɫ��δѡ��ʱ */
	EDIT_BORDER_COLOR2		= CL_BLUE3,		/* �༭���ĸ��ߵ���ɫ,ѡ��ʱ */
	EDIT_BACK_COLOR			= RGB(237, 125, 49),		/* �༭�򱳾���δѡ��ʱ */
	EDIT_BACK_COLOR2		= RGB(255, 192, 0),		/* �༭�򱳾���ɫ��ѡ��ʱ */
};

/* ��ť��� */
enum
{
	/* δ����ʱ */
	BTN_BORDER_COLOR1	= RGB(172, 168,153),	/* ��ť�߿����ɫ */
	BTN_BODY_COLOR1		= RGB(200, 210, 210),	/* ��ť��ɫ */
	BTN_SHADOW_COLOR1	= RGB(220, 230, 230),	/* ��ť�ϰ벿��Ӱɫ */

	/* ����ʱ */
	BTN_BORDER_COLOR2	= RGB(172, 200,153),	/* ����ʱ��ť�߿����ɫ */
	BTN_BODY_COLOR2		= RGB(191, 238, 255),	/* ����ʱ��ť��ɫ */
	BTN_SHADOW_COLOR2	= RGB(200, 255, 255),	/* ����ʱ��ť�ϰ벿��Ӱɫ */	
};

/* ���ڷ�� */
enum
{
	WIN_BORDER_COLOR	= CL_BLUE4,		/* ���ڱ߿� */
	WIN_TITLE_COLOR		= RGB(70, 130, 180),	/* ���ڱ�����������ɫ */  
	WIN_CAPTION_COLOR	= CL_WHITE,		/* ���ڱ�����������ɫ */
	WIN_BODY_COLOR		= RGB( 200, 255, 255),		/* ǳ��  ������ɫ */
};

/* ������ */
enum
{
	CHECK_BOX_BORDER_COLOR	= CL_BLUE2,		/* �����ĸ��ߵ���ɫ */
	CHECK_BOX_BACK_COLOR	= CL_GREY3,		/* ���򱳾� */
	CHECK_BOX_CHECKED_COLOR	= CL_RED,		/* ����򹴵���ɫ */

	CHECK_BOX_H			= 24,				/* ����߶� */
	CHECK_BOX_W			= 24,				/* ����߶� */
};

/* ������� */
typedef enum
{
	FC_ST_12 = 0,		/* ����12x12���� ����x�ߣ� */
	FC_ST_16,			/* ����15x16���� ����x�ߣ� */
	FC_ST_24,			/* ����24x24���� ����x�ߣ� -- ��ʱδ֧�� */
	FC_ST_32,			/* ����32x32���� ����x�ߣ� -- ��ʱδ֧�� */	

	FC_ST_62X40,		/* 64x40����(��62����40)��ASCII�ַ� */
	FC_ST_96X40,		/* 96x40����(��96����40)��ASCII�ַ� */
	
	FC_RA8875_16,		/* RA8875 �������� 16���� */
	FC_RA8875_24,		/* RA8875 �������� 24���� */
	FC_RA8875_32		/* RA8875 �������� 32���� */	
}FONT_CODE_E;

/* �������Խṹ, ����LCD_DispStr() */
typedef struct
{
	FONT_CODE_E FontCode;	/* ������� FONT_CODE_E  */
	uint16_t FrontColor;/* ������ɫ */
	uint16_t BackColor;	/* ���ֱ�����ɫ��͸�� */
	uint16_t Space;		/* ���ּ�࣬��λ = ���� */
}FONT_T;

/* �ؼ�ID */
typedef enum
{
	ID_ICON		= 1,
	ID_WIN		= 2,
	ID_LABEL	= 3,
	ID_BUTTON	= 4,
	ID_CHECK 	= 5,
	ID_EDIT 	= 6,
	ID_GROUP 	= 7,
}CONTROL_ID_T;

/* ͼ��ṹ */
typedef struct
{
	uint8_t id;
	uint16_t Left;		/* ���Ͻ�X���� */
	uint16_t Top;		/* ���Ͻ�Y���� */
	uint16_t Height;	/* ͼ��߶� */
	uint16_t Width;		/* ͼ���� */
	uint16_t *pBmp;		/* ָ��ͼ��ͼƬ���� */
	char  Text[16];	/* ͼ���ı�, �����ʾ5������16���� */
}ICON_T;

/* ����ṹ */
typedef struct
{
	uint8_t id;
	uint16_t Left;
	uint16_t Top;
	uint16_t Height;
	uint16_t Width;
	uint16_t Color;
	FONT_T *Font;
	char *pCaption;
}WIN_T;

/* �ı���ǩ�ṹ */
typedef struct
{
	uint8_t id;
	uint16_t Left;			/* ���Ͻ�X���� */
	uint16_t Top;			/* ���Ͻ�Y���� */
	uint16_t Height;		/* �߶� */
	uint16_t Width;			/* ��� */
	uint16_t MaxLen;		/* �ַ������� */
	FONT_T *Font;			/* ���� */
	char  *pCaption;
}LABEL_T;

/* ��ť�ṹ */
typedef struct
{
	uint8_t id;
	uint8_t Style;		/* ��ť���0=��ͨ��ť�� 1=Բ�ǰ�ť�� 2=ͼƬ��ť */
	uint16_t Left;
	uint16_t Top;
	uint16_t Height;
	uint16_t Width;
	/* ��ť����ɫ���ɵײ��Զ����� */
	FONT_T *Font;			/* ���� */
	char *pCaption;
	uint8_t Focus;			/* ���� */
}BUTTON_T;

/* ��ť��������ı������ṹ */
typedef struct
{
	uint16_t x;
	uint16_t y;
	uint16_t h;
	uint16_t w;
	char *text;		/* ��ť������ */
}BTN_PARAM_T;

/* ͼƬ��ť�ṹ */
typedef struct
{
	uint8_t id;
	uint16_t Left;
	uint16_t Top;
	uint16_t Height;
	uint16_t Width;
	uint32_t Pic1;
	uint32_t Pic2;	
	uint8_t Focus;			/* ���� */
}BMP_BUTTON_T;

/* ѡ�нṹ */
typedef struct
{
	uint16_t Left;
	uint16_t Top;
	uint16_t Height;
	uint16_t Width;
	
}SELECT_T;

/* �༭��ṹ */
typedef struct
{
	uint8_t id;
	uint16_t Left;
	uint16_t Top;
	uint16_t Height;
	uint16_t Width;
	uint16_t Color;
	FONT_T *Font;			/* ���� */
	char   *pCaption;
	uint8_t Focus;
	char Text[32+1];		/* ����༭���ڵ�ֵ */
}EDIT_T;

/* ���� CHECK BOX �ṹ */
typedef struct
{
	uint8_t id;
	uint16_t Left;			/* ���Ͻ�X���� */
	uint16_t Top;			/* ���Ͻ�Y���� */
	uint16_t Height;		/* �߶� */
	uint16_t Width;			/* ��� */
	uint16_t Color;			/* ��ɫ */
	FONT_T *Font;			/* ���� */
	char  *pCaption;
	uint8_t Checked;		/* 1��ʾ�� */
}CHECK_T;

/* �����GROUP BOX �ṹ */
typedef struct
{
	uint8_t id;
	uint16_t Left;			/* ���Ͻ�X���� */
	uint16_t Top;			/* ���Ͻ�Y���� */
	uint16_t Height;		/* �߶� */
	uint16_t Width;			/* ��� */
	FONT_T *Font;			/* ���� */
	char  *pCaption;
}GROUP_T;

/* Pannel���ṹ */
typedef struct
{
	uint8_t id;
	uint16_t Left;			/* ���Ͻ�X���� */
	uint16_t Top;			/* ���Ͻ�Y���� */
	uint16_t Height;		/* �߶� */
	uint16_t Width;			/* ��� */
	uint16_t Arc;			/* Բ�ǻ��뾶 */
	uint16_t Color;			/* �����ɫ */	
}PANNEL_T;

/* ��������� */
#define BRIGHT_MAX		255
#define BRIGHT_MIN		0
#define BRIGHT_DEFAULT	200
#define BRIGHT_STEP		5

/* �ɹ��ⲿģ����õĺ��� */
void LCD_DispStr(uint16_t _usX, uint16_t _usY, char *_ptr, FONT_T *_tFont);
void LCD_DrawPoints(uint16_t *x, uint16_t *y, uint16_t _usSize, uint16_t _usColor);

void LCD_DrawWin(WIN_T *_pWin);
void LCD_DrawIcon(const ICON_T *_tIcon, FONT_T *_tFont, uint8_t _ucFocusMode);

void LCD_DrawEdit(EDIT_T *_pEdit);
uint8_t LCD_EditTouchDown(EDIT_T *_Edit, uint16_t _usX, uint16_t _usY);


void LCD_DrawButton(BUTTON_T *_pBtn);
void LCD_DrawBmpButton(BMP_BUTTON_T *_pBtn);
void LCD_DrawLabel(LABEL_T *_pLabel);
void LCD_DrawCheckBox(CHECK_T *_pCheckBox);

void LCD_InitGroupBox(GROUP_T *_pBox, uint16_t _x, uint16_t _y, uint16_t _h, uint16_t _w,
	char  *pCaption, FONT_T *Font);
void LCD_DrawGroupBox(GROUP_T *_pBox);

void LCD_DispControl(void *_pControl);

void LCD_DrawIcon32(const ICON_T *_tIcon, FONT_T *_tFont, uint8_t _ucFocusMode);
void LCD_DrawBmp32(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth, uint8_t *_pBmp);

uint16_t LCD_GetFontWidth(FONT_T *_tFont);
uint16_t LCD_GetFontHeight(FONT_T *_tFont);
uint16_t LCD_GetStrWidth(char *_ptr, FONT_T *_tFont);
void LCD_DispStrEx(uint16_t _usX, uint16_t _usY, char *_ptr, FONT_T *_tFont, uint16_t _Width,
	uint8_t _Align);

uint8_t LCD_ButtonTouchDown(BUTTON_T *_btn, uint16_t _usX, uint16_t _usY);
uint8_t LCD_ButtonTouchRelease(BUTTON_T *_btn, uint16_t _usX, uint16_t _usY);
void LCD_InitButton(BUTTON_T *_btn, uint16_t _x, uint16_t _y, uint16_t _h, uint16_t _w, char *_pCaption, FONT_T *_pFont);

/* ���� */
void LCD_DrawLabel2(LABEL_T *_pLabel, uint16_t _Color, uint8_t _Arc);
void LCD_DrawLabel3(LABEL_T *_pLabel, uint16_t _Color, uint8_t _Arc);
void LCD_DrawLabel4(LABEL_T *_pLabel);
void LCD_DrawLabel5(LABEL_T *_pLabel, uint16_t _Color, uint8_t _Arc);
void LCD_DrawLabel6(LABEL_T *_pLabel, uint16_t _Color);

uint8_t LCD_SelectTouchDown(SELECT_T *_slt, uint16_t _usX, uint16_t _usY);

void LCD_InitPannel(PANNEL_T *_pnl, uint16_t _x, uint16_t _y, uint16_t _h, uint16_t _w, uint16_t _arc, uint16_t _color);
void LCD_DrawPannel(PANNEL_T *_pnl);

void LCD_InitLabel(LABEL_T *_pLabel, uint16_t _x, uint16_t _y, uint16_t _h, uint16_t _w, 
	char *_Text, FONT_T *_tFont);
	
void LCD_InitEdit(EDIT_T *_pEdit, uint16_t _x, uint16_t _y, uint16_t _h, uint16_t _w,
	char  *pCaption, FONT_T *Font);	
uint8_t LCD_EditTouchDown(EDIT_T *_Edit, uint16_t _usX, uint16_t _usY);
void LCD_EditRefresh(EDIT_T *_Edit);

uint8_t LCD_PannelClick(PANNEL_T *_obj, uint16_t _usX, uint16_t _usY);
uint8_t LCD_LabelClick(LABEL_T *_obj, uint16_t _usX, uint16_t _usY);

void LCD_FillCircle(uint16_t _usX, uint16_t _usY, uint16_t _usRadius, uint16_t _usColor);

void LCD_BeginDrawAll(void);
void LCD_EndDrawAll(uint8_t _slide);
void LCD_BeginDrawWin(uint16_t _usColor);
void LCD_EndDrawWin(uint16_t _usX, uint16_t _usY, uint16_t _Width, uint16_t _Height);
void LCD_PIP_Start(uint8_t _ucSelect, uint16_t _usX, uint16_t _usY, uint16_t _usWidth, uint16_t _usHeight);

void LCD_DrawArc(uint16_t _usX, uint16_t _usY, uint16_t _usRadius, float _StartRnd, float _EndRnd, uint16_t _usColor);

#endif


