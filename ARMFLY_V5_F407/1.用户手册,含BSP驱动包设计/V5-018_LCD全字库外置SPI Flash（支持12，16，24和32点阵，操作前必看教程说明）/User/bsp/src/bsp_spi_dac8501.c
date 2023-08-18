/*
*********************************************************************************************************
*
*	ģ������ : DAC8501 ����ģ��(��ͨ����16λDAC)
*	�ļ����� : bsp_dac8501.c
*	��    �� : V1.0
*	˵    �� : DAC8501ģ���CPU֮�����SPI�ӿڡ�����������֧��Ӳ��SPI�ӿں����SPI�ӿڡ�
*			  ͨ�����л���
*
*	�޸ļ�¼ :
*		�汾��  ����         ����     ˵��
*		V1.0    2015-10-11  armfly  ��ʽ����
*
*	Copyright (C), 2015-2020, ���������� www.armfly.com
*
*********************************************************************************************************
*/
#include "bsp.h"



/*
	DAC8501ģ�����ֱ�Ӳ嵽STM32-V5������CN19��ĸ(2*4P 2.54mm)�ӿ���

    DAC8501ģ��    STM32F407������
	  VCC   ------  3.3V
	  GND   ------  GND
      SCLK  ------  PB3/SPI3_SCK
      MOSI  ------  PB5/SPI3_MOSI
      CS1   ------  PF7/NRF24L01_CSN
	  CS2   ------  PA4/NRF905_TX_EN/NRF24L01_CE/DAC1_OUT
			------  PB4/SPI3_MISO
			------  PH7/NRF24L01_IRQ
*/

/*
	DAC8501��������:
	1������2.7 - 5V;  ������ʹ��3.3V��
	4���ο���ѹ2.5V (�Ƽ�ȱʡ�ģ����õģ�

	��SPI��ʱ���ٶ�Ҫ��: �ߴ�30MHz�� �ٶȺܿ�.
	SCLK�½��ض�ȡ����, ÿ�δ���24bit���ݣ� ��λ�ȴ�
*/

#define CS1_CLK_ENABLE() 	__HAL_RCC_GPIOF_CLK_ENABLE()
#define CS1_GPIO			GPIOF
#define CS1_PIN				GPIO_PIN_7

#define CS1_1()				CS1_GPIO->BSRR = CS1_PIN
#define CS1_0()				CS1_GPIO->BSRR = ((uint32_t)CS1_PIN << 16U)

#define CS2_CLK_ENABLE() 	__HAL_RCC_GPIOA_CLK_ENABLE()
#define CS2_GPIO			GPIOA
#define CS2_PIN				GPIO_PIN_4

#define CS2_1()				CS2_GPIO->BSRR = CS2_PIN 
#define CS2_0()				CS2_GPIO->BSRR = ((uint32_t)CS2_PIN << 16U) 

/* �����ѹ��DACֵ��Ĺ�ϵ�� ����У׼ x��dac y �ǵ�ѹ 0.1mV */
#define X1	100
#define Y1  50

#define X2	65000
#define Y2  49400

/*
*********************************************************************************************************
*	�� �� ��: bsp_InitDAC8501
*	����˵��: ����STM32��GPIO��SPI�ӿ�
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void bsp_InitDAC8501(void)
{
	/* ����CS GPIO */
	{
		GPIO_InitTypeDef gpio_init;

		/* ��GPIOʱ�� */
		CS1_CLK_ENABLE();
		
		gpio_init.Mode = GPIO_MODE_OUTPUT_PP;		/* ����������� */
		gpio_init.Pull = GPIO_NOPULL;				/* ���������費ʹ�� */
		gpio_init.Speed = GPIO_SPEED_FREQ_HIGH;  	/* GPIO�ٶȵȼ� */	
		gpio_init.Pin = CS1_PIN;	
		HAL_GPIO_Init(CS1_GPIO, &gpio_init);	

		/* ��GPIOʱ�� */
		CS2_CLK_ENABLE();
		
		gpio_init.Mode = GPIO_MODE_OUTPUT_PP;		/* ����������� */
		gpio_init.Pull = GPIO_NOPULL;				/* ���������費ʹ�� */
		gpio_init.Speed = GPIO_SPEED_FREQ_HIGH;  	/* GPIO�ٶȵȼ� */	
		gpio_init.Pin = CS2_PIN;	
		HAL_GPIO_Init(CS2_GPIO, &gpio_init);	
	}

	DAC8501_SetDacData(0, 0);	/* CH1���0 */
	DAC8501_SetDacData(1, 0);	/* CH2���0 */
}

/*
*********************************************************************************************************
*	�� �� ��: DAC8501_SetCS1
*	����˵��: DAC8501 Ƭѡ���ƺ���
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void DAC8501_SetCS1(uint8_t _Level)
{
	if (_Level == 0)
	{
		bsp_SpiBusEnter();	/* ռ��SPI����  */	
		bsp_InitSPIParam(SPI_BAUDRATEPRESCALER_4, SPI_PHASE_2EDGE, SPI_POLARITY_LOW);		
		CS1_0();
	}
	else
	{		
		CS1_1();	
		bsp_SpiBusExit();	/* �ͷ�SPI���� */
	}	
}

/*
*********************************************************************************************************
*	�� �� ��: DAC8501_SetCS2(0)
*	����˵��: ����CS2�� ����������SPI����
*	��    ��: ��
	�� �� ֵ: ��
*********************************************************************************************************
*/
void DAC8501_SetCS2(uint8_t _level)
{
	if (_level == 0)
	{
		bsp_SpiBusEnter();	/* ռ��SPI����  */
		bsp_InitSPIParam(SPI_BAUDRATEPRESCALER_4, SPI_PHASE_2EDGE, SPI_POLARITY_LOW);		
		CS2_0();
	}
	else
	{
		CS2_1();
		bsp_SpiBusExit();	/* �ͷ�SPI���� */
	}
}

/*
*********************************************************************************************************
*	�� �� ��: DAC8501_SetDacData
*	����˵��: ����DAC����
*	��    ��: _ch, ͨ��,
*		     _data : ����
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void DAC8501_SetDacData(uint8_t _ch, uint16_t _dac)
{
	uint32_t data;

	/*
		DAC8501.pdf page 12 ��24bit����

		DB24:18 = xxxxx ����
		DB17�� PD1
		DB16�� PD0

		DB15��0  16λ����

		���� PD1 PD0 ����4�ֹ���ģʽ
		      0   0  ---> ��������ģʽ
		      0   1  ---> �����1Kŷ��GND
		      1   0  ---> ���100Kŷ��GND
		      1   1  ---> �������
	*/

	data = _dac; /* PD1 PD0 = 00 ����ģʽ */

	if (_ch == 0)
	{
		DAC8501_SetCS1(0);
	}
	else
	{
		DAC8501_SetCS2(0);
	}

	/*��DAC8501 SCLKʱ�Ӹߴ�30M����˿��Բ��ӳ� */
	g_spiLen = 0;
	g_spiTxBuf[g_spiLen++] = (data >> 16);
	g_spiTxBuf[g_spiLen++] = (data >> 8);
	g_spiTxBuf[g_spiLen++] = (data);
	bsp_spiTransfer();	

	if (_ch == 0)
	{
		DAC8501_SetCS1(1);
	}
	else
	{
		DAC8501_SetCS2(1);
	}
}

/*
*********************************************************************************************************
*	�� �� ��: DAC8501_DacToVoltage
*	����˵��: ��DACֵ����Ϊ��ѹֵ����λ0.1mV
*	��    ��: _dac  16λDAC��
*	�� �� ֵ: ��ѹ����λ0.1mV
*********************************************************************************************************
*/
int32_t DAC8501_DacToVoltage(uint16_t _dac)
{
	int32_t y;

	/* CaculTwoPoint(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x);*/
	y =  CaculTwoPoint(X1, Y1, X2, Y2, _dac);
	if (y < 0)
	{
		y = 0;
	}
	return y;
}

/*
*********************************************************************************************************
*	�� �� ��: DAC8501_DacToVoltage
*	����˵��: ��DACֵ����Ϊ��ѹֵ����λ 0.1mV
*	��    ��: _volt ��ѹ����λ0.1mV
*	�� �� ֵ: 16λDAC��
*********************************************************************************************************
*/
uint32_t DAC8501_VoltageToDac(int32_t _volt)
{
	/* CaculTwoPoint(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x);*/
	return CaculTwoPoint(Y1, X1, Y2, X2, _volt);
}

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
