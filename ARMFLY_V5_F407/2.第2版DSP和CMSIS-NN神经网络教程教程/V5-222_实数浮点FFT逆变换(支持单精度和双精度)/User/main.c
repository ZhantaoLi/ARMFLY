/*
*********************************************************************************************************
*
*	模块名称 : 主程序模块
*	文件名称 : main.c
*	版    本 : V1.0
*	说    明 : 移植实数浮点FTT逆变换（支持单精度和双精度）
*              实验目的：
*                1. 学习实数浮点FFT逆变换，支持单精度浮点和双精度浮点
*              实验内容： 
*                1. 启动一个自动重装软件定时器，每100ms翻转一次LED2。
*                2.	按下按键K1，串口打印1024点实数单精度FFT逆变换。
*                3.	按下按键K2，串口打印1024点实数双精度FFT逆变换。
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
*		V1.0    2021-05-22   Eric2013     1. CMSIS软包版本 V5.7.0
*                                         2. HAL库版本 V1.7.6
*
*	Copyright (C), 2020-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/	
#include "bsp.h"			 	/* 底层硬件驱动 */
#include "arm_math.h"
#include "arm_const_structs.h"


/* 定义例程名和例程发布日期 */
#define EXAMPLE_NAME	"V5-实数浮点FTT逆变换（支持单精度和双精度）"
#define EXAMPLE_DATE	"2021-06-04"
#define DEMO_VER		"1.0"


/* 仅允许本文件内调用的函数声明 */
static void PrintfLogo(void);
static void PrintfHelp(void);
static void arm_rfft_f32_app(void);
static void arm_rfft_f64_app(void);


/* 变量 */
uint32_t ifftFlag = 0; 
uint32_t fftSize = 0;

/* 输入和输出缓冲 */
#define TEST_LENGTH_SAMPLES 1024 

static float32_t testOutput_f32[TEST_LENGTH_SAMPLES*2]; 
static float32_t testOutputIn_f32[TEST_LENGTH_SAMPLES*2]; 
static float32_t testInput_f32[TEST_LENGTH_SAMPLES*2];

static float64_t testOutput_f64[TEST_LENGTH_SAMPLES*2]; 
static float64_t testOutputIn_f64[TEST_LENGTH_SAMPLES*2]; 
static float64_t testInput_f64[TEST_LENGTH_SAMPLES*2];


/*
*********************************************************************************************************
*	函 数 名: arm_rfft_f32_app
*	功能说明: 调用函数arm_rfft_fast_f32计算FFT逆变换和正变换
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void arm_rfft_f32_app(void)
{
	uint16_t i;
	arm_rfft_fast_instance_f32 S;
	
	
	/* 正变换 */
    ifftFlag = 0; 
	
	/* 初始化结构体S中的参数 */
 	arm_rfft_fast_init_f32(&S, TEST_LENGTH_SAMPLES);
	
	for(i=0; i<1024; i++)
	{
		/* 波形是由直流分量，50Hz正弦波组成，波形采样率1024，初始相位60° */
		testInput_f32[i] = 1 + cos(2*3.1415926f*50*i/1024 + 3.1415926f/3);
		testOutputIn_f32[i] = testInput_f32[i];
	}
	
	/* 1024点实序列快速FFT, testInput_f32是输入数据，testOutput_f32是输出 */ 
	arm_rfft_fast_f32(&S, testInput_f32, testOutput_f32, ifftFlag);
	
	/* 逆变换 */
    ifftFlag = 1; 
	
	/* 1024点实序列快速FFT逆变换，testOutput_f32是输入数据，testInput_f32是输出数据 */ 
	arm_rfft_fast_f32(&S, testOutput_f32, testInput_f32, ifftFlag);

	printf("=========================================\r\n");	
	
	/* 串口打印，testOutputIn_f32原始信号，testInput_f32逆变换后的信号 */
	for(i=0; i<TEST_LENGTH_SAMPLES; i++)
	{
		printf("%f, %f\r\n", testOutputIn_f32[i], testInput_f32[i]);
	}
}

/*
*********************************************************************************************************
*	函 数 名: arm_rfft_f64_app
*	功能说明: 调用函数arm_rfft_fast_f64计算FFT逆变换和正变换
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void arm_rfft_f64_app(void)
{
	uint16_t i;
	arm_rfft_fast_instance_f64 S;
	
	
	/* 正变换 */
    ifftFlag = 0; 
	
	/* 初始化结构体S中的参数 */
 	arm_rfft_fast_init_f64(&S, TEST_LENGTH_SAMPLES);
	
	for(i=0; i<1024; i++)
	{
		/* 波形是由直流分量，50Hz正弦波组成，波形采样率1024，初始相位60° */
		testInput_f64[i] = 1 + cos(2*3.1415926*50*i/1024 + 3.1415926/3);
		testOutputIn_f64[i] = testInput_f64[i];
	}
	
	/* 1024点实序列快速FFT, testInput_f64是输入数据，testOutput_f64是输出 */ 
	arm_rfft_fast_f64(&S, testInput_f64, testOutput_f64, ifftFlag);
	
	/* 逆变换 */
    ifftFlag = 1; 
	
	/* 1024点实序列快速FFT逆变换，testOutput_f64是输入数据，testInput_f64是输出数据 */ 
	arm_rfft_fast_f64(&S, testOutput_f64, testInput_f64, ifftFlag);
	
	printf("=========================================\r\n");	
	
	/* 串口打印，testOutputIn_f32原始信号，testInput_f32逆变换后的信号 */
	for(i=0; i<TEST_LENGTH_SAMPLES; i++)
	{
		printf("%.11f, %.11f\r\n", testOutputIn_f64[i], testInput_f64[i]);
	}	
			
}

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
	PrintfLogo();	/* 打印例程信息到串口1 */

	PrintfHelp();	/* 打印操作提示信息 */
	

	bsp_StartAutoTimer(0, 100);	/* 启动1个100ms的自动重装的定时器 */

	/* 进入主程序循环体 */
	while (1)
	{
		bsp_Idle();		/* 这个函数在bsp.c文件。用户可以修改这个函数实现CPU休眠和喂狗 */
		

		if (bsp_CheckTimer(0))	/* 判断定时器超时时间 */
		{
			/* 每隔100ms 进来一次 */
			bsp_LedToggle(4);	/* 翻转LED2的状态 */   
		}
		
		ucKeyCode = bsp_GetKey();	/* 读取键值, 无键按下时返回 KEY_NONE = 0 */
		if (ucKeyCode != KEY_NONE)
		{
			switch (ucKeyCode)
			{
				case KEY_DOWN_K1:		    /* K1键按下 */
					arm_rfft_f32_app();
					break;
				
				case KEY_DOWN_K2:		    /* K2键按下 */
					arm_rfft_f64_app();
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
*	功能说明: 显示操作提示菜单
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void PrintfHelp(void)
{
	printf("1. 启动一个自动重装软件定时器，每100ms翻转一次LED2\r\n");
	printf("2. 按下按键K1，串口打印1024点实数单精度FFT的逆变换\r\n");
	printf("3. 按下按键K2，串口打印1024点实数双精度FFT的逆变换\r\n");
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
