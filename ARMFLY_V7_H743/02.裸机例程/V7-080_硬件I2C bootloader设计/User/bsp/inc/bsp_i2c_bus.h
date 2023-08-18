/*
*********************************************************************************************************
*
*	ģ������ : i2c��������
*	�ļ����� : bsp_i2c_bus.h
*	��    �� : V1.0
*	˵    �� : ͷ�ļ�
*
*	Copyright (C), 2020-2030, ���������� www.armfly.com
*
*********************************************************************************************************
*/

#ifndef __BSP_I2C_BUS_H
#define __BSP_I2C_BUS_H

void bsp_InitI2CBus(void);
void bsp_i2cTransfer(void);
void bsp_i2cReceive(void);

enum {
	TRANSFER_WAIT,
	TRANSFER_COMPLETE,
	TRANSFER_ERROR
};

#define	I2C_BUFFER_SIZE		(4 * 1024)	

extern uint8_t g_i2cTxBuf[I2C_BUFFER_SIZE];
extern uint8_t g_i2cRxBuf[I2C_BUFFER_SIZE];
extern uint32_t g_i2cLen;

#endif

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
