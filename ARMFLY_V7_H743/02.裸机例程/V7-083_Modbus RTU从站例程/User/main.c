/*
*********************************************************************************************************
*
*	模块名称 : 主程序模块
*	文件名称 : main.c
*	版    本 : V1.0
*	说    明 : RS485 MODBUS从站例程（使用的是串口3）。
*              本例程主要讲解MODBUS协议从站的命令处理方法,包含了常用的命令。
*   实验内容：
*              1. 接好硬件,(1)串口1(打印实验数据)  (2)485接口(收发命令)
*              2. MODBUS调试助手发送送命令,串口打印出结果。
*   注意事项：
*              1. 本实验推荐使用串口软件SecureCRT或者H7-TOOL上位机软件查看打印信息，波特率115200，数据位8，奇偶校验位无，停止位2。
*              2. 务必将编辑器的缩进参数和TAB设置为4来阅读本文件，要不代码显示不整齐。
*
*	修改记录 :
*		版本号   日期         作者        说明
*		V1.0    2022-10-02   Eric2013     1. CMSIS软包版本 V5.7.0
*                                         2. HAL库版本 V1.9.0
*                                         
*	Copyright (C), 2020-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/	
#include "bsp.h"			 /* 底层硬件驱动 */
#include "modbus_slave.h"


/* 定义例程名和例程发布日期 */
#define EXAMPLE_NAME	"V7-RS485 MODBUS RTU从站例程"
#define EXAMPLE_DATE	"2022-10-02"
#define DEMO_VER		"1.0"

static void PrintfLogo(void);
static void SetLed(void);

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
	MSG_T ucMsg;

	bsp_Init();							/* 硬件初始化 */
	bsp_InitMsg();
	PrintfLogo();						/* 打印例程信息到串口1 */
	
	bsp_StartAutoTimer(0, 100); /* 启动 1 个 100ms 的自动重装的定时器 */
	
	/* 进入主程序循环体 */
	while (1)
	{
		bsp_Idle(); /* Modbus解析在此函数里面 */
		
		if (bsp_CheckTimer(0)) /* 判断定时器超时时间 */
		{
			/* 每隔 100ms 进来一次 */
			bsp_LedToggle(2); /* 翻转 LED 的状态 */
		}
		
		if (bsp_GetMsg(&ucMsg))
		{
			switch (ucMsg.MsgCode)
			{
				case MSG_MODS_05H:		/* 打印 发送的命令 和 应答的命令  刷新LED状态 */
					SetLed();			/* 设置LED亮灭(处理05H指令) */
					break;
				
				default:
					break;
			}
		}
	}
}

/*
*********************************************************************************************************
*	函 数 名: SetLed
*	功能说明: 设置LED亮灭(处理05H指令)
*	形    参：无
*	返 回 值: 错误代码(无需处理)
*********************************************************************************************************
*/
static void SetLed(void)
{
	if (g_tVar.D01 == 0xFF00) 
	{
		bsp_LedOn(1);
	}
	else
	{
		bsp_LedOff(1);
	}
	
	if (g_tVar.D02 == 0xFF00) 
	{
		bsp_LedOn(2);
	}
	else
	{
		bsp_LedOff(2);
	}	
	
	if (g_tVar.D03 == 0xFF00) 
	{
		bsp_LedOn(3);
	}
	else
	{
		bsp_LedOff(3);
	}	
	
	if (g_tVar.D04 == 0xFF00) 
	{
		bsp_LedOn(4);
	}
	else
	{
		bsp_LedOff(4);
	}	
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
	printf("*************************************************************\r\n");
	
	/* 检测CPU ID */
	{
		uint32_t CPU_Sn0, CPU_Sn1, CPU_Sn2;
		
		CPU_Sn0 = *(__IO uint32_t*)(0x1FF1E800);
		CPU_Sn1 = *(__IO uint32_t*)(0x1FF1E800 + 4);
		CPU_Sn2 = *(__IO uint32_t*)(0x1FF1E800 + 8);

		printf("CPU : STM32H743XIH6, BGA240, 主频: %dMHz\r\n", SystemCoreClock / 1000000);
		printf("UID = %08X %08X %08X\r\n", CPU_Sn2, CPU_Sn1, CPU_Sn0);
	}

	printf("*************************************************************\r\n");
	printf("* 例程名称   : %s\r\n", EXAMPLE_NAME);	/* 打印例程名称 */
	printf("* 例程版本   : %s\r\n", DEMO_VER);		/* 打印例程版本 */
	printf("* 发布日期   : %s\r\n", EXAMPLE_DATE);	/* 打印例程日期 */

	/* 打印ST的HAL库版本 */
	printf("* HAL库版本  : V1.9.0 (STM32H7xx HAL Driver)\r\n");
	printf("* \r\n");	/* 打印一行空格 */
	printf("* QQ    : 1295744630 \r\n");
	printf("* 旺旺  : armfly\r\n");
	printf("* Email : armfly@qq.com \r\n");
	printf("* 微信公众号: anfulai_com \r\n");
	printf("* 淘宝店: anfulai.taobao.com\r\n");
	printf("* Copyright www.armfly.com 安富莱电子\r\n");
	printf("*************************************************************\r\n");
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
