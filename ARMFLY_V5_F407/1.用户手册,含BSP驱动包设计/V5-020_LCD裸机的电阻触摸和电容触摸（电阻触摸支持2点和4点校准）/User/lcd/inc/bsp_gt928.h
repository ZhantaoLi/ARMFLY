/*
*********************************************************************************************************
*
*	ģ������ : GT928���ݴ���оƬ��������
*	�ļ����� : bsp_ct928.h
*	��    �� : V1.0
*	˵    �� : ͷ�ļ�
*
*	Copyright (C), 2015-2020, ���������� www.armfly.com
*
*********************************************************************************************************
*/
#ifndef _BSP_GT928_H_
#define _BSP_GT928_H_

#define	GT928_I2C_ADDR		0x28		/* 0x28��0xBA����INT���ϵ�״̬���������Թ�����,ֻ��0x28�ܹ��� */
	
typedef struct
{
	uint8_t Enable;
	uint8_t TimerCount;

	uint8_t TouchReg;		/* �����Ĵ��� */

	uint8_t Id[10];			/* ���ID */
	uint16_t X[10];			/* 10��������X������ */
	uint16_t Y[10];			/* 10��������Y������ */
	uint16_t Size[10];		/* ������ߴ� */
	
	uint8_t TouchkeyValue;	/* ������ֵ */
}GT928_T;

void GT928_Timer1ms(void);
void GT928_InitHard(void);
void GT928_Scan(void);

extern GT928_T g_GT928;

#endif

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
