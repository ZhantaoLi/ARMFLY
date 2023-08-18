/*
*********************************************************************************************************
*
*	ģ������ : DAC8562/8563 ����ģ��(˫ͨ����16λDAC)
*	�ļ����� : bsp_spidam_dac8562.c
*	��    �� : V1.0
*	˵    �� : DAC8562/8563ģ���CPU֮�����SPI�ӿڣ�����������֧��Ӳ��SPI��ʽ��
*
*
*	�޸ļ�¼ :
*		�汾��  ����         ����     ˵��
*		V1.0    2020-04-02  armfly  ��ʽ����
*
*	Copyright (C), 2020-2030, ���������� www.armfly.com
*
*********************************************************************************************************
*/

#include "bsp.h"



/*
	DAC8562ģ�����ֱ�Ӳ嵽STM32-V7������CN19��ĸ(2*4P 2.54mm)�ӿ���
	DAC8562/8563ģ�飺

	GND   ------  GND
	VCC   ------  3.3V

	SYNC  ------  PG10/SPI1_NSS
	SCLK  ------  PB3/SPI1_SCK
	DIN   ------  PB5/SPI1_MOSI

		  ------  PB4/SPI3_MISO               --- DAC�޶�������
	CLR   ------  PE4/NRF24L01_IRQ	          --- �Ƽ���GND��
	LDAC  ------  ��չIO/NRF24L01_CE/DAC1_OUT --- ����ӵػ�ָ��ǰ��0V

	DAC8562��������:
	1������2.7 - 5V;  ������ʹ��3.3V��
	2���ο���ѹ2.5V��ʹ���ڲ��ο�

	��SPI��ʱ���ٶ�Ҫ��: �ߴ�50MHz�� �ٶȺܿ�.
	SCLK�½��ض�ȡ����, ÿ�δ���24bit���ݣ� ��λ�ȴ�
*/

/* SYNC, Ҳ����CSƬѡ */	
#define CS_CLK_ENABLE() 	__HAL_RCC_GPIOG_CLK_ENABLE()
#define CS_GPIO				GPIOG
#define CS_PIN				GPIO_PIN_10
#define CS_1()				CS_GPIO->BSRR = CS_PIN
#define CS_0()				CS_GPIO->BSRR = ((uint32_t)CS_PIN << 16U)

/* CLR */	
#define CLR_CLK_ENABLE() 	__HAL_RCC_GPIOE_CLK_ENABLE()
#define CLR_GPIO			GPIOE
#define CLR_PIN				GPIO_PIN_4
#define CLR_1()				CLR_GPIO->BSRR = CLR_PIN
#define CLR_0()				CLR_GPIO->BSRR = ((uint32_t)CLR_PIN << 16U)

/* LDAC ʹ����չIO */	
#define LDAC_1()			HC574_SetPin(NRF24L01_CE, 1);
#define LDAC_0()			HC574_SetPin(NRF24L01_CE, 0);


/* �����ѹ��DACֵ��Ĺ�ϵ�� ����У׼ x��dac y �ǵ�ѹ 0.1mV */
#define X1	0
#define Y1  -100000

#define X2	65535
#define Y2  100000

/*
*********************************************************************************************************
*	�� �� ��: bsp_InitDAC8562
*	����˵��: ����GPIO����ʼ��DAC8562�Ĵ���
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void bsp_InitDAC8562(void)
{
	/* ����GPIO */
	{
		GPIO_InitTypeDef gpio_init;

		/* ��GPIOʱ�� */
		CS_CLK_ENABLE();
		CLR_CLK_ENABLE();
		
		gpio_init.Mode = GPIO_MODE_OUTPUT_PP;		/* ����������� */
		gpio_init.Pull = GPIO_NOPULL;				/* ���������費ʹ�� */
		gpio_init.Speed = GPIO_SPEED_FREQ_MEDIUM;  	/* GPIO�ٶȵȼ� */	
		
		gpio_init.Pin = CS_PIN;	
		HAL_GPIO_Init(CS_GPIO, &gpio_init);	

		gpio_init.Pin = CLR_PIN;	
		HAL_GPIO_Init(CLR_GPIO, &gpio_init);			
	}
	
	CLR_0();		/* CLR��GND�ɿ�һЩ��CLR���½��ش��� */
	LDAC_0();		/* �����첽����ģʽ�������Ž�GND */

	/* ��ʼ��DAC8562�Ĵ��� */
	{
		/* Power up DAC-A and DAC-B */
		DAC8562_WriteCmd((4 << 19) | (0 << 16) | (3 << 0));

		/* LDAC pin inactive for DAC-B and DAC-A  ��ʹ��LDAC���Ÿ������� */
		DAC8562_WriteCmd((6 << 19) | (0 << 16) | (3 << 0));

		/* ��λ2��DAC���м�ֵ, ���0V */
		DAC8562_SetDacData(0, 32767);
		DAC8562_SetDacData(1, 32767);

		/* ѡ���ڲ��ο�����λ2��DAC������=2 ����λʱ���ڲ��ο��ǽ�ֹ��) */
		DAC8562_WriteCmd((7 << 19) | (0 << 16) | (1 << 0));
	}
}

/*
*********************************************************************************************************
*	�� �� ��: DAC8562_SetCS
*	����˵��: DAC8562 Ƭѡ���ƺ���
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void DAC8562_SetCS(uint8_t _Level)
{
	if (_Level == 0)
	{
		bsp_SpiBusEnter();	/* ռ��SPI����  */	
		bsp_InitSPIParam(SPI_BAUDRATEPRESCALER_4, SPI_PHASE_2EDGE, SPI_POLARITY_LOW);		
		CS_0();
	}
	else
	{		
		CS_1();	
		bsp_SpiBusExit();	/* �ͷ�SPI���� */
	}	
}

/*
*********************************************************************************************************
*	�� �� ��: DAC8562_WriteCmd
*	����˵��: ��SPI���߷���24��bit���ݡ�
*	��    ��: _cmd : ����
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void DAC8562_WriteCmd(uint32_t _cmd)
{
	DAC8562_SetCS(0);

	g_spiLen = 0;
	g_spiTxBuf[g_spiLen++] = (_cmd >> 16);
	g_spiTxBuf[g_spiLen++] = (_cmd >> 8);
	g_spiTxBuf[g_spiLen++] = (_cmd);
	bsp_spiTransfer();		

	DAC8562_SetCS(1);
}

/*
*********************************************************************************************************
*	�� �� ��: DAC8562_SetDacData
*	����˵��: ����DAC��������������¡�
*	��    ��: _ch, ͨ��, 0 , 1
*		     _data : ����
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void DAC8562_SetDacData(uint8_t _ch, uint16_t _dac)
{
	if (_ch == 0)
	{
		/* Write to DAC-A input register and update DAC-A; */
		DAC8562_WriteCmd((3 << 19) | (0 << 16) | (_dac << 0));
	}
	else if (_ch == 1)
	{
		/* Write to DAC-B input register and update DAC-A; */
		DAC8562_WriteCmd((3 << 19) | (1 << 16) | (_dac << 0));
	}
}

/*
*********************************************************************************************************
*	�� �� ��: DAC8562_DacToVoltage
*	����˵��: ��DACֵ����Ϊ��ѹֵ����λ0.1mV
*	��    ��: _dac  16λDAC��
*	�� �� ֵ: ��ѹ����λ0.1mV
*********************************************************************************************************
*/
int32_t DAC8562_DacToVoltage(uint16_t _dac)
{
	int32_t y;

	/* CaculTwoPoint(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x);*/
	y =  CaculTwoPoint(X1, Y1, X2, Y2, _dac);
	return y;
}

/*
*********************************************************************************************************
*	�� �� ��: DAC8562_VoltageToDac
*	����˵��: ����ѹֵת��ΪDACֵ
*	��    ��: _volt ��ѹ����λ0.1mV
*	�� �� ֵ: 16λDAC��
*********************************************************************************************************
*/
uint32_t DAC8562_VoltageToDac(int32_t _volt)
{
	/* CaculTwoPoint(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x);*/
	return CaculTwoPoint(Y1, X1, Y2, X2, _volt);
}

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
