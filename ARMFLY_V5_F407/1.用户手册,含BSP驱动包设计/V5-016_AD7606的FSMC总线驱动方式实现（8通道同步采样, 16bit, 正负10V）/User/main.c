/*
*********************************************************************************************************
*
*	模块名称 : 主程序模块
*	文件名称 : main.c
*	版    本 : V1.0
*	说    明 : AD7606的FMC驱动方式实现。
*              AD7606 的配置很简单，它没有内部寄存器，量程范围和过采样参数是通过外部IO控制的。
*              采样速率由MCU或DSP提供的脉冲频率控制。
*              实验目的：
*                1. 学习AD7606的FMC驱动方式实现。
*              重要提示：
*                1、板子上电后，默认是软件定时采集，0.5秒一次，适合串口展示数据。
*                2、如果需要使用J-Scope实时展示采集的波形效果，需要按下K2按键切换到FIFO模式。
*                3、如果使用的JLINK速度不够快，导致J-Scope无法最高速度实时上传，可以使用摇杆上下键设置过采样来降低上传速度。
*                4、默认情况下，程序仅上传了AD7606通道1采集的数据。
*                5、串口数据展示推荐使用SecureCRT，因为数据展示做了特别处理，方便采集数据在串口软件
*                   同一个位置不断刷新。
*              实验内容：
*                1、AD7606的FMC驱动做了两种采集方式
*                （1）软件定时获取方式，适合低速查询获取。
*                （2）FIFO工作模式，适合8路实时采集，支持最高采样率200Ksps。
*                2、数据展示方式：
*                （1）软件查询方式，数据通过串口打印输出。
*                （2）FIFO工作模式，数据通过J-Scope实时输出。
*                （3）J-Scope的实时输出方法请看V5板子用户手册对应的AD7606章节。
*                3、将模拟输入接地时，采样值是0左右。
*                4、模拟输入端悬空时，采样值在某个范围浮动（这是正常的，这是AD7606内部输入电阻导致的浮动电压）。
*                5、出厂的AD7606模块缺省是8080 并行接口。如果用SPI接口模式，需要修改 R1 R2电阻配置。
*                6、配置CVA CVB 引脚为PWM输出模式，周期设置为需要的采样频率，之后MCU将产生周期非常稳定的AD转换信号。
*              实验操作：
*                1. 启动一个自动重装软件定时器，每100ms翻转一次LED2。
*                2. K1键       : 切换量程(5V或10V)。
*                3. K2键       : 进入FIFO工作模式。
*                4. K3键       : 进入软件定时采集模式。
*                5. 摇杆上下键 : 调节过采样参数。
*              注意事项：
*                1. 本实验推荐使用串口软件SecureCRT查看打印信息，波特率115200，数据位8，奇偶校验位无，停止位1。
*                2. 务必将编辑器的缩进参数和TAB设置为4来阅读本文件，要不代码显示不整齐。
*
*	修改记录 :
*		版本号   日期         作者        说明
*		V1.0    2020-05-01   Eric2013     1. CMSIS软包版本 V5.6.0
*                                         2. HAL库版本 V1.24.0
*
*	Copyright (C), 2020-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/
#include "bsp.h"			/* 底层硬件驱动 */



/* 定义例程名和例程发布日期 */
#define EXAMPLE_NAME	"V5-AD7606的FMC总线驱动方式实现（8通道同步采样, 16bit, 正负10V）"
#define EXAMPLE_DATE	"2020-05-01"
#define DEMO_VER		"1.0"

static void PrintfLogo(void);
extern void DemoFmcAD7606(void);

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
	bsp_Init();		/* 硬件初始化 */
	
	PrintfLogo();	/* 打印例程名称和版本等信息 */

	DemoFmcAD7606(); /* AD7606测试 */
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
