/*
*********************************************************************************************************
*
*	模块名称 : 主程序模块
*	文件名称 : main.c
*	版    本 : V1.1
*	说    明 : 学习串口与PC通信
*              实验目的：
*                1. 学习串口与PC通信。
*              实验内容：
*                1. 启动一个自动重装软件定时器，每100ms翻转一次LED2。
*              实验操作：
*                1. 串口接收到字符命令'1'，返回串口消息"接收到串口命令1"。
*                2. 串口接收到字符命令'2'，返回串口消息"接收到串口命令2"。
*                3. 串口接收到字符命令'3'，返回串口消息"接收到串口命令3"。
*                4. 串口接收到字符命令'4'，返回串口消息"接收到串口命令4"。
*                5. K1按键按下，串口打印"按键K1按下"。
*                6. K2按键按下，串口打印"按键K2按下"。
*                7. K3按键按下，串口打印"按键K3按下"。
*              注意事项：
*                1. 本实验推荐使用串口软件SecureCRT查看打印信息，波特率115200，数据位8，奇偶校验位无，停止位1。
*                2. 务必将编辑器的缩进参数和TAB设置为4来阅读本文件，要不代码显示不整齐。
*
*	修改记录 :
*		版本号   日期         作者        说明
*		V1.0    2019-08-17   Eric2013     1. CMSIS软包版本 V5.5.0
*                                         2. HAL库版本 V2.4.0
*		V1.1    2020-04-06   Eric2013     1. 更新串口标志清除错误。
*
*	Copyright (C), 2018-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/	
#include "bsp.h"			/* 底层硬件驱动 */



/* 定义例程名和例程发布日期 */
#define EXAMPLE_NAME	"V5-串口和PC机通信（驱动支持6个串口FIFO）"
#define EXAMPLE_DATE	"2020-04-06"
#define DEMO_VER		"1.1"

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
	uint8_t ucKeyCode;	
	uint8_t read;
	const char buf1[] = "接收到串口命令1\r\n";
	const char buf2[] = "接收到串口命令2\r\n";
	const char buf3[] = "接收到串口命令3\r\n";
	const char buf4[] = "接收到串口命令4\r\n";
	
	
	bsp_Init();		/* 硬件初始化 */
	
	PrintfLogo();	/* 打印例程名称和版本等信息 */
	PrintfHelp();	/* 打印操作提示 */

	bsp_StartAutoTimer(0, 100);	/* 启动1个100ms的自动重装的定时器 */
	
	/* 主程序大循环 */
	while (1)
	{
		/* CPU空闲时执行的函数，在 bsp.c */
		bsp_Idle();		
		
		/* 判断定时器超时时间 */
		if (bsp_CheckTimer(0))	
		{
			/* 每隔100ms 进来一次 */
			/* 翻转LED2的状态 */
			bsp_LedToggle(2);	
		}
		
		/* 接收到的串口命令处理 */
		if (comGetChar(COM1, &read))
		{
			switch (read)
			{
				case '1':
					comSendBuf(COM1, (uint8_t *)buf1, strlen(buf1));
					break;

				case '2':
					comSendBuf(COM1, (uint8_t *)buf2, strlen(buf2));
					break;

				case '3':
					comSendBuf(COM1, (uint8_t *)buf3, strlen(buf3));
					break;

				case '4':
					comSendBuf(COM1, (uint8_t *)buf4, strlen(buf4));
					break;	
				
				default:
					break;
			}
		}
		
		/* 处理按键事件 */
		ucKeyCode = bsp_GetKey();
		if (ucKeyCode > 0)
		{
			/* 有键按下 */
			switch (ucKeyCode)
			{
				case KEY_DOWN_K1:		/* 按键K1键按下 */
					printf("按键K1按下\r\n");
					bsp_LedToggle(1);	
					break;		
				
				case KEY_DOWN_K2:		/* 按键K2键按下 */
					printf("按键K2按下\r\n");
					bsp_LedToggle(3);					
					break;

				case KEY_DOWN_K3:		/* 按键K3键按下 */
					printf("按键K3按下\r\n");	
					bsp_LedToggle(4);	
					break;				
				
				default:
					break;
			}
		}
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
	printf("1. 启动一个自动重装软件定时器，每100ms翻转一次LED2\r\n");
	printf("2. 串口接收到字符命令'1'，返回串口消息接收到串口命令1\r\n");
	printf("3. 串口接收到字符命令'2'，返回串口消息接收到串口命令2\r\n");
	printf("4. 串口接收到字符命令'3'，返回串口消息接收到串口命令3\r\n");
	printf("5. 串口接收到字符命令'4'，返回串口消息接收到串口命令4\r\n");
	printf("6. K1按键按下，串口打印按键K1按下\r\n");
	printf("7. K2按键按下，串口打印按键K2按下\r\n");
	printf("8. K3按键按下，串口打印按键K3按下\r\n");
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
