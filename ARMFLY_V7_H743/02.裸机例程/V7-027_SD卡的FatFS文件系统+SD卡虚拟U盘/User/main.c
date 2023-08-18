/*
*********************************************************************************************************
*
*	模块名称 : 主程序模块
*	文件名称 : main.c
*	版    本 : V1.0
*	说    明 : 学习SD卡的FatFS移植和SD卡模拟U盘的实现。
*              实验目的：
*                1. 学习SD卡的FatFS移植和模拟U盘的实现。
*              实验内容：
*                1、V7开发板的SD卡接口是用的SDMMC1，而这个接口仅支持AXI SRAM区访问，其它SRAM和TCP均不支持。
*                2、AXI SRAM的主频是200MHz，需要Cache配合提速，所以此例子主RAM直接使用性能最高的DTCM。仅DMA
*                   操作SDIO的地方使用AXI SRAM。
*                3、详情在此贴进行了说明：http://forum.armfly.com/forum.php?mod=viewthread&tid=91531
*                4、板子正常运行时LED2闪烁。
*              实验操作：
*                支持以下6个功能，用户通过电脑端串口软件发送字符命令给开发板即可
*                printf("请选择操作命令，打开SD卡模拟U盘操作期间不支持再调用命令1-6:\r\n");
*                printf("1 - 显示根目录下的文件列表\r\n");
*                printf("2 - 创建一个新文件armfly.txt\r\n");
*                printf("3 - 读armfly.txt文件的内容\r\n");
*                printf("4 - 创建目录\r\n");
*                printf("5 - 删除文件和目录\r\n");
*                printf("6 - 读写文件速度测试\r\n");
*                printf("a - 打开SD卡模拟U盘\r\n");
*                printf("b - 关闭SD卡模拟U盘\r\n");
*              注意事项：
*                1. 本实验推荐使用串口软件SecureCRT查看打印信息，波特率115200，数据位8，奇偶校验位无，停止位1。
*                2. 务必将编辑器的缩进参数和TAB设置为4来阅读本文件，要不代码显示不整齐。
*                3. SD卡模拟U盘是用的MicroUSB接口。
*
*	修改记录 :
*		版本号   日期         作者        说明
*		V1.0    2018-12-12   Eric2013     1. CMSIS软包版本 V5.4.0
*                                         2. HAL库版本 V1.3.0
*                                         3. FatFS版本V0.12c
*
*	Copyright (C), 2018-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/
#include "bsp.h"			/* 底层硬件驱动 */



/* 定义例程名和例程发布日期 */
#define EXAMPLE_NAME	"V7-SD卡的FatFS文件系统+SD卡虚拟U盘"
#define EXAMPLE_DATE	"2018-12-12"
#define DEMO_VER		"1.0"

/* 定义个变量，避免警告 */
#if defined ( __CC_ARM )
__attribute__((section (".RAM_D1"))) uint16_t TempValues1;
__attribute__((section (".RAM_D2"))) uint16_t TempValues2;
__attribute__((section (".RAM_D3"))) uint16_t TempValues3;
#endif

static void PrintfLogo(void);
extern void DemoFatFS(void);

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

#if defined ( __CC_ARM )	
	TempValues1 = 0; /* 避免MDK警告 */  
	TempValues2 = 0;
	TempValues3 = 0;		
#endif
	
	bsp_Init();		/* 硬件初始化 */
	
	PrintfLogo();	/* 打印例程名称和版本等信息 */

	DemoFatFS();    /* SD卡测试 */
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
