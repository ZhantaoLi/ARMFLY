/*
*********************************************************************************************************
*	                                  
*	ģ������ : CAN������ʾ����
*	�ļ����� : can_network.h
*	��    �� : V1.0
*	˵    �� : ͷ�ļ�
*	�޸ļ�¼ :
*		�汾��  ����       ����    ˵��
*		v1.0    2011-09-01 armfly  ST�̼���V3.5.0�汾��
*
*	Copyright (C), 2010-2011, ���������� www.armfly.com
*
*********************************************************************************************************
*/


#ifndef _CAN_NETWORK_H
#define _CAN_NETWORK_H

void can_Init(void);
void can_DeInit(void);

/* Ӧ�ò�Э�� */
void can_LedOn(uint8_t _addr, uint8_t _led_no);
void can_LedOff(uint8_t _addr, uint8_t _led_no);
void can_BeepCtrl(uint8_t _addr, uint8_t _cmd);

void can1_Analyze(void);
void can2_Analyze(void);

#endif


