/*
*********************************************************************************************************
*
*	模块名称 : 主程序模块
*	文件名称 : main.c
*	版    本 : V1.0
*	说    明 : LCD的汉字小字库和全字库制作实验。
*              实验目的：
*                1. 学习LCD的汉字小字库和全字库制作实验。
*              实验内容：
*                1. 小字库和全字库通过此软件生成：
*                   http://forum.armfly.com/forum.php?mod=viewthread&tid=202 。 
*                2. LCD界面上展示ASCII字符和GB2312编码汉字。
*                3. 启动1个200ms的自动重装定时器，让LED2每200ms翻转一次。
*              实验操作：
*                1. 摇杆上键，增加LCD背景光亮度。
*                2. 摇杆下键，降低LCD背景光亮度。
*                3. 摇杆左键，显示上一页汉字。
*                4. 摇杆右键，显示下一页汉字。
*                5. 摇杆OK键，返回首页。
*              注意事项：
*                1. 本实验推荐使用串口软件SecureCRT查看打印信息，波特率115200，数据位8，奇偶校验位无，停止位1。
*                2. 务必将编辑器的缩进参数和TAB设置为4来阅读本文件，要不代码显示不整齐。
*
*	修改记录 :
*		版本号   日期         作者        说明
*		V1.0    2019-04-08   Eric2013     1. CMSIS软包版本 V5.4.0
*                                         2. HAL库版本 V1.3.0
*                                         
*	Copyright (C), 2018-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/	
#include "bsp.h"			 /* 底层硬件驱动 */


/* 定义例程名和例程发布日期 */
#define EXAMPLE_NAME	"V7-LCD的汉字小字库和全字库制作实验"
#define EXAMPLE_DATE	"2019-04-08"
#define DEMO_VER		"1.0"

#define	DEMO_PAGE_COUNT	89	/* Demo界面个数 */

static void PrintfHelp(void);
static void DispFirstPage(void);
static void DispAsciiDot(void);
static void DispHZK16(uint8_t _ucIndex);

/*
*********************************************************************************************************
*	函 数 名: demo_tft_lcd
*	功能说明: lcd测试
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void demo_tft_lcd(void)
{
	uint16_t ucBright;	   	/* 背光亮度(0-255) */
	uint8_t ucKeyCode;		/* 按键代码 */
	uint8_t ucStatus;		/* 主程序状态字 */
	uint8_t fRefresh;		/* 刷屏请求标志,1表示需要刷新 */
	

	PrintfHelp();	/* 打印操作提示 */

	/* 延迟200ms再点亮背光，避免瞬间高亮 */
	bsp_DelayMS(200); 
	
	DispFirstPage();	/* 显示第1页 */
	
	/* 界面整体显示完毕后，再打开背光，设置为缺省亮度 */
	bsp_DelayMS(100); 
	ucBright = BRIGHT_DEFAULT;
	LCD_SetBackLight(ucBright);
	
	bsp_StartAutoTimer(0, 200); /* 启动1个200ms的自动重装的定时器，软件定时器0 */
	
	/* 进入主程序循环体 */
	ucStatus = 0;
	fRefresh = 0;	
	while (1)
	{
		/* 判断软件定时器0是否超时 */
		if(bsp_CheckTimer(0))
		{
			/* 每隔200ms 进来一次 */  
			bsp_LedToggle(2);
		}
		
		if (fRefresh == 1)
		{
			fRefresh = 0;

			switch (ucStatus)
			{
				case 0:
					DispFirstPage();	/* 显示第1页 */
					break;

				case 1:
					DispAsciiDot();		/* 显示ASCII点阵 */
					break;

				default:
					/* 区码范围 ：1 - 87 */
					if (ucStatus <= 89)
					{
						DispHZK16(ucStatus);	/* 显示一个区的94个汉字 */
					}
					break;
			}
		}

		ucKeyCode = bsp_GetKey();	/* 读取键值, 无键按下时返回 KEY_NONE = 0 */
		if (ucKeyCode != KEY_NONE)
		{
			/* 有键按下 */
			switch (ucKeyCode)
			{
				case JOY_DOWN_L:		/* 摇杆LEFT键按下 */
					if (ucStatus > 0)
					{
						ucStatus--;
					}
					fRefresh = 1;		/* 请求刷新LCD */
					break;

				case JOY_DOWN_R:		/* 摇杆RIGHT键按下 */
					if (ucStatus < DEMO_PAGE_COUNT - 1)
					{
						ucStatus++;
					}
					fRefresh = 1;		/* 请求刷新LCD */
					break;

				case  JOY_DOWN_OK:		/* 摇杆OK键 */
					ucStatus = 0;		/* 返回首页 */					
					fRefresh = 1;		/* 请求刷新LCD */
					break;

				case JOY_DOWN_U:		/* 摇杆UP键按下 */
					ucBright += BRIGHT_STEP;
					if (ucBright > BRIGHT_MAX)
					{
						ucBright = BRIGHT_MAX;
					}
					LCD_SetBackLight(ucBright);
					printf("当前背景光亮度 : %d\r\n", ucBright);
					break;

				case JOY_DOWN_D:		/* 摇杆DOWN键按下 */
					if (ucBright < BRIGHT_STEP)
					{
						ucBright = 0;
					}
					else
					{
						ucBright -= BRIGHT_STEP;
					}
					LCD_SetBackLight(ucBright);
					printf("当前背景光亮度 : %d\r\n", ucBright);
					break;

				default:
					break;
			}
		}
	}
}

/*
*********************************************************************************************************
*	函 数 名: DispFirstPage
*	功能说明: 显示操作提示
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DispFirstPage(void)
{
	FONT_T tFont;		/* 定义一个字体结构体变量，用于设置字体参数 */
	uint16_t y;			/* Y坐标 */
	uint16_t usLineCap;	/* 行高 */
	uint8_t buf[50];

	LCD_ClrScr(CL_BLUE);  		/* 清屏，背景蓝色 */
	
	
	/* 设置字体属性 */
	tFont.FontCode = FC_ST_16;		/* 字体选择宋体16点阵，高16x宽15) */
	tFont.FrontColor = CL_WHITE;	/* 字体颜色设置为白色 */
	tFont.BackColor = CL_MASK;	 	/* 文字背景颜色，透明 */
	tFont.Space = 0;				/* 字符水平间距, 单位 = 像素 */

	y = 0;
	usLineCap = 18; /* 行间距 */
	LCD_DispStr(5, y, "安富莱STM32-V7开发板  www.armfly.com", &tFont);
	y += usLineCap;
	LCD_DispStr(5, y, "汉字小字库和全字库测试实验", &tFont);
	
	y += 2 * usLineCap;
	LCD_DispStr(30, y, "操作提示:", &tFont);
	
	y += usLineCap;
	LCD_DispStr(50, y, "摇杆上键 = 增加背光亮度", &tFont);
	
	y += usLineCap;
	LCD_DispStr(50, y, "摇杆下键 = 降低背光亮度", &tFont);
	
	y += usLineCap;
	LCD_DispStr(50, y, "摇杆左键 = 向前翻页", &tFont);
	
	y += usLineCap;
	LCD_DispStr(50, y, "摇杆右键 = 向后翻页", &tFont);
	
	y += usLineCap;
	LCD_DispStr(50, y, "摇杆OK键 = 返回首页", &tFont);

	y += 2 * usLineCap;	
	
	sprintf((char *)buf, "显示器分辨率 ：%dx%d", g_LcdWidth, g_LcdHeight);
	LCD_DispStr(5, y, (char *)buf, &tFont);
	
	y += usLineCap;
	LCD_DispStr(5, y, "每行可以显示25个汉字，或50个字符", &tFont);
}

/*
*********************************************************************************************************
*	函 数 名: DispAsciiDot
*	功能说明: 显示ASCII点阵
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DispAsciiDot(void)
{
	uint8_t i,k;
	FONT_T tFont;		/* 定义一个字体结构体变量，用于设置字体参数 */
	uint16_t x;			/* X坐标 */
	uint16_t y;			/* Y坐标 */	
	char buf[32 + 1];
	uint8_t ascii;

	LCD_ClrScr(CL_BLUE);  		/* 清屏，背景蓝色 */
	
	/* 设置字体属性 */
	tFont.FontCode = FC_ST_16;		/* 字体选择宋体16点阵，高16x宽15) */
	tFont.FrontColor = CL_WHITE;	/* 字体颜色设置为白色 */
	tFont.BackColor = CL_MASK;	 	/* 文字背景颜色，透明 */
	tFont.Space = 2;				/* 字符水平间距, 单位 = 像素 */

	LCD_DispStr(50, 0, "16点阵ASCII码字库，代码1-127", &tFont);

	x = 50;
	y = 40;
	ascii = 0;
	for (i = 0; i < 4; i++)
	{
		for (k = 0; k < 32; k++)
		{
			buf[k] = ascii++;
		}
		buf[32] = 0;
		if (buf[0] == 0)
		{
			buf[0] = ' ';
		}
		LCD_DispStr(x, y, buf, &tFont);
		y += 20;
	}
}

/*
*********************************************************************************************************
*	函 数 名: DispHZK16
*	功能说明: 显示16点阵汉字阵
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DispHZK16(uint8_t _ucIndex)
{
	uint8_t i,k;
	uint16_t x,y;
	char buf[50 + 1];
	uint8_t code1,code2;	/* 汉字内码 */
	uint8_t usLineCap = 18;
	FONT_T tFont;			/* 定义一个字体结构体变量，用于设置字体参数 */

	printf(" Display HZK Area Code = %d\r\n", _ucIndex - 1);

	if (_ucIndex == 2)
	{
		/* 第1次清屏，以后显示位置不变，可以不清屏，避免闪烁 */
		LCD_ClrScr(CL_BLUE);  		/* 清屏，背景蓝色 */
	}
	
	/* 设置字体属性 */
	tFont.FontCode = FC_ST_16;		/* 字体选择宋体16点阵，高16x宽15) */
	tFont.FrontColor = CL_WHITE;	/* 字体颜色设置为白色 */
	tFont.BackColor = CL_BLUE;	 	/* 文字背景颜色，蓝色 */
	tFont.Space = 0;				/* 字符水平间距, 单位 = 像素 */

	y = 0;
	LCD_DispStr(20, y, "国标GB2312 16点阵汉字库(区码1-87，位码1-94)", &tFont);

	code1 = _ucIndex - 1; /* 得到区码 */
	code2 = 1;	/* 位码从1开始 */

	y += usLineCap;
	sprintf((char *)buf, (char *)"当前区码: %2d, 本页位码:1-94, 第10-15区无字符", code1);
	LCD_DispStr(20, y, buf, &tFont);
	y += (2 * usLineCap);

	/*
		机内码高位 = 区码 + 0xA0
		机内码低位 = 位码 + 0xA0

		区码范围 ：1 - 87
		位码范围 : 1 - 94

		每行显示20个汉字，一个区是94个汉字，需要5行显示,第5行显示14个汉字
	*/
	
	x = 40;
	code1 += 0xA0;	/* 换算到内码高位 */
	code2 = 0xA1;	/* 内码低位起始 */
	for (i = 0; i < 5; i++)
	{
		for (k = 0; k < 20; k++)
		{
			buf[2 * k] = code1;
			buf[2 * k + 1] = code2;
			code2++;
			if ((i == 4) && (k == 13))
			{
				k++;
				break;
			}
		}
		buf[2 * k] = 0;
		LCD_DispStr(x, y, buf, &tFont);
		y += usLineCap;
	}
}

/*
*********************************************************************************************************
*	函 数 名: PrintfHelp
*	功能说明: 打印操作提示
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void PrintfHelp(void)
{
	printf("操作提示:\r\n");
	printf("1. 摇杆上键，增加LCD背景光亮度\r\n");
	printf("2. 摇杆下键，降低LCD背景光亮度\r\n");
	printf("3. 摇杆左键，显示上一页汉字\r\n");
	printf("4. 摇杆右键，显示下一页汉字\r\n");
	printf("5. 摇杆OK键，返回首页\r\n");
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
