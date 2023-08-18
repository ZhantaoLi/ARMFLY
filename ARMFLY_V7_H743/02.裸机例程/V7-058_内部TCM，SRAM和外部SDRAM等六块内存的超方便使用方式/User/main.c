/*
*********************************************************************************************************
*
*	模块名称 : 主程序模块
*	文件名称 : main.c
*	版    本 : V1.0
*	说    明 : 内部TCM，SRAM，外部SDRAM等六块内存的超方便使用方式。
*              实验目的：
*                1. 学习TCM，SRAM等六块内存的超方便使用方式。
*              实验内容：
*                1. 启动自动重装软件定时器0，每100ms翻转一次LED2。
*              实验操作：
*                1. K1键按下，操作AXI SRAM。
*                2. K2键按下，操作D2域的SRAM1，SRAM2和SRAM3。
*                3. K3键按下，操作D3域的SRAM4。
*                4. OK键按下，操作SDRAM。        
*              注意事项：
*                1. 本实验推荐使用串口软件SecureCRT查看打印信息，波特率115200，数据位8，奇偶校验位无，停止位1。
*                2. 务必将编辑器的缩进参数和TAB设置为4来阅读本文件，要不代码显示不整齐。
*
*	修改记录 :
*		版本号   日期         作者        说明
*		V1.0    2020-07-08   Eric2013     1. CMSIS软包版本 V5.4.0
*                                         2. HAL库版本 V1.3.0
*
*	Copyright (C), 2018-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/	
#include "bsp.h"			/* 底层硬件驱动 */



/* 定义例程名和例程发布日期 */
#define EXAMPLE_NAME	"V7-内部TCM，SRAM，外部SDRAM等六块内存的超方便使用方式"
#define EXAMPLE_DATE	"2020-07-08"
#define DEMO_VER		"1.0"

static void PrintfLogo(void);
static void PrintfHelp(void);


/* IAR ---------------------------------------------*/
#if defined ( __ICCARM__ )
/* 定义在512KB AXI SRAM里面的变量 */
#pragma location = ".RAM_D1"  
uint32_t AXISRAMBuf[10];
#pragma location = ".RAM_D1"  
uint16_t AXISRAMCount;

/* 定义在128KB SRAM1(0x30000000) + 128KB SRAM2(0x30020000) + 32KB SRAM3(0x30040000)里面的变量 */
#pragma location = ".RAM_D2" 
uint32_t D2SRAMBuf[10];
#pragma location = ".RAM_D2" 
uint16_t D2SRAMount;

/* 定义在64KB SRAM4(0x38000000)里面的变量 */
#pragma location = ".RAM_D3"  
uint32_t D3SRAMBuf[10];
#pragma location = ".RAM_D3"  
uint16_t D3SRAMCount;

/* 定义在64KB SRAM4(0x38000000)里面的变量 */
#pragma location = ".RAM_D3"  
uint32_t D3SRAMBuf[10];
#pragma location = ".RAM_D3"  
uint16_t D3SRAMCount;

/* 定义在32MB SDRAM(0xC0000000)里面的变量 */
#pragma location = ".RAM_SDRAM"  
uint32_t SDRAMSRAMBuf[10];
#pragma location = ".RAM_SDRAM"  
uint16_t SDRAMSRAMCount;

/* MDK ----------------------------------------------*/
#elif defined ( __CC_ARM )

/* 定义在512KB AXI SRAM里面的变量 */
__attribute__((section (".RAM_D1"))) uint32_t AXISRAMBuf[10];
__attribute__((section (".RAM_D1"))) uint16_t AXISRAMCount;

/* 定义在128KB SRAM1(0x30000000) + 128KB SRAM2(0x30020000) + 32KB SRAM3(0x30040000)里面的变量 */
__attribute__((section (".RAM_D2"))) uint32_t D2SRAMBuf[10];
__attribute__((section (".RAM_D2"))) uint16_t D2SRAMount;

/* 定义在64KB SRAM4(0x38000000)里面的变量 */
__attribute__((section (".RAM_D3"))) uint32_t D3SRAMBuf[10];
__attribute__((section (".RAM_D3"))) uint16_t D3SRAMCount;

/* 定义在32MB SDRAM(0xC0000000)里面的变量 */
__attribute__((section (".RAM_SDRAM"),zero_init)) uint16_t SDRAMSRAMCount;
__attribute__((section (".RAM_SDRAM"),zero_init)) uint32_t SDRAMSRAMBuf[10];

#endif


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
	
	PrintfLogo();	/* 打印例程名称和版本等信息 */
	PrintfHelp();	/* 打印操作提示 */

	bsp_StartAutoTimer(0, 100);	/* 启动1个100ms的自动重装的定时器 */
	
	AXISRAMCount = 0;
	D2SRAMount = 0;
	D3SRAMCount = 0;
	SDRAMSRAMCount = 0;
	
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

		/* 按键滤波和检测由后台systick中断服务程序实现，我们只需要调用bsp_GetKey读取键值即可。 */
		ucKeyCode = bsp_GetKey();	/* 读取键值, 无键按下时返回 KEY_NONE = 0 */
		if (ucKeyCode != KEY_NONE)
		{
			switch (ucKeyCode)
			{
				case KEY_DOWN_K1:			/* K1键按下，操作AXI SRAM */
					AXISRAMBuf[0] = AXISRAMCount++;
					AXISRAMBuf[5] = AXISRAMCount++;
					AXISRAMBuf[9] = AXISRAMCount++;
					printf("K1键按下, AXISRAMBuf[0] = %d, AXISRAMBuf[5] = %d, AXISRAMBuf[9] = %d\r\n", 
																						AXISRAMBuf[0],
																						AXISRAMBuf[5],
																						AXISRAMBuf[9]);
					break;

				case KEY_DOWN_K2:			/* K2键按下，操作D2域的SRAM1，SRAM2和SRAM3 */
					D2SRAMBuf[0] = D2SRAMount++;
					D2SRAMBuf[5] = D2SRAMount++;
					D2SRAMBuf[9] = D2SRAMount++;
					printf("K2键按下, D2SRAMBuf[0] = %d, D2SRAMBuf[5] = %d, D2SRAMBuf[9] = %d\r\n", 
																						D2SRAMBuf[0],
																						D2SRAMBuf[5],
																						D2SRAMBuf[9]);
                    break;
				
				case KEY_DOWN_K3:			/* K3键按下，操作D3域的SRAM4 */			
                 	D3SRAMBuf[0] = D3SRAMCount++;
					D3SRAMBuf[5] = D3SRAMCount++;
					D3SRAMBuf[9] = D3SRAMCount++;
					printf("K3键按下, D3SRAMBuf[0] = %d, D3SRAMBuf[5] = %d, D3SRAMBuf[9] = %d\r\n", 
																						D3SRAMBuf[0],
																						D3SRAMBuf[5],
																						D3SRAMBuf[9]);
                  break;
				
				case JOY_DOWN_OK:			/* 摇杆OK键按下，操作SDRAM */			
                 	SDRAMSRAMBuf[0] = SDRAMSRAMCount++;
					SDRAMSRAMBuf[5] = SDRAMSRAMCount++;
					SDRAMSRAMBuf[9] = SDRAMSRAMCount++;
					printf("K3键按下, SDRAMSRAMBuf[0] = %d, SDRAMSRAMBuf[5] = %d, SDRAMSRAMBuf[9] = %d\r\n", 
																						SDRAMSRAMBuf[0],
																						SDRAMSRAMBuf[5],
																						SDRAMSRAMBuf[9]);
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
*	功能说明: 打印操作提示
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void PrintfHelp(void)
{
	printf("操作提示:\r\n");
	printf("1. K1键按下，操作AXI SRAM\r\n");
	printf("2. K2键按下，操作D2域的SRAM1，SRAM2和SRAM3\r\n");
	printf("3. K3键按下，操作D3域的SRAM4\r\n");
	printf("4. OK键按下，操作SDRAM\r\n");
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
