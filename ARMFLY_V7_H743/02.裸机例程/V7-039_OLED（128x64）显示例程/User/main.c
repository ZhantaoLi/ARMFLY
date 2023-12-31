/*
*********************************************************************************************************
*
*	模块名称 : 主程序模块
*	文件名称 : main.c
*	版    本 : V1.0
*	说    明 : OLED显示实现。
*              实验目的：
*                1. 学习OLED的驱动和界面设置。
*              实验内容：
*                每个都测试了四种情况
*                1. 摇杆上下键 ： 调节对比度。
*                2. 摇杆左右键 ： 切换显示界面。
*                3. 摇杆OK键   ： 切换显示方向（显示内容翻转180度)。
*              注意事项：
*                1. OLED采用的FMC接口，8bit方式。
*                2. 本实验推荐使用串口软件SecureCRT查看打印信息，波特率115200，数据位8，奇偶校验位无，停止位1。
*                3. 务必将编辑器的缩进参数和TAB设置为4来阅读本文件，要不代码显示不整齐。
*
*	修改记录 :
*		版本号   日期         作者        说明
*		V1.0    2019-07-15   Eric2013     1. CMSIS软包版本 V5.4.0
*                                         2. HAL库版本 V1.3.0
*
*	Copyright (C), 2018-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/	
#include "bsp.h"			 /* 底层硬件驱动 */



/* 定义例程名和例程发布日期 */
#define EXAMPLE_NAME	"V7-OLED（128x64）显示器例程"
#define EXAMPLE_DATE	"2019-07-15"
#define DEMO_VER		"1.0"

#define DEMO_PAGE_COUNT		8	/* 演示页面的个数 */

/* 仅允许本文件内调用的函数声明 */
static void PrintfLogo(void);
static void DispMenu(void);

/*
*********************************************************************************************************
*	函 数 名: main
*	功能说明: c程序入口
*	形    参：无
*	返 回 值: 错误代码(无需处理)
*********************************************************************************************************
*/
int main(void)
{
	FONT_T tFont12, tFont16, tFont24, tFont32;
	uint8_t ucKeyCode;
	uint8_t ucItem;
	uint8_t ucDispReq;	/* 刷屏请求标志 */
	uint8_t ucContrast = 0x80;
	uint8_t ucDir = 0;	/* 显示方向, 0 表示正常方向，1表示倒180度 */

	/*
		ST固件库中的启动文件已经执行了 SystemInit() 函数，该函数在 system_stm32f4xx.c 文件，主要功能是
		配置CPU系统的时钟，内部Flash访问时序，配置FSMC用于外部SRAM
	*/

	bsp_Init();		/* 硬件初始化 */
	PrintfLogo();	/* 打印例程信息到串口1 */

	DispMenu();		/* 打印操作提示 */

	OLED_InitHard();	 /* 初始化OLED硬件 */
	OLED_ClrScr(0x00);	 /* 清屏，0x00表示黑底； 0xFF 表示白底 */

	/* 设置字体参数 */
	{
		tFont16.FontCode = FC_ST_16;	/* 字体代码 16点阵 */
		tFont16.FrontColor = 1;		/* 字体颜色 0 或 1 */
		tFont16.BackColor = 0;		/* 文字背景颜色 0 或 1 */
		tFont16.Space = 0;			/* 文字间距，单位 = 像素 */

		tFont12.FontCode = FC_ST_12;	/* 字体代码 12点阵 */
		tFont12.FrontColor = 1;		/* 字体颜色 0 或 1 */
		tFont12.BackColor = 0;		/* 文字背景颜色 0 或 1 */
		tFont12.Space = 1;			/* 文字间距，单位 = 像素 */
		
		tFont24.FontCode = FC_ST_24;	/* 字体代码 24点阵 */
		tFont24.FrontColor = 1;		/* 字体颜色 0 或 1 */
		tFont24.BackColor = 0;		/* 文字背景颜色 0 或 1 */
		tFont24.Space = 1;			/* 文字间距，单位 = 像素 */
		
		tFont32.FontCode = FC_ST_32;	/* 字体代码 32点阵 */
		tFont32.FrontColor = 1;		/* 字体颜色 0 或 1 */
		tFont32.BackColor = 0;		/* 文字背景颜色 0 或 1 */
		tFont32.Space = 1;			/* 文字间距，单位 = 像素 */		
	}
	ucItem = 0;
	ucDispReq = 1;
	while (1)
	{
		bsp_Idle();		/* 这个函数在bsp.c文件。用户可以修改这个函数实现CPU休眠和喂狗 */

		if (ucDispReq == 1)
		{
			ucDispReq = 0;

			switch (ucItem)
			{
				case 0:
					OLED_ClrScr(0);			 /* 清屏，黑底 */
					OLED_DrawRect(0, 0, 64, 128, 1);
					OLED_DispStr(8, 3, "安富莱OLED例程", &tFont16);		/* 在(8,3)坐标处显示一串汉字 */
					OLED_DispStr(10, 22, "请操作摇杆!", &tFont16);
					OLED_DispStr(5, 22 + 20, "www.ARMfly.com", &tFont16);
					break;

				case 1:
					//OLED_StartDraw();	  调用改函数，只刷新缓冲区，不送显示
					OLED_ClrScr(0);
					OLED_DispStr(0, 0,  "故人西辞黄鹤楼，", &tFont16);
					OLED_DispStr(0, 16, "烟花三月下扬州。", &tFont16);
					OLED_DispStr(0, 32, "孤帆远影碧空尽，", &tFont16);
					OLED_DispStr(0, 48, "唯见长江天际流。", &tFont16);
					//OLED_EndDraw();	  调用改函数，将缓冲区中数据送显示
					break;

				case 2:
					OLED_ClrScr(0);
					OLED_DispStr(5, 0,  "《送孟浩然之广陵》", &tFont12);
					OLED_DispStr(0, 13, "故人西辞黄鹤楼，", &tFont12);
					OLED_DispStr(0, 26, "烟花三月下扬州。", &tFont12);
					OLED_DispStr(0, 39, "孤帆远影碧空尽，", &tFont12);
					OLED_DispStr(0, 52, "唯见长江天际流。", &tFont12);

					OLED_DispStr(110, 14, "安", &tFont16);
					OLED_DispStr(110, 30, "富", &tFont16);
					OLED_DispStr(110, 46, "莱", &tFont16);
					OLED_DrawRect(109,13, 50, 17,1);
					break;

				case 3:
					/* 白底黑字 */
					tFont12.FrontColor = 0;		/* 字体颜色 0 或 1 */
					tFont12.BackColor = 1;		/* 文字背景颜色 0 或 1 */
					OLED_ClrScr(0xFF);
					OLED_DispStr(5, 0,  "《送孟浩然之广陵》", &tFont12);
					OLED_DispStr(0, 13, "故人西辞黄鹤楼，", &tFont12);
					OLED_DispStr(0, 26, "烟花三月下扬州。", &tFont12);
					OLED_DispStr(0, 39, "孤帆远影碧空尽，", &tFont12);
					OLED_DispStr(0, 52, "唯见长江天际流。", &tFont12);

					OLED_DispStr(110, 14, "安", &tFont16);
					OLED_DispStr(110, 30, "富", &tFont16);
					OLED_DispStr(110, 46, "莱", &tFont16);
					OLED_DrawRect(109,13, 50, 17, 0);

					/* 恢复黑底白字 */
					tFont12.FrontColor = 1;		/* 字体颜色 0 或 1 */
					tFont12.BackColor = 0;		/* 文字背景颜色 0 或 1 */
					break;

				case 4:
					OLED_ClrScr(0);
					OLED_DispStr(5, 0,  "安富莱123", &tFont24);
					OLED_DispStr(0, 26, "开发板8", &tFont32);
					break;
				
				case 5:
					OLED_ClrScr(0);
					OLED_DrawRect(0,0, 10,10,1);	/* 在(0,0)坐标处绘制一个高10宽10的矩形 */
					OLED_DrawRect(10,10, 20,30,1);	/* 在(10,10)坐标处绘制一个高20宽30的矩形 */
					OLED_DrawCircle(64,32,30,1);	/* 在(64,32)绘制半径30的圆 */
					OLED_DrawLine(127,0,0,63,1);	/* 在(127,0) 和 (0,63) 之间绘制一条直线 */
					break;

				case 6:
					OLED_ClrScr(0x00);				/* 清屏，黑底 */
					break;

				case 7:
					OLED_ClrScr(0xFF);				/* 清屏，白底 */
					break;
			}
		}

		ucKeyCode = bsp_GetKey();
		if (ucKeyCode != KEY_NONE)
		{
			/* 有键按下 */
			switch (ucKeyCode)
			{
				case JOY_DOWN_U:		/* 摇杆上键按下 */
					if (ucContrast < 255)
					{
						ucContrast++;
					}
					OLED_SetContrast(ucContrast);
					ucDispReq = 1;
					break;

				case JOY_DOWN_D:		/* 摇杆下键按下 */
					if (ucContrast > 0)
					{
						ucContrast--;
					}
					OLED_SetContrast(ucContrast);
					ucDispReq = 1;
					break;

				case JOY_DOWN_L:		/* 摇杆LEFT键按下 */
					if (ucItem > 0)
					{
						ucItem--;
					}
					else
					{
						ucItem = DEMO_PAGE_COUNT - 1;
					}
					ucDispReq = 1;
					break;

				case JOY_DOWN_R:	/* 摇杆RIGHT键按下 */
					if (ucItem < DEMO_PAGE_COUNT - 1)
					{
						ucItem++;
					}
					else
					{
						ucItem = 0;
					}
					ucDispReq = 1;
					break;

				case JOY_DOWN_OK:		/* 摇杆OK键 */
					if (ucDir == 0)
					{
						ucDir = 1;
						OLED_SetDir(1);	/* 设置显示方向 */
					}
					else
					{
						ucDir = 0;
						OLED_SetDir(0);	/* 设置显示方向 */
					}
					ucDispReq = 1;
					break;
			}
		}
	}
}

/*
*********************************************************************************************************
*	函 数 名: DispMenu
*	功能说明: 显示操作提示菜单
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DispMenu(void)
{
	printf("操作提示:\r\n");
	printf("本例程需要插上OLED显示模块才能运行！\r\n");
	printf("请操作摇杆:\r\n");
	printf("1. 摇杆上下键 ： 调节对比度\r\n");
	printf("2. 摇杆左右键 ： 切换显示界面\r\n");
	printf("3. 摇杆OK键   ： 切换显示方向（显示内容翻转180度）\r\n");
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
		
		CPU_Sn0 = *(__IO uint32_t*)(0x1FF1E800);
		CPU_Sn1 = *(__IO uint32_t*)(0x1FF1E800 + 4);
		CPU_Sn2 = *(__IO uint32_t*)(0x1FF1E800 + 8);

		printf("\r\nCPU : STM32H743XIH6, BGA240, 主频: %dMHz\r\n", SystemCoreClock / 1000000);
		printf("UID = %08X %08X %08X\n\r", CPU_Sn2, CPU_Sn1, CPU_Sn0);
	}

	printf("\n\r");
	printf("*************************************************************\n\r");
	printf("* 例程名称   : %s\r\n", EXAMPLE_NAME);	/* 打印例程名称 */
	printf("* 例程版本   : %s\r\n", DEMO_VER);		/* 打印例程版本 */
	printf("* 发布日期   : %s\r\n", EXAMPLE_DATE);	/* 打印例程日期 */

	/* 打印ST的HAL库版本 */
	printf("* HAL库版本  : V1.3.0 (STM32H7xx HAL Driver)\r\n");
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
