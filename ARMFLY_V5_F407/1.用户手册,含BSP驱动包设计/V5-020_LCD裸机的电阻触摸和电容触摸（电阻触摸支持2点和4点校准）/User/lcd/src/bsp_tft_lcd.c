/*
*********************************************************************************************************
*
*	模块名称 : TFT液晶显示器驱动模块
*	文件名称 : bsp_tft_lcd.c
*	版    本 : V4.8
*	说    明 : 	支持3.0， 3.5， 4.3， 5.0， 7.0寸显示模块.
*			  	3.0寸的支持的LCD内部驱动芯片型号有: SPFD5420A、OTM4001A、R61509V
*				3.5寸的LCD内置驱动ILI9488， 4.3以上的是RA8875驱动
*	修改记录 :
*		版本号  日期       作者    说明
*		v1.0    2011-08-21 armfly  ST固件库版本 V3.5.0版本。
*					a) 取消访问寄存器的结构体，直接定义
*		V2.0    2011-10-16 armfly  增加R61509V驱动，实现图标显示函数
*		V2.1    2012-07-06 armfly  增加RA8875驱动，支持4.3寸屏
*		V2.2    2012-07-13 armfly  改进LCD_DispStr函数，支持12点阵字符;修改LCD_DrawRect,解决差一个像素问题
*		V2.3    2012-08-08 armfly  将底层芯片寄存器操作相关的函数放到单独的文件，支持RA8875
*   	V3.0    2013-05-20 增加图标结构; 修改	LCD_DrawIconActive  修改DispStr函数支持文本透明
*		V3.1    2013-06-12 解决LCD_DispStr()函数BUG，如果内嵌字库中汉字个数多于256，则出现死循环。
*		V3.2    2013-06-28 完善Label控件, 当显示字符串比之前短时，自动清除多余的文字
*		V3.3    2013-06-29 FSMC初始化时，配置时序，写时序和读时序分开设置。 LCD_FSMCConfig 函数。
*		V3.4    2013-07-06 增加显示32位带Alpha图标的函数 LCD_DrawIcon32
*		V3.5    2013-07-24 增加显示32位带Alpha图片的函数 LCD_DrawBmp32
*		V3.6    2013-07-30 修改 DispEdit() 支持12点阵汉字对齐
*		V3.7    2014-09-06 修改 LCD_InitHard() 同时支持 RA8875-SPI接口和8080接口
*		V3.8    2014-09-15 增加若干函数:
*					（1） LCD_DispStrEx() 可以自动对齐自动填白的显示字符串函数
*					（2） LCD_GetStrWidth() 计算字符串的像素宽度
*		V3.9    2014-10-18
*					(1) 增加 LCD_ButtonTouchDown() LCD_ButtonTouchRelease 判断触摸坐标并重绘按钮
*					(2) 增加3.5寸LCD驱动
*					(3) 增加 LCD_SetDirection() 函数，设置显示屏方向（横屏 竖屏动态切换）
*		V4.0   2015-04-04 
*				(1) 按钮、编辑框控件增加RA8875字体，内嵌字库和RA8875字库统一编码。字体代码增加 
*				    FC_RA8875_16, FC_RA8875_24,	FC_RA8875_32
*				(2) FONT_T结构体成员FontCode的类型由 uint16_t 修改为 FONT_CODE_E枚举，便于编译器查错;
*				(3) 修改 LCD_DispStrEx(), 将读点阵的语句独立到函数：_LCD_ReadAsciiDot(), _LCD_ReadHZDot()
*				(4) LCD_DispStr() 函数简化，直接调用 LCD_DispStrEx() 实现。
*				(5) LCD_DispStrEx() 函数支持 RA8875字体。
*				(6) LCD_ButtonTouchDown() 增加按键提示音
*		V4.1   2015-04-18 
*				(1) 添加RA885 ASCII字体的宽度表。LCD_DispStrEx() 函数可以支持RA8875 ASCII变长宽度计算。
*				(2) 添加 LCD_HardReset(）函数，支持LCD复位由GPIO控制的产品。STM32-V5 不需要GPIO控制。
*		V4.2   2015-07-23
*				(1) 添加函数LCD_InitButton()
*				(2) h文件中使能按键提示音 #define BUTTON_BEEP()	BEEP_KeyTone();
*		V4.3   2016-01-30 
*				(1) 将LCD底层硬件访问函数独立出去，放到 bsp_tft_port.c 文件。
*				(2) 增加结构体 BTN_PARAM_T， 方便主程序使用for循环方式初始化按钮的坐标等参数
*				(3) 按钮的结构体，增加按钮风格成员变量
*		V4.4	2016-02-03
*				(1) 新增圆角矩形控件，TPannel
*				(2) 增加函数 LCD_InitGroupBox();
*				(3) LCD_DispStrEx(). 添加支持RA8875字体背景透明， 新增 RA8875_SetTextTransp（）
*				(4) LCD_DrawGroupBox(), 支持文字透明模式，顶部横线分2段绘制
*				(5) 增加 LCD_InitLabel()函数
*				(6) 增加 LCD_InitEdit() 函数
*				(7) 增加 LCD_PannelClick(), LCD_LabelClick()函数
*		V4.5	2016-04-22
*				(1) 修改 LCD_DispStrEx函数，支持内嵌24,32点阵字符和汉字，读自摸函数独立出来，支持SPI Flash
*		V4.6	2016-04-24
*				(1) 修改 LCD_ButtonTouchRelease() ，避免触摸抬起时按钮轻微闪屏现象。
*				(2)	修改 控件的属性
*		V4.7    2016-07-02
*				增加4个RA8876特效的相关函数
*					整个界面特效。左滑、右滑、无刷屏感
*						LCD_BeginDrawAll()
*						LCD_EndDrawAll()
*					消除窗口刷屏感
*						LCD_BeginDrawWin()		
*						LCD_EndDrawWin()
*		V4.8	2017-06-17
*				(1) 支持按钮文字换行显示.
*		V4.9	2018-03-10 
*				(1) 增加 62*40 ASCII点阵0-9
*				(2) 增加 96*40 ASCII点阵0-9
*				(3) 修改 _LCD_ReadHZDot()函数，支持字定义汉字图形
*
*	Copyright (C), 2015-2020, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

/*
	移植事项
	1. 控制PWM背光的函数需要更改。  LCD_SetPwmBackLight
	2. FSMC配置的片选需要更改	LCD_FSMCConfig
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
*	函 数 名: LCD_BeginDrawAll
*	功能说明: 开始特效画整个界面时，将内存地址指向第3块内存。
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD_BeginDrawAll(void)
{
	uint16_t width;
	
	if (g_LcdDirection > 1)			/* 竖屏 */
	{
		width = g_LcdHeight;
	}
	else							/* 横屏 */
	{
		width = g_LcdWidth;
	}
    
    (void)width;
	
	if (g_ChipID == IC_8876)
	{
		#if 0
		Canvas_Image_Start_address(width * 2 * 2);		/* 先将数据写到第3个内存中，写完后，在将数据复制到第1或第2块内存中 */
		g_Drawing = 1;				/* 特效画图 */
		#endif
	}
	else
	{
		;
	}
}

/*
*********************************************************************************************************
*	函 数 名: LCD_EndDrawAll
*	功能说明: 画窗口结束。支持3种类特效
*	形    参: _slide ：0-2   0表示不滑动但无刷屏感 1表示左滑动 2表示右滑动
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD_EndDrawAll(uint8_t _slide)
{
	uint16_t width, height;	
		
	
	if (g_LcdDirection > 1)			/* 竖屏 */
	{
		width = g_LcdHeight;
		height = g_LcdWidth;
	}
	else							/* 横屏 */
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
		
		if (_slide == 0)			/* 不滑动 */
		{
			SAddr = (width * 2) * 2;				/* 源地址（第3块内存） */
			DAddr = (width * 2) * g_Interface;		/* 目的地址 */
			
			/* 将第3块内存的数据复制到主界面内存（第1块） */
			BTE_Memory_Copy(SAddr, CanvasWidth, 0, 0, SAddr, CanvasWidth, 0, 0, DAddr,CanvasWidth,0,0,12, width, height);
			
			Canvas_Image_Start_address(DAddr);				/* 将内存写入地址还原到从界面起始地址 */
		}
		else if (_slide == 1)		/* 左滑动 */
		{
			if (g_Interface == 0)					/* 当前显示第1块内存 */
			{
				SAddr = 0;							/* 源地址（第1块内存） */
				DAddr = (width * 2) * 1;			/* 目的地址（第2块内存） */
				
				/* 将第1块内存的数据复制到第2块 */
				BTE_Memory_Copy(SAddr, CanvasWidth, 0, 0, SAddr, CanvasWidth, 0, 0, DAddr,CanvasWidth,0,0,12, width, height);
				Main_Image_Start_Address(DAddr);			/* 将显存指向第2块内存地址 */
				
				SAddr = (width * 2) * 2;			/* 源地址（第3块内存） */
				DAddr = 0;							/* 目的地址（第1块内存） */
				BTE_Memory_Copy(SAddr, CanvasWidth, 0, 0, SAddr, CanvasWidth, 0, 0, DAddr,CanvasWidth,0,0,12, width, height);
			}
			else											/* 当前显示第2块内存 */
			{
				SAddr = (width * 2) * 2;			/* 源地址（第3块内存） */
				DAddr = 0;							/* 目的地址（第1块内存） */
				BTE_Memory_Copy(SAddr, CanvasWidth, 0, 0, SAddr, CanvasWidth, 0, 0, DAddr,CanvasWidth,0,0,12, width, height);
			}
			
			/* 开始滑动，第1块内存 <- 第2块内存 */
			for (i = width; i > 0; i--)
			{
				Main_Image_Start_Address(i * 2);
				bsp_DelayUS(500);
			}
			
			Canvas_Image_Start_address(DAddr);				/* 将内存写入地址还原到主界面起始地址 */
			g_Interface = 0;		/* 最终的显示内存为第1块内存 */
		}
		else if (_slide == 2)		/* 右滑动 */
		{
			if (g_Interface == 0)					/* 当前显示第1块内存 */
			{
				SAddr = (width * 2) * 2;			/* 源地址（第3块内存） */
				DAddr = (width * 2) * 1;			/* 目的地址（第2块内存） */
				BTE_Memory_Copy(SAddr, CanvasWidth, 0, 0, SAddr, CanvasWidth, 0, 0, DAddr,CanvasWidth,0,0,12, width, height);
			}
			else									/* 当前显示第2块内存 */
			{
				SAddr = (width * 2) * 1;			/* 源地址（第2块内存） */
				DAddr = 0;							/* 目的地址（第1块内存） */
				BTE_Memory_Copy(SAddr, CanvasWidth, 0, 0, SAddr, CanvasWidth, 0, 0, DAddr,CanvasWidth,0,0,12, width, height);
		
				Main_Image_Start_Address(DAddr);			/* 将显存指向第1块内存地址 */
				
				SAddr = (width * 2) * 2;			/* 源地址（第3块内存） */
				DAddr = (width * 2) * 1;			/* 目的地址（第2块内存） */
				BTE_Memory_Copy(SAddr, CanvasWidth, 0, 0, SAddr, CanvasWidth, 0, 0, DAddr,CanvasWidth,0,0,12, width, height);
			}
			
			/* 开始滑动，第1块内存 -> 第2块内存 */
			for (i = 0; i <= width; i++)
			{
				Main_Image_Start_Address(i * 2);			/* 主界面到从界面 */
				bsp_DelayUS(500);
			}
			
			Canvas_Image_Start_address(DAddr);				/* 将内存写入地址还原到从界面起始地址 */
			g_Interface = 1;		/* 最终的显示内存为第2块内存 */
		}
		g_Drawing = 0; 			/* 特效画图结束 */
		#endif
	}
	else
	{
		;
	}
}

/*
*********************************************************************************************************
*	函 数 名: LCD_BeginDrawWin
*	功能说明: 画整部分窗口。将画的区域复制到显示内存块
*	形    参: _usColor : 窗口底色
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD_BeginDrawWin(uint16_t _usColor)
{
	uint16_t width, height;
	
	
	if (g_LcdDirection > 1)			/* 竖屏 */
	{
		width = g_LcdHeight;
		height = g_LcdWidth;
	}
	else							/* 横屏 */
	{
		width = g_LcdWidth;
		height = g_LcdHeight;
	}
    
    
	
	if (g_ChipID == IC_8876)
	{
	#if 0
        uint32_t MemCopyAddr;
		MemCopyAddr = (width * 2) * 2;			/* 第三块内存起始地址 */
		Canvas_Image_Start_address(MemCopyAddr);			/* 将内存操作起始地址设置为第三个内存 */
		BTE_Solid_Fill(0, CanvasWidth, width * 2, 0, _usColor, width, height); /* 将第3块内存清屏 */	
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
*	函 数 名: LCD_EndDrawWin
*	功能说明: 画整部分窗口结束。将画的区域复制到显示内存块
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD_EndDrawWin(uint16_t _usX, uint16_t _usY, uint16_t _Width, uint16_t _Height)
{
	uint16_t width; //height;
	uint16_t temp;   
	
	if (g_LcdDirection > 1)			/* 竖屏 */
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
	else							/* 横屏 */
	{
		width = g_LcdWidth;
//		height = g_LcdHeight;
	}
    
	
	if (g_ChipID == IC_8876)
	{
		#if 0
        uint32_t SAddr, DAddr;
        
		SAddr = (width * 2) * 2;			/* 第三块内存起始地址 */
	
		if (g_Interface == 0)					/* 当前显示第1块内存 */
		{
			DAddr = 0;						/* 目标地址，第1块内存起始地址 */
			/* 将源内存的内容复制到目的内存中 */
			BTE_Memory_Copy(SAddr, CanvasWidth,_usX,_usY, SAddr, CanvasWidth,_usX,_usY, DAddr,CanvasWidth,_usX,_usY,12, _Width, _Height);	
			Canvas_Image_Start_address(DAddr);				/* 操作内存地址切回到原来的区域 */
		}
		else
		{
			DAddr = (width * 2) * 1;		/* 目标地址，第2块内存起始地址 */
			/* 将源内存的内容复制到目的内存中 */
			BTE_Memory_Copy(SAddr, CanvasWidth,_usX,_usY, SAddr, CanvasWidth,_usX,_usY, DAddr,CanvasWidth,_usX,_usY,12, _Width, _Height);	
			Canvas_Image_Start_address(DAddr);				/* 操作内存地址切回到原来的区域 */
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
*	函 数 名: LCD_DispStr
*	功能说明: 在LCD指定坐标（左上角）显示一个字符串
*	形    参:
*		_usX : X坐标
*		_usY : Y坐标
*		_ptr  : 字符串指针
*		_tFont : 字体结构体，包含颜色、背景色(支持透明)、字体代码、文字间距等参数
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD_DispStr(uint16_t _usX, uint16_t _usY, char *_ptr, FONT_T *_tFont)
{
	LCD_DispStrEx0(_usX, _usY, _ptr, _tFont, 0, 0);
}


/*
*********************************************************************************************************
*	函 数 名: LCD_DispStrEx
*	功能说明: 在LCD指定坐标（左上角）显示一个字符串。 增强型函数。支持左\中\右对齐，支持定长清屏。 支持换行
*	形    参:
*		_usX : X坐标
*		_usY : Y坐标
*		_ptr  : 字符串指针
*		_tFont : 字体结构体，包含颜色、背景色(支持透明)、字体代码、文字间距等参数。可以指定RA8875字库显示汉字
*		_Width : 字符串显示区域的宽度. 0 表示不处理留白区域，此时_Align无效
*		_Align :字符串在显示区域的对齐方式，
*				ALIGN_LEFT = 0,
*				ALIGN_CENTER = 1,
*				ALIGN_RIGHT = 2
*	返 回 值: 无
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
		else if (ch == '\r')	/* 换行指令，后面2个字符表示垂直间距（单位像素） 支持重叠 */
		{
			uint8_t cap;
			
			LCD_DispStrEx0(x, y, str_buf, _tFont, _Width, _Align);
			
			len = 0;
			
			x = _usX;
			
			cap = (_ptr[i + 1] - '0') * 10 + _ptr[i + 2] - '0';		/* 间距 */
			y += cap;
			i += 2;
		}
		else if (ch == '\t')	/* 划线指令，后面8个字符表示 X1, Y2, X2,  Y2 00 99 02 02 */
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
*	函 数 名: LCD_GetFontWidth
*	功能说明: 读取字体的宽度（像素单位)
*	形    参:
*		_tFont : 字体结构体，包含颜色、背景色(支持透明)、字体代码、文字间距等参数
*	返 回 值: 字体的宽度（像素单位)
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
*	函 数 名: LCD_GetFontHeight
*	功能说明: 读取字体的高度（像素单位)
*	形    参:
*		_tFont : 字体结构体，包含颜色、背景色(支持透明)、字体代码、文字间距等参数
*	返 回 值: 字体的宽度（像素单位)
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
*	函 数 名: LCD_GetStrWidth
*	功能说明: 计算字符串宽度(像素单位)
*	形    参:
*		_ptr  : 字符串指针
*		_tFont : 字体结构体，包含颜色、背景色(支持透明)、字体代码、文字间距等参数
*	返 回 值: 无
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
		code1 = *p;	/* 读取字符串数据， 该数据可能是ascii代码，也可能汉字代码的高字节 */
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
	//							  /* 字库搜索完毕，未找到，则填充全FF */
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
	//							  /* 字库搜索完毕，未找到，则填充全FF */
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
					//对秒进行特殊处理
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
					//对秒进行特殊处理
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
		else	/* 汉字 */
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
*	函 数 名: _LCD_ReadSmallDot
*	功能说明: 读取1个小语种字符的点阵数据
*	形    参:
*		_code : ASCII字符的编码，1字节。1-128
*		_fontcode ：字体代码
*		_pBuf : 存放读出的字符点阵数据
*	返 回 值: 文字宽度
*********************************************************************************************************
*/
static void _LCD_ReadSmallDot(uint8_t _code, uint8_t _fontcode, uint8_t *_pBuf)
{
#ifdef USE_SMALL_FONT	/* 使用CPU 内部Flash 小字库 */
	const uint8_t *pAscDot;
	uint32_t font_bytes = 0;
	uint16_t m;
	uint16_t address;
	uint8_t fAllHz = 0;	/* 1表示程序中内嵌全部的ASCII字符集 */
	
	pAscDot = 0;
	switch (_fontcode)
	{
		case FC_ST_12:		/* 12点阵 */
			font_bytes = 24 / 2;
			pAscDot = g_Ascii12;	
			fAllHz = 1;
			break;
		
		case FC_ST_16:
			/* 缺省是16点阵 */
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

	if (fAllHz == 1)	/* 内嵌全部ASCII字符点阵 */
	{
		/* 将CPU内部Flash中的ascii字符点阵复制到buf */
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
			  /* 字库搜索完毕，未找到，则填充全FF */
			  memset(_pBuf, 0xFF, font_bytes);
			  break;
		   }	   
	   }
	}
	else	/* 内嵌部分字符，字模数组首字节是ASCII码 */
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
			  /* 字库搜索完毕，未找到，则填充全FF */
			  memset(_pBuf, 0xFF, font_bytes);
			  break;
		   }	   
	   }
   }
#else	/* 用全字库 */
	uint32_t pAscDot;
	uint8_t font_bytes = 0;

	pAscDot = 0;
	switch (_fontcode)
	{
		case FC_ST_12:		/* 12点阵 */
			font_bytes = 12;
			#if 0
				pAscDot = ASC12_ADDR;	/* 字库芯片的16点阵字符不好看,笔画细了，而且是非等宽字体 */
			#else
				pAscDot = (uint32_t)&g_Ascii12[' ' * 12];	/* 使用CPU内嵌的16点阵字符 */
			#endif			
			break;
		
		case FC_ST_16:
			font_bytes = 16;
			#if 0
				pAscDot = ASC16_ADDR;	/* 字库芯片的16点阵字符不好看,笔画细了，而且是非等宽字体 */
			#else
				pAscDot = (uint32_t)&g_Ascii16[' ' * 16];	/* 使用CPU内嵌的16点阵字符 */
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

#ifdef USE_NOR_FONT		/* NOR Flash全字库 */
	/* 将CPU内部Flash中的ascii字符点阵复制到buf */
	memcpy(_pBuf, (char *)pAscDot, font_bytes);	
#endif

#ifdef USE_SPI_FONT		/* 串行 Flash全字库 */
	if (_fontcode == FC_ST_12 || _fontcode == FC_ST_16)
	{
		memcpy(_pBuf, (char *)pAscDot, font_bytes);	
	}
	else
	{
		/* 字库芯片的12点阵和16点阵字符不好看,笔画细了，而且是非等宽字体 */
		sf_ReadBuffer(_pBuf, pAscDot, font_bytes);
	}
#endif
	
#endif
}

/*
*********************************************************************************************************
*	函 数 名: _LCD_ReadAsciiDot
*	功能说明: 读取1个ASCII字符的点阵数据
*	形    参:
*		_code : ASCII字符的编码，1字节。1-128
*		_fontcode ：字体代码
*		_pBuf : 存放读出的字符点阵数据
*	返 回 值: 文字宽度
*********************************************************************************************************
*/
static void _LCD_ReadAsciiDot(uint8_t _code, uint8_t _fontcode, uint8_t *_pBuf)
{
#ifdef USE_SMALL_FONT	/* 使用CPU 内部Flash 小字库 */
	const uint8_t *pAscDot;
	uint32_t font_bytes = 0;
	uint16_t m;
	uint16_t address;
	uint8_t fAllHz = 0;	/* 1表示程序中内嵌全部的ASCII字符集 */

	pAscDot = 0;
	switch (_fontcode)
	{
		case FC_ST_12:		/* 12点阵 */
			font_bytes = 24 / 2;
			pAscDot = g_Ascii12;	
			fAllHz = 1;
			break;
		
		case FC_ST_16:
			/* 缺省是16点阵 */
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

	if (fAllHz == 1)	/* 内嵌全部ASCII字符点阵 */
	{
		/* 将CPU内部Flash中的ascii字符点阵复制到buf */
		memcpy(_pBuf, &pAscDot[_code * (font_bytes)], (font_bytes));		
	}
	else	/* 内嵌部分字符，字模数组首字节是ASCII码 */
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
			  /* 字库搜索完毕，未找到，则填充全FF */
			  memset(_pBuf, 0xFF, font_bytes);
			  break;
		   }	   
	   }
   }
#else	/* 用全字库 */
	uint32_t pAscDot;
	uint8_t font_bytes = 0;

	pAscDot = 0;
	switch (_fontcode)
	{
		case FC_ST_12:		/* 12点阵 */
			font_bytes = 12;
			#if 0
				pAscDot = ASC12_ADDR;	/* 字库芯片的16点阵字符不好看,笔画细了，而且是非等宽字体 */
			#else
				pAscDot = (uint32_t)&g_Ascii12[' ' * 12];	/* 使用CPU内嵌的16点阵字符 */
			#endif			
			break;
		
		case FC_ST_16:
			font_bytes = 16;
			#if 0
				pAscDot = ASC16_ADDR;	/* 字库芯片的16点阵字符不好看,笔画细了，而且是非等宽字体 */
			#else
				pAscDot = (uint32_t)&g_Ascii16[' ' * 16];	/* 使用CPU内嵌的16点阵字符 */
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

#ifdef USE_NOR_FONT		/* NOR Flash全字库 */
	/* 将CPU内部Flash中的ascii字符点阵复制到buf */
	memcpy(_pBuf, (char *)pAscDot, font_bytes);	
#endif

#ifdef USE_SPI_FONT		/* 串行 Flash全字库 */
	if (_fontcode == FC_ST_12 || _fontcode == FC_ST_16)
	{
		memcpy(_pBuf, (char *)pAscDot, font_bytes);	
	}
	else
	{
		/* 字库芯片的12点阵和16点阵字符不好看,笔画细了，而且是非等宽字体 */
		sf_ReadBuffer(_pBuf, pAscDot, font_bytes);
	}
#endif
	
#endif
}

/*
*********************************************************************************************************
*	函 数 名: _LCD_ReadHZDot
*	功能说明: 读取1个汉字的点阵数据
*	形    参:
*		_code1, _cod2 : 汉字内码. GB2312编码
*		_fontcode ：字体代码
*		_pBuf : 存放读出的字符点阵数据
*	返 回 值: 无
*********************************************************************************************************
*/
static void _LCD_ReadHZDot(uint8_t _code1, uint8_t _code2,  uint8_t _fontcode, uint8_t *_pBuf)
{
#ifdef USE_SMALL_FONT	/* 使用CPU 内部Flash 小字库 */
	uint8_t *pDot;
	uint8_t font_bytes = 0;
	uint32_t address;
	uint16_t m;

	pDot = 0;	/* 仅仅用于避免告警 */
	switch (_fontcode)
	{
		case FC_ST_12:		/* 12点阵 */
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
			/* 字库搜索完毕，未找到，则填充全FF */
			memset(_pBuf, 0xFF, font_bytes);
			break;
		}
	}
#else	/* 用全字库 */
	uint32_t offset = 0;
	uint8_t font_bytes = 0;
		
	switch (_fontcode)
	{
		case FC_ST_12:		/* 12点阵 */
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

	/* 此处需要根据字库文件存放位置进行修改 
		GB2312范围： 0xA1A1 - 0xFEFE
		其中汉字范围 : 0xB0A1 - 0xF7FE
	
		GBK 范围： 0x8140 - 0xFEFE 
	
		安富莱自定义汉字编码错开GBK和GB2312编码空间： 0x8000 - 0x813F （319个）		
	*/
	if (_code1 >=0xA1 && _code1 <= 0xA9 && _code2 >=0xA1)
	{
		offset += ((_code1 - 0xA1) * 94 + (_code2 - 0xA1)) * font_bytes;
	}
	else if (_code1 >=0xB0 && _code1 <= 0xF7 && _code2 >=0xA1)
	{
		offset += ((_code1 - 0xB0) * 94 + (_code2 - 0xA1) + 846) * font_bytes;
	}
	else	/* 2018-03-13 增加自定义汉字编码，用于实现特殊图标符号 */
	{
		uint16_t code16;
		uint8_t *pDot;
		uint32_t address;
		uint16_t m;		
		
		code16 = _code1 * 256 + _code2;
		if (code16 >= 0x8000 && code16 <= 0x813F)	/* 自定义汉字点阵，固定使用CPU片内部小字库 */
		{
			pDot = 0;	/* 仅仅用于避免告警 */
			switch (_fontcode)
			{
				case FC_ST_12:		/* 12点阵 */
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
					/* 字库搜索完毕，未找到，则填充全FF */
					memset(_pBuf, 0xFF, font_bytes);
					break;
				}
			}	
			return;
		}
	}

#ifdef USE_NOR_FONT		/* NOR Flash全字库 */
	/* 将CPU内部Flash中的ascii字符点阵复制到buf */
	memcpy(_pBuf, (char *)offset, font_bytes);	
#endif

#ifdef USE_SPI_FONT		/* NOR Flash全字库 */
	sf_ReadBuffer(_pBuf, offset, font_bytes);
#endif
	
#endif
}

/*
*********************************************************************************************************
*	函 数 名: SeachStr_a
*	功能说明: 搜索一个字符串是否有‘\a’
*	形    参:
*		_ptr  : 字符串指针
*	返 回 值: 无
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
*	函 数 名: LCD_DispStrEx0
*	功能说明: 在LCD指定坐标（左上角）显示一个字符串。 增强型函数。支持左\中\右对齐，支持定长清屏。
*	形    参:
*		_usX : X坐标
*		_usY : Y坐标
*		_ptr  : 字符串指针
*		_tFont : 字体结构体，包含颜色、背景色(支持透明)、字体代码、文字间距等参数。可以指定RA8875字库显示汉字
*		_Width : 字符串显示区域的宽度. 0 表示不处理留白区域，此时_Align无效
*		_Align :字符串在显示区域的对齐方式，
*				ALIGN_LEFT = 0,
*				ALIGN_CENTER = 1,
*				ALIGN_RIGHT = 2
*	返 回 值: 无
*********************************************************************************************************
*/
static void LCD_DispStrEx0(uint16_t _usX, uint16_t _usY, char *_ptr, FONT_T *_tFont, uint16_t _Width,
	uint8_t _Align)
{
	uint32_t i;
	uint8_t code1;
	uint8_t code2;
	//uint8_t buf[32 * 32 / 8];	/* 最大支持32点阵汉字 */
	uint8_t buf[96 * 40 / 8];	/* 最大支持96x40点阵字符 */
	uint8_t width;
	uint16_t m;
	uint8_t font_width = 0;
	uint8_t font_height = 0;
	uint16_t x, y;
	uint16_t offset;
	uint16_t str_width;	/* 字符串实际宽度  */
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
		case FC_ST_12:		/* 12点阵 */
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
			ra8875_use = 1;	/* 表示用RA8875字库 */
			break;
			
		case FC_RA8875_24:
			ra8875_font_code = RA_FONT_24;
			a_flag = SeachStr_a(_ptr);	/* 搜索字符串中是否有'\a' */
			if (a_flag == 0)	/**/
			{
				ra8875_use = 1;	/* 表示用RA8875字库 */
			}
			else
			{
				ra8875_use = 0;	/* 表示不用RA8875字库 */
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
				ra8875_use = 1;	/* 表示用RA8875字库 */
			}
			else
			{
				ra8875_use = 0;	/* 表示不用RA8875字库 */
				font_height = 32;
				font_width = 32;
				asc_bytes = 4;
				hz_bytes = 4;
			}
			break;
	}
	
	str_width = LCD_GetStrWidth(_ptr, _tFont);	/* 计算字符串实际宽度(RA8875内部ASCII点阵宽度为变长 */
	offset = 0;
	if (_Width > str_width)
	{
		if (_Align == ALIGN_RIGHT)	/* 右对齐 */
		{
			offset = _Width - str_width;
		}
		else if (_Align == ALIGN_CENTER)	/* 左对齐 */
		{
			offset = (_Width - str_width) / 2;
		}
		else	/* 左对齐 ALIGN_LEFT */
		{
			;
		}
	}

	/* 左侧填背景色, 中间对齐和右边对齐  */
	if (offset > 0)
	{
		if (_tFont->BackColor != CL_MASK)	/* 透明色 */
		{
			LCD_Fill_Rect(_usX, _usY, LCD_GetFontHeight(_tFont), offset,  _tFont->BackColor);	
		}
		_usX += offset;
		
	}
	
	/* 右侧填背景色 */
	if (_Width > str_width)
	{
		if (_tFont->BackColor != CL_MASK)	/* 透明色 */
		{
			LCD_Fill_Rect(_usX + str_width, _usY, LCD_GetFontHeight(_tFont), _Width - str_width - offset,  _tFont->BackColor);
		}
	}
	
	if (ra8875_use == 1)	/* 使用RA8875外挂的字库芯片 */
	{
		if (g_ChipID == IC_8875)
		{
			if (_tFont->BackColor == CL_MASK)	/* 透明色 */
			{
				RA8875_SetTextTransp(1);
			}
			RA8875_SetFrontColor(_tFont->FrontColor);			/* 设置字体前景色 */
			RA8875_SetBackColor(_tFont->BackColor);				/* 设置字体背景色 */
			RA8875_SetFont(ra8875_font_code, 0, _tFont->Space);	/* 字体代码，行间距，字间距 */
			RA8875_DispStr(_usX, _usY, _ptr);
			if (_tFont->BackColor == CL_MASK)	/* 透明色 */
			{
				RA8875_SetTextTransp(0);
			}	
		}
		else if (g_ChipID == IC_8876)
		{
		  #if 0
			if (_tFont->BackColor == CL_MASK)	/* 透明色 */
			{
				Font_Background_select_Original_Canvas();		/* 显示背景原来的颜色 */	
			}
			RA8876_SetFont(ra8875_font_code, 0, _tFont->Space);	/* 字体代码，行间距，字间距 */
			RA8876_DispStr(0, _usX, _usY, _tFont->FrontColor, _tFont->BackColor, _ptr);		/* 第一个形参表示RA8876选中串行flash1 */
			if (_tFont->BackColor == CL_MASK)	/* 透明色 */
			{
				Font_Background_select_Color();					/* 显示背景选择的颜色 */
			}
		  #endif
		}
	}
	else	/* 使用CPU内部字库. 点阵信息由CPU读取 */
	{
		/* 开始循环处理字符 */
		while (*_ptr != 0)
		{
			code1 = *_ptr;	/* 读取字符串数据， 该数据可能是ascii代码，也可能汉字代码的高字节 */
				
			if (code1 < 0x80)
			{
				if (a_flag == 0)
				{
					RA8875_flag = 0;
					/* 将ascii字符点阵复制到buf */
					_LCD_ReadAsciiDot(code1, _tFont->FontCode, buf);	/* 读取ASCII字符点阵 */
					
					//对秒进行特殊处理,避免宽度过大
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
	//							  /* 字库搜索完毕，未找到，则填充全FF */
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
	//							  /* 字库搜索完毕，未找到，则填充全FF */
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
				/* 读1个汉字的点阵 */
				_LCD_ReadHZDot(code1, code2, _tFont->FontCode, buf);
				width = font_width;
				line_bytes = hz_bytes;
			}
			
			y = _usY;
			if (RA8875_flag == 0)
			{
				/* 开始刷LCD */
				for (m = 0; m < font_height; m++)	/* 字符高度 */
				{
					x = _usX;
					for (i = 0; i < width; i++)	/* 字符宽度 */
					{
						if ((buf[m * line_bytes + i / 8] & (0x80 >> (i % 8 ))) != 0x00)
						{
							LCD_PutPixel(x, y, _tFont->FrontColor);	/* 设置像素颜色为文字色 */
						}
						else
						{
							if (_tFont->BackColor != CL_MASK)	/* 透明色 */
							{
								LCD_PutPixel(x, y, _tFont->BackColor);	/* 设置像素颜色为文字背景色 */
							}
						}
		
						x++;
					}
					
					for (i = 0; i < _tFont->Space; i++)	/* 字符宽度 */
					{
						if (_tFont->BackColor != CL_MASK)	/* 透明色 */
						{						
							/* 如果文字底色按_tFont->usBackColor，并且字间距大于点阵的宽度，那么需要在文字之间填充(暂时未实现) */
							LCD_PutPixel(x + i, y, _tFont->BackColor);	/* 设置像素颜色为文字背景色 */
						}
					}
					y++;					
				}
			}
			else
			{
				if (_tFont->BackColor == CL_MASK)	/* 透明色 */
				{
					RA8875_SetTextTransp(1);
				}
				RA8875_SetFrontColor(_tFont->FrontColor);			/* 设置字体前景色 */
				RA8875_SetBackColor(_tFont->BackColor);				/* 设置字体背景色 */
				RA8875_SetFont(ra8875_font_code, 0, _tFont->Space);	/* 字体代码，行间距，字间距 */
				RA8875_DispStr(_usX, _usY, (char *)&code1);
				if (_tFont->BackColor == CL_MASK)	/* 透明色 */
				{
					RA8875_SetTextTransp(0);
				}	
			}
	
			_usX += width + _tFont->Space;	/* 列地址递增 */
			_ptr++;			/* 指向下一个字符 */
		}
	}
}

/*
*********************************************************************************************************
*	函 数 名: LCD_DrawPoints
*	功能说明: 采用 Bresenham 算法，绘制一组点，并将这些点连接起来。可用于波形显示。
*	形    参:
*			x, y     : 坐标数组
*			_usColor : 颜色
*	返 回 值: 无
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
*	函 数 名: LCD_DrawWin
*	功能说明: 在LCD上绘制一个窗口
*	形    参: 结构体指针
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD_DrawWin(WIN_T *_pWin)
{
#if 1
	uint16_t TitleHegiht;

	TitleHegiht = 20;

	/* 绘制窗口外框 */
	LCD_DrawRect(_pWin->Left, _pWin->Top, _pWin->Height, _pWin->Width, WIN_BORDER_COLOR);
	LCD_DrawRect(_pWin->Left + 1, _pWin->Top + 1, _pWin->Height - 2, _pWin->Width - 2, WIN_BORDER_COLOR);

	/* 窗口标题栏 */
	LCD_Fill_Rect(_pWin->Left + 2, _pWin->Top + 2, TitleHegiht, _pWin->Width - 4, WIN_TITLE_COLOR);

	/* 窗体填充 */
	LCD_Fill_Rect(_pWin->Left + 2, _pWin->Top + TitleHegiht + 2, _pWin->Height - 4 - TitleHegiht,
		_pWin->Width - 4, WIN_BODY_COLOR);

	LCD_DispStr(_pWin->Left + 3, _pWin->Top + 2, _pWin->pCaption, _pWin->Font);
#else
	if (g_ChipID == IC_8875)
	{
		uint16_t TitleHegiht;

		TitleHegiht = 28;

		/* 绘制窗口外框 */
		RA8875_DrawRect(_pWin->Left, _pWin->Top, _pWin->Height, _pWin->Width, WIN_BORDER_COLOR);
		RA8875_DrawRect(_pWin->Left + 1, _pWin->Top + 1, _pWin->Height - 2, _pWin->Width - 2, WIN_BORDER_COLOR);

		/* 窗口标题栏 */
		RA8875_FillRect(_pWin->Left + 2, _pWin->Top + 2, TitleHegiht, _pWin->Width - 4, WIN_TITLE_COLOR);

		/* 窗体填充 */
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
*	函 数 名: LCD_DrawIcon
*	功能说明: 在LCD上绘制一个图标，四角自动切为弧脚
*	形    参: _pIcon : 图标结构
*			  _tFont : 字体属性
*			  _ucFocusMode : 焦点模式。0 表示正常图标  1表示选中的图标
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD_DrawIcon(const ICON_T *_tIcon, FONT_T *_tFont, uint8_t _ucFocusMode)
{
	const uint16_t *p;
	uint16_t usNewRGB;
	uint16_t x, y;		/* 用于记录窗口内的相对坐标 */

	p = _tIcon->pBmp;
	for (y = 0; y < _tIcon->Height; y++)
	{
		for (x = 0; x < _tIcon->Width; x++)
		{
			usNewRGB = *p++;	/* 读取图标的颜色值后指针加1 */
			/* 将图标的4个直角切割为弧角，弧角外是背景图标 */
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
				if (_ucFocusMode != 0)	/* 1表示选中的图标 */
				{
					/* 降低原始像素的亮度，实现图标被激活选中的效果 */
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

	/* 绘制图标下的文字 */
	{
		uint16_t len;
		uint16_t width;

		len = strlen(_tIcon->Text);

		if  (len == 0)
		{
			return;	/* 如果图标文本长度为0，则不显示 */
		}

		/* 计算文本的总宽度 */
		if (_tFont->FontCode == FC_ST_12)		/* 12点阵 */
		{
			width = 6 * (len + _tFont->Space);
		}
		else	/* FC_ST_16 */
		{
			width = 8 * (len + _tFont->Space);
		}


		/* 水平居中 */
		x = (_tIcon->Left + _tIcon->Width / 2) - width / 2;
		y = _tIcon->Top + _tIcon->Height + 2;
		LCD_DispStr(x, y, (char *)_tIcon->Text, _tFont);
	}
}

/*
*********************************************************************************************************
*	函 数 名: LCD_Blend565
*	功能说明: 对像素透明化 颜色混合
*	形    参: src : 原始像素
*			  dst : 混合的颜色
*			  alpha : 透明度 0-32
*	返 回 值: 无
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
*	函 数 名: LCD_DrawIcon32
*	功能说明: 在LCD上绘制一个图标, 带有透明信息的位图(32位， RGBA). 图标下带文字
*	形    参: _pIcon : 图标结构
*			  _tFont : 字体属性
*			  _ucFocusMode : 焦点模式。0 表示正常图标  1表示选中的图标
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD_DrawIcon32(const ICON_T *_tIcon, FONT_T *_tFont, uint8_t _ucFocusMode)
{
	const uint8_t *p;
	uint16_t usOldRGB, usNewRGB;
	int16_t x, y;		/* 用于记录窗口内的相对坐标 */
	uint8_t R1,G1,B1,A;	/* 新像素色彩分量 */
	uint8_t R0,G0,B0;	/* 旧像素色彩分量 */

	p = (const uint8_t *)_tIcon->pBmp;
	p += 54;		/* 直接指向图像数据区 */

	/* 按照BMP位图次序，从左至右，从上至下扫描 */
	for (y = _tIcon->Height - 1; y >= 0; y--)
	{
		for (x = 0; x < _tIcon->Width; x++)
		{
			B1 = *p++;
			G1 = *p++;
			R1 = *p++;
			A = *p++;	/* Alpha 值(透明度)，0-255, 0表示透明，1表示不透明, 中间值表示透明度 */

			if (A == 0x00)	/* 需要透明,显示背景 */
			{
				;	/* 不用刷新背景 */
			}
			else if (A == 0xFF)	/* 完全不透明， 显示新像素 */
			{
				usNewRGB = RGB(R1, G1, B1);
				if (_ucFocusMode == 1)
				{
					usNewRGB = LCD_Blend565(usNewRGB, CL_YELLOW, 10);
				}
				LCD_PutPixel(x + _tIcon->Left, y + _tIcon->Top, usNewRGB);
			}
			else 	/* 半透明 */
			{
				/* 计算公式： 实际显示颜色 = 前景颜色 * Alpha / 255 + 背景颜色 * (255-Alpha) / 255 */
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

	/* 绘制图标下的文字 */
	{
		uint16_t len;
		uint16_t width;

		len = strlen(_tIcon->Text);

		if  (len == 0)
		{
			return;	/* 如果图标文本长度为0，则不显示 */
		}

		/* 计算文本的总宽度 */
		if (_tFont->FontCode == FC_ST_12)		/* 12点阵 */
		{
			width = 6 * (len + _tFont->Space);
		}
		else	/* FC_ST_16 */
		{
			width = 8 * (len + _tFont->Space);
		}


		/* 水平居中 */
		x = (_tIcon->Left + _tIcon->Width / 2) - width / 2;
		y = _tIcon->Top + _tIcon->Height + 2;
		LCD_DispStr(x, y, (char *)_tIcon->Text, _tFont);
	}
}

/*
*********************************************************************************************************
*	函 数 名: LCD_DrawBmp32
*	功能说明: 在LCD上绘制一个32位的BMP图, 带有透明信息的位图(32位， RGBA)
*	形    参: _usX, _usY : 显示坐标
*			  _usHeight, _usWidth : 图片高度和宽度
*			  _pBmp : 图片数据（带BMP文件头）
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD_DrawBmp32(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth, uint8_t *_pBmp)
{
	const uint8_t *p;
	uint16_t usOldRGB, usNewRGB;
	int16_t x, y;		/* 用于记录窗口内的相对坐标 */
	uint8_t R1,G1,B1,A;	/* 新像素色彩分量 */
	uint8_t R0,G0,B0;	/* 旧像素色彩分量 */

	p = (const uint8_t *)_pBmp;
	p += 54;		/* 直接指向图像数据区 */

	/* 按照BMP位图次序，从左至右，从上至下扫描 */
	for (y = _usHeight - 1; y >= 0; y--)
	{
		for (x = 0; x < _usWidth; x++)
		{
			B1 = *p++;
			G1 = *p++;
			R1 = *p++;
			A = *p++;	/* Alpha 值(透明度)，0-255, 0表示透明，1表示不透明, 中间值表示透明度 */

			if (A == 0x00)	/* 需要透明,显示背景 */
			{
				;	/* 不用刷新背景 */
			}
			else if (A == 0xFF)	/* 完全不透明， 显示新像素 */
			{
				usNewRGB = RGB(R1, G1, B1);
				//if (_ucFocusMode == 1)
				//{
				//	usNewRGB = Blend565(usNewRGB, CL_YELLOW, 10);
				//}
				LCD_PutPixel(x + _usX, y + _usY, usNewRGB);
			}
			else 	/* 半透明 */
			{
				/* 计算公式： 实际显示颜色 = 前景颜色 * Alpha / 255 + 背景颜色 * (255-Alpha) / 255 */
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
*	函 数 名: LCD_DrawLabel
*	功能说明: 绘制一个文本标签
*	形    参: _pLabel : Label结构体指针
*	返 回 值: 无
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
*	函 数 名: LCD_DrawLabel
*	功能说明: 绘制一个文本标签
*	形    参: 结构体指针
*	返 回 值: 无
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
			dispbuf[i] = ' ';		/* 末尾填充空格 */
		}
		dispbuf[i] = 0;
		LCD_DispStr(_pLabel->Left, _pLabel->Top, dispbuf, _pLabel->Font);
	}
#else
	if (g_ChipID == IC_8875)
	{
		RA8875_SetFont(_pLabel->Font->FontCode, 0, 0);	/* 设置32点阵字体，行间距=0，字间距=0 */

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
*	函 数 名: LCD_DrawCheckBox
*	功能说明: 绘制一个检查框
*	形    参: 结构体指针
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD_DrawCheckBox(CHECK_T *_pCheckBox)
{
#if 1
	uint16_t x, y;

	/* 目前只做了16点阵汉字的大小 */

	/* 绘制外框 */
	x = _pCheckBox->Left;
	LCD_DrawRect(x, _pCheckBox->Top, CHECK_BOX_H, CHECK_BOX_W, CHECK_BOX_BORDER_COLOR);
	LCD_DrawRect(x + 1, _pCheckBox->Top + 1, CHECK_BOX_H - 2, CHECK_BOX_W - 2, CHECK_BOX_BORDER_COLOR);
	LCD_Fill_Rect(x + 2, _pCheckBox->Top + 2, CHECK_BOX_H - 4, CHECK_BOX_W - 4, CHECK_BOX_BACK_COLOR);

	/* 绘制文本标签 */
	x = _pCheckBox->Left + CHECK_BOX_W + 2;
	y = _pCheckBox->Top + CHECK_BOX_H / 2 - 8;
	LCD_DispStr(x, y, _pCheckBox->pCaption, _pCheckBox->Font);

	if (_pCheckBox->Checked)
	{
		FONT_T font;

	    font.FontCode = FC_ST_16;
		font.BackColor = CL_MASK;
		font.FrontColor = CHECK_BOX_CHECKED_COLOR;	/* 钩的颜色 */
		font.Space = 0;
		x = _pCheckBox->Left;
		LCD_DispStr(x + 3, _pCheckBox->Top + 3, "√", &font);
	}
#else
	if (g_ChipID == IC_8875)
	{
		uint16_t x;

		RA8875_SetFont(_pCheckBox->Font.FontCode, 0, 0);	/* 设置32点阵字体，行间距=0，字间距=0 */

		/* 绘制标签 */
		//RA8875_SetBackColor(_pCheckBox->Font.BackColor);
		RA8875_SetBackColor(WIN_BODY_COLOR);
		RA8875_SetFrontColor(_pCheckBox->Font.FrontColor);
		RA8875_DispStr(_pCheckBox->Left, _pCheckBox->Top, _pCheckBox->Caption);

		/* 绘制外框 */
		x = _pCheckBox->Left + _pCheckBox->Width - CHECK_BOX_W;
		RA8875_DrawRect(x, _pCheckBox->Top, CHECK_BOX_H, CHECK_BOX_W, CHECK_BOX_BORDER_COLOR);
		RA8875_DrawRect(x + 1, _pCheckBox->Top + 1, CHECK_BOX_H - 2, CHECK_BOX_W - 2, CHECK_BOX_BORDER_COLOR);
		RA8875_FillRect(x + 2, _pCheckBox->Top + 2, CHECK_BOX_H - 4, CHECK_BOX_W - 4, CHECK_BOX_BACK_COLOR);

		if (_pCheckBox->Checked)
		{
			RA8875_SetBackColor(CHECK_BOX_BACK_COLOR);
			RA8875_SetFrontColor(CL_RED);
			RA8875_DispStr(x + 3, _pCheckBox->Top + 3, "√");
		}
	}
	else
	{

	}
#endif

}

/*
*********************************************************************************************************
*	函 数 名: LCD_InitEdit
*	功能说明: 初始化编辑框参数
*	形    参: _pBox 分组框
*	返 回 值: 无
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
*	函 数 名: LCD_DrawEdit
*	功能说明: 在LCD上绘制一个编辑框
*	形    参: _pEdit 编辑框结构体指针
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD_DrawEdit(EDIT_T *_pEdit)
{
	uint16_t len, x, y;
	
	/* 仿XP风格，平面编辑框 */
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
	
	/* 文字居中 */
	len = LCD_GetStrWidth(_pEdit->Text,  _pEdit->Font);
	x = _pEdit->Left +  (_pEdit->Width - len) / 2;
	y = _pEdit->Top + (_pEdit->Height - LCD_GetFontHeight(_pEdit->Font)) / 2;

	LCD_DispStr(x, y, _pEdit->Text, _pEdit->Font);
}

/*
*********************************************************************************************************
*	函 数 名: LCD_EditTouchDown
*	功能说明: 判断按钮是否被按下. 检查触摸坐标是否在按钮的范围之内。并重绘按钮。
*	形    参:  _edit : 编辑框对象
*			  _usX, _usY: 触摸坐标
*	返 回 值: 1 表示在范围内
*********************************************************************************************************
*/
uint8_t LCD_EditTouchDown(EDIT_T *_Edit, uint16_t _usX, uint16_t _usY)
{
	if ((_usX > _Edit->Left) && (_usX < _Edit->Left + _Edit->Width)
		&& (_usY > _Edit->Top) && (_usY < _Edit->Top + _Edit->Height))
	{
		BUTTON_BEEP();	/* 按键提示音 bsp_tft_lcd.h 文件开头可以使能和关闭 */
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
*	函 数 名: LCD_EditTouchRelease
*	功能说明: 编辑框退出编辑状态，重绘
*	形    参:  _Edit : 编辑框对象
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD_EditRefresh(EDIT_T *_Edit)
{
	_Edit->Focus = 0;
	LCD_DrawEdit(_Edit);
}

/*
*********************************************************************************************************
*	函 数 名: LCD_InitGroupBox
*	功能说明: 初始化分组框参数
*	形    参: _pBox 分组框
*	返 回 值: 无
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
*	函 数 名: LCD_DrawGroupBox
*	功能说明: 在LCD上绘制一个分组框
*	形    参: _pBox 分组框
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD_DrawGroupBox(GROUP_T *_pBox)
{
	uint16_t x, y;
	uint16_t x1,y1;		/* 矩形左上角 */
	uint16_t x2, y2;	/* 矩形右下角 */
	uint16_t len;

	
	len = LCD_GetStrWidth(_pBox->pCaption, _pBox->Font);	/* 字符串的总宽度 */
	
	/* 画阴影线 */
	//LCD_DrawRect(_pBox->Left + 1, _pBox->Top + 5, _pBox->Height, _pBox->Width - 1, CL_BOX_BORDER2);
	x1 = _pBox->Left + 1;
	y1 = _pBox->Top + 5;
	x2 = _pBox->Left + 1 + _pBox->Width - 2;
	y2 = _pBox->Top + 5 + _pBox->Height - 1;
	
	LCD_DrawLine(x1, y1, x1 + 6, y1, CL_BOX_BORDER2);	/* 顶1 */
	LCD_DrawLine(x1 + 8 + len + 1, y1, x2, y1, CL_BOX_BORDER2);	/* 顶2 */	
	LCD_DrawLine(x1, y2, x2, y2, CL_BOX_BORDER2);	/* 底 */
	LCD_DrawLine(x1, y1, x1, y2, CL_BOX_BORDER2);	/* 左 */
	LCD_DrawLine(x2, y1, x2, y2, CL_BOX_BORDER2);	/* 右 */	

	/* 画主框线 */
	//LCD_DrawRect(_pBox->Left, _pBox->Top + 4, _pBox->Height, _pBox->Width - 1, CL_BOX_BORDER1);
	x1 = _pBox->Left;
	y1 = _pBox->Top + 4;
	x2 = _pBox->Left + _pBox->Width - 2;
	y2 = _pBox->Top + 4 + _pBox->Height - 1;
	
	LCD_DrawLine(x1, y1, x1 + 6, y1, CL_BOX_BORDER1);	/* 顶1 */
	LCD_DrawLine(x1 + 9 + len + 1, y1, x2, y1, CL_BOX_BORDER1);	/* 顶2 */	
	LCD_DrawLine(x1, y2, x2, y2, CL_BOX_BORDER1);	/* 底 */
	LCD_DrawLine(x1, y1, x1, y2, CL_BOX_BORDER1);	/* 左 */
	LCD_DrawLine(x2, y1, x2, y2, CL_BOX_BORDER1);	/* 右 */		

	/* 显示分组框标题（文字在左上角） */
	x = _pBox->Left + 9;
	y = _pBox->Top;
	LCD_DispStr(x, y, _pBox->pCaption, _pBox->Font);
}

/*
*********************************************************************************************************
*	函 数 名: LCD_DispControl
*	功能说明: 绘制控件
*	形    参: _pControl 控件指针
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD_DispControl(void *_pControl)
{
	uint8_t id;

	id = *(uint8_t *)_pControl;	/* 读取ID */

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
*	函 数 名: LCD_InitButton
*	功能说明: 初始化按钮结构体成员。
*	形    参:  _x, _y : 坐标
*			  _h, _w : 高度和宽度
*			  _pCaption : 按钮文字
*			  _pFont : 按钮字体
*	返 回 值: 无
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
*	函 数 名: LCD_DrawButton2
*	功能说明: 在LCD上绘制一个按钮，类似emwin按钮
*	形    参:
*			_usX, _usY : 图片的坐标
*			_usHeight  : 图片高度
*			_usWidth   : 图片宽度
*			_ptr       : 图片点阵指针
*	返 回 值: 无
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
		y = _pBtn->Top + (_pBtn->Height - LCD_GetFontHeight(_pBtn->Font)) / 2;	/* 单行文本垂直居中 */
	}
	else
	{
		y = _pBtn->Top;		/* 多行文本,垂直坐标从顶部开始 */
	}	
		
	if (g_ChipID == IC_8875)
	{
		uint8_t Arc = 5;
				
		if (_pBtn->Focus == 0)
		{
			RA8875_DrawRoundRect(_pBtn->Left, _pBtn->Top, _pBtn->Height, _pBtn->Width, Arc,  BTN_BORDER_COLOR1);
			RA8875_FillRoundRect(_pBtn->Left + 1, _pBtn->Top + 1, _pBtn->Height - 2, _pBtn->Width - 2, Arc,  BTN_BODY_COLOR1);			
			LCD_Fill_Rect(_pBtn->Left + Arc, _pBtn->Top + 1, _pBtn->Height / 2, _pBtn->Width - 2 * Arc, BTN_SHADOW_COLOR1);	/* 画阴影对比色 */
		}
		else
		{
			RA8875_DrawRoundRect(_pBtn->Left, _pBtn->Top, _pBtn->Height, _pBtn->Width, Arc,  BTN_BORDER_COLOR2);
			RA8875_FillRoundRect(_pBtn->Left + 1, _pBtn->Top + 1, _pBtn->Height - 2, _pBtn->Width - 2, Arc, BTN_BODY_COLOR2);			
			LCD_Fill_Rect(_pBtn->Left + Arc, _pBtn->Top + 1, _pBtn->Height / 2, _pBtn->Width - 2 * Arc, BTN_SHADOW_COLOR2);	/* 画阴影对比色 */
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
			LCD_Fill_Rect(_pBtn->Left + Arc, _pBtn->Top + 1, _pBtn->Height / 2, _pBtn->Width - 2 * Arc, BTN_SHADOW_COLOR1);	/* 画阴影对比色 */
		}
		else
		{
			RA8876_DrawRoundRect(_pBtn->Left, _pBtn->Top, _pBtn->Height, _pBtn->Width, Arc,  BTN_BORDER_COLOR2);
			RA8876_FillRoundRect(_pBtn->Left + 1, _pBtn->Top + 1, _pBtn->Height - 2, _pBtn->Width - 2, Arc, BTN_BODY_COLOR2);			
			LCD_Fill_Rect(_pBtn->Left + Arc, _pBtn->Top + 1, _pBtn->Height / 2, _pBtn->Width - 2 * Arc, BTN_SHADOW_COLOR2);	/* 画阴影对比色 */
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
			LCD_Fill_Rect(_pBtn->Left + Arc, _pBtn->Top + 1, _pBtn->Height / 2, _pBtn->Width - 2 * Arc, BTN_SHADOW_COLOR1);	/* 画阴影对比色 */
		}
		else
		{
			SOFT_FillRoundRect(_pBtn->Left, _pBtn->Top, _pBtn->Height, _pBtn->Width, Arc,  BTN_BODY_COLOR2);			
			SOFT_DrawRoundRect(_pBtn->Left, _pBtn->Top, _pBtn->Height, _pBtn->Width, Arc,  BTN_BORDER_COLOR2);	
			LCD_Fill_Rect(_pBtn->Left + Arc, _pBtn->Top + 1, _pBtn->Height / 2, _pBtn->Width - 2 * Arc, BTN_SHADOW_COLOR2);	/* 画阴影对比色 */
		}

		LCD_DispStrEx(x, y, _pBtn->pCaption, _pBtn->Font, _pBtn->Width, ALIGN_CENTER);
	}
}

/*
*********************************************************************************************************
*	函 数 名: LCD_ButtonTouchDown
*	功能说明: 判断按钮是否被按下. 检查触摸坐标是否在按钮的范围之内。并重绘按钮。
*	形    参:  _btn : 按钮对象
*			  _usX, _usY: 触摸坐标
*	返 回 值: 1 表示在范围内
*********************************************************************************************************
*/
uint8_t LCD_ButtonTouchDown(BUTTON_T *_btn, uint16_t _usX, uint16_t _usY)
{
	if ((_usX > _btn->Left) && (_usX < _btn->Left + _btn->Width)
		&& (_usY > _btn->Top) && (_usY < _btn->Top + _btn->Height))
	{
		BUTTON_BEEP();	/* 按键提示音 bsp_tft_lcd.h 文件开头可以使能和关闭 */
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
*	函 数 名: LCD_ButtonTouchRelease
*	功能说明: 判断按钮是否被触摸释放. 并重绘按钮。在触摸释放事件中被调用。
*	形    参:  _btn : 按钮对象
*			  _usX, _usY: 触摸坐标
*	返 回 值: 1 表示在范围内
*********************************************************************************************************
*/
uint8_t LCD_ButtonTouchRelease(BUTTON_T *_btn, uint16_t _usX, uint16_t _usY)
{
	/* 2016-04-24 避免闪屏 */
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
*	函 数 名: LCD_DrawBmpButton
*	功能说明: 在LCD上绘制一个图片按钮
*	形    参:
*			_usX, _usY : 图片的坐标
*			_usHeight  : 图片高度
*			_usWidth   : 图片宽度
*			_ptr       : 图片点阵指针
*	返 回 值: 无
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
*	函 数 名: LCD_BmpButtonTouchDown
*	功能说明: 判断图片按钮按钮是否被按下. 检查触摸坐标是否在按钮的范围之内。并重绘按钮。
*	形    参:  _btn : 按钮对象
*			  _usX, _usY: 触摸坐标
*	返 回 值: 1 表示在范围内
*********************************************************************************************************
*/
uint8_t LCD_BmpButtonTouchDown(BMP_BUTTON_T *_btn, uint16_t _usX, uint16_t _usY)
{
	if ((_usX > _btn->Left) && (_usX < _btn->Left + _btn->Width)
		&& (_usY > _btn->Top) && (_usY < _btn->Top + _btn->Height))
	{
		BUTTON_BEEP();	/* 按键提示音 bsp_tft_lcd.h 文件开头可以使能和关闭 */
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
*	函 数 名: LCD_BmpButtonTouchRelease
*	功能说明: 判断图片按钮是否被触摸释放. 并重绘按钮。在触摸释放事件中被调用。
*	形    参:  _btn : 按钮对象
*			  _usX, _usY: 触摸坐标
*	返 回 值: 1 表示在范围内
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
*	函 数 名: LCD_SelectTouchDown
*	功能说明: 选中要校准的参数
*	形    参:  _slt : 选中对象
*			  _usX, _usY: 触摸坐标
*	返 回 值: 1 表示在范围内
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
*	函 数 名: LCD_InitPannel
*	功能说明: 初始化面板结构体成员。
*	形    参: _panl : pannel 对象
*			  _x, _y : 坐标
*			  _h, _w : 高度和宽度
*			  _pCaption : 按钮文字
*			  _pFont : 按钮字体
*	返 回 值: 无
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
*	函 数 名: LCD_DrawPannel
*	功能说明: 在LCD上绘制一个面板
*	形    参: 
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD_DrawPannel(PANNEL_T *_pnl)
{
	if (g_ChipID == IC_8875)
	{
		/* 绘制一个圆角矩形，填充底色 */
		RA8875_FillRoundRect(_pnl->Left, _pnl->Top, _pnl->Height, _pnl->Width, _pnl->Arc, _pnl->Color);
	}
	else
	{
		;	
	}
}

/*
*********************************************************************************************************
*	函 数 名: LCD_PannelClick
*	功能说明: 判断Pannel是否被点击. 检查触摸坐标是否在按钮的范围之内。
*	形    参:  _obj : PANNEL对象
*			  _usX, _usY: 触摸坐标
*	返 回 值: 1 表示在范围内 0表示不在
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
*	函 数 名: LCD_LabelClick
*	功能说明: 判断Label是否被点击. 检查触摸坐标是否在按钮的范围之内。
*	形    参:  _obj : PANNEL对象
*			  _usX, _usY: 触摸坐标
*	返 回 值: 1 表示在范围内 0表示不在
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
*	函 数 名: LCD_DrawArc
*	功能说明: 绘制一个圆弧，笔宽为1个像素
*	形    参:
*			_usX,_usY  ：圆心的坐标
*			_usRadius  ：圆的半径
*			_StartAng  : 起始角度
*			_EndAng	   : 终止角度
*			_usColor   : 圆弧颜色
*	返 回 值: 无
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

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/

