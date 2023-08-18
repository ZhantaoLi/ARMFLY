/*
*********************************************************************************************************
*
*	模块名称 : 主程序模块
*	文件名称 : main.c
*	版    本 : V1.0
*	说    明 : 基于硬件I2C的bootloader设计
*              实验目的：
*                1. 基于硬件I2C的bootloader设计
*              注意事项：
*                1. 打印方式：
*                   默认使用串口打印，可以使用SecureCRT或者H7-TOOL上位机串口助手查看打印信息，
*                   波特率115200，数据位8，奇偶校验位无，停止位1。
*                2. 务必将编辑器的缩进参数和TAB设置为4来阅读本文件，要不代码显示不整齐。
*
*	修改记录 :
*		版本号   日期         作者        说明
*		V1.0    2022-07-18  Eric2013     1. CMSIS软包版本 V5.7.0
*                                         2. HAL库版本 V1.10.0
*
*	Copyright (C), 2020-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/	
#include "bsp.h"			 /* 底层硬件驱动 */


/* 定义例程名和例程发布日期 */
#define EXAMPLE_NAME	"V7-基于硬件I2C的bootloader设计"
#define EXAMPLE_DATE	"2022-07-18"
#define DEMO_VER		"1.0"


/* 仅允许本文件内调用的函数声明 */
static void PrintfLogo(void);
static void PrintfHelp(void);

extern __IO uint32_t wTransferState;

/*
*********************************************************************************************************
*	                                       宏定义
*********************************************************************************************************
*/
#define AppAddr  0x08100000    /* APP地址 */

__IO uint32_t uwCRCValue;
__IO uint32_t uwExpectedCRCValue;
__IO uint32_t uwAppSize;

uint8_t buf[1024];
uint32_t RecCount = 0;
uint32_t RecCount0 = 0;
uint32_t RecSize = 0;
uint8_t RecCplt = 0;
uint32_t filesize = 0;

static void JumpToApp(void);

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
	uint32_t SectorCount = 0;
	uint32_t SectorRemain = 0;
	uint32_t i;
    uint32_t TotalSize = 0;
	uint8_t ucState;
	

	bsp_Init();		/* 硬件初始化 */
	PrintfLogo();	/* 打印例程信息到串口1 */
	PrintfHelp();	/* 打印操作提示信息 */

	bsp_StartAutoTimer(0, 100);	/* 启动1个100ms的自动重装的定时器 */
	
	/* 首次使用，先设置64字节接收 */
	g_i2cLen = 69;
	bsp_i2cReceive();
	
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
		
        if (wTransferState != TRANSFER_WAIT)
        {
			/* 传输固件 */
			if(g_i2cRxBuf[0] == '*')
			{
				/* 获取文件大小 */
				filesize = g_i2cRxBuf[1] + (g_i2cRxBuf[2] << 8) + (g_i2cRxBuf[3] << 16) + (g_i2cRxBuf[4] << 24);
				uwAppSize = filesize;
				for(int i = 0; i < 69; i++)
				{
					printf("%x ", g_i2cRxBuf[i]);
				}
				
				/* 根据文件大小执行擦除 */
				SectorCount = filesize/(128*1024);
				SectorRemain = filesize%(128*1024);	
				
				printf("filesize = %d\r\n", filesize);
				for(i = 0; i < SectorCount; i++)
				{
					bsp_EraseCpuFlash((uint32_t)(AppAddr + i*128*1024));
				}
				
				if(SectorRemain)
				{
					bsp_EraseCpuFlash((uint32_t)(AppAddr + i*128*1024));
				}
				
				/* 返回0x30，表示擦除成功 */
				g_i2cLen = 1;
				g_i2cTxBuf[0] = 0x30;
				bsp_i2cTransfer();
				
				/* 继续执行下次接收 */
				g_i2cLen = 69;
				bsp_i2cReceive();
			}
			
			/* 传输完成命令 **************/
			if(g_i2cRxBuf[0]  == '#')
			{
				JumpToApp();
			}
	
			/* 开始传输固件命令 **************/
			if(g_i2cRxBuf[0]  == '$')
			{					   
				/* 接收数据个数 */
				RecSize = g_i2cRxBuf[1];
				
				/* 编程内部Flash, */
				ucState = bsp_WriteCpuFlash((uint32_t)(AppAddr + TotalSize),  (uint8_t *)&g_i2cRxBuf[2], RecSize);
				TotalSize += RecSize;
				printf("=====%d\r\n", TotalSize);
				
				/* 如果返回非0，表示编程失败 */
				if(ucState != 0)
				{
					/* 返回0x60，表示编程失败 */
					g_i2cLen = 1;
					g_i2cTxBuf[0] = 0x60;
					bsp_i2cTransfer();
				}
				
				/* 返回0x30，表示编程成功 */  
				g_i2cLen = 1;
				g_i2cTxBuf[0] = 0x30;
				bsp_i2cTransfer();
				
				/* 继续执行下次接收 */
				g_i2cLen = 69;
				bsp_i2cReceive();
			}
			
		}
	}
}

/*
*********************************************************************************************************
*	函 数 名: JumpToApp
*	功能说明: 跳转到应用JumpToApp
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void JumpToApp(void)
{
	uint32_t i=0;
	void (*AppJump)(void);         /* 声明一个函数指针 */
    
    /* 关闭全局中断 */
	DISABLE_INT(); 
    
    /* 设置所有时钟到默认状态，使用HSI时钟 */
	HAL_RCC_DeInit();
    
	/* 关闭滴答定时器，复位到默认值 */
	SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL = 0;

	/* 关闭所有中断，清除所有中断挂起标志 */
	for (i = 0; i < 8; i++)
	{
		NVIC->ICER[i]=0xFFFFFFFF;
		NVIC->ICPR[i]=0xFFFFFFFF;
	}	

	/* 使能全局中断 */
	ENABLE_INT();

	/* 跳转到应用程序，首地址是MSP，地址+4是复位中断服务程序地址 */
	AppJump = (void (*)(void)) (*((uint32_t *) (AppAddr + 4)));

	/* 设置主堆栈指针 */
	__set_MSP(*(uint32_t *)AppAddr);
	
	/* 在RTOS工程，这条语句很重要，设置为特权级模式，使用MSP指针 */
	__set_CONTROL(0);

	/* 跳转到系统BootLoader */
	AppJump(); 

	/* 跳转成功的话，不会执行到这里，用户可以在这里添加代码 */
	while (1)
	{

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
	printf("1. I2C主机发送字符1命令后，从机发送17个字符给主机 \r\n");
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
