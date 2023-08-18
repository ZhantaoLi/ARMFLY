/*
*********************************************************************************************************
*
*	模块名称 : 低功耗串口中断+FIFO驱动模块
*	文件名称 : bsp_lpuart_fifo.h
*	说    明 : 头文件
*
*	Copyright (C), 2020-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#ifndef _BSP_LPUSART_FIFO_H_
#define _BSP_LPUSART_FIFO_H_

/*
	安富莱STM32-V7 低功耗串口分配：
	【串口1】 RS232 芯片第1路。
		PA9/LPUART1_TX  --- 打印调试口
		P10/LPUART1_RX  
*/
#define	LPUART1_FIFO_EN	1

/* 定义端口号 */
typedef enum
{
	LPCOM1 = 0,	/* LPUSART1 */
}LPCOM_PORT_E;

/* 定义串口波特率和FIFO缓冲区大小，分为发送缓冲区和接收缓冲区, 支持全双工 */
#if LPUART1_FIFO_EN == 1
	#define LPUART1_BAUD		 115200
	#define LPUART1_TX_BUF_SIZE	 1*1024
	#define LPUART1_RX_BUF_SIZE	 1*1024
#endif


/* 串口设备结构体 */
typedef struct
{
	USART_TypeDef *uart;		/* STM32内部串口设备指针 */
	uint8_t *pTxBuf;			/* 发送缓冲区 */
	uint8_t *pRxBuf;			/* 接收缓冲区 */
	uint16_t usTxBufSize;		/* 发送缓冲区大小 */
	uint16_t usRxBufSize;		/* 接收缓冲区大小 */
	__IO uint16_t usTxWrite;	/* 发送缓冲区写指针 */
	__IO uint16_t usTxRead;		/* 发送缓冲区读指针 */
	__IO uint16_t usTxCount;	/* 等待发送的数据个数 */

	__IO uint16_t usRxWrite;	/* 接收缓冲区写指针 */
	__IO uint16_t usRxRead;		/* 接收缓冲区读指针 */
	__IO uint16_t usRxCount;	/* 还未读取的新数据个数 */

	void (*SendBefor)(void); 	/* 开始发送之前的回调函数指针（主要用于RS485切换到发送模式） */
	void (*SendOver)(void); 	/* 发送完毕的回调函数指针（主要用于RS485将发送模式切换为接收模式） */
	void (*ReciveNew)(uint8_t _byte);	/* 串口收到数据的回调函数指针 */
	uint8_t Sending;			/* 正在发送中 */
}LPUART_T;

extern UART_HandleTypeDef UartHandle;

void bsp_InitLPUart(void);
void lpcomSendBuf(LPCOM_PORT_E _ucPort, uint8_t *_ucaBuf, uint16_t _usLen);
void lpcomSendChar(LPCOM_PORT_E _ucPort, uint8_t _ucByte);
uint8_t lpcomGetChar(LPCOM_PORT_E _ucPort, uint8_t *_pByte);
void lpcomSendBuf(LPCOM_PORT_E _ucPort, uint8_t *_ucaBuf, uint16_t _usLen);
void lpcomClearTxFifo(LPCOM_PORT_E _ucPort);
void lpcomClearRxFifo(LPCOM_PORT_E _ucPort);
void lpcomSetBaud(LPCOM_PORT_E _ucPort, uint32_t _BaudRate);

void USART_SetBaudRate(USART_TypeDef* USARTx, uint32_t BaudRate);
void bsp_SetUartParam(USART_TypeDef *Instance,  uint32_t BaudRate, uint32_t Parity, uint32_t Mode);
#endif

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
