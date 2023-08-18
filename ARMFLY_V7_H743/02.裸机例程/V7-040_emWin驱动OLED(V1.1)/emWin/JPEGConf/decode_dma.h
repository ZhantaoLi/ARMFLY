/*
*********************************************************************************************************
*
*	模块名称 : 图片文件
*	文件名称 : decode_dma.h
*	版    本 : V1.0
*	说    明 : JPEG图片解码
*	修改记录 :
*		版本号   日期         作者       说明
*		V1.0    2019-05-11  Eric2013    正式发布
*
*	Copyright (C), 2019-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/
#ifndef __DECODE_DMA_H
#define __DECODE_DMA_H

#include "bsp.h"

extern __IO uint32_t Jpeg_HWDecodingEnd;
extern JPEG_HandleTypeDef    JPEG_Handle;
extern  JPEG_ConfTypeDef       JPEG_Info;

uint32_t JPEG_Decode_DMA(JPEG_HandleTypeDef *hjpeg, uint32_t FrameSourceAddress ,uint32_t FrameSize, uint32_t DestAddress);

#endif 

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
