/*
*********************************************************************************************************
*
*	模块名称 : 主程序模块
*	文件名称 : main.c
*	版    本 : V1.0
*	说    明 : DSP基础运算（相反数，偏移，移位，减法和比例因子）
*              实验目的：
*                1. 学习基础运算（相反数，偏移，移位，减法和比例因子）
*              实验内容： 
*                1. 启动一个自动重装软件定时器，每100ms翻转一次LED2。
*                2. 按下按键K1, DSP求相反数运算。
*                3. 按下按键K2, DSP求偏移运算。
*                4. 按下按键K3, DSP求移位运算。
*                5. 按下摇杆OK键, DSP求减法运算。
*                6. 按下摇杆上键, DSP比例因子运算。		
*              注意事项：
*                1. 本实验推荐使用串口软件SecureCRT查看打印信息，波特率115200，数据位8，奇偶校验位无，停止位1。
*                2. 务必将编辑器的缩进参数和TAB设置为4来阅读本文件，要不代码显示不整齐。
*
*	修改记录 :
*		版本号   日期         作者        说明
*		V1.0    2019-10-05   Eric2013     1. CMSIS软包版本 V5.6.0
*                                         2. HAL库版本 V1.3.0
*
*	Copyright (C), 2018-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/	
#include "bsp.h"			 /* 底层硬件驱动 */
#include "arm_math.h"



/* 定义例程名和例程发布日期 */
#define EXAMPLE_NAME	"V7-基础运算（相反数，偏移，移位，减法和比例因子）"
#define EXAMPLE_DATE	"2019-10-05"
#define DEMO_VER		"1.0"

/* 仅允许本文件内调用的函数声明 */
static void PrintfLogo(void);
static void PrintfHelp(void);
static void DSP_Negate(void);
static void DSP_Offset(void);
static void DSP_Shift(void);
static void DSP_Sub(void);
static void DSP_Scale(void);


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
				case KEY_DOWN_K1:			/* K1键按下，求相反数 */
					DSP_Negate();
					break;

				case KEY_DOWN_K2:			/* K2键按下, 求偏移 */
					DSP_Offset();
					break;

				case KEY_DOWN_K3:			/* K3键按下，求移位 */
					DSP_Shift();
					break;
	
				case JOY_DOWN_OK:	        /* 摇杆OK键按下，求减法 */
					DSP_Sub();
					break;
				
				case JOY_DOWN_U:	        /* 摇杆上键按下，求比例因子计算 */
					DSP_Scale();
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
*	函 数 名: DSP_Negate
*	功能说明: 求相反数
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DSP_Negate(void)
{
	float32_t pSrc = 0.0f;
    float32_t pDst;
	
	q31_t pSrc1 = 0;
	q31_t pDst1;
	
	q15_t pSrc2 = 0;
	q15_t pDst2;
	
	q7_t pSrc3 = 0; 
	q7_t pDst3;
	
	/*求相反数*********************************/	
	pSrc -= 1.23f;
	arm_negate_f32(&pSrc, &pDst, 1);
	printf("arm_negate_f32 = %f\r\n", pDst);

	pSrc1 -= 1;
	arm_negate_q31(&pSrc1, &pDst1, 1);
	printf("arm_negate_q31 = %d\r\n", pDst1);

	pSrc2 -= 1;
	arm_negate_q15(&pSrc2, &pDst2, 1);
	printf("arm_negate_q15 = %d\r\n", pDst2);

	pSrc3 += 1; 
	arm_negate_q7(&pSrc3, &pDst3, 1);
	printf("arm_negate_q7 = %d\r\n", pDst3);
	printf("***********************************\r\n");
}

/*
*********************************************************************************************************
*	函 数 名: DSP_Offset
*	功能说明: 偏移
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DSP_Offset(void)
{
	float32_t   pSrcA = 0.0f;
	float32_t   Offset = 0.0f;  
	float32_t   pDst;  
	
	q31_t  pSrcA1 = 0;  
	q31_t  Offset1 = 0;  
	q31_t  pDst1;  

	q15_t  pSrcA2 = 0;  
	q15_t  Offset2 = 0;  
	q15_t  pDst2; 

	q7_t  pSrcA3 = 0; 
	q7_t  Offset3 = 0;  
	q7_t  pDst3;  

	/*求偏移*********************************/		
	Offset--;
	arm_offset_f32(&pSrcA, Offset, &pDst, 1);
	printf("arm_offset_f32 = %f\r\n", pDst);

	Offset1--;
	arm_offset_q31(&pSrcA1, Offset1, &pDst1, 1);
	printf("arm_offset_q31 = %d\r\n", pDst1);

	Offset2--;
	arm_offset_q15(&pSrcA2, Offset2, &pDst2, 1);
	printf("arm_offset_q15 = %d\r\n", pDst2);

	Offset3--;
	arm_offset_q7(&pSrcA3, Offset3, &pDst3, 1);
	printf("arm_offset_q7 = %d\r\n", pDst3);
	printf("***********************************\r\n");
}

/*
*********************************************************************************************************
*	函 数 名: DSP_Shift
*	功能说明: 移位
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DSP_Shift(void)
{
	q31_t  pSrcA1 = 0x88886666;  
	q31_t  pDst1;  

	q15_t  pSrcA2 = 0x8866;  
	q15_t  pDst2; 

	q7_t  pSrcA3 = 0x86; 
	q7_t  pDst3;  


	/*求移位*********************************/	
	arm_shift_q31(&pSrcA1, 3, &pDst1, 1);
	printf("arm_shift_q31 = %8x\r\n", pDst1);

	arm_shift_q15(&pSrcA2, -3, &pDst2, 1);
	printf("arm_shift_q15 = %4x\r\n", pDst2);

	arm_shift_q7(&pSrcA3, 3, &pDst3, 1);
	printf("arm_shift_q7 = %2x\r\n", pDst3);
	printf("***********************************\r\n");
}

/*
*********************************************************************************************************
*	函 数 名: DSP_Sub
*	功能说明: 减法
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DSP_Sub(void)
{
	float32_t   pSrcA[5] = {1.0f,1.0f,1.0f,1.0f,1.0f};
	float32_t   pSrcB[5] = {1.0f,1.0f,1.0f,1.0f,1.0f};  
	float32_t   pDst[5];  
	
	q31_t  pSrcA1[5] = {1,1,1,1,1};  
	q31_t  pSrcB1[5] = {1,1,1,1,1};  
	q31_t  pDst1[5];   

	q15_t  pSrcA2[5] = {1,1,1,1,1};  
	q15_t  pSrcB2[5] = {1,1,1,1,1};  
	q15_t  pDst2[5];   

	q7_t  pSrcA3[5] = {0x70,1,1,1,1}; 
	q7_t  pSrcB3[5] = {0x7f,1,1,1,1};  
	q7_t  pDst3[5];  


	/*求减法*********************************/	
	pSrcA[0] += 1.1f;
	arm_sub_f32(pSrcA, pSrcB, pDst, 5);
	printf("arm_sub_f32 = %f\r\n", pDst[0]);
	
	pSrcA1[0] += 1;
	arm_sub_q31(pSrcA1, pSrcB1, pDst1, 5);
	printf("arm_sub_q31 = %d\r\n", pDst1[0]);

	pSrcA2[0] += 1;
	arm_sub_q15(pSrcA2, pSrcB2, pDst2, 5);
	printf("arm_sub_q15 = %d\r\n", pDst2[0]);

	pSrcA3[0] += 1;
	arm_sub_q7(pSrcA3, pSrcB3, pDst3, 5);
	printf("arm_sub_q7 = %d\r\n", pDst3[0]);
	printf("***********************************\r\n");
}

/*
*********************************************************************************************************
*	函 数 名: DSP_Scale
*	功能说明: 比例因子
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DSP_Scale(void)
{
	float32_t   pSrcA[5] = {1.0f,1.0f,1.0f,1.0f,1.0f};
	float32_t   scale = 0.0f;  
	float32_t   pDst[5];  
	
	q31_t  pSrcA1[5] = {0x6fffffff,1,1,1,1};  
	q31_t  scale1 = 0x6fffffff;  
	q31_t  pDst1[5];   

	q15_t  pSrcA2[5] = {0x6fff,1,1,1,1};  
	q15_t  scale2 = 0x6fff;  
	q15_t  pDst2[5];   

	q7_t  pSrcA3[5] = {0x70,1,1,1,1}; 
	q7_t  scale3 = 0x6f;  
	q7_t pDst3[5];  

	/*求比例因子计算*********************************/	
	scale += 0.1f;
	arm_scale_f32(pSrcA, scale, pDst, 5);
	printf("arm_scale_f32 = %f\r\n", pDst[0]);
	
	scale1 += 1;
	arm_scale_q31(pSrcA1, scale1, 0, pDst1, 5);
	printf("arm_scale_q31 = %x\r\n", pDst1[0]);

	scale2 += 1;
	arm_scale_q15(pSrcA2, scale2, 0, pDst2, 5);
	printf("arm_scale_q15 = %x\r\n", pDst2[0]);

	scale3 += 1;
	arm_scale_q7(pSrcA3, scale3, 0, pDst3, 5);
	printf("arm_scale_q7 = %x\r\n", pDst3[0]);
	printf("***********************************\r\n");
}

/*
*********************************************************************************************************
*	函 数 名: PrintfHelp
*	功能说明: 显示操作提示菜单
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void PrintfHelp(void)
{
	printf("操作提示:\r\n");
	printf("1. 启动一个自动重装软件定时器，每100ms翻转一次LED2\r\n");
	printf("2. 按下按键K1, DSP求相反数运算\r\n");
	printf("3. 按下按键K2, DSP求偏移运算\r\n");
	printf("4. 按下按键K3, DSP求移位运算\r\n");
	printf("5. 按下摇杆OK键, DSP求减法运算\r\n");
	printf("6. 按下摇杆上键, DSP比例因子运算\r\n");		
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
