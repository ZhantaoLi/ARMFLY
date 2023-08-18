/*
*********************************************************************************************************
*
*	模块名称 : 低功耗串口中断+FIFO驱动模块
*	文件名称 : bsp_uart_fifo.c
*	版    本 : V1.0
*	说    明 : 采用串口中断+FIFO模式实现多个串口的同时访问
*	修改记录 :
*		版本号  日期       作者       说明
*		V1.0    2020-02-11 Eric2013  正式发布
*
*	Copyright (C), 2020-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/
#include "bsp.h"



/* 低功耗串口使用的GPIO  PA9, PA10*/
#define LPUART1_CLK_ENABLE()              __HAL_RCC_LPUART1_CLK_ENABLE()

#define LPUART1_TX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOA_CLK_ENABLE()
#define LPUART1_TX_GPIO_PORT              GPIOA
#define LPUART1_TX_PIN                    GPIO_PIN_9
#define LPUART1_TX_AF                     GPIO_AF3_LPUART

#define LPUART1_RX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOA_CLK_ENABLE()
#define LPUART1_RX_GPIO_PORT              GPIOA
#define LPUART1_RX_PIN                    GPIO_PIN_10
#define LPUART1_RX_AF                     GPIO_AF3_LPUART

/* 选择LPUART的时钟源，如果需要低功耗模式唤醒，必须使用LSE或者HSI时钟，波特率在bsp_lpuart_fifo.h定义 */
//#define LPUART_CLOCK_SOURCE_LSE    /* LPUART时钟选择LSE(32768Hz)，最高速度是10922bps，最低8bps */	
#define LPUART_CLOCK_SOURCE_HSI    /* LPUART时钟选择HSI(64MHz)，最高值是21MHz，最小值15625bps */	
//#define LPUART_CLOCK_SOURCE_D3PCLK1  /* LPUART时钟选择D3PCLK1(100MHz)，最大值33Mbps，最小值24414bps */	

/* 定义每个串口结构体变量 */
#if LPUART1_FIFO_EN == 1
	static LPUART_T g_tLPUart1;
	static uint8_t g_TxBuf1[LPUART1_TX_BUF_SIZE];		/* 发送缓冲区 */
	static uint8_t g_RxBuf1[LPUART1_RX_BUF_SIZE];		/* 接收缓冲区 */
#endif

UART_HandleTypeDef UartHandle = {0};
		
static void LPUartVarInit(void);
static void InitHardLPUart(void);
static void LPUartSend(LPUART_T *_pUart, uint8_t *_ucaBuf, uint16_t _usLen);
static uint8_t LPUartGetChar(LPUART_T *_pUart, uint8_t *_pByte);
static void LPUartIRQ(LPUART_T *_pUart);
void bsp_SetLPUartParam(USART_TypeDef *Instance,  uint32_t BaudRate, uint32_t Parity, uint32_t Mode);

/*
*********************************************************************************************************
*	函 数 名: bsp_InitLPUart
*	功能说明: 初始化串口硬件，并对全局变量赋初值.
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_InitLPUart(void)
{
	LPUartVarInit();		/* 必须先初始化全局变量,再配置硬件 */
	InitHardLPUart();		/* 配置串口的硬件参数(波特率等) */
}

/*
*********************************************************************************************************
*	函 数 名: ComToLPUart
*	功能说明: 将COM端口号转换为LPUART指针
*	形    参: _ucPort: 端口号(LPCOM1)
*	返 回 值: uart指针
*********************************************************************************************************
*/
LPUART_T *ComToLPUart(LPCOM_PORT_E _ucPort)
{
	if (_ucPort == LPCOM1)
	{
		#if LPUART1_FIFO_EN == 1
			return &g_tLPUart1;
		#else
			return 0;
		#endif
	}
	else
	{
		Error_Handler(__FILE__, __LINE__);
		return 0;
	}
}

/*
*********************************************************************************************************
*	函 数 名: ComToLPUARTx
*	功能说明: 将COM端口号转换为 USART_TypeDef* USARTx
*	形    参: _ucPort: 端口号(COM1 - COM8)
*	返 回 值: USART_TypeDef*,  LPUSART1。
*********************************************************************************************************
*/
USART_TypeDef *ComToLPUARTx(LPCOM_PORT_E _ucPort)
{
	if (_ucPort == COM1)
	{
		#if LPUART1_FIFO_EN == 1
			return LPUART1;
		#else
			return 0;
		#endif
	}
	else
	{
		/* 不做任何处理 */
		return 0;
	}
}

/*
*********************************************************************************************************
*	函 数 名: lpcomSendBuf
*	功能说明: 向串口发送一组数据。数据放到发送缓冲区后立即返回，由中断服务程序在后台完成发送
*	形    参: _ucPort: 端口号(LPCOM1)
*			  _ucaBuf: 待发送的数据缓冲区
*			  _usLen : 数据长度
*	返 回 值: 无
*********************************************************************************************************
*/
void lpcomSendBuf(LPCOM_PORT_E _ucPort, uint8_t *_ucaBuf, uint16_t _usLen)
{
	LPUART_T *pUart;

	pUart = ComToLPUart(_ucPort);
	if (pUart == 0)
	{
		return;
	}

	if (pUart->SendBefor != 0)
	{
		pUart->SendBefor();		/* 如果是RS485通信，可以在这个函数中将RS485设置为发送模式 */
	}

	LPUartSend(pUart, _ucaBuf, _usLen);
}

/*
*********************************************************************************************************
*	函 数 名: lpcomSendChar
*	功能说明: 向串口发送1个字节。数据放到发送缓冲区后立即返回，由中断服务程序在后台完成发送
*	形    参: _ucPort: 端口号(LPCOM1)
*			  _ucByte: 待发送的数据
*	返 回 值: 无
*********************************************************************************************************
*/
void lpcomSendChar(LPCOM_PORT_E _ucPort, uint8_t _ucByte)
{
	lpcomSendBuf(_ucPort, &_ucByte, 1);
}

/*
*********************************************************************************************************
*	函 数 名: lpcomGetChar
*	功能说明: 从接收缓冲区读取1字节，非阻塞。无论有无数据均立即返回。
*	形    参: _ucPort: 端口号(LPCOM1)
*			  _pByte: 接收到的数据存放在这个地址
*	返 回 值: 0 表示无数据, 1 表示读取到有效字节
*********************************************************************************************************
*/
uint8_t lpcomGetChar(LPCOM_PORT_E _ucPort, uint8_t *_pByte)
{
	LPUART_T *pUart;

	pUart = ComToLPUart(_ucPort);
	if (pUart == 0)
	{
		return 0;
	}

	return LPUartGetChar(pUart, _pByte);
}

/*
*********************************************************************************************************
*	函 数 名: lpcomClearTxFifo
*	功能说明: 清零串口发送缓冲区
*	形    参: _ucPort: 端口号(LPCOM1)
*	返 回 值: 无
*********************************************************************************************************
*/
void lpcomClearTxFifo(LPCOM_PORT_E _ucPort)
{
	LPUART_T *pUart;

	pUart = ComToLPUart(_ucPort);
	if (pUart == 0)
	{
		return;
	}

	pUart->usTxWrite = 0;
	pUart->usTxRead = 0;
	pUart->usTxCount = 0;
}

/*
*********************************************************************************************************
*	函 数 名: lpcomClearRxFifo
*	功能说明: 清零串口接收缓冲区
*	形    参: _ucPort: 端口号(LPCOM1)
*	返 回 值: 无
*********************************************************************************************************
*/
void lpcomClearRxFifo(LPCOM_PORT_E _ucPort)
{
	LPUART_T *pUart;

	pUart = ComToLPUart(_ucPort);
	if (pUart == 0)
	{
		return;
	}

	pUart->usRxWrite = 0;
	pUart->usRxRead = 0;
	pUart->usRxCount = 0;
}

/*
*********************************************************************************************************
*	函 数 名: lpcomSetBaud
*	功能说明: 设置串口的波特率. 本函数固定设置为无校验，收发都使能模式
*	形    参: _ucPort: 端口号(LPCOM1)
*			  _BaudRate: 波特率，8倍过采样  波特率.0-12.5Mbps
*                                16倍过采样 波特率.0-6.25Mbps
*	返 回 值: 无
*********************************************************************************************************
*/
void lpcomSetBaud(LPCOM_PORT_E _ucPort, uint32_t _BaudRate)
{
	USART_TypeDef* USARTx;
	
	USARTx = ComToLPUARTx(_ucPort);
	if (USARTx == 0)
	{
		return;
	}
	
	bsp_SetLPUartParam(USARTx,  _BaudRate, UART_PARITY_NONE, UART_MODE_TX_RX);
}

/* 如果是RS485通信，请按如下格式编写函数 */

#if 0
/*
*********************************************************************************************************
*	函 数 名: RS485_InitTXE
*	功能说明: 配置RS485发送使能口线 TXE
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void RS485_InitTXE(void)
{
	GPIO_InitTypeDef gpio_init;
	
	/* 打开GPIO时钟 */
	RS485_TXEN_GPIO_CLK_ENABLE();
	
	/* 配置引脚为推挽输出 */
	gpio_init.Mode = GPIO_MODE_OUTPUT_PP;			/* 推挽输出 */
	gpio_init.Pull = GPIO_NOPULL;					/* 上下拉电阻不使能 */
	gpio_init.Speed = GPIO_SPEED_FREQ_VERY_HIGH;	/* GPIO速度等级 */
	gpio_init.Pin = RS485_TXEN_PIN;
	HAL_GPIO_Init(RS485_TXEN_GPIO_PORT, &gpio_init);
}

/*
*********************************************************************************************************
*	函 数 名: RS485_SetBaud
*	功能说明: 修改485串口的波特率。
*	形    参: _baud : 8倍过采样  波特率.0-12.5Mbps
*                     16倍过采样 波特率.0-6.25Mbps
*	返 回 值: 无
*********************************************************************************************************
*/
void RS485_SetBaud(uint32_t _baud)
{
	comSetBaud(COM3, _baud);
}

/*
*********************************************************************************************************
*	函 数 名: RS485_SendBefor
*	功能说明: 发送数据前的准备工作。对于RS485通信，请设置RS485芯片为发送状态，
*			  并修改 UartVarInit()中的函数指针等于本函数名，比如 g_tUart2.SendBefor = RS485_SendBefor
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void RS485_SendBefor(void)
{
	RS485_TX_EN();	/* 切换RS485收发芯片为发送模式 */
}

/*
*********************************************************************************************************
*	函 数 名: RS485_SendOver
*	功能说明: 发送一串数据结束后的善后处理。对于RS485通信，请设置RS485芯片为接收状态，
*			  并修改 UartVarInit()中的函数指针等于本函数名，比如 g_tUart2.SendOver = RS485_SendOver
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void RS485_SendOver(void)
{
	RS485_RX_EN();	/* 切换RS485收发芯片为接收模式 */
}

/*
*********************************************************************************************************
*	函 数 名: RS485_SendBuf
*	功能说明: 通过RS485芯片发送一串数据。注意，本函数不等待发送完毕。
*	形    参: _ucaBuf : 数据缓冲区
*			  _usLen : 数据长度
*	返 回 值: 无
*********************************************************************************************************
*/
void RS485_SendBuf(uint8_t *_ucaBuf, uint16_t _usLen)
{
	comSendBuf(COM3, _ucaBuf, _usLen);
}

/*
*********************************************************************************************************
*	函 数 名: RS485_SendStr
*	功能说明: 向485总线发送一个字符串，0结束。
*	形    参: _pBuf 字符串，0结束
*	返 回 值: 无
*********************************************************************************************************
*/
void RS485_SendStr(char *_pBuf)
{
	RS485_SendBuf((uint8_t *)_pBuf, strlen(_pBuf));
}

/*
*********************************************************************************************************
*	函 数 名: RS485_ReciveNew
*	功能说明: 接收到新的数据
*	形    参: _byte 接收到的新数据
*	返 回 值: 无
*********************************************************************************************************
*/
//extern void MODH_ReciveNew(uint8_t _byte);
void RS485_ReciveNew(uint8_t _byte)
{
//	MODH_ReciveNew(_byte);
}
#endif	

/*
*********************************************************************************************************
*	函 数 名: LPUartVarInit
*	功能说明: 初始化串口相关的变量
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void LPUartVarInit(void)
{
#if LPUART1_FIFO_EN == 1
	g_tLPUart1.uart = LPUART1;						/* STM32 串口设备 */
	g_tLPUart1.pTxBuf = g_TxBuf1;					/* 发送缓冲区指针 */
	g_tLPUart1.pRxBuf = g_RxBuf1;					/* 接收缓冲区指针 */
	g_tLPUart1.usTxBufSize = LPUART1_TX_BUF_SIZE;	/* 发送缓冲区大小 */
	g_tLPUart1.usRxBufSize = LPUART1_RX_BUF_SIZE;	/* 接收缓冲区大小 */
	g_tLPUart1.usTxWrite = 0;						/* 发送FIFO写索引 */
	g_tLPUart1.usTxRead = 0;						/* 发送FIFO读索引 */
	g_tLPUart1.usRxWrite = 0;						/* 接收FIFO写索引 */
	g_tLPUart1.usRxRead = 0;						/* 接收FIFO读索引 */
	g_tLPUart1.usRxCount = 0;						/* 接收到的新数据个数 */
	g_tLPUart1.usTxCount = 0;						/* 待发送的数据个数 */
	g_tLPUart1.SendBefor = 0;						/* 发送数据前的回调函数 */
	g_tLPUart1.SendOver = 0;						/* 发送完毕后的回调函数 */
	g_tLPUart1.ReciveNew = 0;						/* 接收到新数据后的回调函数 */
	g_tLPUart1.Sending = 0;						/* 正在发送中标志 */
#endif
}

/*
*********************************************************************************************************
*	函 数 名: bsp_SetUartParam
*	功能说明: 配置串口的硬件参数（波特率，数据位，停止位，起始位，校验位，中断使能）适合于STM32- H7开发板
*	形    参: Instance   USART_TypeDef类型结构体
*             BaudRate   波特率
*             Parity     校验类型，奇校验或者偶校验
*             Mode       发送和接收模式使能
*	返 回 值: 无
*********************************************************************************************************
*/	
void bsp_SetLPUartParam(USART_TypeDef *Instance,  uint32_t BaudRate, uint32_t Parity, uint32_t Mode)
{
	/*##-1- 配置串口硬件参数 ######################################*/
	/* 异步串口模式 (UART Mode) */
	/* 配置如下:
	  - 字长    = 8 位
	  - 停止位  = 1 个停止位
	  - 校验    = 参数Parity
	  - 波特率  = 参数BaudRate
	  - 硬件流控制关闭 (RTS and CTS signals) */

	UartHandle.Instance        = LPUART1;
	UartHandle.Init.BaudRate   = BaudRate;
	UartHandle.Init.WordLength = UART_WORDLENGTH_8B;
	UartHandle.Init.StopBits   = UART_STOPBITS_1;
	UartHandle.Init.Parity     = Parity;
	UartHandle.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
	UartHandle.Init.Mode       = Mode;
	UartHandle.Init.OverSampling = UART_OVERSAMPLING_16;
	UartHandle.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	UartHandle.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    
	if (HAL_UART_Init(&UartHandle) != HAL_OK)
	{
		Error_Handler(__FILE__, __LINE__);
	}
}

/*
*********************************************************************************************************
*	函 数 名: InitHardLPUart
*	功能说明: 配置串口的硬件参数（波特率，数据位，停止位，起始位，校验位，中断使能）适合于STM32-H7开发板
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void InitHardLPUart(void)
{
	GPIO_InitTypeDef  GPIO_InitStruct;
	RCC_PeriphCLKInitTypeDef   RCC_PeriphCLKInitStruct = {0};

/* 使用LSE(32768Hz)，最高速度是10922bps，最低8bps */	
#if defined (LPUART_CLOCK_SOURCE_LSE)
	{
		RCC_OscInitTypeDef RCC_OscInitStruct = {0};

		RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE;
		RCC_OscInitStruct.LSEState = RCC_LSE_ON;
		RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;

		if (HAL_RCC_OscConfig(&RCC_OscInitStruct)!= HAL_OK)
		{
			Error_Handler(__FILE__, __LINE__);		
		}
		
		RCC_PeriphCLKInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LPUART1;
		RCC_PeriphCLKInitStruct.Lpuart1ClockSelection = RCC_LPUART1CLKSOURCE_LSE;
		HAL_RCCEx_PeriphCLKConfig(&RCC_PeriphCLKInitStruct);
	}	
/* LPUART时钟选择HSI(64MHz)，最高值是21MHz，最小值15625bps */	
#elif defined (LPUART_CLOCK_SOURCE_HSI)
	{

		RCC_OscInitTypeDef RCC_OscInitStruct = {0};

		  RCC_OscInitStruct.OscillatorType      = RCC_OSCILLATORTYPE_HSI;
		  RCC_OscInitStruct.HSIState            = RCC_HSI_ON;
		  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
		  RCC_OscInitStruct.PLL.PLLState        = RCC_PLL_NONE;

		if (HAL_RCC_OscConfig(&RCC_OscInitStruct)!= HAL_OK)
		{
			Error_Handler(__FILE__, __LINE__);		
		}
		
		RCC_PeriphCLKInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LPUART1;
		RCC_PeriphCLKInitStruct.Lpuart1ClockSelection = RCC_LPUART1CLKSOURCE_HSI;
		HAL_RCCEx_PeriphCLKConfig(&RCC_PeriphCLKInitStruct);
	}
/* LPUART时钟选择D3PCLK1（100MHz），最大值33Mbps，最小值24414bps */	
#elif defined (LPUART_CLOCK_SOURCE_D3PCLK1)
	
	RCC_PeriphCLKInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LPUART1;
	RCC_PeriphCLKInitStruct.Lptim1ClockSelection = RCC_LPUART1CLKSOURCE_D3PCLK1;
	HAL_RCCEx_PeriphCLKConfig(&RCC_PeriphCLKInitStruct);
#else
	#error Please select the LPTIM Clock source inside the bsp_lpuart_fifo.c file
#endif

#if LPUART1_FIFO_EN == 1		/* 串口1 */
	/* 使能 GPIO TX/RX 时钟 */
	LPUART1_TX_GPIO_CLK_ENABLE();
	LPUART1_RX_GPIO_CLK_ENABLE();
	
	/* 使能 USARTx 时钟 */
	LPUART1_CLK_ENABLE();	

	/* 配置TX引脚 */
	GPIO_InitStruct.Pin       = LPUART1_TX_PIN;
	GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull      = GPIO_PULLUP;
	GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.Alternate = LPUART1_TX_AF;
	HAL_GPIO_Init(LPUART1_TX_GPIO_PORT, &GPIO_InitStruct);	
	
	/* 配置RX引脚 */
	GPIO_InitStruct.Pin = LPUART1_RX_PIN;
	GPIO_InitStruct.Alternate = LPUART1_RX_AF;
	HAL_GPIO_Init(LPUART1_RX_GPIO_PORT, &GPIO_InitStruct);

	/* 配置NVIC the NVIC for UART */   
	HAL_NVIC_SetPriority(LPUART1_IRQn, 0, 1);
	HAL_NVIC_EnableIRQ(LPUART1_IRQn);
  
	/* 配 置波特率、奇偶校验 */
	bsp_SetLPUartParam(LPUART1,  LPUART1_BAUD, UART_PARITY_NONE, UART_MODE_TX_RX);

	SET_BIT(LPUART1->ICR, USART_ICR_TCCF);	 /* 清除TC发送完成标志 */
	SET_BIT(LPUART1->RQR, USART_RQR_RXFRQ);  /* 清除RXNE接收标志 */
	// USART_CR1_PEIE | USART_CR1_RXNEIE
	SET_BIT(LPUART1->CR1, USART_CR1_RXNEIE); /* 使能PE. RX接受中断 */
#endif
}

/*
*********************************************************************************************************
*	函 数 名: LPUartSend
*	功能说明: 填写数据到UART发送缓冲区,并启动发送中断。中断处理函数发送完毕后，自动关闭发送中断
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void LPUartSend(LPUART_T *_pUart, uint8_t *_ucaBuf, uint16_t _usLen)
{
	uint16_t i;

	for (i = 0; i < _usLen; i++)
	{
		/* 如果发送缓冲区已经满了，则等待缓冲区空 */
		while (1)
		{
			__IO uint16_t usCount;

			DISABLE_INT();
			usCount = _pUart->usTxCount;
			ENABLE_INT();

			if (usCount < _pUart->usTxBufSize)
			{
				break;
			}
			else if(usCount == _pUart->usTxBufSize)/* 数据已填满缓冲区 */
			{
				if((_pUart->uart->CR1 & USART_CR1_TXEIE) == 0)
				{
					SET_BIT(_pUart->uart->CR1, USART_CR1_TXEIE);
				}  
			}
		}

		/* 将新数据填入发送缓冲区 */
		_pUart->pTxBuf[_pUart->usTxWrite] = _ucaBuf[i];

		DISABLE_INT();
		if (++_pUart->usTxWrite >= _pUart->usTxBufSize)
		{
			_pUart->usTxWrite = 0;
		}
		_pUart->usTxCount++;
		ENABLE_INT();
	}

	SET_BIT(_pUart->uart->CR1, USART_CR1_TXEIE);	/* 使能发送中断（缓冲区空） */
}

/*
*********************************************************************************************************
*	函 数 名: LPUartGetChar
*	功能说明: 从串口接收缓冲区读取1字节数据 （用于主程序调用）
*	形    参: _pUart : 串口设备
*			  _pByte : 存放读取数据的指针
*	返 回 值: 0 表示无数据  1表示读取到数据
*********************************************************************************************************
*/
static uint8_t LPUartGetChar(LPUART_T *_pUart, uint8_t *_pByte)
{
	uint16_t usCount;

	/* usRxWrite 变量在中断函数中被改写，主程序读取该变量时，必须进行临界区保护 */
	DISABLE_INT();
	usCount = _pUart->usRxCount;
	ENABLE_INT();

	/* 如果读和写索引相同，则返回0 */
	//if (_pUart->usRxRead == usRxWrite)
	if (usCount == 0)	/* 已经没有数据 */
	{
		return 0;
	}
	else
	{
		*_pByte = _pUart->pRxBuf[_pUart->usRxRead];		/* 从串口接收FIFO取1个数据 */

		/* 改写FIFO读索引 */
		DISABLE_INT();
		if (++_pUart->usRxRead >= _pUart->usRxBufSize)
		{
			_pUart->usRxRead = 0;
		}
		_pUart->usRxCount--;
		ENABLE_INT();
		return 1;
	}
}

/*
*********************************************************************************************************
*   函 数 名: LPUartTxEmpty
*   功能说明: 判断发送缓冲区是否为空。
*   形    参:  _pUart : 串口设备
*   返 回 值: 1为空。0为不空。
*********************************************************************************************************
*/
uint8_t LPUartTxEmpty(LPCOM_PORT_E _ucPort)
{
   LPUART_T *pUart;
   uint8_t Sending;
   
   pUart = ComToLPUart(_ucPort);
   if (pUart == 0)
   {
      return 0;
   }

   Sending = pUart->Sending;

   if (Sending != 0)
   {
      return 0;
   }
   return 1;
}

/*
*********************************************************************************************************
*	函 数 名: LPUartIRQ
*	功能说明: 供中断服务程序调用，通用串口中断处理函数
*	形    参: _pUart : 串口设备
*	返 回 值: 无
*********************************************************************************************************
*/
static void LPUartIRQ(LPUART_T *_pUart)
{
	uint32_t isrflags   = READ_REG(_pUart->uart->ISR);
	uint32_t cr1its     = READ_REG(_pUart->uart->CR1);
	uint32_t cr3its     = READ_REG(_pUart->uart->CR3);
	
	/* 处理接收中断  */
	if ((isrflags & USART_ISR_RXNE_RXFNE) != RESET)
	{
		/* 从串口接收数据寄存器读取数据存放到接收FIFO */
		uint8_t ch;

		ch = READ_REG(_pUart->uart->RDR);
		_pUart->pRxBuf[_pUart->usRxWrite] = ch;
		if (++_pUart->usRxWrite >= _pUart->usRxBufSize)
		{
			_pUart->usRxWrite = 0;
		}
		if (_pUart->usRxCount < _pUart->usRxBufSize)
		{
			_pUart->usRxCount++;
		}

		/* 回调函数,通知应用程序收到新数据,一般是发送1个消息或者设置一个标记 */
		//if (_pUart->usRxWrite == _pUart->usRxRead)
		//if (_pUart->usRxCount == 1)
		{
			if (_pUart->ReciveNew)
			{
				_pUart->ReciveNew(ch); /* 比如，交给MODBUS解码程序处理字节流 */
			}
		}
	}

	/* 处理发送缓冲区空中断 */
	if ( ((isrflags & USART_ISR_TXE_TXFNF) != RESET) && (cr1its & USART_CR1_TXEIE) != RESET)
	{
		//if (_pUart->usTxRead == _pUart->usTxWrite)
		if (_pUart->usTxCount == 0)
		{
			/* 发送缓冲区的数据已取完时， 禁止发送缓冲区空中断 （注意：此时最后1个数据还未真正发送完毕）*/
			//USART_ITConfig(_pUart->uart, USART_IT_TXE, DISABLE);
			CLEAR_BIT(_pUart->uart->CR1, USART_CR1_TXEIE);

			/* 使能数据发送完毕中断 */
			//USART_ITConfig(_pUart->uart, USART_IT_TC, ENABLE);
			SET_BIT(_pUart->uart->CR1, USART_CR1_TCIE);
		}
		else
		{
			_pUart->Sending = 1;
			
			/* 从发送FIFO取1个字节写入串口发送数据寄存器 */
			//USART_SendData(_pUart->uart, _pUart->pTxBuf[_pUart->usTxRead]);
			_pUart->uart->TDR = _pUart->pTxBuf[_pUart->usTxRead];
			if (++_pUart->usTxRead >= _pUart->usTxBufSize)
			{
				_pUart->usTxRead = 0;
			}
			_pUart->usTxCount--;
		}

	}
	/* 数据bit位全部发送完毕的中断 */
	if (((isrflags & USART_ISR_TC) != RESET) && ((cr1its & USART_CR1_TCIE) != RESET))
	{
		//if (_pUart->usTxRead == _pUart->usTxWrite)
		if (_pUart->usTxCount == 0)
		{
			/* 如果发送FIFO的数据全部发送完毕，禁止数据发送完毕中断 */
			//USART_ITConfig(_pUart->uart, USART_IT_TC, DISABLE);
			CLEAR_BIT(_pUart->uart->CR1, USART_CR1_TCIE);

			/* 回调函数, 一般用来处理RS485通信，将RS485芯片设置为接收模式，避免抢占总线 */
			if (_pUart->SendOver)
			{
				_pUart->SendOver();
			}
			
			_pUart->Sending = 0;
		}
		else
		{
			/* 正常情况下，不会进入此分支 */

			/* 如果发送FIFO的数据还未完毕，则从发送FIFO取1个数据写入发送数据寄存器 */
			//USART_SendData(_pUart->uart, _pUart->pTxBuf[_pUart->usTxRead]);
			_pUart->uart->TDR = _pUart->pTxBuf[_pUart->usTxRead];
			if (++_pUart->usTxRead >= _pUart->usTxBufSize)
			{
				_pUart->usTxRead = 0;
			}
			_pUart->usTxCount--;
		}
	}
	
	/* 清除中断标志 */
	SET_BIT(_pUart->uart->ICR, UART_CLEAR_PEF);
	SET_BIT(_pUart->uart->ICR, UART_CLEAR_FEF);
	SET_BIT(_pUart->uart->ICR, UART_CLEAR_NEF);
	SET_BIT(_pUart->uart->ICR, UART_CLEAR_OREF);
	SET_BIT(_pUart->uart->ICR, UART_CLEAR_IDLEF);
	SET_BIT(_pUart->uart->ICR, UART_CLEAR_TCF);
	SET_BIT(_pUart->uart->ICR, UART_CLEAR_LBDF);
	SET_BIT(_pUart->uart->ICR, UART_CLEAR_CTSF);
	SET_BIT(_pUart->uart->ICR, UART_CLEAR_CMF);
	SET_BIT(_pUart->uart->ICR, UART_CLEAR_WUF);
	SET_BIT(_pUart->uart->ICR, UART_CLEAR_TXFECF);
}

/*
*********************************************************************************************************
*	函 数 名: USART1_IRQHandler  USART2_IRQHandler USART3_IRQHandler UART4_IRQHandler UART5_IRQHandler等
*	功能说明: USART中断服务程序
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
#if LPUART1_FIFO_EN == 1
void LPUART1_IRQHandler(void)
{
	LPUartIRQ(&g_tLPUart1);
}
#endif

/*
*********************************************************************************************************
*	函 数 名: fputc
*	功能说明: 重定义putc函数，这样可以使用printf函数从串口1打印输出
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
int fputc(int ch, FILE *f)
{
#if 1	/* 将需要printf的字符通过串口中断FIFO发送出去，printf函数会立即返回 */
	lpcomSendChar(LPCOM1, ch);
	
	return ch;
#else	/* 采用阻塞方式发送每个字符,等待数据发送完毕 */
	/* 写一个字节到USART1 */
	LPUART1->TDR = ch;
	
	/* 等待发送结束 */
	while((LPUART1->ISR & USART_ISR_TC) == 0)
	{}
	
	return ch;
#endif
}

/*
*********************************************************************************************************
*	函 数 名: fgetc
*	功能说明: 重定义getc函数，这样可以使用getchar函数从串口1输入数据
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
int fgetc(FILE *f)
{

#if 1	/* 从串口接收FIFO中取1个数据, 只有取到数据才返回 */
	uint8_t ucData;

	while(lpcomGetChar(LPCOM1, &ucData) == 0);

	return ucData;
#else
	/* 等待接收到数据 */
	while((USART1->ISR & USART_ISR_RXNE) == 0)
	{}

	return (int)USART1->RDR;
#endif
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
