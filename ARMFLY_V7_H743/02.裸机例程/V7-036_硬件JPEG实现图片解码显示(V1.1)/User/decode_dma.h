/*
*********************************************************************************************************
*
*	ģ������ : ͼƬ�ļ�
*	�ļ����� : decode_dma.h
*	��    �� : V1.0
*	˵    �� : JPEGͼƬ����
*	�޸ļ�¼ :
*		�汾��   ����         ����       ˵��
*		V1.0    2019-05-11  Eric2013    ��ʽ����
*
*	Copyright (C), 2019-2030, ���������� www.armfly.com
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

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
