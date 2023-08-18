/*
*********************************************************************************************************
*
*	ģ������ : DAC8562 ����ģ��(˫ͨ����16λDAC)
*	�ļ����� : bsp_spidma_dac8562.h
*
*	Copyright (C), 2020-2030, ���������� www.armfly.com
*
*********************************************************************************************************
*/

#ifndef _BSP_SPIDMA_DAC8562_H
#define _BSP_SPIDMA_DAC8562_H

void bsp_InitDAC8562(void);
void DAC8562_SetDacData(uint8_t _ch, uint16_t _dac);
void DAC8562_WriteCmd(uint32_t _cmd);
int32_t DAC8562_DacToVoltage(uint16_t _dac);
uint32_t DAC8562_VoltageToDac(int32_t _volt);
void DAC8562_SetDacDataDMA(uint8_t _ch, uint16_t *_pbuf1, uint16_t *_pbuf2, uint32_t _size1, uint32_t _size2, uint32_t _ulFreq);

#endif

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
