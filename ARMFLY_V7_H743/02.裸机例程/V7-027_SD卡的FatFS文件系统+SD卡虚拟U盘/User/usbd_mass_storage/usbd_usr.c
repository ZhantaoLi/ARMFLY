/*
*********************************************************************************************************
*
*	ģ������ : USB devie �����������
*	�ļ����� : usbd_usr.c
*	��    �� : V1.0
*	˵    �� : ��װ����U�̲����������ṩ��APPʹ��.
*
*	�޸ļ�¼ :
*		�汾��  ����        ����     ˵��
*		V1.0    2018-09-05 armfly  ��ʽ����
*
*	Copyright (C), 2015-2030, ���������� www.armfly.com
*
*********************************************************************************************************
*/

#include "usbd_def.h"
#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_msc.h"
#include "usbd_storage.h"
#include "usbd_usr.h"

USBD_HandleTypeDef USBD_Device;
PCD_HandleTypeDef hpcd;

/*
*********************************************************************************************************
*	�� �� ��: usbd_OpenMassStorage
*	����˵��: ��USB
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void usbd_OpenMassStorage(void)
{
	/* Init Device Library */
	USBD_Init(&USBD_Device, &MSC_Desc, 0);


	/* Add Supported Class */
	USBD_RegisterClass(&USBD_Device, USBD_MSC_CLASS);

	/* Add Storage callbacks for MSC Class */
	USBD_MSC_RegisterStorage(&USBD_Device, &USBD_DISK_fops);

	/* Start Device Process */
	USBD_Start(&USBD_Device);

	HAL_PWREx_EnableUSBVoltageDetector();	
}

/*
*********************************************************************************************************
*	�� �� ��: usbd_CloseMassStorage
*	����˵��: �ر�USB
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void usbd_CloseMassStorage(void)
{
	USBD_Stop(&USBD_Device);
	
	USBD_DeInit(&USBD_Device);
}

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
