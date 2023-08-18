/*
*********************************************************************************************************
*
*	ģ������ : �͹��Ĵ����ж�+FIFO����ģ��
*	�ļ����� : bsp_uart_fifo.c
*	��    �� : V1.0
*	˵    �� : ���ô����ж�+FIFOģʽʵ�ֶ�����ڵ�ͬʱ����
*	�޸ļ�¼ :
*		�汾��  ����       ����       ˵��
*		V1.0    2020-02-11 Eric2013  ��ʽ����
*
*	Copyright (C), 2020-2030, ���������� www.armfly.com
*
*********************************************************************************************************
*/
#include "bsp.h"



/* �͹��Ĵ���ʹ�õ�GPIO  PA9, PA10*/
#define LPUART1_CLK_ENABLE()              __HAL_RCC_LPUART1_CLK_ENABLE()

#define LPUART1_TX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOA_CLK_ENABLE()
#define LPUART1_TX_GPIO_PORT              GPIOA
#define LPUART1_TX_PIN                    GPIO_PIN_9
#define LPUART1_TX_AF                     GPIO_AF3_LPUART

#define LPUART1_RX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOA_CLK_ENABLE()
#define LPUART1_RX_GPIO_PORT              GPIOA
#define LPUART1_RX_PIN                    GPIO_PIN_10
#define LPUART1_RX_AF                     GPIO_AF3_LPUART

/* ѡ��LPUART��ʱ��Դ�������Ҫ�͹���ģʽ���ѣ�����ʹ��LSE����HSIʱ�ӣ���������bsp_lpuart_fifo.h���� */
//#define LPUART_CLOCK_SOURCE_LSE    /* LPUARTʱ��ѡ��LSE(32768Hz)������ٶ���10922bps�����8bps */	
#define LPUART_CLOCK_SOURCE_HSI    /* LPUARTʱ��ѡ��HSI(64MHz)�����ֵ��21MHz����Сֵ15625bps */	
//#define LPUART_CLOCK_SOURCE_D3PCLK1  /* LPUARTʱ��ѡ��D3PCLK1(100MHz)�����ֵ33Mbps����Сֵ24414bps */	

/* ����ÿ�����ڽṹ����� */
#if LPUART1_FIFO_EN == 1
	static LPUART_T g_tLPUart1;
	static uint8_t g_TxBuf1[LPUART1_TX_BUF_SIZE];		/* ���ͻ����� */
	static uint8_t g_RxBuf1[LPUART1_RX_BUF_SIZE];		/* ���ջ����� */
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
*	�� �� ��: bsp_InitLPUart
*	����˵��: ��ʼ������Ӳ��������ȫ�ֱ�������ֵ.
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void bsp_InitLPUart(void)
{
	LPUartVarInit();		/* �����ȳ�ʼ��ȫ�ֱ���,������Ӳ�� */
	InitHardLPUart();		/* ���ô��ڵ�Ӳ������(�����ʵ�) */
}

/*
*********************************************************************************************************
*	�� �� ��: ComToLPUart
*	����˵��: ��COM�˿ں�ת��ΪLPUARTָ��
*	��    ��: _ucPort: �˿ں�(LPCOM1)
*	�� �� ֵ: uartָ��
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
*	�� �� ��: ComToLPUARTx
*	����˵��: ��COM�˿ں�ת��Ϊ USART_TypeDef* USARTx
*	��    ��: _ucPort: �˿ں�(COM1 - COM8)
*	�� �� ֵ: USART_TypeDef*,  LPUSART1��
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
		/* �����κδ��� */
		return 0;
	}
}

/*
*********************************************************************************************************
*	�� �� ��: lpcomSendBuf
*	����˵��: �򴮿ڷ���һ�����ݡ����ݷŵ����ͻ��������������أ����жϷ�������ں�̨��ɷ���
*	��    ��: _ucPort: �˿ں�(LPCOM1)
*			  _ucaBuf: �����͵����ݻ�����
*			  _usLen : ���ݳ���
*	�� �� ֵ: ��
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
		pUart->SendBefor();		/* �����RS485ͨ�ţ���������������н�RS485����Ϊ����ģʽ */
	}

	LPUartSend(pUart, _ucaBuf, _usLen);
}

/*
*********************************************************************************************************
*	�� �� ��: lpcomSendChar
*	����˵��: �򴮿ڷ���1���ֽڡ����ݷŵ����ͻ��������������أ����жϷ�������ں�̨��ɷ���
*	��    ��: _ucPort: �˿ں�(LPCOM1)
*			  _ucByte: �����͵�����
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void lpcomSendChar(LPCOM_PORT_E _ucPort, uint8_t _ucByte)
{
	lpcomSendBuf(_ucPort, &_ucByte, 1);
}

/*
*********************************************************************************************************
*	�� �� ��: lpcomGetChar
*	����˵��: �ӽ��ջ�������ȡ1�ֽڣ��������������������ݾ��������ء�
*	��    ��: _ucPort: �˿ں�(LPCOM1)
*			  _pByte: ���յ������ݴ���������ַ
*	�� �� ֵ: 0 ��ʾ������, 1 ��ʾ��ȡ����Ч�ֽ�
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
*	�� �� ��: lpcomClearTxFifo
*	����˵��: ���㴮�ڷ��ͻ�����
*	��    ��: _ucPort: �˿ں�(LPCOM1)
*	�� �� ֵ: ��
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
*	�� �� ��: lpcomClearRxFifo
*	����˵��: ���㴮�ڽ��ջ�����
*	��    ��: _ucPort: �˿ں�(LPCOM1)
*	�� �� ֵ: ��
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
*	�� �� ��: lpcomSetBaud
*	����˵��: ���ô��ڵĲ�����. �������̶�����Ϊ��У�飬�շ���ʹ��ģʽ
*	��    ��: _ucPort: �˿ں�(LPCOM1)
*			  _BaudRate: �����ʣ�8��������  ������.0-12.5Mbps
*                                16�������� ������.0-6.25Mbps
*	�� �� ֵ: ��
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

/* �����RS485ͨ�ţ��밴���¸�ʽ��д���� */

#if 0
/*
*********************************************************************************************************
*	�� �� ��: RS485_InitTXE
*	����˵��: ����RS485����ʹ�ܿ��� TXE
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void RS485_InitTXE(void)
{
	GPIO_InitTypeDef gpio_init;
	
	/* ��GPIOʱ�� */
	RS485_TXEN_GPIO_CLK_ENABLE();
	
	/* ��������Ϊ������� */
	gpio_init.Mode = GPIO_MODE_OUTPUT_PP;			/* ������� */
	gpio_init.Pull = GPIO_NOPULL;					/* ���������費ʹ�� */
	gpio_init.Speed = GPIO_SPEED_FREQ_VERY_HIGH;	/* GPIO�ٶȵȼ� */
	gpio_init.Pin = RS485_TXEN_PIN;
	HAL_GPIO_Init(RS485_TXEN_GPIO_PORT, &gpio_init);
}

/*
*********************************************************************************************************
*	�� �� ��: RS485_SetBaud
*	����˵��: �޸�485���ڵĲ����ʡ�
*	��    ��: _baud : 8��������  ������.0-12.5Mbps
*                     16�������� ������.0-6.25Mbps
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void RS485_SetBaud(uint32_t _baud)
{
	comSetBaud(COM3, _baud);
}

/*
*********************************************************************************************************
*	�� �� ��: RS485_SendBefor
*	����˵��: ��������ǰ��׼������������RS485ͨ�ţ�������RS485оƬΪ����״̬��
*			  ���޸� UartVarInit()�еĺ���ָ����ڱ������������� g_tUart2.SendBefor = RS485_SendBefor
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void RS485_SendBefor(void)
{
	RS485_TX_EN();	/* �л�RS485�շ�оƬΪ����ģʽ */
}

/*
*********************************************************************************************************
*	�� �� ��: RS485_SendOver
*	����˵��: ����һ�����ݽ�������ƺ�������RS485ͨ�ţ�������RS485оƬΪ����״̬��
*			  ���޸� UartVarInit()�еĺ���ָ����ڱ������������� g_tUart2.SendOver = RS485_SendOver
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void RS485_SendOver(void)
{
	RS485_RX_EN();	/* �л�RS485�շ�оƬΪ����ģʽ */
}

/*
*********************************************************************************************************
*	�� �� ��: RS485_SendBuf
*	����˵��: ͨ��RS485оƬ����һ�����ݡ�ע�⣬���������ȴ�������ϡ�
*	��    ��: _ucaBuf : ���ݻ�����
*			  _usLen : ���ݳ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void RS485_SendBuf(uint8_t *_ucaBuf, uint16_t _usLen)
{
	comSendBuf(COM3, _ucaBuf, _usLen);
}

/*
*********************************************************************************************************
*	�� �� ��: RS485_SendStr
*	����˵��: ��485���߷���һ���ַ�����0������
*	��    ��: _pBuf �ַ�����0����
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void RS485_SendStr(char *_pBuf)
{
	RS485_SendBuf((uint8_t *)_pBuf, strlen(_pBuf));
}

/*
*********************************************************************************************************
*	�� �� ��: RS485_ReciveNew
*	����˵��: ���յ��µ�����
*	��    ��: _byte ���յ���������
*	�� �� ֵ: ��
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
*	�� �� ��: LPUartVarInit
*	����˵��: ��ʼ��������صı���
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void LPUartVarInit(void)
{
#if LPUART1_FIFO_EN == 1
	g_tLPUart1.uart = LPUART1;						/* STM32 �����豸 */
	g_tLPUart1.pTxBuf = g_TxBuf1;					/* ���ͻ�����ָ�� */
	g_tLPUart1.pRxBuf = g_RxBuf1;					/* ���ջ�����ָ�� */
	g_tLPUart1.usTxBufSize = LPUART1_TX_BUF_SIZE;	/* ���ͻ�������С */
	g_tLPUart1.usRxBufSize = LPUART1_RX_BUF_SIZE;	/* ���ջ�������С */
	g_tLPUart1.usTxWrite = 0;						/* ����FIFOд���� */
	g_tLPUart1.usTxRead = 0;						/* ����FIFO������ */
	g_tLPUart1.usRxWrite = 0;						/* ����FIFOд���� */
	g_tLPUart1.usRxRead = 0;						/* ����FIFO������ */
	g_tLPUart1.usRxCount = 0;						/* ���յ��������ݸ��� */
	g_tLPUart1.usTxCount = 0;						/* �����͵����ݸ��� */
	g_tLPUart1.SendBefor = 0;						/* ��������ǰ�Ļص����� */
	g_tLPUart1.SendOver = 0;						/* ������Ϻ�Ļص����� */
	g_tLPUart1.ReciveNew = 0;						/* ���յ������ݺ�Ļص����� */
	g_tLPUart1.Sending = 0;						/* ���ڷ����б�־ */
#endif
}

/*
*********************************************************************************************************
*	�� �� ��: bsp_SetUartParam
*	����˵��: ���ô��ڵ�Ӳ�������������ʣ�����λ��ֹͣλ����ʼλ��У��λ���ж�ʹ�ܣ��ʺ���STM32- H7������
*	��    ��: Instance   USART_TypeDef���ͽṹ��
*             BaudRate   ������
*             Parity     У�����ͣ���У�����żУ��
*             Mode       ���ͺͽ���ģʽʹ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/	
void bsp_SetLPUartParam(USART_TypeDef *Instance,  uint32_t BaudRate, uint32_t Parity, uint32_t Mode)
{
	/*##-1- ���ô���Ӳ������ ######################################*/
	/* �첽����ģʽ (UART Mode) */
	/* ��������:
	  - �ֳ�    = 8 λ
	  - ֹͣλ  = 1 ��ֹͣλ
	  - У��    = ����Parity
	  - ������  = ����BaudRate
	  - Ӳ�������ƹر� (RTS and CTS signals) */

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
*	�� �� ��: InitHardLPUart
*	����˵��: ���ô��ڵ�Ӳ�������������ʣ�����λ��ֹͣλ����ʼλ��У��λ���ж�ʹ�ܣ��ʺ���STM32-H7������
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void InitHardLPUart(void)
{
	GPIO_InitTypeDef  GPIO_InitStruct;
	RCC_PeriphCLKInitTypeDef   RCC_PeriphCLKInitStruct = {0};

/* ʹ��LSE(32768Hz)������ٶ���10922bps�����8bps */	
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
/* LPUARTʱ��ѡ��HSI(64MHz)�����ֵ��21MHz����Сֵ15625bps */	
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
/* LPUARTʱ��ѡ��D3PCLK1��100MHz�������ֵ33Mbps����Сֵ24414bps */	
#elif defined (LPUART_CLOCK_SOURCE_D3PCLK1)
	
	RCC_PeriphCLKInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LPUART1;
	RCC_PeriphCLKInitStruct.Lptim1ClockSelection = RCC_LPUART1CLKSOURCE_D3PCLK1;
	HAL_RCCEx_PeriphCLKConfig(&RCC_PeriphCLKInitStruct);
#else
	#error Please select the LPTIM Clock source inside the bsp_lpuart_fifo.c file
#endif

#if LPUART1_FIFO_EN == 1		/* ����1 */
	/* ʹ�� GPIO TX/RX ʱ�� */
	LPUART1_TX_GPIO_CLK_ENABLE();
	LPUART1_RX_GPIO_CLK_ENABLE();
	
	/* ʹ�� USARTx ʱ�� */
	LPUART1_CLK_ENABLE();	

	/* ����TX���� */
	GPIO_InitStruct.Pin       = LPUART1_TX_PIN;
	GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull      = GPIO_PULLUP;
	GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.Alternate = LPUART1_TX_AF;
	HAL_GPIO_Init(LPUART1_TX_GPIO_PORT, &GPIO_InitStruct);	
	
	/* ����RX���� */
	GPIO_InitStruct.Pin = LPUART1_RX_PIN;
	GPIO_InitStruct.Alternate = LPUART1_RX_AF;
	HAL_GPIO_Init(LPUART1_RX_GPIO_PORT, &GPIO_InitStruct);

	/* ����NVIC the NVIC for UART */   
	HAL_NVIC_SetPriority(LPUART1_IRQn, 0, 1);
	HAL_NVIC_EnableIRQ(LPUART1_IRQn);
  
	/* �� �ò����ʡ���żУ�� */
	bsp_SetLPUartParam(LPUART1,  LPUART1_BAUD, UART_PARITY_NONE, UART_MODE_TX_RX);

	SET_BIT(LPUART1->ICR, USART_ICR_TCCF);	 /* ���TC������ɱ�־ */
	SET_BIT(LPUART1->RQR, USART_RQR_RXFRQ);  /* ���RXNE���ձ�־ */
	// USART_CR1_PEIE | USART_CR1_RXNEIE
	SET_BIT(LPUART1->CR1, USART_CR1_RXNEIE); /* ʹ��PE. RX�����ж� */
#endif
}

/*
*********************************************************************************************************
*	�� �� ��: LPUartSend
*	����˵��: ��д���ݵ�UART���ͻ�����,�����������жϡ��жϴ�����������Ϻ��Զ��رշ����ж�
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void LPUartSend(LPUART_T *_pUart, uint8_t *_ucaBuf, uint16_t _usLen)
{
	uint16_t i;

	for (i = 0; i < _usLen; i++)
	{
		/* ������ͻ������Ѿ����ˣ���ȴ��������� */
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
			else if(usCount == _pUart->usTxBufSize)/* ���������������� */
			{
				if((_pUart->uart->CR1 & USART_CR1_TXEIE) == 0)
				{
					SET_BIT(_pUart->uart->CR1, USART_CR1_TXEIE);
				}  
			}
		}

		/* �����������뷢�ͻ����� */
		_pUart->pTxBuf[_pUart->usTxWrite] = _ucaBuf[i];

		DISABLE_INT();
		if (++_pUart->usTxWrite >= _pUart->usTxBufSize)
		{
			_pUart->usTxWrite = 0;
		}
		_pUart->usTxCount++;
		ENABLE_INT();
	}

	SET_BIT(_pUart->uart->CR1, USART_CR1_TXEIE);	/* ʹ�ܷ����жϣ��������գ� */
}

/*
*********************************************************************************************************
*	�� �� ��: LPUartGetChar
*	����˵��: �Ӵ��ڽ��ջ�������ȡ1�ֽ����� ��������������ã�
*	��    ��: _pUart : �����豸
*			  _pByte : ��Ŷ�ȡ���ݵ�ָ��
*	�� �� ֵ: 0 ��ʾ������  1��ʾ��ȡ������
*********************************************************************************************************
*/
static uint8_t LPUartGetChar(LPUART_T *_pUart, uint8_t *_pByte)
{
	uint16_t usCount;

	/* usRxWrite �������жϺ����б���д���������ȡ�ñ���ʱ����������ٽ������� */
	DISABLE_INT();
	usCount = _pUart->usRxCount;
	ENABLE_INT();

	/* �������д������ͬ���򷵻�0 */
	//if (_pUart->usRxRead == usRxWrite)
	if (usCount == 0)	/* �Ѿ�û������ */
	{
		return 0;
	}
	else
	{
		*_pByte = _pUart->pRxBuf[_pUart->usRxRead];		/* �Ӵ��ڽ���FIFOȡ1������ */

		/* ��дFIFO������ */
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
*   �� �� ��: LPUartTxEmpty
*   ����˵��: �жϷ��ͻ������Ƿ�Ϊ�ա�
*   ��    ��:  _pUart : �����豸
*   �� �� ֵ: 1Ϊ�ա�0Ϊ���ա�
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
*	�� �� ��: LPUartIRQ
*	����˵��: ���жϷ��������ã�ͨ�ô����жϴ�����
*	��    ��: _pUart : �����豸
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void LPUartIRQ(LPUART_T *_pUart)
{
	uint32_t isrflags   = READ_REG(_pUart->uart->ISR);
	uint32_t cr1its     = READ_REG(_pUart->uart->CR1);
	uint32_t cr3its     = READ_REG(_pUart->uart->CR3);
	
	/* ��������ж�  */
	if ((isrflags & USART_ISR_RXNE_RXFNE) != RESET)
	{
		/* �Ӵ��ڽ������ݼĴ�����ȡ���ݴ�ŵ�����FIFO */
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

		/* �ص�����,֪ͨӦ�ó����յ�������,һ���Ƿ���1����Ϣ��������һ����� */
		//if (_pUart->usRxWrite == _pUart->usRxRead)
		//if (_pUart->usRxCount == 1)
		{
			if (_pUart->ReciveNew)
			{
				_pUart->ReciveNew(ch); /* ���磬����MODBUS����������ֽ��� */
			}
		}
	}

	/* �����ͻ��������ж� */
	if ( ((isrflags & USART_ISR_TXE_TXFNF) != RESET) && (cr1its & USART_CR1_TXEIE) != RESET)
	{
		//if (_pUart->usTxRead == _pUart->usTxWrite)
		if (_pUart->usTxCount == 0)
		{
			/* ���ͻ�������������ȡ��ʱ�� ��ֹ���ͻ��������ж� ��ע�⣺��ʱ���1�����ݻ�δ����������ϣ�*/
			//USART_ITConfig(_pUart->uart, USART_IT_TXE, DISABLE);
			CLEAR_BIT(_pUart->uart->CR1, USART_CR1_TXEIE);

			/* ʹ�����ݷ�������ж� */
			//USART_ITConfig(_pUart->uart, USART_IT_TC, ENABLE);
			SET_BIT(_pUart->uart->CR1, USART_CR1_TCIE);
		}
		else
		{
			_pUart->Sending = 1;
			
			/* �ӷ���FIFOȡ1���ֽ�д�봮�ڷ������ݼĴ��� */
			//USART_SendData(_pUart->uart, _pUart->pTxBuf[_pUart->usTxRead]);
			_pUart->uart->TDR = _pUart->pTxBuf[_pUart->usTxRead];
			if (++_pUart->usTxRead >= _pUart->usTxBufSize)
			{
				_pUart->usTxRead = 0;
			}
			_pUart->usTxCount--;
		}

	}
	/* ����bitλȫ��������ϵ��ж� */
	if (((isrflags & USART_ISR_TC) != RESET) && ((cr1its & USART_CR1_TCIE) != RESET))
	{
		//if (_pUart->usTxRead == _pUart->usTxWrite)
		if (_pUart->usTxCount == 0)
		{
			/* �������FIFO������ȫ��������ϣ���ֹ���ݷ�������ж� */
			//USART_ITConfig(_pUart->uart, USART_IT_TC, DISABLE);
			CLEAR_BIT(_pUart->uart->CR1, USART_CR1_TCIE);

			/* �ص�����, һ����������RS485ͨ�ţ���RS485оƬ����Ϊ����ģʽ��������ռ���� */
			if (_pUart->SendOver)
			{
				_pUart->SendOver();
			}
			
			_pUart->Sending = 0;
		}
		else
		{
			/* ��������£��������˷�֧ */

			/* �������FIFO�����ݻ�δ��ϣ���ӷ���FIFOȡ1������д�뷢�����ݼĴ��� */
			//USART_SendData(_pUart->uart, _pUart->pTxBuf[_pUart->usTxRead]);
			_pUart->uart->TDR = _pUart->pTxBuf[_pUart->usTxRead];
			if (++_pUart->usTxRead >= _pUart->usTxBufSize)
			{
				_pUart->usTxRead = 0;
			}
			_pUart->usTxCount--;
		}
	}
	
	/* ����жϱ�־ */
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
*	�� �� ��: USART1_IRQHandler  USART2_IRQHandler USART3_IRQHandler UART4_IRQHandler UART5_IRQHandler��
*	����˵��: USART�жϷ������
*	��    ��: ��
*	�� �� ֵ: ��
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
*	�� �� ��: fputc
*	����˵��: �ض���putc��������������ʹ��printf�����Ӵ���1��ӡ���
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
int fputc(int ch, FILE *f)
{
#if 1	/* ����Ҫprintf���ַ�ͨ�������ж�FIFO���ͳ�ȥ��printf�������������� */
	lpcomSendChar(LPCOM1, ch);
	
	return ch;
#else	/* ����������ʽ����ÿ���ַ�,�ȴ����ݷ������ */
	/* дһ���ֽڵ�USART1 */
	LPUART1->TDR = ch;
	
	/* �ȴ����ͽ��� */
	while((LPUART1->ISR & USART_ISR_TC) == 0)
	{}
	
	return ch;
#endif
}

/*
*********************************************************************************************************
*	�� �� ��: fgetc
*	����˵��: �ض���getc��������������ʹ��getchar�����Ӵ���1��������
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
int fgetc(FILE *f)
{

#if 1	/* �Ӵ��ڽ���FIFO��ȡ1������, ֻ��ȡ�����ݲŷ��� */
	uint8_t ucData;

	while(lpcomGetChar(LPCOM1, &ucData) == 0);

	return ucData;
#else
	/* �ȴ����յ����� */
	while((USART1->ISR & USART_ISR_RXNE) == 0)
	{}

	return (int)USART1->RDR;
#endif
}

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
