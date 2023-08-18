/*
*********************************************************************************************************
*
*	模块名称 : 主程序模块
*	文件名称 : main.c
*	版    本 : V1.0
*	说    明 : 样条插补实现
*              实验目的：
*                1.	学习样条插补实现
*              实验内容： 
*                1. 启动一个自动重装软件定时器，每100ms翻转一次LED2。
*                2.	K1键按下，自然样条插补测试。
*                3.	K2键按下，抛物线样插补测试。
*              注意事项：
*                1. 打印方式：
*                   方式一：
*                     默认使用串口打印，可以使用SecureCRT或者H7-TOOL上位机串口助手查看打印信息，
*                     波特率115200，数据位8，奇偶校验位无，停止位1。
*                   方式二：
*                     使用RTT打印，可以使用SEGGE RTT或者H7-TOOL RTT打印。
*                     MDK AC5，MDK AC6或IAR通过使能bsp.h文件中的宏定义为1即可
*                     #define Enable_RTTViewer  1
*                2. 务必将编辑器的缩进参数和TAB设置为4来阅读本文件，要不代码显示不整齐。
*
*	修改记录 :
*		版本号   日期         作者        说明
*		V1.0    2021-10-31   Eric2013     1. CMSIS软包版本 V5.8.0
*                                         2. HAL库版本 V1.10.0
*
*	Copyright (C), 2020-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/	
#include "bsp.h"			 /* 底层硬件驱动 */
#include "arm_math.h"



/* 定义例程名和例程发布日期 */
#define EXAMPLE_NAME	"V7-样条插补实现，波形拟合丝滑顺畅"
#define EXAMPLE_DATE	"2021-10-31"
#define DEMO_VER		"1.0"

static void PrintfLogo(void);
static void PrintfHelp(void);


#define INPUT_TEST_LENGTH_SAMPLES 	128    /* 输入数据个数 */
#define OUT_TEST_LENGTH_SAMPLES 	1024   /* 输出数据个数 */

#define SpineTab (OUT_TEST_LENGTH_SAMPLES/INPUT_TEST_LENGTH_SAMPLES ) /* 插补末尾的8个坐标值不使用 */


float32_t xn[INPUT_TEST_LENGTH_SAMPLES];   /* 输入数据x轴坐标 */
float32_t yn[INPUT_TEST_LENGTH_SAMPLES];   /* 输入数据y轴坐标 */

float32_t coeffs[3*(INPUT_TEST_LENGTH_SAMPLES - 1)];     /* 插补系数缓冲 */  
float32_t tempBuffer[2 * INPUT_TEST_LENGTH_SAMPLES - 1]; /* 插补临时缓冲 */  

float32_t xnpos[OUT_TEST_LENGTH_SAMPLES];  /* 插补计算后X轴坐标值 */
float32_t ynpos[OUT_TEST_LENGTH_SAMPLES];  /* 插补计算后Y轴数值 */

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
	uint32_t i;
	uint32_t idx2;
	uint8_t ucKeyCode;	
	arm_spline_instance_f32 S;
	

	bsp_Init();		/* 硬件初始化 */
	PrintfLogo();	/* 打印例程名称和版本等信息 */
	PrintfHelp();	/* 打印操作提示 */
	
	bsp_StartAutoTimer(0, 100);	/* 启动1个100ms的自动重装的定时器 */
	
	
	/* 原始x轴数值和y轴数值 */
	for(i=0; i<INPUT_TEST_LENGTH_SAMPLES; i++)
	{
		xn[i] = i*SpineTab;
		yn[i] = 1 + cos(2*3.1415926*50*i/256 + 3.1415926/3);
	}
	
	/* 插补后X轴坐标值，这个是需要用户设置的 */
	for(i=0; i<OUT_TEST_LENGTH_SAMPLES; i++)
	{
		xnpos[i] = i;
	}
	
	while (1)
	{
		bsp_Idle();		/* 这个函数在bsp.c文件。用户可以修改这个函数实现CPU休眠和喂狗 */

		/* 判断定时器超时时间 */
		if (bsp_CheckTimer(0))	
		{
			/* 每隔100ms 进来一次 */  
			bsp_LedToggle(2);
		}

		ucKeyCode = bsp_GetKey();	/* 读取键值, 无键按下时返回 KEY_NONE = 0 */
		if (ucKeyCode != KEY_NONE)
		{
			switch (ucKeyCode)
			{
				case KEY_DOWN_K1:  /* K1键按下，自然样条插补 */
					/* 样条初始化 */
					arm_spline_init_f32(&S,
										ARM_SPLINE_NATURAL ,
										xn,
										yn,
										INPUT_TEST_LENGTH_SAMPLES,
										coeffs,
										tempBuffer);
					/* 样条计算 */
					arm_spline_f32	(&S,
									 xnpos,
									 ynpos,
									 OUT_TEST_LENGTH_SAMPLES);

				
					/* 打印输出输出 */
					idx2 = 0;
					for (i = 0; i < OUT_TEST_LENGTH_SAMPLES-SpineTab; i++)
					{	
							if ((i % SpineTab) == 0)
							{
									printf("%f,%f\r\n", ynpos[i], yn[idx2++]);
							}
							else
							{
									printf("%f,\r\n", ynpos[i]);
							}
					}
					break;

				case KEY_DOWN_K2:			/* K2键按下，抛物线样条插补 */
					/* 样条初始化 */
					arm_spline_init_f32(&S,
										ARM_SPLINE_PARABOLIC_RUNOUT , 
										xn,
										yn,
										INPUT_TEST_LENGTH_SAMPLES,
										coeffs,
										tempBuffer);
					/* 样条计算 */
					arm_spline_f32	(&S,
									 xnpos,
									 ynpos,
									 OUT_TEST_LENGTH_SAMPLES);

				
					/* 打印输出输出 */
					idx2 = 0;
					for (i = 0; i < OUT_TEST_LENGTH_SAMPLES-SpineTab; i++)
					{	
							if ((i % SpineTab) == 0)
							{
									printf("%f,%f\r\n", ynpos[i], yn[idx2++]);
							}
							else
							{
									printf("%f,\r\n", ynpos[i]);
							}
					}
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
	printf("1. 上电启动了一个软件定时器，每100ms翻转一次LED2\r\n");
	printf("2. K1键按下，自然样条插补测试\r\n");
	printf("3. K2键按下，抛物线样插补测试\r\n");
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
	printf("* HAL库版本  : V1.10.0 (STM32H7xx HAL Driver)\r\n");
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
