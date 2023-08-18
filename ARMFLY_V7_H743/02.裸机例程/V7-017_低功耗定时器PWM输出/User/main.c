/*
*********************************************************************************************************
*
*	模块名称 : 主程序模块
*	文件名称 : main.c
*	版    本 : V1.0
*	说    明 : 低功耗定时器PWM输出
*              实验目的：
*                1. 学习低功耗定时器PWM输出。
*              实验内容：
*                1. 使用LPTIM的好处是系统处于睡眠，停机状态依然可以正常工作（除了待机模式）。停机状态可以
*                   正常工作的关键是LSE，LSI时钟不会被关闭，同时也可以选择使用外部时钟源。
*                2. 例子默认用的LSE时钟供LPTIM1使用，大家可以通过bsp_lptim_pwm.c文件开头宏定义切换到LSI或
*                   者PLCK。
*                3. PWM输出引脚采用的PD13，输出频谱1024Hz，占空比50%。
*              实验操作：
*                1. K1键按下，进入低功耗的停机模式，LED2停止闪烁。
*                2. K3键按下，退出停机模式，LED2继续闪烁。
*              注意事项：
*                1. 本实验推荐使用串口软件SecureCRT查看打印信息，波特率115200，数据位8，奇偶校验位无，停止位1。
*                2. 务必将编辑器的缩进参数和TAB设置为4来阅读本文件，要不代码显示不整齐。
*
*	修改记录 :
*		版本号   日期         作者        说明
*		V1.0    2018-12-12   Eric2013     1. CMSIS软包版本 V5.4.0
*                                         2. HAL库版本 V1.3.0
*
*	Copyright (C), 2018-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/	
#include "bsp.h"			/* 底层硬件驱动 */



/* 定义例程名和例程发布日期 */
#define EXAMPLE_NAME	"V7-低功耗定时器PWM输出"
#define EXAMPLE_DATE	"2018-12-12"
#define DEMO_VER		"1.0"

static void PrintfLogo(void);
static void PrintfHelp(void);
static void PwrExitStopMode(void);
extern void SystemClock_Config(void);

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
	

	bsp_Init();		   /* 硬件初始化 */
	PwrExitStopMode(); /* 配置K3按键用于唤醒停机模式 */
	
	PrintfLogo();	/* 打印例程名称和版本等信息 */
	PrintfHelp();	/* 打印操作提示 */

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

		/* 按键滤波和检测由后台systick中断服务程序实现，我们只需要调用bsp_GetKey读取键值即可。 */
		ucKeyCode = bsp_GetKey();	/* 读取键值, 无键按下时返回 KEY_NONE = 0 */
		if (ucKeyCode != KEY_NONE)
		{
			switch (ucKeyCode)
			{
				case KEY_DOWN_K1:			/* K1键按下，进入停机模式 */
					printf("--进入停机模式\r\n");
					HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
					SystemClock_Config();   /* 特别注意，退出后要重新配置系统时钟 */
					break;
				
				case KEY_DOWN_K3:			/* K3键按下，唤醒停机模式 */
					printf("--退出停机模式\r\n");
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
*	函 数 名: PwrExitStopMode
*	功能说明: 起初按键K3的GPIO配置为输入模式用于按键后，再次配置EXTI Event模式后，按键功能依然可以正常使用。
*             配置为EXTI Event是因为可以唤醒停机模式。
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void PwrExitStopMode(void)
{
	GPIO_InitTypeDef   		   GPIO_InitStruct = {0};
		
	GPIO_InitStruct.Pin = GPIO_PIN_4;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.Mode = GPIO_MODE_EVT_FALLING; /* 配置为下降沿触发Event */     
	HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);
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
	printf("1. K1键按下，进入低功耗的停机模式，LED2停止闪烁\r\n");
  	printf("2. K3键按下，退出停机模式，LED2继续闪烁\r\n");   
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
