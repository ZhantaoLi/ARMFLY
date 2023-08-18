/*
*********************************************************************************************************
*
*	模块名称 : 主程序模块
*	文件名称 : main.c
*	版    本 : V1.0
*	说    明 : ARM官方DSP库移植
*              实验目的：
*                1. 学习官方DSP库的移植。
*              实验内容：
*                1. 按下按键K1, 串口打印函数arm_abs_f32的输出结果。
*                2. 按下按键K2, 串口打印函数arm_abs_q31的输出结果。
*                3. 按下按键K3, 串口打印函数arm_abs_q15的输出结果。
*              注意事项：
*                1. 本实验推荐使用串口软件SecureCRT查看打印信息，波特率115200，数据位8，奇偶校验位无，停止位1。
*                2. 务必将编辑器的缩进参数和TAB设置为4来阅读本文件，要不代码显示不整齐。
*
*	修改记录 :
*		版本号   日期         作者        说明
*		V1.0    2019-07-31   Eric2013     1. CMSIS软包版本 V5.6.0
*                                         2. HAL库版本 V1.3.0
*
*	Copyright (C), 2018-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/	
#include "bsp.h"			/* 底层硬件驱动 */
#include "arm_math.h"



/* 定义例程名和例程发布日期 */
#define EXAMPLE_NAME	"V7-ARM的DSP移植模板（库方式）"
#define EXAMPLE_DATE	"2019-07-31"
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
	float32_t pSrc;
    float32_t pDst;
	q31_t pSrc1;
	q31_t pDst1;
	q15_t pSrc2;
	q15_t pDst2;

	
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

		/* 处理按键事件 */
		ucKeyCode = bsp_GetKey();
		if (ucKeyCode > 0)
		{
			/* 有键按下 */
			switch (ucKeyCode)
			{
				case KEY_DOWN_K1:			   /* K1键按下 */
					pSrc -= 1.23f;
					arm_abs_f32(&pSrc, &pDst, 1);
					printf("pDst = %f\r\n", pDst);
					break;
					
				case KEY_DOWN_K2:				/* K1键弹起 */
					pSrc1 -= 1;
					arm_abs_q31(&pSrc1, &pDst1, 1);
					printf("pDst1 = %d\r\n", pDst1);
					break;

				case KEY_DOWN_K3:				/* K2键弹起 */
					pSrc2 -= 1;
					arm_abs_q15(&pSrc2, &pDst2, 1);
					printf("pDst2 = %d\r\n", pDst2);
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
	printf("2. 按下按键K1, 串口打印函数arm_abs_f32的输出结果\r\n");
	printf("3. 按下按键K2, 串口打印函数arm_abs_q31的输出结果\r\n");
	printf("4. 按下按键K3, 串口打印函数arm_abs_q15的输出结果\r\n");
}

/*
*********************************************************************************************************
*	函 数 名: PrintfLogo
*	功能说明: 打印例程名称和例程发布日期, 接上串口线后，打开PC机的超级终端软件可以观察结果
*	形    参：无
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
