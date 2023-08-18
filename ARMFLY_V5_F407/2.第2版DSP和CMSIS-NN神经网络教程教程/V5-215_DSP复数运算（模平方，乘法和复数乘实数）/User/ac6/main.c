/*
*********************************************************************************************************
*
*	模块名称 : 主程序模块
*	文件名称 : main.c
*	版    本 : V1.0
*	说    明 : DSP复数运算(模平方，复数乘复数和复数乘实数)
*              实验目的：
*                1. 学习DSP复数运算(模平方，复数乘复数和复数乘实数)
*              实验内容： 
*                1. 启动一个自动重装软件定时器，每100ms翻转一次LED2。
*                2.	按下按键K1，串口打函数DSP_MagSquared的输出数据。
*				 3.	按下按键K2，串口打函数DSP_CmplxMult的输出数据。
*                4.	按下按键K3，串口打函数DSP_CmplxMultReal的输出数据。
*              注意事项：
*                1. 本实验推荐使用串口软件SecureCRT查看打印信息，波特率115200，数据位8，奇偶校验位无，停止位1。
*                2. 务必将编辑器的缩进参数和TAB设置为4来阅读本文件，要不代码显示不整齐。
*
*	修改记录 :
*		版本号   日期         作者        说明
*		V1.0    2020-03-21   Eric2013     1. CMSIS软包版本 V5.6.0
*                                         2. HAL库版本 V1.7.6
*
*	Copyright (C), 2020-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/	
#include "bsp.h"			 /* 底层硬件驱动 */
#include "arm_math.h"



/* 定义例程名和例程发布日期 */
#define EXAMPLE_NAME	"V5-DSP复数运算(模平方，复数乘复数和复数乘实数)"
#define EXAMPLE_DATE	"2020-03-21"
#define DEMO_VER		"1.0"


/* 仅允许本文件内调用的函数声明 */
static void PrintfLogo(void);
static void PrintfHelp(void);
static void DSP_MagSquared(void);
static void DSP_CmplxMult(void);
static void DSP_CmplxMultReal(void);


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
				case KEY_DOWN_K1:		    /* K1键按下，求模平方 */
				    DSP_MagSquared();
					break;
					
				case KEY_DOWN_K2:			/* K2键按下，求复数乘复数 */
					DSP_CmplxMult();
					break;

				case KEY_DOWN_K3:			/* K3键按下，求复数乘实数 */
					DSP_CmplxMultReal();
					break;

				default:
					/* 其他的键值不处理 */
					break;
			}
		}
	}
}

/*
*********************************************************************************************************
*	函 数 名: DSP_MagSquared
*	功能说明: 复数模的平方
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DSP_MagSquared(void)
{
	uint8_t i;
	float32_t pSrc[10] = {1.1f, 1.1f, 2.1f, 2.1f, 3.1f, 3.1f, 4.1f, 4.1f, 5.1f, 5.1f};
	float32_t pDst[10];
	
	q31_t pSrc1[10] = {1*268435456, 1*268435456, 2*268435456, 2*268435456, 3*268435456, 3*268435456, 
	                    4*268435456, 4*268435456, 5*268435456, 5*268435456};
	q31_t pDst1[10];

	q15_t pSrc2[10] = {5000, 10000, 15000, 20000, 25000,  5000, 10000, 15000, 20000, 25000};
	q15_t pDst2[10];
	
	/***浮点数模平方*******************************************************************************/
	arm_cmplx_mag_squared_f32(pSrc, pDst, 5);
	for(i = 0; i < 5; i++)
	{
		printf("pDst[%d] = %f\r\n", i, pDst[i]);
	}
	
	/***定点数模平方Q31*******************************************************************************/
	arm_cmplx_mag_squared_q31(pSrc1, pDst1, 5);
	for(i = 0; i < 5; i++)
	{
		printf("pDst1[%d] = %d\r\n", i, pDst1[i]);
	}
	
	/***定点数模平方Q15*******************************************************************************/
	arm_cmplx_mag_squared_q15(pSrc2, pDst2, 5);
	for(i = 0; i < 5; i++)
	{
		printf("pDst2[%d] = %d\r\n", i, pDst2[i]);
	}
}

/*
*********************************************************************************************************
*	函 数 名: DSP_CmplxMult
*	功能说明: 复数乘法
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DSP_CmplxMult(void)
{
	uint8_t i;
	float32_t pSrcA[10] = {1.1f, 1.2f, 2.1f, 2.2f, 3.1f, 3.2f, 4.1f, 4.2f, 5.1f, 5.2f};
	float32_t pSrcB[10] = {1.2f, 1.2f, 2.2f, 2.2f, 3.2f, 3.2f, 4.2f, 4.2f, 5.2f, 5.2f};
	float32_t pDst[10];
	
	q31_t pSrcA1[10] = {1*268435456, 1*268435456, 2*268435456, 2*268435456, 3*268435456, 3*268435456, 
	                    4*268435456, 4*268435456, 5*268435456, 5*268435456};
	q31_t pSrcB1[10] = {1*268435456, 1*268435456, 2*268435456, 2*268435456, 3*268435456, 3*268435456, 
	                    4*268435456, 4*268435456, 5*268435456, 5*268435456};
	q31_t pDst1[10];
	
	q15_t pSrcA2[10] = {5000, 10000, 15000, 20000, 25000,  5000, 10000, 15000, 20000, 25000};
	q15_t pSrcB2[10] = {6000, 11000, 15000, 20000, 25000,  5000, 10000, 15000, 20000, 25000};
	q15_t pDst2[10];
	
	/***浮点数乘法*******************************************************************************/
	arm_cmplx_mult_cmplx_f32(pSrcA, pSrcB, pDst, 5);
	for(i = 0; i < 5; i++)
	{
		printf("pDst[%d] = %f %fj\r\n", i, pDst[2*i], pDst[2*i+1]);
	}
	
	/***定点数乘法Q31*******************************************************************************/
	arm_cmplx_mult_cmplx_q31(pSrcA1, pSrcB1, pDst1, 5);
	for(i = 0; i < 5; i++)
	{
		printf("pDst1[%d] = %d %dj\r\n", i, pDst1[2*i], pDst1[2*i+1]);
	}
	
	/***定点数乘法Q15*******************************************************************************/
	arm_cmplx_mult_cmplx_q15(pSrcA2, pSrcB2, pDst2, 5);
	for(i = 0; i < 5; i++)
	{
		printf("pDst1[%d] = %d %dj\r\n", i, pDst2[2*i], pDst2[2*i+1]);
	}
}

/*
*********************************************************************************************************
*	函 数 名: DSP_CmplxMultReal
*	功能说明: 复数乘实数
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DSP_CmplxMultReal(void)
{
	uint8_t i;
	float32_t pSrcCmplx[10] = {1.1f, 1.2f, 2.1f, 2.2f, 3.1f, 3.2f, 4.1f, 4.2f, 5.1f, 5.2f};
	float32_t pSrcReal[5] = {1.2f, 1.2f, 2.2f, 2.2f, 3.2f};
	float32_t pCmplxDst[10];
	
	q31_t pSrcCmplx1[10] = {1*268435456, 1*268435456, 2*268435456, 2*268435456, 3*268435456, 3*268435456, 
	                    4*268435456, 4*268435456, 5*268435456, 5*268435456};
						
	q31_t pSrcReal1[10] = {1*268435456, 1*268435456, 2*268435456, 2*268435456, 3*268435456};
	q31_t pCmplxDst1[10];
	
	q15_t pSrcCmplx2[10] = {14000, 16000, 20000, 20000, 30000, 31000, 12000, 13000, 14000, 25000};
	q15_t pSrcReal2[10] =  {15000, 17000, 20000, 20000, 30000};
	q15_t pCmplxDst2[10];
	
	/***浮点数*******************************************************************************/
	arm_cmplx_mult_cmplx_f32(pSrcCmplx, pSrcReal, pCmplxDst, 5);
	for(i = 0; i < 5; i++)
	{
		printf("pCmplxDst[%d] = %f %fj\r\n", i, pCmplxDst[2*i], pCmplxDst[2*i+1]);
	}
	
	/***定点数Q31*******************************************************************************/
	arm_cmplx_mult_cmplx_q31(pSrcCmplx1, pSrcReal1, pCmplxDst1, 5);
	for(i = 0; i < 5; i++)
	{
		printf("pCmplxDst1[%d] = %d %dj\r\n", i, pCmplxDst1[2*i], pCmplxDst1[2*i+1]);
	}
	
	/***定点数Q15*******************************************************************************/
	arm_cmplx_mult_cmplx_q15(pSrcCmplx2, pSrcReal2, pCmplxDst2, 5);
	for(i = 0; i < 5; i++)
	{
		printf("pCmplxDst2[%d] = %d %dj\r\n", i, pCmplxDst2[2*i], pCmplxDst2[2*i+1]);
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
	printf("操作提示:\r\n");
	printf("1. 启动一个自动重装软件定时器，每100ms翻转一次LED2\r\n");
	printf("2. 按下按键K1，串口打函数DSP_MagSquared的输出数据\r\n");
	printf("3. 按下按键K2，串口打函数DSP_CmplxMult的输出数据\r\n");
	printf("4. 按下按键K3，串口打函数DSP_CmplxMultReal的输出数据\r\n");
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
