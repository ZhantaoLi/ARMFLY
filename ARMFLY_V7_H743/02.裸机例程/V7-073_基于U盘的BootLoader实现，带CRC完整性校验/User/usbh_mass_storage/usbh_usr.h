/*
*********************************************************************************************************
*
*	ģ������ : U�������û��ӿ�
*	�ļ����� : usbh_usr.c
*	��    �� : V1.0
*	˵    �� : ��װU�̲����������ṩ��APPʹ��.
*
*	�޸ļ�¼ :
*		�汾��  ����        ����     ˵��
*		V1.0    2018-09-05 armfly  ��ʽ����
*
*	Copyright (C), 2015-2030, ���������� www.armfly.com
*
*********************************************************************************************************
*/
#ifndef __USH_USR_H__
#define __USH_USR_H__

#include "ff.h"
#include "usbh_core.h"
#include "usbh_conf.h"
#include <stdio.h>

void usbh_OpenMassStorage(void);
void usbh_CloseMassStorage(void);
void usbh_Poll(void);

extern USBH_HandleTypeDef hUSBHost;
extern FATFS USBH_fatfs;
 
#endif

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
