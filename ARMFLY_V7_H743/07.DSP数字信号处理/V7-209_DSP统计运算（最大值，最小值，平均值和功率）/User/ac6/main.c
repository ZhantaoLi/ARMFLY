/*
*********************************************************************************************************
*
*	模块名称 : 主程序模块
*	文件名称 : main.c
*	版    本 : V1.0
*	说    明 : DSP统计运算（最大值，最小值，平均值和功率）
*              实验目的：
*                1. 学习DSP统计运算（最大值，最小值，平均值和功率）
*              实验内容： 
*                1. 启动一个自动重装软件定时器，每100ms翻转一次LED2。
*                2. 按下按键K1, DSP求最大值。
*                3. 按下按键K2, DSP求最小值。
*                4. 按下按键K3, DSP求平均值。
*                5. 按下摇杆OK键, DSP求功率。
*              注意事项：
*                1. 本实验推荐使用串口软件SecureCRT查看打印信息，波特率115200，数据位8，奇偶校验位无，停止位1。
*                2. 务必将编辑器的缩进参数和TAB设置为4来阅读本文件，要不代码显示不整齐。
*
*	修改记录 :
*		版本号   日期         作者        说明
*		V1.0    2019-10-25   Eric2013     1. CMSIS软包版本 V5.6.0
*                                         2. HAL库版本 V1.3.0
*
*	Copyright (C), 2018-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/	
#include "bsp.h"			 /* 底层硬件驱动 */
#include "arm_math.h"



/* 定义例程名和例程发布日期 */
#define EXAMPLE_NAME	"V7-DSP统计运算（最大值，最小值，平均值和功率）"
#define EXAMPLE_DATE	"2019-10-25"
#define DEMO_VER		"1.0"

/* 仅允许本文件内调用的函数声明 */
static void PrintfLogo(void);
static void PrintfHelp(void);
static void DSP_Max(void);
static void DSP_Min(void);
static void DSP_Mean(void);
static void DSP_Power(void);


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
				case KEY_DOWN_K1:			/* K1键按下，求最大值 */
					DSP_Max();
					break;

				case KEY_DOWN_K2:			/* K2键按下, 求小值 */
					DSP_Min();
					break;

				case KEY_DOWN_K3:			/* K3键按下，求平方根 */
					DSP_Mean();
					break;
				
				case JOY_DOWN_OK:            /* 摇杆上键，求功率  */         
					DSP_Power();
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
*	函 数 名: DSP_Max
*	功能说明: 求最大值
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DSP_Max(void)
{
	float32_t pSrc[10] = {0.6948f, 0.3171f, 0.9502f, 0.0344f, 0.4387f, 0.3816f, 0.7655f, 0.7952f, 0.1869f, 0.4898f};
	float32_t pResult;
	uint32_t pIndex;
	
	q31_t pSrc1[10];
	q31_t pResult1;
	
	q15_t pSrc2[10];
	q15_t pResult2;
	
	q7_t pSrc3[10];
	q7_t pResult3;
	
	arm_max_f32(pSrc, 10, &pResult, &pIndex);
	printf("arm_max_f32 : pResult = %f  pIndex = %d\r\n", pResult, pIndex);
	
	/*****************************************************************/
	for(pIndex = 0; pIndex < 10; pIndex++)
	{
		 pSrc1[pIndex] = rand();
	}
	arm_max_q31(pSrc1, 10, &pResult1, &pIndex);
	printf("arm_max_q31 : pResult = %d  pIndex = %d\r\n", pResult1, pIndex);
	
	/*****************************************************************/
	for(pIndex = 0; pIndex < 10; pIndex++)
	{
		 pSrc2[pIndex] = rand()%32768;
	}
	arm_max_q15(pSrc2, 10, &pResult2, &pIndex);
	printf("arm_max_q15 : pResult = %d  pIndex = %d\r\n", pResult2, pIndex);
	
	/*****************************************************************/
	for(pIndex = 0; pIndex < 10; pIndex++)
	{
		 pSrc3[pIndex] = rand()%128;
	}
	arm_max_q7(pSrc3, 10, &pResult3, &pIndex);
	printf("arm_max_q7 : pResult = %d  pIndex = %d\r\n", pResult3, pIndex);
	printf("******************************************************************\r\n");
}

/*
*********************************************************************************************************
*	函 数 名: DSP_Min
*	功能说明: 求最小值
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DSP_Min(void)
{
	float32_t pSrc[10] = {0.6948f, 0.3171f, 0.9502f, 0.0344f, 0.4387f, 0.3816f, 0.7655f, 0.7952f, 0.1869f, 0.4898f};
	float32_t pResult;
	uint32_t pIndex;
	
	q31_t pSrc1[10];
	q31_t pResult1;
	
	q15_t pSrc2[10];
	q15_t pResult2;
	
	q7_t pSrc3[10];
	q7_t pResult3;
	
	arm_min_f32(pSrc, 10, &pResult, &pIndex);
	printf("arm_min_f32 : pResult = %f  pIndex = %d\r\n", pResult, pIndex);
	
	/*****************************************************************/
	for(pIndex = 0; pIndex < 10; pIndex++)
	{
		 pSrc1[pIndex] = rand();
	}
	arm_min_q31(pSrc1, 10, &pResult1, &pIndex);
	printf("arm_min_q31 : pResult = %d  pIndex = %d\r\n", pResult1, pIndex);
	
	/*****************************************************************/
	for(pIndex = 0; pIndex < 10; pIndex++)
	{
		 pSrc2[pIndex] = rand()%32768;
	}
	arm_min_q15(pSrc2, 10, &pResult2, &pIndex);
	printf("arm_min_q15 : pResult = %d  pIndex = %d\r\n", pResult2, pIndex);
	
	/*****************************************************************/
	for(pIndex = 0; pIndex < 10; pIndex++)
	{
		 pSrc3[pIndex] = rand()%128;
	}
	arm_min_q7(pSrc3, 10, &pResult3, &pIndex);
	printf("arm_min_q7 : pResult = %d  pIndex = %d\r\n", pResult3, pIndex);
	printf("******************************************************************\r\n");
}

/*
*********************************************************************************************************
*	函 数 名: DSP_Mean
*	功能说明: 求平均
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DSP_Mean(void)
{
	float32_t pSrc[10] = {0.6948f, 0.3171f, 0.9502f, 0.0344f, 0.4387f, 0.3816f, 0.7655f, 0.7952f, 0.1869f, 0.4898f};
	float32_t pResult;
	uint32_t pIndex;
	
	q31_t pSrc1[10];
	q31_t pResult1;
	
	q15_t pSrc2[10];
	q15_t pResult2;
	
	q7_t pSrc3[10];
	q7_t pResult3;
	
	arm_mean_f32(pSrc, 10, &pResult);
	printf("arm_mean_f32 : pResult = %f\r\n", pResult);
	
	/*****************************************************************/
	for(pIndex = 0; pIndex < 10; pIndex++)
	{
		 pSrc1[pIndex] = rand();
	}
	arm_mean_q31(pSrc1, 10, &pResult1);
	printf("arm_mean_q31 : pResult = %d\r\n", pResult1);
	
	/*****************************************************************/
	for(pIndex = 0; pIndex < 10; pIndex++)
	{
		 pSrc2[pIndex] = rand()%32768;
	}
	arm_mean_q15(pSrc2, 10, &pResult2);
	printf("arm_mean_q15 : pResult = %d\r\n", pResult2);
	
	/*****************************************************************/
	for(pIndex = 0; pIndex < 10; pIndex++)
	{
		 pSrc3[pIndex] = rand()%128;
	}
	arm_mean_q7(pSrc3, 10, &pResult3);
	printf("arm_mean_q7 : pResult = %d\r\n", pResult3);
	printf("******************************************************************\r\n");
}

/*
*********************************************************************************************************
*	函 数 名: DSP_Power
*	功能说明: 求功率
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DSP_Power(void)
{
	float32_t pSrc[10] = {0.6948f, 0.3171f, 0.9502f, 0.0344f, 0.4387f, 0.3816f, 0.7655f, 0.7952f, 0.1869f, 0.4898f};
	float32_t pResult;
	uint32_t pIndex;
	
	q31_t pSrc1[10];
	q63_t pResult1;
	
	q15_t pSrc2[10];
	q63_t pResult2;
	
	q7_t pSrc3[10];
	q31_t pResult3;
	
	arm_power_f32(pSrc, 10, &pResult);
	printf("arm_power_f32 : pResult = %f\r\n", pResult);
	
	/*****************************************************************/
	for(pIndex = 0; pIndex < 10; pIndex++)
	{
		 pSrc1[pIndex] = rand();
	}
	arm_power_q31(pSrc1, 10, &pResult1);
	printf("arm_power_q31 : pResult = %lld\r\n", pResult1);
	
	/*****************************************************************/
	for(pIndex = 0; pIndex < 10; pIndex++)
	{
		 pSrc2[pIndex] = rand()%32768;
	}
	arm_power_q15(pSrc2, 10, &pResult2);
	printf("arm_power_q15 : pResult = %lld\r\n", pResult2);
	
	/*****************************************************************/
	for(pIndex = 0; pIndex < 10; pIndex++)
	{
		 pSrc3[pIndex] = rand()%128;
	}
	arm_power_q7(pSrc3, 10, &pResult3);
	printf("arm_power_q7 : pResult = %d\r\n", pResult3);
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
	printf("2. 按下按键K1, DSP求最大值\r\n");
	printf("3. 按下按键K2, DSP求最小值\r\n");
	printf("4. 按下按键K3, DSP求平均值\r\n");		
	printf("5. 按下摇杆OK键, DSP求功率\r\n");
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
