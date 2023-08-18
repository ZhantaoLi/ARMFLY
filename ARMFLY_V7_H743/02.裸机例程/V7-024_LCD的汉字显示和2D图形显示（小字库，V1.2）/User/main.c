/*
*********************************************************************************************************
*
*	模块名称 : 主程序模块
*	文件名称 : main.c
*	版    本 : V1.0
*	说    明 : LCD小字库的实现和2D图形显示。
*              实验目的：
*                1. 学习LCD的小字库实现和2D图形显示。
*              实验内容：
*                1、小字库通过此软件生成：
*                   http://www.armbbs.cn/forum.php?mod=viewthread&tid=202。 
*                2、LCD界面上显示了汉字和2D图形。
*                3、启动1个200ms的自动重装定时器，让LED2每200ms翻转一次。
*                4、同时在LCD界面上实现一个简单计数，每200ms加1，计数到255后继续从0开始。            
*              注意事项：
*                1. 本实验推荐使用串口软件SecureCRT查看打印信息，波特率115200，数据位8，奇偶校验位无，停止位1。
*                2. 务必将编辑器的缩进参数和TAB设置为4来阅读本文件，要不代码显示不整齐。
*
*	修改记录 :
*		版本号   日期         作者        说明
*		V1.0    2018-12-12   Eric2013     1. CMSIS软包版本 V5.4.0
*                                         2. HAL库版本 V1.3.0
*		V1.1    2019-04-06   Eric2013     1. 添加2D图形显示
*		V1.2    2021-02-01   Eric2013     1. CMSIS软包版本 V5.7.0
*                                         2. HAL库版本 V1.9.0
*                                         3. 更新LCD驱动
*                                         
*	Copyright (C), 2021-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/	
#include "bsp.h"			 /* 底层硬件驱动 */



/* 定义例程名和例程发布日期 */
#define EXAMPLE_NAME	"V7-LCD的汉字显示和2D图形显示（小字库）"
#define EXAMPLE_DATE	"2021-02-01"
#define DEMO_VER		"1.2"

static void PrintfLogo(void);


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
	FONT_T tFont12;			/* 定义一个字体结构体变量，用于设置字体参数 */
	FONT_T tFont16;			/* 定义一个字体结构体变量，用于设置字体参数 */
	uint8_t buf[100], count = 0;

	/* 设置字体参数 */
	{
		tFont12.FontCode = FC_ST_12;	    /* 字体代码 12点阵 */
		tFont12.FrontColor = CL_WHITE;		/* 字体颜色 */
		tFont12.BackColor = CL_BLUE;	    /* 文字背景颜色 */
		tFont12.Space = 0;					/* 文字间距，单位 = 像素 */
	}
	
	/* 设置字体参数 */
	{
		tFont16.FontCode = FC_ST_16;	    /* 字体代码 16点阵 */
		tFont16.FrontColor = CL_WHITE;		/* 字体颜色 */
		tFont16.BackColor = CL_BLUE;	    /* 文字背景颜色 */
		tFont16.Space = 0;					/* 文字间距，单位 = 像素 */
	}
	
	
	bsp_Init();		/* 硬件初始化 */
	PrintfLogo();	/* 打印例程名称和版本等信息 */

	/* 延迟200ms再点亮背光，避免瞬间高亮 */
	bsp_DelayMS(200); 
	
	/* 清屏 */
	LCD_ClrScr(CL_BLUE);

	/* 显示汉字 */
	LCD_DispStr(5, 3, "故人西辞黄鹤楼，烟花三月下扬州。", &tFont12); 
	LCD_DispStr(5, 18, "孤帆远影碧空尽，唯见长江天际流。", &tFont12); 
	LCD_DispStr(5, 38, "故人西辞黄鹤楼，烟花三月下扬州。", &tFont16); 
	LCD_DispStr(5, 58, "孤帆远影碧空尽，唯见长江天际流。", &tFont16); 
	
	/* 绘制2D图形 */
	LCD_DrawLine(5, 120, 100, 220, CL_RED);
	LCD_DrawRect(120, 120, 100, 100, CL_RED);
	LCD_DrawCircle(280, 170, 50, CL_RED);
	LCD_Fill_Rect (340, 120, 100, 100, CL_BUTTON_GREY);
	
	/* 界面整体显示完毕后，再打开背光，设置为缺省亮度 */
	bsp_DelayMS(100); 
	LCD_SetBackLight(BRIGHT_DEFAULT);	
	
	bsp_StartAutoTimer(0, 200); /* 启动1个200ms的自动重装的定时器，软件定时器0 */
	
	while (1)
	{
		/* 判断软件定时器0是否超时 */
		if(bsp_CheckTimer(0))
		{
			/* 每隔200ms 进来一次 */  
			bsp_LedToggle(2);
			
			sprintf((char *)buf, "count = %03d", count++);
			LCD_DispStr(5, 90, (char *)buf, &tFont16); 
		}
	}
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
