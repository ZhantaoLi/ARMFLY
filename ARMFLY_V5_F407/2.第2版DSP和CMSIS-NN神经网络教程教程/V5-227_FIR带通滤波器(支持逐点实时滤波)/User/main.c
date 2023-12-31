/*
*********************************************************************************************************
*
*	模块名称 : 主程序模块
*	文件名称 : main.c
*	版    本 : V1.0
*	说    明 : FIR带通滤波器的实现，支持实时滤波
*              实验目的：
*                1.	学习FIR带通滤波器的实现，支持实时滤波
*              实验内容： 
*                1. 启动一个自动重装软件定时器，每100ms翻转一次LED2。
*                2.	按下按键K1，打印原始波形数据和滤波后的波形数据。
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
*		V1.0    2020-08-01   Eric2013     1. CMSIS软包版本 V5.8.0
*                                         2. HAL库版本 V1.10.0
*
*	Copyright (C), 2020-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/	
#include "bsp.h"			 	/* 底层硬件驱动 */
#include "arm_math.h"
#include "arm_const_structs.h"


/* 定义例程名和例程发布日期 */
#define EXAMPLE_NAME	"FIR带通滤波器"
#define EXAMPLE_DATE	"2021-08-01"
#define DEMO_VER		"1.0"


/* 仅允许本文件内调用的函数声明 */
static void PrintfLogo(void);
static void PrintfHelp(void);
static void arm_fir_f32_bp(void);


#define TEST_LENGTH_SAMPLES  1024    /* 采样点数 */
#define BLOCK_SIZE           1    	 /* 调用一次arm_fir_f32处理的采样点个数 */
#define NUM_TAPS             29      /* 滤波器系数个数 */

uint32_t blockSize = BLOCK_SIZE;
uint32_t numBlocks = TEST_LENGTH_SAMPLES/BLOCK_SIZE;            /* 需要调用arm_fir_f32的次数 */

static float32_t testInput_f32_50Hz_200Hz[TEST_LENGTH_SAMPLES]; /* 采样点 */
static float32_t testOutput[TEST_LENGTH_SAMPLES];               /* 滤波后的输出 */
static float32_t firStateF32[BLOCK_SIZE + NUM_TAPS - 1];        /* 状态缓存，大小numTaps + blockSize - 1*/


/* 高通滤波器系数 通过fadtool获取*/
const float32_t firCoeffs32BP[NUM_TAPS] = {
	0.003531039227f,    0.0002660876198f,   -0.001947779674f,  0.001266813371f,  -0.008019094355f,
	-0.01986379735f,    0.01018396299f,     0.03163734451f,    0.00165955862f,   0.03312643617f,
	0.0622616075f,      -0.1229852438f,     -0.2399847955f,    0.07637182623f,   0.3482480049f,
	0.07637182623f,     -0.2399847955f,     -0.1229852438f,    0.0622616075f,    0.03312643617f,
	0.00165955862f,     0.03163734451f,     0.01018396299f,    -0.01986379735f,  -0.008019094355f,
	0.001266813371f,   -0.001947779674f,    0.0002660876198f,  0.003531039227f
};

/*
*********************************************************************************************************
*	函 数 名: arm_fir_f32_bp
*	功能说明: 调用函数arm_fir_f32_bp实现带通滤波器
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void arm_fir_f32_bp(void)
{
	uint32_t i;
	arm_fir_instance_f32 S;
	float32_t  *inputF32, *outputF32;

	/* 初始化输入输出缓存指针 */
	inputF32 = &testInput_f32_50Hz_200Hz[0];
	outputF32 = &testOutput[0];

	/* 初始化结构体S */
	arm_fir_init_f32(&S,                            
					 NUM_TAPS, 
	                (float32_t *)&firCoeffs32BP[0], 
	                 &firStateF32[0], 
	                 blockSize);

	/* 实现FIR滤波，这里每次处理1个点 */
	for(i=0; i < numBlocks; i++)
	{
		arm_fir_f32(&S, inputF32 + (i * blockSize),  outputF32 + (i * blockSize),  blockSize);
	}
	

	/* 打印滤波后结果 */
	for(i=0; i<TEST_LENGTH_SAMPLES; i++)
	{
		printf("%f, %f\r\n", testOutput[i], inputF32[i]);
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
	uint16_t i;

	
	bsp_Init();		/* 硬件初始化 */
	PrintfLogo();	/* 打印例程信息到串口1 */

	PrintfHelp();	/* 打印操作提示信息 */
	
	for(i=0; i<TEST_LENGTH_SAMPLES; i++)
	{
		/* 50Hz正弦波+200Hz正弦波，采样率1KHz */
		testInput_f32_50Hz_200Hz[i] = arm_sin_f32(2*3.1415926f*50*i/1000) + arm_sin_f32(2*3.1415926f*200*i/1000);
	}
	

	bsp_StartAutoTimer(0, 100);	/* 启动1个100ms的自动重装的定时器 */

	/* 进入主程序循环体 */
	while (1)
	{
		bsp_Idle();		/* 这个函数在bsp.c文件。用户可以修改这个函数实现CPU休眠和喂狗 */
		

		if (bsp_CheckTimer(0))	/* 判断定时器超时时间 */
		{
			/* 每隔100ms 进来一次 */
			bsp_LedToggle(2);	/* 翻转LED的状态 */
		}
		
		ucKeyCode = bsp_GetKey();	/* 读取键值, 无键按下时返回 KEY_NONE = 0 */
		if (ucKeyCode != KEY_NONE)
		{
			switch (ucKeyCode)
			{
				case KEY_DOWN_K1:		    /* K1键按下 */
					arm_fir_f32_bp();
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
	printf("2. 按下按键K1，打印原始波形数据和滤波后的波形数据\r\n");
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
