/*
*********************************************************************************************************
*
*	模块名称 : 主程序模块
*	文件名称 : main.c
*	版    本 : V1.0
*	说    明 : Matlab的串口数据通信。
*              实验目的：
*                1、学习matlab的串口数据通信。
*              实验内容：
*                1、启动一个自动重装软件定时器，每100ms翻转一次LED2。
*                2、请优先运行开发板，然后运行matlab。
*                3、调试matlab串口数据发送前，请务必关闭串口助手。
*              注意事项：
*                1、本实验推荐使用串口软件SecureCRT查看打印信息，波特率115200，数据位8，奇偶校验位无，停止位1。
*                2、务必将编辑器的缩进参数和TAB设置为4来阅读本文件，要不代码显示不整齐。
*
*	修改记录 :
*		版本号   日期         作者        说明
*		V1.0    2019-09-07   Eric2013     1. CMSIS软包版本 V5.4.0
*                                         2. HAL库版本 V1.3.0
*
*	Copyright (C), 2018-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/	
#include "bsp.h"			/* 底层硬件驱动 */



/* 定义例程名和例程发布日期 */
#define EXAMPLE_NAME	"V7-Matlab的串口数据通信"
#define EXAMPLE_DATE	"2019-09-07"
#define DEMO_VER		"1.0"

static void PrintfLogo(void);
static void PrintfHelp(void);
static void Serial_sendDataMATLAB(void);

/* 定义发送给matlab的数据格式  */
typedef struct
{
	uint8_t  data4;	
	uint8_t  data5;
	uint8_t  data6;		
	uint8_t  data7;
	uint16_t data1;
	uint16_t data2;	
	uint16_t data3;	
}
SENDPARAM_T;

SENDPARAM_T g_SendData;

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
	uint8_t read;
	
	bsp_Init();		/* 硬件初始化 */
	
	PrintfLogo();	/* 打印例程名称和版本等信息 */
	PrintfHelp();	/* 打印操作提示 */

	bsp_StartAutoTimer(0, 50);  /* 启动1个100ms的自动重装的定时器 */
	
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
		
		if (comGetChar(COM1, &read))
		{
			/* 接收到同步帧'$'*/
			if(read == 13)
			{
				bsp_LedToggle(4);
				bsp_DelayMS(10);
				Serial_sendDataMATLAB();
			}
		}
		
		/* 按键滤波和检测由后台systick中断服务程序实现，我们只需要调用bsp_GetKey读取键值即可。 */
		ucKeyCode = bsp_GetKey();	/* 读取键值, 无键按下时返回 KEY_NONE = 0 */
		if (ucKeyCode != KEY_NONE)
		{
			switch (ucKeyCode)
			{
				case KEY_DOWN_K1:			/* K1键按下 */
					printf("K1键按下\r\n");
					break;

				case KEY_UP_K1:				/* K1键弹起 */
					break;

				case KEY_DOWN_K2:			/* K2键按下 */
					printf("K2键按下\r\n");
					break;

				case KEY_UP_K2:				/* K2键弹起 */
					printf("K2键弹起\r\n");
					break;

				case KEY_DOWN_K3:			/* K3键按下 */
					printf("K3键按下\r\n");
					break;

				case KEY_UP_K3:				/* K3键弹起 */
					printf("K3键弹起\r\n");
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
*	函 数 名: Serial_sendDataMATLAB
*	功能说明: 发送串口数据给matlab
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void Serial_sendDataMATLAB(void)
{
	/* 先发同步信号'$' */
	comSendChar(COM1, 13);
	
	/* 发送数据，一共10个字节 */
	g_SendData.data1 = rand()%65536;
	g_SendData.data2 = rand()%65536;
	g_SendData.data3 = rand()%65536;
	g_SendData.data4 = rand()%256;  
	g_SendData.data5 = rand()%256;  
	g_SendData.data6 = rand()%256;  
	g_SendData.data7 = rand()%256;  
	comSendBuf(COM1, (uint8_t *)&g_SendData, 10);
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
	printf("1. 启动一个自动重装软件定时器，每100ms翻转一次LED2\r\n");
	printf("2. 请优先运行开发板，然后运行matlab\r\n");
	printf("3. 调试matlab串口数据发送前，请务必关闭串口助手\r\n");
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
	printf("* 例程名称   : %s\n\r", EXAMPLE_NAME);	/* 打印例程名称 */
	printf("* 例程版本   : %s\n\r", DEMO_VER);		/* 打印例程版本 */
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
