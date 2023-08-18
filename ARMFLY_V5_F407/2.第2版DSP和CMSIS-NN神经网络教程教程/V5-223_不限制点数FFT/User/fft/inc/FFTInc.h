/*
*********************************************************************************************************
*
*	ģ������ : FFT
*	�ļ����� : FFTInc.h
*	��    �� : V1.0
*	˵    �� : �����Ƶ���FFT�������ɺ궨��MAX_FFT_N���á�
*              ����8192��16384�㣬�����sin��con��ֵ�浽���ڲ�Flash���档
*              ��С֧��16�㣬���㲻�ޣ�����2^n���ɡ�
*
*	�޸ļ�¼ :
*		�汾��  ����         ����       ˵��
*		V1.0    2021-06-23  Eric2013   ��ʽ����
*
*	Copyright (C), 2021-2030, ���������� www.armfly.com
*
*********************************************************************************************************
*/
#ifndef __FFTInc_H_
#define __FFTInc_H_

#include "bsp.h"
#include "math.h"
#include "arm_math.h"

#define   MAX_FFT_N		 8192	

struct  compx 
{
	float32_t real, imag;
};   

void InitTableFFT(uint32_t n);
void cfft(struct compx *_ptr, uint32_t FFT_N );

#endif

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
