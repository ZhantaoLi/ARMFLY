/*
*********************************************************************************************************
*
*	模块名称 : 主程序模块
*	文件名称 : main.c
*	版    本 : V1.0
*	说    明 : 按键检测（软件滤波和FIFO机制）。
*              实验目的：
*                1. 学习按键的按下，弹起，长按和组合键的实现。
*              实验内容：
*                1、启动一个自动重装软件定时器，每100ms翻转一次LED2。
*              实验内容：
*                1、3个独立按键和5向摇杆按下时均有串口消息打印。
*                2、5向摇杆的左键和右键长按时，会有连发的串口消息。
*                3、独立按键K1和K2按键按下，串口打印消息。
*              注意事项：
*                1. 本实验推荐使用串口软件SecureCRT查看打印信息，波特率115200，数据位8，奇偶校验位无，停止位1。
*                2. 务必将编辑器的缩进参数和TAB设置为4来阅读本文件，要不代码显示不整齐。
*
*	修改记录 :
*		版本号   日期         作者        说明
*		V1.0    2019-07-27   Eric2013     1. CMSIS软包版本 V5.5.0
*                                         2. HAL库版本 V2.4.0
*
*	Copyright (C), 2018-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/	
#include "bsp.h"			/* 底层硬件驱动 */



/* 定义例程名和例程发布日期 */
#define EXAMPLE_NAME	"V5-按键检测（软件滤波，FIFO机制）"
#define EXAMPLE_DATE	"2019-07-27"
#define DEMO_VER		"1.0"

static void PrintfLogo(void);
static void PrintfHelp(void);

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
	uint8_t ucKeyCode;		/* 按键代码 */
	
	bsp_Init();		/* 硬件初始化 */
	
	PrintfLogo();	/* 打印例程名称和版本等信息 */
	PrintfHelp();	/* 打印操作提示 */


	bsp_StartAutoTimer(0, 100); /* 启动1个100ms的自动重装的定时器 */
	
	/* 进入主程序循环体 */
	while (1)
	{
		bsp_Idle();		/* 这个函数在bsp.c文件。用户可以修改这个函数实现CPU休眠和喂狗 */

		/* 判断定时器超时时间 */
		if (bsp_CheckTimer(0))	
		{
			/* 每隔100ms 进来一次 */  
			bsp_LedToggle(2);			
		}
		
		/* 按键滤波和检测由后台systick中断服务程序实现，我们只需要调用bsp_GetKey读取键值即可。 */
		ucKeyCode = bsp_GetKey();	/* 读取键值, 无键按下时返回 KEY_NONE = 0 */
		if (ucKeyCode != KEY_NONE)
		{
			switch (ucKeyCode)
			{
				case KEY_DOWN_K1:			/* K1键按下 */
					printf("K1键按下\r\n");
					break;

				case KEY_UP_K1:				/* K1键弹起 */
					printf("K1键弹起\r\n");
					break;

				case KEY_DOWN_K2:			/* K2键按下 */
					printf("K2键按下\r\n");
					break;

				case KEY_UP_K2:				/* K2键弹起 */
					printf("K2键弹起\r\n");
					break;

				case KEY_DOWN_K3:			/* K3键按下 */
					printf("K3键按下\r\n");
					break;

				case KEY_UP_K3:				/* K3键弹起 */
					printf("K3键弹起\r\n");
					break;

				case JOY_DOWN_U:			/* 摇杆UP键按下 */
					printf("摇杆上键按下\r\n");
					break;

				case JOY_DOWN_D:			/* 摇杆DOWN键按下 */
					printf("摇杆下键按下\r\n");
					break;

				case JOY_DOWN_L:			/* 摇杆LEFT键按下 */
					printf("摇杆左键按下\r\n");
					break;
				
				case JOY_LONG_L:            /* 摇杆LEFT键长按 */
					printf("摇杆左键长按\r\n");
					break;

				case JOY_DOWN_R:			/* 摇杆RIGHT键按下 */
					printf("摇杆右键按下\r\n");
					break;
				
				case JOY_LONG_R:            /* 摇杆RIGHT键长按 */
					printf("摇杆右键长按\r\n");
					break;

				case JOY_DOWN_OK:			/* 摇杆OK键按下 */
					printf("摇杆OK键按下\r\n");
					break;

				case JOY_UP_OK:				/* 摇杆OK键弹起 */
					printf("摇杆OK键弹起\r\n");
					break;
                    
                case SYS_DOWN_K1K2:			/* 摇杆OK键弹起 */
					printf("K1和K2组合键按下\r\n");
					break;

				default:
					/* 其它的键值不处理 */
					break;
			}
		
		}
	}
}

/*
*********************************************************************************************************
*	函 数 名: PrintfHelp
*	功能说明: 打印操作提示
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void PrintfHelp(void)
{
	printf("操作提示:\r\n");
	printf("1. 启动一个自动重装软件定时器，每100ms翻转一次LED2\r\n");
	printf("2. 3个独立按键和5向摇杆按下时均有串口消息打印\r\n");
	printf("3. 5向摇杆的左键和右键长按时，会有连发的串口消息\r\n");	
    printf("4. 独立按键K1和K2按键按下，串口打印消息\r\n");	
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
	printf("* HAL库版本  : V2.4.0 (STM32F407 HAL Driver)\r\n");
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
