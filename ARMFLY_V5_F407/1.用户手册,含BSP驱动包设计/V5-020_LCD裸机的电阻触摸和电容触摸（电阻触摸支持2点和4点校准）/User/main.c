/*
*********************************************************************************************************
*
*	模块名称 : 主程序模块
*	文件名称 : main.c
*	版    本 : V1.0
*	说    明 : LCD的电阻触摸和电容触摸（电阻触摸支持2点和4点校准）。
*              实验目的：
*                1. 学习LCD的电阻触摸和电容触摸。
*              实验内容：
*                1. 电容屏无需校准，电阻屏需要校准。
*                2. LCD界面实现了一个简单的画板功能，方便检测触摸是否准确。
*                3. 启动1个200ms的自动重装定时器，让LED2每200ms翻转一次。
*              实验操作：
*                1. 摇杆上键，增加LCD背景光亮度。
*                2. 摇杆下键，降低LCD背景光亮度。
*                3. 摇杆左键，电阻触摸屏校准。
*                4. 摇杆OK键，清屏。
*              注意事项：
*                1. 本实验推荐使用串口软件SecureCRT查看打印信息，波特率115200，数据位8，奇偶校验位无，停止位1。
*                2. 务必将编辑器的缩进参数和TAB设置为4来阅读本文件，要不代码显示不整齐。
*
*	修改记录 :
*		版本号   日期         作者        说明
*		V1.0    2020-11-15   Eric2013     1. CMSIS软包版本 V5.4.0
*                                         2. HAL库版本 V1.7.6
*                                         
*	Copyright (C), 2020-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/	
#include "bsp.h"			 /* 底层硬件驱动 */



/* 定义例程名和例程发布日期 */
#define EXAMPLE_NAME	"V5-LCD的电阻触摸和电容触摸（电阻触摸支持2点和4点校准）"
#define EXAMPLE_DATE	"2020-11-15"
#define DEMO_VER		"1.0"


static void PrintfHelp(void);
static void PrintfLogo(void);
static void DispFirstPage(void);


/*
*********************************************************************************************************
*	函 数 名: main
*	功能说明: c程序入口
*	形    参: 无
*	返 回 值: 错误代码(无需处理)
*********************************************************************************************************
*/
int main(void)
{
	uint16_t ucBright;	   	/* 背光亮度(0-255) */
	uint8_t ucKeyCode;		/* 按键代码 */
	uint8_t fRefresh;		/* 刷屏请求标志,1表示需要刷新 */
    FONT_T tFont;		    /* 定义一个字体结构体变量，用于设置字体参数 */
	char buf[64];
    uint16_t usAdcX, usAdcY;
	int16_t tpX, tpY;
    uint8_t ucTouch;		/* 触摸事件 */


	/* 设置字体参数 */
	{
		tFont.FontCode = FC_ST_16;		/* 字体代码 16点阵 */
		tFont.FrontColor = CL_WHITE;	/* 字体颜色 */
		tFont.BackColor = CL_BLUE;		/* 文字背景颜色 */
		tFont.Space = 0;				/* 文字间距，单位 = 像素 */
	}	
	
	bsp_Init();		/* 硬件初始化 */
	PrintfLogo();	/* 打印例程名称和版本等信息 */
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
	fRefresh = 1;	
	while (1)
	{
       bsp_Idle();
         
		/* 判断软件定时器0是否超时 */
		if(bsp_CheckTimer(0))
		{
			/* 每隔200ms 进来一次 */  
			bsp_LedToggle(2);
		}
		
        if (fRefresh)
		{
			fRefresh = 0;

			/* 实时刷新触摸ADC采样值和转换后的坐标 */
			{
				
				/* 读取并显示当前X轴和Y轴的ADC采样值 */
				usAdcX = TOUCH_ReadAdcX();
				usAdcY = TOUCH_ReadAdcY();
				sprintf(buf, "触摸ADC值 X = %4d, Y = %4d   ", usAdcX, usAdcY);
				LCD_DispStr(5, 40, buf, &tFont);

				/* 读取并显示当前触摸坐标 */
				tpX = TOUCH_GetX();
				tpY = TOUCH_GetY();
				sprintf(buf, "触摸坐标  X = %4d, Y = %4d   ", tpX, tpY);
				LCD_DispStr(5, 60, buf, &tFont);

				/* 在触笔所在位置显示一个小圈 */
				if ((tpX > 0) && (tpY > 0))
				{
					LCD_DrawCircle(tpX, tpY, 2, CL_YELLOW);
				}
			}

			/* 在屏幕边沿绘制2个矩形框(用于检测面板边缘像素是否正常) */
			LCD_DrawRect(0, 0, LCD_GetHeight(), LCD_GetWidth(), CL_WHITE);
			LCD_DrawRect(2, 2, LCD_GetHeight() - 4, LCD_GetWidth() - 4, CL_YELLOW);
            
            /* 显示背光值 */
			sprintf(buf, "当前背光亮度: %3d", ucBright);
			LCD_DispStr(5, 80, buf, &tFont);
		}
        
        ucTouch = TOUCH_GetKey(&tpX, &tpY);	/* 读取触摸事件 */
		if (ucTouch != TOUCH_NONE)
		{
			switch (ucTouch)
			{
				case TOUCH_DOWN:		/* 触笔按下事件 */
                  
					/* 在触笔所在位置显示一个小圈 */
					if ((tpX > 0) && (tpY > 0))
					{
						LCD_DrawCircle(tpX, tpY, 3, CL_RED);
					}
					break;

				case TOUCH_MOVE:		/* 触笔移动事件 */
					/* 实时刷新触摸ADC采样值和转换后的坐标 */
					{
						/* 读取并显示当前X轴和Y轴的ADC采样值 */
						usAdcX = TOUCH_ReadAdcX();
						usAdcY = TOUCH_ReadAdcY();
						sprintf(buf, "触摸ADC值 X = %4d, Y = %4d   ", usAdcX, usAdcY);
						LCD_DispStr(5, 40, buf, &tFont);

						/* 读取并显示当前触摸坐标 */
						//tpX = TOUCH_GetX();
						//tpY = TOUCH_GetY();
						sprintf(buf, "触摸坐标  X = %4d, Y = %4d   ", tpX, tpY);
						LCD_DispStr(5, 60, buf, &tFont);

						/* 在触笔所在位置显示一个小圈 */
						if ((tpX > 0) && (tpY > 0))
						{
							LCD_DrawCircle(tpX, tpY, 2, CL_YELLOW);
						}
					}
					break;

				case TOUCH_RELEASE:		/* 触笔释放事件 */
					/* 在触笔所在位置显示一个小圈 */
					if ((tpX > 0) && (tpY > 0))
					{
						LCD_DrawCircle(tpX, tpY, 4, CL_WHITE);
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
				case JOY_DOWN_L:		/* 摇杆LEFT键按下, 2点触摸校准 */
					TOUCH_Calibration(2);
					DispFirstPage();				
					fRefresh = 1;		/* 请求刷新LCD */
					break;

				case  JOY_DOWN_OK:		/* 摇杆OK键，清屏 */
					DispFirstPage();				
					fRefresh = 1;		/* 请求刷新LCD */
					break;

				case JOY_DOWN_U:		/* 摇杆UP键按下 */
					ucBright += BRIGHT_STEP;
					if (ucBright > BRIGHT_MAX)
					{
						ucBright = BRIGHT_MAX;
					}
					LCD_SetBackLight(ucBright);
                    fRefresh = 1;		/* 请求刷新LCD */
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
                    fRefresh = 1;		/* 请求刷新LCD */
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
	char buf[50];

	LCD_ClrScr(CL_BLUE);  		/* 清屏，背景蓝色 */
	
	
	/* 设置字体属性 */
	tFont.FontCode = FC_ST_16;		/* 字体选择宋体16点阵，高16x宽15) */
	tFont.FrontColor = CL_WHITE;	/* 字体颜色设置为白色 */
	tFont.BackColor = CL_MASK;	 	/* 文字背景颜色，透明 */
	tFont.Space = 0;				/* 字符水平间距, 单位 = 像素 */

	y = 4;
	usLineCap = 18; /* 行间距 */
	LCD_DispStr(5, y, "安富莱STM32-V5开发板  www.armfly.com", &tFont);
	y += usLineCap;
	LCD_DispStr(5, y, "电阻屏和电容屏实验，电容屏无需校准，电阻需要校准", &tFont);
	
	y += 5 * usLineCap;
	LCD_DispStr(30, y, "操作提示:", &tFont);
	
	y += usLineCap;
	LCD_DispStr(50, y, "摇杆上键 = 增加背光亮度", &tFont);
	
	y += usLineCap;
	LCD_DispStr(50, y, "摇杆下键 = 降低背光亮度", &tFont);
	
	y += usLineCap;
	LCD_DispStr(50, y, "摇杆左键 = 电阻屏触摸校准", &tFont);
	
	y += usLineCap;
	LCD_DispStr(50, y, "摇杆OK键 = 清屏", &tFont);

	y += 2 * usLineCap;	
	
    
    /* 显示TFT控制器型号和屏幕分辨率 */
    //LCD_GetChipDescribe(buf);	/* 读取TFT驱动芯片型号 */
    if (g_TouchType == CT_GT811)
    {	
        strcpy(buf, "STM32F407 + GT811");
    }
    else if (g_TouchType == CT_GT911)
    {	
        strcpy(buf, "STM32F407 + GT911");
    }
    else if (g_TouchType == CT_FT5X06)
    {
        strcpy(buf, "STM32F407 + FT5X06");
    }
    else if (g_TouchType == CT_STMPE811)
    {
        strcpy(buf, "STM32F407 + STMPE811");
    }
    else
    {					
        strcpy(buf, "STM32F407 + RA8875");				
    }	
    sprintf(&buf[strlen(buf)], "   %d x %d", LCD_GetWidth(), LCD_GetHeight());
    LCD_DispStr(5, y, (char *)buf, &tFont);
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
	printf("3. 摇杆左键，电阻触摸屏校准\r\n");
	printf("4. 摇杆OK键，清屏\r\n");
}

/*
*********************************************************************************************************
*	函 数 名: PrintfLogo
*	功能说明: 打印例程名称和例程发布日期, 接上串口线后，打开PC机的超级终端软件可以观察结果
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void PrintfLogo(void)
{
	printf("*************************************************************\n\r");
	
	/* 检测CPU ID */
	{
		uint32_t CPU_Sn0, CPU_Sn1, CPU_Sn2;
		
		CPU_Sn0 = *(__IO uint32_t*)(0x1FFF7A10);
		CPU_Sn1 = *(__IO uint32_t*)(0x1FFF7A10 + 4);
		CPU_Sn2 = *(__IO uint32_t*)(0x1FFF7A10 + 8);

		printf("\r\nCPU : STM32F407IGT6, LQFP176, 主频: %dMHz\r\n", SystemCoreClock / 1000000);
		printf("UID = %08X %08X %08X\n\r", CPU_Sn2, CPU_Sn1, CPU_Sn0);
	}

	printf("\n\r");
	printf("*************************************************************\n\r");
	printf("* 例程名称   : %s\r\n", EXAMPLE_NAME);	/* 打印例程名称 */
	printf("* 例程版本   : %s\r\n", DEMO_VER);		/* 打印例程版本 */
	printf("* 发布日期   : %s\r\n", EXAMPLE_DATE);	/* 打印例程日期 */

	/* 打印ST的HAL库版本 */
	printf("* HAL库版本  : V1.7.6 (STM32F407 HAL Driver)\r\n");
	printf("* \r\n");	/* 打印一行空格 */
	printf("* QQ    : 1295744630 \r\n");
	printf("* 旺旺  : armfly\r\n");
	printf("* Email : armfly@qq.com \r\n");
	printf("* 微信公众号: armfly_com \r\n");
	printf("* 淘宝店: armfly.taobao.com\r\n");
	printf("* Copyright www.armfly.com 安富莱电子\r\n");
	printf("*************************************************************\n\r");
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
