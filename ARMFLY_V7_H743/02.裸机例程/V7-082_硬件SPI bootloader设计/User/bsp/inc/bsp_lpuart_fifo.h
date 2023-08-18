/*
*********************************************************************************************************
*
*	ģ������ : �͹��Ĵ����ж�+FIFO����ģ��
*	�ļ����� : bsp_lpuart_fifo.h
*	˵    �� : ͷ�ļ�
*
*	Copyright (C), 2020-2030, ���������� www.armfly.com
*
*********************************************************************************************************
*/

#ifndef _BSP_LPUSART_FIFO_H_
#define _BSP_LPUSART_FIFO_H_

/*
	������STM32-V7 �͹��Ĵ��ڷ��䣺
	������1�� RS232 оƬ��1·��
		PA9/LPUART1_TX  --- ��ӡ���Կ�
		P10/LPUART1_RX  
*/
#define	LPUART1_FIFO_EN	1

/* ����˿ں� */
typedef enum
{
	LPCOM1 = 0,	/* LPUSART1 */
}LPCOM_PORT_E;

/* ���崮�ڲ����ʺ�FIFO��������С����Ϊ���ͻ������ͽ��ջ�����, ֧��ȫ˫�� */
#if LPUART1_FIFO_EN == 1
	#define LPUART1_BAUD		 115200
	#define LPUART1_TX_BUF_SIZE	 1*1024
	#define LPUART1_RX_BUF_SIZE	 1*1024
#endif


/* �����豸�ṹ�� */
typedef struct
{
	USART_TypeDef *uart;		/* STM32�ڲ������豸ָ�� */
	uint8_t *pTxBuf;			/* ���ͻ����� */
	uint8_t *pRxBuf;			/* ���ջ����� */
	uint16_t usTxBufSize;		/* ���ͻ�������С */
	uint16_t usRxBufSize;		/* ���ջ�������С */
	__IO uint16_t usTxWrite;	/* ���ͻ�����дָ�� */
	__IO uint16_t usTxRead;		/* ���ͻ�������ָ�� */
	__IO uint16_t usTxCount;	/* �ȴ����͵����ݸ��� */

	__IO uint16_t usRxWrite;	/* ���ջ�����дָ�� */
	__IO uint16_t usRxRead;		/* ���ջ�������ָ�� */
	__IO uint16_t usRxCount;	/* ��δ��ȡ�������ݸ��� */

	void (*SendBefor)(void); 	/* ��ʼ����֮ǰ�Ļص�����ָ�루��Ҫ����RS485�л�������ģʽ�� */
	void (*SendOver)(void); 	/* ������ϵĻص�����ָ�루��Ҫ����RS485������ģʽ�л�Ϊ����ģʽ�� */
	void (*ReciveNew)(uint8_t _byte);	/* �����յ����ݵĻص�����ָ�� */
	uint8_t Sending;			/* ���ڷ����� */
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

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
