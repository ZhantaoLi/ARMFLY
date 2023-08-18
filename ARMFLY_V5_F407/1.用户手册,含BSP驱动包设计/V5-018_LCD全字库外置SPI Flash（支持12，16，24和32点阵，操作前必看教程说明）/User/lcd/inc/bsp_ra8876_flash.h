/*
*********************************************************************************************************
*
*	模块名称 :  RA8876芯片外挂的串行Flash驱动模块
*	文件名称 : bsp_ra8876_flash.h
*	版    本 : V1.0
*	说    明 : 头文件
*
*	Copyright (C), 2015-2020, 安富莱电子 www.armfly.com
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

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
