/*
*********************************************************************************************************
*
*	模块名称 : 主程序模块
*	文件名称 : main.c
*	版    本 : V1.0
*	说    明 : DSP功能函数（数据拷贝，数据填充和浮点转定点）
*              实验目的：
*                1. 学习DSP功能函数（数据拷贝，数据填充和浮点转定点）
*              实验内容： 
*                1. 启动一个自动重装软件定时器，每100ms翻转一次LED2。
*                2. 按下按键K1, 串口打印函数DSP_Copy的输出结果
*                3. 按下按键K2, 串口打印函数DSP_Fill的输出结果
*                4. 按下按键K3, 串口打印函数DSP_FloatToFix的输出结果
*              注意事项：
*                1. 本实验推荐使用串口软件SecureCRT查看打印信息，波特率115200，数据位8，奇偶校验位无，停止位1。
*                2. 务必将编辑器的缩进参数和TAB设置为4来阅读本文件，要不代码显示不整齐。
*
*	修改记录 :
*		版本号   日期         作者        说明
*		V1.0    2019-11-02   Eric2013     1. CMSIS软包版本 V5.6.0
*                                         2. HAL库版本 V2.4.0
*
*	Copyright (C), 2018-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/	
#include "bsp.h"			 /* 底层硬件驱动 */
#include "arm_math.h"



/* 定义例程名和例程发布日期 */
#define EXAMPLE_NAME	"V5-DSP功能函数（数据拷贝，数据填充和浮点转定点）"
#define EXAMPLE_DATE	"2019-12-26"
#define DEMO_VER		"1.0"

/* 仅允许本文件内调用的函数声明 */
static void PrintfLogo(void);
static void PrintfHelp(void);
static void DSP_Copy(void);
static void DSP_Fill(void);
static void DSP_FloatToFix(void);


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
				case KEY_DOWN_K1:		    /* K1键按下，数据复制 */
		 		    DSP_Copy();
					break;
					
				case KEY_DOWN_K2:			/* K2键按下，数据填充 */
					DSP_Fill();
					break;

				case KEY_DOWN_K3:			/* K3键按下，浮点转定点 */
					DSP_FloatToFix();
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
*	函 数 名: DSP_Copy
*	功能说明: 数据拷贝
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DSP_Copy(void)
{
	float32_t pSrc[10] = {0.6557,  0.0357,  0.8491,  0.9340, 0.6787,  0.7577,  0.7431,  0.3922,  0.6555,  0.1712};
	float32_t pDst[10];
	uint32_t pIndex;
	
	q31_t pSrc1[10];
	q31_t pDst1[10];
	
	q15_t pSrc2[10];
	q15_t pDst2[10];
	
	q7_t pSrc3[10];
	q7_t pDst3[10];
	
	for(pIndex = 0; pIndex < 10; pIndex++)
	{
		printf("pSrc[%d] = %f\r\n", pIndex, pSrc[pIndex]);
	}
	arm_copy_f32(pSrc, pDst, 10);
	for(pIndex = 0; pIndex < 10; pIndex++)
	{
		printf("arm_copy_f32: pDst[%d] = %f\r\n", pIndex, pDst[pIndex]);
	}

	/*****************************************************************/
	for(pIndex = 0; pIndex < 10; pIndex++)
	{
		pSrc1[pIndex] = rand();
		printf("pSrc1[%d] = %d\r\n", pIndex, pSrc1[pIndex]);
	}
	arm_copy_q31(pSrc1, pDst1, 10);
	for(pIndex = 0; pIndex < 10; pIndex++)
	{
		printf("arm_copy_q31: pDst1[%d] = %d\r\n", pIndex, pDst1[pIndex]);
	}
	/*****************************************************************/
	for(pIndex = 0; pIndex < 10; pIndex++)
	{
		pSrc2[pIndex] = rand()%32768;
		printf("pSrc2[%d] = %d\r\n", pIndex, pSrc2[pIndex]);
	}
	arm_copy_q15(pSrc2, pDst2, 10);
	for(pIndex = 0; pIndex < 10; pIndex++)
	{
		printf("arm_copy_q15: pDst2[%d] = %d\r\n", pIndex, pDst2[pIndex]);
	}
	/*****************************************************************/
	for(pIndex = 0; pIndex < 10; pIndex++)
	{
		pSrc3[pIndex] = rand()%128;
		printf("pSrc3[%d] = %d\r\n", pIndex, pSrc3[pIndex]);
	}
	arm_copy_q7(pSrc3, pDst3, 10);
	for(pIndex = 0; pIndex < 10; pIndex++)
	{
		printf("arm_copy_q7: pDst3[%d] = %d\r\n", pIndex, pDst3[pIndex]);
	}
	/*****************************************************************/
	printf("******************************************************************\r\n");
}

/*
*********************************************************************************************************
*	函 数 名: DSP_Fill
*	功能说明: 数据填充
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DSP_Fill(void)
{
	float32_t pDst[10];
	uint32_t pIndex;
	q31_t pDst1[10];
	q15_t pDst2[10];
	q7_t pDst3[10];
	

	arm_fill_f32(3.33f, pDst, 10);
	for(pIndex = 0; pIndex < 10; pIndex++)
	{
		printf("arm_fill_f32: pDst[%d] = %f\r\n", pIndex, pDst[pIndex]);
	}

	/*****************************************************************/
	arm_fill_q31(0x11111111, pDst1, 10);
	for(pIndex = 0; pIndex < 10; pIndex++)
	{
		printf("arm_fill_q31: pDst1[%d] = %x\r\n", pIndex, pDst1[pIndex]);
	}
	/*****************************************************************/
	arm_fill_q15(0x1111, pDst2, 10);
	for(pIndex = 0; pIndex < 10; pIndex++)
	{
		printf("arm_fill_q15: pDst2[%d] = %d\r\n", pIndex, pDst2[pIndex]);
	}
	/*****************************************************************/
	arm_fill_q7(0x11, pDst3, 10);
	for(pIndex = 0; pIndex < 10; pIndex++)
	{
		printf("arm_fill_q7: pDst3[%d] = %d\r\n", pIndex, pDst3[pIndex]);
	}
	/*****************************************************************/
	printf("******************************************************************\r\n");
}

/*
*********************************************************************************************************
*	函 数 名: DSP_FloatToFix
*	功能说明: 浮点数转定点数
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DSP_FloatToFix(void)
{
	float32_t pSrc[10] = {0.6557,  0.0357,  0.8491,  0.9340, 0.6787,  0.7577,  0.7431,  0.3922,  0.6555,  0.1712};
	uint32_t pIndex;
	q31_t pDst1[10];
	q15_t pDst2[10];
	q7_t pDst3[10];
	
	for(pIndex = 0; pIndex < 10; pIndex++)
	{
		printf("pSrc[%d] = %f\r\n", pIndex, pSrc[pIndex]);
	}
	
	/*****************************************************************/
	arm_float_to_q31(pSrc, pDst1, 10);
	for(pIndex = 0; pIndex < 10; pIndex++)
	{
		printf("arm_float_to_q31: pDst[%d] = %d\r\n", pIndex, pDst1[pIndex]);
	}
	
	/*****************************************************************/
	arm_float_to_q15(pSrc, pDst2, 10);
	for(pIndex = 0; pIndex < 10; pIndex++)
	{
		printf("arm_float_to_q15: pDst1[%d] = %d\r\n", pIndex, pDst2[pIndex]);
	}
	
	/*****************************************************************/
	arm_float_to_q7(pSrc, pDst3, 10);
	for(pIndex = 0; pIndex < 10; pIndex++)
	{
		printf("arm_float_to_q7: pDst2[%d] = %d\r\n", pIndex, pDst3[pIndex]);
	}
	/*****************************************************************/
	printf("******************************************************************\r\n");
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
	printf("2. 按下按键K1, 数据拷贝\r\n");
	printf("3. 按下按键K2, 数据填充\r\n");
	printf("4. 按下按键K3, 浮点转定点\r\n");
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
