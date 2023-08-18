/*
*********************************************************************************************************
*
*	ģ������ : DAC8501����ģ��(˫·�䣬16bit DAC)
*	�ļ����� : bsp_spidma_dac8591.h
*
*	Copyright (C), 2020-2030, ���������� www.armfly.com
*
*********************************************************************************************************
*/

#ifndef _BSP_SPIDMA_DAC8501_H
#define _BSP_SPIDMA_DAC8501_H

void bsp_InitDAC8501(void);
void DAC8501_SetDacData(uint8_t _ch, uint16_t _dac);
void DAC8501_WriteCmd(uint32_t _cmd);
int32_t DAC8501_DacToVoltage(uint16_t _dac);
uint32_t DAC8501_VoltageToDac(int32_t _volt);
void DAC8501_SetDacDataDMA(uint8_t _ch, uint16_t *_pbufch1, uint32_t _sizech1, uint32_t _ulFreq);

#endif

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
