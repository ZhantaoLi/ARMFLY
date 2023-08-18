/*
*********************************************************************************************************
*
*	模块名称 : RA8876芯片驱动模块
*	文件名称 : LCD_RA8876.c
*	版    本 : V1.0
*	说    明 : RA8876底层驱动函数集。
*	修改记录 :
*		版本号  日期       作者    说明
*		V1.0    2016-06-28 armfly  发布首版
*	Copyright (C), 2016-2020, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#include "bsp.h"
#include "math.h"
#include "lcd_ra8876.h"
#include "main_menu.h"

/* RA8876特效相关全局变量 */
uint8_t g_Drawing = 0;			/* 是否在用特效画整个界面 */
uint8_t g_Interface = 0;		/* 当前界面 0:主界面(第1块内存) 1:从界面(第2块内存) */
uint16_t CanvasWidth;			/* 内存底图宽度 */

extern uint8_t g_TouchType;		/* 触摸类型 */
extern uint8_t g_LcdType;		/* LCD屏类型 */
/*
*********************************************************************************************************
*	函 数 名: RA8876_InitHard
*	功能说明: 初始化RA8876。本函数不开启背光，主程序应该在初始化完毕后再调用 RA8876_SetBackLight()函数开背光
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
/*
  定义了3块并排的内存，第一块存放主界面，第二块存放从界面，第三块为周转内存（每次刷新界面先写到该内存，再复制到 主/从界面 内存）

			 ---------------->---  ---------------->---   ---------------->---
			|(_usX，_usY)        |					    |					   |
 _usHeight	V     第1块内存      V 		第2块内存      	V       第3块内存	   V 
			|                    |						|	                   |
			 ---------------->---  ---------------->---   ---------------->---
				  _usWidth
*/

void RA8876_InitHard(void)
{
	uint8_t id;		
	uint8_t i;
		
	/* 电阻触摸板 */	
	for (i = 0; i < 5; i++)
	{
		if (i2c_CheckDevice(STMPE811_I2C_ADDRESS) == 0)
		{
			/*	RA8876 电阻屏型号识别	
			0  = 3.5寸屏（480X320）
			1  = 5.0寸屏（800X480）
			2  = 7.0寸屏（800X480）
			3  = 7.0寸屏（1024X600）
			4  = 10.1寸屏 （1024X600）
			*/							
			STMPE811_InitHard();				/* 必须先配置才能读取ID */
			
			id = STMPE811_ReadIO();				/* 识别LCD硬件类型 */	
			switch (id)
			{
				case 0:
					g_LcdType = LCD_35_480X320;
					/* 设置屏的宽度和高度 */
					g_LcdHeight = 320;
					g_LcdWidth = 480;
					break;

				case 1:
					g_LcdType = LCD_50_800X480;
					/* 设置屏的宽度和高度 */
					g_LcdHeight = 480;
					g_LcdWidth = 800;
					break;

				case 2:
					g_LcdType = LCD_70_800X480;
					/* 设置屏的宽度和高度 */
					g_LcdHeight = 480;
					g_LcdWidth = 800;
					break;

				case 3:
					g_LcdType = LCD_70_1024X600;
					/* 设置屏的宽度和高度 */
					g_LcdHeight = 600;
					g_LcdWidth = 1024;
					break;

				case 4:
					g_LcdType = LCD_101_1024X600;
					/* 设置屏的宽度和高度 */
					g_LcdHeight = 600;
					g_LcdWidth = 1024;
					break;
				
				default:
					g_LcdType = LCD_70_800X480;
					/* 设置屏的宽度和高度 */
					g_LcdHeight = 480;
					g_LcdWidth = 800;
					break;
			}		
		}
		else		/* 默认为10.1寸屏 */
		{
			g_LcdType = LCD_101_1024X600;
			/* 设置屏的宽度和高度 */
			g_LcdHeight = 600;
			g_LcdWidth = 1024;
		}
	}
	
	STMPE811_ResetRA8876();			/* 利用STMPE811来复位RA8876 */
	
	CanvasWidth = g_LcdWidth * 3;
	
	/* 由于V5出厂程序采用ID识别的方式，复位在识别之前做了。实际项目中可在此处硬件复位 */
//	RA8876_HW_Reset();				/* 外部复位，并检查MCU */
//	Check_IC_ready();				/* 检查MCU是否准备好  */
//	RA8876_SW_Reset();				/* 软件复位，官方例子中没有用 */
//	Check_IC_ready();	  

	RA8876_PLL();					/* 设置RA8876的PLL时钟 */
	RA8876_SDRAM_initial();			/* 设置SDRAM控制器 */ 

//	TFT_24bit();//RA8876 only
//	TFT_18bit();//RA8876 only	
	TFT_16bit();					/* 设置RA8876为16位输出（根据TFT屏数据引脚数设置） */
//	Without_TFT();//RA8876 only					
	
	/* 主控端数据总线选择 */
#if defined (MCU_8bit_ColorDepth_8bpp) || defined (MCU_8bit_ColorDepth_16bpp) || defined (MCU_8bit_ColorDepth_24bpp)	
	Host_Bus_8bit();
#endif
#if defined (MCU_16bit_ColorDepth_16bpp) || defined (MCU_16bit_ColorDepth_24bpp_Mode_1) || defined (MCU_16bit_ColorDepth_24bpp_Mode_2)	
	Host_Bus_16bit();				/* 设置为16bit */
#endif

	/* MPU 针对内存的读写数据格式 */
#ifdef MCU_8bit_ColorDepth_8bpp	
	Data_Format_8b_8bpp();
#endif
#ifdef MCU_8bit_ColorDepth_16bpp	
	Data_Format_8b_16bpp();
#endif
#ifdef MCU_8bit_ColorDepth_24bpp	
	Data_Format_8b_24bpp();
#endif
#ifdef MCU_16bit_ColorDepth_16bpp	
	Data_Format_16b_16bpp();
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_1	
	Data_Format_16b_24bpp_mode1();
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_2	
	Data_Format_16b_24bpp_mode2();
#endif	
	
	MemWrite_Left_Right_Top_Down();	/* 设置内存写入方向（仅对图形模式有效） */
//	MemWrite_Top_Down_Left_Right();
	
	
	Graphic_Mode();					/* 图形模式,必须先确定 core task busy 是否正在忙碌或闲置中，而 core task busy 是状态缓存器 */
//	Text_Mode();					/* 文本模式 */	

/* 选择 SDRAM 为 image/pattern/使用者自订字型的数据写入目的，支持 Read-modify-Write。 */
	Memory_Select_SDRAM();			
/* 选择 RGB 色的 Gamma table 为写入目的。 每个颜色的都是 256 bytes。
使用者需要指定需要写入的 gamma table 然后再连续写入 256 bytes。 */
//	Memory_Select_Gamma_Table();

	/* 设置颜色深度 */
#ifdef MCU_8bit_ColorDepth_8bpp	
	Select_Main_Window_8bpp();		//[10h] Set main window color depth
	Memory_8bpp_Mode();				//[5Eh] Set active memory color depth

	Select_PIP1_Window_8bpp();		//[11h] PIP 1 Window Color Depth
	Select_PIP2_Window_8bpp();		//[11h] PIP 2 Window Color Depth

	BTE_S0_Color_8bpp();			//[92h] Source_0 Color Depth
	BTE_S1_Color_8bpp();			//[92h] Source_1 Color Depth
	BTE_Destination_Color_8bpp();	//[92h] Destination Color Depth	
#endif

#if defined (MCU_8bit_ColorDepth_16bpp) || defined (MCU_16bit_ColorDepth_16bpp)
	Select_Main_Window_16bpp();		//[10h]Set main window color depth
	Memory_16bpp_Mode();			//[5Eh]Set active memory color depth

	Select_PIP1_Window_16bpp();		//[11h] PIP 1 Window Color Depth
	Select_PIP2_Window_16bpp();		//[11h] PIP 2 Window Color Depth

	BTE_S0_Color_16bpp();			//[92h] Source_0 Color Depth
	BTE_S1_Color_16bpp();			//[92h] Source_1 Color Depth
	BTE_Destination_Color_16bpp();	//[92h] Destination Color Depth	
#endif

#if defined (MCU_8bit_ColorDepth_24bpp) || defined (MCU_16bit_ColorDepth_24bpp_Mode_1) || defined (MCU_16bit_ColorDepth_24bpp_Mode_2)	
	Select_Main_Window_24bpp();		//[10h]Set main window color depth
	Memory_24bpp_Mode();			//[5Eh]Set active memory color depth

	Select_PIP1_Window_24bpp();		//[11h] PIP 1 Window Color Depth
	Select_PIP2_Window_24bpp();		//[11h] PIP 2 Window Color Depth

	BTE_S0_Color_24bpp();			//[92h] Source_0 Color Depth
	BTE_S1_Color_24bpp();			//[92h] Source_1 Color Depth
	BTE_Destination_Color_24bpp();	//[92h] Destination Color Depth	
#endif

	Set_LCD_Panel(); 					/* 设置LCD面板 */
	
/* RA8876手册 P157  LCD 显示控制缓存器 */
	Main_Image_Start_Address(0);		/* 主显示窗口的起始地址（显存，取SDRAM内存的数据），可调整显示图片的位置 */				
	Main_Image_Width(CanvasWidth);		/* 主显示窗口宽度，和底图内存宽度设置成一样，数据才能对应上 */
	Main_Window_Start_XY(0,0);			/* 从主显示显存的起始坐标开始读取SDRAM内存中的数据 */
	
/* RA8876手册 P51 */
	Canvas_Image_Start_address(0);		/* 从地址0开始写入SDRAM内存 */
	Canvas_image_width(CanvasWidth);	/* SDRAM内存底图宽度 */
	
/* 工作窗口起点坐标，改坐标必须在整个地图范围内 */	
	Active_Window_XY(0,0);				/* 设定工作窗口起点坐标,x和y之前的数据都不会写到内存 */
/* CanvasWidth * g_LcdHeight 为整个底图范围。宽度表示工作的区域，一般设置和内存底图宽度CanvasWidth相等 */
	Active_Window_WH(CanvasWidth, g_LcdHeight);		/* 设定工作窗口的高度和宽度 */

	Memory_XY_Mode();					/* 内存坐标寻址方法 */
//	Memory_Linear_Mode();

	Set_Serial_Flash_IF();				/* 设置串行Flash接口 */

	Goto_Pixel_XY(0,0);					/* 将像素坐标设为（0,0） */

	Display_ON();						/* 开显示 */
	
	bsp_DelayMS(1);						/* 需要延时一下 */
	
/* 设置LCD_ONOFF（按键引脚暂时没用到） */
	Set_GPIO_D_In_Out((1<<0) | (1<<1)| (1<<2) | (0<<3)| (0<<4)| (0<<5)| (1<<6)| (0<<7));
	Write_GPIO_D_7_0((1<<0) | (1<<1)| (1<<2) | (1<<3)| (1<<4)| (1<<5)| (1<<6)| (1<<7));
}

/*
*********************************************************************************************************
*	函 数 名: RA8876_DispOn
*	功能说明: 打开显示
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
void RA8876_DispOn(void)
{
	Display_ON();
}

/*
*********************************************************************************************************
*	函 数 名: RA8876_DispOff
*	功能说明: 关闭显示
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
void RA8876_DispOff(void)
{
	Display_OFF();
}

/*
*********************************************************************************************************
*	函 数 名: RA8876_ClrScr
*	功能说明: 根据输入的颜色值清屏.RA8875支持硬件单色填充。该函数仅对当前激活的显示窗口进行清屏. 显示
*			 窗口的位置和大小由 RA8875_SetDispWin() 函数进行设置
*	形    参:  _usColor : 背景色
*	返 回 值: 无
*********************************************************************************************************
*/
void RA8876_ClrScr(uint16_t _usColor)
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
	
	/* 如果正在调用LCD_BeginDrawAll画整个窗口,则清屏是清下一块SDRAM内存块。
	   如果一般的显示函数，则清屏是清当前SDRAM内存块。
	*/

	if (g_Drawing == 1)		/* 正在特效画图 */
	{
		BTE_Solid_Fill(0, CanvasWidth, width * 2, 0, _usColor, width, height);		/* 填充第3块内存 */
	}
	else
	{
		if (g_Interface == 0)
		{
			BTE_Solid_Fill(0, CanvasWidth, 0, 0, _usColor, width, height);			/* 填充第1块内存 */
		}
		else
		{
			BTE_Solid_Fill(0, CanvasWidth, width, 0, _usColor, width, height);		/* 填充第2块内存 */
		}
	}
}

/*
*********************************************************************************************************
*	函 数 名: RA8876_PutPixel
*	功能说明: 画1个像素
*	形    参:
*			_usX,_usY : 像素坐标
*			_usColor  :像素颜色
*	返 回 值: 无
*********************************************************************************************************
*/
void RA8876_PutPixel(uint16_t _usX, uint16_t _usY, uint16_t _usColor)
{
	if (g_LcdDirection == 0)					/* 横屏 */
	{
		Goto_Pixel_XY(_usX, _usY);				/* 设置点坐标 */
	}
	else if (g_LcdDirection == 1)				/* 横屏，旋转180度 */
	{
		Goto_Pixel_XY(g_LcdWidth - _usX, _usY);				
	}
	else if (g_LcdDirection == 2)				/* 竖屏 */
	{
		Goto_Pixel_XY(_usY, _usX);			
	}
	else if (g_LcdDirection == 3)				/* 竖屏，旋转180度 */
	{
		Goto_Pixel_XY(g_LcdHeight - _usY, _usX);				
	}
	
	RA8876_CmdWrite(0x04);						/* 内存SDRAM数据读/写接口 */
  	Check_Mem_WR_FIFO_not_Full();

#ifdef MCU_8bit_ColorDepth_8bpp	
	RA8876_DataWrite(_usColor);
#endif
	
#ifdef MCU_8bit_ColorDepth_16bpp	
	RA8876_DataWrite(_usColor);
	Check_Mem_WR_FIFO_not_Full();
	RA8876_DataWrite(_usColor >> 8);
#endif
	
#ifdef MCU_8bit_ColorDepth_24bpp	
	RA8876_DataWrite(_usColor);
	Check_Mem_WR_FIFO_not_Full();
	RA8876_DataWrite(_usColor >> 8);
	Check_Mem_WR_FIFO_not_Full();
	RA8876_DataWrite(_usColor >> 16);
#endif

#ifdef MCU_16bit_ColorDepth_16bpp			/* 选择16位16bpp颜色深度 */
	RA8876_DataWrite(_usColor);				/* 写入颜色 */
#endif

#ifdef MCU_16bit_ColorDepth_24bpp_Mode_2	
	RA8876_DataWrite(_usColor);
	Check_Mem_WR_FIFO_not_Full();
	RA8876_DataWrite(_usColor >> 16);
#endif
}

/*
*********************************************************************************************************
*	函 数 名: RA8876_GetPixel
*	功能说明: 读取1个像素
*	形    参:
*			_usX,_usY : 像素坐标
*			_usColor  :像素颜色
*	返 回 值: RGB颜色值
*********************************************************************************************************
*/
uint16_t RA8876_GetPixel(uint16_t _usX, uint16_t _usY)
{
	uint16_t usRGB;

	if (g_LcdDirection == 0)					/* 横屏 */
	{
		Goto_Pixel_XY(_usX, _usY);				/* 设置点坐标 */
	}
	else if (g_LcdDirection == 1)				/* 横屏，旋转180度 */
	{
		Goto_Pixel_XY(g_LcdWidth - _usX, _usY);				
	}
	else if (g_LcdDirection == 2)				/* 竖屏 */
	{
		Goto_Pixel_XY(_usY, _usX);			
	}
	else if (g_LcdDirection == 3)				/* 竖屏，旋转180度 */
	{
		Goto_Pixel_XY(g_LcdHeight - _usY, _usX);				
	}
	
	RA8876_CmdWrite(0x04);			/* 内存读写端口 */
	usRGB = RA8876_DataRead();		/* 第一次读取数据丢弃 */
	bsp_DelayUS(1);				/* 这里必须要延时。具体需要延时多久还需测试 */
	usRGB = RA8876_DataRead();	

	return usRGB;
}

/*
*********************************************************************************************************
*	函 数 名: RA8876_DrawLine
*	功能说明: 采用RA8876的硬件功能，在2点间画一条直线。
*	形    参:
*			_usX1, _usY1 :起始点坐标
*			_usX2, _usY2 :终止点Y坐标
*			_usColor     :颜色
*	返 回 值: 无
*********************************************************************************************************
*/
void RA8876_DrawLine(uint16_t _usX1 , uint16_t _usY1 , uint16_t _usX2 , uint16_t _usY2 , uint16_t _usColor)
{
	uint16_t temp;
	
	if (g_LcdDirection == 0)
	{
	}
	else if (g_LcdDirection == 1) 
	{
		/* X轴镜像、Y轴镜像 */
		_usX1 = g_LcdWidth - _usX1;
		_usY1 = _usY1; 
		
		_usX2 = g_LcdWidth - _usX2;
		_usY2 = _usY2; 
	}
	else if (g_LcdDirection == 2)
	{
		temp = _usX1;
		_usX1 = _usY1;
		_usY1 = temp;
		
		temp = _usX2;
		_usX2 = _usY2;
		_usY2 = temp;
	}
	else if (g_LcdDirection == 3)
	{
		temp = _usY1;
		_usY1 = _usX1;
		_usX1 = g_LcdHeight - temp;
		
		temp = _usY2;
		_usY2 = _usX2;
		_usX2 = g_LcdHeight - temp;
	}
	
	Draw_Line(_usColor, _usX1 , _usY1 , _usX2 , _usY2);
}

/*
*********************************************************************************************************
*	函 数 名: RA8876_DrawRect
*	功能说明: 采用RA8876硬件功能绘制矩形
*	形    参:
*			_usX,_usY:矩形左上角的坐标
*			_usHeight :矩形的高度
*			_usWidth  :矩形的宽度
*			_usColor  :颜色
*	返 回 值: 无
*********************************************************************************************************
*/
void RA8876_DrawRect(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth, uint16_t _usColor)
{
	uint16_t temp;
	
	if (g_LcdDirection > 1)	/* 竖屏  */
	{
		temp = _usX;
		_usX = _usY;
		_usY = temp;
		
		temp = _usHeight;
		_usHeight = _usWidth;
		_usWidth = temp;
	}	
	
	if (g_LcdDirection == 0)				/* 正方向 */
	{
		Draw_Square(_usColor, _usX, _usY, _usX + _usWidth - 1, _usY + _usHeight - 1);
	}
	else if (g_LcdDirection == 1) 			/* 180°旋转 */
	{
		_usX = g_LcdWidth - _usX;
		_usY = _usY; 
		Draw_Square(_usColor, _usX - 1, _usY, _usX - (_usWidth), _usY + (_usHeight - 1));
	}
	else if (g_LcdDirection == 2)			/* 90°旋转 */
	{
		Draw_Square(_usColor, _usX, _usY, _usX + _usWidth - 1, _usY + _usHeight - 1);
	}
	else if (g_LcdDirection == 3)			/* 270°旋转 */
	{
		_usX = _usX;
		_usY = g_LcdWidth - (_usY + _usHeight);
		
		_usX = g_LcdHeight - _usX;
		_usY = g_LcdWidth - _usY;
		
		Draw_Square(_usColor, _usX - 1, _usY - 1, _usX - (_usWidth), _usY - (_usHeight));
	}
}

/*
*********************************************************************************************************
*	函 数 名: RA8875_DrawCircle
*	功能说明: 绘制一个圆，笔宽为1个像素
*	形    参:
*			_usX,_usY  :圆心的坐标
*			_usRadius  :圆的半径
*			_usColor  :颜色
*	返 回 值: 无
*********************************************************************************************************
*/
void RA8876_DrawCircle(uint16_t _usX, uint16_t _usY, uint16_t _usRadius, uint16_t _usColor)
{
	uint16_t temp;
	
	if (g_LcdDirection > 1)	/* 竖屏  */
	{	
		temp = _usX;
		_usX = _usY;
		_usY = temp;
	}

	if (g_LcdDirection == 0)				/* 正方向 */
	{
	}
	else if (g_LcdDirection == 1) 			/* 180°旋转 */
	{
		_usX = g_LcdWidth - _usX;
		_usY = _usY;
	}
	else if (g_LcdDirection == 2)			/* 90°旋转 */
	{
	}
	else if (g_LcdDirection == 3)			/* 270°旋转 */
	{
		_usX = g_LcdHeight - _usX;
		_usY = _usY; 
	}	
	
	Draw_Circle(_usColor, _usX, _usY, _usRadius);
}

/*
*********************************************************************************************************
*	函 数 名: RA8876_FillRect
*	功能说明: 采用RA8876硬件功能填充一个矩形为单色
*	形    参:
*			_usX,_usY:矩形左上角的坐标
*			_usHeight :矩形的高度
*			_usWidth  :矩形的宽度
*			_usColor  :颜色
*	返 回 值: 无
*********************************************************************************************************
*/
void RA8876_FillRect(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth, uint16_t _usColor)
{
	uint16_t temp;
	
	if (g_LcdDirection > 1)	/* 竖屏  */
	{
		temp = _usX;
		_usX = _usY;
		_usY = temp;
		
		temp = _usHeight;
		_usHeight = _usWidth;
		_usWidth = temp;
	}	
	
	if (g_LcdDirection == 0)				/* 正方向 */
	{
		Draw_Square_Fill(_usColor, _usX, _usY, _usX + _usWidth - 1, _usY + _usHeight - 1);
	}
	else if (g_LcdDirection == 1) 			/* 180°旋转 */
	{
		_usX = g_LcdWidth - _usX;
		_usY = _usY; 
		Draw_Square_Fill(_usColor, _usX, _usY, _usX - (_usWidth), _usY + (_usHeight - 1));
	}
	else if (g_LcdDirection == 2)			/* 90°旋转 */
	{
		Draw_Square_Fill(_usColor, _usX, _usY, _usX + _usWidth - 1, _usY + _usHeight - 1);
	}
	else if (g_LcdDirection == 3)			/* 270°旋转 */
	{
		_usX = _usX;
		_usY = g_LcdWidth - (_usY + _usHeight);
		
		_usX = g_LcdHeight - _usX;
		_usY = g_LcdWidth - _usY;
		
		Draw_Square_Fill(_usColor, _usX, _usY, _usX - (_usWidth), _usY - (_usHeight));
	}
}

/*
*********************************************************************************************************
*	函 数 名: RA8876_DrawBMP
*	功能说明: 在LCD上显示一个BMP位图，位图点阵扫描次序:从左到右，从上到下
*	形    参:
*			_usX, _usY : 图片的坐标
*			_usHeight  :图片高度
*			_usWidth   :图片宽度
*			_ptr       :图片点阵指针
*	返 回 值: 无
*********************************************************************************************************
*/
void RA8876_DrawBMP(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth, uint16_t *_ptr)
{
	uint16_t temp;
	
	if (g_LcdDirection == 0)
	{
	}
	else if (g_LcdDirection == 1)	/* 横屏旋转180° */
	{
		_usX = g_LcdWidth - _usWidth;
		_usY = _usY;
	}
	else if (g_LcdDirection == 2)	/* 竖屏旋转90° */
	{
		temp = _usHeight;
		_usHeight = _usWidth;
		_usWidth = temp;
	}
	MPU16_16bpp_Memory_Write(_usX, _usY, _usWidth, _usHeight, _ptr);
}
	
/*
*********************************************************************************************************
*	函 数 名: RA8876_FillCircle
*	功能说明: 填充一个圆
*	形    参:
*			_usX,_usY  :圆心的坐标
*			_usRadius  :圆的半径
*			_usColor   :颜色
*	返 回 值: 无
*********************************************************************************************************
*/
void RA8876_FillCircle(uint16_t _usX, uint16_t _usY, uint16_t _usRadius, uint16_t _usColor)
{
	uint16_t temp;
	
	if (g_LcdDirection > 1)	/* 竖屏  */
	{	
		temp = _usX;
		_usX = _usY;
		_usY = temp;
	}

	if (g_LcdDirection == 0)				/* 正方向 */
	{
	}
	else if (g_LcdDirection == 1) 			/* 180°旋转 */
	{
		_usX = g_LcdWidth - _usX;
		_usY = _usY;
	}
	else if (g_LcdDirection == 2)			/* 90°旋转 */
	{
	}
	else if (g_LcdDirection == 3)			/* 270°旋转 */
	{
		_usX = g_LcdHeight - _usX;
		_usY = _usY; 
	}	
	
	Draw_Circle_Fill(_usColor, _usX, _usY, _usRadius);
}

/*
*********************************************************************************************************
*	函 数 名: RA8875_DispBmpInFlash
*	功能说明: 显示RA8875外挂串行Flash中的BMP位图.
*			    位图格式:从左到右 从上到下扫描, 每个像素2字节, RGB = 565 格式, 高位在前.
*	形    参:
*			_usX, _usY : 图片的坐标
*			_usHeight  :图片高度
*			_usWidth   :图片宽度
*			_uiFlashAddr       :串行Flash地址
*	返 回 值: 无
*********************************************************************************************************
*/
void RA8876_DispBmpInFlash(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth,
	uint32_t _uiFlashAddr)
{	
	/* pdf 179页 7-10-1 连续内存直接存取模式
		1. 设定工作窗口范围 (REG[30h] ~REG[37h])和内存写入位置 (REG[46h] ~REG[49h])
		2. 设定Serial Flash/ROM 组态 (REG[05h])
		3. 设定 内存直接存取数据来源起始位置 (REG[B0h] ~REG[B2h])
		4. 设定 内存直接存取区块宽度 (REG[B4h] 和REG[B5h])
		5. 设定 内存直接存取区块高度 (REG[B6h] 和 REG[B7h])
		6. 设定内存直接存取来源图片宽度 (REG[B8h] 和 REG[B9h])
		7. 开启内存直接存取为区块搬移模式 (REG[BFh] bit 1)
		8. 开启内存直接存取起始讯号且检查内存直接存取忙碌讯号 (REG[BFh] bit 0)
	*/
	
/*
	
. S0 的地址缓存器是 REG [93h], REG[94h], REG[95h], REG[96h], REG[97h], REG[98h],
REG[99h], REG[9Ah], REG[9Bh], REG[9Ch]
2. S1 的地址缓存器是[9Dh], REG[9Eh], REG[9Fh], REG[A0h], REG[A1h] , REG[A2h],
REG[A3h], REG[A4h], REG[A5h], REG[A6h]
3. D 的地址缓存器是 REG [A7h], REG[A8h], REG[A9h], REG[AAh], REG [ABh], REG[ACh],
REG[ADh], REG[AEh], REG[AFh], REG[B0h]	
*/
#ifdef MCU_8bit_ColorDepth_8bpp					   //setting in UserDef.h
	DMA_24bit(1,0,0,0,1024,640,1024,0);
#endif
#ifdef MCU_8bit_ColorDepth_16bpp				   //setting in UserDef.h
	DMA_24bit(1,0,0,0,1024,640,1024,655360);
#endif
#ifdef MCU_8bit_ColorDepth_24bpp				   //setting in UserDef.h
	DMA_24bit(1,0,0,0,1024,640,1024,1966080);
#endif
#ifdef MCU_16bit_ColorDepth_16bpp				   //setting in UserDef.h
//	DMA_24bit(1,0,0,0,1024,640,1024,655360);
	DMA_24bit(1,0,_usX,_usY,_usWidth,_usHeight,_usWidth,_uiFlashAddr);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_1		   //setting in UserDef.h
	DMA_24bit(1,0,0,0,1024,640,1024,1966080);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_2		   //setting in UserDef.h
	DMA_24bit(1,0,0,0,1024,640,1024,1966080);
#endif
}

/*
*********************************************************************************************************
*	函 数 名: RA8875_DispStr
*	功能说明: 显示字符串，字符点阵来自于RA8875外接的字库芯片，支持汉字.
*			文字颜色、背景颜色、是否通透由其他的函数进行设置
*			void RA8875_SetFrontColor(uint16_t _usColor);
*			void RA8875_SetBackColor(uint16_t _usColor);
*	形    参:  _usX, _usY : 显示位置（坐标）
*			 _ptr : AascII字符串，已0结束
*	返 回 值: 无
*********************************************************************************************************
*/
void RA8876_DispStr(uint8_t SCS, uint16_t _usX, uint16_t _usY, uint32_t FontColor, uint32_t BackGroundColor, char *_ptr)
{	
	uint16_t temp;
	
	if (g_LcdDirection == 0)
	{
		Font_0_degree();		/* 汉字不旋转 */
	}
	else if (g_LcdDirection == 2)
	{
		temp = _usX;
		_usX = _usY;
		_usY = temp;
		
		Font_90_degree();		/* 汉字旋转90° */
	}
	
	Select_SFI_Font_Mode();			/* [B7H]bit6串行Flash选择模式：字符模式-使用在CGROM */

	/* 设置外部字体芯片型号为 GT23L32S4W, 编码为GB2312 */
	CGROM_Select_Genitop_FontROM();	/* [CCH]bit7-6字符源选择：外部CGROM为字符来源（集通闪存） */

	SPI_Clock_Period(0);		/* [BBH]SPI时钟周期，SPI CLK = System Clock / 2*(Clk+1) 8M串行Flash的访问速度:SPI 时钟频率:80MHz(max.) */
	Set_GTFont_Decoder(0x01);   /* [CFH]集通字符解码：GB2312 */
	if(SCS == 0)
	{
		Select_SFI_0();				/* 串行flash选择：串行闪存/ROM 0 被选择 */
	}

	if(SCS == 1)
	{
		Select_SFI_1();
	}
#ifdef MCU_8bit_ColorDepth_8bpp	
	Foreground_color_256(FontColor);		/* 设置字体色 */
	Background_color_256(BackGroundColor);	/* 设置背景色 */
#endif
#ifdef MCU_8bit_ColorDepth_16bpp	
	Foreground_color_65k(FontColor);
	Background_color_65k(BackGroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_24bpp	
	Foreground_color_16M(FontColor);
	Background_color_16M(BackGroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_16bpp	
	Foreground_color_65k(FontColor);		/* 设置字体颜色 */
	Background_color_65k(BackGroundColor);	/* 设置背景颜色 */
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_1	
	Foreground_color_16M(FontColor);
	Background_color_16M(BackGroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_2	
	Foreground_color_16M(FontColor);
	Background_color_16M(BackGroundColor);
#endif
	Text_Mode();					/* 设置为文本模式 */
	//Active_Window_XY(0, 0);		/* 整个在外面做了 */
	if (g_LcdDirection > 1)			/* 竖屏 */
	{
		Active_Window_WH(g_LcdHeight, g_LcdWidth);
	}
	else
	{
		Active_Window_WH(g_LcdWidth, g_LcdHeight);
	}
	Goto_Text_XY(_usX, _usY);		/* 设置文本显示位置，注意文本模式的写入光标和图形模式的写入光标是不同的寄存器 */
	//  sprintf(tmp1,"%s",tmp2); 
	//  Show_String(tmp1);
	Show_String(_ptr);		/* 显示字符串，并设置为图形模式 */
}

/*
*********************************************************************************************************
*	函 数 名: RA8876_SetFont
*	功能说明: 文本模式，设置文字字体、行距和字距
*	形    参:
*			_ucFontType : 字体类型: RA_FONT_16, RA_FONT_24, RA_FONT_32
*			_ucLineSpace : 行距，像素单位 (0-31)
*			_ucCharSpace : 字距，像素单位 (0-63)
*
*	返 回 值: 无
*********************************************************************************************************
*/
void RA8876_SetFont(uint8_t _ucFontType, uint8_t _ucLineSpace, uint8_t _ucCharSpace)
{
	/*
		[D0H]在文字模式下，用来设定文字间的行距 (单位: 像素) 。
		只有低5个bit有效，0-31
	*/
	if (_ucLineSpace >31)
	{
		_ucLineSpace = 31;
	}
	Font_Line_Distance(_ucLineSpace);		/* 设置行间距 */
	
	/*
		[D1H] 设置字符间距（像素单位，0-63），和字体（16*16，24*24，32*32）
	*/
	if (_ucCharSpace > 63)
	{
		_ucCharSpace = 63;	
	}
	Set_Font_to_Font_Width(_ucCharSpace);	/* 设置字符间距 */
	
	if (_ucFontType > RA_FONT_32)
	{
		_ucFontType = RA_FONT_16;
	}
	
	if (_ucFontType == RA_FONT_16)
	{
		Font_Select_8x16_16x16();			/* 设置字体尺寸 */
	}
	else if (_ucFontType == RA_FONT_24)
	{
		Font_Select_12x24_24x24();
	}
	else if (_ucFontType == RA_FONT_32)
	{
		Font_Select_16x32_32x32();
	}
}

/*
*********************************************************************************************************
*	函 数 名: RA8876_SetDirection
*	功能说明: 设置显示方向。
*	形    参:  _ucDir : 显示方向代码 0 横屏正常, 1=横屏180度翻转, 2=竖屏, 3=竖屏180度翻转
*	返 回 值: 无
*********************************************************************************************************
*/
void RA8876_SetDirection(uint8_t _ucDir)
{
/*
	Display Configuration Register[12H]
bit3:
	VDIR
	Vertical Scan direction
	0 : 由上到下。
	1 : 由下到上
*/
	if (_ucDir == 0)
	{
		VSCAN_T_to_B();		/* 横屏，正常 */
		MemWrite_Left_Right_Top_Down();
	}
	else if (_ucDir == 1)
	{
		VSCAN_B_to_T();		/* 横屏，180° */
		MemWrite_Right_Left_Top_Down();		/* 01b */
	}
	else if (_ucDir == 2)
	{
		VSCAN_B_to_T();		/* 竖屏，90° */
		MemWrite_Top_Down_Left_Right();		/* 10b */
	}
	else if (_ucDir == 3)
	{
		VSCAN_T_to_B();		/* 竖屏，270° */
		MemWrite_Left_Right_Top_Down();
	}
	
	if (_ucDir > 1)	/* 竖屏  */
	{
		uint16_t temp;
		
		if (g_LcdHeight < g_LcdWidth)
		{
			temp = g_LcdHeight;
			g_LcdHeight = g_LcdWidth;
			g_LcdWidth = temp;
		}
		
	}		
	else	/* 横屏 */
	{
		uint16_t temp;
		
		if (g_LcdHeight > g_LcdWidth)
		{
			temp = g_LcdHeight;
			g_LcdHeight = g_LcdWidth;
			g_LcdWidth = temp;
		}
	}
}

/*
*********************************************************************************************************
*	函 数 名: RA8876_SetBackLight
*	功能说明: 配置RA8875芯片的PWM1相关寄存器，控制LCD背光
*	形    参:  _bright 亮度，0是灭，255是最亮
*	返 回 值: 无
*********************************************************************************************************
*/
void RA8876_SetBackLight(uint8_t _bright)
{
/* 
	PWM CLK = (Core CLK / Prescalar ) /2^ divided clock 
	PWM CLK = (120*1000*1000) / 10 /2 = 6M
	PWM频率 = PWM CLK / TCNTBn = 6*1000*1000/255 = 23529 = 23.5KHz
	占空比 = TCMPBn / TCNTBn
*/
	/* 1: 打开PWM    1: 1分频   10: Prescalar   255: TCNTBn   _bright: TCMPBn */
	
	if (_bright == 0)
	{
		PWM1(0,1,10,255,_bright); 					/* 关PWM背光 */
	}
	else
	{
		PWM1(1,1,10,255,_bright); 					/* 开PWM背光 */
	}
}

/*
*********************************************************************************************************
*	函 数 名: RA8876_FillRoundRect
*	功能说明: 采用RA8876硬件功能填充圆角矩形
*	形    参:
*			_usX,_usY:矩形左上角的坐标
*			_usHeight :矩形的高度
*			_usWidth  :矩形的宽度
*			_usArc    :圆角的弧
*			_usColor  :颜色
*	返 回 值: 无
*********************************************************************************************************
*/
void RA8876_FillRoundRect(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth, 
	uint16_t _usArc, uint16_t _usColor)
{
/*
	RA8876 支持绘制圆角矩形，使用者可以使用少量的 MPU 周期达成绘制圆角矩形。经由设定圆角矩形起始
点 REG[68h~6Ch] ，结束点 REG[6Dh~6Fh]，圆角矩形长短轴半径 REG[77h~7Ah]，颜色 REG[D2h~D4h]，
最后设定绘制图形为圆角矩形 REG[76h] Bit5~4 为 11b，并且致能 REG[76h] Bit7 = 1，那么 RA8876 将会在
底图上绘制圆角矩形，更进一步的，使用者可以设定填满功能 REG[76h] Bit6 = 1。
	
	 ---------------->---
	|(_usX，_usY)        |
	V                    V  _usHeight
	|                    |
	 ---------------->---
		  _usWidth
*/
	uint16_t temp;
		
	if (g_LcdDirection > 1)	/* 竖屏  */
	{
		temp = _usX;
		_usX = _usY;
		_usY = temp;

		temp = _usHeight;
		_usHeight = _usWidth;
		_usWidth = temp;		
	}
	
	if (g_LcdDirection == 0)			/* 横屏 */
	{
		Draw_Circle_Square_Fill(_usColor, _usX, _usY,  _usX + _usWidth - 1, _usY + _usHeight - 1, _usArc, _usArc);
	}
	else if (g_LcdDirection == 1)		/* 横屏,旋转180°*/
	{
		_usX = g_LcdWidth - _usX;
		_usY = _usY;
		
		Draw_Circle_Square_Fill(_usColor, _usX, _usY,  _usX - (_usWidth - 1), _usY + (_usHeight - 1), _usArc, _usArc);
	}
	else if (g_LcdDirection == 2)		/* 竖屏，旋转90° */
	{	
		Draw_Circle_Square_Fill(_usColor, _usX, _usY,  _usX + (_usWidth - 1), _usY + (_usHeight - 1), _usArc, _usArc);
	}
	else if (g_LcdDirection == 3)		/* 竖屏，旋转270° */
	{
		_usX = g_LcdHeight - (_usX + _usWidth - 1);
		_usY = _usY;
		
		Draw_Circle_Square_Fill(_usColor, _usX, _usY,  _usX + _usWidth - 1, _usY + _usHeight - 1, _usArc, _usArc);
	}
}

/*
*********************************************************************************************************
*	函 数 名: RA8876_DrawRoundRect
*	功能说明: 采用RA8875硬件功能绘制圆角矩形
*	形    参:
*			_usX,_usY:矩形左上角的坐标
*			_usHeight :矩形的高度
*			_usWidth  :矩形的宽度
*			_usArc    :圆角的弧
*			_usColor  :颜色
*	返 回 值: 无
*********************************************************************************************************
*/
void RA8876_DrawRoundRect(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth, 
	uint16_t _usArc, uint16_t _usColor)
{
/*
	RA8876 支持绘制圆角矩形，使用者可以使用少量的 MPU 周期达成绘制圆角矩形。经由设定圆角矩形起始
点 REG[68h~6Ch] ，结束点 REG[6Dh~6Fh]，圆角矩形长短轴半径 REG[77h~7Ah]，颜色 REG[D2h~D4h]，
最后设定绘制图形为圆角矩形 REG[76h] Bit5~4 为 11b，并且致能 REG[76h] Bit7 = 1，那么 RA8876 将会在
底图上绘制圆角矩形，更进一步的，使用者可以设定填满功能 REG[76h] Bit6 = 1。
	
	---------------->---
	|(_usX，_usY)        |
	V                    V  _usHeight
	|                    |
	 ---------------->---
		  _usWidth
*/
	uint16_t temp;
		
	if (g_LcdDirection > 1)	/* 竖屏  */
	{	
		temp = _usX;
		_usX = _usY;
		_usY = temp;

		temp = _usHeight;
		_usHeight = _usWidth;
		_usWidth = temp;		
	}
	
	if (g_LcdDirection == 0)			/* 横屏 */
	{
		Draw_Circle_Square(_usColor, _usX, _usY,  _usX + _usWidth - 1, _usY + _usHeight - 1, _usArc, _usArc);
	}
	else if (g_LcdDirection == 1)		/* 横屏,旋转180°*/
	{
		_usX = g_LcdWidth - _usX;
		_usY = _usY;
		
		Draw_Circle_Square(_usColor, _usX, _usY,  _usX - (_usWidth - 1), _usY + (_usHeight - 1), _usArc, _usArc);
	}
	else if (g_LcdDirection == 2)		/* 竖屏，旋转90° */
	{	
		Draw_Circle_Square(_usColor, _usX, _usY,  _usX + (_usWidth - 1), _usY + (_usHeight - 1), _usArc, _usArc);
	}
	else if (g_LcdDirection == 3)		/* 竖屏，旋转270° */
	{
		_usX = g_LcdHeight - (_usX + _usWidth - 1);
		_usY = _usY;
		
		Draw_Circle_Square(_usColor, _usX, _usY,  _usX + _usWidth - 1, _usY + _usHeight - 1, _usArc, _usArc);
	}
}

/*
*********************************************************************************************************
*	函 数 名: RA8876_SetTextTransp
*	功能说明: 文本模式，设置文字背景是否通透
*	形    参:_Enable : 0表示不通透， 1表示通透
*	返 回 值: 无
*********************************************************************************************************
*/
void RA8876_SetTextTransp(uint8_t _Enable)
{
	/*
		pdf 第203页		[CDH]
		bit7 用于对齐，一般不用，缺省设0
		bit6 0 : 字符的数据 0 会显示为指定的颜色 1 : 字符的数据 0 会显示为底图 (Canvas)
		bit4 用于旋转90读，一般不用，缺省设0
		bit3-2 水平放大倍数
		bit1-0 垂直放大倍数
	*/
	if (_Enable == 0)
	{
		Font_Background_select_Color();
	}
	else
	{
		Font_Background_select_Original_Canvas();
	}
}

/*
*********************************************************************************************************
*	函 数 名: RA8876_SetTextZoom
*	功能说明: 文本模式，设置文字的放大模式，1X,2X,3X, 4X
*	形    参:
*			_ucHSize : 文字水平放大倍数，RA_SIZE_X1、RA_SIZE_X2、RA_SIZE_X3、RA_SIZE_X4
*			_ucVSize : 文字处置放大倍数，RA_SIZE_X1、RA_SIZE_X2、RA_SIZE_X3、RA_SIZE_X4		
*	返 回 值: 无
*********************************************************************************************************
*/
void RA8876_SetTextZoom(uint8_t _ucHSize, uint8_t _ucVSize)
{
	/*
		pdf 第22页		[22H]
		bit7 用于对齐，一般不用，缺省设0
		bit6 用于通透，0 表示文字背景不通透， 1表示通透
		bit4 用于旋转90读，一般不用，缺省设0
		bit3-2 水平放大倍数
		bit1-0 垂直放大倍数
	*/
	/* 水平尺寸 */
	if (_ucHSize == RA_SIZE_X1)
	{
		Font_Width_X1();
	}
	else if (_ucHSize == RA_SIZE_X2)
	{
		Font_Width_X2();
	}
	else if (_ucHSize == RA_SIZE_X3)
	{
		Font_Width_X3();
	}
	else if (_ucHSize == RA_SIZE_X4)
	{
		Font_Width_X4();
	}
	
	/* 垂直尺寸 */
	if (_ucVSize == RA_SIZE_X1)
	{
		Font_Height_X1();
	}
	else if (_ucVSize == RA_SIZE_X2)
	{
		Font_Height_X2();
	}
	else if (_ucVSize == RA_SIZE_X3)
	{
		Font_Height_X3();
	}
	else if (_ucVSize == RA_SIZE_X4)
	{
		Font_Height_X4();
	}
}


/********************************************************************************************************
*																										*
*							下面为官方例子提供的API代码													*
*																										*
*********************************************************************************************************/

/*
*********************************************************************************************************
*	函 数 名: RA8876_HW_Reset
*	功能说明: RA8876硬件复位函数
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void RA8876_HW_Reset(void)
{
/*
	RA8876_RST = 0;
	delay_ms(1);
	RA8876_RST = 1;
	Check_IC_ready();
*/
	GPIO_SetBits(GPIOI, GPIO_Pin_8);
	bsp_DelayMS(1);

	GPIO_ResetBits(GPIOI, GPIO_Pin_8);
	bsp_DelayMS(1); // XnRST have to keep low at least 256 OSC clocks. 

	GPIO_SetBits(GPIOI, GPIO_Pin_8 );	
	bsp_DelayMS(1);
	
	//=====
	System_Check_Temp();	//double check RA8876_HW_Reset(); 
}

/*
*********************************************************************************************************
*	函 数 名: System_Check_Temp
*	功能说明: 检测RA8876是否OK
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void System_Check_Temp(void)
{
	uint8_t i=0;
	uint8_t temp=0;
	uint8_t system_ok=0;
	
/*
	状态缓存器 bit1
Operation mode status
	0: Normal 操作。
	1: Inhibit 操作。
	Inhibit 操作表示 RA8876 内部正在进行内部复位或是开机显示或
	是进入了省电模式当中。
	在省电模式模式，此位会维持在 1 直到 PLL 频率被停止。所以这个
	bit 与 REG([DFh]bit[7]) 会有一点点的时间差。
*/	
	do
	{
		if((RA8876_StatusRead()&0x02)==0x00)
		{
			bsp_DelayMS(1); 					/* 输入MCU接口太快，也许需要一些延时 */
			RA8876_CmdWrite(0x01);
			bsp_DelayMS(1);  					/* 输入MCU接口太快，也许需要一些延时 */
			temp =RA8876_DataRead();
			if((temp & 0x80)==0x80)
			{
				system_ok=1;
				i=0;
			}
			else
			{
				bsp_DelayMS(1); 					/* 输入MCU接口太快，也许需要一些延时 */
				RA8876_CmdWrite(0x01);
				bsp_DelayMS(1); 					/* 输入MCU接口太快，也许需要一些延时 */
				RA8876_DataWrite(0x80);
			}
		}
		else
		{
			system_ok=0;
			i++;
		}
		if(system_ok==0 && i==5)
		{
			RA8876_HW_Reset(); //note1
			i=0;
		}
	}while(system_ok==0);
}
//==============================================================================
void LCD_RegisterWrite(uint8_t Cmd,uint8_t Data)
{
	Check_2D_Busy();
//	bsp_DelayUS(1);
	RA8876_CmdWrite(Cmd);
//	RA8875_Delaly1ms();
	RA8876_DataWrite(Data);
}  
//---------------------//
uint8_t LCD_RegisterRead(uint8_t Cmd)
{
	uint8_t temp;
	Check_2D_Busy();
	RA8876_CmdWrite(Cmd);
	temp=RA8876_DataRead();

	return temp;
}
//==============================================================================
void Check_Mem_WR_FIFO_not_Full(void)
{
/*[Status Register] bit7
Host Memory Write FIFO full
0: Memory Write FIFO is not full.
1: Memory Write FIFO is full.
Only when Memory Write FIFO is not full, MPU may write another
one pixel.
*/ 
	uint16_t i;

	/* you can using the following */
	// Please according to your MCU speed to modify. // 叫ㄌ沮眤MCU硉э丁
	for(i=0;i<1000;i++)
	{
		if( (RA8876_StatusRead()&0x80)==0 ){break;}
	}
	/* or the following */
	do{	}
	while( RA8876_StatusRead()&0x80 );
}
//---------------------//
void Check_Mem_WR_FIFO_Empty(void)
{
/*[Status Register] bit6
Host Memory Write FIFO empty
0: Memory Write FIFO is not empty.
1: Memory Write FIFO is empty.
When Memory Write FIFO is empty, MPU may write 8bpp data 64
pixels, or 16bpp data 32 pixels, 24bpp data 16 pixels directly.
*/	
	uint16_t i;		 

	/* you can using the following */
	// Please according to your MCU speed to modify. // 叫ㄌ沮眤MCU硉э丁
	for(i=0;i<1000;i++)
	{
		if( (RA8876_StatusRead()&0x40)==0x40 ){break;}
	}
	/* or the following */
	do{	}
	while( (RA8876_StatusRead()&0x40) == 0x00 );
}
//---------------------//
void Check_Mem_RD_FIFO_not_Full(void)
{
/*	[Status Register] bit5
Host Memory Read FIFO full
0: Memory Read FIFO is not full.
1: Memory Read FIFO is full.
When Memory Read FIFO is full, MPU may read 8bpp data 32
pixels, or 16bpp data 16 pixels, 24bpp data 8 pixels directly.
*/
	uint16_t i;

	/* you can using the following */
	// Please according to your MCU speed to modify. // 叫ㄌ沮眤MCU硉э丁
	for(i=0;i<1000;i++)
	{
		if( (RA8876_StatusRead()&0x20)==0x00 ){break;}
	}
	/* or the following */
	do{	}
	while( RA8876_StatusRead()&0x20 );
}
//---------------------//
void Check_Mem_RD_FIFO_Full(void)
{
/*	[Status Register] bit5
Host Memory Read FIFO full
0: Memory Read FIFO is not full.
1: Memory Read FIFO is full.
When Memory Read FIFO is full, MPU may read 8bpp data 32
pixels, or 16bpp data 16 pixels, 24bpp data 8 pixels directly.
*/
	uint16_t i;

	/* you can using the following */
	// Please according to your MCU speed to modify. // 叫ㄌ沮眤MCU硉э丁
	for(i=0;i<1000;i++)
	{
		if( (RA8876_StatusRead()&0x20)==0x20 ){break;}
	}
	/* or the following */
	do{	}
	while( (RA8876_StatusRead()&0x20)==0x00  );
}
//---------------------//
void Check_Mem_RD_FIFO_not_Empty(void)
{
/*	[Status Register] bit4
Host Memory Read FIFO empty
0: Memory Read FIFO is not empty.
1: Memory Read FIFO is empty.
*/
	uint16_t i;

	/* you can using the following */
	// Please according to your MCU speed to modify. // 叫ㄌ沮眤MCU硉э丁
	for(i=0;i<1000;i++)
	{
		if( (RA8876_StatusRead()&0x10)==0x00 ){break;}
	}
	/* or the following */
	do{	}
	while( RA8876_StatusRead()&0x10 );
}

/*
*********************************************************************************************************
*	函 数 名: Check_Mem_RD_FIFO_Empty
*	功能说明: 检查内存读fifo是否为空
*	形    参：无
*	返 回 值: 1：为空   0：不为空
*********************************************************************************************************
*/
uint8_t Check_Mem_RD_FIFO_Empty(void)
{
/*	[Status Register] bit4
Host Memory Read FIFO empty
0: Memory Read FIFO is not empty.
1: Memory Read FIFO is empty.
*/
	unsigned short i;

	/* you can using the following */
	// Please according to your MCU speed to modify. // 叫ㄌ沮眤MCU硉э丁
	for(i=0;i<1000;i++)
	{
		if( (RA8876_StatusRead()&0x10)==0x10 ){return 1;}
	}
	
	return 0;
}

//---------------------//
void Check_2D_Busy(void)
{
/*	[Status Register] bit3
Core task is busy
Following task is running:
BTE, Geometry engine, Serial flash DMA, Text write or Graphic write
0: task is done or idle.
1: task is busy.													
*/
	uint16_t i;
	
	/* you can using the following */
	// Please according to your MCU speed to modify. // 叫ㄌ沮眤MCU硉э丁
		for(i=0;i<10000;i++)
		{
			if( (RA8876_StatusRead()&0x08)==0x00 ){break;}
		}

	/* or the following */
		do{	}
		while(( RA8876_StatusRead()&0x08)==0x08 );

}  

/*
*********************************************************************************************************
*	函 数 名: Check_SDRAM_Ready
*	功能说明: 检测SDRAM是否准备就绪
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/ 
void Check_SDRAM_Ready(void)
{
/*	[Status Register] bit2
SDRAM ready for access
0: SDRAM is not ready for access
1: SDRAM is ready for access		
*/	
	uint16_t i;
	/* you can using the following */
	// Please according to your MCU speed to modify. // 叫ㄌ沮眤MCU硉э丁
	for(i=0;i<10000;i++)
	{
		if( (RA8876_StatusRead()&0x04)==0x04 ){break;}
	}
	/* or the following */
	do{	}
	while( (RA8876_StatusRead()&0x04) == 0x00 );
}

/*
*********************************************************************************************************
*	函 数 名: Check_IC_ready
*	功能说明: 检测芯片是否准备好
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/ 
void Check_IC_ready(void)
{
/*	状态缓存器 bit1
Operation mode status
0: Normal 操作。
1: Inhibit 操作。
Inhibit 操作表示 RA8876 内部正在进行内部复位或是开机显示或
是进入了省电模式当中。
在省电模式模式，此位会维持在 1 直到 PLL 频率被停止。所以这个
bit 与 REG([DFh]bit[7]) 会有一点点的时间差。	
*/	
	uint16_t i;
	/* you can using the following */
	// Please according to your MCU speed to modify. // 叫ㄌ沮眤MCU硉э丁
	for(i=0;i<10000;i++)
	{
		if( (RA8876_StatusRead()&0x02)==0x00 ){break;}
	}
	/* or the following */
	do{	}
	while(RA8876_StatusRead() & 0x02);
}

//---------------------//
uint8_t Power_Saving_Status(void)
{
/*	[Status Register] bit1
Operation mode status
0: Normal operation state
1: Inhibit operation state
Inhibit operation state means internal reset event keep running or
initial display still running or chip enter power saving state.		
*/
  	  uint8_t temp;

	  if((RA8876_StatusRead()&0x02)==0x02)
	    temp = 1;
	  else
	    temp = 0;

	  return temp;
}

void Check_Power_is_Saving(void)
{  
/*	[Status Register] bit1
Operation mode status
0: Normal operation state
1: Inhibit operation state
Inhibit operation state means internal reset event keep running or
initial display still running or chip enter power saving state.		
*/
	uint16_t i;
	/* you can using the following */
	// Please according to your MCU speed to modify. // 叫ㄌ沮眤MCU硉э丁
	for(i=0;i<1000;i++)
	{
		if( (RA8876_StatusRead()&0x02)==0x02 ){break;}
	}
	/* or the following */
	do{	}
	while( (RA8876_StatusRead()&0x02) == 0x00 );
}
//---------------------//
void Check_NO_Interrupt(void)
{
/*	[Status Register] bit0
Interrupt pin state
0: without interrupt active
1: interrupt active		
*/	
	uint16_t i;
	/* you can using the following */
	// Please according to your MCU speed to modify. // 叫ㄌ沮眤MCU硉э丁
	for(i=0;i<1000;i++)
	{
		if( (RA8876_StatusRead()&0x01)==0x00 ){break;}
	}
	/* or the following */
	do{	}
	while( RA8876_StatusRead()&0x01 );
}
void Check_Interrupt_Occur(void)
{
/*	[Status Register] bit0
Interrupt pin state
0: without interrupt active
1: interrupt active		
*/	
	uint16_t i;
	/* you can using the following */
	// Please according to your MCU speed to modify. // 叫ㄌ沮眤MCU硉э丁
	for(i=0;i<1000;i++)
	{
		if( (RA8876_StatusRead()&0x01)==0x01 ){break;}
	}
	/* or the following */
	do{	}
	while( (RA8876_StatusRead()&0x01) == 0x00 );
}

//==============================================================================



/*
*********************************************************************************************************
*	函 数 名: RA8876_SW_Reset
*	功能说明: RA8876软件复位
*	形    参：无
*	返 回 值: 错误代码(无需处理)
*********************************************************************************************************
*/
void RA8876_SW_Reset(void)
{/*
Software Reset
0: Normal operation.
1: Software Reset.
Software Reset only reset internal state machine. Configuration
Registers value wonˇt be reset. So all read-only flag in the
register will return to its initial value. User should have proper
action to make sure flag is in desired state.
Note: The bit will auto clear after reset.
*/	
	uint16_t i;
	uint8_t temp;

	RA8876_CmdWrite(0x00);
	temp = RA8876_DataRead();
	temp |= 0x01;
	RA8876_DataWrite(temp);
	bsp_DelayUS(100);			// it must wait 100us after Software_Reset.

	// Please according to your MCU speed to modify. // 叫ㄌ沮眤MCU硉э丁
	for(i=0;i<100;i++)
	{
		if( (LCD_RegisterRead(0x01)&0x80)==0x80 )
		{break;}
	}

}
/*
*********************************************************************************************************
*	函 数 名: Enable_PLL
*	功能说明: RA8876时钟使能
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void Enable_PLL(void)
{
/*  0: PLL disable; allow change PLL parameter.
    1: PLL enable; cannot change PLL parameter.*/
/*
	芯片配置寄存器[01H]
	
bit7：
	Reconfigure PLL frequency
	对这个 bit 写“1”可以重新设定 PLL 频率。
	注
	a. 当使用者更改 PLL 相关参数，PLL 频率不会马上改变，使用
		者还必须再次将这个 bit 设定为 1，PLL 频率才会改变。
	b. 使用者可以读取(检查)这个 bit 以知道系统是否已经切换到
		PLL 频率，”1”表示 PLL 频率已经就绪并且切换成功	
	
*/
	static uint8_t temp;
	uint16_t i;

	RA8876_CmdWrite(0x01);
	temp = RA8876_DataRead();
	temp |= cSetb7;
	RA8876_DataWrite(temp);

	bsp_DelayUS(100);// PLL lock time = 1024 T OSC clocks, if OSC=10MHz, PLL lock time = 10 us.  

	/*check PLL was ready ( Please according to your MCU speed to modify. 叫ㄌ沮眤MCU硉э丁)	 */
	for(i=0;i<10000;i++)
	{
		RA8876_CmdWrite(0x01);
		temp=RA8876_DataRead();
		if( (temp&0x80)==0x80 ){break;}
	}
}

void Sent_XnWAIT_Status_When_CS_Low(void)
{/*  
	Mask XnWAIT on XnCS deassert
	0 : No mask
		XnWAIT keep assert if internal state keep busy and cannot 
		accept next R/W cycle, no matter XnCS assert/deassert. If
		MCU cycle cannot be extended while XnWAIT keep low, user
		should poll XnWAIT and wait it goes high then start next
		access.
	1 : Mask
		XnWAIT deassert when XnCS deassert. Use in MCU cycle can
		be extended by XnWAIT automatically.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x01);
	temp = RA8876_DataRead();
	temp |= cSetb6;
	RA8876_DataWrite(temp);    
}
void Sent_XnWAIT_Status_At_Any_Time(void)
{/*  
	Mask XnWAIT on XnCS deassert
	0 : No mask
		XnWAIT keep assert if internal state keep busy and cannot 
		accept next R/W cycle, no matter XnCS assert/deassert. If
		MCU cycle cannot be extended while XnWAIT keep low, user
		should poll XnWAIT and wait it goes high then start next
		access.
	1 : Mask
		XnWAIT deassert when XnCS deassert. Use in MCU cycle can
		be extended by XnWAIT automatically.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x01);
	temp = RA8876_DataRead();
	temp &= cClrb6;
	RA8876_DataWrite(temp);    
}
void Key_Scan_Enable(void)
{
/*  0: Disable.
    1: Enable.*/
	uint8_t temp;
	RA8876_CmdWrite(0x01);
	temp = RA8876_DataRead();
	temp |= cSetb5;
	RA8876_DataWrite(temp);    
}
void Key_Scan_Disable(void)
{
/*  0: Disable.
    1: Enable.*/
	uint8_t temp;
	RA8876_CmdWrite(0x01);
	temp = RA8876_DataRead();
	temp &= cClrb5;
	RA8876_DataWrite(temp);    
}

/*
*********************************************************************************************************
*	函 数 名: TFT_24bit
*	功能说明: 24bits TFT输出
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void TFT_24bit(void)//RA8876 only
{
/*  TFT Panel I/F Output pin Setting
00b: 24-bits TFT output.
01b: 18-bits TFT output.
10b: 16-bits TFT output.
11b: w/o TFT output.
Other unused TFT output pins are set as GPIO or Key function.*/
	uint8_t temp;
	RA8876_CmdWrite(0x01);
	temp = RA8876_DataRead();
	temp &= cClrb4;
    temp &= cClrb3;
	RA8876_DataWrite(temp);    
}

/*
*********************************************************************************************************
*	函 数 名: TFT_18bit
*	功能说明: 18bits TFT输出
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void TFT_18bit(void)//RA8876 only
{
/*  TFT Panel I/F Output pin Setting
00b: 24-bits TFT output.
01b: 18-bits TFT output.
10b: 16-bits TFT output.
11b: w/o TFT output.
Other unused TFT output pins are set as GPIO or Key function.*/
	uint8_t temp;
	RA8876_CmdWrite(0x01);
	temp = RA8876_DataRead();
	temp &= cClrb4;
    temp |= cSetb3;
	RA8876_DataWrite(temp);  
}

/*
*********************************************************************************************************
*	函 数 名: TFT_16bit
*	功能说明: 16bits TFT输出
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void TFT_16bit(void)//RA8876 only
{
/*  
	TFT Panel I/F Output pin Setting
00b: 24-bits TFT output.
01b: 18-bits TFT output.
10b: 16-bits TFT output.
11b: w/o TFT output.
Other unused TFT output pins are set as GPIO or Key function.*/
/*
	芯片配置寄存器[]01H
bit4-3
	For RA8876 TFT Panel I/F Output pin Setting
	00b: 24-bits TFT output。
	01b: 18-bits TFT output。
	10b: 16-bits TFT output。
	11b: w/o TFT output。
	其它未使用的 TFT 输出引脚被设定为 GPIO 与按键功能 Key。	
*/	

	uint8_t temp;
	RA8876_CmdWrite(0x01);
	temp = RA8876_DataRead();
	temp |= cSetb4;
    temp &= cClrb3;
	RA8876_DataWrite(temp);  
}

void Without_TFT(void)//RA8876 only
{
/*  TFT Panel I/F Output pin Setting
00b: 24-bits TFT output.
01b: 18-bits TFT output.
10b: 16-bits TFT output.
11b: w/o TFT output.
Other unused TFT output pins are set as GPIO or Key function.*/
	uint8_t temp;
	RA8876_CmdWrite(0x01);
	temp = RA8876_DataRead();
	temp |= cSetb4;
    temp |= cSetb3;
	RA8876_DataWrite(temp);  
}

void RA8876_I2CM_Enable(void)
{
/*  I2C master Interface Enable/Disable
		0: Disable (GPIO function)
		1: Enable (I2C master function)*/
	uint8_t temp;
	RA8876_CmdWrite(0x01);
	temp = RA8876_DataRead();
	temp |= cSetb2;
	RA8876_DataWrite(temp);    
}

void RA8876_I2CM_Disable(void)
{
/*  I2C master Interface Enable/Disable
		0: Disable (GPIO function)
		1: Enable (I2C master function)*/
	uint8_t temp;
	RA8876_CmdWrite(0x01);
	temp = RA8876_DataRead();
	temp &= cClrb2;
	RA8876_DataWrite(temp);     
}

/*
*********************************************************************************************************
*	函 数 名: Enable_SFlash_SPI
*	功能说明: 致能SPI flash串行口线
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void Enable_SFlash_SPI(void)
{
/*	Serial Flash or SPI Interface Enable/Disable
		0: Disable (GPIO function)
		1: Enable (SPI master function)
		When SDR SDRAM 32bits bus function enable, this bit is ignored
		& Serial flash pins become SDR SDRAM bus function.*/
/*
	芯片控制寄存器[01h]
bit1:
	Serial Flash or SPI Interface Enable/Disable
		0: 禁能 (GPIO function)。
		1: 致能 (SPI master function)。
	当 SDR SDRAM 32bits 总线功能被致能时，这个 bit 会被忽略，并
	且 Serial flash pins 将会变成 SDR SDRAM bus 的功能。	
*/	
	uint8_t temp;
	RA8876_CmdWrite(0x01);
	temp = RA8876_DataRead();
	temp |= cSetb1;
	RA8876_DataWrite(temp);     
}

/*
*********************************************************************************************************
*	函 数 名: Disable_SFlash_SPI
*	功能说明: 禁能SPI flash串行口线
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void Disable_SFlash_SPI(void)
{
/*	Serial Flash or SPI Interface Enable/Disable
		0: Disable (GPIO function)
		1: Enable (SPI master function)
		When SDR SDRAM 32bits bus function enable, this bit is ignored
		& Serial flash pins become SDR SDRAM bus function.*/
/*
	芯片控制寄存器[01h]
bit1:
	Serial Flash or SPI Interface Enable/Disable
		0: 禁能 (GPIO function)。
		1: 致能 (SPI master function)。
	当 SDR SDRAM 32bits 总线功能被致能时，这个 bit 会被忽略，并
	且 Serial flash pins 将会变成 SDR SDRAM bus 的功能。	
*/	
	uint8_t temp;
	RA8876_CmdWrite(0x01);
	temp = RA8876_DataRead();
	temp &= cClrb1;
	RA8876_DataWrite(temp);     
}

/*
*********************************************************************************************************
*	函 数 名: Host_Bus_8bit
*	功能说明: 8位数据总线
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void Host_Bus_8bit(void)
{
/*  Parallel Host Data Bus Width Selection
    0: 8-bit Parallel Host Data Bus.
    1: 16-bit Parallel Host Data Bus.*/
	uint8_t temp;
	RA8876_CmdWrite(0x01);
	temp = RA8876_DataRead();
	temp &= cClrb0;
	RA8876_DataWrite(temp);
}

/*
*********************************************************************************************************
*	函 数 名: Host_Bus_16bit
*	功能说明: 16位数据总线
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void Host_Bus_16bit(void)
{
/*  Parallel Host Data Bus Width Selection
    0: 8-bit Parallel Host Data Bus.
    1: 16-bit Parallel Host Data Bus.*/
	
/*
	芯片配置寄存器[01H]
bit0：
	Host Data Bus Width Selection
		0: 8-bit 主控端数据总线。
		1: 16-bit 主控端数据流排。
	*** 如果 Serial host I/F 被选择或是在开机显示的操作周期，
	RA8876 将会将这个 bit 设为 0，并且只允许 8-bit 宽度的存取。
	与 SDR SDRAM 32bit 的设定优先权比较，这个 bit 的优先权势较
	低的。	
*/
	uint8_t temp;
	RA8876_CmdWrite(0x01);
	temp = RA8876_DataRead();
	temp |= cSetb0;
	RA8876_DataWrite(temp);
}

//[02h][02h][02h][02h][02h][02h][02h][02h][02h][02h][02h][02h][02h][02h][02h][02h]

void Data_Format_8b_8bpp(void)
{
/* MPU read/write data format when access memory data port.
0xb: Direct write for 
	all 8 bits MPU I/F,
	16 bits MPU I/F with 8bpp data mode 2, 
	16 bits MPU I/F with 16bpp, 
	16 bits MPU I/F with 24bpp data mode 1, 
	and serial host interface.				 				*/
	uint8_t temp;
	RA8876_CmdWrite(0x02);
	temp = RA8876_DataRead();
	temp &= cClrb7;
	RA8876_DataWrite(temp);
}
void Data_Format_8b_16bpp(void)
{  
/* MPU read/write data format when access memory data port.
0xb: Direct write for 
	all 8 bits MPU I/F,
	16 bits MPU I/F with 8bpp data mode 2, 
	16 bits MPU I/F with 16bpp, 
	16 bits MPU I/F with 24bpp data mode 1, 
	and serial host interface.				 				*/
	uint8_t temp;
	RA8876_CmdWrite(0x02);
	temp = RA8876_DataRead();
	temp &= cClrb7;
	RA8876_DataWrite(temp);
}
void Data_Format_8b_24bpp(void)
{
/* MPU read/write data format when access memory data port.
0xb: Direct write for 
	all 8 bits MPU I/F,
	16 bits MPU I/F with 8bpp data mode 2, 
	16 bits MPU I/F with 16bpp, 
	16 bits MPU I/F with 24bpp data mode 1, 
	and serial host interface.				 				*/
	uint8_t temp;
	RA8876_CmdWrite(0x02);
	temp = RA8876_DataRead();
	temp &= cClrb7;
	RA8876_DataWrite(temp);
}

void Data_Format_16b_8bpp(void)
{
/* MPU read/write data format when access memory data port.
10b: Mask high byte of each data (ex. 16 bit MPU I/F with 8-bpp data mode 1)	*/
	uint8_t temp;
	RA8876_CmdWrite(0x02);
	temp = RA8876_DataRead();
	temp |= cSetb7;
	temp &= cClrb6;
	RA8876_DataWrite(temp);
}

/*
*********************************************************************************************************
*	函 数 名: Data_Format_16b_16bpp
*	功能说明: MPU针对内存的读写数据格式
*	形    参：无
*	返 回 值: 错误代码(无需处理)
*********************************************************************************************************
*/
void Data_Format_16b_16bpp(void)
{
/* MPU read/write data format when access memory data port.
0xb: Direct write for 
	all 8 bits MPU I/F,
	16 bits MPU I/F with 8bpp data mode 2, 
	16 bits MPU I/F with 16bpp, 
	16 bits MPU I/F with 24bpp data mode 1, 
	and serial host interface.				 				*/
/*
	存储器访问控制寄存器[02H]
bit7-6：
	Host Read/Write image Data Format
	MPU 针对内存的读写数据格式。
	0xb:直接写入，可以使用格式如下:
		1. 8 bits MPU I/F
		2. 16 bits MPU I/F with 8bpp data mode 1 & 2
		3. 16 bits MPU I/F with 16/24-bpp data mode 1
		4. serial host interface
	10b: 对每笔数据皆屏蔽 high byte(如 16 bit MPU I/F 使用的是
	8-bpp data mode 1 数据格式)。
	11b: 对偶数数据屏蔽 high byte(如 16 bit MPU I/F 使用 24-bpp
	data mode 2)。	
*/
	uint8_t temp;
	RA8876_CmdWrite(0x02);
	temp = RA8876_DataRead();
	temp &= cClrb7;
//	temp |= cSetb6;
	RA8876_DataWrite(temp);
}
/*
*********************************************************************************************************
*	函 数 名: Data_Format_16b_16bpp
*	功能说明: MPU针对内存的读写数据格式
*	形    参：无
*	返 回 值: 错误代码(无需处理)
*********************************************************************************************************
*/
void Data_Format_16b_24bpp_mode1(void)
{
/* MPU read/write data format when access memory data port.
0xb: Direct write for 
	all 8 bits MPU I/F,
	16 bits MPU I/F with 8bpp data mode 2, 
	16 bits MPU I/F with 16bpp, 
	16 bits MPU I/F with 24bpp data mode 1, 
	and serial host interface.				 				*/
	uint8_t temp;
	RA8876_CmdWrite(0x02);
	temp = RA8876_DataRead();
	temp &= cClrb7;
//	temp &= cClrb6;
	RA8876_DataWrite(temp);
}
/*
*********************************************************************************************************
*	函 数 名: Data_Format_16b_16bpp
*	功能说明: MPU针对内存的读写数据格式
*	形    参：无
*	返 回 值: 错误代码(无需处理)
*********************************************************************************************************
*/
void Data_Format_16b_24bpp_mode2(void)
{
/* MPU read/write data format when access memory data port.
11b: Mask high byte of even data (ex. 16 bit MPU I/F with 24-bpp data mode 2)	*/
	uint8_t temp;
	RA8876_CmdWrite(0x02);
	temp = RA8876_DataRead();
	temp |= cSetb7;
	temp |= cSetb6;
	RA8876_DataWrite(temp);
}

void MemRead_Left_Right_Top_Down(void)
{
/* Host Read Memory Direction (Only for Graphic Mode)
00b: Left .. Right then Top .. Bottom.
Ignored if canvas in linear addressing mode.		*/
	uint8_t temp;
	RA8876_CmdWrite(0x02);
	temp = RA8876_DataRead();
	temp &= cClrb5;
	temp &= cClrb4;
	RA8876_DataWrite(temp);
}
void MemRead_Right_Left_Top_Down(void)
{
/* Host Read Memory Direction (Only for Graphic Mode)
01b: Right .. Left then Top ..Bottom.
Ignored if canvas in linear addressing mode.		*/
	uint8_t temp;
	RA8876_CmdWrite(0x02);
	temp = RA8876_DataRead();
	temp &= cClrb5;
	temp |= cSetb4;
	RA8876_DataWrite(temp);
}
void MemRead_Top_Down_Left_Right(void)
{
/* Host Read Memory Direction (Only for Graphic Mode)
10b: Top .. Bottom then Left .. Right.
Ignored if canvas in linear addressing mode.		*/
	uint8_t temp;
	RA8876_CmdWrite(0x02);
	temp = RA8876_DataRead();
	temp |= cSetb5;
    temp &= cClrb4;
	RA8876_DataWrite(temp);
}
void MemRead_Down_Top_Left_Right(void)
{
/* Host Read Memory Direction (Only for Graphic Mode)
11b: Bottom .. Top then Left .. Right.
Ignored if canvas in linear addressing mode.		*/
	uint8_t temp;
	RA8876_CmdWrite(0x02);
	temp = RA8876_DataRead();
	temp |= cSetb5;
	temp |= cSetb4;
	RA8876_DataWrite(temp);
}

/*
*********************************************************************************************************
*	函 数 名: MemWrite_Left_Right_Top_Down
*	功能说明: 写寄存器方向（Only for Graphic Mode） 
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
/* 特别说明：中几个改变内存写入方向的函数，只有效于直接写内存。对2D画图无效。 
	例如：MPU16_16bpp_Memory_Write() 可以实现图片旋转。
		  Draw_Square_Fill()		不可以
*/
void MemWrite_Left_Right_Top_Down(void)
{
/* Host Write Memory Direction (Only for Graphic Mode)
00b: Left .. Right then Top ..Bottom.
Ignored if canvas in linear addressing mode.		*/
/*
	存储区访问寄存器[02H]
	Host Write Memory Direction (Only for Graphic Mode)
	00b: 左->右 然后 上->下. (Original)。
	01b: 右->左 然后 上->下. (Horizontal flip)。
	10b: 上->下 然后 左->右. (Rotate right 90°& Horizontal flip)。
	11b: 下->上 然后 左->右. (Rotate left 90°)。
	如果底图设定是线性寻址模式，则此两 bit 可忽略。
*/
	uint8_t temp;
	RA8876_CmdWrite(0x02);
	temp = RA8876_DataRead();
	temp &= cClrb2;
	temp &= cClrb1;
	RA8876_DataWrite(temp);
}
void MemWrite_Right_Left_Top_Down(void)
{
/* Host Write Memory Direction (Only for Graphic Mode)
01b: Right .. Left then Top .. Bottom.
Ignored if canvas in linear addressing mode.		*/
	uint8_t temp;
	RA8876_CmdWrite(0x02);
	temp = RA8876_DataRead();
	temp &= cClrb2;
	temp |= cSetb1;
	RA8876_DataWrite(temp);
}
void MemWrite_Top_Down_Left_Right(void)
{
/* Host Write Memory Direction (Only for Graphic Mode)
10b: Top .. Bottom then Left .. Right.
Ignored if canvas in linear addressing mode.		*/
	uint8_t temp;
	RA8876_CmdWrite(0x02);
	temp = RA8876_DataRead();
	temp |= cSetb2;
    temp &= cClrb1;
	RA8876_DataWrite(temp);
}
void MemWrite_Down_Top_Left_Right(void)
{
/* Host Write Memory Direction (Only for Graphic Mode)
11b: Bottom .. Top then Left .. Right.
Ignored if canvas in linear addressing mode.		*/
	uint8_t temp;
	RA8876_CmdWrite(0x02);
	temp = RA8876_DataRead();
	temp |= cSetb2;
	temp |= cSetb1;
	RA8876_DataWrite(temp);
}
//[03h][03h][03h][03h][03h][03h][03h][03h][03h][03h][03h][03h][03h][03h][03h][03h]
void Interrupt_Active_Low(void)
{
/*  Output to MPU Interrupt active level
	0 : active low.
	1 : active high.						*/
	uint8_t temp;
	RA8876_CmdWrite(0x03);
	temp = RA8876_DataRead();
	temp &= cClrb7;
	RA8876_DataWrite(temp);
}
void Interrupt_Active_High(void)
{
/*  Output to MPU Interrupt active level
	0 : active low.
	1 : active high.						*/
	uint8_t temp;
	RA8876_CmdWrite(0x03);
	temp = RA8876_DataRead();
	temp |= cSetb7;
	RA8876_DataWrite(temp);
}
void ExtInterrupt_Debounce(void)
{
/*  External interrupt input (XPS[0] pin) de-bounce
    0 : without de-bounce
    1 : enable de-bounce (1024 OSC clock)			*/
	uint8_t temp;
	RA8876_CmdWrite(0x03);
	temp = RA8876_DataRead();
	temp |= cSetb6;
	RA8876_DataWrite(temp);
}
void ExtInterrupt_Nodebounce(void)
{
/*  External interrupt input (XPS[0] pin) de-bounce
    0 : without de-bounce
    1 : enable de-bounce (1024 OSC clock)			*/
	uint8_t temp;
	RA8876_CmdWrite(0x03);
	temp = RA8876_DataRead();
	temp &= cClrb6;
	RA8876_DataWrite(temp);
}
void ExtInterrupt_Input_Low_Level_Trigger(void)
{
/* External interrupt input (XPS[0] pin) trigger type
00 : low level trigger
*/
	uint8_t temp;
	RA8876_CmdWrite(0x03);
	temp = RA8876_DataRead();
	temp &= cClrb5;
    temp &= cClrb4;
	RA8876_DataWrite(temp);
}
void ExtInterrupt_Input_High_Level_Trigger(void)
{
/* External interrupt input (XPS[0] pin) trigger type
01 : falling edge trigger
*/
	uint8_t temp;
	RA8876_CmdWrite(0x03);

	temp = RA8876_DataRead();
	temp |= cSetb5;
    temp &= cClrb4;
	RA8876_DataWrite(temp);
}
void ExtInterrupt_Input_Falling_Edge_Trigger(void)
{
/* External interrupt input (XPS[0] pin) trigger type
10 : high level trigger
*/
	uint8_t temp;
	RA8876_CmdWrite(0x03);
	temp = RA8876_DataRead();
    temp &= cClrb5;
    temp |= cSetb4;
	RA8876_DataWrite(temp);
}
void ExtInterrupt_Input_Rising_Edge_Trigger(void)
{
/* External interrupt input (XPS[0] pin) trigger type
11 : rising edge trigger
*/
	uint8_t temp;
	RA8876_CmdWrite(0x03);
	temp = RA8876_DataRead();
	temp |= cSetb5;
    temp |= cSetb4;
	RA8876_DataWrite(temp);
}
void LVDS_Format1(void)//RA8877 only
{
/* FPD-Link Data Format / LVDS Data Format
0 : Format 1 (VESA format) 
1 : Format 2 (JEIDA format) 				*/
	uint8_t temp;
	RA8876_CmdWrite(0x03);
	temp = RA8876_DataRead();
    temp &= cClrb3;
	RA8876_DataWrite(temp);
}
void LVDS_Format2(void)//RA8877 only
{
/* FPD-Link Data Format / LVDS Data Format
0 : Format 1 (VESA format) 
1 : Format 2 (JEIDA format) 				*/
	uint8_t temp;
	RA8876_CmdWrite(0x03);
	temp = RA8876_DataRead();
    temp |= cSetb3;
	RA8876_DataWrite(temp);
}

/*
*********************************************************************************************************
*	函 数 名: Graphic_Mode
*	功能说明: 设置图形模式
*	形    参：无
*	返 回 值: 错误代码(无需处理)
*********************************************************************************************************
*/
void Graphic_Mode(void)
{
/*
	输入控制寄存器[03H]
bit2：
	
	Text Mode Enable
		0 : 图形模式。
		1 : 文字模式。
	在设定这个 bit 之前，必须先确定 core task busy 是否正在忙碌或
	闲置中，而 core task busy 是状态缓存器。
	如果在 linear 寻址模式中，这个 bit 始终为 0。
		
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x03);
	temp = RA8876_DataRead();
    temp &= cClrb2;
	RA8876_DataWrite(temp);
}


/*
*********************************************************************************************************
*	函 数 名: Text_Mode
*	功能说明: 设置文本模式
*	形    参：无
*	返 回 值: 错误代码(无需处理)
*********************************************************************************************************
*/
void Text_Mode(void)
{
/*
	输入控制寄存器[03H]
bit2：
	
	Text Mode Enable
		0 : 图形模式。
		1 : 文字模式。
	在设定这个 bit 之前，必须先确定 core task busy 是否正在忙碌或
	闲置中，而 core task busy 是状态缓存器。
	如果在 linear 寻址模式中，这个 bit 始终为 0。
		
*/
	uint8_t temp;
	RA8876_CmdWrite(0x03);
	temp = RA8876_DataRead();
    temp |= cSetb2;
	RA8876_DataWrite(temp);
}

/*
*********************************************************************************************************
*	函 数 名: Memory_Select_SDRAM
*	功能说明: 内存端口读写目的选择
*	形    参：无
*	返 回 值: 错误代码(无需处理)
*********************************************************************************************************
*/
void Memory_Select_SDRAM(void)
{
/*
	输入控制寄存器[03H]
bit1-0：
	Memory port Read/Write Destination Selection
		00b: 选择 SDRAM 为 image/pattern/使用者自订字型的数据写入
		目的，支持 Read-modify-Write。
		01b: 选择 RGB 色的 Gamma table 为写入目的。 每个颜色的都
		是 256 bytes。使用者需要指定需要写入的 gamma table 然后再连
		续写入 256 bytes。
	
*/
	uint8_t temp;

	RA8876_CmdWrite(0x03);
	temp = RA8876_DataRead();
    temp &= cClrb1;
    temp &= cClrb0;
	RA8876_DataWrite(temp);

}
void Memory_Select_Gamma_Table(void)
{
/*
	输入控制寄存器[03H]
bit1-0：
	Memory port Read/Write Destination Selection
		00b: 选择 SDRAM 为 image/pattern/使用者自订字型的数据写入
		目的，支持 Read-modify-Write。
		01b: 选择 RGB 色的 Gamma table 为写入目的。 每个颜色的都
		是 256 bytes。使用者需要指定需要写入的 gamma table 然后再连
		续写入 256 bytes。
	
*/
	uint8_t temp;
	RA8876_CmdWrite(0x03);
	temp = RA8876_DataRead();
    temp &= cClrb1;
    temp |= cSetb0;
	RA8876_DataWrite(temp);  

}
void Memory_Select_Graphic_Cursor_RAM(void)
{
	uint8_t temp;
	RA8876_CmdWrite(0x03);
	temp = RA8876_DataRead();
    temp |= cSetb1;
    temp &= cClrb0;
	RA8876_DataWrite(temp);
}
void Memory_Select_Color_Palette_RAM(void)
{
	uint8_t temp;
	RA8876_CmdWrite(0x03);
	temp = RA8876_DataRead();
    temp |= cSetb1;
    temp |= cSetb0;
	RA8876_DataWrite(temp);
}

//[05h]=========================================================================
//[06h]=========================================================================
//[07h]=========================================================================
//[08h]=========================================================================
//[09h]=========================================================================
//[0Ah]=========================================================================
//[0Bh]=========================================================================

void Enable_Resume_Interrupt(void)
{
/*
Wakeup/resume Interrupt Enable
0: Disable.
1: Enable.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0B);
	temp = RA8876_DataRead();
    temp |= cSetb7;
	RA8876_DataWrite(temp);
}
void Disable_Resume_Interrupt(void)
{
/*
Wakeup/resume Interrupt Enable
0: Disable.
1: Enable.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0B);
	temp = RA8876_DataRead();
    temp &= cClrb7;
	RA8876_DataWrite(temp);
}
void Enable_ExtInterrupt_Input(void)
{
/*
External Interrupt (PS[0] pin) Enable
0: Disable.
1: Enable.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0B);
	temp = RA8876_DataRead();
    temp |= cSetb6;
	RA8876_DataWrite(temp);
}
void Disable_ExtInterrupt_Input(void)
{
/*
External Interrupt (PS[0] pin) Enable
0: Disable.
1: Enable.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0B);
	temp = RA8876_DataRead();
    temp &= cClrb6;
	RA8876_DataWrite(temp);
}
void Enable_I2CM_Interrupt(void)
{
/*
I2C Master Interrupt Enable
0: Disable.
1: Enable.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0B);
	temp = RA8876_DataRead();
    temp |= cSetb5;
	RA8876_DataWrite(temp);
}
void Disable_I2CM_Interrupt(void)
{
/*
I2C Master Interrupt Enable
0: Disable.
1: Enable.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0B);
	temp = RA8876_DataRead();
    temp &= cClrb5;
	RA8876_DataWrite(temp);
}
void Enable_Vsync_Interrupt(void)
{
/*
Vsync time base interrupt Enable Bit
0: Disable Interrupt.
1: Enable Interrupt.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0B);
	temp = RA8876_DataRead();
    temp |= cSetb4;
	RA8876_DataWrite(temp);
}
void Disable_Vsync_Interrupt(void)
{
/*
Vsync time base interrupt Enable Bit
0: Disable Interrupt.
1: Enable Interrupt.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0B);
	temp = RA8876_DataRead();
    temp &= cClrb4;
	RA8876_DataWrite(temp);
}
void Enable_KeyScan_Interrupt(void)
{
/*
Key Scan Interrupt Enable Bit
0: Disable Key scan interrupt.
1: Enable Key scan interrupt.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0B);
	temp = RA8876_DataRead();
    temp |= cSetb3;
	RA8876_DataWrite(temp);
}
void Disable_KeyScan_Interrupt(void)
{
/*
Key Scan Interrupt Enable Bit
0: Disable Key scan interrupt.
1: Enable Key scan interrupt.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0B);
	temp = RA8876_DataRead();
    temp &= cClrb3;
	RA8876_DataWrite(temp);
}
void Enable_DMA_Draw_BTE_Interrupt(void)
{
/*
Serial flash DMA Complete | Draw task finished | BTE Process
Complete etc. Interrupt Enable
0: Disable Interrupt.
1: Enable Interrupt.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0B);
	temp = RA8876_DataRead();
    temp |= cSetb2;
	RA8876_DataWrite(temp);
}
void Disable_DMA_Draw_BTE_Interrupt(void)
{
/*
Serial flash DMA Complete | Draw task finished | BTE Process
Complete etc. Interrupt Enable
0: Disable Interrupt.
1: Enable Interrupt.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0B);
	temp = RA8876_DataRead();
    temp &= cClrb2;
	RA8876_DataWrite(temp);
}
void Enable_PWM1_Interrupt(void)
{
/*
PWM timer 1 Interrupt Enable Bit
0: Disable Interrupt.
1: Enable Interrupt.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0B);
	temp = RA8876_DataRead();
    temp |= cSetb1;
	RA8876_DataWrite(temp);
}
void Disable_PWM1_Interrupt(void)
{
/*
PWM timer 1 Interrupt Enable Bit
0: Disable Interrupt.
1: Enable Interrupt.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0B);
	temp = RA8876_DataRead();
    temp &= cClrb1;
	RA8876_DataWrite(temp);
}
void Enable_PWM0_Interrupt(void)
{
/*
PWM timer 0 Interrupt Enable Bit
0: Disable Interrupt.
1: Enable Interrupt.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0B);
	temp = RA8876_DataRead();
    temp |= cSetb0;
	RA8876_DataWrite(temp);
}
void Disable_PWM0_Interrupt(void)
{
/*
PWM timer 0 Interrupt Enable Bit
0: Disable Interrupt.
1: Enable Interrupt.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0B);
	temp = RA8876_DataRead();
    temp &= cClrb0;
	RA8876_DataWrite(temp);
}

//[0Ch]=========================================================================
uint8_t Read_Interrupt_status(void)
{
/*
[Bit7]Read Function ..Resume Interrupt Status
0: No Resume interrupt happens.
1: Resume interrupt happens.
[Bit6]Read Function .. PS[0] pin Interrupt Status
0: No PS[0] pin interrupt happens.
1: PS[0] pin interrupt happens.
[Bit5]Read Function .. I2C master Interrupt Status
0: No I2C master interrupt happens.
1: I2C master interrupt happens.
[Bit4]Read Function .. Vsync Interrupt Status
0: No interrupt happens.
1: interrupt happens.
[Bit3]Read Function ..Key Scan Interrupt Status
0: No Key Scan interrupt happens.
1: Key Scan interrupt happens.
[Bit2]Read Function..Interrupt Status
0: No interrupt happens.
1: interrupt happens.
[Bit1]Read Function..Interrupt Status
0: No interrupt happens.
1: interrupt happens.
[Bit0]Read Function..Interrupt Status
0: No interrupt happens.
1: interrupt happens.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0C);
	temp = RA8876_DataRead();
    return temp;
}

void Check_Vsync_finished(void)
{
/*[Bit4]
Vsync Time base interrupt flag
Read Function .. Vsync Interrupt Status
0: No interrupt happens.
1: interrupt happens.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0C);
	RA8876_DataWrite(0x10);
	do{	   
		temp=RA8876_DataRead();
	
	}while((temp&0x10) == 0x00); 

}

void Clear_Resume_Interrupt_Flag(void)
{
/*
Resume Interrupt flag
Write Function .. Resume Interrupt Clear Bit
0: No operation.
1: Clear Resume interrupt.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0C);
	temp = RA8876_DataRead();
    temp |= cSetb7;
	RA8876_DataWrite(temp);
}
void Clear_ExtInterrupt_Input_Flag(void)
{
/*
External Interrupt (PS[0] pin) flag
Write Function .. PS[0] pin edge Interrupt Clear Bit
0: No operation.
1: Clear the PS[0] pin edge interrupt.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0C);
	temp = RA8876_DataRead();
    temp |= cSetb6;
	RA8876_DataWrite(temp);
}
void Clear_I2CM_Interrupt_Flag(void)
{
/*
I2C master Interrupt flag
Write Function.. I2C master Interrupt Clear Bit
0: No operation.
1: Clear the I2C master interrupt.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0C);
	temp = RA8876_DataRead();
    temp |= cSetb5;
	RA8876_DataWrite(temp);
}
void Clear_Vsync_Interrupt_Flag(void)
{
/*
Vsync Time base interrupt flag
Write Function ..Vsync Interrupt Clear Bit
0: No operation.
1: Clear the interrupt.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0C);
	temp = RA8876_DataRead();
    temp |= cSetb4;
	RA8876_DataWrite(temp);
}
void Clear_KeyScan_Interrupt_Flag(void)
{
/*
Key Scan Interrupt flag
Write Function..Key Scan Interrupt Clear Bit
0: No operation.
1: Clear the Key Scan interrupt.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0C);
	temp = RA8876_DataRead();
    temp |= cSetb3;
	RA8876_DataWrite(temp);
}
void Clear_DMA_Draw_BTE_Interrupt_Flag(void)
{
/*
Serial flash DMA Complete | Draw task finished | BTE
Process Complete etc. Interrupt flag
Write Function.. Interrupt Clear Bit
0: No operation.
1: Clear interrupt.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0C);
	temp = RA8876_DataRead();
    temp |= cSetb2;
	RA8876_DataWrite(temp);
}
void Clear_PWM1_Interrupt_Flag(void)
{
/*
PWM 1 timer Interrupt flag
Write Function..Interrupt Clear Bit
0: No operation.
1: Clear interrupt.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0C);
	temp = RA8876_DataRead();
    temp |= cSetb1;
	RA8876_DataWrite(temp);
}
void Clear_PWM0_Interrupt_Flag(void)
{
/*
PWM 0 timer Interrupt flag
Write Function..Interrupt Clear Bit
0: No operation.
1: Clear interrupt.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0C);
	temp = RA8876_DataRead();
    temp |= cSetb0;
	RA8876_DataWrite(temp);
}
//[0Dh]=========================================================================
void XnINTR_Mask_Resume_Interrupt_Flag(void)
{
/*
Mask Resume Interrupt Flag
0: Enable.
1: Mask.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0D);
	temp = RA8876_DataRead();
    temp |= cSetb7;
	RA8876_DataWrite(temp);
}
void XnINTR_Mask_ExtInterrupt_Input_Flag(void)
{
/*
Mask External Interrupt (PS[0] pin) Flag
0: Enable.
1: Mask.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0D);
	temp = RA8876_DataRead();
    temp |= cSetb6;
	RA8876_DataWrite(temp);
}
void XnINTR_Mask_I2CM_Interrupt_Flag(void)
{
/*
Mask I2C Master Interrupt Flag
0: Enable.
1: Mask.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0D);
	temp = RA8876_DataRead();
    temp |= cSetb5;
	RA8876_DataWrite(temp);
}
void XnINTR_Mask_Vsync_Interrupt_Flag(void)
{
/*
Mask Vsync time base interrupt Flag
0: Enable.
1: Mask.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0D);
	temp = RA8876_DataRead();
    temp |= cSetb4;
	RA8876_DataWrite(temp);
}
void XnINTR_Mask_KeyScan_Interrupt_Flag(void)
{
/*
Mask Key Scan Interrupt Flag
0: Enable.
1: Mask.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0D);
	temp = RA8876_DataRead();
    temp |= cSetb3;
	RA8876_DataWrite(temp);
}
void XnINTR_Mask_DMA_Draw_BTE_Interrupt_Flag(void)
{
/*
Mask Serial flash DMA Complete | Draw task finished | BTE
Process Complete etc. Interrupt Flag
0: Enable.
1: Mask.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0D);
	temp = RA8876_DataRead();
    temp |= cSetb2;
	RA8876_DataWrite(temp);
}
void XnINTR_Mask_PWM1_Interrupt_Flag(void)
{
/*
Mask PWM timer 1 Interrupt Flag
0: Enable.
1: Mask.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0D);
	temp = RA8876_DataRead();
    temp |= cSetb1;
	RA8876_DataWrite(temp);
}
void XnINTR_Mask_PWM0_Interrupt_Flag(void)
{
/*
Mask PWM timer 0 Interrupt Flag
0: Enable.
1: Mask.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0D);
	temp = RA8876_DataRead();
    temp |= cSetb0;
	RA8876_DataWrite(temp);
}

void XnINTR_Unmask_Resume_Interrupt_Flag(void)
{
/*
Mask Resume Interrupt Flag
0: Enable.
1: Mask.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0D);
	temp = RA8876_DataRead();
    temp &= cClrb7;
	RA8876_DataWrite(temp);
}
void XnINTR_Unmask_ExtInterrupt_Input_Flag(void)
{
/*
Mask External Interrupt (PS[0] pin) Flag
0: Enable.
1: Mask.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0D);
	temp = RA8876_DataRead();
    temp &= cClrb6;
	RA8876_DataWrite(temp);
}
void XnINTR_Unmask_I2CM_Interrupt_Flag(void)
{
/*
Mask I2C Master Interrupt Flag
0: Enable.
1: Mask.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0D);
	temp = RA8876_DataRead();
    temp &= cClrb5;
	RA8876_DataWrite(temp);
}
void XnINTR_Unmask_Vsync_Interrupt_Flag(void)
{
/*
Mask Vsync time base interrupt Flag
0: Enable.
1: Mask.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0D);
	temp = RA8876_DataRead();
    temp &= cClrb4;
	RA8876_DataWrite(temp);
}
void XnINTR_Unmask_KeyScan_Interrupt_Flag(void)
{
/*
Mask Key Scan Interrupt Flag
0: Enable.
1: Mask.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0D);
	temp = RA8876_DataRead();
    temp &= cClrb3;
	RA8876_DataWrite(temp);
}
void XnINTR_Unmask_DMA_Draw_BTE_Interrupt_Flag(void)
{
/*
Mask Serial flash DMA Complete | Draw task finished | BTE
Process Complete etc. Interrupt Flag
0: Enable.
1: Mask.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0D);
	temp = RA8876_DataRead();
    temp &= cClrb2;
	RA8876_DataWrite(temp);
}
void XnINTR_Unmask_PWM1_Interrupt_Flag(void)
{
/*
Mask PWM timer 1 Interrupt Flag
0: Enable.
1: Mask.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0D);
	temp = RA8876_DataRead();
    temp &= cClrb1;
	RA8876_DataWrite(temp);
}
void XnINTR_Unmask_PWM0_Interrupt_Flag(void)
{
/*
Mask PWM timer 0 Interrupt Flag
0: Enable.
1: Mask.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0D);
	temp = RA8876_DataRead();
    temp &= cClrb0;
	RA8876_DataWrite(temp);
}

//[0Eh]=========================================================================
void Enable_GPIOF_PullUp(void)
{
/*
GPIO_F[7:0] Pull-Up Enable (XPDAT[23:19, 15:13])
0: Pull-Up Disable
1: Pull-Up Enable
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0E);
	temp = RA8876_DataRead();
    temp |= cSetb5;
	RA8876_DataWrite(temp);
}
void Enable_GPIOE_PullUp(void)
{
/*
GPIO_E[7:0] Pull-Up Enable (XPDAT[12:10, 7:3])
0: Pull-Up Disable
1: Pull-Up Enable
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0E);
	temp = RA8876_DataRead();
    temp |= cSetb4;
	RA8876_DataWrite(temp);
}
void Enable_GPIOD_PullUp(void)
{
/*
GPIO_D[7:0] Pull-Up Enable (XPDAT[18, 2, 17, 16, 9, 8, 1,0])
0: Pull-Up Disable
1: Pull-Up Enable
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0E);
	temp = RA8876_DataRead();
    temp |= cSetb3;
	RA8876_DataWrite(temp);
}
void Enable_GPIOC_PullUp(void)
{
/*
GPIO_C[6:0] Pull-Up Enable (XSDA, XSCL, XnSFCS1,
XnSFCS0, XMISO, XMOSI , XSCK)
0: Pull-Up Disable
1: Pull-Up Enable
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0E);
	temp = RA8876_DataRead();
    temp |= cSetb2;
	RA8876_DataWrite(temp);
}
void Enable_XDB15_8_PullUp(void)
{
/*
XDB[15:8] Pull-Up Enable
0: Pull-Up Disable
1: Pull-Up Enable
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0E);
	temp = RA8876_DataRead();
    temp |= cSetb1;
	RA8876_DataWrite(temp);
}
void Enable_XDB7_0_PullUp(void)
{
/*
XDB[7:0] Pull-Up Enable
0: Pull-Up Disable
1: Pull-Up Enable
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0E);
	temp = RA8876_DataRead();
    temp |= cSetb0;
	RA8876_DataWrite(temp);
}
void Disable_GPIOF_PullUp(void)
{
/*
GPIO_F[7:0] Pull-Up Enable (XPDAT[23:19, 15:13])
0: Pull-Up Disable
1: Pull-Up Enable
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0E);
	temp = RA8876_DataRead();
    temp &= cClrb5;
	RA8876_DataWrite(temp);
}
void Disable_GPIOE_PullUp(void)
{
/*
GPIO_E[7:0] Pull-Up Enable (XPDAT[12:10, 7:3])
0: Pull-Up Disable
1: Pull-Up Enable
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0E);
	temp = RA8876_DataRead();
    temp &= cClrb4;
	RA8876_DataWrite(temp);
}
void Disable_GPIOD_PullUp(void)
{
/*
GPIO_D[7:0] Pull-Up Enable (XPDAT[18, 2, 17, 16, 9, 8, 1,0])
0: Pull-Up Disable
1: Pull-Up Enable
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0E);
	temp = RA8876_DataRead();
    temp &= cClrb3;
	RA8876_DataWrite(temp);
}
void Disable_GPIOC_PullUp(void)
{
/*
GPIO_C[6:0] Pull-Up Enable (XSDA, XSCL, XnSFCS1,
XnSFCS0, XMISO, XMOSI , XSCK)
0: Pull-Up Disable
1: Pull-Up Enable
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0E);
	temp = RA8876_DataRead();
    temp &= cClrb2;
	RA8876_DataWrite(temp);
}
void Disable_XDB15_8_PullUp(void)
{
/*
XDB[15:8] Pull-Up Enable
0: Pull-Up Disable
1: Pull-Up Enable
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0E);
	temp = RA8876_DataRead();
    temp &= cClrb1;
	RA8876_DataWrite(temp);
}
void Disable_XDB7_0_PullUp(void)
{
/*
XDB[7:0] Pull-Up Enable
0: Pull-Up Disable
1: Pull-Up Enable
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0E);
	temp = RA8876_DataRead();
    temp &= cClrb0;
	RA8876_DataWrite(temp);
}
//[0Fh]=========================================================================
void XPDAT18_Set_GPIO_D7(void)
{
/*
XPDAT[18]  not scan function select
0: GPIO-D7
1: KOUT[4]
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0F);
	temp = RA8876_DataRead();
    temp &= cClrb7;
	RA8876_DataWrite(temp);
}
void XPDAT18_Set_KOUT4(void)
{
/*
XPDAT[18]  not scan function select
0: GPIO-D7
1: KOUT[4]
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0F);
	temp = RA8876_DataRead();
    temp |= cSetb7;
	RA8876_DataWrite(temp);
}
void XPDAT17_Set_GPIO_D5(void)
{
/*
XPDAT[17]  not scan function select
0: GPIO-D5
1: KOUT[2]
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0F);
	temp = RA8876_DataRead();
    temp &= cClrb6;
	RA8876_DataWrite(temp);
}
void XPDAT17_Set_KOUT2(void)
{
/*
XPDAT[17]  not scan function select
0: GPIO-D5
1: KOUT[2]
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0F);
	temp = RA8876_DataRead();
    temp |= cSetb6;
	RA8876_DataWrite(temp);
}
void XPDAT16_Set_GPIO_D4(void)
{
/*
XPDAT[16]  not scan function select
0: GPIO-D4
1: KOUT[1]
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0F);
	temp = RA8876_DataRead();
    temp &= cClrb5;
	RA8876_DataWrite(temp);
}
void XPDAT16_Set_KOUT1(void)
{
/*
XPDAT[16]  not scan function select
0: GPIO-D4
1: KOUT[1]
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0F);
	temp = RA8876_DataRead();
    temp |= cSetb5;
	RA8876_DataWrite(temp);
}
void XPDAT9_Set_GPIO_D3(void)
{
/*
XPDAT[9]  not scan function select
0: GPIO-D3
1: KOUT[3]
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0F);
	temp = RA8876_DataRead();
    temp &= cClrb4;
	RA8876_DataWrite(temp);
}
void XPDAT9_Set_KOUT3(void)
{
/*
XPDAT[9]  not scan function select
0: GPIO-D3
1: KOUT[3]
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0F);
	temp = RA8876_DataRead();
    temp |= cSetb4;
	RA8876_DataWrite(temp);
}
void XPDAT8_Set_GPIO_D2(void)
{
/*
XPDAT[8]  not scan function select
0: GPIO-D2
1: KIN[3]
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0F);
	temp = RA8876_DataRead();
    temp &= cClrb3;
	RA8876_DataWrite(temp);
}
void XPDAT8_Set_KIN3(void)
{
/*
XPDAT[8]  not scan function select
0: GPIO-D2
1: KIN[3]
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0F);
	temp = RA8876_DataRead();
    temp |= cSetb3;
	RA8876_DataWrite(temp);
}
void XPDAT2_Set_GPIO_D6(void)
{
/*
XPDAT[2]  not scan function select
0: GPIO-D6
1: KIN[4]
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0F);
	temp = RA8876_DataRead();
    temp &= cClrb2;
	RA8876_DataWrite(temp);
}
void XPDAT2_Set_KIN4(void)
{
/*
XPDAT[2]  not scan function select
0: GPIO-D6
1: KIN[4]
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0F);
	temp = RA8876_DataRead();
    temp |= cSetb2;
	RA8876_DataWrite(temp);
}
void XPDAT1_Set_GPIO_D1(void)
{
/*
XPDAT[1]  not scan function select
0: GPIO-D1
1: KIN[2]
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0F);
	temp = RA8876_DataRead();
    temp &= cClrb1;
	RA8876_DataWrite(temp);
}
void XPDAT1_Set_KIN2(void)
{
/*
XPDAT[1]  not scan function select
0: GPIO-D1
1: KIN[2]
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0F);
	temp = RA8876_DataRead();
    temp |= cSetb1;
	RA8876_DataWrite(temp);
}
void XPDAT0_Set_GPIO_D0(void)
{
/*
XPDAT[0]  not scan function select
0: GPIO-D0
1: KIN[1]
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0F);
	temp = RA8876_DataRead();
    temp &= cClrb0;
	RA8876_DataWrite(temp);
}
void XPDAT0_Set_KIN1(void)
{
/*
XPDAT[0]  not scan function select
0: GPIO-D0
1: KIN[1]
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0F);
	temp = RA8876_DataRead();
    temp |= cSetb0;
	RA8876_DataWrite(temp);
}

//[10h]=========================================================================
void Enable_PIP1(void)
{
/*
PIP 1 window Enable/Disable
0 : PIP 1 window disable.
1 : PIP 1 window enable
PIP 1 window always on top of PIP 2 window.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x10);
	temp = RA8876_DataRead();
    temp |= cSetb7;
	RA8876_DataWrite(temp);
}
void Disable_PIP1(void)
{
/*
PIP 1 window Enable/Disable
0 : PIP 1 window disable.
1 : PIP 1 window enable
PIP 1 window always on top of PIP 2 window.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x10);
	temp = RA8876_DataRead();
    temp &= cClrb7;
	RA8876_DataWrite(temp);
}
void Enable_PIP2(void)
{
/*
PIP 2 window Enable/Disable
0 : PIP 2 window disable.
1 : PIP 2 window enable
PIP 1 window always on top of PIP 2 window.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x10);
	temp = RA8876_DataRead();
    temp |= cSetb6;
	RA8876_DataWrite(temp);
}
void Disable_PIP2(void)
{
/*
PIP 2 window Enable/Disable
0 : PIP 2 window disable.
1 : PIP 2 window enable
PIP 1 window always on top of PIP 2 window.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x10);
	temp = RA8876_DataRead();
    temp &= cClrb6;
	RA8876_DataWrite(temp);
}
void Select_PIP1_Parameter(void)
{
/*
0: To configure PIP 1ˇs parameters.
1: To configure PIP 2ˇs parameters..
*/
	uint8_t temp;
	RA8876_CmdWrite(0x10);
	temp = RA8876_DataRead();
    temp &= cClrb4;
	RA8876_DataWrite(temp);
}
void Select_PIP2_Parameter(void)
{
/*
0: To configure PIP 1ˇs parameters.
1: To configure PIP 2ˇs parameters..
*/
	uint8_t temp;
	RA8876_CmdWrite(0x10);
	temp = RA8876_DataRead();
    temp |= cSetb4;
	RA8876_DataWrite(temp);
}
void Select_Main_Window_8bpp(void)
{
/*
Main Window Color Depth Setting
00b: 8-bpp generic TFT, i.e. 256 colors.
01b: 16-bpp generic TFT, i.e. 65K colors.
1xb: 24-bpp generic TFT, i.e. 1.67M colors.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x10);
	temp = RA8876_DataRead();
    temp &= cClrb3;
    temp &= cClrb2;
	RA8876_DataWrite(temp);
}

/*
*********************************************************************************************************
*	函 数 名: Select_Main_Window_16bpp
*	功能说明: 主窗口颜色设定
*	形    参：无
*	返 回 值: 错误代码(无需处理)
*********************************************************************************************************
*/
void Select_Main_Window_16bpp(void)
{
/*
Main Window Color Depth Setting
00b: 8-bpp generic TFT, i.e. 256 colors.
01b: 16-bpp generic TFT, i.e. 65K colors.
1xb: 24-bpp generic TFT, i.e. 1.67M colors.
*/
/*
	主/PIP窗口控制寄存器[10H]
bit3-2：
	Main Image Color Depth Setting
	00b: 8-bpp generic TFT, i.e. 256 色。
	01b: 16-bpp generic TFT, i.e. 65K 色。
	1xb: 24-bpp generic TFT, i.e. 1.67 色。
*/
	uint8_t temp;
	RA8876_CmdWrite(0x10);
	temp = RA8876_DataRead();
    temp &= cClrb3;
    temp |= cSetb2;
	RA8876_DataWrite(temp);
}

/*
*********************************************************************************************************
*	函 数 名: Select_Main_Window_16bpp
*	功能说明: 主窗口颜色设定
*	形    参：无
*	返 回 值: 错误代码(无需处理)
*********************************************************************************************************
*/
void Select_Main_Window_24bpp(void)
{
/*
Main Window Color Depth Setting
00b: 8-bpp generic TFT, i.e. 256 colors.
01b: 16-bpp generic TFT, i.e. 65K colors.
1xb: 24-bpp generic TFT, i.e. 1.67M colors.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x10);
	temp = RA8876_DataRead();
    temp |= cSetb3;
	RA8876_DataWrite(temp);
}

void Select_LCD_Sync_Mode(void)
{
/*
To Control panel's synchronous signals
0: Sync Mode: Enable XVSYNC, XHSYNC, XDE
1: DE Mode: Only XDE enable, XVSYNC & XHSYNC in idle state
*/
	uint8_t temp;
	RA8876_CmdWrite(0x10);
	temp = RA8876_DataRead();
    temp &= cClrb0;
	RA8876_DataWrite(temp);
}
void Select_LCD_DE_Mode(void)
{
/*
To Control panel's synchronous signals
0: Sync Mode: Enable XVSYNC, XHSYNC, XDE
1: DE Mode: Only XDE enable, XVSYNC & XHSYNC in idle state
*/
	uint8_t temp;
	RA8876_CmdWrite(0x10);
	temp = RA8876_DataRead();
    temp |= cSetb0;
	RA8876_DataWrite(temp);
}

//[11h]=========================================================================
void Select_PIP1_Window_8bpp(void)
{
/*
PIP 1 Window Color Depth Setting
00b: 8-bpp generic TFT, i.e. 256 colors.
01b: 16-bpp generic TFT, i.e. 65K colors.
1xb: 24-bpp generic TFT, i.e. 1.67M colors.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x11);
	temp = RA8876_DataRead();
    temp &= cClrb3;
    temp &= cClrb2;
	RA8876_DataWrite(temp);
}
void Select_PIP1_Window_16bpp(void)
{
/*
PIP 1 Window Color Depth Setting
00b: 8-bpp generic TFT, i.e. 256 colors.
01b: 16-bpp generic TFT, i.e. 65K colors.
1xb: 24-bpp generic TFT, i.e. 1.67M colors.
*/
/*
	PIP窗口颜色深度设定寄存器[11H]
bit3-2：
	PIP 1 Window Color Depth Setting
	00b: 8-bpp generic TFT, i.e. 256 色。
	01b: 16-bpp generic TFT, i.e. 65K 色。
	1xb: 24-bpp generic TFT, i.e. 1.67M 色
*/
	uint8_t temp;
	RA8876_CmdWrite(0x11);
	temp = RA8876_DataRead();
    temp &= cClrb3;
    temp |= cSetb2;
	RA8876_DataWrite(temp);
}
void Select_PIP1_Window_24bpp(void)
{
/*
PIP 1 Window Color Depth Setting
00b: 8-bpp generic TFT, i.e. 256 colors.
01b: 16-bpp generic TFT, i.e. 65K colors.
1xb: 24-bpp generic TFT, i.e. 1.67M colors.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x11);
	temp = RA8876_DataRead();
    temp |= cSetb3;
//    temp &= cClrb2;
	RA8876_DataWrite(temp);
}
void Select_PIP2_Window_8bpp(void)
{
/*
PIP 2 Window Color Depth Setting
00b: 8-bpp generic TFT, i.e. 256 colors.
01b: 16-bpp generic TFT, i.e. 65K colors.
1xb: 24-bpp generic TFT, i.e. 1.67M colors.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x11);
	temp = RA8876_DataRead();
    temp &= cClrb1;
    temp &= cClrb0;
	RA8876_DataWrite(temp);
}
void Select_PIP2_Window_16bpp(void)
{
/*
PIP 2 Window Color Depth Setting
00b: 8-bpp generic TFT, i.e. 256 colors.
01b: 16-bpp generic TFT, i.e. 65K colors.
1xb: 24-bpp generic TFT, i.e. 1.67M colors.
*/
/*
	PIP窗口颜色深度设定寄存器[11H]
bit1-0：
	PIP 2 Window Color Depth Setting
	00b: 8-bpp generic TFT, i.e. 256 色。
	01b: 16-bpp generic TFT, i.e. 65K 色。
	1xb: 24-bpp generic TFT, i.e. 1.67M 色。
*/
	uint8_t temp;
	RA8876_CmdWrite(0x11);
	temp = RA8876_DataRead();
    temp &= cClrb1;
    temp |= cSetb0;
	RA8876_DataWrite(temp);
}
void Select_PIP2_Window_24bpp(void)
{
/*
PIP 2 Window Color Depth Setting
00b: 8-bpp generic TFT, i.e. 256 colors.
01b: 16-bpp generic TFT, i.e. 65K colors.
1xb: 24-bpp generic TFT, i.e. 1.67M colors.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x11);
	temp = RA8876_DataRead();
    temp |= cSetb1;
//    temp &= cClrb0;
	RA8876_DataWrite(temp);
}

//[12h]=========================================================================
void PCLK_Rising(void)	 
{
/*
PCLK Inversion
0: PDAT, DE, HSYNC etc. Drive(/ change) at PCLK falling edge.
1: PDAT, DE, HSYNC etc. Drive(/ change) at PCLK rising edge.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x12);
	temp = RA8876_DataRead();
    temp &= cClrb7;
	RA8876_DataWrite(temp);
}
void PCLK_Falling(void)
{
/*
PCLK Inversion
0: PDAT, DE, HSYNC etc. Drive(/ change) at PCLK falling edge.
1: PDAT, DE, HSYNC etc. Drive(/ change) at PCLK rising edge.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x12);
	temp = RA8876_DataRead();
    temp |= cSetb7;
	RA8876_DataWrite(temp);
}

/*
*********************************************************************************************************
*	函 数 名: Display_ON
*	功能说明: 开显示
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void Display_ON(void)
{
/*	
Display ON/OFF
0b: Display Off.
1b: Display On.
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x12);
	temp = RA8876_DataRead();
	temp |= cSetb6;
	RA8876_DataWrite(temp);
}

/*
*********************************************************************************************************
*	函 数 名: Display_OFF
*	功能说明: 关显示
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void Display_OFF(void)
{
/*	
Display ON/OFF
0b: Display Off.
1b: Display On.
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x12);
	temp = RA8876_DataRead();
	temp &= cClrb6;
	RA8876_DataWrite(temp);
}

/*
*********************************************************************************************************
*	函 数 名: Color_Bar_ON
*	功能说明: 显示测试色盘
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void Color_Bar_ON(void)
{
/*	
Display Test Color Bar
0b: Disable.
1b: Enable.
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x12);
	temp = RA8876_DataRead();
	temp |= cSetb5;
	RA8876_DataWrite(temp);
}


/*
*********************************************************************************************************
*	函 数 名: Color_Bar_OFF
*	功能说明: 关测试色盘
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void Color_Bar_OFF(void)
{
/*	
Display Test Color Bar
0b: Disable.
1b: Enable.
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x12);
	temp = RA8876_DataRead();
	temp &= cClrb5;
	RA8876_DataWrite(temp);
}


void VSCAN_T_to_B(void)
{
/*	
Vertical Scan direction
0 : From Top to Bottom
1 : From bottom to Top
PIP window will be disabled when VDIR set as 1.
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x12);
	temp = RA8876_DataRead();
	temp &= cClrb3;
	RA8876_DataWrite(temp);
}
void VSCAN_B_to_T(void)
{
/*	
Vertical Scan direction
0 : From Top to Bottom
1 : From bottom to Top
PIP window will be disabled when VDIR set as 1.
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x12);
	temp = RA8876_DataRead();
	temp |= cSetb3;
	RA8876_DataWrite(temp);
}
void PDATA_Set_RGB(void)
{
/*parallel PDATA[23:0] Output Sequence
000b : RGB
001b : RBG
010b : GRB
011b : GBR
100b : BRG
101b : BGR
110b : Gray			
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x12);
	temp = RA8876_DataRead();
    temp &=0xf8;
	RA8876_DataWrite(temp);
}
void PDATA_Set_RBG(void)
{
/*parallel PDATA[23:0] Output Sequence
000b : RGB
001b : RBG
010b : GRB
011b : GBR
100b : BRG
101b : BGR
110b : Gray			
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x12);
	temp = RA8876_DataRead();
	temp &=0xf8;
    temp |= cSetb0;
	RA8876_DataWrite(temp);
}
void PDATA_Set_GRB(void)
{
/*parallel PDATA[23:0] Output Sequence
000b : RGB
001b : RBG
010b : GRB
011b : GBR
100b : BRG
101b : BGR
110b : Gray			
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x12);
	temp = RA8876_DataRead();
	temp &=0xf8;
    temp |= cSetb1;
	RA8876_DataWrite(temp);
}
void PDATA_Set_GBR(void)
{
/*parallel PDATA[23:0] Output Sequence
000b : RGB
001b : RBG
010b : GRB
011b : GBR
100b : BRG
101b : BGR
110b : Gray			
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x12);
	temp = RA8876_DataRead();
	temp &=0xf8;
    temp |= cSetb1;
    temp |= cSetb0;
	RA8876_DataWrite(temp);
}
void PDATA_Set_BRG(void)
{
/*parallel PDATA[23:0] Output Sequence
000b : RGB
001b : RBG
010b : GRB
011b : GBR
100b : BRG
101b : BGR
110b : Gray			
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x12);
	temp = RA8876_DataRead();
	temp &=0xf8;
	temp |= cSetb2;
	RA8876_DataWrite(temp);
}
void PDATA_Set_BGR(void)
{
/*parallel PDATA[23:0] Output Sequence
000b : RGB
001b : RBG
010b : GRB
011b : GBR
100b : BRG
101b : BGR
110b : Gray			
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x12);
	temp = RA8876_DataRead();
	temp &=0xf8;
	temp |= cSetb2;
    temp |= cSetb0;
	RA8876_DataWrite(temp);
}
void PDATA_Set_Gray(void)
{
/*parallel PDATA[23:0] Output Sequence
000b : RGB
001b : RBG
010b : GRB
011b : GBR
100b : BRG
101b : BGR
110b : Gray			
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x12);
	temp = RA8876_DataRead();
	temp &=0xf8;
	temp |= cSetb2;
    temp |= cSetb1;
	RA8876_DataWrite(temp);
}

void PDATA_IDLE_STATE(void)
{
/*
111b : XPDAT pins send out PDAT idle state (all 0 or 1, black or white color).
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x12);
	temp = RA8876_DataRead();
	temp |=0x07;
	RA8876_DataWrite(temp);
}


//[13h]=========================================================================
void HSYNC_Low_Active(void)
{
/*	
HSYNC Polarity
0 : Low active.
1 : High active.
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x13);
	temp = RA8876_DataRead();
    temp &= cClrb7;
	RA8876_DataWrite(temp);
}
void HSYNC_High_Active(void)
{
/*	
HSYNC Polarity
0 : Low active.
1 : High active.
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x13);
	temp = RA8876_DataRead();
	temp |= cSetb7;
	RA8876_DataWrite(temp);
}
void VSYNC_Low_Active(void)
{
/*	
VSYNC Polarity
0 : Low active.
1 : High active.
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x13);
	temp = RA8876_DataRead();
    temp &= cClrb6;
	RA8876_DataWrite(temp);
}
void VSYNC_High_Active(void)
{
/*	
VSYNC Polarity
0 : Low active.
1 : High active.
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x13);
	temp = RA8876_DataRead();
	temp |= cSetb6;
	RA8876_DataWrite(temp);
}
void DE_Low_Active(void)
{
/*	
DE Polarity
0 : High active.
1 : Low active.
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x13);
	temp = RA8876_DataRead();
    temp |= cSetb5;
	RA8876_DataWrite(temp);
}
void DE_High_Active(void)
{
/*	
DE Polarity
0 : High active.
1 : Low active.
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x13);
	temp = RA8876_DataRead();
	temp &= cClrb5;
	RA8876_DataWrite(temp);
}
void Idle_DE_Low(void)
{
/*	
DE IDLE STATE(When STANDBY or DISPLAY OFF )
0 : Pin ¨DE〃 output is low.
1 : Pin ¨DE〃 output is high.
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x13);
	temp = RA8876_DataRead();
    temp &= cClrb4;
	RA8876_DataWrite(temp);
}
void Idle_DE_High(void)
{
/*	
DE IDLE STATE(When STANDBY or DISPLAY OFF )
0 : Pin ¨DE〃 output is low.
1 : Pin ¨DE〃 output is high.
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x13);
	temp = RA8876_DataRead();
	temp |= cSetb4;
	RA8876_DataWrite(temp);
}
void Idle_PCLK_Low(void)
{
/*	
PCLK IDLE STATE(When STANDBY or DISPLAY OFF )
0 : Pin ¨PCLK〃 output is low.
1 : Pin ¨PCLK〃 output is high.
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x13);
	temp = RA8876_DataRead();
    temp &= cClrb3;
	RA8876_DataWrite(temp);
}
void Idle_PCLK_High(void)
{
/*	
PCLK IDLE STATE(When STANDBY or DISPLAY OFF )
0 : Pin ¨PCLK〃 output is low.
1 : Pin ¨PCLK〃 output is high.
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x13);
	temp = RA8876_DataRead();
	temp |= cSetb3;
	RA8876_DataWrite(temp);
}
void Idle_PDAT_Low(void)
{
/*	
PDAT IDLE STATE(When STANDBY or DISPLAY OFF )
0 : Pins ¨PDAT[23:0]〃 output is low.
1 : Pins ¨PCLK[23:0]〃 output is high.
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x13);
	temp = RA8876_DataRead();
    temp &= cClrb2;
	RA8876_DataWrite(temp);
}
void Idle_PDAT_High(void)
{
/*	
PDAT IDLE STATE(When STANDBY or DISPLAY OFF )
0 : Pins ¨PDAT[23:0]〃 output is low.
1 : Pins ¨PCLK[23:0]〃 output is high.
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x13);
	temp = RA8876_DataRead();
	temp |= cSetb2;
	RA8876_DataWrite(temp);
}
void Idle_HSYNC_Low(void)
{
/*	
HSYNC IDLE STATE(When STANDBY or DISPLAY OFF )
0 : Pin ¨HSYNC〃 output is low.
1 : Pin ¨HSYNC〃 output is high.
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x13);
	temp = RA8876_DataRead();
    temp &= cClrb1;
	RA8876_DataWrite(temp);
}
void Idle_HSYNC_High(void)
{
/*	
HSYNC IDLE STATE(When STANDBY or DISPLAY OFF )
0 : Pin ¨HSYNC〃 output is low.
1 : Pin ¨HSYNC〃 output is high.
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x13);
	temp = RA8876_DataRead();
	temp |= cSetb1;
	RA8876_DataWrite(temp);
}
void Idle_VSYNC_Low(void)
{
/*	
VSYNC IDLE STATE(When STANDBY or DISPLAY OFF )
0 : Pin ¨VSYNC〃 output is low.
1 : Pin ¨VSYNC〃 output is high.
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x13);
	temp = RA8876_DataRead();
    temp &= cClrb0;
	RA8876_DataWrite(temp);
}
void Idle_VSYNC_High(void)
{
/*	
VSYNC IDLE STATE(When STANDBY or DISPLAY OFF )
0 : Pin ¨VSYNC〃 output is low.
1 : Pin ¨VSYNC〃 output is high.
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x13);
	temp = RA8876_DataRead();
	temp |= cSetb0;
	RA8876_DataWrite(temp);
}

//[14h][15h][1Ah][1Bh]=========================================================================
void LCD_HorizontalWidth_VerticalHeight(uint16_t WX,uint16_t HY)
{
/* 水平宽度和垂直高度设定（跟LCD屏的分辨率有关） */
/*
[14h] Horizontal Display Width Setting Bit[7:0]
[15h] Horizontal Display Width Fine Tuning (HDWFT) [3:0]
The register specifies the LCD panel horizontal display width in
the unit of 8 pixels resolution.
Horizontal display width(pixels) = (HDWR + 1) * 8 + HDWFTR

[1Ah] Vertical Display Height Bit[7:0]
Vertical Display Height(Line) = VDHR + 1
[1Bh] Vertical Display Height Bit[10:8]
Vertical Display Height(Line) = VDHR + 1
*/
	uint8_t temp;
	if(WX<8)
    {
	RA8876_CmdWrite(0x14);
	RA8876_DataWrite(0x00);
    
	RA8876_CmdWrite(0x15);
	RA8876_DataWrite(WX);
    
    temp=HY-1;
	RA8876_CmdWrite(0x1A);
	RA8876_DataWrite(temp);
	    
	temp=(HY-1)>>8;
	RA8876_CmdWrite(0x1B);
	RA8876_DataWrite(temp);
	}
	else
	{
    temp=(WX/8)-1;
	RA8876_CmdWrite(0x14);
	RA8876_DataWrite(temp);
    
    temp=WX%8;
	RA8876_CmdWrite(0x15);
	RA8876_DataWrite(temp);
    
    temp=HY-1;
	RA8876_CmdWrite(0x1A);
	RA8876_DataWrite(temp);
	    
	temp=(HY-1)>>8;
	RA8876_CmdWrite(0x1B);
	RA8876_DataWrite(temp);
	}
}
//[16h][17h]=========================================================================
void LCD_Horizontal_Non_Display(uint16_t WX)
{
/*
[16h] Horizontal Non-Display Period(HNDR) Bit[4:0]
This register specifies the horizontal non-display period. Also
called back porch.
Horizontal non-display period(pixels) = (HNDR + 1) * 8 + HNDFTR

[17h] Horizontal Non-Display Period Fine Tuning(HNDFT) [3:0]
This register specifies the fine tuning for horizontal non-display
period; it is used to support the SYNC mode panel. Each level of
this modulation is 1-pixel.
Horizontal non-display period(pixels) = (HNDR + 1) * 8 + HNDFTR
*/
	uint8_t temp;
	if(WX<8)
	{
	RA8876_CmdWrite(0x16);
	RA8876_DataWrite(0x00);
    
	RA8876_CmdWrite(0x17);
	RA8876_DataWrite(WX);
	}
	else
	{
    temp=(WX/8)-1;
	RA8876_CmdWrite(0x16);
	RA8876_DataWrite(temp);
    
    temp=WX%8;
	RA8876_CmdWrite(0x17);
	RA8876_DataWrite(temp);
	}	
}
//[18h]=========================================================================
void LCD_HSYNC_Start_Position(uint16_t WX)
{
/*
[18h] HSYNC Start Position[4:0]
The starting position from the end of display area to the
beginning of HSYNC. Each level of this modulation is 8-pixel.
Also called front porch.
HSYNC Start Position(pixels) = (HSTR + 1)x8
*/
	uint8_t temp;
	if(WX<8)
	{
	RA8876_CmdWrite(0x18);
	RA8876_DataWrite(0x00);
	}
	else
	{
    temp=(WX/8)-1;
	RA8876_CmdWrite(0x18);
	RA8876_DataWrite(temp);	
	}
}
//[19h]=========================================================================
void LCD_HSYNC_Pulse_Width(uint16_t WX)
{
/* HSYNC周期宽度 */
/*
[19h] HSYNC Pulse Width(HPW) [4:0]
The period width of HSYNC.
HSYNC Pulse Width(pixels) = (HPW + 1)x8
*/
	uint8_t temp;
	if(WX<8)
	{
	RA8876_CmdWrite(0x19);
	RA8876_DataWrite(0x00);
	}
	else
	{
    temp=(WX/8)-1;
	RA8876_CmdWrite(0x19);
	RA8876_DataWrite(temp);	
	}
}
//[1Ch][1Dh]=========================================================================
void LCD_Vertical_Non_Display(uint16_t HY)
{
/*
[1Ch] Vertical Non-Display Period Bit[7:0]
Vertical Non-Display Period(Line) = (VNDR + 1)

[1Dh] Vertical Non-Display Period Bit[9:8]
Vertical Non-Display Period(Line) = (VNDR + 1)
*/
	uint16_t temp;
    temp=HY-1;
	RA8876_CmdWrite(0x1C);
	RA8876_DataWrite(temp);

	RA8876_CmdWrite(0x1D);
	RA8876_DataWrite(temp>>8);
}
//[1Eh]=========================================================================
void LCD_VSYNC_Start_Position(uint16_t HY)
{
/*
[1Eh] VSYNC Start Position[7:0]
The starting position from the end of display area to the beginning of VSYNC.
VSYNC Start Position(Line) = (VSTR + 1)
*/
	uint8_t temp;
    temp=HY-1;
	RA8876_CmdWrite(0x1E);
	RA8876_DataWrite(temp);
}
//[1Fh]=========================================================================
void LCD_VSYNC_Pulse_Width(uint16_t HY)
{
/*
[1Fh] VSYNC Pulse Width[5:0]
The pulse width of VSYNC in lines.
VSYNC Pulse Width(Line) = (VPWR + 1)
*/
	uint8_t temp;
    temp=HY-1;
	RA8876_CmdWrite(0x1F);
	RA8876_DataWrite(temp);
}

/*
*********************************************************************************************************
*	函 数 名: Main_Image_Start_Address
*	功能说明: 主影像起始地址
*	形    参：无
*	返 回 值: 错误代码(无需处理)
*********************************************************************************************************
*/
void Main_Image_Start_Address(unsigned long Addr)	
{
/*
[20h] Main Image Start Address[7:2]
[21h] Main Image Start Address[15:8]
[22h] Main Image Start Address [23:16]
[23h] Main Image Start Address [31:24]
*/
	RA8876_CmdWrite(0x20);	RA8876_DataWrite(Addr);
	RA8876_CmdWrite(0x21);	RA8876_DataWrite(Addr>>8);
	RA8876_CmdWrite(0x22);	RA8876_DataWrite(Addr>>16);
	RA8876_CmdWrite(0x23);	RA8876_DataWrite(Addr>>24);
}

/*
*********************************************************************************************************
*	函 数 名: Main_Image_Width
*	功能说明: 主影像宽度
*	形    参：无
*	返 回 值: 错误代码(无需处理)
*********************************************************************************************************
*/
void Main_Image_Width(uint16_t WX)	
{
/*
[24h] Main Image Width [7:0]
[25h] Main Image Width [12:8]
Unit: Pixel.
It must be divisible by 4. MIW Bit [1:0] tie to ¨0〃 internally.
The value is physical pixel number. Maximum value is 8188 pixels
*/
	RA8876_CmdWrite(0x24);	RA8876_DataWrite(WX);
	RA8876_CmdWrite(0x25);	RA8876_DataWrite(WX>>8);
}

/*
*********************************************************************************************************
*	函 数 名: Main_Window_Start_XY
*	功能说明: 主窗口起始位置
*	形    参：无
*	返 回 值: 错误代码(无需处理)
*********************************************************************************************************
*/
void Main_Window_Start_XY(uint16_t WX,uint16_t HY)	
{
/*
[26h] Main Window Upper-Left corner X-coordination [7:0]
[27h] Main Window Upper-Left corner X-coordination [12:8]
Reference Main Image coordination.
Unit: Pixel
It must be divisible by 4. MWULX Bit [1:0] tie to ¨0〃 internally.
X-axis coordination plus Horizontal display width cannot large than 8188.

[28h] Main Window Upper-Left corner Y-coordination [7:0]
[29h] Main Window Upper-Left corner Y-coordination [12:8]
Reference Main Image coordination.
Unit: Pixel
Range is between 0 and 8191.
*/
	RA8876_CmdWrite(0x26);	RA8876_DataWrite(WX);
	RA8876_CmdWrite(0x27);	RA8876_DataWrite(WX>>8);
	RA8876_CmdWrite(0x28);	RA8876_DataWrite(HY);
	RA8876_CmdWrite(0x29);	RA8876_DataWrite(HY>>8);
}
//[2Ah][2Bh][2Ch][2Dh]=========================================================================
void PIP_Display_Start_XY(uint16_t WX,uint16_t HY)	
{
/*
[2Ah] PIP Window Display Upper-Left corner X-coordination [7:0]
[2Bh] PIP Window Display Upper-Left corner X-coordination [12:8]
Reference Main Window coordination.
Unit: Pixel
It must be divisible by 4. PWDULX Bit [1:0] tie to ¨0〃 internally.
X-axis coordination should less than horizontal display width.
According to bit of Select Configure PIP 1 or 2 Windowˇs parameters. 
Function bit will be configured for relative PIP window.

[2Ch] PIP Window Display Upper-Left corner Y-coordination [7:0]
[2Dh] PIP Window Display Upper-Left corner Y-coordination [12:8]
Reference Main Window coordination.
Unit: Pixel
Y-axis coordination should less than vertical display height.
According to bit of Select Configure PIP 1 or 2 Windowˇs parameters.
Function bit will be configured for relative PIP window.
*/
	RA8876_CmdWrite(0x2A);	RA8876_DataWrite(WX);
	RA8876_CmdWrite(0x2B);	RA8876_DataWrite(WX>>8);
	RA8876_CmdWrite(0x2C);	RA8876_DataWrite(HY);
	RA8876_CmdWrite(0x2D);	RA8876_DataWrite(HY>>8);
}
//[2Eh][2Fh][30h][31h]=========================================================================
void PIP_Image_Start_Address(unsigned long Addr)	
{
/*
[2Eh] PIP Image Start Address[7:2]
[2Fh] PIP Image Start Address[15:8]
[30h] PIP Image Start Address [23:16]
[31h] PIP Image Start Address [31:24]
*/
	RA8876_CmdWrite(0x2E);	RA8876_DataWrite(Addr);
	RA8876_CmdWrite(0x2F);	RA8876_DataWrite(Addr>>8);
	RA8876_CmdWrite(0x30);	RA8876_DataWrite(Addr>>16);
	RA8876_CmdWrite(0x31);	RA8876_DataWrite(Addr>>24);
}
//[32h][33h]=========================================================================
void PIP_Image_Width(uint16_t WX)	
{
/*
[32h] PIP Image Width [7:0]
[33h] PIP Image Width [12:8]
Unit: Pixel.
It must be divisible by 4. PIW Bit [1:0] tie to ¨0〃 internally.
The value is physical pixel number.
This width should less than horizontal display width.
According to bit of Select Configure PIP 1 or 2 Windowˇs parameters.
Function bit will be configured for relative PIP window.
*/
	RA8876_CmdWrite(0x32);	RA8876_DataWrite(WX);
	RA8876_CmdWrite(0x33);	RA8876_DataWrite(WX>>8);
}
//[34h][35h][36h][37h]=========================================================================
void PIP_Window_Image_Start_XY(uint16_t WX,uint16_t HY)	
{

/*
[34h] PIP 1 or 2 Window Image Upper-Left corner X-coordination [7:0]
[35h] PIP Window Image Upper-Left corner X-coordination [12:8]
Reference PIP Image coordination.
Unit: Pixel
It must be divisible by 4. PWIULX Bit [1:0] tie to ¨0〃 internally.
X-axis coordination plus PIP image width cannot large than 8188.
According to bit of Select Configure PIP 1 or 2 Windowˇs parameters. 
Function bit will be configured for relative PIP window.

[36h] PIP Windows Display Upper-Left corner Y-coordination [7:0]
[37h] PIP Windows Image Upper-Left corner Y-coordination [12:8]
Reference PIP Image coordination.
Unit: Pixel
Y-axis coordination plus PIP window height should less than 8191.
According to bit of Select Configure PIP 1 or 2 Windowˇs parameters. 
Function bit will be configured for relative PIP window.
*/
	RA8876_CmdWrite(0x34);	RA8876_DataWrite(WX);
	RA8876_CmdWrite(0x35);	RA8876_DataWrite(WX>>8);
	RA8876_CmdWrite(0x36);	RA8876_DataWrite(HY);
	RA8876_CmdWrite(0x37);	RA8876_DataWrite(HY>>8);
}
//[38h][39h][3Ah][3Bh]=========================================================================
void PIP_Window_Width_Height(uint16_t WX,uint16_t HY)	
{
/*
[38h] PIP Window Width [7:0]
[39h] PIP Window Width [10:8]
Unit: Pixel.
It must be divisible by 4. The value is physical pixel number.
Maximum value is 8188 pixels.
According to bit of Select Configure PIP 1 or 2 Windowˇs parameters. 
Function bit will be configured for relative PIP window.

[3Ah] PIP Window Height [7:0]
[3Bh] PIP Window Height [10:8]
Unit: Pixel
The value is physical pixel number. Maximum value is 8191 pixels.
According to bit of Select Configure PIP 1 or 2 Windowˇs parameters. 
Function bit will be configured for relative PIP window.
*/
	RA8876_CmdWrite(0x38);	RA8876_DataWrite(WX);
	RA8876_CmdWrite(0x39);	RA8876_DataWrite(WX>>8);
	RA8876_CmdWrite(0x3A);	RA8876_DataWrite(HY);
	RA8876_CmdWrite(0x3B);	RA8876_DataWrite(HY>>8);
}

//[3Ch]=========================================================================
void Enable_Gamma_Correction(void)	
{
/*
Gamma correction Enable
0: Disable
1: Enable
Gamma correction is the last output stage.
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x3C);
	temp = RA8876_DataRead();
	temp |= cSetb7;
	RA8876_DataWrite(temp);
}
void Disable_Gamma_Correction(void)	
{
/*
Gamma correction Enable
0: Disable
1: Enable
Gamma correction is the last output stage.
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x3C);
	temp = RA8876_DataRead();
	temp &= cClrb7;
	RA8876_DataWrite(temp);
}
void Gamma_Table_for_Blue(void)	
{
/*
Gamma table select for MPU write gamma data
00b: Gamma table for Blue
01b: Gamma table for Green
10b: Gamma table for Red
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x3C);
	temp = RA8876_DataRead();
	temp &= cClrb6;
	temp &= cClrb5;
	RA8876_DataWrite(temp);
}
void Gamma_Table_for_Green(void)	
{
/*
Gamma table select for MPU write gamma data
00b: Gamma table for Blue
01b: Gamma table for Green
10b: Gamma table for Red
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x3C);
	temp = RA8876_DataRead();
	temp &= cClrb6;
	temp |= cSetb5;
	RA8876_DataWrite(temp);
}
void Gamma_Table_for_Red(void)	
{
/*
Gamma table select for MPU write gamma data
00b: Gamma table for Blue
01b: Gamma table for Green
10b: Gamma table for Red
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x3C);
	temp = RA8876_DataRead();
	temp |= cSetb6;
	temp &= cClrb5;
	RA8876_DataWrite(temp);
}

void Enable_Graphic_Cursor(void)	
{
/*
Graphic Cursor Enable
0 : Graphic Cursor disable.
1 : Graphic Cursor enable.
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x3C);
	temp = RA8876_DataRead();
	temp |= cSetb4;
	RA8876_DataWrite(temp);
}
void Disable_Graphic_Cursor(void)	
{
/*
Graphic Cursor Enable
0 : Graphic Cursor disable.
1 : Graphic Cursor enable.
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x3C);
	temp = RA8876_DataRead();
	temp &= cClrb4;
	RA8876_DataWrite(temp);
}
//
void Select_Graphic_Cursor_1(void)	
{
/*
Graphic Cursor Selection Bit
Select one from four graphic cursor types. (00b to 11b)
00b : Graphic Cursor Set 1.
01b : Graphic Cursor Set 2.
10b : Graphic Cursor Set 3.
11b : Graphic Cursor Set 4.
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x3C);
	temp = RA8876_DataRead();
	temp &= cClrb3;
	temp &= cClrb2;
	RA8876_DataWrite(temp);
}
void Select_Graphic_Cursor_2(void)	
{
/*
Graphic Cursor Selection Bit
Select one from four graphic cursor types. (00b to 11b)
00b : Graphic Cursor Set 1.
01b : Graphic Cursor Set 2.
10b : Graphic Cursor Set 3.
11b : Graphic Cursor Set 4.
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x3C);
	temp = RA8876_DataRead();
	temp &= cClrb3;
	temp |= cSetb2;
	RA8876_DataWrite(temp);
}
void Select_Graphic_Cursor_3(void)	
{
/*
Graphic Cursor Selection Bit
Select one from four graphic cursor types. (00b to 11b)
00b : Graphic Cursor Set 1.
01b : Graphic Cursor Set 2.
10b : Graphic Cursor Set 3.
11b : Graphic Cursor Set 4.
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x3C);
	temp = RA8876_DataRead();
	temp |= cSetb3;
	temp &= cClrb2;
	RA8876_DataWrite(temp);
}
void Select_Graphic_Cursor_4(void)	
{
/*
Graphic Cursor Selection Bit
Select one from four graphic cursor types. (00b to 11b)
00b : Graphic Cursor Set 1.
01b : Graphic Cursor Set 2.
10b : Graphic Cursor Set 3.
11b : Graphic Cursor Set 4.
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x3C);
	temp = RA8876_DataRead();
	temp |= cSetb3;
	temp |= cSetb2;
	RA8876_DataWrite(temp);
}
//
void Enable_Text_Cursor(void)	
{
/*
Text Cursor Enable
0 : Disable.
1 : Enable.
Text cursor & Graphic cursor cannot enable simultaneously.
Graphic cursor has higher priority then Text cursor if enabled simultaneously.
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x3C);
	temp = RA8876_DataRead();
	temp |= cSetb1;
	RA8876_DataWrite(temp);
}
void Disable_Text_Cursor(void)	
{
/*
Text Cursor Enable
0 : Disable.
1 : Enable.
Text cursor & Graphic cursor cannot enable simultaneously.
Graphic cursor has higher priority then Text cursor if enabled simultaneously.
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x3C);
	temp = RA8876_DataRead();
	temp &= cClrb1;
	RA8876_DataWrite(temp);
}
//
void Enable_Text_Cursor_Blinking(void)	
{
/*
Text Cursor Blinking Enable
0 : Disable.
1 : Enable.
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x3C);
	temp = RA8876_DataRead();
	temp |= cSetb0;
	RA8876_DataWrite(temp);
}
void Disable_Text_Cursor_Blinking(void)	
{
/*
Text Cursor Blinking Enable
0 : Disable.
1 : Enable.
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x3C);
	temp = RA8876_DataRead();
	temp &= cClrb0;
	RA8876_DataWrite(temp);
}
//[3Dh]=========================================================================
void Blinking_Time_Frames(uint8_t temp)	
{
/*
Text Cursor Blink Time Setting (Unit: Frame)
00h : 1 frame time.
01h : 2 frames time.
02h : 3 frames time.
:
FFh : 256 frames time.
*/
	RA8876_CmdWrite(0x3D);
	RA8876_DataWrite(temp);
}
//[3Eh]=========================================================================
void Text_Cursor_H_V(uint16_t WX,uint16_t HY)
{
/*
[3Eh]
Text Cursor Horizontal Size Setting[4:0]
Unit : Pixel
Zero-based number. Value ¨0〃 means 1 pixel.
Note : When font is enlarged, the cursor setting will multiply the
same times as the font enlargement.
[3Fh]
Text Cursor Vertical Size Setting[4:0]
Unit : Pixel
Zero-based number. Value ¨0〃 means 1 pixel.
Note : When font is enlarged, the cursor setting will multiply the
same times as the font enlargement.
*/
	RA8876_CmdWrite(0x3E);
	RA8876_DataWrite(WX);
	RA8876_CmdWrite(0x3F);
	RA8876_DataWrite(HY);
}
//[40h][41h][42h][43h]=========================================================================
void Graphic_Cursor_XY(uint16_t WX,uint16_t HY)
{
/*
[40h] Graphic Cursor Horizontal Location[7:0]
[41h] Graphic Cursor Horizontal Location[12:8]
[42h] Graphic Cursor Vertical Location[7:0]
[43h] Graphic Cursor Vertical Location[12:8]
Reference main Window coordination.
*/	
	RA8876_CmdWrite(0x40);	RA8876_DataWrite(WX);
	RA8876_CmdWrite(0x41);	RA8876_DataWrite(WX>>8);
	RA8876_CmdWrite(0x42);	RA8876_DataWrite(HY);
	RA8876_CmdWrite(0x43);	RA8876_DataWrite(HY>>8);
}
//[44h]=========================================================================
void Set_Graphic_Cursor_Color_1(uint8_t temp)
{
/*
[44h] Graphic Cursor Color 0 with 256 Colors
RGB Format [7:0] = RRRGGGBB.
*/	
	LCD_RegisterWrite(0x44,temp);
}
//[45h]=========================================================================
void Set_Graphic_Cursor_Color_2(uint8_t temp)
{
/*
[45h] Graphic Cursor Color 1 with 256 Colors
RGB Format [7:0] = RRRGGGBB.
*/	
	LCD_RegisterWrite(0x45,temp);
}

/*
*********************************************************************************************************
*	函 数 名: Canvas_Image_Start_address
*	功能说明: 覆盖镜像首地址
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void Canvas_Image_Start_address(unsigned long Addr)	
{
/*
[50h] Start address of Canvas [7:0]
[51h] Start address of Canvas [15:8]
[52h] Start address of Canvas [23:16]
[53h] Start address of Canvas [31:24]
*/
	RA8876_CmdWrite(0x50);	RA8876_DataWrite(Addr);
	RA8876_CmdWrite(0x51);	RA8876_DataWrite(Addr>>8);
	RA8876_CmdWrite(0x52);	RA8876_DataWrite(Addr>>16);
	RA8876_CmdWrite(0x53);	RA8876_DataWrite(Addr>>24);
}

/*
*********************************************************************************************************
*	函 数 名: Canvas_image_width
*	功能说明: 画布图像宽度
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void Canvas_image_width(uint16_t WX)	
{
/*
[54h] Canvas image width [7:2]
[55h] Canvas image width [12:8]
*/
	RA8876_CmdWrite(0x54);	RA8876_DataWrite(WX);
	RA8876_CmdWrite(0x55);	RA8876_DataWrite(WX>>8);	
}

/*
*********************************************************************************************************
*	函 数 名: Active_Window_XY
*	功能说明: 活动窗口XY
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void Active_Window_XY(uint16_t WX,uint16_t HY)	
{
/*
[56h] Active Window Upper-Left corner X-coordination [7:0]
[57h] Active Window Upper-Left corner X-coordination [12:8]
[58h] Active Window Upper-Left corner Y-coordination [7:0]
[59h] Active Window Upper-Left corner Y-coordination [12:8]
*/
	RA8876_CmdWrite(0x56);	RA8876_DataWrite(WX);
	RA8876_CmdWrite(0x57);	RA8876_DataWrite(WX>>8);
	RA8876_CmdWrite(0x58);	RA8876_DataWrite(HY);
	RA8876_CmdWrite(0x59);	RA8876_DataWrite(HY>>8);

}

/*
*********************************************************************************************************
*	函 数 名: Active_Window_XY
*	功能说明: 活动窗口高度和宽度
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void Active_Window_WH(uint16_t WX,uint16_t HY)	
{
/*
[5Ah] Width of Active Window [7:0]
[5Bh] Width of Active Window [12:8]
[5Ch] Height of Active Window [7:0]
[5Dh] Height of Active Window [12:8]
*/
	RA8876_CmdWrite(0x5A);	RA8876_DataWrite(WX);
	RA8876_CmdWrite(0x5B);	RA8876_DataWrite(WX>>8);
	RA8876_CmdWrite(0x5C);	RA8876_DataWrite(HY);
	RA8876_CmdWrite(0x5D);	RA8876_DataWrite(HY>>8);
}
//[5Eh]=========================================================================
/*
*********************************************************************************************************
*	函 数 名: Memory_XY_Mode
*	功能说明: 块模式
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void Memory_XY_Mode(void)	
{
/*
Canvas addressing mode
0: Block mode (X-Y coordination addressing)
1: linear mode
*/
/*
Canvas addressing mode
0: Block 模式 (X-Y 坐标寻址方法)。
1: Linear 模式。	
*/
	uint8_t temp;

	RA8876_CmdWrite(0x5E);
	temp = RA8876_DataRead();
	temp &= cClrb2;
	RA8876_DataWrite(temp);
}

/*
*********************************************************************************************************
*	函 数 名: Memory_Linear_Mode
*	功能说明: 线性模式
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void Memory_Linear_Mode(void)	
{
/*
Canvas addressing mode
0: Block mode (X-Y coordination addressing)
1: linear mode
*/
	uint8_t temp;

	RA8876_CmdWrite(0x5E);
	temp = RA8876_DataRead();
	temp |= cSetb2;
	RA8876_DataWrite(temp);
}
void Memory_8bpp_Mode(void)	
{
/*
Canvas imageˇs color depth & memory R/W data width
In Block Mode:
00: 8bpp
01: 16bpp
1x: 24bpp
In Linear Mode:
x0: 8-bits memory data read/write.
x1: 16-bits memory data read/write
*/
	uint8_t temp;

	RA8876_CmdWrite(0x5E);
	temp = RA8876_DataRead();
	temp &= cClrb1;
	temp &= cClrb0;
	RA8876_DataWrite(temp);
}

/*
*********************************************************************************************************
*	函 数 名: Memory_16bpp_Mode
*	功能说明: 设置为16位模式
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void Memory_16bpp_Mode(void)	
{
/*
	Color Depth of Canvas & Active Window 
bit1-0：	
	Canvas imageˇs color depth & memory R/W data width
	In Block Mode:
	00: 8bpp
	01: 16bpp
	1x: 24bpp
	注 : 单色数据的输入方法，可以使用任何一个色深，并搭配适合
	的图像宽度，即可正确输入。
	In Linear Mode:
	X0: 8-bits 内存数据读写。
	X1: 16-bits 内存数据读写。
*/
	uint8_t temp;

	RA8876_CmdWrite(0x5E);
	temp = RA8876_DataRead();
	temp &= cClrb1;
	temp |= cSetb0;
	RA8876_DataWrite(temp);
}
void Memory_24bpp_Mode(void)	
{
/*
Canvas imageˇs color depth & memory R/W data width
In Block Mode:
00: 8bpp
01: 16bpp
1x: 24bpp
In Linear Mode:
x0: 8-bits memory data read/write.
x1: 16-bits memory data read/write
*/
	uint8_t temp;

	RA8876_CmdWrite(0x5E);
	temp = RA8876_DataRead();
	temp |= cSetb1;
//	temp |= cSetb0;
	RA8876_DataWrite(temp);
}

/*
*********************************************************************************************************
*	函 数 名: Goto_Pixel_XY
*	功能说明: 设置图形的读写位置
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void Goto_Pixel_XY(uint16_t WX,uint16_t HY)	
{
/*
Set Graphic Read/Write position

REG[5Eh] bit3, Select to read back Graphic Read/Write position.
When DPRAM Linear mode:Graphic Read/Write Position [31:24][23:16][15:8][7:0]
When DPRAM Active window mode:Graphic Read/Write 
Horizontal Position [12:8][7:0], 
Vertical Position [12:8][7:0].
Reference Canvas image coordination. Unit: Pixel
*/
	RA8876_CmdWrite(0x5F);
	RA8876_DataWrite(WX);
	RA8876_CmdWrite(0x60);
	RA8876_DataWrite(WX>>8);
	RA8876_CmdWrite(0x61);
	RA8876_DataWrite(HY);
	RA8876_CmdWrite(0x62);
	RA8876_DataWrite(HY>>8);
}
void Goto_Linear_Addr(unsigned long Addr)
{
/*
Set Graphic Read/Write position

REG[5Eh] bit3, Select to read back Graphic Read/Write position.
When DPRAM Linear mode:Graphic Read/Write Position [31:24][23:16][15:8][7:0]
When DPRAM Active window mode:Graphic Read/Write 
Horizontal Position [12:8][7:0], 
Vertical Position [12:8][7:0].
Reference Canvas image coordination. Unit: Pixel
*/
	RA8876_CmdWrite(0x5F);	RA8876_DataWrite(Addr);
	RA8876_CmdWrite(0x60);	RA8876_DataWrite(Addr>>8);
	RA8876_CmdWrite(0x61);	RA8876_DataWrite(Addr>>16);
	RA8876_CmdWrite(0x62);	RA8876_DataWrite(Addr>>24);
}

//[63h][64h][65h][66h]=========================================================================
void Goto_Text_XY(uint16_t WX,uint16_t HY)	
{
/*
Set Text Write position
Text Write X-coordination [12:8][7:0]
Text Write Y-coordination [12:8][7:0]
Reference Canvas image coordination.
Unit: Pixel
*/
	RA8876_CmdWrite(0x63);	RA8876_DataWrite(WX);
	RA8876_CmdWrite(0x64);	RA8876_DataWrite(WX>>8);
	RA8876_CmdWrite(0x65);	RA8876_DataWrite(HY);
	RA8876_CmdWrite(0x66);	RA8876_DataWrite(HY>>8);
}

//[67h]=========================================================================
/*
[bit7]Draw Line / Triangle Start Signal
Write Function
0 : Stop the drawing function.
1 : Start the drawing function.
Read Function
0 : Drawing function complete.
1 : Drawing function is processing.
[bit5]Fill function for Triangle Signal
0 : Non fill.
1 : Fill.
[bit1]Draw Triangle or Line Select Signal
0 : Draw Line
1 : Draw Triangle
*/
void Start_Line(void)
{
	RA8876_CmdWrite(0x67);
	RA8876_DataWrite(0x80);

	Check_2D_Busy();
}
void Start_Triangle(void)
{
	RA8876_CmdWrite(0x67);
	RA8876_DataWrite(0x82);//B1000_0010

	Check_2D_Busy();
}
void Start_Triangle_Fill(void)
{
	RA8876_CmdWrite(0x67);
	RA8876_DataWrite(0xA2);//B1010_0010

	Check_2D_Busy();
}
//[68h][69h][6Ah][6Bh]=========================================================================
//絬癬翴
void Line_Start_XY(uint16_t WX,uint16_t HY)
{
/*
[68h] Draw Line/Square/Triangle Start X-coordination [7:0]
[69h] Draw Line/Square/Triangle Start X-coordination [12:8]
[6Ah] Draw Line/Square/Triangle Start Y-coordination [7:0]
[6Bh] Draw Line/Square/Triangle Start Y-coordination [12:8]
*/
	RA8876_CmdWrite(0x68);
	RA8876_DataWrite(WX);

	RA8876_CmdWrite(0x69);
	RA8876_DataWrite(WX>>8);

	RA8876_CmdWrite(0x6A);
	RA8876_DataWrite(HY);

	RA8876_CmdWrite(0x6B);
	RA8876_DataWrite(HY>>8);
}
//[6Ch][6Dh][6Eh][6Fh]=========================================================================
//絬沧翴
void Line_End_XY(uint16_t WX,uint16_t HY)
{
/*
[6Ch] Draw Line/Square/Triangle End X-coordination [7:0]
[6Dh] Draw Line/Square/Triangle End X-coordination [12:8]
[6Eh] Draw Line/Square/Triangle End Y-coordination [7:0]
[6Fh] Draw Line/Square/Triangle End Y-coordination [12:8]
*/
	RA8876_CmdWrite(0x6C);
	RA8876_DataWrite(WX);

	RA8876_CmdWrite(0x6D);
	RA8876_DataWrite(WX>>8);

	RA8876_CmdWrite(0x6E);
	RA8876_DataWrite(HY);

	RA8876_CmdWrite(0x6F);
	RA8876_DataWrite(HY>>8);
}
//[68h]~[73h]=========================================================================
//à-翴1
void Triangle_Point1_XY(uint16_t WX,uint16_t HY)
{
/*
[68h] Draw Line/Square/Triangle Start X-coordination [7:0]
[69h] Draw Line/Square/Triangle Start X-coordination [12:8]
[6Ah] Draw Line/Square/Triangle Start Y-coordination [7:0]
[6Bh] Draw Line/Square/Triangle Start Y-coordination [12:8]
*/
	RA8876_CmdWrite(0x68);
	RA8876_DataWrite(WX);

	RA8876_CmdWrite(0x69);
	RA8876_DataWrite(WX>>8);

	RA8876_CmdWrite(0x6A);
	RA8876_DataWrite(HY);

	RA8876_CmdWrite(0x6B);
	RA8876_DataWrite(HY>>8);
}
//à-翴2
void Triangle_Point2_XY(uint16_t WX,uint16_t HY)
{
/*
[6Ch] Draw Line/Square/Triangle End X-coordination [7:0]
[6Dh] Draw Line/Square/Triangle End X-coordination [12:8]
[6Eh] Draw Line/Square/Triangle End Y-coordination [7:0]
[6Fh] Draw Line/Square/Triangle End Y-coordination [12:8]
*/
	RA8876_CmdWrite(0x6C);
	RA8876_DataWrite(WX);

	RA8876_CmdWrite(0x6D);
	RA8876_DataWrite(WX>>8);

	RA8876_CmdWrite(0x6E);
	RA8876_DataWrite(HY);

	RA8876_CmdWrite(0x6F);
	RA8876_DataWrite(HY>>8);
}
//à-翴3
void Triangle_Point3_XY (uint16_t WX,uint16_t HY)
{
/*
[70h] Draw Triangle Point 3 X-coordination [7:0]
[71h] Draw Triangle Point 3 X-coordination [12:8]
[72h] Draw Triangle Point 3 Y-coordination [7:0]
[73h] Draw Triangle Point 3 Y-coordination [12:8]
*/
	RA8876_CmdWrite(0x70);
	RA8876_DataWrite(WX);

	RA8876_CmdWrite(0x71);
	RA8876_DataWrite(WX>>8);

	RA8876_CmdWrite(0x72);
	RA8876_DataWrite(HY);

	RA8876_CmdWrite(0x73);
	RA8876_DataWrite(HY>>8);
}
//よ癬翴
void Square_Start_XY(uint16_t WX,uint16_t HY)
{
/*
[68h] Draw Line/Square/Triangle Start X-coordination [7:0]
[69h] Draw Line/Square/Triangle Start X-coordination [12:8]
[6Ah] Draw Line/Square/Triangle Start Y-coordination [7:0]
[6Bh] Draw Line/Square/Triangle Start Y-coordination [12:8]
*/
	RA8876_CmdWrite(0x68);
	RA8876_DataWrite(WX);

	RA8876_CmdWrite(0x69);
	RA8876_DataWrite(WX>>8);

	RA8876_CmdWrite(0x6A);
	RA8876_DataWrite(HY);

	RA8876_CmdWrite(0x6B);
	RA8876_DataWrite(HY>>8);
}
//よ沧翴
void Square_End_XY(uint16_t WX,uint16_t HY)
{
/*
[6Ch] Draw Line/Square/Triangle End X-coordination [7:0]
[6Dh] Draw Line/Square/Triangle End X-coordination [12:8]
[6Eh] Draw Line/Square/Triangle End Y-coordination [7:0]
[6Fh] Draw Line/Square/Triangle End Y-coordination [12:8]
*/
	RA8876_CmdWrite(0x6C);
	RA8876_DataWrite(WX);

	RA8876_CmdWrite(0x6D);
	RA8876_DataWrite(WX>>8);

	RA8876_CmdWrite(0x6E);
	RA8876_DataWrite(HY);

	RA8876_CmdWrite(0x6F);
	RA8876_DataWrite(HY>>8);
}
//[76h]=========================================================================
/*
[bit7]
Draw Circle / Ellipse / Square /Circle Square Start Signal 
Write Function
0 : Stop the drawing function.
1 : Start the drawing function.
Read Function
0 : Drawing function complete.
1 : Drawing function is processing.
[bit6]
Fill the Circle / Ellipse / Square / Circle Square Signal
0 : Non fill.
1 : fill.
[bit5 bit4]
Draw Circle / Ellipse / Square / Ellipse Curve / Circle Square Select
00 : Draw Circle / Ellipse
01 : Draw Circle / Ellipse Curve
10 : Draw Square.
11 : Draw Circle Square.
[bit1 bit0]
Draw Circle / Ellipse Curve Part Select
00 : bottom-left Ellipse Curve
01 : upper-left Ellipse Curve
10 : upper-right Ellipse Curve
11 : bottom-right Ellipse Curve
*/
void Start_Circle_or_Ellipse(void)
{
	RA8876_CmdWrite(0x76);
	RA8876_DataWrite(0x80);//B1000_XXXX

	Check_2D_Busy();
}
void Start_Circle_or_Ellipse_Fill(void)
{
	RA8876_CmdWrite(0x76);
	RA8876_DataWrite(0xC0);//B1100_XXXX

	Check_2D_Busy();
}
//
void Start_Left_Down_Curve(void)
{
	RA8876_CmdWrite(0x76);
	RA8876_DataWrite(0x90);//B1001_XX00

	Check_2D_Busy();
}
void Start_Left_Up_Curve(void)
{
	RA8876_CmdWrite(0x76);
	RA8876_DataWrite(0x91);//B1001_XX01

	Check_2D_Busy();
}
void Start_Right_Up_Curve(void)
{
	RA8876_CmdWrite(0x76);
	RA8876_DataWrite(0x92);//B1001_XX10

	Check_2D_Busy();	
}
void Start_Right_Down_Curve(void)
{
	RA8876_CmdWrite(0x76);
	RA8876_DataWrite(0x93);//B1001_XX11

	Check_2D_Busy();	
}
//
void Start_Left_Down_Curve_Fill(void)
{
	RA8876_CmdWrite(0x76);
	RA8876_DataWrite(0xD0);//B1101_XX00

	Check_2D_Busy();
}
void Start_Left_Up_Curve_Fill(void)
{
	RA8876_CmdWrite(0x76);
	RA8876_DataWrite(0xD1);//B1101_XX01

	Check_2D_Busy();
}
void Start_Right_Up_Curve_Fill(void)
{
	RA8876_CmdWrite(0x76);
	RA8876_DataWrite(0xD2);//B1101_XX10

	Check_2D_Busy();
}
void Start_Right_Down_Curve_Fill(void)
{
	RA8876_CmdWrite(0x76);
	RA8876_DataWrite(0xD3);//B1101_XX11

	Check_2D_Busy();
}
//
void Start_Square(void)
{
	RA8876_CmdWrite(0x76);
	RA8876_DataWrite(0xA0);//B1010_XXXX

	Check_2D_Busy();
}
void Start_Square_Fill(void)
{
	RA8876_CmdWrite(0x76);
	RA8876_DataWrite(0xE0);//B1110_XXXX

	Check_2D_Busy();
}
void Start_Circle_Square(void)
{
	RA8876_CmdWrite(0x76);
	RA8876_DataWrite(0xB0);//B1011_XXXX

	Check_2D_Busy();	
}
void Start_Circle_Square_Fill(void)
{
	RA8876_CmdWrite(0x76);
	RA8876_DataWrite(0xF0);//B1111_XXXX

	Check_2D_Busy();
}
//[77h]~[7Eh]=========================================================================
//蛾畖
void Circle_Radius_R (uint16_t WX)
{
/*
[77h] Draw Circle/Ellipse/Circle Square Major radius [7:0]
[78h] Draw Circle/Ellipse/Circle Square Major radius [12:8]
[79h] Draw Circle/Ellipse/Circle Square Minor radius [7:0]
[7Ah] Draw Circle/Ellipse/Circle Square Minor radius [12:8]
*/
	RA8876_CmdWrite(0x77);
	RA8876_DataWrite(WX);

	RA8876_CmdWrite(0x78);
	RA8876_DataWrite(WX>>8);

	RA8876_CmdWrite(0x79);
	RA8876_DataWrite(WX);

	RA8876_CmdWrite(0x7A);
	RA8876_DataWrite(WX>>8);
}

//掘蛾畖
void Ellipse_Radius_RxRy (uint16_t WX,uint16_t HY)
{
/*
[77h] Draw Circle/Ellipse/Circle Square Major radius [7:0]
[78h] Draw Circle/Ellipse/Circle Square Major radius [12:8]
[79h] Draw Circle/Ellipse/Circle Square Minor radius [7:0]
[7Ah] Draw Circle/Ellipse/Circle Square Minor radius [12:8]
*/
	RA8876_CmdWrite(0x77);
	RA8876_DataWrite(WX);

	RA8876_CmdWrite(0x78);
	RA8876_DataWrite(WX>>8);

	RA8876_CmdWrite(0x79);
	RA8876_DataWrite(HY);

	RA8876_CmdWrite(0x7A);
	RA8876_DataWrite(HY>>8);
}

//よ锣à畖
void Circle_Square_Radius_RxRy (uint16_t WX,uint16_t HY)
{
/*
[77h] Draw Circle/Ellipse/Circle Square Major radius [7:0]
[78h] Draw Circle/Ellipse/Circle Square Major radius [12:8]
[79h] Draw Circle/Ellipse/Circle Square Minor radius [7:0]
[7Ah] Draw Circle/Ellipse/Circle Square Minor radius [12:8]
*/
	RA8876_CmdWrite(0x77);
	RA8876_DataWrite(WX);

	RA8876_CmdWrite(0x78);
	RA8876_DataWrite(WX>>8);

	RA8876_CmdWrite(0x79);
	RA8876_DataWrite(HY);

	RA8876_CmdWrite(0x7A);
	RA8876_DataWrite(HY>>8);
}

//蛾いみ
void Circle_Center_XY(uint16_t WX,uint16_t HY)
{
/*
[7Bh] Draw Circle/Ellipse/Circle Square Center X-coordination [7:0]
[7Ch] Draw Circle/Ellipse/Circle Square Center X-coordination [12:8]
[7Dh] Draw Circle/Ellipse/Circle Square Center Y-coordination [7:0]
[7Eh] Draw Circle/Ellipse/Circle Square Center Y-coordination [12:8]
*/
	RA8876_CmdWrite(0x7B);
	RA8876_DataWrite(WX);

	RA8876_CmdWrite(0x7C);
	RA8876_DataWrite(WX>>8);

	RA8876_CmdWrite(0x7D);
	RA8876_DataWrite(HY);

	RA8876_CmdWrite(0x7E);
	RA8876_DataWrite(HY>>8);
}
//掘蛾いみ
void Ellipse_Center_XY(uint16_t WX,uint16_t HY)
{
/*
[7Bh] Draw Circle/Ellipse/Circle Square Center X-coordination [7:0]
[7Ch] Draw Circle/Ellipse/Circle Square Center X-coordination [12:8]
[7Dh] Draw Circle/Ellipse/Circle Square Center Y-coordination [7:0]
[7Eh] Draw Circle/Ellipse/Circle Square Center Y-coordination [12:8]
*/
	RA8876_CmdWrite(0x7B);
	RA8876_DataWrite(WX);

	RA8876_CmdWrite(0x7C);
	RA8876_DataWrite(WX>>8);

	RA8876_CmdWrite(0x7D);
	RA8876_DataWrite(HY);

	RA8876_CmdWrite(0x7E);
	RA8876_DataWrite(HY>>8);
}





/*
*********************************************************************************************************
*	函 数 名: Set_PWM_Prescaler_1_to_256
*	功能说明: 设置PWM预分频值
*	形    参：无
*	返 回 值: 错误代码(无需处理)
*********************************************************************************************************
*/
void Set_PWM_Prescaler_1_to_256(uint16_t WX)
{
/*
PWM Prescaler Register
These 8 bits determine prescaler value for Timer 0 and 1.
Time base is ¨Core_Freq / (Prescaler + 1)〃
*/
	RA8876_CmdWrite(0x84);
	RA8875_Delaly1us();		/* 延迟1us */
	RA8876_DataWrite(WX-1);
	RA8875_Delaly1us();		/* 延迟1us */
}
//[85h]=========================================================================
void Select_PWM1_Clock_Divided_By_1(void)
{
/*
Select MUX input for PWM Timer 1.
00 = 1; 01 = 1/2; 10 = 1/4 ; 11 = 1/8;
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x85);
	temp = RA8876_DataRead();
	temp &= cClrb7;
	temp &= cClrb6;
	RA8876_DataWrite(temp);
}
void Select_PWM1_Clock_Divided_By_2(void)
{
/*
Select MUX input for PWM Timer 1.
00 = 1; 01 = 1/2; 10 = 1/4 ; 11 = 1/8;
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x85);
	temp = RA8876_DataRead();
	temp &= cClrb7;
	temp |= cSetb6;
	RA8876_DataWrite(temp);
}
void Select_PWM1_Clock_Divided_By_4(void)
{
/*
Select MUX input for PWM Timer 1.
00 = 1; 01 = 1/2; 10 = 1/4 ; 11 = 1/8;
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x85);
	temp = RA8876_DataRead();
	temp |= cSetb7;
	temp &= cClrb6;
	RA8876_DataWrite(temp);
}
void Select_PWM1_Clock_Divided_By_8(void)
{
/*
Select MUX input for PWM Timer 1.
00 = 1; 01 = 1/2; 10 = 1/4 ; 11 = 1/8;
*/
	uint8_t temp = 0;
	
	RA8876_CmdWrite(0x85);
	RA8875_Delaly1us();		/* 延迟1us */
	temp = RA8876_DataRead();
	RA8875_Delaly1us();		/* 延迟1us */
	temp |= cSetb7;
	temp |= cSetb6;
	RA8876_DataWrite(temp);
	RA8875_Delaly1us();		/* 延迟1us */
	
#if 0
	//RA8876_CmdWrite(0x85);
	RA8875_Delaly1us();		/* 延迟1us */	
	temp = RA8876_DataRead();
	RA8875_Delaly1us();		/* 延迟1us */
#endif
}
void Select_PWM0_Clock_Divided_By_1(void)
{
/*
Select MUX input for PWM Timer 0.
00 = 1; 01 = 1/2; 10 = 1/4 ; 11 = 1/8;
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x85);
	temp = RA8876_DataRead();
	temp &= cClrb5;
	temp &= cClrb4;
	RA8876_DataWrite(temp);
}
void Select_PWM0_Clock_Divided_By_2(void)
{
/*
Select MUX input for PWM Timer 0.
00 = 1; 01 = 1/2; 10 = 1/4 ; 11 = 1/8;
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x85);
	temp = RA8876_DataRead();
	temp &= cClrb5;
	temp |= cSetb4;
	RA8876_DataWrite(temp);
}
void Select_PWM0_Clock_Divided_By_4(void)
{
/*
Select MUX input for PWM Timer 0.
00 = 1; 01 = 1/2; 10 = 1/4 ; 11 = 1/8;
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x85);
	temp = RA8876_DataRead();
	temp |= cSetb5;
	temp &= cClrb4;
	RA8876_DataWrite(temp);
}
void Select_PWM0_Clock_Divided_By_8(void)
{
/*
Select MUX input for PWM Timer 0.
00 = 1; 01 = 1/2; 10 = 1/4 ; 11 = 1/8;
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x85);
	temp = RA8876_DataRead();
	temp |= cSetb5;
	temp |= cSetb4;
	RA8876_DataWrite(temp);
}
//[85h].[bit3][bit2]=========================================================================
/*
XPWM[1] pin function control
0X:	XPWM[1] output system error flag (REG[00h] bit[1:0], Scan bandwidth insufficient + Memory access out of range)
10:	XPWM[1] enabled and controlled by PWM timer 1
11:	XPWM[1] output oscillator clock
//If XTEST[0] set high, then XPWM[1] will become panel scan clock input.
*/
void Select_PWM1_is_ErrorFlag(void)
{
	uint8_t temp;
	
	RA8876_CmdWrite(0x85);
	temp = RA8876_DataRead();
	temp &= cClrb3;
	RA8876_DataWrite(temp);
}

/*
*********************************************************************************************************
*	函 数 名: Select_PWM1
*	功能说明: 输出PWM计数器1
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void Select_PWM1(void)
{
	uint8_t temp;
	
	RA8876_CmdWrite(0x85);
	RA8875_Delaly1us();		/* 延迟1us */
	temp = RA8876_DataRead();
	RA8875_Delaly1us();		/* 延迟1us */
	temp |= cSetb3;
	temp &= cClrb2;
	RA8876_DataWrite(temp);
	RA8875_Delaly1us();		/* 延迟1us */
}
void Select_PWM1_is_Osc_Clock(void)
{
	uint8_t temp;
	
	RA8876_CmdWrite(0x85);
	temp = RA8876_DataRead();
	temp |= cSetb3;
	temp |= cSetb2;
	RA8876_DataWrite(temp);
}
//[85h].[bit1][bit0]=========================================================================
/*
XPWM[0] pin function control
0X: XPWM[0] becomes GPIO-C[7]
10: XPWM[0] enabled and controlled by PWM timer 0
11: XPWM[0] output core clock
*/
void Select_PWM0_is_GPIO_C7(void)
{
	uint8_t temp;
	
	RA8876_CmdWrite(0x85);
	temp = RA8876_DataRead();
	temp &= cClrb1;
	RA8876_DataWrite(temp);
}
/*
*********************************************************************************************************
*	函 数 名: Select_PWM0
*	功能说明: 输出PWM计数器0
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void Select_PWM0(void)
{
	uint8_t temp;
	
	RA8876_CmdWrite(0x85);
	temp = RA8876_DataRead();
	temp |= cSetb1;
	temp &= cClrb0;
	RA8876_DataWrite(temp);
}
void Select_PWM0_is_Core_Clock(void)
{
	uint8_t temp;
	
	RA8876_CmdWrite(0x85);
	temp = RA8876_DataRead();
	temp |= cSetb1;
	temp |= cSetb0;
	RA8876_DataWrite(temp);
}
//[86h]=========================================================================
//[86h]PWM1
void Enable_PWM1_Inverter(void)
{
/*
PWM Timer 1 output inverter on/off.
Determine the output inverter on/off for Timer 1. 
0 = Inverter off 
1 = Inverter on for PWM1
*/
	uint8_t temp;
	RA8876_CmdWrite(0x86);
	temp = RA8876_DataRead();
	temp |= cSetb6;
	RA8876_DataWrite(temp);
}
void Disable_PWM1_Inverter(void)
{
/*
PWM Timer 1 output inverter on/off.
Determine the output inverter on/off for Timer 1. 
0 = Inverter off 
1 = Inverter on for PWM1
*/
	uint8_t temp;
	RA8876_CmdWrite(0x86);
	temp = RA8876_DataRead();
	temp &= cClrb6;
	RA8876_DataWrite(temp);
}
void Auto_Reload_PWM1(void)
{
/*
PWM Timer 1 auto reload on/off
Determine auto reload on/off for Timer 1. 
0 = One-shot 
1 = Interval mode(auto reload)
*/
	uint8_t temp;
	RA8876_CmdWrite(0x86);
	temp = RA8876_DataRead();
	temp |= cSetb5;
	RA8876_DataWrite(temp);
}
void One_Shot_PWM1(void)
{
/*
PWM Timer 1 auto reload on/off
Determine auto reload on/off for Timer 1. 
0 = One-shot 
1 = Interval mode(auto reload)
*/
	uint8_t temp;
	RA8876_CmdWrite(0x86);
	temp = RA8876_DataRead();
	temp &= cClrb5;
	RA8876_DataWrite(temp);
}
void Start_PWM1(void)
{
/*
PWM Timer 1 start/stop
Determine start/stop for Timer 1. 
0 = Stop 
1 = Start for Timer 1
*/
	uint8_t temp;
	RA8876_CmdWrite(0x86);
	temp = RA8876_DataRead();
	temp |= cSetb4;
	RA8876_DataWrite(temp);
}
void Stop_PWM1(void)
{
/*
PWM Timer 1 start/stop
Determine start/stop for Timer 1. 
0 = Stop 
1 = Start for Timer 1
*/
	uint8_t temp;
	RA8876_CmdWrite(0x86);
	temp = RA8876_DataRead();
	temp &= cClrb4;
	RA8876_DataWrite(temp);
}
//[86h]PWM0
void Enable_PWM0_Dead_Zone(void)
{
/*
PWM Timer 0 Dead zone enable
Determine the dead zone operation. 0 = Disable. 1 = Enable.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x86);
	temp = RA8876_DataRead();
	temp |= cSetb3;
	RA8876_DataWrite(temp);
}
void Disable_PWM0_Dead_Zone(void)
{
/*
PWM Timer 0 Dead zone enable
Determine the dead zone operation. 0 = Disable. 1 = Enable.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x86);
	temp = RA8876_DataRead();
	temp &= cClrb3;
	RA8876_DataWrite(temp);
}
void Enable_PWM0_Inverter(void)
{
/*
PWM Timer 0 output inverter on/off
Determine the output inverter on/off for Timer 0. 
0 = Inverter off 
1 = Inverter on for PWM0
*/
	uint8_t temp;
	RA8876_CmdWrite(0x86);
	temp = RA8876_DataRead();
	temp |= cSetb2;
	RA8876_DataWrite(temp);
}
void Disable_PWM0_Inverter(void)
{
/*
PWM Timer 0 output inverter on/off
Determine the output inverter on/off for Timer 0. 
0 = Inverter off 
1 = Inverter on for PWM0
*/
	uint8_t temp;
	RA8876_CmdWrite(0x86);
	temp = RA8876_DataRead();
	temp &= cClrb2;
	RA8876_DataWrite(temp);
}
void Auto_Reload_PWM0(void)
{
/*
PWM Timer 0 auto reload on/off
Determine auto reload on/off for Timer 0. 
0 = One-shot 
1 = Interval mode(auto reload)
*/
	uint8_t temp;
	RA8876_CmdWrite(0x86);
	temp = RA8876_DataRead();
	temp |= cSetb1;
	RA8876_DataWrite(temp);
}
void One_Shot_PWM0(void)
{
/*
PWM Timer 1 auto reload on/off
Determine auto reload on/off for Timer 1. 
0 = One-shot 
1 = Interval mode(auto reload)
*/
	uint8_t temp;
	RA8876_CmdWrite(0x86);
	temp = RA8876_DataRead();
	temp &= cClrb1;
	RA8876_DataWrite(temp);
}
/*
*********************************************************************************************************
*	函 数 名: Start_PWM0
*	功能说明: 启动PWM0
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void Start_PWM0(void)
{
/*
PWM Timer 0 start/stop
Determine start/stop for Timer 0. 
0 = Stop 
1 = Start for Timer 0
*/
	uint8_t temp;
	RA8876_CmdWrite(0x86);
	temp = RA8876_DataRead();
	temp |= cSetb0;
	RA8876_DataWrite(temp);
}
/*
*********************************************************************************************************
*	函 数 名: Stop_PWM0
*	功能说明: 停止PWM0
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void Stop_PWM0(void)
{
/*
PWM Timer 0 start/stop
Determine start/stop for Timer 0. 
0 = Stop 
1 = Start for Timer 0
*/
	uint8_t temp;
	RA8876_CmdWrite(0x86);
	temp = RA8876_DataRead();
	temp &= cClrb0;
	RA8876_DataWrite(temp);
}
//[87h]=========================================================================
void Set_Timer0_Dead_Zone_Length(uint8_t temp)
{
/*
Timer 0 Dead zone length register
These 8 bits determine the dead zone length. The 1 unit time of
the dead zone length is equal to that of timer 0.
*/
	RA8876_CmdWrite(0x87);
	RA8876_DataWrite(temp);
}
/*
*********************************************************************************************************
*	函 数 名: Set_Timer0_Compare_Buffer
*	功能说明: 设置定时器0比较缓冲区值
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void Set_Timer0_Compare_Buffer(uint16_t WX)
{
/*
Timer 0 compare buffer register
Compare buffer register total has 16 bits.
When timer counter equal or less than compare buffer register will cause PWM out
high level if inv_on bit is off.
*/
	RA8876_CmdWrite(0x88);
	RA8876_DataWrite(WX);
	RA8876_CmdWrite(0x89);
	RA8876_DataWrite(WX>>8);
}
/*
*********************************************************************************************************
*	函 数 名: Set_Timer0_Count_Buffer
*	功能说明: 设置定时器0计数缓冲区
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void Set_Timer0_Count_Buffer(uint16_t WX)
{
/*
Timer 0 count buffer register
Count buffer register total has 16 bits.
When timer counter equal to 0 will cause PWM timer reload Count buffer register if reload_en bit set as enable.
It may read back timer counterˇs real time value when PWM timer start.
*/
	RA8876_CmdWrite(0x8A);				/* 低8位 */
	RA8876_DataWrite(WX);
	RA8876_CmdWrite(0x8B);				/* 高8位 */
	RA8876_DataWrite(WX>>8);			
}
//[8Ch][8Dh]=========================================================================
void Set_Timer1_Compare_Buffer(uint16_t WX)
{
/*
Timer 0 compare buffer register
Compare buffer register total has 16 bits.
When timer counter equal or less than compare buffer register will cause PWM out
high level if inv_on bit is off.
*/
	RA8876_CmdWrite(0x8C);
	RA8875_Delaly1us();		/* 延迟1us */
	RA8876_DataWrite(WX);
	RA8875_Delaly1us();		/* 延迟1us */
	RA8876_CmdWrite(0x8D);
	RA8875_Delaly1us();		/* 延迟1us */
	RA8876_DataWrite(WX>>8);
	RA8875_Delaly1us();		/* 延迟1us */
}
//[8Eh][8Fh]=========================================================================
void Set_Timer1_Count_Buffer(uint16_t WX)
{
/*
Timer 0 count buffer register
Count buffer register total has 16 bits.
When timer counter equal to 0 will cause PWM timer reload Count buffer register if reload_en bit set as enable.
It may read back timer counterˇs real time value when PWM timer start.
*/
	RA8876_CmdWrite(0x8E);
	RA8875_Delaly1us();		/* 延迟1us */
	RA8876_DataWrite(WX);
	RA8875_Delaly1us();		/* 延迟1us */
	RA8876_CmdWrite(0x8F);
	RA8875_Delaly1us();		/* 延迟1us */
	RA8876_DataWrite(WX>>8);
	RA8875_Delaly1us();		/* 延迟1us */	
}


//[90h]~[B5h]=========================================================================

//[90h]=========================================================================
void BTE_Enable(void)
{	
/*
BTE Function Enable
0 : BTE Function disable.
1 : BTE Function enable.
*/
    uint8_t temp;
    RA8876_CmdWrite(0x90);
    temp = RA8876_DataRead();
    temp |= cSetb4 ;
	RA8876_DataWrite(temp);  
}

//[90h]=========================================================================
void BTE_Disable(void)
{	
/*
BTE Function Enable
0 : BTE Function disable.
1 : BTE Function enable.
*/
    uint8_t temp;
    RA8876_CmdWrite(0x90);
    temp = RA8876_DataRead();
    temp &= cClrb4 ;
	RA8876_DataWrite(temp);  
}

//[90h]=========================================================================
void Check_BTE_Busy(void)
{	
/*
case1: Status Register_bit3
0 : BTE function is idle.
1 : BTE function is busy.

case2: [90h]BTE Function Control Register_bit4
Read
0 : BTE function is idle.
1 : BTE function is busy.
*/
	uint8_t temp;
	
	//case1: using status register 	
	do
	{
		temp=RA8876_StatusRead();
	}while(temp&0x08);


	//case2: using BTE Function Control Register
	do
	{
	    RA8876_CmdWrite(0x90);
	    temp = RA8876_DataRead();
	}while(temp&0x10);

}
//[90h]=========================================================================
void Pattern_Format_8X8(void)
{	
/*
Pattern Format
0 : 8X8
1 : 16X16
*/
    uint8_t temp;
    RA8876_CmdWrite(0x90);
    temp = RA8876_DataRead();
    temp &= cClrb0 ;
	RA8876_DataWrite(temp);
}	
//[90h]=========================================================================
void Pattern_Format_16X16(void)
{	
/*
Pattern Format
0 : 8X8
1 : 16X16
*/
    uint8_t temp;
    RA8876_CmdWrite(0x90);
    temp = RA8876_DataRead();
    temp |= cSetb0 ;
	  RA8876_DataWrite(temp);
}	

//[91h]=========================================================================
void BTE_ROP_Code(uint8_t setx)
{	
/*
BTE ROP Code[Bit7:4]
	
0000 : 0(Blackness)
0001 : ~S0.~S1 or ~ ( S0+S1 )
0010 : ~S0.S1
0011 : ~S0
0100 : S0.~S1
0101 : ~S1
0110 : S0^S1
0111 : ~S0+~S1 or ~ ( S0.S1 )
1000 : S0.S1
1001 : ~ ( S0^S1 )
1010 : S1
1011 : ~S0+S1
1100 : S0
1101 : S0+~S1
1110 : S0+S1
1111 : 1 ( Whiteness )
*/
    uint8_t temp;
    RA8876_CmdWrite(0x91);
    temp = RA8876_DataRead();
    temp &= 0x0f ;
		temp |= (setx<<4);
	  RA8876_DataWrite(temp);
}
	
//[91h]=========================================================================
void BTE_Operation_Code(uint8_t setx)
{	
/*
BTE Operation Code[Bit3:0]
	
0000/ 0 : MPU Write BTE with ROP.
0001    : NA
0010/ 2 : Memory copy (move) BTE in positive direction with ROP.
0011    : NA
0100/ 4 : MPU Write with chroma keying (w/o ROP)
0101/ 5 : Memory copy (move) with chroma keying (w/o ROP)
0110/ 6 : Pattern Fill with ROP
0111/ 7 : Pattern Fill with chroma keying (w/o ROP)
1000/ 8 : MPU Write with Color Expansion (w/o ROP)
1001/ 9 : MPU Write with Color Expansion and chroma keying (w/o ROP)
1010/10 : Memory write with opacity (w/o ROP)
1011/11 : MPU Write with opacity (w/o ROP)
1100/12 : Solid Fill (w/o ROP)
1101    : NA
1110/14 : Memory write with Color Expansion (w/o ROP)
1111/15 : Memory write with Color Expansion and chroma keying (w/o ROP)
*/
    uint8_t temp;
    RA8876_CmdWrite(0x91);
    temp = RA8876_DataRead();
	temp &= 0xf0 ;
	temp |= setx ;
	RA8876_DataWrite(temp);

}
//[92h]=========================================================================
void BTE_S0_Color_8bpp(void)
{	
/*
S0 Color Depth
00 : 256 Color
01 : 64k Color
1x : 16M Color
*/	
    uint8_t temp;
    RA8876_CmdWrite(0x92);
    temp = RA8876_DataRead();
    temp &= cClrb6 ;
	temp &= cClrb5 ;
	RA8876_DataWrite(temp);
}	
/*
*********************************************************************************************************
*	函 数 名: BTE_S0_Color_16bpp
*	功能说明: 92H相关
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void BTE_S0_Color_16bpp(void)
{	
/*
	Source 0/1 & Destination Color Depth[92H]
bit6-5：
	S0 Color Depth
	00: 256 色 (8bpp)。
	01: 64k 色 (16bpp)。
	1x: 16M 色 (24bpp)。
*/	
    uint8_t temp;
    RA8876_CmdWrite(0x92);
    temp = RA8876_DataRead();
    temp &= cClrb6 ;
	temp |= cSetb5 ;
	RA8876_DataWrite(temp);

}	
//[92h]=========================================================================
void BTE_S0_Color_24bpp(void)
{	
/*
S0 Color Depth
00 : 256 Color
01 : 64k Color
1x : 16M Color
*/	
    uint8_t temp;
    RA8876_CmdWrite(0x92);
    temp = RA8876_DataRead();
	temp |= cSetb6 ;
	//temp |= cSetb5 ;
	RA8876_DataWrite(temp);
}
//[92h]=========================================================================
void BTE_S1_Color_8bpp(void)
{	
/*
S1 Color Depth
000 : 256 Color
001 : 64k Color
010 : 16M Color
011 : Constant Color
100 : 8 bit pixel alpha blending
101 : 16 bit pixel alpha blending
*/	
    uint8_t temp;
    RA8876_CmdWrite(0x92);
    temp = RA8876_DataRead();
    temp &= cClrb4 ;
		temp &= cClrb3 ;
	  temp &= cClrb2 ;
	  RA8876_DataWrite(temp);
}	
/*
*********************************************************************************************************
*	函 数 名: BTE_S1_Color_16bpp
*	功能说明: 92H相关
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void BTE_S1_Color_16bpp(void)
{	
/*
	Source 0/1 & Destination Color Depth[92H]
bit4-2：
	S1 Color Depth
	000: 256 色 (8bpp)。
	001: 64k 色 (16bpp)。
	010: 16M 色 (24bpp)。
	011: Constant color (S1 memory start address’ setting definition
	change as S1 constant color definition)。
	100: 8 bit pixel alpha blending。
	101: 16 bit pixel alpha blending。
*/	
    uint8_t temp;
    RA8876_CmdWrite(0x92);
    temp = RA8876_DataRead();
    temp &= cClrb4 ;
		temp &= cClrb3 ;
	  temp |= cSetb2 ;
	  RA8876_DataWrite(temp);

}
//[92h]=========================================================================
void BTE_S1_Color_24bpp(void)
{	
/*
S1 Color Depth
000 : 256 Color
001 : 64k Color
010 : 16M Color
011 : Constant Color
100 : 8 bit pixel alpha blending
101 : 16 bit pixel alpha blending
*/	
    uint8_t temp;
    RA8876_CmdWrite(0x92);
    temp = RA8876_DataRead();
	temp &= cClrb4 ;
	temp |= cSetb3 ;
	temp &= cClrb2 ;
	RA8876_DataWrite(temp);
}

//[92h]=========================================================================
void BTE_S1_Color_Constant(void)
{	
/*
S1 Color Depth
000 : 256 Color
001 : 64k Color
010 : 16M Color
011 : Constant Color
100 : 8 bit pixel alpha blending
101 : 16 bit pixel alpha blending
*/	
    uint8_t temp;
    RA8876_CmdWrite(0x92);
    temp = RA8876_DataRead();
    temp &= cClrb4 ;
		temp |= cSetb3 ;
	  temp |= cSetb2 ;
	  RA8876_DataWrite(temp);
}



//[92h]=========================================================================
void BTE_S1_Color_8bit_Alpha(void)
{	
/*
S1 Color Depth
000 : 256 Color
001 : 64k Color
010 : 16M Color
011 : Constant Color
100 : 8 bit pixel alpha blending
101 : 16 bit pixel alpha blending
*/	
    uint8_t temp;
    RA8876_CmdWrite(0x92);
    temp = RA8876_DataRead();
    temp |= cSetb4 ;
		temp &= cClrb3 ;
	  temp &= cClrb2 ;
	  RA8876_DataWrite(temp);
}

//[92h]=========================================================================
void BTE_S1_Color_16bit_Alpha(void)
{	
/*
S1 Color Depth
000 : 256 Color
001 : 64k Color
010 : 16M Color
011 : Constant Color
100 : 8 bit pixel alpha blending
101 : 16 bit pixel alpha blending
*/	
    uint8_t temp;
    RA8876_CmdWrite(0x92);
    temp = RA8876_DataRead();
    temp |= cSetb4 ;
		temp &= cClrb3 ;
	  temp |= cSetb2 ;
	  RA8876_DataWrite(temp);
}

//[92h]=========================================================================
void BTE_Destination_Color_8bpp(void)
{	
/*
	00: 256 色 (8bpp)。
	01: 64k 色 (16bpp)。
	1x: 16M 色 (24bpp)。
*/
    uint8_t temp;
    RA8876_CmdWrite(0x92);
    temp = RA8876_DataRead();
    temp &= cClrb1 ;
		temp &= cClrb0 ;
	  RA8876_DataWrite(temp);
}	
/*
*********************************************************************************************************
*	函 数 名: BTE_Destination_Color_16bpp
*	功能说明: 92H相关
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void BTE_Destination_Color_16bpp(void)
{	
/*
	Source 0/1 & Destination Color Depth[92H]
bit1-0：
	Destination Color Depth
	00: 256 色 (8bpp)。
	01: 64k 色 (16bpp)。
	1x: 16M 色 (24bpp)。
*/	
    uint8_t temp;
    RA8876_CmdWrite(0x92);
    temp = RA8876_DataRead();
    temp &= cClrb1 ;
		temp |= cSetb0 ;
	  RA8876_DataWrite(temp);

}	
//[92h]=========================================================================
void BTE_Destination_Color_24bpp(void)
{	
/*
Destination Color Depth
00 : 256 Color
10 : 64k Color
1x : 16M Color
*/	
    uint8_t temp;
    RA8876_CmdWrite(0x92);
    temp = RA8876_DataRead();
    temp |= cSetb1 ;
		//temp |= cSetb0 ;
	  RA8876_DataWrite(temp);
}


//[93h][94h][95h][96h]=========================================================================
void BTE_S0_Memory_Start_Address(unsigned long Addr)	
{
/*
[93h] BTE S0 Memory Start Address [7:0]
[94h] BTE S0 Memory Start Address [15:8]
[95h] BTE S0 Memory Start Address [23:16]
[96h] BTE S0 Memory Start Address [31:24]
Bit [1:0] tie to ¨0〃 internally.
*/
	LCD_RegisterWrite(0x93,Addr);
	LCD_RegisterWrite(0x94,Addr>>8);
	LCD_RegisterWrite(0x95,Addr>>16);
	LCD_RegisterWrite(0x96,Addr>>24);
}


//[97h][98h]=========================================================================
void BTE_S0_Image_Width(uint16_t WX)	
{
/*
[97h] BTE S0 Image Width [7:0]
[98h] BTE S0 Image Width [12:8]
Unit: Pixel.
Bit [1:0] tie to ¨0〃 internally.
*/
	LCD_RegisterWrite(0x97,WX);
	LCD_RegisterWrite(0x98,WX>>8);
}


//[99h][9Ah][9Bh][9Ch]=========================================================================
void BTE_S0_Window_Start_XY(uint16_t WX,uint16_t HY)	
{
/*
[99h] BTE S0 Window Upper-Left corner X-coordination [7:0]
[9Ah] BTE S0 Window Upper-Left corner X-coordination [12:8]
[9Bh] BTE S0 Window Upper-Left corner Y-coordination [7:0]
[9Ch] BTE S0 Window Upper-Left corner Y-coordination [12:8]
*/

	LCD_RegisterWrite(0x99,WX);
	LCD_RegisterWrite(0x9A,WX>>8);

	LCD_RegisterWrite(0x9B,HY);
	LCD_RegisterWrite(0x9C,HY>>8);
}




//[9Dh][9Eh][9Fh][A0h]=========================================================================
void BTE_S1_Memory_Start_Address(unsigned long Addr)	
{
/*
[9Dh] BTE S1 Memory Start Address [7:0]
[9Eh] BTE S1 Memory Start Address [15:8]
[9Fh] BTE S1 Memory Start Address [23:16]
[A0h] BTE S1 Memory Start Address [31:24]
Bit [1:0] tie to ¨0〃 internally.
*/
	LCD_RegisterWrite(0x9D,Addr);
	LCD_RegisterWrite(0x9E,Addr>>8);
	LCD_RegisterWrite(0x9F,Addr>>16);
	LCD_RegisterWrite(0xA0,Addr>>24);
}


//Input data format:R3G3B2
void S1_Constant_color_256(uint8_t temp)
{
    RA8876_CmdWrite(0x9D);
    RA8876_DataWrite(temp);

    RA8876_CmdWrite(0x9E);
    RA8876_DataWrite(temp<<3);

    RA8876_CmdWrite(0x9F);
    RA8876_DataWrite(temp<<6);
}

//Input data format:R5G6B6
void S1_Constant_color_65k(uint16_t temp)
{
    RA8876_CmdWrite(0x9D);
    RA8876_DataWrite(temp>>8);

    RA8876_CmdWrite(0x9E);
    RA8876_DataWrite(temp>>3);

    RA8876_CmdWrite(0x9F);
    RA8876_DataWrite(temp<<3);
}

//Input data format:R8G8B8
void S1_Constant_color_16M(unsigned long temp)
{
    RA8876_CmdWrite(0x9D);
    RA8876_DataWrite(temp>>16);

    RA8876_CmdWrite(0x9E);
    RA8876_DataWrite(temp>>8);

    RA8876_CmdWrite(0x9F);
    RA8876_DataWrite(temp);
}




//[A1h][A2h]=========================================================================
void BTE_S1_Image_Width(uint16_t WX)	
{
/*
[A1h] BTE S1 Image Width [7:0]
[A2h] BTE S1 Image Width [12:8]
Unit: Pixel.
Bit [1:0] tie to ¨0〃 internally.
*/
	LCD_RegisterWrite(0xA1,WX);
	LCD_RegisterWrite(0xA2,WX>>8);
}


//[A3h][A4h][A5h][A6h]=========================================================================
void BTE_S1_Window_Start_XY(uint16_t WX,uint16_t HY)	
{
/*
[A3h] BTE S1 Window Upper-Left corner X-coordination [7:0]
[A4h] BTE S1 Window Upper-Left corner X-coordination [12:8]
[A5h] BTE S1 Window Upper-Left corner Y-coordination [7:0]
[A6h] BTE S1 Window Upper-Left corner Y-coordination [12:8]
*/

	LCD_RegisterWrite(0xA3,WX);
	LCD_RegisterWrite(0xA4,WX>>8);

	LCD_RegisterWrite(0xA5,HY);
	LCD_RegisterWrite(0xA6,HY>>8);
}




//[A7h][A8h][A9h][AAh]=========================================================================
void BTE_Destination_Memory_Start_Address(unsigned long Addr)	
{
/*
[A7h] BTE Destination Memory Start Address [7:0]
[A8h] BTE Destination Memory Start Address [15:8]
[A9h] BTE Destination Memory Start Address [23:16]
[AAh] BTE Destination Memory Start Address [31:24]
Bit [1:0] tie to ¨0〃 internally.
*/
	LCD_RegisterWrite(0xA7,Addr);
	LCD_RegisterWrite(0xA8,Addr>>8);
	LCD_RegisterWrite(0xA9,Addr>>16);
	LCD_RegisterWrite(0xAA,Addr>>24);
}


//[ABh][ACh]=========================================================================
void BTE_Destination_Image_Width(uint16_t WX)	
{
/*
[ABh] BTE Destination Image Width [7:0]
[ACh] BTE Destination Image Width [12:8]
Unit: Pixel.
Bit [1:0] tie to ¨0〃 internally.
*/
	LCD_RegisterWrite(0xAB,WX);
	LCD_RegisterWrite(0xAC,WX>>8);
}


//[ADh][AEh][AFh][B0h]=========================================================================
void BTE_Destination_Window_Start_XY(uint16_t WX,uint16_t HY)	
{
/*
[ADh] BTE Destination Window Upper-Left corner X-coordination [7:0]
[AEh] BTE Destination Window Upper-Left corner X-coordination [12:8]
[AFh] BTE Destination Window Upper-Left corner Y-coordination [7:0]
[B0h] BTE Destination Window Upper-Left corner Y-coordination [12:8]
*/

	LCD_RegisterWrite(0xAD,WX);
	LCD_RegisterWrite(0xAE,WX>>8);

	LCD_RegisterWrite(0xAF,HY);
	LCD_RegisterWrite(0xB0,HY>>8);
}


//[B1h][B2h][B3h][B4h]===============================================================

void BTE_Window_Size(uint16_t WX, uint16_t HY)

{
/*
[B1h] BTE Window Width [7:0]
[B2h] BTE Window Width [12:8]

[B3h] BTE Window Height [7:0]
[B4h] BTE Window Height [12:8]
*/
        LCD_RegisterWrite(0xB1,WX);
        LCD_RegisterWrite(0xB2,WX>>8);
	
	    LCD_RegisterWrite(0xB3,HY);
        LCD_RegisterWrite(0xB4,HY>>8);
}

//[B5h]=========================================================================
void BTE_Alpha_Blending_Effect(uint8_t temp)
{	
/*
Window Alpha Blending effect for S0 & S1
The value of alpha in the color code ranges from 0.0 to 1.0,
where 0.0 represents a fully transparent color, and 1.0
represents a fully opaque color.
00h: 0
01h: 1/32
02h: 2/32
:
1Eh: 30/32
1Fh: 31/32
2Xh: 1
Output Effect = (S0 image x (1 - alpha setting value)) + (S1 image x alpha setting value)
*/
    RA8876_CmdWrite(0xB5);
	RA8876_DataWrite(temp);  
}


//[B6h]=========================================================================
void Start_SFI_DMA(void)
{
	uint8_t temp;
	RA8876_CmdWrite(0xB6);
	temp = RA8876_DataRead();
    temp |= cSetb0;
	RA8876_DataWrite(temp);
}

void Check_Busy_SFI_DMA(void)
{
	RA8876_CmdWrite(0xB6);
	do
	{		
	}while((RA8876_DataRead()&0x01)==0x01);
}


/*
*********************************************************************************************************
*	函 数 名: Select_SFI_0
*	功能说明: 串行闪存选择
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void Select_SFI_0(void)
{
/*[bit7]
Serial Flash/ROM I/F # Select
0: Serial Flash/ROM 0 I/F is selected.
1: Serial Flash/ROM 1 I/F is selected.
*/
/*
	串行Flash/ROM控制寄存器[B7H]
bit7：
	Serial Flash/ROM I/F # Select
	0: 串行闪存/ROM 0 被选择。
	1: 串行闪存/ROM 1 被选择。
*/
	uint8_t temp;
	RA8876_CmdWrite(0xB7);
	temp = RA8876_DataRead();
    temp &= cClrb7;
	RA8876_DataWrite(temp);
}
void Select_SFI_1(void)
{
/*[bit7]
Serial Flash/ROM I/F # Select
0: Serial Flash/ROM 0 I/F is selected.
1: Serial Flash/ROM 1 I/F is selected.
*/
	uint8_t temp;
	RA8876_CmdWrite(0xB7);
	temp = RA8876_DataRead();
    temp |= cSetb7;
	RA8876_DataWrite(temp);
}

/*
*********************************************************************************************************
*	函 数 名: Select_SFI_Font_Mode
*	功能说明: 选择串行flash / ROM 访问模式
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void Select_SFI_Font_Mode(void)
{
/*[bit6]
Serial Flash /ROM Access Mode
0: Font mode  for external cgrom
1: DMA mode  for cgram , pattern , bootstart image or osd
*/
/*
	串行Flash/ROM控制寄存器[B7H]
bit6:
	Serial Flash /ROM Access Mode
	0: 字符模式– 使用在 CGROM。
	1: DMA模式– 使用在 CGRAM、pattern、boot start image 或 OSD功能上。
*/
	uint8_t temp;
	RA8876_CmdWrite(0xB7);
	temp = RA8876_DataRead();
    temp &= cClrb6;
	RA8876_DataWrite(temp);
}
void Select_SFI_DMA_Mode(void)
{
/*[bit6]
Serial Flash /ROM Access Mode
0: Font mode  for external cgrom
1: DMA mode  for cgram , pattern , bootstart image or osd
*/
	uint8_t temp;
	RA8876_CmdWrite(0xB7);
	temp = RA8876_DataRead();
    temp |= cSetb6;
	RA8876_DataWrite(temp);
}

/*
*********************************************************************************************************
*	函 数 名: Select_SFI_24bit_Address
*	功能说明: 选择串行flash为24位寻址模式
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void Select_SFI_24bit_Address(void)
{
/*[bit5]
Serial Flash/ROM Address Mode
0: 24 bits address mode
1: 32 bits address mode
*/
/*
	串行Flash/ROM控制寄存器[B7H]
bit5:
	Serial Flash/ROM Address Mode
		0: 24 bits 寻址模式。
		1: 32 bits 寻址模式。
	如果使用者希望使用 32 bits 寻址模式，使用者必须自行输入 EX4B
	命令(B7h) 给串行闪存，并且设定此 bit 为 1。
	使用者也可以检查这个位来知道是否在开机显示中已经进入 32bit
	地址模式。
*/
	uint8_t temp;
	RA8876_CmdWrite(0xB7);
	temp = RA8876_DataRead();
    temp &= cClrb5;
	RA8876_DataWrite(temp);
}

void Select_SFI_32bit_Address(void)
{
/*[bit5]
Serial Flash/ROM Address Mode
0: 24 bits address mode
1: 32 bits address mode
*/
	uint8_t temp;
	RA8876_CmdWrite(0xB7);
	temp = RA8876_DataRead();
    temp |= cSetb5;
	RA8876_DataWrite(temp);
}

/*
*********************************************************************************************************
*	函 数 名: Select_standard_SPI_Mode0_or_Mode3
*	功能说明: 选择标准SPI模式0或模式3
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void Select_standard_SPI_Mode0_or_Mode3(void)
{
/*[bit4]
RA8875 compatible mode
0: standard SPI mode 0 or mode 3 timing
1: Follow RA8875 mode 0 & mode 3 timing
*/
/*
	串行Flash/ROM控制寄存器[B7H]
bit4:
	RA8876 compatible mode
		0: 标准 SPI 模式 0 或模式 3 时序图。
		1: 依照 RA8876 模式 0 与模式 3 timing。
	在RA8875兼容模式中，数据读取的位置是在频率的下降缘
	(high->low)，并且数据也是在频率下降缘变化 (high->low)。
	当闲置时，对于 Mode 0， SPI 频率停止在 low。
	当闲置时，对于 Mode 3， SPI 频率停止在 high。
*/
	uint8_t temp;
	RA8876_CmdWrite(0xB7);
	temp = RA8876_DataRead();
    temp &= cClrb4;
	RA8876_DataWrite(temp);
}

void Select_RA8875_SPI_Mode0_and_Mode3(void)
{
/*[bit4]
RA8875 compatible mode
0: standard SPI mode 0 or mode 3 timing
1: Follow RA8875 mode 0 & mode 3 timing
*/
	uint8_t temp;
	RA8876_CmdWrite(0xB7);
	temp = RA8876_DataRead();
    temp |= cSetb4;
	RA8876_DataWrite(temp);
}

void Select_SFI_Single_Mode_Dummy_0T_03h(void)
{
/*
000xb: 1x read command code = 03h. Without dummy cycle between address and data.
010xb: 1x read command code = 0Bh. 8 dummy cycles inserted between address and data.
1x0xb: 1x read command code = 1Bh. 16 dummy cycles inserted between address and data.
*/
	uint8_t temp;
	RA8876_CmdWrite(0xB7);
	temp = RA8876_DataRead();
    temp &= 0xF0;
	RA8876_DataWrite(temp);
}

/*
*********************************************************************************************************
*	函 数 名: Select_SFI_Single_Mode_Dummy_8T_0Bh
*	功能说明: 读命令代码或
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void Select_SFI_Single_Mode_Dummy_8T_0Bh(void)
{
/*
000xb: 1x read command code = 03h. Without dummy cycle between address and data.
010xb: 1x read command code = 0Bh. 8 dummy cycles inserted between address and data.
1x0xb: 1x read command code = 1Bh. 16 dummy cycles inserted between address and data.
*/
/*
	串行Flash/ROM控制寄存器[B7H]	
bit3-0：
	Read Command code & behavior selection
	000xb: 1x 读取命令 03h。读取速度为 Normal read 速度。数据是
	由 xmiso 输入。在地址与数据间不需要空周期。
	010xb: 1x 读取命令 0Bh。为 faster read 速度。数据是由 xmiso
	输入， RA8876 在地址与数据间会塞入 8 个空周期。
	1x0xb: 1x 读取命令 1Bh。为 fastest read 速度，数据是由 xmiso
	输入。 RA8876 在地址与数据间会塞入 16 个空周期
	xx10b: 2x读取命令 3Bh。在xmiso与xmosi具有交错数据输入，在
	地址与数据间会塞入 8 个空周期 (Dual mode 0，请参考 圖 16-7)。
	xx11b: 2x读取命令BBh。地址输出与数据输入透过xmiso与xmosi
	输入，并且皆为交错式输入。在地址与数据间会自动塞入 4 个空
	周期 (Dual mode 1，请参考 圖 16-8)。
	注:: 不是所有的 serial flash 都支持以上命令，请根据使用的 serial
	flash 来选择正确的读取命令
*/	
	
	uint8_t temp;
	RA8876_CmdWrite(0xB7);
	temp = RA8876_DataRead();
	temp &= 0xF0;
   temp |= cSetb2;
	RA8876_DataWrite(temp);
}

void Select_SFI_Single_Mode_Dummy_16T_1Bh(void)
{
/*
000xb: 1x read command code = 03h. Without dummy cycle between address and data.
010xb: 1x read command code = 0Bh. 8 dummy cycles inserted between address and data.
1x0xb: 1x read command code = 1Bh. 16 dummy cycles inserted between address and data.
*/
	uint8_t temp;
	RA8876_CmdWrite(0xB7);
	temp = RA8876_DataRead();
	temp &= 0xF0;
   temp |= cSetb3;
	RA8876_DataWrite(temp);
}
void Select_SFI_Dual_Mode_Dummy_8T_3Bh(void)
{
/*
xx10b: 2x read command code = 3Bh. 8 dummy cycles inserted between address and data phase. (mode 0)
xx11b: 2x read command code = BBh. 4 dummy cycles inserted between address and data phase. (mode 1)
*/
	uint8_t temp;
	RA8876_CmdWrite(0xB7);
	temp = RA8876_DataRead();
	temp &= 0xF0;
  temp |= 0x02;
	RA8876_DataWrite(temp);
}
void Select_SFI_Dual_Mode_Dummy_4T_BBh(void)
{
/*
xx10b: 2x read command code = 3Bh. 8 dummy cycles inserted between address and data phase. (mode 0)
xx11b: 2x read command code = BBh. 4 dummy cycles inserted between address and data phase. (mode 1)
*/
	uint8_t temp;
	RA8876_CmdWrite(0xB7);
	temp = RA8876_DataRead();
	temp &= 0xF0;
  temp |= 0x03;
	RA8876_DataWrite(temp);
}







//REG[B8h] SPI master Tx /Rx FIFO Data Register (SPIDR) 
uint8_t SPI_Master_FIFO_Data_Put(uint8_t Data)
{
    uint8_t temp;
	RA8876_CmdWrite(0xB8);
	RA8876_DataWrite(Data);
	while(Tx_FIFO_Empty_Flag()==0);	//磅︽掸
	temp = SPI_Master_FIFO_Data_Get();

	return temp;
}

uint8_t SPI_Master_FIFO_Data_Get(void)
{
   uint8_t temp;

	while(Rx_FIFO_Empty_Flag()==1);//ぃ琌┕磅︽
	RA8876_CmdWrite(0xB8);
	temp=RA8876_DataRead();
	//while(Rx_FIFO_full_flag()); //硈尿糶16掸戈惠璶
   return temp;
}

//REG[B9h] SPI master Control Register (SPIMCR2) 
//void Mask_SPI_Master_Interrupt_Flag(void)(B9hэ)
void Enable_SPI_Master_Interrupt(void)
{
/*
SPI Master Interrupt enable
0: Disable interrupt.
1: Enable interrupt.
*/
  uint8_t temp;
  RA8876_CmdWrite(0xB9);
  temp = RA8876_DataRead();
  temp |= cSetb6;
  RA8876_DataWrite(temp);
} 
void Disable_SPI_Master_Interrupt(void)
{
/*
SPI Master Interrupt enable
0: Disable interrupt.
1: Enable interrupt.
*/
  uint8_t temp;
  RA8876_CmdWrite(0xB9);
  temp = RA8876_DataRead();
  temp &= cClrb6;
  RA8876_DataWrite(temp);
}

void Select_nSS_drive_on_xnsfcs0(void)
{
  uint8_t temp;
  RA8876_CmdWrite(0xB9);
  temp = RA8876_DataRead();
  temp &= cClrb5;
  RA8876_DataWrite(temp);

}

void Select_nSS_drive_on_xnsfcs1(void)
{
  uint8_t temp;
  RA8876_CmdWrite(0xB9);
  temp = RA8876_DataRead();
  temp |= cSetb5;
  RA8876_DataWrite(temp);

}

//0: inactive (nSS port will goes high) 
void nSS_Inactive(void)
{
  uint8_t temp;
  RA8876_CmdWrite(0xB9);
  temp = RA8876_DataRead();
  temp &= cClrb4;
  RA8876_DataWrite(temp);
}
//1: active (nSS port will goes low) 
void nSS_Active(void)
{
  uint8_t temp;
  RA8876_CmdWrite(0xB9);
  temp = RA8876_DataRead();
  temp |= cSetb4;
  RA8876_DataWrite(temp);
}

//Mask interrupt for FIFO overflow error [OVFIRQEN]
//void OVFIRQEN_Enable(void)(B9hэ)
void Mask_FIFO_overflow_error_Interrupt(void)
{
/*
Mask interrupt for FIFO overflow error [OVFIRQEN]
0: unmask
1: mask
*/
  uint8_t temp;
  RA8876_CmdWrite(0xB9);
  temp = RA8876_DataRead();
  temp |= cSetb3;
  RA8876_DataWrite(temp);
}
void Unmask_FIFO_overflow_error_Interrupt(void)
{
/*
Mask interrupt for FIFO overflow error [OVFIRQEN]
0: unmask
1: mask
*/
  uint8_t temp;
  RA8876_CmdWrite(0xB9);
  temp = RA8876_DataRead();
  temp &= cClrb3;
  RA8876_DataWrite(temp);
}


//void EMTIRQEN_Enable(void)(B9hэ)
void Mask_EMTIRQEN_Interrupt(void)
{
/*
Mask interrupt for while Tx FIFO empty & SPI engine/FSM idle [EMTIRQEN]
0: unmask
1: mask
*/
  uint8_t temp;
  RA8876_CmdWrite(0xB9);
  temp = RA8876_DataRead();
  temp |= cSetb2;
  RA8876_DataWrite(temp);
}
void Unmask_EMTIRQEN_Interrupt(void)
{
/*
Mask interrupt for while Tx FIFO empty & SPI engine/FSM idle [EMTIRQEN]
0: unmask
1: mask
*/
  uint8_t temp;
  RA8876_CmdWrite(0xB9);
  temp = RA8876_DataRead();
  temp &= cClrb2;
  RA8876_DataWrite(temp);
}

/*
SPI operation mode
Only support mode 0 & mode 3, when enable serial flashˇs DMA
or access Getopˇs character serial ROM device.
mode / CPOL:Clock Polarity bit / CPHA:Clock Phase bit
	0	0	0
	1	0	1
	2	1	0
	3	1	1
*/
/*
*********************************************************************************************************
*	函 数 名: Reset_CPOL
*	功能说明: 将CPOL时钟极性位置0
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
/*
	SPI主控制寄存器[B9H]
bit1-0：
	SPI operation mode
	当致能 DMA 或外部 CGROM 时， SPI 只支持 mode 0 与 mode 3。
	mode   CPOL:Clock Polarity bit  CPHA:Clock Phase bit
	0             0                      0
	1             0                      1
	2             1                      0
	3             1                      1
*/
void Reset_CPOL(void)
{
  uint8_t temp;
  RA8876_CmdWrite(0xB9);
  temp = RA8876_DataRead();
  temp &= cClrb1;
  RA8876_DataWrite(temp);
}

void Set_CPOL(void)
{
  uint8_t temp;
  RA8876_CmdWrite(0xB9);
  temp = RA8876_DataRead();
  temp |= cSetb1;
  RA8876_DataWrite(temp);
}

/*
*********************************************************************************************************
*	函 数 名: Reset_CPHA
*	功能说明: 将CPHA时钟极性位置0
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
/*
	SPI主控制寄存器[B9H]
bit1-0：
	SPI operation mode
	当致能 DMA 或外部 CGROM 时， SPI 只支持 mode 0 与 mode 3。
	mode   CPOL:Clock Polarity bit  CPHA:Clock Phase bit
	0             0                      0
	1             0                      1
	2             1                      0
	3             1                      1
*/
void Reset_CPHA(void)
{
  uint8_t temp;
  RA8876_CmdWrite(0xB9);
  temp = RA8876_DataRead();
  temp &= cClrb0;
  RA8876_DataWrite(temp);
}

void Set_CPHA(void)
{
  uint8_t temp;
  RA8876_CmdWrite(0xB9);
  temp = RA8876_DataRead();
  temp |= cSetb0;
  RA8876_DataWrite(temp);
}







//REG[BAh] SPI master Status Register (SPIMSR) 
uint8_t Tx_FIFO_Empty_Flag(void)
{
  RA8876_CmdWrite(0xBA);
  if((RA8876_DataRead()&0x80)==0x80)
  return 1;
  else
  return 0;
}

uint8_t Tx_FIFO_Full_Flag(void)
{
  RA8876_CmdWrite(0xBA);
  if((RA8876_DataRead()&0x40)==0x40)
  return 1;
  else
  return 0;
} 

uint8_t Rx_FIFO_Empty_Flag(void)
{
  RA8876_CmdWrite(0xBA);
  if((RA8876_DataRead()&0x20)==0x20)
  return 1;
  else
  return 0;
} 

uint8_t Rx_FIFO_full_flag(void)
{
   RA8876_CmdWrite(0xBA);
   if((RA8876_DataRead()&0x10)==0x10)
   return 1;
   else
   return 0;
} 

uint8_t OVFI_Flag(void)
{
   RA8876_CmdWrite(0xBA);
   if((RA8876_DataRead()&0x08)==0x08)
   return 1;
   else
   return 0;
}

void Clear_OVFI_Flag(void)
{
   uint8_t temp;
   RA8876_CmdWrite(0xBA);
   temp = RA8876_DataRead();
   temp |= cSetb3;
   RA8876_DataWrite(temp);
}

uint8_t EMTI_Flag(void)
{
   RA8876_CmdWrite(0xBA);
   if((RA8876_DataRead()&0x04)==0x04)
   return 1;
   else
   return 0;
}

void Clear_EMTI_Flag(void)
{
   uint8_t temp;
   RA8876_CmdWrite(0xBA);
   temp = RA8876_DataRead();
   temp |= cSetb2;
   RA8876_DataWrite(temp);
}

/*
*********************************************************************************************************
*	函 数 名: SPI_Clock_Period
*	功能说明: SPI时钟周期
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
//REG[BB] SPI Clock period (SPIDIV) 
void SPI_Clock_Period(uint8_t temp)
{
/*
SPI_clock= CORE_FREQ / ((Divisor+1)x2)
*/
/*
	SPI时钟周期[BBH]
bit7-0:
	SPI Clock period
	参考系统频率及 SPI 装置需要的频率以设定正确周期。
	Fsck = Fcore/(divisor + 1) * 2
*/
   RA8876_CmdWrite(0xBB);
   RA8876_DataWrite(temp);
} 

/*
*********************************************************************************************************
*	函 数 名: SFI_DMA_Source_Start_Address
*	功能说明: 串行Flash DMA源起始地址
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
//[BCh][BDh][BEh][BFh]=========================================================================
void SFI_DMA_Source_Start_Address(unsigned long Addr)
{
/*
DMA Source START ADDRESS
This bits index serial flash address [7:0][15:8][23:16][31:24]
*/
	RA8876_CmdWrite(0xBC);
	RA8876_DataWrite(Addr);
	RA8876_CmdWrite(0xBD);
	RA8876_DataWrite(Addr>>8);
	RA8876_CmdWrite(0xBE);
	RA8876_DataWrite(Addr>>16);
	RA8876_CmdWrite(0xBF);
	RA8876_DataWrite(Addr>>24);
}
//[C0h][C1h][C2h][C3h]=========================================================================
void SFI_DMA_Destination_Start_Address(unsigned long Addr)
{
/*
DMA Destination START ADDRESS 
[1:0]Fix at 0
This bits index SDRAM address [7:0][15:8][23:16][31:24]
*/
	RA8876_CmdWrite(0xC0);
	RA8876_DataWrite(Addr);
	RA8876_CmdWrite(0xC1);
	RA8876_DataWrite(Addr>>8);
	RA8876_CmdWrite(0xC2);
	RA8876_DataWrite(Addr>>16);
	RA8876_CmdWrite(0xC3);
	RA8876_DataWrite(Addr>>24);
}
//[C0h][C1h][C2h][C3h]=========================================================================
void SFI_DMA_Destination_Upper_Left_Corner(uint16_t WX,uint16_t HY)
{
/*
C0h
This register defines DMA Destination Window Upper-Left corner 
X-coordination [7:0] on Canvas area. 
When REG DMACR bit 1 = 1 (Block Mode) 
This register defines Destination address [7:2] in SDRAM. 
C1h
When REG DMACR bit 1 = 0 (Linear Mode) 
This register defines DMA Destination Window Upper-Left corner 
X-coordination [12:8] on Canvas area. 
When REG DMACR bit 1 = 1 (Block Mode) 
This register defines Destination address [15:8] in SDRAM.
C2h
When REG DMACR bit 1 = 0 (Linear Mode) 
This register defines DMA Destination Window Upper-Left corner
Y-coordination [7:0] on Canvas area. 
When REG DMACR bit 1 = 1 (Block Mode) 
This register defines Destination address [23:16] in SDRAM. 
C3h
When REG DMACR bit 1 = 0 (Linear Mode) 
This register defines DMA Destination Window Upper-Left corner 
Y-coordination [12:8] on Canvas area. 
When REG DMACR bit 1 = 1 (Block Mode) 
This register defines Destination address [31:24] in SDRAM. 
*/
/*
	DMA目的窗口左上角坐标[C0h][C1h][C2h][C3h]
*/
 RA8876_CmdWrite(0xC0);
 RA8876_DataWrite(WX);
 RA8876_CmdWrite(0xC1);
 RA8876_DataWrite(WX>>8);
 
 RA8876_CmdWrite(0xC2);
 RA8876_DataWrite(HY);
 RA8876_CmdWrite(0xC3);
 RA8876_DataWrite(HY>>8);
}



//[C6h][C7h][C8h][C9h]=========================================================================
void SFI_DMA_Transfer_Number(unsigned long Addr)
{
/*
When REG DMACR bit 1 = 0 (Linear Mode)
DMA Transfer Number [7:0][15:8][23:16][31:24]

When REG DMACR bit 1 = 1 (Block Mode)
DMA Block Width [7:0][15:8]
DMA Block HIGH[7:0][15:8]
*/
	RA8876_CmdWrite(0xC6);
	RA8876_DataWrite(Addr);
	RA8876_CmdWrite(0xC7);
	RA8876_DataWrite(Addr>>8);
	RA8876_CmdWrite(0xC8);
	RA8876_DataWrite(Addr>>16);
	RA8876_CmdWrite(0xC9);
	RA8876_DataWrite(Addr>>24);
}
void SFI_DMA_Transfer_Width_Height(uint16_t WX,uint16_t HY)
{
/*
When REG DMACR bit 1 = 0 (Linear Mode)
DMA Transfer Number [7:0][15:8][23:16][31:24]

When REG DMACR bit 1 = 1 (Block Mode)
DMA Block Width [7:0][15:8]
DMA Block HIGH[7:0][15:8]
*/
	RA8876_CmdWrite(0xC6);
	RA8876_DataWrite(WX);
	RA8876_CmdWrite(0xC7);
	RA8876_DataWrite(WX>>8);

	RA8876_CmdWrite(0xC8);
	RA8876_DataWrite(HY);
	RA8876_CmdWrite(0xC9);
	RA8876_DataWrite(HY>>8);
}
//[CAh][CBh]=========================================================================
void SFI_DMA_Source_Width(uint16_t WX)
{
/*
DMA Source Picture Width [7:0][12:8]
Unit: pixel
*/
	RA8876_CmdWrite(0xCA);
	RA8876_DataWrite(WX);
	RA8876_CmdWrite(0xCB);
	RA8876_DataWrite(WX>>8);
}

//[CCh]=========================================================================

void Font_Select_UserDefine_Mode(void)
{
/*[bit7-6]
User-defined Font /CGROM Font Selection Bit in Text Mode
00 : Internal CGROM
01 : Genitop serial flash
10 : User-defined Font
*/
	uint8_t temp;
	RA8876_CmdWrite(0xCC);
	temp = RA8876_DataRead();
    temp |= cSetb7;
	temp &= cClrb6;
	RA8876_DataWrite(temp);
}
void CGROM_Select_Internal_CGROM(void)
{
/*[bit7-6]
User-defined Font /CGROM Font Selection Bit in Text Mode
00 : Internal CGROM
01 : Genitop serial flash
10 : User-defined Font
*/
	uint8_t temp;
	RA8876_CmdWrite(0xCC);
	temp = RA8876_DataRead();
	temp &= cClrb7;
    temp &= cClrb6;
	RA8876_DataWrite(temp);
}

/*
*********************************************************************************************************
*	函 数 名: CGROM_Select_Genitop_FontROM
*	功能说明: 字符控制寄存器
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void CGROM_Select_Genitop_FontROM(void)
{
/*[bit7-6]
User-defined Font /CGROM Font Selection Bit in Text Mode
00 : Internal CGROM
01 : Genitop serial flash
10 : User-defined Font
*/
/*
	字符控制寄存器[CCH]
bit7-6：
	Character source selection
	00: 内部 CGROM 为字符来源。
	01: 外部 CGROM 为字符来源 (集通闪存)。
	10: 使用者定义字符。
	11: NA
*/
	uint8_t temp;
	RA8876_CmdWrite(0xCC);
	temp = RA8876_DataRead();
	temp &= cClrb7;
    temp |= cSetb6;
	RA8876_DataWrite(temp);
}
void Font_Select_8x16_16x16(void)
{
/*[bit5-4]
Font Height Setting
00b : 8x16 / 16x16.
01b : 12x24 / 24x24.
10b : 16x32 / 32x32.
*** User-defined Font width is decided by font code. Genitop
serial flashˇs font width is decided by font code or GT Font ROM
control register.
*/
	uint8_t temp;
	RA8876_CmdWrite(0xCC);
	temp = RA8876_DataRead();
    temp &= cClrb5;
    temp &= cClrb4;
	RA8876_DataWrite(temp);
}
void Font_Select_12x24_24x24(void)
{
/*[bit5-4]
Font Height Setting
00b : 8x16 / 16x16.
01b : 12x24 / 24x24.
10b : 16x32 / 32x32.
*** User-defined Font width is decided by font code. Genitop
serial flashˇs font width is decided by font code or GT Font ROM
control register.
*/
	uint8_t temp;
	RA8876_CmdWrite(0xCC);
	temp = RA8876_DataRead();
    temp &= cClrb5;
    temp |= cSetb4;
	RA8876_DataWrite(temp);
}
void Font_Select_16x32_32x32(void)
{
/*[bit5-4]
Font Height Setting
00b : 8x16 / 16x16.
01b : 12x24 / 24x24.
10b : 16x32 / 32x32.
*** User-defined Font width is decided by font code. Genitop
serial flashˇs font width is decided by font code or GT Font ROM
control register.
*/
	uint8_t temp;
	RA8876_CmdWrite(0xCC);
	temp = RA8876_DataRead();
    temp |= cSetb5;
    temp &= cClrb4;
	RA8876_DataWrite(temp);
}
void Internal_CGROM_Select_ISOIEC8859_1(void)
{
/*
Font Selection for internal CGROM
When FNCR0 B7 = 0 and B5 = 0, Internal CGROM supports the
8x16 character sets with the standard coding of ISO/IEC 8859-1,2,4,5,
which supports English and most of European country languages.
00b : ISO/IEC 8859-1.
01b : ISO/IEC 8859-2.
10b : ISO/IEC 8859-4.
11b : ISO/IEC 8859-5.
*/
	uint8_t temp;
	RA8876_CmdWrite(0xCC);
	temp = RA8876_DataRead();
    temp &= cClrb1;
    temp &= cClrb0;
	RA8876_DataWrite(temp);
}
void Internal_CGROM_Select_ISOIEC8859_2(void)
{
/*
Font Selection for internal CGROM
When FNCR0 B7 = 0 and B5 = 0, Internal CGROM supports the
8x16 character sets with the standard coding of ISO/IEC 8859-1,2,4,5,
which supports English and most of European country languages.
00b : ISO/IEC 8859-1.
01b : ISO/IEC 8859-2.
10b : ISO/IEC 8859-4.
11b : ISO/IEC 8859-5.
*/
	uint8_t temp;
	RA8876_CmdWrite(0xCC);
	temp = RA8876_DataRead();
    temp &= cClrb1;
    temp |= cSetb0;
	RA8876_DataWrite(temp);
}
void Internal_CGROM_Select_ISOIEC8859_4(void)
{
/*
Font Selection for internal CGROM
When FNCR0 B7 = 0 and B5 = 0, Internal CGROM supports the
8x16 character sets with the standard coding of ISO/IEC 8859-1,2,4,5, 
which supports English and most of European country languages.
00b : ISO/IEC 8859-1.
01b : ISO/IEC 8859-2.
10b : ISO/IEC 8859-4.
11b : ISO/IEC 8859-5.
*/
	uint8_t temp;
	RA8876_CmdWrite(0xCC);
	temp = RA8876_DataRead();
    temp |= cSetb1;
    temp &= cClrb0;
	RA8876_DataWrite(temp);
}
void Internal_CGROM_Select_ISOIEC8859_5(void)
{
/*
Font Selection for internal CGROM
When FNCR0 B7 = 0 and B5 = 0, Internal CGROM supports the
8x16 character sets with the standard coding of ISO/IEC 8859-1,2,4,5, 
which supports English and most of European country languages.
00b : ISO/IEC 8859-1.
01b : ISO/IEC 8859-2.
10b : ISO/IEC 8859-4.
11b : ISO/IEC 8859-5.
*/
	uint8_t temp;
	RA8876_CmdWrite(0xCC);
	temp = RA8876_DataRead();
    temp |= cSetb1;
    temp |= cSetb0;
	RA8876_DataWrite(temp);
}
//[CDh]=========================================================================
void Enable_Font_Alignment(void)
{
/*
Full Alignment Selection Bit
0 : Full alignment disable.
1 : Full alignment enable.
*/
	uint8_t temp;
	RA8876_CmdWrite(0xCD);
	temp = RA8876_DataRead();
    temp |= cSetb7;
	RA8876_DataWrite(temp);
}
void Disable_Font_Alignment(void)
{
/*
Full Alignment Selection Bit
0 : Full alignment disable.
1 : Full alignment enable.
*/
	uint8_t temp;
	RA8876_CmdWrite(0xCD);
	temp = RA8876_DataRead();
    temp &= cClrb7;
	RA8876_DataWrite(temp);
}
void Font_Background_select_Original_Canvas(void)
{
/*		
Font Transparency
0 : Character抯 background displayed with specified color.
1 : Character抯 background displayed with original canvas background.
*/
	uint8_t temp;
	RA8876_CmdWrite(0xCD);
	temp = RA8876_DataRead();
    temp |= cSetb6;
	RA8876_DataWrite(temp);
}
void Font_Background_select_Color(void)
{
/*
Font Transparency
0 : Character抯 background displayed with specified color.
1 : Character抯 background displayed with original canvas background.
*/
	uint8_t temp;
	RA8876_CmdWrite(0xCD);
	temp = RA8876_DataRead();
    temp &= cClrb6;
	RA8876_DataWrite(temp);
}
void Font_0_degree(void)
{
/*
Font Rotation
0 : Normal
Text direction from left to right then from top to bottom
1 : Counterclockwise 90 degree & horizontal flip
Text direction from top to bottom then from left to right
(it should accommodate with set VDIR as 1)
This attribute can be changed only when previous font write
finished (core_busy = 0)
*/
	uint8_t temp;
	RA8876_CmdWrite(0xCD);
	temp = RA8876_DataRead();
    temp &= cClrb4;
	RA8876_DataWrite(temp);
}
void Font_90_degree(void)
{
/*
Font Rotation
0 : Normal
Text direction from left to right then from top to bottom
1 : Counterclockwise 90 degree & horizontal flip
Text direction from top to bottom then from left to right
(it should accommodate with set VDIR as 1)
This attribute can be changed only when previous font write
finished (core_busy = 0)
*/
	uint8_t temp;
	RA8876_CmdWrite(0xCD);
	temp = RA8876_DataRead();
    temp |= cSetb4;
	RA8876_DataWrite(temp);
}
void Font_Width_X1(void)
{
/*
Horizontal Font Enlargement
00b : X1.
01b : X2.
10b : X3.
11b : X4.
*/
	uint8_t temp;
	RA8876_CmdWrite(0xCD);
	temp = RA8876_DataRead();
    temp &= cClrb3;
    temp &= cClrb2;
	RA8876_DataWrite(temp);
}
void Font_Width_X2(void)
{
/*
Horizontal Font Enlargement
00b : X1.
01b : X2.
10b : X3.
11b : X4.
*/
	uint8_t temp;
	RA8876_CmdWrite(0xCD);
	temp = RA8876_DataRead();
    temp &= cClrb3;
    temp |= cSetb2;
	RA8876_DataWrite(temp);
}
void Font_Width_X3(void)
{
/*
Horizontal Font Enlargement
00b : X1.
01b : X2.
10b : X3.
11b : X4.
*/
	uint8_t temp;
	RA8876_CmdWrite(0xCD);
	temp = RA8876_DataRead();
    temp |= cSetb3;
    temp &= cClrb2;
	RA8876_DataWrite(temp);
}
void Font_Width_X4(void)
{
/*
Horizontal Font Enlargement
00b : X1.
01b : X2.
10b : X3.
11b : X4.
*/
	uint8_t temp;
	RA8876_CmdWrite(0xCD);
	temp = RA8876_DataRead();
    temp |= cSetb3;
    temp |= cSetb2;
	RA8876_DataWrite(temp);
}
void Font_Height_X1(void)
{
/*
Vertical Font Enlargement
00b : X1.
01b : X2.
10b : X3.
11b : X4.
*/
	uint8_t temp;
	RA8876_CmdWrite(0xCD);
	temp = RA8876_DataRead();
    temp &= cClrb1;
    temp &= cClrb0;
	RA8876_DataWrite(temp);
}
void Font_Height_X2(void)
{
/*
Vertical Font Enlargement
00b : X1.
01b : X2.
10b : X3.
11b : X4.
*/
	uint8_t temp;
	RA8876_CmdWrite(0xCD);
	temp = RA8876_DataRead();
    temp &= cClrb1;
    temp |= cSetb0;
	RA8876_DataWrite(temp);
}
void Font_Height_X3(void)
{
/*
Vertical Font Enlargement
00b : X1.
01b : X2.
10b : X3.
11b : X4.
*/
	uint8_t temp;
	RA8876_CmdWrite(0xCD);
	temp = RA8876_DataRead();
    temp |= cSetb1;
    temp &= cClrb0;
	RA8876_DataWrite(temp);
}
void Font_Height_X4(void)
{
/*
Vertical Font Enlargement
00b : X1.
01b : X2.
10b : X3.
11b : X4.
*/
	uint8_t temp;
	RA8876_CmdWrite(0xCD);
	temp = RA8876_DataRead();
    temp |= cSetb1;
    temp |= cSetb0;
	RA8876_DataWrite(temp);
}

//[CEh]=========================================================================
void GTFont_Select_GT21L16T1W(void)
{
/*
GT Serial Font ROM Select
000b: GT21L16T1W
001b: GT30L16U2W
010b: GT30L24T3Y
011b: GT30L24M1Z
100b: GT30L32S4W
101b: GT20L24F6Y
110b: GT21L24S1W
*/
	uint8_t temp;
	RA8876_CmdWrite(0xCE);
	temp = RA8876_DataRead();
    temp &= cClrb7;
    temp &= cClrb6;
    temp &= cClrb5;
	RA8876_DataWrite(temp);
}
void GTFont_Select_GT30L16U2W(void)
{
/*
GT Serial Font ROM Select
000b: GT21L16T1W
001b: GT30L16U2W
010b: GT30L24T3Y
011b: GT30L24M1Z
100b: GT30L32S4W
101b: GT20L24F6Y
110b: GT21L24S1W
*/
	uint8_t temp;
	RA8876_CmdWrite(0xCE);
	temp = RA8876_DataRead();
    temp &= cClrb7;
    temp &= cClrb6;
    temp |= cSetb5;
	RA8876_DataWrite(temp);
}
void GTFont_Select_GT30L24T3Y(void)
{
/*
GT Serial Font ROM Select
000b: GT21L16T1W
001b: GT30L16U2W
010b: GT30L24T3Y
011b: GT30L24M1Z
100b: GT30L32S4W
101b: GT20L24F6Y
110b: GT21L24S1W
*/
	uint8_t temp;
	RA8876_CmdWrite(0xCE);
	temp = RA8876_DataRead();
    temp &= cClrb7;
    temp |= cSetb6;
    temp &= cClrb5;
	RA8876_DataWrite(temp);
}
void GTFont_Select_GT30L24M1Z(void)
{
/*
GT Serial Font ROM Select
000b: GT21L16T1W
001b: GT30L16U2W
010b: GT30L24T3Y
011b: GT30L24M1Z
100b: GT30L32S4W
101b: GT20L24F6Y
110b: GT21L24S1W
*/
	uint8_t temp;
	RA8876_CmdWrite(0xCE);
	temp = RA8876_DataRead();
    temp &= cClrb7;
    temp |= cSetb6;
    temp |= cSetb5;
	RA8876_DataWrite(temp);
}
void GTFont_Select_GT30L32S4W(void)
{
/*
GT Serial Font ROM Select
000b: GT21L16T1W
001b: GT30L16U2W
010b: GT30L24T3Y
011b: GT30L24M1Z
100b: GT30L32S4W
101b: GT20L24F6Y
110b: GT21L24S1W
*/
	uint8_t temp;
	RA8876_CmdWrite(0xCE);
	temp = RA8876_DataRead();
    temp |= cSetb7;
    temp &= cClrb6;
    temp &= cClrb5;
	RA8876_DataWrite(temp);
}
void GTFont_Select_GT20L24F6Y(void)
{
/*
GT Serial Font ROM Select
000b: GT21L16T1W
001b: GT30L16U2W
010b: GT30L24T3Y
011b: GT30L24M1Z
100b: GT30L32S4W
101b: GT20L24F6Y
110b: GT21L24S1W
*/
	uint8_t temp;
	RA8876_CmdWrite(0xCE);
	temp = RA8876_DataRead();
    temp |= cSetb7;
    temp &= cClrb6;
    temp |= cSetb5;
	RA8876_DataWrite(temp);
}
void GTFont_Select_GT21L24S1W(void)
{
/*
GT Serial Font ROM Select
000b: GT21L16T1W
001b: GT30L16U2W
010b: GT30L24T3Y
011b: GT30L24M1Z
100b: GT30L32S4W
101b: GT20L24F6Y
110b: GT21L24S1W
*/
	uint8_t temp;
	RA8876_CmdWrite(0xCE);
	temp = RA8876_DataRead();
    temp |= cSetb7;
    temp |= cSetb6;
    temp &= cClrb5;
	RA8876_DataWrite(temp);
}


/*
*********************************************************************************************************
*	函 数 名: Set_GTFont_Decoder
*	功能说明: 集通字符解码
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void Set_GTFont_Decoder(uint8_t temp)
{
/*
[bit7-3]
FONT ROM Coding Setting
For specific GT serial Font ROM, the coding method must be set for decoding.
00000b: GB2312
00001b: GB12345/GB18030
00010b: BIG5
00011b: UNICODE
00100b: ASCII
00101b: UNI-Japanese
00110b: JIS0208
00111b: Latin/Greek/ Cyrillic / Arabic/Thai/Hebrew
01000b: Korea
10001b: ISO-8859-1
10010b: ISO-8859-2
10011b: ISO-8859-3
10100b: ISO-8859-4
10101b: ISO-8859-5
10110b: ISO-8859-6
10111b: ISO-8859-7
11000b: ISO-8859-8
11001b: ISO-8859-9
11010b: ISO-8859-10
11011b: ISO-8859-11
11100b: ISO-8859-12
11101b: ISO-8859-13
11110b: ISO-8859-14
11111b: ISO-8859-15

[bit1-0]
ASCII / Latin/Greek/ Cyrillic / Arabic
		(ASCII)		(Latin/Greek/Cyrillic)	(Arabic)
00b		Normal			Normal 					NA
01b		Arial 		Variable Width 			Presentation Forms-A
10b		Roman 			NA 					Presentation Forms-B
11b		Bold			NA 						NA
*/
/* 
	GT字符ROM控制寄存器
bit7-3：
	Character sets
	对于指定的集通 CGROM，编码方式与译码方式必须是对应的。
bit1-0：
	字符宽度设定
*/
	RA8876_CmdWrite(0xCF);
	RA8876_DataWrite(temp);
}
//[D0h]=========================================================================
void Font_Line_Distance(uint8_t temp)
{
/*[bit4-0]
Font Line Distance Setting
Setting the font character line distance when setting memory font
write cursor auto move. (Unit: pixel)
*/
	RA8876_CmdWrite(0xD0);
	RA8876_DataWrite(temp);
}
//[D1h]=========================================================================
void Set_Font_to_Font_Width(uint8_t temp)
{
/*[bit5-0]
Font to Font Width Setting (Unit: pixel)
*/
	RA8876_CmdWrite(0xD1);
	RA8876_DataWrite(temp);
}
//[D2h]~[D4h]=========================================================================
void Foreground_RGB(uint8_t RED,uint8_t GREEN,uint8_t BLUE)
{
/*
[D2h] Foreground Color - Red, for draw, text or color expansion
[D3h] Foreground Color - Green, for draw, text or color expansion
[D4h] Foreground Color - Blue, for draw, text or color expansion
*/  
    RA8876_CmdWrite(0xD2);
 RA8876_DataWrite(RED);
 
    RA8876_CmdWrite(0xD3);
 RA8876_DataWrite(GREEN);
  
    RA8876_CmdWrite(0xD4);
 RA8876_DataWrite(BLUE);
}

//Input data format:R3G3B2  
void Foreground_color_256(uint8_t temp) 
{
    RA8876_CmdWrite(0xD2);
 RA8876_DataWrite(temp);
 
    RA8876_CmdWrite(0xD3);
 RA8876_DataWrite(temp<<3);
  
    RA8876_CmdWrite(0xD4);
 RA8876_DataWrite(temp<<6);
}
 
//Input data format:R5G6B5 
void Foreground_color_65k(uint16_t temp)
{
    RA8876_CmdWrite(0xD2);
 RA8876_DataWrite(temp>>8);
 
    RA8876_CmdWrite(0xD3);
 RA8876_DataWrite(temp>>3);
  
    RA8876_CmdWrite(0xD4);
 RA8876_DataWrite(temp<<3);
}
 
//Input data format:R8G8B8 
void Foreground_color_16M(unsigned long temp)
{
    RA8876_CmdWrite(0xD2);
 RA8876_DataWrite(temp>>16);
 
    RA8876_CmdWrite(0xD3);
 RA8876_DataWrite(temp>>8);
  
    RA8876_CmdWrite(0xD4);
 RA8876_DataWrite(temp);
}
 
 
 
//[D5h]~[D7h]=========================================================================
/*
[D5h] Background Color - Red, for Text or color expansion
[D6h] Background Color - Green, for Text or color expansion
[D7h] Background Color - Blue, for Text or color expansion
*/ 
void Background_RGB(uint8_t RED,uint8_t GREEN,uint8_t BLUE)
{
   
    RA8876_CmdWrite(0xD5);
 RA8876_DataWrite(RED);
  
    RA8876_CmdWrite(0xD6);
 RA8876_DataWrite(GREEN);
   
    RA8876_CmdWrite(0xD7);
 RA8876_DataWrite(BLUE);
}
 
//Input data format:R3G3B2
void Background_color_256(uint8_t temp)
{
    RA8876_CmdWrite(0xD5);
 RA8876_DataWrite(temp);
  
    RA8876_CmdWrite(0xD6);
 RA8876_DataWrite(temp<<3);
   
    RA8876_CmdWrite(0xD7);
 RA8876_DataWrite(temp<<6);
}
 
//Input data format:R5G6B6
void Background_color_65k(uint16_t temp)
{
    RA8876_CmdWrite(0xD5);
 RA8876_DataWrite(temp>>8);
  
    RA8876_CmdWrite(0xD6);
 RA8876_DataWrite(temp>>3);
   
    RA8876_CmdWrite(0xD7);
 RA8876_DataWrite(temp<<3);
}
 
//Input data format:R8G8B8
void Background_color_16M(unsigned long temp)
{
    RA8876_CmdWrite(0xD5);
 RA8876_DataWrite(temp>>16);
  
    RA8876_CmdWrite(0xD6);
 RA8876_DataWrite(temp>>8);
   
    RA8876_CmdWrite(0xD7);
 RA8876_DataWrite(temp);
}

//[DBh]~[DEh]=========================================================================
void CGRAM_Start_address(unsigned long Addr)
{
/*
CGRAM START ADDRESS [31:0]
*/	 
    RA8876_CmdWrite(0xDB);
	RA8876_DataWrite(Addr);
    RA8876_CmdWrite(0xDC);
	RA8876_DataWrite(Addr>>8);
    RA8876_CmdWrite(0xDD);
	RA8876_DataWrite(Addr>>16);
    RA8876_CmdWrite(0xDE);
	RA8876_DataWrite(Addr>>24);
}

//[DFh]=========================================================================
/*
[bit7] Enter Power saving state
0: Normal state.
1: Enter power saving state.
[bit1][bit0] Power saving Mode definition
00: NA
01: Standby Mode
10: Suspend Mode
11: Sleep Mode
*/
void Power_Normal_Mode(void)
{
	RA8876_CmdWrite(0xDF);
	RA8876_DataWrite(0x00);
	Check_IC_ready();
}
void Power_Saving_Standby_Mode(void)
{
	RA8876_CmdWrite(0xDF);
//	RA8876_DataWrite(0x01);
//	RA8876_CmdWrite(0xDF);
	RA8876_DataWrite(0x81);
}
void Power_Saving_Suspend_Mode(void)
{
	RA8876_CmdWrite(0xDF);
//	RA8876_DataWrite(0x02);
//	RA8876_CmdWrite(0xDF);
	RA8876_DataWrite(0x82);
}
void Power_Saving_Sleep_Mode(void)
{
	RA8876_CmdWrite(0xDF);
	RA8876_DataWrite(0x03);
	RA8876_CmdWrite(0xDF);
	RA8876_DataWrite(0x83);
}










//[E5h]~[E6h]=========================================================================
void RA8876_I2CM_Clock_Prescale(uint16_t WX)
{
/*
I2C Master Clock Pre-scale [7:0]
I2C Master Clock Pre-scale [15:8]
XSCL = CCLK / (5*(prescale + 2))
*/  
    RA8876_CmdWrite(0xE5);
 RA8876_DataWrite(WX);
    RA8876_CmdWrite(0xE6);
 RA8876_DataWrite(WX>>8);
}
//[E7h]=========================================================================
void RA8876_I2CM_Transmit_Data(uint8_t temp)
{
/*
I2C Master Transmit[7:0]
*/  
    RA8876_CmdWrite(0xE7);
 RA8876_DataWrite(temp);
}
//[E8h]=========================================================================
uint8_t RA8876_I2CM_Receiver_Data(void)
{
/*
I2C Master Receiver [7:0]
*/  
 uint8_t temp;
    RA8876_CmdWrite(0xE8);
 temp=RA8876_DataRead();
 return temp;
}
//[E9h]=========================================================================
/*
[bit7] START
	Generate (repeated) start condition and be cleared by hardware automatically
	Note : This bit is always read as 0.
[bit6] STOP
	Generate stop condition and be cleared by hardware automatically
	Note : This bit is always read as 0.
[bit5] READ
	READ(READ and WRITE canˇt be used simultaneously)
	Read form slave and be cleared by hardware automatically
	Note : This bit is always read as 0.
[bit4] WRITE
	WRITE(READ and WRITE canˇt be used simultaneously)
	Write to slave and be cleared by hardware automatically
	Note : This bit is always read as 0.
[bit3] ACKNOWLEDGE
	When as a I2C master receiver
	0 : Sent ACK.
	1 : Sent NACK.
	Note : This bit is always read as 0.
[bit0] Noise Filter
	0 : Enable.
	1 : Disable.
*/

//[bit7] START
void RA8876_I2CM_Write_With_Start(void)
{
 RA8876_CmdWrite(0xE9);
#ifdef Disable_I2CM_Noise_Filter 
 RA8876_DataWrite(0x90);
#endif
 
#ifdef Enable_I2CM_Noise_Filter 
 RA8876_DataWrite(0x91);
#endif		  
}

//[bit6] STOP 
void RA8876_I2CM_Stop(void)
{
 RA8876_CmdWrite(0xE9);
#ifdef Disable_I2CM_Noise_Filter 
 RA8876_DataWrite(0x40);
#endif
 
#ifdef Enable_I2CM_Noise_Filter 
 RA8876_DataWrite(0x41);
#endif
}

//[bit5] READ
void RA8876_I2CM_Read_With_Ack(void)
{		  
 RA8876_CmdWrite(0xE9);
#ifdef Disable_I2CM_Noise_Filter 
 RA8876_DataWrite(0x20);
#endif
#ifdef Enable_I2CM_Noise_Filter 
 RA8876_DataWrite(0x21);
#endif
}
 
void RA8876_I2CM_Read_With_Nack(void)
{		  
 RA8876_CmdWrite(0xE9);
#ifdef Disable_I2CM_Noise_Filter 
 RA8876_DataWrite(0x68);
#endif
#ifdef Enable_I2CM_Noise_Filter 
 RA8876_DataWrite(0x69);
#endif
}

//[bit4] WRITE 
void RA8876_I2CM_Write(void)
{
 RA8876_CmdWrite(0xE9);
#ifdef Disable_I2CM_Noise_Filter 
 RA8876_DataWrite(0x10);
#endif
#ifdef Enable_I2CM_Noise_Filter 
 RA8876_DataWrite(0x11);
#endif
}
 
 
 
//[EAh]=========================================================================

uint8_t RA8876_I2CM_Check_Slave_ACK(void)
{
/*[bit7]
Received acknowledge from slave
0 : Acknowledge received.
1 : No Acknowledge received. 
*/ 
uint8_t temp;
 
 RA8876_CmdWrite(0xEA);
 temp=RA8876_DataRead();
 if((temp&0x80)==0x80)
  return 1;
 else
  return 0; 
}

uint8_t RA8876_I2CM_Bus_Busy(void)
{
/*[bit6]
I2C Bus is Busy
0 : Idle.
1 : Busy.  
*/ 
 uint8_t temp; 

 RA8876_CmdWrite(0xEA);
 temp=RA8876_DataRead();
 if((temp&0x40)==0x40)
  return 1;
 else
  return 0;    
}

uint8_t RA8876_I2CM_transmit_Progress(void)
{
/*[bit6]
 0=Complete
 1=Transferring
*/ 
 uint8_t temp; 

 RA8876_CmdWrite(0xEA);
 temp=RA8876_DataRead();
 if((temp&0x02)==0x02)
 return 1;
 else
 return 0;
}
 
uint8_t RA8876_I2CM_Arbitration(void)
{
/*[bit6]
I2C Bus is Busy
0 : Idle.
1 : Busy.  
*/ 
 uint8_t temp; 

 RA8876_CmdWrite(0xEA);
 temp=RA8876_DataRead();
 temp&=0x01;
 return temp;
}


//[F0h]=========================================================================
void Set_GPIO_A_In_Out(uint8_t temp)
{
/*
GPO-A_dir[7:0] : General Purpose I/O direction control.
0: Output
1: Input
*/
	RA8876_CmdWrite(0xF0);
	RA8876_DataWrite(temp);
}
//[F1h]=========================================================================
void Write_GPIO_A_7_0(uint8_t temp)
{
/*
GPI-A[7:0] : General Purpose Input, share with DB[15:8]
GPO-A[7:0] : General Purpose Output, share with DB[15:8]
*/
	RA8876_CmdWrite(0xF1);
	RA8876_DataWrite(temp);
}
uint8_t Read_GPIO_A_7_0(void)
{
/*
GPI-A[7:0] : General Purpose Input, share with DB[15:8]
GPO-A[7:0] : General Purpose Output, share with DB[15:8]
*/
	uint8_t temp;
	RA8876_CmdWrite(0xF1);
	temp=RA8876_DataRead();
	return temp;
}
//[F2h]=========================================================================
void Write_GPIO_B_7_4(uint8_t temp)
{
/*
Bit [7:4] is port B's General Purpose Output bit [7:4]. They are share with XKOUT[3:0]
Bit [3:0] is not writable.
*/
	RA8876_CmdWrite(0xF2);
	RA8876_DataWrite(temp);
}
uint8_t Read_GPIO_B_7_0(void)
{
/*
Bit[7:0] are share with {XKIN[3:0], XA0, XnWR, XnRD, XnCS}
Bit[3:0] are only available in serial MPU I/F, otherwise fix at 0.
*/
	uint8_t temp;
	RA8876_CmdWrite(0xF2);
	temp=RA8876_DataRead();
	return temp;
}

//[F3h]=========================================================================
void Set_GPIO_C_In_Out(uint8_t temp)
{
/*
GPIO-C_dir[7:0] : General Purpose I/O direction control.
0: Output
1: Input
*/
	RA8876_CmdWrite(0xF3);
	RA8876_DataWrite(temp);
}
//[F4h]=========================================================================
void Write_GPIO_C_7_0(uint8_t temp)
{
/*
REG[F4h] GPIO-C
GPIO-C[7] : XPWM0,
GPIO_C[4:0] : XnSFCS1, XnSFCS0, XMISO, XMOSI, XSCLK.
*/
	RA8876_CmdWrite(0xF4);
	RA8876_DataWrite(temp);
}
uint8_t Read_GPIO_C_7_0(void)
{
/*
REG[F4h] GPIO-C
GPIO-C[7] : XPWM0,
GPIO_C[4:0] : XnSFCS1, XnSFCS0, XMISO, XMOSI, XSCLK.
*/
	uint8_t temp;
	RA8876_CmdWrite(0xF4);
	temp=RA8876_DataRead();
	return temp;
}
//[F5h]=========================================================================
void Set_GPIO_D_In_Out(uint8_t temp)
{
/*
GPIO-D_dir[7:0] : General Purpose I/O direction control.
0: Output
1: Input
*/
	RA8876_CmdWrite(0xF5);
	RA8876_DataWrite(temp);
}
//[F6h]=========================================================================
void Write_GPIO_D_7_0(uint8_t temp)
{
/*
GPIO-D[7:0] : General Purpose Input/Output
*/
	RA8876_CmdWrite(0xF6);
	RA8876_DataWrite(temp);
}
uint8_t Read_GPIO_D_7_0(void)
{
/*
GPIO-D[7:0] : General Purpose Input/Output
*/
	uint8_t temp;
	RA8876_CmdWrite(0xF6);
	temp=RA8876_DataRead();
	return temp;
}
//[F7h]=========================================================================
void Set_GPIO_E_In_Out(uint8_t temp)
{
/*
GPIO-E_dir[7:0] : General Purpose I/O direction control.
0: Output
1: Input
*/
	RA8876_CmdWrite(0xF7);
	RA8876_DataWrite(temp);
}
//[F8h]=========================================================================
void Write_GPIO_E_7_0(uint8_t temp)
{
/*
GPIO-E[7:0] : General Purpose Input/Output.
share with {PDAT[23:19], PDAT[15:13]}
*/
	RA8876_CmdWrite(0xF8);
	RA8876_DataWrite(temp);
}
uint8_t Read_GPIO_E_7_0(void)
{
/*
GPIO-E[7:0] : General Purpose Input/Output.
share with {PDAT[23:19], PDAT[15:13]}
*/
	uint8_t temp;
	RA8876_CmdWrite(0xF8);
	temp=RA8876_DataRead();
	return temp;
}
//[F9h]=========================================================================
void Set_GPIO_F_In_Out(uint8_t temp)
{
/*
GPIO-F_dir[7:0] : General Purpose I/O direction control.
0: Output
1: Input
*/
	RA8876_CmdWrite(0xF9);
	RA8876_DataWrite(temp);
}
//[FAh]=========================================================================
void Write_GPIO_F_7_0(uint8_t temp)
{
/*
GPIO-F[7:0] : General Purpose Input/Output.
share with {XPDAT[12:10], XPDAT[7:3]}
*/
	RA8876_CmdWrite(0xFA);
	RA8876_DataWrite(temp);
}
uint8_t Read_GPIO_F_7_0(void)
{
/*
GPIO-F[7:0] : General Purpose Input/Output.
share with {XPDAT[12:10], XPDAT[7:3]}
*/
	uint8_t temp;
	RA8876_CmdWrite(0xFA);
	temp=RA8876_DataRead();
	return temp;
}

//[FBh]=========================================================================


void Long_Key_enable(void)
{
 /*
Key-Scan Control Register 1
[bit6]		LongKey Enable Bit
1 : Enable. Long key period is set by KSCR2 bit4-2.
0 : Disable.
*/
	uint8_t temp;
	RA8876_CmdWrite(0xFB);
	temp=RA8876_DataRead();
	temp |= cSetb6;
	RA8876_DataWrite(temp);
}


void Key_Scan_Freg(uint8_t setx)
{
/*KF2-0: Key-Scan Frequency */
  uint8_t temp;
  RA8876_CmdWrite(0xFB);
  temp = RA8876_DataRead();
  temp &= 0xf0;
  temp|= (setx&0x07);
  RA8876_DataWrite(temp);
}


//[FCh]=========================================================================

void Key_Scan_Wakeup_Function_Enable(void)
{
/*
Key-Scan Controller Register 2
[bit7]	
Key-Scan Wakeup Function Enable Bit
0: Key-Scan Wakeup function is disabled.
1: Key-Scan Wakeup function is enabled.
*/
	uint8_t temp;
	RA8876_CmdWrite(0xFC);
	temp=RA8876_DataRead();
	temp |= cSetb7;
	RA8876_DataWrite(temp);
}


void Long_Key_Timing_Adjustment(uint8_t setx)
{
 /*Long Key Timing Adjustment*/ 
  uint8_t temp,temp1;
  temp = setx & 0x1c;
  RA8876_CmdWrite(0xFC);
  temp1 = RA8876_DataRead();
  temp1|=temp;
  RA8876_DataWrite(temp1);   
 
}

uint8_t Numbers_of_Key_Hit(void)
{  
   uint8_t temp;
   RA8876_CmdWrite(0xFC);
   temp = RA8876_DataRead();   //read key touch number
   temp = temp & 0x03;	 //絋粄Τ碭龄砆
   return temp;
}

//[FDh][FEh][FFh]=========================================================================
uint8_t Read_Key_Strobe_Data_0(void)
{
/*
Key Strobe Data 0
The corresponding key code 0 that is pressed.
*/
	uint8_t temp;
	RA8876_CmdWrite(0xFD);
	temp=RA8876_DataRead();
	return temp;
}
uint8_t Read_Key_Strobe_Data_1(void)
{
/*
Key Strobe Data 1
The corresponding key code 1 that is pressed.
*/
	uint8_t temp;
	RA8876_CmdWrite(0xFE);
	temp=RA8876_DataRead();
	return temp;
}
uint8_t Read_Key_Strobe_Data_2(void)
{
/*
Key Strobe Data 2
The corresponding key code 2 that is pressed.
*/
	uint8_t temp;
	RA8876_CmdWrite(0xFF);
	temp=RA8876_DataRead();
	return temp;
}



/*
*********************************************************************************************************
*	函 数 名: Set_LCD_Panel
*	功能说明: 设定LCD面
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void Set_LCD_Panel(void)
{
	if (g_LcdType == LCD_70_800X480)	/* OTA7001 */
	{
		//**[10h]**//
		Select_LCD_Sync_Mode();	// Enable XVSYNC, XHSYNC, XDE.
		//	Select_LCD_DE_Mode();	// XVSYNC & XHSYNC in idle state.

		PCLK_Falling(); 
		//PCLK_Rising();

		VSCAN_T_to_B();
		PDATA_Set_RGB();

		HSYNC_Low_Active();
		VSYNC_Low_Active();
		DE_High_Active();
		//DE_Low_Active();

	/* 根据 OTA7001A手册 P13 设定 */
		LCD_HorizontalWidth_VerticalHeight(g_LcdWidth,g_LcdHeight);//INNOLUX 800x480

		LCD_Horizontal_Non_Display(88);	//INNOLUX800x480	88
		LCD_HSYNC_Start_Position(40);	//INNOLUX800x48040
		LCD_HSYNC_Pulse_Width(48);		//INNOLUX800x4801~48

		LCD_Vertical_Non_Display(32);	//INNOLUX800x480  32
		LCD_VSYNC_Start_Position(13);	//INNOLUX800x48013
		LCD_VSYNC_Pulse_Width(3);		//INNOLUX800x4803
	}
	else if (g_LcdType == LCD_101_1024X600)		/* INCH101 */
	{
		//**[10h]**//
		Select_LCD_Sync_Mode();	// Enable XVSYNC, XHSYNC, XDE.
		//	Select_LCD_DE_Mode();	// XVSYNC & XHSYNC in idle state.

		PCLK_Falling(); 
		//PCLK_Rising();

		VSCAN_T_to_B();
		PDATA_Set_RGB();

		HSYNC_Low_Active();
		VSYNC_Low_Active();
		DE_High_Active();
		//DE_Low_Active();

	/* 根据 10.1寸LCD手册 P11 设定 */
		LCD_HorizontalWidth_VerticalHeight(g_LcdWidth,g_LcdHeight);//INNOLUX 800x480

		LCD_Horizontal_Non_Display(160);	/* HS Blanking      -  160  -  */
		LCD_HSYNC_Start_Position(160);		/* HS Front Porch   16 160 216 */
		LCD_HSYNC_Pulse_Width(48);			/* HS pulse width    1  -  140 */

		LCD_Vertical_Non_Display(23);		/* VS Blanking 		 -  23  -  */
		LCD_VSYNC_Start_Position(12);		/* VS Front Porch    1  12 127 */
		LCD_VSYNC_Pulse_Width(10);			/* VS pulse width    1  -  20  */
	}

#ifdef EJ080NA_04B	//##INNOLUX_1024x768_EJ080NA-04B

//**[03h]**//
	LVDS_Format1();//(VESA format)//only for RA8877
//	LVDS_Format2();//(JEIDA format) //only for RA8877

//**[10h]**//
	Select_LCD_Sync_Mode();	// Enable XVSYNC, XHSYNC, XDE.
//	Select_LCD_DE_Mode();	// XVSYNC & XHSYNC in idle state.

//**[12h]**//
//	PCLK_Rising();
	PCLK_Falling();

	VSCAN_T_to_B();
//	VSCAN_B_to_T();

	PDATA_Set_RGB();
//	PDATA_Set_RBG();
//	PDATA_Set_GRB();
//	PDATA_Set_GBR();
//	PDATA_Set_BRG();
//	PDATA_Set_BGR();
//	PDATA_Set_Gray();
//	PDATA_IDLE_STATE();

//**[13h]**//
	HSYNC_Low_Active();
//	HSYNC_High_Active();

	VSYNC_Low_Active();
//	VSYNC_High_Active();
	
//	DE_Low_Active();
	DE_High_Active();

	//**[14h][15h][1Ah][1Bh]**//
	LCD_HorizontalWidth_VerticalHeight(1024,768);// INNOLUX 1024x768

/*	[16h][17h] : Figure 19-3 [HND] 
	[18h] :		 Figure 19-3 [HST] 
	[19h] :		 Figure 19-3 [HPW]	*/
	LCD_Horizontal_Non_Display(100);	//Non Display or Back-Porch //Blank=90~376, TYP.=320
	LCD_HSYNC_Start_Position(100);		//Start Position or Front-Porch
	LCD_HSYNC_Pulse_Width(120);			//Pulse Width

/*	[1Ch][1Dh] : Figure 19-3 [VND]
	[1Eh] :		 Figure 19-3 [VST] 
	[1Fh] :		 Figure 19-3 [VPW]	*/
	LCD_Vertical_Non_Display(10);		//Non Display or Back-Porch  //Blank=10~77, TYP.=38
	LCD_VSYNC_Start_Position(10);		//Start Position or Front-Porch
	LCD_VSYNC_Pulse_Width(18);			//Pulse Width

#endif

#ifdef LQ150X1LGN2C	//##SHARP_LQ150X1LGN2C_LVDS1 & LVDS2

//**[03h]**//
	LVDS_Format1();//(VESA format)//only for RA8877
//	LVDS_Format2();//(JEIDA format) //only for RA8877

//**[10h]**//
	Select_LCD_Sync_Mode();	// Enable XVSYNC, XHSYNC, XDE.
//	Select_LCD_DE_Mode();	// XVSYNC & XHSYNC in idle state.

//**[12h]**//
//	PCLK_Rising();
	PCLK_Falling();

	VSCAN_T_to_B();
//	VSCAN_B_to_T();

	PDATA_Set_RGB();
//	PDATA_Set_RBG();
//	PDATA_Set_GRB();
//	PDATA_Set_GBR();
//	PDATA_Set_BRG();
//	PDATA_Set_BGR();
//	PDATA_Set_Gray();
//	PDATA_IDLE_STATE();

//**[13h]**//
	HSYNC_Low_Active();
//	HSYNC_High_Active();

	VSYNC_Low_Active();
//	VSYNC_High_Active();
	
//	DE_Low_Active();
	DE_High_Active();

//**[14h][15h][1Ah][1Bh]**//
	LCD_HorizontalWidth_VerticalHeight(1024,768);//SHARP1024x768
//**[16h][17h]**//
	LCD_Horizontal_Non_Display(30);//SHARP1024x768Blank=32~320~696
//**[18h]**//
	LCD_HSYNC_Start_Position(10);//SHARP1024x768
//**[19h]**//
	LCD_HSYNC_Pulse_Width(10);//SHARP1024x768
//**[1Ch][1Dh]**//
	LCD_Vertical_Non_Display(20);//SHARP1024x768Blank=5~38~222
//**[1Eh]**//
	LCD_VSYNC_Start_Position(10);//SHARP1024x768
//**[1Fh]**//
	LCD_VSYNC_Pulse_Width(10);//SHARP1024x768
#endif

#ifdef N070ICG_LD1	//##INNOLUX_1280x800_N070ICG-LD1

//**[03h]**//
	LVDS_Format1();//(VESA format)//only for RA8877
//	LVDS_Format2();//(JEIDA format) //only for RA8877

//**[10h]**//
	Select_LCD_Sync_Mode();	// Enable XVSYNC, XHSYNC, XDE.
	Select_LCD_DE_Mode();	// XVSYNC & XHSYNC in idle state.


//**[12h]**//
//	PCLK_Rising();
	PCLK_Falling();

	VSCAN_T_to_B();
//	VSCAN_B_to_T();

	PDATA_Set_RGB();
//	PDATA_Set_RBG();
//	PDATA_Set_GRB();
//	PDATA_Set_GBR();
//	PDATA_Set_BRG();
//	PDATA_Set_BGR();
//	PDATA_Set_Gray();
//	PDATA_IDLE_STATE();

//**[13h]**//
	HSYNC_Low_Active();
//	HSYNC_High_Active();

	VSYNC_Low_Active();
//	VSYNC_High_Active();
	
//	DE_Low_Active();
	DE_High_Active();

//**[14h][15h][1Ah][1Bh]**//
	LCD_HorizontalWidth_VerticalHeight(1280,800);//
//**[16h][17h]**//
	LCD_Horizontal_Non_Display(50);//
//**[18h]**//
	LCD_HSYNC_Start_Position(50);//
//**[19h]**//
	LCD_HSYNC_Pulse_Width(60);//
//**[1Ch][1Dh]**//
	LCD_Vertical_Non_Display(8);//
//**[1Eh]**//
	LCD_VSYNC_Start_Position(8);//
//**[1Fh]**//
	LCD_VSYNC_Pulse_Width(8);//	


#endif

#ifdef B116XW03_V0	//##AUO1366x768_B116XW03 V0

//**[03h]**//
	LVDS_Format1();//(VESA format)//only for RA8877
//	LVDS_Format2();//(JEIDA format) //only for RA8877

//**[10h]**//
	Select_LCD_Sync_Mode();	// Enable XVSYNC, XHSYNC, XDE.
	Select_LCD_DE_Mode();	// XVSYNC & XHSYNC in idle state.

//**[12h]**//
//	PCLK_Rising();
	PCLK_Falling();

	VSCAN_T_to_B();
//	VSCAN_B_to_T();

	PDATA_Set_RGB();
//	PDATA_Set_RBG();
//	PDATA_Set_GRB();
//	PDATA_Set_GBR();
//	PDATA_Set_BRG();
//	PDATA_Set_BGR();
//	PDATA_Set_Gray();
//	PDATA_IDLE_STATE();

//**[13h]**//
//	HSYNC_Low_Active();
//	HSYNC_High_Active();

//	VSYNC_Low_Active();
//	VSYNC_High_Active();
	
//	DE_Low_Active();
	DE_High_Active();

//**[14h][15h][1Ah][1Bh]**//
	LCD_HorizontalWidth_VerticalHeight(1366,768);//
//**[16h][17h]**//
	LCD_Horizontal_Non_Display(15);//
//**[18h]**//
	LCD_HSYNC_Start_Position(15);//
//**[19h]**//
	LCD_HSYNC_Pulse_Width(44);//
//**[1Ch][1Dh]**//
	LCD_Vertical_Non_Display(8);//
//**[1Eh]**//
	LCD_VSYNC_Start_Position(8);//
//**[1Fh]**//
	LCD_VSYNC_Pulse_Width(8);//
#endif

#ifdef LQ121S1LG81	//##SHARP_LQ121S1LG81

//**[03h]**//
	LVDS_Format1();//(VESA format)//only for RA8877
//	LVDS_Format2();//(JEIDA format) //only for RA8877

//**[10h]**//
	Select_LCD_Sync_Mode();	// Enable XVSYNC, XHSYNC, XDE.
//	Select_LCD_DE_Mode();	// XVSYNC & XHSYNC in idle state.

//**[12h]**//
//	PCLK_Rising();
	PCLK_Falling();

	VSCAN_T_to_B();
//	VSCAN_B_to_T();

	PDATA_Set_RGB();
//	PDATA_Set_RBG();
//	PDATA_Set_GRB();
//	PDATA_Set_GBR();
//	PDATA_Set_BRG();
//	PDATA_Set_BGR();
//	PDATA_Set_Gray();
//	PDATA_IDLE_STATE();

//**[13h]**//
	HSYNC_Low_Active();
//	HSYNC_High_Active();

	VSYNC_Low_Active();
//	VSYNC_High_Active();
	
//	DE_Low_Active();
	DE_High_Active();

//**[14h][15h][1Ah][1Bh]**//
	LCD_HorizontalWidth_VerticalHeight(800,600);//
//**[16h][17h]**//
	LCD_Horizontal_Non_Display(88);//
//**[18h]**//
	LCD_HSYNC_Start_Position(40);//
//**[19h]**//
	LCD_HSYNC_Pulse_Width(128);//
//**[1Ch][1Dh]**//
	LCD_Vertical_Non_Display(23);//
//**[1Eh]**//
	LCD_VSYNC_Start_Position(39);//
//**[1Fh]**//
	LCD_VSYNC_Pulse_Width(4);//
	
#endif


#ifdef LQ035NC111
	//**[10h]**//
		Select_LCD_Sync_Mode();	// Enable XVSYNC, XHSYNC, XDE.
	//	Select_LCD_DE_Mode();	// XVSYNC & XHSYNC in idle state.

	//PCLK_Falling();
	PCLK_Rising();

	VSCAN_T_to_B();
	PDATA_Set_RGB();


	//HSYNC_High_Active();
	//VSYNC_High_Active();

	HSYNC_Low_Active();
	VSYNC_Low_Active();
	//DE_High_Active();
	DE_Low_Active();

	LCD_HorizontalWidth_VerticalHeight(320,240);//

	LCD_Horizontal_Non_Display(44);//
	LCD_HSYNC_Start_Position(20);//
	LCD_HSYNC_Pulse_Width(24);//	non_display + pulse width = 68

	LCD_Vertical_Non_Display(15);//
	LCD_VSYNC_Start_Position(2);//
	LCD_VSYNC_Pulse_Width(1);//


	Select_Main_Window_16bpp();
	Main_Image_Start_Address(0);				
	Main_Image_Width(320);							
	Main_Window_Start_XY(0,0);	
	Canvas_Image_Start_address(0);
	Canvas_image_width(320);//
	Active_Window_XY(0,0);
	Active_Window_WH(320,240);
#endif


#ifdef AT080TN52

	//**[10h]**//
		Select_LCD_Sync_Mode();	// Enable XVSYNC, XHSYNC, XDE.
	//	Select_LCD_DE_Mode();	// XVSYNC & XHSYNC in idle state.

	PCLK_Falling();
	//PCLK_Rising();

	VSCAN_T_to_B();
	PDATA_Set_RGB();

	HSYNC_Low_Active();
	VSYNC_Low_Active();
	DE_High_Active();
	//DE_Low_Active();

	LCD_HorizontalWidth_VerticalHeight(800,600);//INNOLUX 800x600
	LCD_Horizontal_Non_Display(46);//INNOLUX800x60046
	LCD_HSYNC_Start_Position(16);//INNOLUX800x60016~354
	LCD_HSYNC_Pulse_Width(3);//INNOLUX800x6001~40
	LCD_Vertical_Non_Display(23);//INNOLUX800x60023
	LCD_VSYNC_Start_Position(3);//INNOLUX800x6001~77
	LCD_VSYNC_Pulse_Width(3);//INNOLUX800x6001~20
#endif

#ifdef AT070TN92

//**[10h]**//
	Select_LCD_Sync_Mode();	// Enable XVSYNC, XHSYNC, XDE.
//	Select_LCD_DE_Mode();	// XVSYNC & XHSYNC in idle state.

	PCLK_Falling();
	//PCLK_Rising();

	VSCAN_T_to_B();
	PDATA_Set_RGB();

	HSYNC_Low_Active();
	VSYNC_Low_Active();
	DE_High_Active();
	//DE_Low_Active();

	LCD_HorizontalWidth_VerticalHeight(800,480);//INNOLUX 800x480
	LCD_Horizontal_Non_Display(46);//INNOLUX800x60046


	LCD_HSYNC_Start_Position(210);//INNOLUX800x60016~354
	LCD_HSYNC_Pulse_Width(10);//INNOLUX800x6001~40
	LCD_Vertical_Non_Display(23);//INNOLUX800x60023
	LCD_VSYNC_Start_Position(22);//INNOLUX800x6001~147
	LCD_VSYNC_Pulse_Width(10);//INNOLUX800x6001~20
#endif

#ifdef ET101000DM6	//##EDT_1024x600_ET101000DM6

//**[03h]**//
	LVDS_Format1();//(VESA format)//only for RA8877
//	LVDS_Format2();//(JEIDA format) //only for RA8877

//**[10h]**//
	Select_LCD_Sync_Mode();	// Enable XVSYNC, XHSYNC, XDE.
//	Select_LCD_DE_Mode();	// XVSYNC & XHSYNC in idle state.

//**[12h]**//
//	PCLK_Rising();
	PCLK_Falling();

	VSCAN_T_to_B();
//	VSCAN_B_to_T();

	PDATA_Set_RGB();
//	PDATA_Set_RBG();
//	PDATA_Set_GRB();
//	PDATA_Set_GBR();
//	PDATA_Set_BRG();
//	PDATA_Set_BGR();
//	PDATA_Set_Gray();
//	PDATA_IDLE_STATE();

//**[13h]**//
	HSYNC_Low_Active();
//	HSYNC_High_Active();

	VSYNC_Low_Active();
//	VSYNC_High_Active();
	
//	DE_Low_Active();
	DE_High_Active();

//**[14h][15h][1Ah][1Bh]**//
	LCD_HorizontalWidth_VerticalHeight(1024,600);//EDT_1024x600
//**[16h][17h]**//
	LCD_Horizontal_Non_Display(160);//EDT_1024x600Blank=90~376@320
//**[18h]**//
	LCD_HSYNC_Start_Position(160);//EDT_1024x600Blank=90~376@320
//**[19h]**//
	LCD_HSYNC_Pulse_Width(60);//EDT_1024x600Blank=90~376@320
//**[1Ch][1Dh]**//
	LCD_Vertical_Non_Display(23);//EDT_1024x600Blank=10~200@35
//**[1Eh]**//
	LCD_VSYNC_Start_Position(12);//EDT_1024x600Blank=10~200@35
//**[1Fh]**//
	LCD_VSYNC_Pulse_Width(5);//EDT_1024x600Blank=10~200@35	
	
#endif


#ifdef G190SVT01	//##AUO_1680x342_G190SVT01

//**[10h]**//
	Select_LCD_Sync_Mode();	// Enable XVSYNC, XHSYNC, XDE.
//	Select_LCD_DE_Mode();	// XVSYNC & XHSYNC in idle state.

//**[12h]**//
//	PCLK_Rising();
	PCLK_Falling();

	VSCAN_T_to_B();
//	VSCAN_B_to_T();

	PDATA_Set_RGB();
//	PDATA_Set_RBG();
//	PDATA_Set_GRB();
//	PDATA_Set_GBR();
//	PDATA_Set_BRG();
//	PDATA_Set_BGR();
//	PDATA_Set_Gray();
//	PDATA_IDLE_STATE();

//**[13h]**//
	HSYNC_Low_Active();
//	HSYNC_High_Active();

	VSYNC_Low_Active();
//	VSYNC_High_Active();
	
//	DE_Low_Active();
	DE_High_Active();

//**[14h][15h][1Ah][1Bh]**//
	LCD_HorizontalWidth_VerticalHeight(1680,350);//1680x342
//**[16h][17h]**//
	LCD_Horizontal_Non_Display(30);//Blank=288
//**[18h]**//
	LCD_HSYNC_Start_Position(30);//
//**[19h]**//
	LCD_HSYNC_Pulse_Width(30);//
//**[1Ch][1Dh]**//
	LCD_Vertical_Non_Display(6);//Blank=16
//**[1Eh]**//
	LCD_VSYNC_Start_Position(6);//
//**[1Fh]**//
	LCD_VSYNC_Pulse_Width(6);//	    
	
#endif


#ifdef ZJ070NA_01B	//##INNOLUX_1024x600_ZJ070NA_01B

//**[03h]**//
	LVDS_Format1();//(VESA format)//only for RA8877
//	LVDS_Format2();//(JEIDA format) //only for RA8877

//**[10h]**//
	Select_LCD_Sync_Mode();	// Enable XVSYNC, XHSYNC, XDE.
//	Select_LCD_DE_Mode();	// XVSYNC & XHSYNC in idle state.

//**[12h]**//
//	PCLK_Rising();
	PCLK_Falling();

	VSCAN_T_to_B();
//	VSCAN_B_to_T();

	PDATA_Set_RGB();
//	PDATA_Set_RBG();
//	PDATA_Set_GRB();
//	PDATA_Set_GBR();
//	PDATA_Set_BRG();
//	PDATA_Set_BGR();
//	PDATA_Set_Gray();
//	PDATA_IDLE_STATE();

//**[13h]**//
	HSYNC_Low_Active();
//	HSYNC_High_Active();

	VSYNC_Low_Active();
//	VSYNC_High_Active();
	
//	DE_Low_Active();
	DE_High_Active();

//**[14h][15h][1Ah][1Bh]**//
	LCD_HorizontalWidth_VerticalHeight(1024,600);//1024x600
//**[16h][17h]**//
	LCD_Horizontal_Non_Display(50);//Blank=320
//**[18h]**//
	LCD_HSYNC_Start_Position(50);//
//**[19h]**//
	LCD_HSYNC_Pulse_Width(220);//
//**[1Ch][1Dh]**//
	LCD_Vertical_Non_Display(8);//Blank=35
//**[1Eh]**//
	LCD_VSYNC_Start_Position(7);//
//**[1Fh]**//
	LCD_VSYNC_Pulse_Width(20);//		    
	
#endif

#ifdef LQ190E1LW52	//##SHARP_1280x1024_LQ190E1LW52

//**[10h]**//
	Select_LCD_Sync_Mode();	// Enable XVSYNC, XHSYNC, XDE.
//	Select_LCD_DE_Mode();	// XVSYNC & XHSYNC in idle state.

//**[12h]**//
//	PCLK_Rising();
	PCLK_Falling();

	VSCAN_T_to_B();
//	VSCAN_B_to_T();

	PDATA_Set_RGB();
//	PDATA_Set_RBG();
//	PDATA_Set_GRB();
//	PDATA_Set_GBR();
//	PDATA_Set_BRG();
//	PDATA_Set_BGR();
//	PDATA_Set_Gray();
//	PDATA_IDLE_STATE();

//**[13h]**//
	HSYNC_Low_Active();
//	HSYNC_High_Active();

	VSYNC_Low_Active();
//	VSYNC_High_Active();
	
//	DE_Low_Active();
	DE_High_Active();

//**[14h][15h][1Ah][1Bh]**//
	LCD_HorizontalWidth_VerticalHeight(1280,1024);//1024x600
//**[16h][17h]**//
	LCD_Horizontal_Non_Display(110);//Blank=204x2
//**[18h]**//
	LCD_HSYNC_Start_Position(110);//
//**[19h]**//
	LCD_HSYNC_Pulse_Width(110);//
//**[1Ch][1Dh]**//
	LCD_Vertical_Non_Display(15);//Blank=42
//**[1Eh]**//
	LCD_VSYNC_Start_Position(15);//
//**[1Fh]**//
	LCD_VSYNC_Pulse_Width(12);//	    
	
#endif	

#ifdef HJ070IA_02F	//##INNOLUX_1280x800_HJ070IA_02F

//**[03h]**//
	LVDS_Format1();//(VESA format)//only for RA8877
//	LVDS_Format2();//(JEIDA format) //only for RA8877

//**[10h]**//
	Select_LCD_Sync_Mode();	// Enable XVSYNC, XHSYNC, XDE.
//	Select_LCD_DE_Mode();	// XVSYNC & XHSYNC in idle state.

//**[12h]**//
//	PCLK_Rising();
	PCLK_Falling();

	VSCAN_T_to_B();
//	VSCAN_B_to_T();

	PDATA_Set_RGB();
//	PDATA_Set_RBG();
//	PDATA_Set_GRB();
//	PDATA_Set_GBR();
//	PDATA_Set_BRG();
//	PDATA_Set_BGR();
//	PDATA_Set_Gray();
//	PDATA_IDLE_STATE();

//**[13h]**//
	HSYNC_Low_Active();
//	HSYNC_High_Active();

	VSYNC_Low_Active();
//	VSYNC_High_Active();
	
//	DE_Low_Active();
	DE_High_Active();

//**[14h][15h][1Ah][1Bh]**//
	LCD_HorizontalWidth_VerticalHeight(1280,800);//1280x800
//**[16h][17h]**//
	LCD_Horizontal_Non_Display(30);//Blank=160
//**[18h]**//
	LCD_HSYNC_Start_Position(30);//
//**[19h]**//
	LCD_HSYNC_Pulse_Width(100);//
//**[1Ch][1Dh]**//
	LCD_Vertical_Non_Display(3);//Blank=23
//**[1Eh]**//
	LCD_VSYNC_Start_Position(3);//
//**[1Fh]**//
	LCD_VSYNC_Pulse_Width(18);//	    
#endif


#ifdef LQ156M1LG21	//##SHARP_1920x1080_LQ156M1LG21

//**[03h]**//
	LVDS_Format1();//(VESA format)//only for RA8877
//	LVDS_Format2();//(JEIDA format) //only for RA8877

//**[10h]**//
	Select_LCD_Sync_Mode();	// Enable XVSYNC, XHSYNC, XDE.
//	Select_LCD_DE_Mode();	// XVSYNC & XHSYNC in idle state.

//**[12h]**//
//	PCLK_Rising();
	PCLK_Falling();

	VSCAN_T_to_B();
//	VSCAN_B_to_T();

	PDATA_Set_RGB();
//	PDATA_Set_RBG();
//	PDATA_Set_GRB();
//	PDATA_Set_GBR();
//	PDATA_Set_BRG();
//	PDATA_Set_BGR();
//	PDATA_Set_Gray();
//	PDATA_IDLE_STATE();

//**[13h]**//
//	HSYNC_Low_Active();
	HSYNC_High_Active();

//	VSYNC_Low_Active();
	VSYNC_High_Active();
	
//	DE_Low_Active();
	DE_High_Active();

//**[14h][15h][1Ah][1Bh]**//
	LCD_HorizontalWidth_VerticalHeight(1920,1080);//1280x800
//**[16h][17h]**//
	LCD_Horizontal_Non_Display(40);//Blank min.=140
//**[18h]**//
	LCD_HSYNC_Start_Position(20);//
//**[19h]**//
	LCD_HSYNC_Pulse_Width(100);//
//**[1Ch][1Dh]**//
	LCD_Vertical_Non_Display(3);//Blank min.=31
//**[1Eh]**//
	LCD_VSYNC_Start_Position(3);//
//**[1Fh]**//
	LCD_VSYNC_Pulse_Width(20);//	    
#endif

#ifdef LQ201U1LW32	//##SHARP_1600x1200_LQ201U1LW32

//**[03h]**//
	LVDS_Format1();//(VESA format)//only for RA8877
//	LVDS_Format2();//(JEIDA format) //only for RA8877

//**[10h]**//
	Select_LCD_Sync_Mode();	// Enable XVSYNC, XHSYNC, XDE.
//	Select_LCD_DE_Mode();	// XVSYNC & XHSYNC in idle state.

//**[12h]**//
//	PCLK_Rising();
	PCLK_Falling();

	VSCAN_T_to_B();
//	VSCAN_B_to_T();

	PDATA_Set_RGB();
//	PDATA_Set_RBG();
//	PDATA_Set_GRB();
//	PDATA_Set_GBR();
//	PDATA_Set_BRG();
//	PDATA_Set_BGR();
//	PDATA_Set_Gray();
//	PDATA_IDLE_STATE();

//**[13h]**//
//	HSYNC_Low_Active();
	HSYNC_High_Active();

//	VSYNC_Low_Active();
	VSYNC_High_Active();
	
//	DE_Low_Active();
	DE_High_Active();

//**[14h][15h][1Ah][1Bh]**//
	LCD_HorizontalWidth_VerticalHeight(1600,1200);//
//**[16h][17h]**//
	LCD_Horizontal_Non_Display(15);
//**[18h]**//
	LCD_HSYNC_Start_Position(15);
//**[19h]**//
	LCD_HSYNC_Pulse_Width(40);
//**[1Ch][1Dh]**//
	LCD_Vertical_Non_Display(5);
//**[1Eh]**//
	LCD_VSYNC_Start_Position(5);
//**[1Fh]**//
	LCD_VSYNC_Pulse_Width(5);
#endif
}



/*
*********************************************************************************************************
*	函 数 名: Set_Serial_Flash_IF
*	功能说明: 设置串行flash
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void Set_Serial_Flash_IF(void)
{

//(*) Using Serial Flash
	#ifdef MX25L25635E
		Enable_SFlash_SPI();
	#endif

//(*) Using GT_ROM
	#ifdef GT21L16T1W
		GTFont_Select_GT21L16T1W();
		Enable_SFlash_SPI();
	#endif
	#ifdef GT30L16U2W
		GTFont_Select_GT30L16U2W();
		Enable_SFlash_SPI();
	#endif
	#ifdef GT30L24T3Y
		GTFont_Select_GT30L24T3Y();
		Enable_SFlash_SPI();
	#endif
	#ifdef GT30L24M1Z	
		GTFont_Select_GT30L24M1Z();
		Enable_SFlash_SPI();
	#endif
	#ifdef GT30L32S4W
		GTFont_Select_GT30L32S4W();
		Enable_SFlash_SPI();
	#endif
	#ifdef GT20L24F6Y
		GTFont_Select_GT20L24F6Y();
		Enable_SFlash_SPI();
	#endif
	#ifdef GT21L24S1W
		GTFont_Select_GT21L24S1W();
		Enable_SFlash_SPI();
	#endif

//(*) Set RA8876/77 Master SPI address width. 
	Select_SFI_24bit_Address();				/* 串行Flash配置为24bit寻址模式 */
	//Select_SFI_32bit_Address();
	
//(*)	
	Select_standard_SPI_Mode0_or_Mode3();	/* RA8876兼容模式，标准SPI模式0或模式3时序图 */
//	Select_RA8875_SPI_Mode0_and_Mode3();
	
//(*)
//	Select_SFI_Single_Mode_Dummy_0T_03h();	/* 无空周期 */
//	Select_SFI_Single_Mode_Dummy_8T_0Bh();	/* 数据是由 xmiso输入， RA8876 在地址与数据间会塞入 8 个空周期 */
//	Select_SFI_Single_Mode_Dummy_16T_1Bh();
	Select_SFI_Dual_Mode_Dummy_8T_3Bh();	/* 双线模式0,增加刷图速度 */
//	Select_SFI_Dual_Mode_Dummy_4T_BBh();

//(*)
/*
1. At CPOL=0， SCK 频率在未动作时为 0。
	o For CPHA=0，数据是在频率的上升缘读取(low->high)，并且数据是在下降缘(high->low)变化。
	o For CPHA=1，数据是在频率的下升缘读取(high->low)，并且数据是在上降缘变化(low->high)。
2 At CPOL=1 ， SCK 频率再未动作时为 1(与 CPOL=0 反相)。
	o For CPHA=0, 数据是在频率的下升缘读取(high->low)，并且数据是在上降缘变化(low->high)。
	o For CPHA=1, 数据是在频率的上升缘读取(low->high)，并且数据是在下降缘(high->low)变化。
*/
	Reset_CPOL();				/* 将CPOL时钟极性位置0 */
//	Set_CPOL();
	Reset_CPHA();				/* 将CPHA时钟相位置0 */
//	Set_CPHA();

/******************************/
}



/*
*********************************************************************************************************
*	函 数 名: RA8876_SDRAM_initial
*	功能说明: SDRAM初始化函数
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/ 
void RA8876_SDRAM_initial(void)
{
uint8_t	CAS_Latency;
uint16_t	Auto_Refresh;

/*
	Ex. 如果 SDRAM 频率是 100MHz，SDRAM 的刷新周期 Tref 是
	64ms，并且 row size 为 8192，那么内部刷新时间应该是小于
	64e-3 / 8192 * 100e6 ~= 781 = 30Dh，因此此缓存器[E2h][E3h]
	就是设定 30Dh
*/	
	
#ifdef HY57V641620F
	CAS_Latency = 2;
	Auto_Refresh = (64 * DRAM_FREQ * 1000) / (4096);
	
	// 行地址 A0-A11; 列地址A0-A7
	LCD_RegisterWrite(0xe0, (0 << 7) | (0 << 6) | (1 << 5) | (1 << 3) | (0 << 0));	/* 0x28 */
	
	LCD_RegisterWrite(0xe1, CAS_Latency);      //CAS:2=0x02CAS:3=0x03
	LCD_RegisterWrite(0xe2, Auto_Refresh);
	LCD_RegisterWrite(0xe3, Auto_Refresh >> 8);
	LCD_RegisterWrite(0xe4, 0x01);		//0x01
#endif

#ifdef IS42SM16160D

	if(DRAM_FREQ<=133)	CAS_Latency=2;
	else 				CAS_Latency=3;

	Auto_Refresh=(64*DRAM_FREQ*1000)/(8192);
	
	LCD_RegisterWrite(0xe0,0xf9);        
	LCD_RegisterWrite(0xe1,CAS_Latency);      //CAS:2=0x02CAS:3=0x03
	LCD_RegisterWrite(0xe2,Auto_Refresh);
	LCD_RegisterWrite(0xe3,Auto_Refresh>>8);
	LCD_RegisterWrite(0xe4,0x09);
#endif

#ifdef IS42S16320B
	
	if(DRAM_FREQ<=133)	CAS_Latency=2;
	else 				CAS_Latency=3;
	
	Auto_Refresh=(64*DRAM_FREQ*1000)/(8192);
	
	LCD_RegisterWrite(0xe0,0x32);	
	LCD_RegisterWrite(0xe1,CAS_Latency);
	LCD_RegisterWrite(0xe2,Auto_Refresh);
	LCD_RegisterWrite(0xe3,Auto_Refresh>>8);
	LCD_RegisterWrite(0xe4,0x09);

#endif

#ifdef IS42S16400F

	if(DRAM_FREQ<143)	CAS_Latency=2;
	else 				CAS_Latency=3;

	Auto_Refresh=(64*DRAM_FREQ*1000)/(4096);
	
	LCD_RegisterWrite(0xe0,0x28);      
	LCD_RegisterWrite(0xe1,CAS_Latency);      //CAS:2=0x02CAS:3=0x03
	LCD_RegisterWrite(0xe2,Auto_Refresh);
	LCD_RegisterWrite(0xe3,Auto_Refresh>>8);
	LCD_RegisterWrite(0xe4,0x01);
#endif

#ifdef M12L32162A
	CAS_Latency=3;
	Auto_Refresh=(64*DRAM_FREQ*1000)/(4096);
	
	LCD_RegisterWrite(0xe0,0x08);      
	LCD_RegisterWrite(0xe1,CAS_Latency);      //CAS:2=0x02CAS:3=0x03
	LCD_RegisterWrite(0xe2,Auto_Refresh);
	LCD_RegisterWrite(0xe3,Auto_Refresh>>8);
	LCD_RegisterWrite(0xe4,0x09);
#endif

#ifdef M12L2561616A
	CAS_Latency=3;	
	Auto_Refresh=(64*DRAM_FREQ*1000)/(8192);
	
	LCD_RegisterWrite(0xe0,0x31);      
	LCD_RegisterWrite(0xe1,CAS_Latency);      //CAS:2=0x02CAS:3=0x03
	LCD_RegisterWrite(0xe2,Auto_Refresh);
	LCD_RegisterWrite(0xe3,Auto_Refresh>>8);
	LCD_RegisterWrite(0xe4,0x01);
#endif

#ifdef M12L64164A
	CAS_Latency=3;
	Auto_Refresh=(64*DRAM_FREQ*1000)/(4096);
	
	LCD_RegisterWrite(0xe0,0x28);      
	LCD_RegisterWrite(0xe1,CAS_Latency);      //CAS:2=0x02CAS:3=0x03
	LCD_RegisterWrite(0xe2,Auto_Refresh);
	LCD_RegisterWrite(0xe3,Auto_Refresh>>8);
	LCD_RegisterWrite(0xe4,0x09);
#endif

#ifdef W9825G6JH
	CAS_Latency=3;
	Auto_Refresh=(64*DRAM_FREQ*1000)/(8192);
	
	LCD_RegisterWrite(0xe0,0x31);      
	LCD_RegisterWrite(0xe1,CAS_Latency);      //CAS:2=0x02CAS:3=0x03
	LCD_RegisterWrite(0xe2,Auto_Refresh);
	LCD_RegisterWrite(0xe3,Auto_Refresh>>8);
	LCD_RegisterWrite(0xe4,0x01);
#endif

#ifdef W9812G6JH
	CAS_Latency=3;
	Auto_Refresh=(64*DRAM_FREQ*1000)/(4096);
	
	LCD_RegisterWrite(0xe0,0x29);      
	LCD_RegisterWrite(0xe1,CAS_Latency);      //CAS:2=0x02CAS:3=0x03
	LCD_RegisterWrite(0xe2,Auto_Refresh);
	LCD_RegisterWrite(0xe3,Auto_Refresh>>8);
	LCD_RegisterWrite(0xe4,0x01);
#endif

#ifdef MT48LC4M16A
	CAS_Latency=3;
	Auto_Refresh=(64*DRAM_FREQ*1000)/(4096);
	
	LCD_RegisterWrite(0xe0,0x28);      
	LCD_RegisterWrite(0xe1,CAS_Latency);      //CAS:2=0x02CAS:3=0x03
	LCD_RegisterWrite(0xe2,Auto_Refresh);
	LCD_RegisterWrite(0xe3,Auto_Refresh>>8);
	LCD_RegisterWrite(0xe4,0x01);
#endif

#ifdef K4S641632N
	CAS_Latency=3;
	Auto_Refresh=(64*DRAM_FREQ*1000)/(4096);
	
	LCD_RegisterWrite(0xe0,0x28);      
	LCD_RegisterWrite(0xe1,CAS_Latency);      //CAS:2=0x02CAS:3=0x03
	LCD_RegisterWrite(0xe2,Auto_Refresh);
	LCD_RegisterWrite(0xe3,Auto_Refresh>>8);
	LCD_RegisterWrite(0xe4,0x01);
#endif

#ifdef K4S281632K
	CAS_Latency=3;
	Auto_Refresh=(64*DRAM_FREQ*1000)/(4096);
	
	LCD_RegisterWrite(0xe0,0x29);      
	LCD_RegisterWrite(0xe1,CAS_Latency);      //CAS:2=0x02CAS:3=0x03
	LCD_RegisterWrite(0xe2,Auto_Refresh);
	LCD_RegisterWrite(0xe3,Auto_Refresh>>8);
	LCD_RegisterWrite(0xe4,0x01);
#endif

#ifdef IS42S16100
	CAS_Latency=3;
	Auto_Refresh=(32*DRAM_FREQ*1000)/(2048);
	
	LCD_RegisterWrite(0xe0,0x00);      
	LCD_RegisterWrite(0xe1,CAS_Latency);      //CAS:2=0x02CAS:3=0x03
	LCD_RegisterWrite(0xe2,Auto_Refresh);
	LCD_RegisterWrite(0xe3,Auto_Refresh>>8);
	LCD_RegisterWrite(0xe4,0x01);
#endif

	Check_SDRAM_Ready();
}

/*
*********************************************************************************************************
*	函 数 名: RA8876_PLL
*	功能说明: RA8876时钟设置函数
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
/*
1. 只有在 PLL 禁能时才能修改 PLL 参数。
2. 在 REG[05h] ~ REG[0Ah] 被修改后，PLL 需要有 30us 的时间来稳定频率输出。
3. 输入 OSC 频率 F IN 与 PLLDIVM 必须符合以下条件：
4. 内部倍频 
*/
void RA8876_PLL(void) 
{
/*
(1) 10MHz <= OSC_FREQ <= 15MHz
(2) 10MHz <= (OSC_FREQ/PLLDIVM) <= 40MHz
(3) 100MHz <= [OSC_FREQ/(PLLDIVM+1)]x(PLLDIVN+1) <= 600MHz

PLLDIVM:0
PLLDIVN:1~63
PLLDIVK:CPLL & MPLL = 1/2/4/8.SPLL = 1/2/4/8/16/32/64/128.

ex:
 OSC_FREQ = 10MHz
 Set X_DIVK=2
 Set X_DIVM=0
 => (X_DIVN+1)=(XPLLx4)/10
*/

	uint16_t x_Divide,PLLC1,PLLC2;

	switch(3)
	{
	//if SCAN_FREQ >= 50MHz.
	case 0:	//(All PLL Divided by 2)
		LCD_RegisterWrite(0x05,0x02);
		LCD_RegisterWrite(0x07,0x02);
		LCD_RegisterWrite(0x09,0x02);
		LCD_RegisterWrite(0x06,(SCAN_FREQ*2/10)-1);
		LCD_RegisterWrite(0x08,(DRAM_FREQ*2/10)-1);
		LCD_RegisterWrite(0x0A,(CORE_FREQ*2/10)-1);
	break;

	//if SCAN_FREQ >= 25MHz. DRAM_FREQ <=150MHz.
	case 1:	//(All PLL Divided by 4
		LCD_RegisterWrite(0x05,0x04);
		LCD_RegisterWrite(0x07,0x04);
		LCD_RegisterWrite(0x09,0x04);
		LCD_RegisterWrite(0x06,(SCAN_FREQ*4/10)-1);
		LCD_RegisterWrite(0x08,(DRAM_FREQ*4/10)-1);
		LCD_RegisterWrite(0x0A,(CORE_FREQ*4/10)-1);
	break;

	case 2:	
		// Set pixel clock
		x_Divide=(64*OSC_FREQ) / SCAN_FREQ;
		if(  1<x_Divide)	PLLC1=0x00;
		if(  2<x_Divide)	PLLC1=0x02;
		if(  4<x_Divide)	PLLC1=0x04;
		if(  8<x_Divide)	PLLC1=0x06;
		if( 16<x_Divide)	PLLC1=0x08;
		if( 32<x_Divide)	PLLC1=0x0A;
		if( 64<x_Divide)	PLLC1=0x0C;
		if(128<x_Divide)	PLLC1=0x0E;
	
		PLLC2 = (uint32_t)(SCAN_FREQ * pow(2,(PLLC1>>1)) / OSC_FREQ )-1;
	
		LCD_RegisterWrite(0x05,PLLC1);				
		LCD_RegisterWrite(0x06,PLLC2);

		// Set SDRAM clock
		x_Divide=(64*OSC_FREQ) / DRAM_FREQ;
		if(  1<x_Divide)	PLLC1=0x00;
		if(  2<x_Divide)	PLLC1=0x02;
		if(  4<x_Divide)	PLLC1=0x04;
		if(  8<x_Divide)	PLLC1=0x06;
	
		PLLC2 = (uint32_t)(DRAM_FREQ*pow(2,(PLLC1>>1))/OSC_FREQ)-1;
	
		LCD_RegisterWrite(0x07,PLLC1);				
		LCD_RegisterWrite(0x08,PLLC2);

		// Set Core clock
		x_Divide=(64*OSC_FREQ) / CORE_FREQ;
		if(  1<x_Divide)	PLLC1=0x00;
		if(  2<x_Divide)	PLLC1=0x02;
		if(  4<x_Divide)	PLLC1=0x04;
		if(  8<x_Divide)	PLLC1=0x06;
	
		PLLC2 = (uint32_t)(CORE_FREQ*pow(2,(PLLC1>>1))/OSC_FREQ)-1;
	
		LCD_RegisterWrite(0x09,PLLC1);				
		LCD_RegisterWrite(0x0A,PLLC2);

	break;



	case 3:	
/*
	内建可程序锁相回路 PLL 以提供系统频率、LCD 扫描频率与 SDRAM 频率使用
	单一石英晶体震荡输入: (XI/XO: 10-15MHz)
	内部核心最大系统频率 (最大值 120MHz)
	SDRAM 频率 (最大值 166MHz)
	LCD 屏幕扫描频率 (最大值 100MHz)	
*/	
/*
	SCLK PLL 时钟控制寄存器1[05H]
	
bit0：
	SCLK PLLDIVM
	PCLK PLL Pre-driver parameter.
	0b: 除 1。
	1b: 除 2。
	
bit2-1：
	SCLK PLLDIVK[1:0]
	SCLK PLL 输出除频
	00b: 除 1。
	01b: 除 2。
	10b: 除 4。
	11b: 除 8。
	
bit5-3：
	SCLK extra divider
	xx1b: 除 16。
	000b: 除 1。
	010b: 除 2。
	100b: 除 4。
	110b: 除 8。
*/

/*
	SCLK PLL 时钟控制寄存器2[06H]
bit7-6
	NA
	0  RO
bit5-0
	SCLK PLLDIVN[5:0]
	SCLK PLL 输入参数，数值应该在 1~63。 (数值 0 是禁止的)。
*/
  		/* 设置像素时钟 */
		if( 50<SCAN_FREQ)					
		{
			LCD_RegisterWrite(0x05,0x02);				/* PLL除2 */
			LCD_RegisterWrite(0x06,(SCAN_FREQ*2/OSC_FREQ)-1);
		}
		if((25<SCAN_FREQ)&&(SCAN_FREQ<=50))	
		{								  	
			LCD_RegisterWrite(0x05,0x04);				/* PLL除4 */
			LCD_RegisterWrite(0x06,(SCAN_FREQ*4/OSC_FREQ)-1);
		}
		if((12<SCAN_FREQ)&&(SCAN_FREQ<=25))	
		{								  	
			LCD_RegisterWrite(0x05,0x06);				/* PLL除8 */
			LCD_RegisterWrite(0x06,(SCAN_FREQ*8/OSC_FREQ)-1);
		}
		if(( 7<SCAN_FREQ)&&(SCAN_FREQ<=12))	
		{
			LCD_RegisterWrite(0x05,0x08);				/* PLL除16 */
			LCD_RegisterWrite(0x06,(SCAN_FREQ*16/OSC_FREQ)-1);
		}
		if(SCAN_FREQ<=7)	
		{
			LCD_RegisterWrite(0x05,0x0A);				/* PLL除32 */
			LCD_RegisterWrite(0x06,(SCAN_FREQ*32/OSC_FREQ)-1);
		}								    

/*
	MCLK PLL 控制寄存器1[07H]
		
bit7-3
	NA
		
bit2-1
	MCLK PLLDIVK[1:0]
	PCLK PLL Output divider
	00b: 除 1。
	01b: 除 2。
	10b: 除 4。
	11b: 除 8。

bit0
	MCLK PLLDIVM
	MCLK PLL Pre-driver parameter.
	0b: 除 1。
	1b: 除 2
*/		
/*
	MCLK PLL 控制寄存器2[08H]
bit7-6
	NA

bit5-0
	MCLK PLLDIVN[5:0]
	MCLK PLL输入参数，数值应该在 1~63。 (数值 0 是禁止的)。
*/

		/* 设置SDRAM时钟 */
		if( 50<DRAM_FREQ)					
		{
			LCD_RegisterWrite(0x07,0x02);				//PLL Divided by 2
			LCD_RegisterWrite(0x08,(DRAM_FREQ*2/OSC_FREQ)-1);
		}
		if((25<DRAM_FREQ)&&(DRAM_FREQ<=50))	
		{
			LCD_RegisterWrite(0x07,0x04);				//PLL Divided by 4
			LCD_RegisterWrite(0x08,(DRAM_FREQ*4/OSC_FREQ)-1);
		}
		if((12<DRAM_FREQ)&&(DRAM_FREQ<=25))	
		{
			LCD_RegisterWrite(0x07,0x06);				//PLL Divided by 8
			LCD_RegisterWrite(0x08,(DRAM_FREQ*8/OSC_FREQ)-1);
		}
	
/*
	CCLK PLL控制寄存器1[09H] 
bit7-3
	NA

bit2-1
	CCLK PLLDIVK[1:0]
	CCLK PLL 输出除频
	00b: 除 1。
	01b: 除 2。
	10b: 除 4。
	11b: 除 8。

bit0
	CCLK PLLDIVM
	CCLK PLL Pre-driver parameter.
	0b: 除 1。
	1b: 除 2	
*/
/*
	CCLK PLL控制寄存器2[0AH] 
bit7-6
	NA

bit5-0
	CCLK PLLDIVN[5:0]
	CCLK PLL输入参数，数值应该在 1~63。 (数值 0 是禁止的)
*/
		/* 设置核心时钟 */
		if( 50<CORE_FREQ)					
		{
			LCD_RegisterWrite(0x09,0x02);				//PLL Divided by 2
			LCD_RegisterWrite(0x0A,(CORE_FREQ*2/OSC_FREQ)-1);
		}
		if((25<CORE_FREQ)&&(CORE_FREQ<=50))	
		{
			LCD_RegisterWrite(0x09,0x04);				//PLL Divided by 4
			LCD_RegisterWrite(0x0A,(CORE_FREQ*4/OSC_FREQ)-1);
		}
		if((12<CORE_FREQ)&&(CORE_FREQ<=25))	
		{
			LCD_RegisterWrite(0x09,0x06);				//PLL Divided by 8
			LCD_RegisterWrite(0x0A,(CORE_FREQ*8/OSC_FREQ)-1);
		}	 									
		break;
	}
	Enable_PLL();

	bsp_DelayMS(200);	//delay_ms(100);
}



/* RA8876官方API函数 */
char tmp1[32];		 

void MPU8_8bpp_Memory_Write
(
unsigned short x //x of coordinate 
,unsigned short y // y of coordinate 
,unsigned short w //width
,unsigned short h //height
,const unsigned char *data //8bit data
)
{														  
	unsigned short i,j;
	Graphic_Mode();
    Active_Window_XY(x,y);
	Active_Window_WH(w,h); 					
	Goto_Pixel_XY(x,y);
	RA8876_CmdWrite(0x04);	
for(i=0;i< h;i++)
{	
	for(j=0;j< w;j++)
 	{	   
	 Check_Mem_WR_FIFO_not_Full();
	 RA8876_DataWrite(*data);
	 data++;
	}
}
	Check_Mem_WR_FIFO_Empty();
}	 

			
void MPU8_16bpp_Memory_Write
(
unsigned short x // x of coordinate 
,unsigned short y // y of coordinate 
,unsigned short w // width
,unsigned short h // height
,const unsigned char *data // 8bit data
)
{
	unsigned short i,j;
	Graphic_Mode();
    Active_Window_XY(x,y);
	Active_Window_WH(w,h); 					
	Goto_Pixel_XY(x,y);
	RA8876_CmdWrite(0x04);
for(i=0;i< h;i++)
{	
	for(j=0;j< w;j++)
 	{
	 Check_Mem_WR_FIFO_not_Full();
	 RA8876_DataWrite(*data);
	 data++;
	 Check_Mem_WR_FIFO_not_Full();
	 RA8876_DataWrite(*data);
	 data++;
	}
}
	Check_Mem_WR_FIFO_Empty();
}		 


void MPU8_24bpp_Memory_Write 
(
unsigned short x // x of coordinate
,unsigned short y // y of coordinate 
,unsigned short w // width
,unsigned short h // height
,const unsigned char *data // 8bit data
)

{
	unsigned short i,j;
	Graphic_Mode();
    Active_Window_XY(x,y);
	Active_Window_WH(w,h); 					
	Goto_Pixel_XY(x,y);
	RA8876_CmdWrite(0x04);
for(i=0;i< h;i++)
{	
	for(j=0;j< w;j++)
 	{
	 Check_Mem_WR_FIFO_not_Full();
	 RA8876_DataWrite(*data);
	 data++;
	 Check_Mem_WR_FIFO_not_Full();
	 RA8876_DataWrite(*data);
	 data++;
	 Check_Mem_WR_FIFO_not_Full();
	 RA8876_DataWrite(*data);
	 data++;
	}
}
	Check_Mem_WR_FIFO_Empty();
}


/*
*********************************************************************************************************
*	函 数 名: MPU16_16bpp_Memory_Write
*	功能说明: 描点的方法将数据写入内存
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void MPU16_16bpp_Memory_Write 
(
unsigned short x //x of coordinate
,unsigned short y //y of coordinate 
,unsigned short w //width
,unsigned short h //height
,const unsigned short *data //16bit data
)			
{
	uint32_t i;
	Graphic_Mode();
    Active_Window_XY(x,y);
	Active_Window_WH(w,h); 					
	Goto_Pixel_XY(x,y);
	RA8876_CmdWrite(0x04);

	for(i = 0; i < w * h; i++)
 	{
		//Check_Mem_WR_FIFO_not_Full();		/* 减少刷屏时间 */
		RA8876_DataWrite(*data);
		data++;
	}
	Check_Mem_WR_FIFO_Empty();
}




void MPU16_24bpp_Mode1_Memory_Write 
(
unsigned short x //x of coordinate
,unsigned short y //y of coordinate
,unsigned short w //width
,unsigned short h //height
,const unsigned short *data //16bit data
)	
{
	unsigned short i,j;
	Graphic_Mode();
    Active_Window_XY(x,y);
	Active_Window_WH(w,h); 					
	Goto_Pixel_XY(x,y);
	RA8876_CmdWrite(0x04);
for(i=0;i< h;i++)
{	
	for(j=0;j< w/2;j++)
 	{
	 RA8876_DataWrite(*data);
	 Check_Mem_WR_FIFO_not_Full();
	 data++;
	 RA8876_DataWrite(*data);
	 Check_Mem_WR_FIFO_not_Full();
	 data++;
	 RA8876_DataWrite(*data);
	 Check_Mem_WR_FIFO_not_Full();
	 data++;
	}
}
	Check_Mem_WR_FIFO_Empty();
}


void MPU16_24bpp_Mode2_Memory_Write
(
unsigned short x //x of coordinate
,unsigned short y //y of coordinate
,unsigned short w //width
,unsigned short h //height
,const unsigned short *data //16bit data
)	
{
	unsigned short i,j;
	Graphic_Mode();
    Active_Window_XY(x,y);
	Active_Window_WH(w,h); 					
	Goto_Pixel_XY(x,y);
	RA8876_CmdWrite(0x04);
for(i=0;i< h;i++)
{	
	for(j=0;j< w;j++)
 	{
	 Check_Mem_WR_FIFO_not_Full();
	 RA8876_DataWrite(*data);
	 data++;
	 Check_Mem_WR_FIFO_not_Full();
	 RA8876_DataWrite(*data);
	 data++;
	}
}
	Check_Mem_WR_FIFO_Empty();
}




/*
*********************************************************************************************************
*	函 数 名: PIP
*	功能说明: 画中画参数配置函数
*	形    参：  On_Off ：0 : disable PIP, 1 : enable PIP
*			Select_PIP ： 1 : use PIP1 , 2 : use PIP2		
*				 PAddr ： PIP起始地址
*					XP ：PIP窗口的坐标X
*					YP ：PIP窗口的坐标Y
*			ImageWidth ：PIP图片的宽度
*				 X_Dis ：显示图片的坐标X
*				 Y_Dis ：显示图片的坐标Y
*				   X_W ：显示图片的宽度
*				   Y_H ：显示图片的高度
*	返 回 值: 无
*********************************************************************************************************
*/
void PIP
(
unsigned char On_Off // 0 : disable PIP, 1 : enable PIP, 2 : To maintain the original state//使能或失能
,unsigned char Select_PIP // 1 : use PIP1 , 2 : use PIP2									//PIP1或PIP2
,unsigned long PAddr //start address of PIP													//PIP的起始地址
,unsigned short XP //coordinate X of PIP Window, It must be divided by 4.					//PIP窗口的坐标X
,unsigned short YP //coordinate Y of PIP Window, It must be divided by 4.					//PIP窗口的坐标Y
,unsigned long ImageWidth //Image Width of PIP (recommend = canvas image width)			  	//PIP图片宽度
,unsigned short X_Dis //coordinate X of Display Window									//显示图片的坐标X
,unsigned short Y_Dis //coordinate Y of Display Window									//显示图片的坐标Y
,unsigned short X_W //width of PIP and Display Window, It must be divided by 4.			//显示图片的宽度
,unsigned short Y_H //height of PIP and Display Window , It must be divided by 4.		//显示图片的高度
)
{

/*	
	PIP 窗口的参数包含 : 色深、起始地址、图像宽度、显示坐标、
窗口坐标、窗口宽度、窗口高度 
*/
	if(Select_PIP == 1 )  
	{
	Select_PIP1_Parameter();			/* 选择配置PIP1的参数 */
	}
	if(Select_PIP == 2 )  
	{
	Select_PIP2_Parameter();			/* 选择配置PIP2的参数 */
	}
	PIP_Display_Start_XY(X_Dis,Y_Dis);	/* PIP1或2窗口显示左上角的坐标 */
	PIP_Image_Start_Address(PAddr);		/* 画中画1或2图像起始地址 */
	PIP_Image_Width(ImageWidth);		/* PIP1或2图像宽度 */
	PIP_Window_Image_Start_XY(XP,YP);	/* PIP1或2窗口图片左上角的坐标 */
	PIP_Window_Width_Height(X_W,Y_H);	/* PIP1或2窗口图片宽度和高度 */


	if(On_Off == 0)
    {
  		if(Select_PIP == 1 )  
		{ 
		Disable_PIP1();	
		}
		if(Select_PIP == 2 )  
		{
		Disable_PIP2();
		}
    }

    if(On_Off == 1)
    {
  		if(Select_PIP == 1 )  
		{ 
		Enable_PIP1();	
		}
		if(Select_PIP == 2 )  
		{
		Enable_PIP2();
		}
    }
   
}

 


/*internal font
x : Print font start coordinate of X
y : Print font start coordinate of Y
X_W : active window width
Y_H : active window height
FontColor : Set Font Color
BackGroundColor : Set Font BackGround Color 
Font Color and BackGround Color dataformat : 
ColorDepth_8bpp : R3G3B2
ColorDepth_16bpp : R5G6B5
ColorDepth_24bpp : R8G8B8
tmp2 : Hex variable you want print (range : 0 ~ 32bit)
*/
void Print_Internal_Font_Hexvariable(unsigned short x,unsigned short y,unsigned short X_W,unsigned short Y_H,unsigned long FontColor,unsigned long BackGroundColor,  unsigned int tmp2)
{	
   	Text_Mode();
	CGROM_Select_Internal_CGROM();
#ifdef MCU_8bit_ColorDepth_8bpp
    Foreground_color_256(FontColor);
	Background_color_256(BackGroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_16bpp
    Foreground_color_65k(FontColor);
	Background_color_65k(BackGroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_24bpp
    Foreground_color_16M(FontColor);
	Background_color_16M(BackGroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_16bpp	
    Foreground_color_65k(FontColor);
	Background_color_65k(BackGroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_1	
    Foreground_color_16M(FontColor);
	Background_color_16M(BackGroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_2	
    Foreground_color_16M(FontColor);
	Background_color_16M(BackGroundColor);
#endif
	Active_Window_XY(x,y);
    Active_Window_WH(X_W,Y_H);
	Goto_Text_XY(x,y);
	sprintf(tmp1 ,"%X", tmp2); 
	Show_String(tmp1);
}



/*internal font
x : Print font start coordinate of X
y : Print font start coordinate of Y
X_W : active window width
Y_H : active window height
FontColor : Set Font Color
BackGroundColor : Set Font BackGround Color 
Font Color and BackGround Color dataformat :
ColorDepth_8bpp : R3G3B2
ColorDepth_16bpp : R5G6B5
ColorDepth_24bpp : R8G8B8
tmp2 : Decimalvariable you want print (range : 0 ~ 32bit)
*/
void Print_Internal_Font_Decimalvariable(unsigned short x,unsigned short y,unsigned short X_W,unsigned short Y_H,unsigned long FontColor,unsigned long BackGroundColor,unsigned int tmp2)
{	

	Text_Mode();
	CGROM_Select_Internal_CGROM();
#ifdef MCU_8bit_ColorDepth_8bpp
    Foreground_color_256(FontColor);
	Background_color_256(BackGroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_16bpp
    Foreground_color_65k(FontColor);
	Background_color_65k(BackGroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_24bpp
    Foreground_color_16M(FontColor);
	Background_color_16M(BackGroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_16bpp	
    Foreground_color_65k(FontColor);
	Background_color_65k(BackGroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_1	
    Foreground_color_16M(FontColor);
	Background_color_16M(BackGroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_2	
    Foreground_color_16M(FontColor);
	Background_color_16M(BackGroundColor);
#endif
    Active_Window_XY(x,y);
    Active_Window_WH(X_W,Y_H);
	Goto_Text_XY(x,y);
	sprintf(tmp1 ,"%d", tmp2); 
	Show_String(tmp1);
}





void Print_Internal_Font_String
(
unsigned short x //coordinate x for print string
,unsigned short y //coordinate x for print string
,unsigned short X_W //active window width
,unsigned short Y_H //active window height
,unsigned long FontColor //FontColor : Set Font Color
,unsigned long BackGroundColor 
/*BackGroundColor : Set Font BackGround Color.Font Color and BackGround Color dataformat :
ColorDepth_8bpp : R3G3B2
ColorDepth_16bpp : R5G6B5
ColorDepth_24bpp : R8G8B8*/
,char tmp2[] //tmp2 : Font String which you want print on LCD
)
{

  	Text_Mode();
	CGROM_Select_Internal_CGROM();
	Font_Select_12x24_24x24();
	#ifdef MCU_8bit_ColorDepth_8bpp	
    Foreground_color_256(FontColor);
	Background_color_256(BackGroundColor);
	#endif
	#ifdef MCU_8bit_ColorDepth_16bpp	
    Foreground_color_65k(FontColor);
	Background_color_65k(BackGroundColor);
	#endif
	#ifdef MCU_8bit_ColorDepth_24bpp	
    Foreground_color_16M(FontColor);
	Background_color_16M(BackGroundColor);
	#endif
	#ifdef MCU_16bit_ColorDepth_16bpp	
    Foreground_color_65k(FontColor);
	Background_color_65k(BackGroundColor);
	#endif
	#ifdef MCU_16bit_ColorDepth_24bpp_Mode_1	
    Foreground_color_16M(FontColor);
	Background_color_16M(BackGroundColor);
	#endif
	#ifdef MCU_16bit_ColorDepth_24bpp_Mode_2	
    Foreground_color_16M(FontColor);
	Background_color_16M(BackGroundColor);
	#endif
	Active_Window_XY(x,y);							 
	Active_Window_WH(X_W,Y_H);
	Goto_Text_XY(x,y);
//	sprintf(tmp1,"%s",tmp2); 
//	Show_String(tmp1);
	Show_String(tmp2);

}





void Print_BIG5String
(
unsigned char Clk //SPI CLK = System Clock / 2*(Clk+1)
,unsigned char SCS //0 : use CS0 , 1 : use CS1
,unsigned short x //coordinate x for print string
,unsigned short y //coordinate y for print string
,unsigned short X_W //active window width
,unsigned short Y_H //active window height
,unsigned long FontColor //Set Font Color
,unsigned long BackGroundColor //Set Font BackGround Color 
/*Font Color and BackGround Color dataformat :
ColorDepth_8bpp : R3G3B2
ColorDepth_16bpp : R5G6B5
ColorDepth_24bpp : R8G8B8*/
,char *tmp2 //tmp2 : BIG5 Font String which you want print on LCD
)
{

  Select_SFI_Font_Mode();
  CGROM_Select_Genitop_FontROM();
  SPI_Clock_Period(Clk);
  Set_GTFont_Decoder(0x11);      
  if(SCS == 0)
  { 
  Select_SFI_0();
  }
  if(SCS == 1)
  {
  Select_SFI_1();
  }

#ifdef MCU_8bit_ColorDepth_8bpp	
  Foreground_color_256(FontColor);
  Background_color_256(BackGroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_16bpp	
  Foreground_color_65k(FontColor);
  Background_color_65k(BackGroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_24bpp	
  Foreground_color_16M(FontColor);
  Background_color_16M(BackGroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_16bpp	
  Foreground_color_65k(FontColor);					                               
  Background_color_65k(BackGroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_1	
  Foreground_color_16M(FontColor);
  Background_color_16M(BackGroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_2	
  Foreground_color_16M(FontColor);
  Background_color_16M(BackGroundColor);
#endif
  Text_Mode();
  Active_Window_XY(x,y);
  Active_Window_WH(X_W,Y_H);
  Goto_Text_XY(x,y);
//  sprintf(tmp1,"%s",tmp2); 
//  Show_String(tmp1);
  Show_String(tmp2);

}




void Print_GB2312String
(
unsigned char Clk //Clk : SPI CLK = System Clock / 2*(Clk+1)
,unsigned char SCS //SCS : 0 = CS0 , 1 = CS1
,unsigned short x //coordinate x for print string
,unsigned short y //coordinate y for print string
,unsigned short X_W //active window width
,unsigned short Y_H //active window height
,unsigned long FontColor //Set Font Color
,unsigned long BackGroundColor //Set Font BackGround Color 
/*Font Color and BackGround Color dataformat :
ColorDepth_8bpp : R3G3B2
ColorDepth_16bpp : R5G6B5
ColorDepth_24bpp : R8G8B8*/
,char tmp2[] //tmp2 : GB2312 Font String which you want print on LCD
)
{

  Select_SFI_Font_Mode();
  CGROM_Select_Genitop_FontROM();
  SPI_Clock_Period(Clk);
  Set_GTFont_Decoder(0x01);      
  if(SCS == 0)
  {
  Select_SFI_0();
  }
  if(SCS == 1)
  {
  Select_SFI_1();
  }
#ifdef MCU_8bit_ColorDepth_8bpp	
  Foreground_color_256(FontColor);
  Background_color_256(BackGroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_16bpp	
  Foreground_color_65k(FontColor);
  Background_color_65k(BackGroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_24bpp	
  Foreground_color_16M(FontColor);
  Background_color_16M(BackGroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_16bpp	
  Foreground_color_65k(FontColor);
  Background_color_65k(BackGroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_1	
  Foreground_color_16M(FontColor);
  Background_color_16M(BackGroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_2	
  Foreground_color_16M(FontColor);
  Background_color_16M(BackGroundColor);
#endif
  Text_Mode();
  Active_Window_XY(x,y);
  Active_Window_WH(X_W,Y_H);
  Goto_Text_XY(x,y);
//  sprintf(tmp1,"%s",tmp2); 
//  Show_String(tmp1);
  Show_String(tmp2);

}


void Print_GB12345String
(
unsigned char Clk //Clk : SPI CLK = System Clock / 2*(Clk+1)
,unsigned char SCS //SCS : 0 = CS0 , 1 = CS1
,unsigned short x //coordinate x for print string
,unsigned short y ////coordinate y for print string
,unsigned short X_W //active window width
,unsigned short Y_H //active window height
,unsigned long FontColor //Set Font Color
,unsigned long BackGroundColor //Set Font BackGround Color 
/*Font Color and BackGround Color dataformat :
ColorDepth_8bpp : R3G3B2
ColorDepth_16bpp : R5G6B5
ColorDepth_24bpp : R8G8B8*/
,char *tmp2 //tmp2 : GB12345 Font String which you want print on LCD
)
{  
  Select_SFI_Font_Mode();
  CGROM_Select_Genitop_FontROM();
  SPI_Clock_Period(Clk);
  Set_GTFont_Decoder(0x05);      
  if(SCS == 0)
  {
  Select_SFI_0();
  }
  
  if(SCS == 1)
  {
  Select_SFI_1();
  }
#ifdef MCU_8bit_ColorDepth_8bpp	
  Foreground_color_256(FontColor);
  Background_color_256(BackGroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_16bpp	
  Foreground_color_65k(FontColor);
  Background_color_65k(BackGroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_24bpp	
  Foreground_color_16M(FontColor);
  Background_color_16M(BackGroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_16bpp	
  Foreground_color_65k(FontColor);
  Background_color_65k(BackGroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_1	
  Foreground_color_16M(FontColor);
  Background_color_16M(BackGroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_2	
  Foreground_color_16M(FontColor);
  Background_color_16M(BackGroundColor);
#endif
  Text_Mode();
  Active_Window_XY(x,y);
  Active_Window_WH(X_W,Y_H);
  Goto_Text_XY(x,y);
//  sprintf(tmp1,"%s",tmp2); 
//  Show_String(tmp1);
  Show_String(tmp2);
}




 
void Print_UnicodeString
(
unsigned char Clk //SPI CLK = System Clock / 2*(Clk+1)
,unsigned char SCS //SCS : 0 = CS0 , 1 = CS1
,unsigned short x //Print font start coordinate of X
,unsigned short y //Print font start coordinate of Y
,unsigned short X_W //active window width
,unsigned short Y_H //active window height
,unsigned long FontColor //Set Font Color
,unsigned long BackGroundColor //Set Font BackGround Color 
/*Font Color and BackGround Color dataformat :
ColorDepth_8bpp : R3G3B2
ColorDepth_16bpp : R5G6B5
ColorDepth_24bpp : R8G8B8*/
,unsigned short *tmp2 /*tmp2 : Unicode Font String which you want print on LCD (L"string" in keil c is Unicode string)*/
)
{
  Select_SFI_Font_Mode();
  CGROM_Select_Genitop_FontROM();
  SPI_Clock_Period(Clk);
  if(SCS == 0)
  {
  Select_SFI_0();
  }
  if(SCS == 1)
  {
  Select_SFI_1();
  }
#ifdef MCU_8bit_ColorDepth_8bpp	
  Foreground_color_256(FontColor);
  Background_color_256(BackGroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_16bpp	
  Foreground_color_65k(FontColor);
  Background_color_65k(BackGroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_24bpp	
  Foreground_color_16M(FontColor);
  Background_color_16M(BackGroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_16bpp	
  Foreground_color_65k(FontColor);
  Background_color_65k(BackGroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_1	
  Foreground_color_16M(FontColor);
  Background_color_16M(BackGroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_2	
  Foreground_color_16M(FontColor);
  Background_color_16M(BackGroundColor);
#endif
  Text_Mode();
  Active_Window_XY(x,y);
  Active_Window_WH(X_W,Y_H);
  Goto_Text_XY(x,y);
  while(*tmp2 != '\0')
  {
    if((*tmp2)>0x0020 && (*tmp2)<0x0080)
		{
			/* ASCII Code*/
			Set_GTFont_Decoder(0x21); 
			RA8876_CmdWrite(0x04);
			RA8876_DataWrite(*tmp2);
			Check_2D_Busy();
		}
    else
		{
			/* Unicode */
	        Set_GTFont_Decoder(0x18);
			RA8876_CmdWrite(0x04);
			RA8876_DataWrite((*tmp2)>>8); 
            RA8876_DataWrite(*tmp2);
            Check_2D_Busy();
      
		}
 	        ++tmp2;
  }
  
  Graphic_Mode(); //back to graphic mode
}





void Select_Font_Height_WxN_HxN_ChromaKey_Alignment
(
unsigned char Font_Height 
/*Font_Height:
16 : Font = 8x16 16x16
24 : Font = 12x2424x24  
32 : Font = 16x3232x32 */
,unsigned char XxN //XxN :Font Width x 1~4
,unsigned char YxN //YxN :Font Height x 1~4
,unsigned char ChromaKey 
/*ChromaKey :
0 : Font Background color not transparency
1 : Set Font Background color transparency*/
,unsigned char Alignment // 0 : no alignment, 1 : Set font alignment
)
{
	if(Font_Height==16)	Font_Select_8x16_16x16();
	if(Font_Height==24)	Font_Select_12x24_24x24();
	if(Font_Height==32)	Font_Select_16x32_32x32();

	//(*)
	if(XxN==1)	Font_Width_X1();
	if(XxN==2)	Font_Width_X2();
	if(XxN==3)	Font_Width_X3();
	if(XxN==4)	Font_Width_X4();

	//(*)	
	if(YxN==1)	Font_Height_X1();
	if(YxN==2)	Font_Height_X2();
	if(YxN==3)	Font_Height_X3();
	if(YxN==4)	Font_Height_X4();

	//(*)
	if(ChromaKey==0)
	{	Font_Background_select_Color();	}

	if(ChromaKey==1)
	{	Font_Background_select_Original_Canvas();	}

	//(*)
	if(Alignment==0)
	{	Disable_Font_Alignment();	}

	if(Alignment==1)
	{	Enable_Font_Alignment();	}

} 




void Show_String(char *str)
{  
	Check_Mem_WR_FIFO_Empty();
	
	Text_Mode();
	
	RA8876_CmdWrite(0x04);
	while(*str != '\0')
	{
		Check_Mem_WR_FIFO_not_Full();  
		RA8876_DataWrite(*str);
		++str; 
	} 
	Check_2D_Busy();
	Graphic_Mode(); //back to graphic mode
}



void Draw_Line
(
unsigned long LineColor 
/*LineColor : Set Draw Line color. Line Color dataformat :
ColorDepth_8bpp : R3G3B2ColorDepth_16bpp : R5G6B5ColorDepth_24bpp : R8G8B8*/
,unsigned short X1 //X of point1 coordinate
,unsigned short Y1 //Y of point1 coordinate
,unsigned short X2 //X of point2 coordinate
,unsigned short Y2 // Y of point2 coordinate
)
{

#ifdef MCU_8bit_ColorDepth_8bpp	
  Foreground_color_256(LineColor);
#endif
#ifdef MCU_8bit_ColorDepth_16bpp	
  Foreground_color_65k(LineColor);
#endif
#ifdef MCU_8bit_ColorDepth_24bpp	
  Foreground_color_16M(LineColor);
#endif
#ifdef MCU_16bit_ColorDepth_16bpp	
  Foreground_color_65k(LineColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_1	
  Foreground_color_16M(LineColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_2	
  Foreground_color_16M(LineColor);
#endif
 Line_Start_XY(X1,Y1);
 Line_End_XY(X2,Y2);
 Start_Line();
 Check_2D_Busy(); 
}



void Draw_Triangle
(
unsigned long ForegroundColor 
/*ForegroundColor: Set Draw Triangle color. ForegroundColor Color dataformat :
ColorDepth_8bpp : R3G3B2ColorDepth_16bpp : R5G6B5ColorDepth_24bpp : R8G8B8*/
,unsigned short X1 //X of point1 coordinate
,unsigned short Y1 //Y of point1 coordinate
,unsigned short X2 //X of point2 coordinate
,unsigned short Y2 //Y of point2 coordinate
,unsigned short X3 //X of point3 coordinate
,unsigned short Y3 //Y of point3 coordinate
)
{ 
#ifdef MCU_8bit_ColorDepth_8bpp	
  Foreground_color_256(ForegroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_16bpp	
  Foreground_color_65k(ForegroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_24bpp	
  Foreground_color_16M(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_16bpp	
  Foreground_color_65k(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_1	
  Foreground_color_16M(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_2	
  Foreground_color_16M(ForegroundColor);
#endif
  Triangle_Point1_XY(X1,Y1);
  Triangle_Point2_XY(X2,Y2);
  Triangle_Point3_XY(X3,Y3);
  Start_Triangle();
  Check_2D_Busy(); 
}


void Draw_Triangle_Fill
(
unsigned long ForegroundColor 
/*ForegroundColor: Set Draw Triangle color. ForegroundColor Color dataformat :
ColorDepth_8bpp : R3G3B2ColorDepth_16bpp : R5G6B5ColorDepth_24bpp : R8G8B8*/
,unsigned short X1 //X of point1 coordinate
,unsigned short Y1 //Y of point1 coordinate
,unsigned short X2 //X of point2 coordinate
,unsigned short Y2 //Y of point2 coordinate
,unsigned short X3 //X of point3 coordinate
,unsigned short Y3 //Y of point3 coordinate
)
{
#ifdef MCU_8bit_ColorDepth_8bpp	
  Foreground_color_256(ForegroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_16bpp	
  Foreground_color_65k(ForegroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_24bpp	
  Foreground_color_16M(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_16bpp	
  Foreground_color_65k(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_1	
  Foreground_color_16M(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_2	
  Foreground_color_16M(ForegroundColor);
#endif
  Triangle_Point1_XY(X1,Y1);
  Triangle_Point2_XY(X2,Y2);
  Triangle_Point3_XY(X3,Y3);
  Start_Triangle_Fill();
  Check_2D_Busy(); 
}



void Draw_Circle
(
unsigned long ForegroundColor //ForegroundColor: Set Draw Circle or Circle Fill color
/*ForegroundColor Color dataformat :
ColorDepth_8bpp : R3G3B2ColorDepth_16bpp : R5G6B5ColorDepth_24bpp : R8G8B8*/
,unsigned short XCenter //coordinate X of Center
,unsigned short YCenter //coordinate Y of Center
,unsigned short R //Circle Radius
)
{
#ifdef MCU_8bit_ColorDepth_8bpp	
  Foreground_color_256(ForegroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_16bpp	
  Foreground_color_65k(ForegroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_24bpp	
  Foreground_color_16M(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_16bpp	
  Foreground_color_65k(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_1	
  Foreground_color_16M(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_2	
  Foreground_color_16M(ForegroundColor);
#endif
  Circle_Center_XY(XCenter,YCenter);
  Circle_Radius_R(R);
  Start_Circle_or_Ellipse();
  Check_2D_Busy(); 
}


void Draw_Circle_Fill
(
unsigned long ForegroundColor 
/*ForegroundColor: Set Draw Circle or Circle Fill color 
ForegroundColor Color dataformat :
ColorDepth_8bpp : R3G3B2ColorDepth_16bpp : R5G6B5ColorDepth_24bpp : R8G8B8*/
,unsigned short XCenter //coordinate X of Center
,unsigned short YCenter //coordinate Y of Center
,unsigned short R //Circle Radius
)
{
#ifdef MCU_8bit_ColorDepth_8bpp	
  Foreground_color_256(ForegroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_16bpp	
  Foreground_color_65k(ForegroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_24bpp	
  Foreground_color_16M(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_16bpp	
  Foreground_color_65k(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_1	
  Foreground_color_16M(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_2	
  Foreground_color_16M(ForegroundColor);
#endif
  Circle_Center_XY(XCenter,YCenter);
  Circle_Radius_R(R);
  Start_Circle_or_Ellipse_Fill();
  Check_2D_Busy(); 
}



void Draw_Ellipse
(
unsigned long ForegroundColor //ForegroundColor : Set Draw Ellipse or Ellipse Fill color
/*ForegroundColor Color dataformat :
ColorDepth_8bpp : R3G3B2ColorDepth_16bpp : R5G6B5ColorDepth_24bpp : R8G8B8*/
,unsigned short XCenter //coordinate X of Center
,unsigned short YCenter //coordinate Y of Center
,unsigned short X_R // Radius Width of Ellipse
,unsigned short Y_R // Radius Length of Ellipse
)
{
#ifdef MCU_8bit_ColorDepth_8bpp	
  Foreground_color_256(ForegroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_16bpp	
  Foreground_color_65k(ForegroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_24bpp	
  Foreground_color_16M(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_16bpp	
  Foreground_color_65k(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_1	
  Foreground_color_16M(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_2	
  Foreground_color_16M(ForegroundColor);
#endif
  Ellipse_Center_XY(XCenter,YCenter);
  Ellipse_Radius_RxRy(X_R,Y_R);
  Start_Circle_or_Ellipse();
  Check_2D_Busy(); 
}



void Draw_Ellipse_Fill
(
unsigned long ForegroundColor //ForegroundColor : Set Draw Ellipse or Ellipse Fill color
/*ForegroundColor Color dataformat :
ColorDepth_8bpp : R3G3B2ColorDepth_16bpp : R5G6B5ColorDepth_24bpp : R8G8B8*/
,unsigned short XCenter //coordinate X of Center
,unsigned short YCenter //coordinate Y of Center
,unsigned short X_R // Radius Width of Ellipse
,unsigned short Y_R // Radius Length of Ellipse
)				
{
#ifdef MCU_8bit_ColorDepth_8bpp	
  Foreground_color_256(ForegroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_16bpp	
  Foreground_color_65k(ForegroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_24bpp	
  Foreground_color_16M(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_16bpp	
  Foreground_color_65k(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_1	
  Foreground_color_16M(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_2	
  Foreground_color_16M(ForegroundColor);
#endif
  Ellipse_Center_XY(XCenter,YCenter);
  Ellipse_Radius_RxRy(X_R,Y_R);
  Start_Circle_or_Ellipse_Fill();
  Check_2D_Busy(); 
}




void Draw_Left_Up_Curve
(
unsigned long ForegroundColor //ForegroundColor: Set Curve or Curve Fill color 
/*ForegroundColor Color dataformat :
ColorDepth_8bpp : R3G3B2ColorDepth_16bpp : R5G6B5ColorDepth_24bpp : R8G8B8*/
,unsigned short XCenter //coordinate X of Center
,unsigned short YCenter //coordinate Y of Center
,unsigned short X_R // Radius Width of Curve
,unsigned short Y_R // Radius Length of Curve
)
{
#ifdef MCU_8bit_ColorDepth_8bpp	
  Foreground_color_256(ForegroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_16bpp	
  Foreground_color_65k(ForegroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_24bpp	
  Foreground_color_16M(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_16bpp	
  Foreground_color_65k(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_1	
  Foreground_color_16M(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_2	
  Foreground_color_16M(ForegroundColor);
#endif
 Ellipse_Center_XY(XCenter,YCenter);
 Ellipse_Radius_RxRy(X_R,Y_R);
 Start_Left_Up_Curve();
 Check_2D_Busy(); 
}



void Draw_Left_Up_Curve_Fill
(
unsigned long ForegroundColor //ForegroundColor: Set Curve or Curve Fill color 
/*ForegroundColor Color dataformat :
ColorDepth_8bpp : R3G3B2ColorDepth_16bpp : R5G6B5ColorDepth_24bpp : R8G8B8*/
,unsigned short XCenter //coordinate X of Center
,unsigned short YCenter //coordinate Y of Center
,unsigned short X_R // Radius Width of Curve
,unsigned short Y_R // Radius Length of Curve
)
{
#ifdef MCU_8bit_ColorDepth_8bpp	
  Foreground_color_256(ForegroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_16bpp	
  Foreground_color_65k(ForegroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_24bpp	
  Foreground_color_16M(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_16bpp	
  Foreground_color_65k(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_1	
  Foreground_color_16M(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_2	
  Foreground_color_16M(ForegroundColor);
#endif
  Ellipse_Center_XY(XCenter,YCenter);
  Ellipse_Radius_RxRy(X_R,Y_R);
  Start_Left_Up_Curve_Fill();
  Check_2D_Busy(); 
}




void Draw_Right_Down_Curve
(
unsigned long ForegroundColor //ForegroundColor: Set Curve or Curve Fill color 
/*ForegroundColor Color dataformat :
ColorDepth_8bpp : R3G3B2ColorDepth_16bpp : R5G6B5ColorDepth_24bpp : R8G8B8*/
,unsigned short XCenter //coordinate X of Center
,unsigned short YCenter //coordinate Y of Center
,unsigned short X_R // Radius Width of Curve
,unsigned short Y_R // Radius Length of Curve
) 
{
#ifdef MCU_8bit_ColorDepth_8bpp	
  Foreground_color_256(ForegroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_16bpp	
  Foreground_color_65k(ForegroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_24bpp	
  Foreground_color_16M(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_16bpp	
  Foreground_color_65k(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_1	
  Foreground_color_16M(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_2	
  Foreground_color_16M(ForegroundColor);
#endif
  Ellipse_Center_XY(XCenter,YCenter);
  Ellipse_Radius_RxRy(X_R,Y_R);
  Start_Right_Down_Curve();
  Check_2D_Busy(); 
}



void Draw_Right_Down_Curve_Fill
(
unsigned long ForegroundColor //ForegroundColor: Set Curve or Curve Fill color 
/*ForegroundColor Color dataformat :
ColorDepth_8bpp : R3G3B2ColorDepth_16bpp : R5G6B5ColorDepth_24bpp : R8G8B8*/
,unsigned short XCenter //coordinate X of Center
,unsigned short YCenter //coordinate Y of Center
,unsigned short X_R // Radius Width of Curve
,unsigned short Y_R // Radius Length of Curve
)
{
#ifdef MCU_8bit_ColorDepth_8bpp	
  Foreground_color_256(ForegroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_16bpp	
  Foreground_color_65k(ForegroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_24bpp	
  Foreground_color_16M(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_16bpp	
  Foreground_color_65k(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_1	
  Foreground_color_16M(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_2	
  Foreground_color_16M(ForegroundColor);
#endif
  Ellipse_Center_XY(XCenter,YCenter);
  Ellipse_Radius_RxRy(X_R,Y_R);
  Start_Right_Down_Curve_Fill();
  Check_2D_Busy(); 
}



void Draw_Right_Up_Curve
(
unsigned long ForegroundColor //ForegroundColor: Set Curve or Curve Fill color 
/*ForegroundColor Color dataformat :
ColorDepth_8bpp : R3G3B2ColorDepth_16bpp : R5G6B5ColorDepth_24bpp : R8G8B8*/
,unsigned short XCenter //coordinate X of Center
,unsigned short YCenter //coordinate Y of Center
,unsigned short X_R // Radius Width of Curve
,unsigned short Y_R // Radius Length of Curve
)
{
#ifdef MCU_8bit_ColorDepth_8bpp	
  Foreground_color_256(ForegroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_16bpp	
  Foreground_color_65k(ForegroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_24bpp	
  Foreground_color_16M(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_16bpp	
  Foreground_color_65k(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_1	
  Foreground_color_16M(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_2	
  Foreground_color_16M(ForegroundColor);
#endif
  Ellipse_Center_XY(XCenter,YCenter);
  Ellipse_Radius_RxRy(X_R,Y_R);
  Start_Right_Up_Curve();
  Check_2D_Busy(); 
}



void Draw_Right_Up_Curve_Fill
(
unsigned long ForegroundColor //ForegroundColor: Set Curve or Curve Fill color 
/*ForegroundColor Color dataformat :
ColorDepth_8bpp : R3G3B2ColorDepth_16bpp : R5G6B5ColorDepth_24bpp : R8G8B8*/
,unsigned short XCenter //coordinate X of Center
,unsigned short YCenter //coordinate Y of Center
,unsigned short X_R // Radius Width of Curve
,unsigned short Y_R // Radius Length of Curve
)
{
#ifdef MCU_8bit_ColorDepth_8bpp	
  Foreground_color_256(ForegroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_16bpp	
  Foreground_color_65k(ForegroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_24bpp	
  Foreground_color_16M(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_16bpp	
  Foreground_color_65k(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_1	
  Foreground_color_16M(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_2	
  Foreground_color_16M(ForegroundColor);
#endif
  Ellipse_Center_XY(XCenter,YCenter);
  Ellipse_Radius_RxRy(X_R,Y_R);
  Start_Right_Up_Curve_Fill();
  Check_2D_Busy();
}



void Draw_Left_Down_Curve
(
unsigned long ForegroundColor //ForegroundColor: Set Curve or Curve Fill color 
/*ForegroundColor Color dataformat :
ColorDepth_8bpp : R3G3B2ColorDepth_16bpp : R5G6B5ColorDepth_24bpp : R8G8B8*/
,unsigned short XCenter //coordinate X of Center
,unsigned short YCenter //coordinate Y of Center
,unsigned short X_R // Radius Width of Curve
,unsigned short Y_R // Radius Length of Curve
)
{
#ifdef MCU_8bit_ColorDepth_8bpp	
  Foreground_color_256(ForegroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_16bpp	
  Foreground_color_65k(ForegroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_24bpp	
  Foreground_color_16M(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_16bpp	
  Foreground_color_65k(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_1	
  Foreground_color_16M(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_2	
  Foreground_color_16M(ForegroundColor);
#endif
  Ellipse_Center_XY(XCenter,YCenter);
  Ellipse_Radius_RxRy(X_R,Y_R);
  Start_Left_Down_Curve();
  Check_2D_Busy(); 
}



void Draw_Left_Down_Curve_Fill
(
unsigned long ForegroundColor //ForegroundColor: Set Curve or Curve Fill color 
/*ForegroundColor Color dataformat :
ColorDepth_8bpp : R3G3B2ColorDepth_16bpp : R5G6B5ColorDepth_24bpp : R8G8B8*/
,unsigned short XCenter //coordinate X of Center
,unsigned short YCenter //coordinate Y of Center
,unsigned short X_R // Radius Width of Curve
,unsigned short Y_R // Radius Length of Curve
)
{
#ifdef MCU_8bit_ColorDepth_8bpp	
  Foreground_color_256(ForegroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_16bpp	
  Foreground_color_65k(ForegroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_24bpp	
  Foreground_color_16M(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_16bpp	
  Foreground_color_65k(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_1	
  Foreground_color_16M(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_2	
  Foreground_color_16M(ForegroundColor);
#endif
  Ellipse_Center_XY(XCenter,YCenter);
  Ellipse_Radius_RxRy(X_R,Y_R);
  Start_Left_Down_Curve_Fill();
  Check_2D_Busy(); 
}


void Draw_Square
(
unsigned long ForegroundColor 
/*ForegroundColor: Set Curve or Curve Fill color. ForegroundColor Color dataformat :
ColorDepth_8bpp : R3G3B2ColorDepth_16bpp : R5G6B5ColorDepth_24bpp : R8G8B8*/
,unsigned short X1 //X of point1 coordinate
,unsigned short Y1 //Y of point1 coordinate
,unsigned short X2 //X of point2 coordinate
,unsigned short Y2 //Y of point2 coordinate
)
{
#ifdef MCU_8bit_ColorDepth_8bpp	
  Foreground_color_256(ForegroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_16bpp	
  Foreground_color_65k(ForegroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_24bpp	
  Foreground_color_16M(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_16bpp	
  Foreground_color_65k(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_1	
  Foreground_color_16M(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_2	
  Foreground_color_16M(ForegroundColor);
#endif
  Square_Start_XY(X1,Y1);
  Square_End_XY(X2,Y2);
  Start_Square();
  Check_2D_Busy(); 
}


void Draw_Square_Fill
(
unsigned long ForegroundColor 
/*ForegroundColor: Set Curve or Curve Fill color. ForegroundColor Color dataformat :
ColorDepth_8bpp : R3G3B2ColorDepth_16bpp : R5G6B5ColorDepth_24bpp : R8G8B8*/
,unsigned short X1 //X of point1 coordinate
,unsigned short Y1 //Y of point1 coordinate
,unsigned short X2 //X of point2 coordinate
,unsigned short Y2 //Y of point2 coordinate
)
{
#ifdef MCU_8bit_ColorDepth_8bpp	
  Foreground_color_256(ForegroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_16bpp	
  Foreground_color_65k(ForegroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_24bpp	
  Foreground_color_16M(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_16bpp	
  Foreground_color_65k(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_1	
  Foreground_color_16M(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_2	
  Foreground_color_16M(ForegroundColor);
#endif
  Square_Start_XY(X1,Y1);
  Square_End_XY(X2,Y2);
  Start_Square_Fill();
  Check_2D_Busy(); 
}


/*
*********************************************************************************************************
*	函 数 名: Draw_Circle_Square
*	功能说明: 绘制圆角矩形
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void Draw_Circle_Square
(
unsigned long ForegroundColor 
/*ForegroundColor : Set Circle Square color. ForegroundColor Color dataformat :
ColorDepth_8bpp : R3G3B2ColorDepth_16bpp : R5G6B5ColorDepth_24bpp : R8G8B8*/
,unsigned short X1 //X of point1 coordinate
,unsigned short Y1 //Y of point1 coordinate
,unsigned short X2 //X of point2 coordinate
,unsigned short Y2 //Y of point2 coordinate
,unsigned short X_R //Radius Width of Circle Square
,unsigned short Y_R //Radius Length of Circle Square
)
{
#ifdef MCU_8bit_ColorDepth_8bpp	
  Foreground_color_256(ForegroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_16bpp	
  Foreground_color_65k(ForegroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_24bpp	
  Foreground_color_16M(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_16bpp	
  Foreground_color_65k(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_1	
  Foreground_color_16M(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_2	
  Foreground_color_16M(ForegroundColor);
#endif 
  Square_Start_XY(X1,Y1);
  Square_End_XY(X2,Y2); 
  Circle_Square_Radius_RxRy(X_R,Y_R);
  Start_Circle_Square();
  Check_2D_Busy(); 
}



/*
*********************************************************************************************************
*	函 数 名: Draw_Circle_Square_Fill
*	功能说明: 填充圆角矩形
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void Draw_Circle_Square_Fill
(
unsigned long ForegroundColor 
/*ForegroundColor : Set Circle Square color. ForegroundColor Color dataformat :
ColorDepth_8bpp : R3G3B2ColorDepth_16bpp : R5G6B5ColorDepth_24bpp : R8G8B8*/
,unsigned short X1 //X of point1 coordinate
,unsigned short Y1 //Y of point1 coordinate
,unsigned short X2 //X of point2 coordinate
,unsigned short Y2 //Y of point2 coordinate
,unsigned short X_R //Radius Width of Circle Square
,unsigned short Y_R //Radius Length of Circle Square
)
{

#ifdef MCU_8bit_ColorDepth_8bpp	
  Foreground_color_256(ForegroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_16bpp	
  Foreground_color_65k(ForegroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_24bpp	
  Foreground_color_16M(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_16bpp	
  Foreground_color_65k(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_1	
  Foreground_color_16M(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_2	
  Foreground_color_16M(ForegroundColor);
#endif
  Square_Start_XY(X1,Y1);
  Square_End_XY(X2,Y2); 
  Circle_Square_Radius_RxRy(X_R,Y_R);
  Start_Circle_Square_Fill();
  Check_2D_Busy(); 
}



void BTE_Pattern_Fill
(
unsigned char P_8x8_or_16x16 //0 : use 8x8 Icon , 1 : use 16x16 Icon.
,unsigned long S0_Addr //Start address of Source 0
,unsigned short S0_W //image width of Source 0 (recommend = canvas image width)
,unsigned short XS0 // coordinate X of Source 0
,unsigned short YS0 // coordinate Y of Source 0
,unsigned long S1_Addr //Start address of Source 1
,unsigned short S1_W //image width of Source 1 (recommend = canvas image width)
,unsigned short XS1 //coordinate X of Source 1
,unsigned short YS1 //coordinate Y of Source 1
,unsigned long Des_Addr // start address of Destination
,unsigned short Des_W //image width of Destination (recommend = canvas image width)
, unsigned short XDes //coordinate X of Destination
,unsigned short YDes //coordinate Y of Destination
,unsigned int ROP_Code 
/*ROP_Code :
   0000b		0(Blackness)
   0001b		~S0!E~S1 or ~(S0+S1)
   0010b		~S0!ES1
   0011b		~S0
   0100b		S0!E~S1
   0101b		~S1
   0110b		S0^S1
   0111b		~S0 + ~S1 or ~(S0 + S1)
   1000b		S0!ES1
   1001b		~(S0^S1)
   1010b		S1
   1011b		~S0+S1
   1100b		S0
   1101b		S0+~S1
   1110b		S0+S1
   1111b		1(whiteness)*/
,unsigned short X_W //Width of BTE Winodw
,unsigned short Y_H //Length of BTE Winodw
)
{
  if(P_8x8_or_16x16 == 0)
  {
  Pattern_Format_8X8();
   }
  if(P_8x8_or_16x16 == 1)
  {														    
  Pattern_Format_16X16();
  }	  
  BTE_S0_Memory_Start_Address(S0_Addr);
  BTE_S0_Image_Width(S0_W);
  BTE_S0_Window_Start_XY(XS0,YS0);

  BTE_S1_Memory_Start_Address(S1_Addr);
  BTE_S1_Image_Width(S1_W); 
  BTE_S1_Window_Start_XY(XS1,YS1);

  BTE_Destination_Memory_Start_Address(Des_Addr);
  BTE_Destination_Image_Width(Des_W);
  BTE_Destination_Window_Start_XY(XDes,YDes);	
   
  BTE_ROP_Code(ROP_Code);	
  BTE_Operation_Code(0x06); //BTE Operation: Pattern Fill with ROP.
  BTE_Window_Size(X_W,Y_H); 
  BTE_Enable();
  Check_BTE_Busy();
}

void BTE_Pattern_Fill_With_Chroma_key
(
unsigned char P_8x8_or_16x16 //0 : use 8x8 Icon , 1 : use 16x16 Icon.
,unsigned long S0_Addr //Start address of Source 0
,unsigned short S0_W //image width of Source 0 (recommend = canvas image width)
,unsigned short XS0 //coordinate X of Source 0
,unsigned short YS0 //coordinate Y of Source 0
,unsigned long S1_Addr //Start address of Source 1
,unsigned short S1_W //image width of Source 1 (recommend = canvas image width)
,unsigned short XS1 //coordinate X of Source 1
,unsigned short YS1 //coordinate Y of Source 1
,unsigned long Des_Addr //Des_Addr : start address of Destination
,unsigned short Des_W //Des_W : image width of Destination (recommend = canvas image width)
,unsigned short XDes //coordinate X of Destination
,unsigned short YDes //coordinate Y of Destination
,unsigned int ROP_Code
/*ROP_Code :
   0000b		0(Blackness)
   0001b		~S0!E~S1 or ~(S0+S1)
   0010b		~S0!ES1
   0011b		~S0
   0100b		S0!E~S1
   0101b		~S1
   0110b		S0^S1
   0111b		~S0 + ~S1 or ~(S0 + S1)
   1000b		S0!ES1
   1001b		~(S0^S1)
   1010b		S1
   1011b		~S0+S1
   1100b		S0
   1101b		S0+~S1
   1110b		S0+S1
   1111b		1(whiteness)*/
,unsigned long Background_color //Transparent color
,unsigned short X_W //Width of BTE Window
,unsigned short Y_H //Length of BTE Window
)
{
#ifdef MCU_8bit_ColorDepth_8bpp	
  Background_color_256(Background_color); 
#endif
#ifdef MCU_8bit_ColorDepth_16bpp	
  Background_color_65k(Background_color); 
#endif
#ifdef MCU_8bit_ColorDepth_24bpp	
  Background_color_16M(Background_color); 
#endif
#ifdef MCU_16bit_ColorDepth_16bpp	
  Background_color_65k(Background_color); 
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_1	
  Background_color_16M(Background_color); 
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_2	
  Background_color_16M(Background_color); 
#endif
  if(P_8x8_or_16x16 == 0)
  {
  Pattern_Format_8X8();
   }
  if(P_8x8_or_16x16 == 1)
  {														    
  Pattern_Format_16X16();
  }	  
  BTE_S0_Memory_Start_Address(S0_Addr);
  BTE_S0_Image_Width(S0_W);
  BTE_S0_Window_Start_XY(XS0,YS0);

  BTE_S1_Memory_Start_Address(S1_Addr);
  BTE_S1_Image_Width(S1_W); 
  BTE_S1_Window_Start_XY(XS1,YS1);

  BTE_Destination_Memory_Start_Address(Des_Addr);
  BTE_Destination_Image_Width(Des_W);
  BTE_Destination_Window_Start_XY(XDes,YDes);	
   
  BTE_ROP_Code(ROP_Code);	
  BTE_Operation_Code(0x07); //BTE Operation: Pattern Fill with Chroma key.
  BTE_Window_Size(X_W,Y_H); 
  BTE_Enable();
  Check_BTE_Busy();
}

void BTE_Memory_Copy
(
unsigned long S0_Addr //Start address of Source 0
,unsigned short S0_W //image width of Source 0 (recommend = canvas image width)
,unsigned short XS0 //coordinate X of Source 0
,unsigned short YS0 //coordinate Y of Source 0
,unsigned long S1_Addr //Start address of Source 1
,unsigned short S1_W //image width of Source 1 (recommend = canvas image width)
,unsigned short XS1 //coordinate X of Source 1
,unsigned short YS1 //coordinate Y of Source 1
,unsigned long Des_Addr //start address of Destination
,unsigned short Des_W //image width of Destination (recommend = canvas image width)
,unsigned short XDes //coordinate X of Destination
,unsigned short YDes //coordinate Y of Destination
,unsigned int ROP_Code
/*ROP_Code :
   0000b		0(Blackness)
   0001b		~S0!E~S1 or ~(S0+S1)
   0010b		~S0!ES1
   0011b		~S0
   0100b		S0!E~S1
   0101b		~S1
   0110b		S0^S1
   0111b		~S0 + ~S1 or ~(S0 + S1)
   1000b		S0!ES1
   1001b		~(S0^S1)
   1010b		S1
   1011b		~S0+S1
   1100b		S0
   1101b		S0+~S1
   1110b		S0+S1
   1111b		1(whiteness)*/
,unsigned short X_W //X_W : Width of BTE Window
,unsigned short Y_H //Y_H : Length of BTE Window
)
{
  BTE_S0_Memory_Start_Address(S0_Addr);
  BTE_S0_Image_Width(S0_W);
  BTE_S0_Window_Start_XY(XS0,YS0);

  BTE_S1_Memory_Start_Address(S1_Addr);
  BTE_S1_Image_Width(S1_W); 
  BTE_S1_Window_Start_XY(XS1,YS1);

  BTE_Destination_Memory_Start_Address(Des_Addr);
  BTE_Destination_Image_Width(Des_W);
  BTE_Destination_Window_Start_XY(XDes,YDes);	
   
  BTE_ROP_Code(ROP_Code);	
  BTE_Operation_Code(0x02); //BTE Operation: Memory copy (move) with ROP.
  BTE_Window_Size(X_W,Y_H); 
  BTE_Enable();
  Check_BTE_Busy();
}






void BTE_Memory_Copy_Chroma_key
(
unsigned long S0_Addr //Start address of Source 0
,unsigned short S0_W //image width of Source 0 (recommend = canvas image width) 
,unsigned short XS0 //coordinate X of Source 0
,unsigned short YS0 //coordinate Y of Source 0
,unsigned long Des_Addr //start address of Destination
,unsigned short Des_W //image width of Destination (recommend = canvas image width) 
,unsigned short XDes //coordinate X of Destination
,unsigned short YDes //coordinate Y of Destination
,unsigned long Background_color // transparent color
,unsigned short X_W //Width of BTE Window
,unsigned short Y_H //Length of BTE Window
)
{

#ifdef MCU_8bit_ColorDepth_8bpp	
  Background_color_256(Background_color); 
#endif
#ifdef MCU_8bit_ColorDepth_16bpp	
  Background_color_65k(Background_color); 
#endif
#ifdef MCU_8bit_ColorDepth_24bpp	
  Background_color_16M(Background_color); 
#endif
#ifdef MCU_16bit_ColorDepth_16bpp	
  Background_color_65k(Background_color); 
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_1	
  Background_color_16M(Background_color); 
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_2	
  Background_color_16M(Background_color); 
#endif
  BTE_S0_Memory_Start_Address(S0_Addr);
  BTE_S0_Image_Width(S0_W);
  BTE_S0_Window_Start_XY(XS0,YS0);	

  BTE_Destination_Memory_Start_Address(Des_Addr);
  BTE_Destination_Image_Width(Des_W);
  BTE_Destination_Window_Start_XY(XDes,YDes);
   
  BTE_Operation_Code(0x05);	//BTE Operation: Memory copy (move) with chroma keying (w/o ROP)
  BTE_Window_Size(X_W,Y_H); 
  BTE_Enable();
  Check_BTE_Busy();
}


void BTE_MCU_Write_MCU_8bit
(
unsigned long S1_Addr //Start address of Source 1
,unsigned short S1_W //image width of Source 1 (recommend = canvas image width)
,unsigned short XS1 //coordinate X of Source 1
,unsigned short YS1 //coordinate Y of Source 1
,unsigned long Des_Addr //start address of Destination
,unsigned short Des_W //image width of Destination (recommend = canvas image width)
,unsigned short XDes //coordinate X of Destination
,unsigned short YDes //coordinate Y of Destination
,unsigned int ROP_Code
/*ROP_Code :
   0000b		0(Blackness)
   0001b		~S0!E~S1 or ~(S0+S1)
   0010b		~S0!ES1
   0011b		~S0
   0100b		S0!E~S1
   0101b		~S1
   0110b		S0^S1
   0111b		~S0 + ~S1 or ~(S0 + S1)
   1000b		S0!ES1
   1001b		~(S0^S1)
   1010b		S1
   1011b		~S0+S1
   1100b		S0
   1101b		S0+~S1
   1110b		S0+S1
   1111b		1(whiteness)*/
,unsigned short X_W // Width of BTE Window
,unsigned short Y_H // Length of BTE Window
,const unsigned char *data // 8-bit data
)
{		
//  unsigned short i,j;

  BTE_S1_Memory_Start_Address(S1_Addr);
  BTE_S1_Image_Width(S1_W); 
  BTE_S1_Window_Start_XY(XS1,YS1);

  BTE_Destination_Memory_Start_Address(Des_Addr);
  BTE_Destination_Image_Width(Des_W);
  BTE_Destination_Window_Start_XY(XDes,YDes);
  
  BTE_Window_Size(X_W,Y_H);
  BTE_ROP_Code(ROP_Code);
  BTE_Operation_Code(0x00);		//BTE Operation: MPU Write with ROP.
  BTE_Enable();
  RA8876_CmdWrite(0x04);			//Memory Data Read/Write Port

#ifdef MCU_8bit_ColorDepth_8bpp	    			 		
	for(i=0;i< Y_H;i++)
    {	
	  for(j=0;j< (X_W);j++)
 	  {
	   Check_Mem_WR_FIFO_not_Full();
	   RA8876_DataWrite(*data);
	   data++;
	  }
    }
  Check_Mem_WR_FIFO_Empty();
#endif

#ifdef MCU_8bit_ColorDepth_16bpp	
	for(i=0;i< Y_H;i++)
    {	
	  for(j=0;j< (X_W*2);j++)
 	  {
	   Check_Mem_WR_FIFO_not_Full();
	   RA8876_DataWrite(*data);
	   data++;
	  }
    }
  Check_Mem_WR_FIFO_Empty();
#endif

#ifdef MCU_8bit_ColorDepth_24bpp	
	for(i=0;i< Y_H;i++)
    {	
	  for(j=0;j< (X_W*3);j++)
 	  {
	   Check_Mem_WR_FIFO_not_Full();
	   RA8876_DataWrite(*data);
	   data++;
	  }
    }
  Check_Mem_WR_FIFO_Empty();
#endif

}


void BTE_MCU_Write_MCU_16bit
(
unsigned long S1_Addr //Start address of Source 1
,unsigned short S1_W //image width of Source 1 (recommend = canvas image width)
,unsigned short XS1 //coordinate X of Source 1
,unsigned short YS1 //coordinate Y of Source 1
,unsigned long Des_Addr //start address of Destination
,unsigned short Des_W //image width of Destination (recommend = canvas image width)
,unsigned short XDes //coordinate X of Destination
,unsigned short YDes //coordinate Y of Destination
,unsigned int ROP_Code
/*ROP_Code :
   0000b		0(Blackness)
   0001b		~S0!E~S1 or ~(S0+S1)
   0010b		~S0!ES1
   0011b		~S0
   0100b		S0!E~S1
   0101b		~S1
   0110b		S0^S1
   0111b		~S0 + ~S1 or ~(S0 + S1)
   1000b		S0!ES1
   1001b		~(S0^S1)
   1010b		S1
   1011b		~S0+S1
   1100b		S0
   1101b		S0+~S1
   1110b		S0+S1
   1111b		1(whiteness)*/
,unsigned short X_W // Width of BTE Window
,unsigned short Y_H // Length of BTE Window
,const unsigned short *data // 16-bit data 
)
{	
  unsigned short i,j;

  BTE_S1_Memory_Start_Address(S1_Addr);
  BTE_S1_Image_Width(S1_W); 
  BTE_S1_Window_Start_XY(XS1,YS1);

  BTE_Destination_Memory_Start_Address(Des_Addr);
  BTE_Destination_Image_Width(Des_W);
  BTE_Destination_Window_Start_XY(XDes,YDes);
  
  BTE_Window_Size(X_W,Y_H);
  BTE_ROP_Code(ROP_Code);
  BTE_Operation_Code(0x00);		//BTE Operation: MPU Write with ROP.
  BTE_Enable();
  RA8876_CmdWrite(0x04);				 		//Memory Data Read/Write Port
#ifdef MCU_16bit_ColorDepth_16bpp	
	for(i=0;i< Y_H;i++)
    {	
	  for(j=0;j< (X_W);j++)
 	  {
	   Check_Mem_WR_FIFO_not_Full();
	   RA8876_DataWrite((*data));
	   data++;
	  }
    }
  Check_Mem_WR_FIFO_Empty();
#endif

#ifdef MCU_16bit_ColorDepth_24bpp_Mode_1	
	for(i=0;i< Y_H;i++)
    {	
	  for(j=0;j< (X_W/2);j++)
 	  {
	    Check_Mem_WR_FIFO_not_Full();
	    RA8876_DataWrite(*data);
	    data++;
	    Check_Mem_WR_FIFO_not_Full();
	    RA8876_DataWrite(*data);
	    data++;
	    Check_Mem_WR_FIFO_not_Full();
	    RA8876_DataWrite(*data);
	    data++;
	  }
    }
  Check_Mem_WR_FIFO_Empty();
#endif
  

#ifdef MCU_16bit_ColorDepth_24bpp_Mode_2	
	for(i=0;i< Y_H;i++)
    {	
	  for(j=0;j< (X_W);j++)
 	  {
	    Check_Mem_WR_FIFO_not_Full();
	    RA8876_DataWrite(*data);
	    data++;
	    Check_Mem_WR_FIFO_not_Full();
	    RA8876_DataWrite(*data);
	    data++;
	  }
    }
  Check_Mem_WR_FIFO_Empty();
#endif

  Check_BTE_Busy();
	
}






void BTE_MCU_Write_Chroma_key_MCU_8bit
(
unsigned long Des_Addr //start address of Destination
,unsigned short Des_W //image width of Destination (recommend = canvas image width)
,unsigned short XDes //coordinate X of Destination
,unsigned short YDes //coordinate Y of Destination
,unsigned long Background_color //transparency color
,unsigned short X_W //Width of BTE Window
,unsigned short Y_H //Length of BTE Window
,const unsigned char *data // 8-bit data
)
{
// unsigned short i,j;
	
#ifdef MCU_8bit_ColorDepth_8bpp	
  Background_color_256(Background_color);
#endif

#ifdef MCU_8bit_ColorDepth_16bpp	
  Background_color_65k(Background_color);
#endif

#ifdef MCU_8bit_ColorDepth_24bpp	
  Background_color_16M(Background_color);
#endif
  BTE_Destination_Memory_Start_Address(Des_Addr);
  BTE_Destination_Image_Width(Des_W);
  BTE_Destination_Window_Start_XY(XDes,YDes);
  
  BTE_Window_Size(X_W,Y_H);
  BTE_Operation_Code(0x04);		//BTE Operation: MPU Write w/ chroma keying (w/o ROP)
  BTE_Enable();
  
  RA8876_CmdWrite(0x04);				 		//Memory Data Read/Write Port

#ifdef MCU_8bit_ColorDepth_8bpp	
    			 		
	for(i=0;i< Y_H;i++)
    {	
	  for(j=0;j< (X_W);j++)
 	  {
	   Check_Mem_WR_FIFO_not_Full();
	   RA8876_DataWrite(*data);
	   data++;
	  }
    }
  Check_Mem_WR_FIFO_Empty();
#endif

#ifdef MCU_8bit_ColorDepth_16bpp	
	for(i=0;i< Y_H;i++)
    {	
	  for(j=0;j< (X_W*2);j++)
 	  {
	   Check_Mem_WR_FIFO_not_Full();
	   RA8876_DataWrite(*data);
	   data++;
	  }
    }
  Check_Mem_WR_FIFO_Empty();
#endif

#ifdef MCU_8bit_ColorDepth_24bpp	
	for(i=0;i< Y_H;i++)
    {	
	  for(j=0;j< (X_W*3);j++)
 	  {
	   Check_Mem_WR_FIFO_not_Full();
	   RA8876_DataWrite(*data);
	   data++;
	  }
    }
  Check_Mem_WR_FIFO_Empty();
#endif
	Check_BTE_Busy();
}



void BTE_MCU_Write_Chroma_key_MCU_16bit
(
unsigned long Des_Addr //start address of Destination
,unsigned short Des_W //image width of Destination (recommend = canvas image width)
,unsigned short XDes //coordinate X of Destination
,unsigned short YDes //coordinate Y of Destination
,unsigned long Background_color //transparency color
,unsigned short X_W //Width of BTE Window
,unsigned short Y_H //Length of BTE Window
,const unsigned short *data // 16-bit data
)
{	
  unsigned int i,j;

  
#ifdef MCU_16bit_ColorDepth_16bpp				//setting in UserDef.h
  Background_color_65k(Background_color);
#endif	
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_1		//setting in UserDef.h
  Background_color_16M(Background_color);
#endif	
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_2		//setting in UserDef.h
  Background_color_16M(Background_color);
#endif
  BTE_Destination_Memory_Start_Address(Des_Addr);
  BTE_Destination_Image_Width(Des_W);
  BTE_Destination_Window_Start_XY(XDes,YDes);
  
  BTE_Window_Size(X_W,Y_H);
  BTE_Operation_Code(0x04);		//BTE Operation: MPU Write with chroma keying (w/o ROP)
  BTE_Enable();
  RA8876_CmdWrite(0x04);			//Memory Data Read/Write Port
#ifdef MCU_16bit_ColorDepth_16bpp				//setting in UserDef.h
  				 		
	for(i=0;i< Y_H;i++)
    {	
	  for(j=0;j< X_W;j++)
 	  {
	    Check_Mem_WR_FIFO_not_Full();
		  RA8876_DataWrite(*data);  
	    data++;
	  }
    }
  Check_Mem_WR_FIFO_Empty();
#endif	

#ifdef MCU_16bit_ColorDepth_24bpp_Mode_1		//setting in UserDef.h
	for(i=0;i< Y_H;i++)
    {	
	  for(j=0;j< (X_W/2);j++)
 	  {
	    Check_Mem_WR_FIFO_not_Full();
		  RA8876_DataWrite(*data);  
	    data++;
		  Check_Mem_WR_FIFO_not_Full();
		  RA8876_DataWrite(*data);  
	    data++;
		  Check_Mem_WR_FIFO_not_Full();
		  RA8876_DataWrite(*data);  
	    data++;
	  }
    }
  Check_Mem_WR_FIFO_Empty(); 
#endif	
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_2		//setting in UserDef.h
	for(i=0;i< Y_H;i++)
    {	
	  for(j=0;j< (X_W);j++)
 	  {
	    Check_Mem_WR_FIFO_not_Full();
		  RA8876_DataWrite(*data);  
	    data++;
		  Check_Mem_WR_FIFO_not_Full();
		  RA8876_DataWrite(*data);  
	    data++;

	  }
    }
  Check_Mem_WR_FIFO_Empty();
#endif

	Check_BTE_Busy();

}



/*
No support 24bpp color depth 
S0_Addr : Source 0 Start address
S0_W : Source 0 image width(recommend = canvas image width) 
XS0 : Source 0 coordinate of X
YS0 : Source 0 coordinate of Y
Des_Addr : Destination start address
Des_W :	 Destination image width(recommend = canvas image width) 
XDes : Destination coordinate of X
YDes : Destination coordinate of Y
X_W : BTE Window Size of X
Y_H : BTE Window Size of Y
Foreground_color : The source (1bit map picture) map data 1 translate to Foreground color by color expansion
Background_color : The source (1bit map picture) map data 0 translate to background color by color expansion
*/
void BTE_Memory_Copy_ColorExpansion(unsigned long S0_Addr,unsigned short S0_W,unsigned short XS0,unsigned short YS0,unsigned long Des_Addr,unsigned short Des_W, unsigned short XDes,unsigned short YDes,unsigned short X_W,unsigned short Y_H,unsigned long Foreground_color,unsigned long Background_color)
{

#ifdef MCU_8bit_ColorDepth_8bpp
    Foreground_color_256(Foreground_color);
    Background_color_256(Background_color);
	BTE_ROP_Code(7);	//MCU data bus width: 8bit, ROP Code=7
#endif
#ifdef MCU_8bit_ColorDepth_16bpp
    Foreground_color_65k(Foreground_color);
    Background_color_65k(Background_color);
	BTE_ROP_Code(15);	//MCU data bus width: 8bit, ROP Code=7
#endif
#ifdef MCU_16bit_ColorDepth_16bpp
    Foreground_color_65k(Foreground_color);
    Background_color_65k(Background_color);
	BTE_ROP_Code(15);	//MCU data bus width: 16bit, ROP Code=7
#endif


  BTE_S0_Memory_Start_Address(S0_Addr);
  BTE_S0_Image_Width(S0_W);
  BTE_S0_Window_Start_XY(XS0,YS0);
  
  BTE_Destination_Memory_Start_Address(Des_Addr);
  BTE_Destination_Image_Width(Des_W);
  BTE_Destination_Window_Start_XY(XDes,YDes);
   
  BTE_Window_Size(X_W,Y_H);
  BTE_Operation_Code(0xE);		//BTE Operation: Memory write with Color Expansion (w/o ROP)

#ifdef MCU_8bit_ColorDepth_8bpp
  BTE_Enable();
  Check_BTE_Busy();
#endif
#ifdef MCU_8bit_ColorDepth_16bpp
  BTE_Enable();
  Check_BTE_Busy();
#endif
#ifdef MCU_16bit_ColorDepth_16bpp
  BTE_Enable();
  Check_BTE_Busy();
#endif	

}


/*
No support 24bpp color depth
S0_Addr : Source 0 Start address
S0_W : Source 0 image width(recommend = canvas image width) 
XS0 : Source 0 coordinate of X
YS0 : Source 0 coordinate of Y
Des_Addr : Destination start address
Des_W :	 Destination image width(recommend = canvas image width) 
XDes : Destination coordinate of X
YDes : Destination coordinate of Y
X_W : BTE Window Size of X
Y_H : BTE Window Size of Y
Foreground_color : The source (1bit map picture) map data 1 translate to Foreground color by color expansion
*/
void BTE_Memory_Copy_ColorExpansion_Chroma_key(unsigned long S0_Addr,unsigned short S0_W,unsigned short XS0,unsigned short YS0,unsigned long Des_Addr,unsigned short Des_W, unsigned short XDes,unsigned short YDes,unsigned short X_W,unsigned short Y_H,unsigned long Foreground_color)
{
#ifdef MCU_8bit_ColorDepth_8bpp
    Foreground_color_256(Foreground_color);
	BTE_ROP_Code(7);	//MCU data bus width: 8bit, ROP Code=7
#endif
#ifdef MCU_8bit_ColorDepth_16bpp
    Foreground_color_65k(Foreground_color);
	BTE_ROP_Code(15);	//MCU data bus width: 8bit, ROP Code=7
#endif
#ifdef MCU_16bit_ColorDepth_16bpp
    Foreground_color_65k(Foreground_color);
	BTE_ROP_Code(15);	//MCU data bus width: 16bit, ROP Code=7
#endif

  BTE_S0_Memory_Start_Address(S0_Addr);
  BTE_S0_Image_Width(S0_W);
  BTE_S0_Window_Start_XY(XS0,YS0);

  BTE_Destination_Memory_Start_Address(Des_Addr);
  BTE_Destination_Image_Width(Des_W);
  BTE_Destination_Window_Start_XY(XDes,YDes);
   
  BTE_Window_Size(X_W,Y_H);
  BTE_Operation_Code(0xF);		//BTE Operation: Memory write with Color Expansion and chroma keying (w/o ROP)
	
#ifdef MCU_8bit_ColorDepth_8bpp
  BTE_Enable();
  Check_BTE_Busy();
#endif
#ifdef MCU_8bit_ColorDepth_16bpp
  BTE_Enable();
  Check_BTE_Busy();
#endif
#ifdef MCU_16bit_ColorDepth_16bpp
  BTE_Enable();
  Check_BTE_Busy();
#endif	

}


void BTE_MCU_Write_ColorExpansion_MCU_8bit
(
unsigned long Des_Addr //start address of Destination
,unsigned short Des_W //image width of Destination (recommend = canvas image width)
,unsigned short XDes //coordinate X of Destination
,unsigned short YDes //coordinate Y of Destination
,unsigned short X_W //Width of BTE Window
,unsigned short Y_H //Length of BTE Window
,unsigned long Foreground_color 
/*Foreground_color : The source (1bit map picture) map data 1 translate to Foreground color by color expansion*/
,unsigned long Background_color 
/*Background_color : The source (1bit map picture) map data 0 translate to Foreground color by color expansion*/
,const unsigned char *data // 8-bit data
)
{    
    unsigned short i,j;
	
	/* set BTE Parameters and Run */
#ifdef MCU_8bit_ColorDepth_8bpp
    Foreground_color_256(Foreground_color);
    Background_color_256(Background_color);
#endif
#ifdef MCU_8bit_ColorDepth_16bpp
    Foreground_color_65k(Foreground_color);
    Background_color_65k(Background_color);
#endif
#ifdef MCU_8bit_ColorDepth_24bpp
    Foreground_color_16M(Foreground_color);
    Background_color_16M(Background_color);
#endif 

	BTE_Destination_Memory_Start_Address(Des_Addr);
	BTE_Destination_Image_Width(Des_W);
	BTE_Destination_Window_Start_XY(XDes,YDes);

	BTE_ROP_Code(7);	//MCU data bus width: 8bit, ROP Code=7
	BTE_Window_Size(X_W,Y_H);
	BTE_Operation_Code(0x8);		//BTE Operation: MPU Write with Color Expansion (w/o ROP)

	BTE_Enable();		
		
  RA8876_CmdWrite(0x04);				 		//Memory Data Read/Write Port
    
	for(i=0;i< Y_H;i++)
    {	
	  for(j=0;j< X_W/8;j++)
 	  {
	    Check_Mem_WR_FIFO_not_Full();
		  RA8876_DataWrite(*data);  
	    data++;
	  }
    }
	Check_Mem_WR_FIFO_Empty();
	  
	Check_BTE_Busy();


}

void BTE_MCU_Write_ColorExpansion_MCU_16bit
(
unsigned long Des_Addr //start address of Destination
,unsigned short Des_W //image width of Destination (recommend = canvas image width)
,unsigned short XDes //coordinate X of Destination
,unsigned short YDes //coordinate Y of Destination
,unsigned short X_W //Width of BTE Window
,unsigned short Y_H //Length of BTE Window
,unsigned long Foreground_color 
/*Foreground_color : The source (1bit map picture) map data 1 translate to Foreground color by color expansion*/
,unsigned long Background_color 
/*Background_color : The source (1bit map picture) map data 0 translate to Background color by color expansion*/
,const unsigned short *data //16-bit data
)
{ 
  unsigned short i,j;


#ifdef MCU_16bit_ColorDepth_16bpp
	Data_Format_16b_16bpp();
    Foreground_color_65k(Foreground_color);
    Background_color_65k(Background_color);
	BTE_ROP_Code(15);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_1
  Data_Format_16b_8bpp();
  Foreground_color_16M(Foreground_color);
  Background_color_16M(Background_color);
  BTE_ROP_Code(7);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_2
  Data_Format_16b_8bpp();
  Foreground_color_16M(Foreground_color);
  Background_color_16M(Background_color);
  BTE_ROP_Code(7);
#endif

  BTE_Destination_Memory_Start_Address(Des_Addr);
  BTE_Destination_Image_Width(Des_W);
  BTE_Destination_Window_Start_XY(XDes,YDes);

  BTE_Window_Size(X_W,Y_H);
  BTE_Operation_Code(0x8);		//BTE Operation: MPU Write with Color Expansion (w/o ROP)
  BTE_Enable();
	

#ifdef MCU_16bit_ColorDepth_16bpp
  RA8876_CmdWrite(0x04);				 		//Memory Data Read/Write Port  
	for(i=0;i< Y_H;i++)
    {	
	  for(j=0;j< X_W/16;j++)
 	  {
	    Check_Mem_WR_FIFO_not_Full();
		  RA8876_DataWrite(*data);  
	    data++;
	  }
    }
  Check_Mem_WR_FIFO_Empty();
#endif

#ifdef MCU_16bit_ColorDepth_24bpp_Mode_1
	RA8876_CmdWrite(0x04);				 		//Memory Data Read/Write Port 
	for(i=0;i< Y_H;i++)
    {	
	  for(j=0;j< X_W/16;j++)
 	  {
	    Check_Mem_WR_FIFO_not_Full();
			RA8876_DataWrite(*data>>8);  

	    Check_Mem_WR_FIFO_not_Full();
			RA8876_DataWrite(*data);  
	    data++;
	  }
    }
	Check_Mem_WR_FIFO_Empty();
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_2
	RA8876_CmdWrite(0x04);				 		//Memory Data Read/Write Port 
	for(i=0;i< Y_H;i++)
    {	
	  for(j=0;j< X_W/16;j++)
 	  {
	    Check_Mem_WR_FIFO_not_Full();
			RA8876_DataWrite(*data>>8);  

	    Check_Mem_WR_FIFO_not_Full();
			RA8876_DataWrite(*data);  
	    data++;
	  }
    }
	Check_Mem_WR_FIFO_Empty();
#endif	

    Check_BTE_Busy();			

#ifdef MCU_16bit_ColorDepth_16bpp	
	Data_Format_16b_16bpp();
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_1	
	Data_Format_16b_24bpp_mode1();
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_2	
	Data_Format_16b_24bpp_mode2();
#endif
}




void BTE_MCU_Write_ColorExpansion_Chroma_key_MCU_8bit
(
unsigned long Des_Addr //start address of Destination
,unsigned short Des_W //image width of Destination (recommend = canvas image width) 
,unsigned short XDes //coordinate X of Destination
,unsigned short YDes //coordinate Y of Destination
,unsigned short X_W //Width of BTE Window
,unsigned short Y_H //Length of BTE Window
,unsigned long Foreground_color 
/*Foreground_color : The source (1bit map picture) map data 1 translate to Foreground color by color expansion*/
,const unsigned char *data //8-bit data
)
{
  unsigned short i,j;

//  Data_Format_8b_8bpp();
	
	/* set BTE Parameters and Run */
#ifdef MCU_8bit_ColorDepth_8bpp
  Foreground_color_256(Foreground_color);
#endif
#ifdef MCU_8bit_ColorDepth_16bpp
  Foreground_color_65k(Foreground_color);
#endif
#ifdef MCU_8bit_ColorDepth_24bpp
  Foreground_color_16M(Foreground_color);
#endif 

	BTE_Destination_Memory_Start_Address(Des_Addr);
	BTE_Destination_Image_Width(Des_W);
	BTE_Destination_Window_Start_XY(XDes,YDes);

	BTE_ROP_Code(7);	//MCU data bus width: 8bit, ROP Code=7
	BTE_Window_Size(X_W,Y_H);
	BTE_Operation_Code(0x9);		//BTE Operation: MPU Write with Color Expansion and chroma keying (w/o ROP)

	BTE_Enable();		
		
  RA8876_CmdWrite(0x04);				 		//Memory Data Read/Write Port
    
	for(i=0;i< Y_H;i++)
  {	
	  for(j=0;j< X_W/8;j++)
 	  {
	    Check_Mem_WR_FIFO_not_Full();
		  RA8876_DataWrite(*data);  
	    data++;
	  }
  }
	Check_Mem_WR_FIFO_Empty();
	  
	Check_BTE_Busy();


}



void BTE_MCU_Write_ColorExpansion_Chroma_key_MCU_16bit
(
unsigned long Des_Addr //start address of Destination
,unsigned short Des_W //image width of Destination (recommend = canvas image width)
,unsigned short XDes //coordinate X of Destination
,unsigned short YDes //coordinate Y of Destination
,unsigned short X_W //Width of BTE Window
,unsigned short Y_H //Length of BTE Window
,unsigned long Foreground_color
/*Foreground_color : The source (1bit map picture) map data 1 translate to Foreground color by color expansion*/
,const unsigned short *data //16-bit data
)
{
  unsigned short i,j;

#ifdef MCU_16bit_ColorDepth_16bpp
	Data_Format_16b_16bpp();
    Foreground_color_65k(Foreground_color);
	BTE_ROP_Code(15);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_1
	Data_Format_16b_8bpp();
	Foreground_color_16M(Foreground_color);
	BTE_ROP_Code(7);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_2
	Data_Format_16b_8bpp();
	Foreground_color_16M(Foreground_color);
	BTE_ROP_Code(7);
#endif

	BTE_Destination_Memory_Start_Address(Des_Addr);
	BTE_Destination_Image_Width(Des_W);
	BTE_Destination_Window_Start_XY(XDes,YDes);


	BTE_Window_Size(X_W,Y_H);
	BTE_Operation_Code(0x9);		//BTE Operation: MPU Write with Color Expansion and chroma keying (w/o ROP)
	BTE_Enable();	
 	RA8876_CmdWrite(0x04);				 		//Memory Data Read/Write Port

#ifdef MCU_16bit_ColorDepth_16bpp						//setting in UserDef.h
	for(i=0;i< Y_H;i++)
  {	
	  for(j=0;j< X_W/16;j++)
 	  {
	    Check_Mem_WR_FIFO_not_Full();
		  RA8876_DataWrite(*data);  
	    data++;
	  }
  }
	Check_Mem_WR_FIFO_Empty();
#endif	

#ifdef MCU_16bit_ColorDepth_24bpp_Mode_1				//setting in UserDef.h
	for(i=0;i< Y_H;i++)
  {	
	  for(j=0;j< X_W/16;j++)
 	  {
	    Check_Mem_WR_FIFO_not_Full();
			RA8876_DataWrite(*data>>8);  

	    Check_Mem_WR_FIFO_not_Full();
			RA8876_DataWrite(*data);  
	    data++;
	  }
  }
	Check_Mem_WR_FIFO_Empty();
#endif	

#ifdef MCU_16bit_ColorDepth_24bpp_Mode_2				//setting in UserDef.h
	for(i=0;i< Y_H;i++)
  {	
	  for(j=0;j< X_W/16;j++)
 	  {
	    Check_Mem_WR_FIFO_not_Full();
			RA8876_DataWrite(*data>>8);  

	    Check_Mem_WR_FIFO_not_Full();
			RA8876_DataWrite(*data);  
	    data++;
	  }
  }
	Check_Mem_WR_FIFO_Empty();
#endif
	Check_BTE_Busy();			


}



void BTE_Solid_Fill
(
unsigned long Des_Addr //start address of destination 
,unsigned short Des_W // image width of destination (recommend = canvas image width) 
, unsigned short XDes //coordinate X of destination 
,unsigned short YDes //coordinate Y of destination 
,unsigned long Foreground_color //Solid Fill color
,unsigned short X_W //Width of BTE Window
,unsigned short Y_H //Length of BTE Window 
)
{

#ifdef MCU_8bit_ColorDepth_8bpp	
  Foreground_color_256(Foreground_color);
#endif
#ifdef MCU_8bit_ColorDepth_16bpp	
  Foreground_color_65k(Foreground_color);
#endif
#ifdef MCU_8bit_ColorDepth_24bpp	
  Foreground_color_16M(Foreground_color);
#endif

#ifdef MCU_16bit_ColorDepth_16bpp	
  Foreground_color_65k(Foreground_color);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_1	
  Foreground_color_16M(Foreground_color);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_2	
  Foreground_color_16M(Foreground_color);
#endif
  BTE_Destination_Memory_Start_Address(Des_Addr);
  BTE_Destination_Image_Width(Des_W);
  BTE_Destination_Window_Start_XY(XDes,YDes);
  BTE_Window_Size(X_W,Y_H);
  BTE_Operation_Code(0x0c);		//BTE Operation: Solid Fill (w/o ROP)
  BTE_Enable();
  Check_BTE_Busy();  
}


/*
S0_Addr : Source 0 Start address
S0_W : Source 0 image width(recommend = canvas image width) 
XS0 : Source 0 coordinate of X
YS0 : Source 0 coordinate of Y
S1_Addr : Source 1 Start address
S1_W : Source 1 image width(recommend = canvas image width) 
XS1 : Source 1 coordinate of X
YS1 : Source 1 coordinate of Y
Des_Addr : Destination start address
Des_W :	 Destination image width(recommend = canvas image width) 
XDes : Destination coordinate of X
YDes : Destination coordinate of Y
X_W : BTE Window Size of X
Y_H : BTE Window Size of Y
alpha : Alpha Blending effect 0 ~ 32, Destination data = (Source 0 * (1- alpha)) + (Source 1 * alpha)
*/
void BTE_Alpha_Blending(unsigned long S0_Addr,unsigned short S0_W,unsigned short XS0,unsigned short YS0,unsigned long S1_Addr,unsigned short S1_W,unsigned short XS1,unsigned short YS1,unsigned long Des_Addr,unsigned short Des_W, unsigned short XDes,unsigned short YDes,unsigned short X_W,unsigned short Y_H,unsigned char alpha)
{
  BTE_S0_Memory_Start_Address(S0_Addr);
  BTE_S0_Image_Width(S0_W);
  BTE_S0_Window_Start_XY(XS0,YS0);

  BTE_S1_Memory_Start_Address(S1_Addr);
  BTE_S1_Image_Width(S1_W); 
  BTE_S1_Window_Start_XY(XS1,YS1);

  BTE_Destination_Memory_Start_Address(Des_Addr);
  BTE_Destination_Image_Width(Des_W);
  BTE_Destination_Window_Start_XY(XDes,YDes);

  BTE_Window_Size(X_W,Y_H);
  BTE_Operation_Code(0x0A);		//BTE Operation: Memory write with opacity (w/o ROP)
  BTE_Alpha_Blending_Effect(alpha);
  BTE_Enable();
  Check_BTE_Busy();
}


void DMA_24bit
(
unsigned char SCS //SCS : 0 = Use SCS0, 1 = Use SCS1
,unsigned char Clk //Clk : SPI Clock = System Clock /{(Clk+1)*2}
,unsigned short X1 //X of DMA Coordinate
,unsigned short Y1 //Y of DMA Coordinate 
,unsigned short X_W //DMA Block width
,unsigned short Y_H //DMA Block height
,unsigned short P_W //DMA Picture width
,unsigned long Addr //DMA Source Start address
)
{    Enable_SFlash_SPI();									   //Enable Serial Flash
	if(SCS == 0)
	{
		Select_SFI_0();										   //Select Serial Flash 0
	}
	if(SCS == 1)
	{														    
		Select_SFI_1();										/* 选择Serial Flash接口0 */
	}					   
	Select_SFI_DMA_Mode();									/* 设置为Serial Flash DMA模式 */
//	Select_SFI_24bit_Address();								/* 设置24位寻址模式 */		
//	Select_standard_SPI_Mode0_or_Mode3();					/* 标准 SPI 模式 0 或模式 3 时序图 */
//	Select_SFI_Single_Mode_Dummy_8T_0Bh();					/* 1x 读取命令0Bh。为faster read速度。数据是由xmiso输入，RA8876在地址与数据间会塞入 8 个空周期。 */

	SPI_Clock_Period(Clk);									/* Serial Flash spi时钟，8M spi flash 最大时钟频率为80Mhz */
	/////////////////////////////////////////****************DMA 
	Goto_Pixel_XY(X1,Y1);									//set Memory coordinate in Graphic Mode
	SFI_DMA_Destination_Upper_Left_Corner(X1,Y1);			//DMA Destination position(x,y)
	SFI_DMA_Transfer_Width_Height(X_W,Y_H);					//Set DMA Block (Height , Width)
	SFI_DMA_Source_Width(P_W);								//Set DMA Source Picture Width
	SFI_DMA_Source_Start_Address(Addr); 						  //Set Serial Flash DMA Source Starting Address
//	Canvas_image_width(800);			/* 底图宽度 */
//	Memory_16bpp_Mode();				
//	
//	Reset_CPOL();				/* 将CPOL时钟极性位置0 */
////	Set_CPOL();
//	Reset_CPHA();				/* 将CPHA时钟相位置0 */
////	Set_CPHA();
//	
//	//BTE_Operation_Code(0x0e);
//	Graphic_Mode();						/* 图形模式 */
//	Memory_XY_Mode();										/* 选择块模式 */
////	Memory_Linear_Mode();
	Start_SFI_DMA();									  //Start DMA
	Check_Busy_SFI_DMA();								  //DMA Busy Check
}


void DMA_32bit
(
unsigned char SCS //SCS : 0 = Use SCS0, 1 = Use SCS1
,unsigned char Clk //Clk : SPI Clock = System Clock /{(Clk+1)*2}
,unsigned short X1 //X of DMA Coordinate
,unsigned short Y1 //Y of DMA Coordinate 
,unsigned short X_W //DMA Block width
,unsigned short Y_H //DMA Block height
,unsigned short P_W //DMA Picture width
,unsigned long Addr //DMA Source Start address
)
{  

//  Enable_SFlash_SPI();									   //Enable Serial Flash
  if(SCS == 0)
  {
   Select_SFI_0();										   //Select Serial Flash 0
  }
  if(SCS == 1)
  {														    
   Select_SFI_1();										   //Select Serial Flash 1
  }	 
  Select_SFI_DMA_Mode();								   //Set Serial Flash DMA Mode
  SPI_Clock_Period(Clk);

  Select_SFI_32bit_Address();							   //Set Serial Flash/ROM 32bits Address
  /////////////////////////////////////////****************DMA 
  Goto_Pixel_XY(X1,Y1);									  //set Memory coordinate in Graphic Mode
  SFI_DMA_Destination_Upper_Left_Corner(X1,Y1);			  //DMA Destination position(x,y)
  SFI_DMA_Transfer_Width_Height(X_W,Y_H);				  //Set DMA Block (Height , Width)
  SFI_DMA_Source_Width(P_W);							  //Set DMA Source Picture Width
  SFI_DMA_Source_Start_Address(Addr); 						  //Set Serial Flash DMA Source Starting Address

  Start_SFI_DMA();									  //Start DMA
  Check_Busy_SFI_DMA();								  //DMA Busy Check
  Select_SFI_24bit_Address();
}


void switch_24bits_to_32bits(unsigned char SCS)
{

  if(SCS == 0)
  {
   Select_nSS_drive_on_xnsfcs0();
  }
  if(SCS == 1)
  {														    
   Select_nSS_drive_on_xnsfcs1();
  }	 
//**************************//data are read on the clock's rising edge(low△high transition)
//**************************//and data are changed on a falling edge (high△low clock transition) 
  Reset_CPOL();						   
  //Set_CPOL();
  Reset_CPHA();
  //Set_CPHA();
//**************************
  //Enter 4-byte mode
  								   
  nSS_Active();					   //nSS port will goes low
  SPI_Master_FIFO_Data_Put(0xB7);  //ち传32家Α

  bsp_DelayMS(1); 
  nSS_Inactive();				   //nSS port will goes high
}



/*
*********************************************************************************************************
*	函 数 名: PWM0
*	功能说明: PWM配置并输出
*	形    参：  on_off：PWM使能/失能  1/0
*		 Clock_Divided：PWM时钟分频 0-3(1,1/2,1/4,1/8)
*			 Prescalar：预分频 1-256
*		  Count_Buffer：PWM输出周期时间
*		Compare_Buffer：PWM输出高电平时间
*	返 回 值: 无
*********************************************************************************************************
*/
/*Such as the following formula :
PWM CLK = (Core CLK / Prescalar ) /2^ divided clock 
PWM output period = (Count Buffer + 1) x PWM CLK time
PWM output high level time = (Compare Buffer + 1) x PWM CLK time */
void PWM0(uint8_t on_off, uint8_t Clock_Divided, uint8_t Prescalar, uint16_t Count_Buffer, uint16_t Compare_Buffer)  
{
	Select_PWM0();								/* 使能并控制PWM定时器0 */
	Set_PWM_Prescaler_1_to_256(Prescalar);		/* 设置PWM预分频值 */

	if(Clock_Divided ==0)							/* PWM0时钟分屏 1/1 */
	{
		Select_PWM0_Clock_Divided_By_1();
	}
	if(Clock_Divided ==1)
	{
		Select_PWM0_Clock_Divided_By_2();			/* PWM0时钟分屏 1/2 */
	}
	if(Clock_Divided ==2)
	{
		Select_PWM0_Clock_Divided_By_4();			/* PWM0时钟分屏 1/4 */
	}
	if(Clock_Divided ==3)
	{
		Select_PWM0_Clock_Divided_By_8();			/* PWM0时钟分屏 1/8 */
	}

	Set_Timer0_Count_Buffer(Count_Buffer);  		/* 设置定时器0缓冲区值 */
    Set_Timer0_Compare_Buffer(Compare_Buffer);		/* 设置定时器0比较缓冲区值 */
	
	if (on_off == 1)
	{
		Start_PWM0();								/* 启动PWM0 */
	}	 
	if (on_off == 0)
	{
		Stop_PWM0();								/* 关闭PWM0 */
	}
}

/*
*********************************************************************************************************
*	函 数 名: PWM1
*	功能说明: PWM配置并输出
*	形    参：  on_off：PWM使能/失能  1/0
*		 Clock_Divided：PWM时钟分频 0-3(1,1/2,1/4,1/8)
*			 Prescalar：预分频 1-256
*		  Count_Buffer：PWM输出周期时间
*		Compare_Buffer：PWM输出高电平时间
*	返 回 值: 无
*********************************************************************************************************
*/
/*Such as the following formula :
PWM CLK = (Core CLK / Prescalar ) /2^ divided clock 
PWM output period = (Count Buffer + 1) x PWM CLK time
PWM output high level time = (Compare Buffer + 1) x PWM CLK time */

	//PWM1(1,3,100,4,3); 		/* 开PWM背光 */
void PWM1(uint8_t on_off, uint8_t Clock_Divided, uint8_t Prescalar, uint16_t Count_Buffer, uint16_t Compare_Buffer)  
{
#if 1
	Select_PWM1();								/* 使能并控制PWM定时器1 */
	Set_PWM_Prescaler_1_to_256(Prescalar);		/* 设置PWM预分频值 */

//	Auto_Reload_PWM1();		/* 自动重载 */
//	Enable_PWM1_Interrupt();	/* 开PWM1中断 */
//	Enable_PWM1_Inverter();		/* 开输出反向 */
	
	if(Clock_Divided ==0)
	{
		Select_PWM1_Clock_Divided_By_1();		/* PWM1时钟分频率 1/1 */
	}
	if(Clock_Divided ==1)
	{
		Select_PWM1_Clock_Divided_By_2();		/* PWM1时钟分频率 1/2 */
	}
	if(Clock_Divided ==2)
	{
		Select_PWM1_Clock_Divided_By_4();		/* PWM1时钟分频率 1/4 */
	}
	if(Clock_Divided ==3)
	{
		Select_PWM1_Clock_Divided_By_8();		/* PWM1时钟分频率 1/8 */
	}

	Set_Timer1_Count_Buffer(Count_Buffer);		/* 设置定时器0缓冲区值，输出周期时间 */
	Set_Timer1_Compare_Buffer(Compare_Buffer); 	/* 设置定时器0比较缓冲区值,输出高电平时间 */


#endif
	if (on_off == 1)
	{
		Start_PWM1();							/* 启动PWM1 */
	}	 
	if (on_off == 0)
	{
		Stop_PWM1();							/* 关闭PWM1 */
	}

}



// Note. this API does not support the case that MCU=16bit, 24bpp and mode1
void putPixel
(
unsigned short x //x of coordinate
,unsigned short y //y of coordinate
,unsigned long color 
/*color : 8bpp:R3G3B2
16bpp:R5G6B5
24bpp:R8G8B8 */
)

{

	Goto_Pixel_XY(x,y);
	RA8876_CmdWrite(0x04);	
  	Check_Mem_WR_FIFO_not_Full();

	#ifdef MCU_8bit_ColorDepth_8bpp	
	RA8876_DataWrite(color);
	#endif
	#ifdef MCU_8bit_ColorDepth_16bpp	
	RA8876_DataWrite(color);
	Check_Mem_WR_FIFO_not_Full();
	RA8876_DataWrite(color>>8);
	#endif
	#ifdef MCU_8bit_ColorDepth_24bpp	
	RA8876_DataWrite(color);
	Check_Mem_WR_FIFO_not_Full();
	RA8876_DataWrite(color>>8);
	Check_Mem_WR_FIFO_not_Full();
	RA8876_DataWrite(color>>16);
	#endif
	#ifdef MCU_16bit_ColorDepth_16bpp	
	RA8876_DataWrite(color);
	#endif
	#ifdef MCU_16bit_ColorDepth_24bpp_Mode_2	
	RA8876_DataWrite(color);
	Check_Mem_WR_FIFO_not_Full();
	RA8876_DataWrite(color>>16);
	#endif
}

#if 0
// Note. this API does not support the case that MCU=16bit, 24bpp and mode1
void lcdPutChar8x12
(
unsigned short x // x of coordinate
,unsigned short y // y of coordinate
,unsigned long fgcolor //fgcolor : foreground color(font color)
,unsigned long bgcolor //bgcolor : background color
/*8bpp:R3G3B2
16bpp:R5G6B5
24bpp:R8G8B8*/
, unsigned char bg_transparent 
/*bg_transparent = 0, background color with no transparent
bg_transparent = 1, background color with transparent*/
,unsigned char code //code : font char
)
{ 
  unsigned short i=0;
  unsigned short j=0;
  unsigned char tmp_char=0,LB;
  unsigned long temp1,temp2;

  for (i=0;i<12;i++)
  {
   tmp_char = ascii_table_8x12[((code-0x20)*12)+i];//minus 32 offset, because this table from ascii table "space" 

   for (j=0;j<8;j++)
   {

    if ( (tmp_char >>7-j) & 0x01 == 0x01)

        putPixel(x+j,y+i,fgcolor); //

    else
	{   

        if(!bg_transparent)

        putPixel(x+j,y+i,bgcolor); //

    } 


   }

  }

}


// Note. this API does not support the case that MCU=16bit, 24bpp and mode1
void lcdPutString8x12
(
unsigned short x //x of coordinate 
,unsigned short y //y of coordinate 
, unsigned long fgcolor //fgcolor : foreground color(font color)
,unsigned long bgcolor //bgcolor : background color
/*8bpp:R3G3B2
16bpp:R5G6B5
24bpp:R8G8B8*/
, unsigned char bg_transparent 
/*bg_transparent = 0, background color with no transparent
bg_transparent = 1, background color with transparent*/
,char *ptr //*ptr: font string
)
{
  unsigned short i = 0;
  //screen width = 800,  800/8 = 100 
  //if string more then 100 fonts, no show
  while ((*ptr != 0) & (i < 100))
  {
    lcdPutChar8x12(x, y, fgcolor, bgcolor,bg_transparent, *ptr);
    x += 8;
    ptr++;
    i++;  
  }

}

 
// Note. this API does not support the case that MCU=16bit, 24bpp and mode1
void lcdPutChar16x24
(
unsigned short x //x of coordinate
,unsigned short y //y of coordinate
,unsigned long fgcolor //fgcolor : foreground color(font color)
,unsigned long bgcolor //bgcolor : background color
/*8bpp:R3G3B2
16bpp:R5G6B5
24bpp:R8G8B8*/
, unsigned char bg_transparent
/*bg_transparent = 0, background color with no transparent
bg_transparent = 1, background color with transparent*/
,unsigned char code //code : font char
)
{ 
  unsigned short i=0;
  unsigned short j=0;
  unsigned long array_addr =0;
  unsigned int tmp_char=0;

  for (i=0;i<24;i++)
  {
    //minus 32 offset, because this table from ascii table "space"  
    array_addr = ((code-0x20)*2*24)+(i*2); 
    tmp_char = ascii_table_16x24[array_addr]<<8|ascii_table_16x24[array_addr+1];

   for (j=0;j<16;j++)
   {

    if ( (tmp_char >>15-j) & 0x01 == 0x01)

        putPixel(x+j,y+i,fgcolor); //

    else
	   {
	    if(!bg_transparent)

        putPixel(x+j,y+i,bgcolor); // 

       }
   }
  }
}

 
// Note. this API does not support the case that MCU=16bit, 24bpp and mode1
void lcdPutString16x24
(
unsigned short x //x of coordinate
,unsigned short y //y of coordinate
, unsigned long fgcolor //fgcolor : foreground color(font color)
, unsigned long bgcolor //bgcolor : background color
/*8bpp:R3G3B2
16bpp:R5G6B5
24bpp:R8G8B8*/
, unsigned char bg_transparent 
/*bg_transparent = 0, background color with no transparent
bg_transparent = 1, background color with transparent*/
,char *ptr //*ptr : font string
) 
{
  unsigned short i = 0;
  //screen width = 800,  800/16 = 50 
  //if string more then 50 fonts, no show
  while ((*ptr != 0) & (i < 50))
  {
    lcdPutChar16x24(x, y, fgcolor, bgcolor,bg_transparent, *ptr);
    x += 16;
    ptr++;
    i++;  
  }

}

 
// Note. this API does not support the case that MCU=16bit, 24bpp and mode1
void lcdPutChar32x48
(
unsigned short x //x of coordinate
,unsigned short y //y of coordinate
,unsigned long fgcolor //fgcolor : foreground color(font color)
,unsigned long bgcolor //bgcolor : background color
/*8bpp:R3G3B2
16bpp:R5G6B5
24bpp:R8G8B8*/
, unsigned char bg_transparent 
/*bg_transparent = 0, background color with no transparent 
bg_transparent = 1, background color with transparent*/
,unsigned char code //code : font char
)
{ 
  unsigned short i=0;
  unsigned short j=0;
  unsigned long array_addr =0;
  unsigned long tmp_char=0;

  for (i=0;i<48;i++)
  {

    //minus 32 offset, because this table from ascii table "space"  
	array_addr = ((code-0x20)*4*48)+(i*4); 
	tmp_char = ascii_table_32x48[array_addr]<<24|ascii_table_32x48[array_addr+1]<<16|ascii_table_32x48[array_addr+2]<<8|ascii_table_32x48[array_addr+3];
	     
     for (j=0;j<32;j++)
	 {

     if ( (tmp_char >> (31-j)) & 0x01 == 0x01)

       putPixel(x+j,y+i,fgcolor); //

     else

         {
		 if(!bg_transparent)
		    putPixel(x+j,y+i,bgcolor); // 
		 }
	 } 

  }

}

 
// Note. this API does not support the case that MCU=16bit, 24bpp and mode1
void lcdPutString32x48
(
unsigned short x //x of coordinate
,unsigned short y //y of coordinate
, unsigned long fgcolor //fgcolor : foreground color(font color)
, unsigned long bgcolor //bgcolor : background color
/*8bpp:R3G3B2
16bpp:R5G6B5
24bpp:R8G8B8*/
, unsigned char bg_transparent, 
/*bg_transparent = 0, background color with no transparent
bg_transparent = 1, background color with transparent*/
char *ptr //*ptr: font string
)
{
  unsigned short i = 0;
  //screen width = 800,  800/32 = 25 
  //if string more then 25 fonts, no show
  while ((*ptr != 0) & (i < 25))
  {
    lcdPutChar32x48(x, y, fgcolor, bgcolor,bg_transparent, *ptr);
    x += 32;
    ptr++;
    i++;  
  }

}
#endif

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
