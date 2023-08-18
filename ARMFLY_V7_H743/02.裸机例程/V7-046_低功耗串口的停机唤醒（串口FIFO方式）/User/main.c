/*
*********************************************************************************************************
*
*	模块名称 : 主程序模块
*	文件名称 : main.c
*	版    本 : V1.0
*	说    明 : 低功耗串口LPUART的停机唤醒。
*              实验目的：
*                1、学习低功耗串口的停机唤醒。
*              实验内容：
*                1、当前程序使用的串口打印就是用的低功耗串口，即USART1和LPUART1都可以使用PA9和PA10。
*                2、上电启动了一个软件定时器，每100ms翻转一次LED2。
*                3、USART1和LPUART都可以使用PA9和PA10引脚做串口打印功能，本例子是用的LPUART做开发板串口打印。
*                4、LPUART可以选择HSI时钟，LSE时钟和D3PCLK1时钟，在bsp_lpuart_fifo.c文件开头可以配置。
*                   如果需要低功耗模式唤醒，必须使用LSE或者HSI时钟，波特率在bsp_lpuart_fifo.h定义，本例子是用的HSI时钟。
*                   LPUART时钟选择LSE(32768Hz)，最高速度是10922bps，最低8bps。
*                   LPUART时钟选择HSI(64MHz)，最高值是21MHz，最小值15625bps。
*                   LPUART时钟选择D3PCLK1(100MHz)，最大值33Mbps，最小值24414bps。
*              实验操作：
*                1、K1键按下，进入停机模式，低功耗串口接收任意字节数据可以唤醒。
*                2、K2键按下，进入停机模式，低功耗串口检测到起始位可以唤醒。
*                3、K3键按下，进入停机模式，低功耗串口检测到地址0x99可以唤醒。
*              注意事项：
*                1. 本实验推荐使用串口软件SecureCRT查看打印信息，波特率115200，数据位8，奇偶校验位无，停止位1。
*                2. 务必将编辑器的缩进参数和TAB设置为4来阅读本文件，要不代码显示不整齐。
*
*	修改记录 :
*		版本号   日期         作者        说明
*		V1.0    2020-02-11   Eric2013     1. CMSIS软包版本 V5.6.0
*                                         2. HAL库版本 V1.7.0
*
*	Copyright (C), 2020-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/	
#include "bsp.h"			 /* 底层硬件驱动 */



/* 定义例程名和例程发布日期 */
#define EXAMPLE_NAME	"V7-低功耗串口的停机唤醒（串口FIFO方式）"
#define EXAMPLE_DATE	"2020-02-11"
#define DEMO_VER		"1.0"

static void PrintfLogo(void);
static void PrintfHelp(void);
UART_WakeUpTypeDef WakeUpSelection = {0};

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
	uint8_t ucKeyCode;	/* 按键代码 */
	uint8_t ucReceive;
	

	bsp_Init();		/* 硬件初始化 */
	PrintfLogo();	/* 打印例程名称和版本等信息 */
	PrintfHelp();	/* 打印操作提示 */
	
	HAL_EnableDBGStopMode(); /* 使能停机模式下，LPUART工程可以继续调试 */
	__HAL_RCC_WAKEUPSTOP_CLK_CONFIG(RCC_STOP_WAKEUPCLOCK_HSI); /* 从停机模式唤醒后使用HSI时钟 */
	__HAL_RCC_LPUART1_CLKAM_ENABLE();     /* 激活LPUART的自主模式，即停机状态下可以继续接收消息 */
	__HAL_UART_ENABLE_IT(&UartHandle, UART_IT_WUF);/* 使能唤醒中断 */
	
	
	bsp_StartAutoTimer(0, 100);	/* 启动1个100ms的自动重装的定时器 */
	
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
				case KEY_DOWN_K1:			/* K1键按下，进入停机模式，低功耗串口接收任意字节数据可以唤醒 */
					/* 使能LPUART的停机唤醒 */
					HAL_UARTEx_EnableStopMode(&UartHandle); 

					/* 确保LPUART没有在通信中 */
					while(__HAL_UART_GET_FLAG(&UartHandle, USART_ISR_BUSY) == SET){}
					while(__HAL_UART_GET_FLAG(&UartHandle, USART_ISR_REACK) == RESET){}

					/* 接收到数据唤醒，即RXNE标志置位 */
					WakeUpSelection.WakeUpEvent = UART_WAKEUP_ON_READDATA_NONEMPTY;
					if (HAL_UARTEx_StopModeWakeUpSourceConfig(&UartHandle, WakeUpSelection)!= HAL_OK)
					{
						Error_Handler(__FILE__, __LINE__);						
					}

					/* 进入停机模式 */
					HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);

					/* 退出停机模式要重新配置HSE和PLL*/
					SystemClock_Config();

					/* 关闭LPUART的停机唤醒 */
					HAL_UARTEx_DisableStopMode(&UartHandle);
					
					lpcomGetChar(LPCOM1, &ucReceive);
					
					printf("低功耗串口接收到数据 %x 后唤醒\r\n", ucReceive);
					break;
					
				case KEY_DOWN_K2:			/* K2键按下，进入停机模式，低功耗串口检测到起始位可以唤醒 */
					/* 使能LPUART的停机唤醒 */
					HAL_UARTEx_EnableStopMode(&UartHandle); 

					/* 确保LPUART没有在通信中 */
					while(__HAL_UART_GET_FLAG(&UartHandle, USART_ISR_BUSY) == SET){}
					while(__HAL_UART_GET_FLAG(&UartHandle, USART_ISR_REACK) == RESET){}

					/* 接收起始位唤醒 */
					WakeUpSelection.WakeUpEvent = UART_WAKEUP_ON_STARTBIT;
					if (HAL_UARTEx_StopModeWakeUpSourceConfig(&UartHandle, WakeUpSelection)!= HAL_OK)
					{
						Error_Handler(__FILE__, __LINE__);						
					}

					/* 进入停机模式 */
					HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);

					/* 退出停机模式要重新配置HSE和PLL*/
					SystemClock_Config();

					/* 关闭LPUART的停机唤醒 */
					HAL_UARTEx_DisableStopMode(&UartHandle);
					
					lpcomGetChar(LPCOM1, &ucReceive);
					
					printf("低功耗串口检测到起始位（数据） %x 后唤醒\r\n", ucReceive);
					break;
					
				case KEY_DOWN_K3:			/* K3键按下，进入停机模式，低功耗串口检测到地址0x99可以唤醒 */
					/* 使能LPUART的停机唤醒 */
					HAL_UARTEx_EnableStopMode(&UartHandle); 

					/* 确保LPUART没有在通信中 */
					while(__HAL_UART_GET_FLAG(&UartHandle, USART_ISR_BUSY) == SET){}
					while(__HAL_UART_GET_FLAG(&UartHandle, USART_ISR_REACK) == RESET){}

					/* 接收地址0x99（发送的数据MSB位要为1），可以唤醒 */
					WakeUpSelection.WakeUpEvent   = UART_WAKEUP_ON_ADDRESS;
					WakeUpSelection.AddressLength = UART_ADDRESS_DETECT_7B;
					WakeUpSelection.Address       = 0x19;
					if (HAL_UARTEx_StopModeWakeUpSourceConfig(&UartHandle, WakeUpSelection)!= HAL_OK)
					{
						Error_Handler(__FILE__, __LINE__);						
					}
					
					CLEAR_BIT(LPUART1->CR1, USART_CR1_RXNEIE); /* 关闭串口接收中断 */
					
					/* 进入停机模式 */
					HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);

					/* 退出停机模式要重新配置HSE和PLL*/
					SystemClock_Config();

					SET_BIT(LPUART1->CR1, USART_CR1_RXNEIE);  /* 使能串口接收中断 */
					
					/* 关闭LPUART的停机唤醒 */
					HAL_UARTEx_DisableStopMode(&UartHandle);
					
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
	printf("1. 上电启动了一个软件定时器，每100ms翻转一次LED2\r\n");
	printf("2. K1键按下，进入停机模式，低功耗串口接收任意字节数据可以唤醒\r\n");
	printf("3. K2键按下，进入停机模式，低功耗串口检测到起始位可以唤醒\r\n");
	printf("4. K3键按下，进入停机模式，低功耗串口检测到地址0x99可以唤醒\r\n");
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
	printf("* HAL库版本  : V1.7.0 (STM32H7xx HAL Driver)\r\n");
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
