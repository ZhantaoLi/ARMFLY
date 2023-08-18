/*
*********************************************************************************************************
*
*	ģ������ : FlashDev
*	�ļ����� : FlashDev.c
*	��    �� : V1.0
*	˵    �� : Flash����
*
*	�޸ļ�¼ :
*		�汾��  ����         ����       ˵��
*		V1.0    2020-11-06  Eric2013   ��ʽ����
*
*	Copyright (C), 2020-2030, ���������� www.armfly.com
*
*********************************************************************************************************
*/
#include "FlashOS.H"       


#ifdef FLASH_MEM
struct FlashDevice const FlashDevice  =  {
//    FLASH_DRV_VERS,                   /* �����汾�����޸ģ������MDK���� */
//    "ARMFLY_STM32H7x_QSPI_W25Q256",   /* �㷨��������㷨��MDK��װĿ¼����ʾ������ */
//    EXTSPI,                           /* �豸���� */
//    0x90000000,                       /* Flash��ʼ��ַ */
//    32 * 1024 * 1024,                 /* Flash��С��32MB */
//    4 * 1024,                         /* ���ҳ��С */
//    0,                                /* ����������Ϊ0 */
//    0xFF,                             /* ���������ֵ */
//    1000,                             /* ҳ��̵ȴ�ʱ�� */
//    6000,                             /* ���������ȴ�ʱ�� */
//    64 * 1024, 0x000000,              /* ������С��������ַ */
//    SECTOR_END    
      FLASH_DRV_VERS,              /* Driver Version, do not modify! */
    "ARMFLY_STM32H7x_SPI_25Q64",     /* Device Name */
    EXTSPI,                       /* Device Type */
    0xC0000000,                   /* Device Start Address */
    8 * 1024 * 1024,              /* Device Size in Bytes (2048kB) */
    256,                          /* Programming Page Size */
    0,                          /* Reserved, must be 0 */
    0xFF,                       /* Initial Content of Erased Memory */
    6000,                        /* Program Page Timeout 1000 mSec */
    6000,                       /* Erase Sector Timeout 1000 mSec */
    /* Specify Size and Address of Sectors */
    4 * 1024, 0x000000,        /* Sector Size  128kB (16 Sectors) */
    SECTOR_END    
};
#endif 

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
