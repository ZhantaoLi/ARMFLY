/*
*********************************************************************************************************
*
*	模块名称 : 主程序模块
*	文件名称 : main.c
*	版    本 : V1.0
*	说    明 : DSP复数运算(加法，减法和逆矩阵)
*              实验目的：
*                1. 学习DSP复数运算(加法，减法和逆矩阵)
*              实验内容： 
*                1. 启动一个自动重装软件定时器，每100ms翻转一次LED2。
*                2.	按下按键K1，串口打函数DSP_MatInit的输出数据。
*				 3.	按下按键K2，串口打函数DSP_MatAdd的输出数据。
*                4.	按下按键K3，串口打函数DSP_MatInverse的输出数据。
*                5.	按下摇杆OK键，串口打函数DSP_MatSub的输出数据。
*              注意事项：
*                1. 本实验推荐使用串口软件SecureCRT查看打印信息，波特率115200，数据位8，奇偶校验位无，停止位1。
*                2. 务必将编辑器的缩进参数和TAB设置为4来阅读本文件，要不代码显示不整齐。
*
*	修改记录 :
*		版本号   日期         作者        说明
*		V1.0    2020-03-21   Eric2013     1. CMSIS软包版本 V5.6.0
*                                         2. HAL库版本 V1.3.0
*
*	Copyright (C), 2020-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/	
#include "bsp.h"			 /* 底层硬件驱动 */
#include "arm_math.h"



/* 定义例程名和例程发布日期 */
#define EXAMPLE_NAME	"V7-DSP矩阵运算(加法，减法和逆矩阵)"
#define EXAMPLE_DATE	"2020-03-21"
#define DEMO_VER		"1.0"


/* 仅允许本文件内调用的函数声明 */
static void PrintfLogo(void);
static void PrintfHelp(void);
static void DSP_MatInit(void);
static void DSP_MatAdd(void);
static void DSP_MatInverse(void);
static void DSP_MatSub(void);


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
				case KEY_DOWN_K1:		    /* 按下按键K1，串口打函数DSP_MatInit的输出数据 */
				    DSP_MatInit();
					break;
					
				case KEY_DOWN_K2:			/* 按下按键K2，串口打函数DSP_MatAdd的输出数据 */
					DSP_MatAdd();
					break;

				case KEY_DOWN_K3:			/* 按下按键K3，串口打函数DSP_MatInverse的输出数据 */
					DSP_MatInverse();
					break;
				
				case JOY_DOWN_OK:          /* 按下摇杆OK键，串口打函数DSP_MatSub的输出数据 */
					DSP_MatSub();
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
*	函 数 名: DSP_MatSub
*	功能说明: 矩阵减法
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DSP_MatSub(void)
{
	uint8_t i;
    
	/****浮点数数组******************************************************************/
	float32_t pDataA[9] = {1.1f, 1.1f, 2.1f, 2.1f, 3.1f, 3.1f, 4.1f, 4.1f, 5.1f};
	float32_t pDataB[9] = {1.1f, 1.1f, 2.1f, 2.1f, 3.1f, 3.1f, 4.1f, 4.1f, 5.1f};
	float32_t pDataDst[9];
	
	arm_matrix_instance_f32 pSrcA; //3行3列数据
	arm_matrix_instance_f32 pSrcB; //3行3列数据
	arm_matrix_instance_f32 pDst;
	
	/****定点数Q31数组******************************************************************/
	q31_t pDataA1[9] = {1, 1, 2, 2, 3, 3, 4, 4, 5};
	q31_t pDataB1[9] = {2, 2, 2, 2, 2, 2, 2, 2, 2};
	q31_t pDataDst1[9];
	
	arm_matrix_instance_q31 pSrcA1; //3行3列数据
	arm_matrix_instance_q31 pSrcB1; //3行3列数据
	arm_matrix_instance_q31 pDst1;
	
	/****定点数Q15数组******************************************************************/
	q15_t pDataA2[9] = {1, 1, 2, 2, 3, 3, 4, 4, 5};
	q15_t pDataB2[9] = {2, 2, 2, 2, 23, 2, 2, 2, 2};
	q15_t pDataDst2[9];
	
	arm_matrix_instance_q15 pSrcA2; //3行3列数据
	arm_matrix_instance_q15 pSrcB2; //3行3列数据
	arm_matrix_instance_q15 pDst2;
	
	/****浮点数***********************************************************************/
	pSrcA.numCols = 3;
	pSrcA.numRows = 3;
	pSrcA.pData = pDataA;
	
	pSrcB.numCols = 3;
	pSrcB.numRows = 3;
	pSrcB.pData = pDataB;
	
	pDst.numCols = 3;
	pDst.numRows = 3;
	pDst.pData = pDataDst;
	
	printf("****浮点数******************************************\r\n");
	arm_mat_sub_f32(&pSrcA, &pSrcB, &pDst);
	for(i = 0; i < 9; i++)
	{
		printf("pDataDst[%d] = %f\r\n", i, pDataDst[i]);
	}
	
	/****定点数Q31***********************************************************************/
	pSrcA1.numCols = 3;
	pSrcA1.numRows = 3;
	pSrcA1.pData = pDataA1;
	
	pSrcB1.numCols = 3;
	pSrcB1.numRows = 3;
	pSrcB1.pData = pDataB1;
	
	pDst1.numCols = 3;
	pDst1.numRows = 3;
	pDst1.pData = pDataDst1;
	
	printf("****定点数Q31******************************************\r\n");
	arm_mat_sub_q31(&pSrcA1, &pSrcB1, &pDst1);
	for(i = 0; i < 9; i++)
	{
		printf("pDataDst1[%d] = %d\r\n", i, pDataDst1[i]);
	}
	
	
	/****定点数Q15***********************************************************************/
	pSrcA2.numCols = 3;
	pSrcA2.numRows = 3;
	pSrcA2.pData = pDataA2;
	
	pSrcB2.numCols = 3;
	pSrcB2.numRows = 3;
	pSrcB2.pData = pDataB2;
	
	pDst2.numCols = 3;
	pDst2.numRows = 3;
	pDst2.pData = pDataDst2;
	
	printf("****定点数Q15******************************************\r\n");
	arm_mat_sub_q15(&pSrcA2, &pSrcB2, &pDst2);
	for(i = 0; i < 9; i++)
	{
		printf("pDataDst2[%d] = %d\r\n", i, pDataDst2[i]);
	}
}

/*
*********************************************************************************************************
*	函 数 名: DSP_MatAdd
*	功能说明: 矩阵求和
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DSP_MatAdd(void)
{
	uint8_t i;

	/****浮点数数组******************************************************************/
	float32_t pDataA[9] = {1.1f, 1.1f, 2.1f, 2.1f, 3.1f, 3.1f, 4.1f, 4.1f, 5.1f};
	float32_t pDataB[9] = {1.1f, 1.1f, 2.1f, 2.1f, 3.1f, 3.1f, 4.1f, 4.1f, 5.1f};
	float32_t pDataDst[9];
	
	arm_matrix_instance_f32 pSrcA; //3行3列数据
	arm_matrix_instance_f32 pSrcB; //3行3列数据
	arm_matrix_instance_f32 pDst;
	
	/****定点数Q31数组******************************************************************/
	q31_t pDataA1[9] = {1, 1, 2, 2, 3, 3, 4, 4, 5};
	q31_t pDataB1[9] = {1, 1, 2, 2, 3, 3, 4, 4, 5};
	q31_t pDataDst1[9];
	
	arm_matrix_instance_q31 pSrcA1; //3行3列数据
	arm_matrix_instance_q31 pSrcB1; //3行3列数据
	arm_matrix_instance_q31 pDst1;
	
	/****定点数Q15数组******************************************************************/
	q15_t pDataA2[9] = {1, 1, 2, 2, 3, 3, 4, 4, 5};
	q15_t pDataB2[9] = {1, 1, 2, 2, 3, 3, 4, 4, 5};
	q15_t pDataDst2[9];
	
	arm_matrix_instance_q15 pSrcA2; //3行3列数据
	arm_matrix_instance_q15 pSrcB2; //3行3列数据
	arm_matrix_instance_q15 pDst2;
	
	/****浮点数***********************************************************************/
	pSrcA.numCols = 3;
	pSrcA.numRows = 3;
	pSrcA.pData = pDataA;
	
	pSrcB.numCols = 3;
	pSrcB.numRows = 3;
	pSrcB.pData = pDataB;
	
	pDst.numCols = 3;
	pDst.numRows = 3;
	pDst.pData = pDataDst;
	
	printf("****浮点数******************************************\r\n");
	arm_mat_add_f32(&pSrcA, &pSrcB, &pDst);
	for(i = 0; i < 9; i++)
	{
		printf("pDataDst[%d] = %f\r\n", i, pDataDst[i]);
	}
	
	
	/****定点数Q31***********************************************************************/
	pSrcA1.numCols = 3;
	pSrcA1.numRows = 3;
	pSrcA1.pData = pDataA1;
	
	pSrcB1.numCols = 3;
	pSrcB1.numRows = 3;
	pSrcB1.pData = pDataB1;
	
	pDst1.numCols = 3;
	pDst1.numRows = 3;
	pDst1.pData = pDataDst1;
	
	printf("****定点数Q31******************************************\r\n");
	arm_mat_add_q31(&pSrcA1, &pSrcB1, &pDst1);
	for(i = 0; i < 9; i++)
	{
		printf("pDataDst1[%d] = %d\r\n", i, pDataDst1[i]);
	}
	
	
	/****定点数Q15***********************************************************************/
	pSrcA2.numCols = 3;
	pSrcA2.numRows = 3;
	pSrcA2.pData = pDataA2;
	
	pSrcB2.numCols = 3;
	pSrcB2.numRows = 3;
	pSrcB2.pData = pDataB2;
	
	pDst2.numCols = 3;
	pDst2.numRows = 3;
	pDst2.pData = pDataDst2;
	
	printf("****定点数Q15******************************************\r\n");
	arm_mat_add_q15(&pSrcA2, &pSrcB2, &pDst2);
	for(i = 0; i < 9; i++)
	{
		printf("pDataDst2[%d] = %d\r\n", i, pDataDst2[i]);
	}
	
}

/*
*********************************************************************************************************
*	函 数 名: DSP_MatInit
*	功能说明: 矩阵数据初始化
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DSP_MatInit(void)
{
	uint8_t i;
    
	/****浮点数数组******************************************************************/
	float32_t pDataA[9] = {1.1f, 1.1f, 2.1f, 2.1f, 3.1f, 3.1f, 4.1f, 4.1f, 5.1f};
	
	arm_matrix_instance_f32 pSrcA; //3行3列数据
	
	/****定点数Q31数组******************************************************************/
	q31_t pDataA1[9] = {1, 1, 2, 2, 3, 3, 4, 4, 5};
	
	arm_matrix_instance_q31 pSrcA1; //3行3列数据
	
	/****定点数Q15数组******************************************************************/
	q15_t pDataA2[9] = {1, 1, 2, 2, 3, 3, 4, 4, 5};
	
	arm_matrix_instance_q15 pSrcA2; //3行3列数据
	
	/****浮点数***********************************************************************/	
	printf("****浮点数******************************************\r\n");
	arm_mat_init_f32(&pSrcA, 3,3, pDataA);
	for(i = 0; i < 9; i++)
	{
		printf("pDataA[%d] = %f\r\n", i, pDataA[i]);
	}
	
	/****定点数Q31***********************************************************************/
	printf("****浮点数******************************************\r\n");
	arm_mat_init_q31(&pSrcA1, 3,3, pDataA1);
	for(i = 0; i < 9; i++)
	{
		printf("pDataA1[%d] = %d\r\n", i, pDataA1[i]);
	}
	
	/****定点数Q15***********************************************************************/
	printf("****浮点数******************************************\r\n");
	arm_mat_init_q15(&pSrcA2, 3,3, pDataA2);
	for(i = 0; i < 9; i++)
	{
		printf("pDataA2[%d] = %d\r\n", i, pDataA2[i]);
	}
}

/*
*********************************************************************************************************
*	函 数 名: DSP_MatInverse
*	功能说明: 求逆矩阵
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DSP_MatInverse(void)
{
	uint8_t i;
    
	arm_status sta;
	
	/****浮点数数组******************************************************************/
	float32_t pDataB[36];
	float32_t pDataA[36] = {
                1.0f,   0.0f,   0.0f,  0.0f,  0.0f,  0.0f,
                0.0f,   1.0f,   0.0f,  0.0f,  1.0f,  0.0f,
                0.0f,   0.0f,   2.0f,  0.0f,  0.0f,  0.0f,
                0.0f,   0.0f,   0.0f,  2.0f,  0.0f,  1.0f,
                0.0f,   0.0f,   0.0f,  0.0f,  3.0f,  0.0f,
                0.0f,   0.0f,   0.0f,  0.0f,  0.0f,  4.0f};
	 
		
	arm_matrix_instance_f32 pSrcA; //6行6列数据
	arm_matrix_instance_f32 pSrcB; //6行6列数据;

	
	/****浮点数***********************************************************************/
	pSrcA.numCols = 6;
	pSrcA.numRows = 6;
	pSrcA.pData = pDataA;
	
	pSrcB.numCols = 6;
	pSrcB.numRows = 6;
	pSrcB.pData = pDataB;
	
	sta = arm_mat_inverse_f32(&pSrcA, &pSrcB);

	/*
		sta = ARM_MATH_SUCCESS, 即返回0，表示求逆矩阵成功。
		sta = ARM_MATH_SINGULAR, 即返回-5，表示求逆矩阵失败，也表示不可逆。
		注意，ARM提供的DSP库逆矩阵求发有局限性，通过Matlab验证是可以求逆矩阵的，而DSP库却不能正确求解。
		
	*/
	printf("----sta %d\r\n", sta);
    
	for(i = 0; i < 36; i++)
	{
		printf("pDataB[%d] = %f\r\n", i, pDataB[i]);
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
	printf("2. 按下按键K1，串口打函数DSP_MatInit的输出数据\r\n");
	printf("3. 按下按键K2，串口打函数DSP_MatAdd的输出数据\r\n");
	printf("4. 按下按键K3，串口打函数DSP_MatInverse的输出数据\r\n");
	printf("5. 按下摇杆OK键，串口打函数DSP_MatSub的输出数据\r\n");
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
