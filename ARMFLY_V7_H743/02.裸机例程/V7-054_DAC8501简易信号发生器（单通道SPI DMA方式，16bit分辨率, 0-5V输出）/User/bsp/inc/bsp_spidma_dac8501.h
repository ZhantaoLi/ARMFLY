/*
*********************************************************************************************************
*
*	模块名称 : DAC8501驱动模块(双路输，16bit DAC)
*	文件名称 : bsp_spidma_dac8591.h
*
*	Copyright (C), 2020-2030, 安富莱电子 www.armfly.com
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

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
