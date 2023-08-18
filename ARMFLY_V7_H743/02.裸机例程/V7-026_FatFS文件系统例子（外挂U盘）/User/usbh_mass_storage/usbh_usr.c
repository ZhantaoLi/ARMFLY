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
#include "bsp.h"
#include "usbh_usr.h"
#include "usbh_msc.h"
#include "ff_gen_drv.h"
#include "usbh_diskio.h"
 
#define usbh_printf	printf
 
FATFS usbfs;
USBH_HandleTypeDef hUSBHost;
char USBDISKPath[4];  /* USB�߼�����·�������̷�0������"0:/" */
  
static void USBH_UserProcess(USBH_HandleTypeDef * phost, uint8_t id);

/*
*********************************************************************************************************
*	�� �� ��: USBH_UserProcess
*	����˵��: USB host �ص�����. �����ڴ˸�app������Ϣ
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void USBH_UserProcess(USBH_HandleTypeDef * phost, uint8_t id)
{
	switch (id)
	{
		case HOST_USER_SELECT_CONFIGURATION:
			break;

		case HOST_USER_DISCONNECTION:		/* U�̶Ͽ� */
			if (f_mount(NULL, USBDISKPath, 0) != FR_OK)
			{
				USBH_ErrLog("Cannot DeInitialize FatFs!");
			}
			if (FATFS_UnLinkDriver(USBDISKPath) != 0)
			{
				USBH_ErrLog("Cannot UnLink FatFS Driver!");
			}
			break;

		case HOST_USER_CLASS_ACTIVE:
			break;

		case HOST_USER_CONNECTION:		/* U�̲��� */
			if (FATFS_LinkDriver(&USBH_Driver, USBDISKPath) == 0)
			{
				if (f_mount(&usbfs, USBDISKPath, 0) != FR_OK)
				{
					USBH_ErrLog("Cannot Initialize FatFs!");
				}
				else
				{
					USBH_UsrLog("Initialize FatFs!");
				}
			}
			break;

		default:
		break;
	}
}

/*
*********************************************************************************************************
*	�� �� ��: usbh_OpenMassStorage
*	����˵��: ��U���豸
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void usbh_OpenMassStorage(void)
{
	/* Init Host Library */
	USBH_Init(&hUSBHost, USBH_UserProcess, 0);

	/* Add Supported Class */
	USBH_RegisterClass(&hUSBHost, USBH_MSC_CLASS);

	/* Start Host Process */
	USBH_Start(&hUSBHost);
	
	HAL_PWREx_EnableUSBVoltageDetector();
}

/*
*********************************************************************************************************
*	�� �� ��: usbh_CloseMassStorage
*	����˵��: �ر�U���豸
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void usbh_CloseMassStorage(void)
{
	USBH_Stop(&hUSBHost);
	
	USBH_DeInit(&hUSBHost);
}

/*
*********************************************************************************************************
*	�� �� ��: usbh_Poll
*	����˵��: ��������Ҫ��ѯִ�б�����
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void usbh_Poll(void)
{
	/* USB Host Background task , �� usbh_core.c �е��� */
    USBH_Process(&hUSBHost);
}

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
