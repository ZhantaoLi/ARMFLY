/*
*********************************************************************************************************
*
*	模块名称 : 主程序模块
*	文件名称 : main.c
*	版    本 : V1.0
*	说    明 : 移植复数浮点FFT，支持单精度浮点和双精度浮点
*              实验目的：
*                1. 学习复数浮点FFT，支持单精度浮点和双精度浮点
*              实验内容： 
*                1. 启动一个自动重装软件定时器，每100ms翻转一次LED2。
*                2.	按下按键K1，串口打印1024点复数单精度FFT的幅频响应和相频响应。
*                3.	按下按键K2，串口打印1024点复数双精度FFT的幅频响应和相频响应。
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
*                                         2. HAL库版本 V1.10.0
*
*	Copyright (C), 2021-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/	
#include "bsp.h"			 	/* 底层硬件驱动 */
#include "arm_math.h"
#include "arm_const_structs.h"


/* 定义例程名和例程发布日期 */
#define EXAMPLE_NAME	"V7-复数浮点FTT（支持单精度和双精度）"
#define EXAMPLE_DATE	"2021-05-22"
#define DEMO_VER		"1.0"


/* 仅允许本文件内调用的函数声明 */
static void PrintfLogo(void);
static void PrintfHelp(void);
static void arm_cfft_f32_app(void);
static void arm_cfft_f64_app(void);


/* 变量 */
uint32_t ifftFlag = 0; 
uint32_t doBitReverse = 1; 

/* 输入和输出缓冲 */
#define TEST_LENGTH_SAMPLES 1024 
static float32_t testOutput_f32[TEST_LENGTH_SAMPLES*2]; 
static float32_t testInput_f32[TEST_LENGTH_SAMPLES*2];
static float32_t Phase_f32[TEST_LENGTH_SAMPLES*2]; /* 相位*/ 

static float64_t testOutput_f64[TEST_LENGTH_SAMPLES*2]; 
static float64_t testInput_f64[TEST_LENGTH_SAMPLES*2];
static float64_t Phase_f64[TEST_LENGTH_SAMPLES*2]; /* 相位*/ 

/*
*********************************************************************************************************
*	函 数 名: PowerPhaseRadians_f32
*	功能说明: 求相位
*	形    参：_ptr  复位地址，含实部和虚部
*             _phase 求出相位，单位角度制，范围(-180, 180]
*             _usFFTPoints  复数个数，每个复数是两个float32_t数值
*             _uiCmpValue  比较值，需要求出相位的数值
*	返 回 值: 无
*********************************************************************************************************
*/
void PowerPhaseRadians_f32(float32_t *_ptr, float32_t *_phase, uint16_t _usFFTPoints, float32_t _uiCmpValue)		
{
	float32_t lX, lY;
	uint16_t i;
	float32_t phase;
	float32_t mag;
	
	
	for (i=0; i <_usFFTPoints; i++)
	{
		lX= _ptr[2*i];  	  /* 实部 */
		lY= _ptr[2*i + 1];    /* 虚部 */ 
		
 		phase = atan2f(lY, lX);    		  				 /* atan2求解的结果范围是(-pi, pi], 弧度制 */
		arm_sqrt_f32((float32_t)(lX*lX+ lY*lY), &mag);   /* 求模 */
		
		if(_uiCmpValue > mag)
		{
			Phase_f32[i] = 0;			
		}
		else
		{
			Phase_f32[i] = phase* 180.0f/3.1415926f;   /* 将求解的结果由弧度转换为角度 */
		}
	}
}

/*
*********************************************************************************************************
*	函 数 名: PowerPhaseRadians_f64
*	功能说明: 求相位
*	形    参：_ptr  复位地址，含实部和虚部
*             _phase 求出相位，单位角度制，范围(-180, 180]
*             _usFFTPoints  复数个数，每个复数是两个float64_t数值
*             _uiCmpValue  比较值，需要求出相位的数值
*	返 回 值: 无
*********************************************************************************************************
*/
void PowerPhaseRadians_f64(float64_t *_ptr, float64_t *_phase, uint16_t _usFFTPoints, float64_t _uiCmpValue)		
{
	float64_t lX, lY;
	uint16_t i;
	float64_t phase;
	float64_t mag;
	
	
	for (i=0; i <_usFFTPoints; i++)
	{
		lX= _ptr[2*i];  	  /* 实部 */
		lY= _ptr[2*i + 1];    /* 虚部 */ 
		
 		phase = atan2(lY, lX);      /* atan2求解的结果范围是(-pi, pi], 弧度制 */
		mag = sqrt(lX*lX+ lY*lY);   /* 求模 */
		
		if(_uiCmpValue > mag)
		{
			Phase_f64[i] = 0;			
		}
		else
		{
			Phase_f64[i] = phase* 180.0/3.1415926;  /* 将求解的结果由弧度转换为角度 */
		}
	}
}

/*
*********************************************************************************************************
*	函 数 名: arm_cfft_f32_app
*	功能说明: 调用函数arm_cfft_f32计算幅频和相频
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void arm_cfft_f32_app(void)
{
	uint16_t i;
	
    ifftFlag = 0; 
    doBitReverse = 1; 
	
	/* 按照实部，虚部，实部，虚部..... 的顺序存储数据 */
	for(i=0; i<TEST_LENGTH_SAMPLES; i++)
	{
		/* 波形是由直流分量，50Hz正弦波组成，波形采样率1024，初始相位60° */
		testInput_f32[i*2] = 1 + cos(2*3.1415926f*50*i/1024 + 3.1415926f/3);
		testInput_f32[i*2+1] = 0;
	}
	
	/* CFFT变换 */ 
	arm_cfft_f32(&arm_cfft_sR_f32_len1024, testInput_f32, ifftFlag, doBitReverse);

	/* 求解模值  */ 
	arm_cmplx_mag_f32(testInput_f32, testOutput_f32, TEST_LENGTH_SAMPLES);
	

	printf("=========================================\r\n");	
	
	/* 求相频 */
	PowerPhaseRadians_f32(testInput_f32, Phase_f32, TEST_LENGTH_SAMPLES, 0.5f);
	
	/* 求出实际大小 */
//	testOutput_f32[0] = testOutput_f32[0] / TEST_LENGTH_SAMPLES;
//	
//	for(i=1; i<TEST_LENGTH_SAMPLES; i++)
//	{
//		testOutput_f32[i] = testOutput_f32[i] /(TEST_LENGTH_SAMPLES/2);
//	}
	
	/* 串口打印求解的模值 */
	for(i=0; i<TEST_LENGTH_SAMPLES; i++)
	{
		printf("%f, %f\r\n", testOutput_f32[i], Phase_f32[i]);
	}
}

/*
*********************************************************************************************************
*	函 数 名: arm_cfft_f64_app
*	功能说明: 调用函数arm_cfft_f64计算幅频和相频
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void arm_cfft_f64_app(void)
{
	uint16_t i;
	float64_t lX,lY;
	
    ifftFlag = 0; 
    doBitReverse = 1; 
	
	/* 按照实部，虚部，实部，虚部..... 的顺序存储数据 */
	for(i=0; i<TEST_LENGTH_SAMPLES; i++)
	{
		/* 波形是由直流分量，50Hz正弦波组成，波形采样率1024，初始相位60° */
		testInput_f64[i*2] = 1 + cos(2*3.1415926*50*i/1024 + 3.1415926/3);
		testInput_f64[i*2+1] = 0;
	}
	
	/* CFFT变换 */ 
	arm_cfft_f64(&arm_cfft_sR_f64_len1024, testInput_f64, ifftFlag, doBitReverse);

	/* 求解模值  */ 
	for (i =0; i < TEST_LENGTH_SAMPLES; i++)
	{
 		lX = testInput_f64[2*i];            /* 实部*/
		lY = testInput_f64[2*i+1];          /* 虚部 */  
		testOutput_f64[i] = sqrt(lX*lX+ lY*lY);   /* 求模 */
	}
	
	printf("=========================================\r\n");	
	
	/* 求相频 */
	PowerPhaseRadians_f64(testInput_f64, Phase_f64, TEST_LENGTH_SAMPLES, 0.5);
	
	
	/* 串口打印求解的模值 */
	for(i=0; i<TEST_LENGTH_SAMPLES; i++)
	{
		printf("%.11f, %.11f\r\n", testOutput_f64[i], Phase_f64[i]);
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
					arm_cfft_f32_app();
					break;
				
				case KEY_DOWN_K2:		    /* K2键按下 */
					arm_cfft_f64_app();
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
	printf("2. 按下按键K1，串口打印1024点复数单精度FFT的幅频响应和相频响应\r\n");
	printf("3. 按下按键K2，串口打印1024点复数双精度FFT的幅频响应和相频响应\r\n");
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
