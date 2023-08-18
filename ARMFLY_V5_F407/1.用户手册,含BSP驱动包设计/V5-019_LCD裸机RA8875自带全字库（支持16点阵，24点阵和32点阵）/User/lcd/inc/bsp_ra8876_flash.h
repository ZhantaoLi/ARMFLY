/*
*********************************************************************************************************
*
*	ģ������ :  RA8876оƬ��ҵĴ���Flash����ģ��
*	�ļ����� : bsp_ra8876_flash.h
*	��    �� : V1.0
*	˵    �� : ͷ�ļ�
*
*	Copyright (C), 2015-2020, ���������� www.armfly.com
*
*********************************************************************************************************
*/

#ifndef _BSP_RA8876_FLASH_H
#define _BSP_RA8876_FLASH_H

void bsp_InitRA8876Flash(void);
void RA8876_w25_CtrlByMCU(void);
void RA8876_w25_CtrlByRA8876(void);

void RA8876_w25_EraseSector(uint32_t _uiSectorAddr);
void RA8876_w25_EraseChip(void);
void RA8876_w25_WritePage(uint8_t * _pBuf, uint32_t _uiWriteAddr, uint16_t _usSize);
void RA8876_w25_ReadBuffer(uint8_t * _pBuf, uint32_t _uiReadAddr, uint32_t _uiSize);
void RA8876_w25_SelectChip(uint8_t _idex);

#endif

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
