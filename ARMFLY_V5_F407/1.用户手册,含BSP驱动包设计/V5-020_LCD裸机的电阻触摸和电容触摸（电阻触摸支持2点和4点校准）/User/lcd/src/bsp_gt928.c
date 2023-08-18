/*
*********************************************************************************************************
*
*	ģ������ : ���ݴ���оƬGT928��������
*	�ļ����� : bsp_ct928.c
*	��    �� : V1.0
*	˵    �� : GT928����оƬ��������
*	�޸ļ�¼ :
*		�汾��   ����        ����     ˵��
*		V1.0    2016-07-18  armfly   ��ʽ����
*
*	Copyright (C), 2015-2020, ���������� www.armfly.com
*********************************************************************************************************
*/
#include "bsp.h"

#if 0		/* �����ѱ��̻���GT928�� */
/* gt928 cfg */
uint8_t s_GT928_CfgParams[] = {
	/* 
	0x8047 R/W  �����ļ��汾��
	*/
	0x41,
	/* 
	0x8048 R/W  X_Ou_Max_L X ����������ֵ  600
	0x8049 R/W  X_Ou_Max_H
	*/
	0x58,0x02,	//0x00,0x04,//0x00,0x10,
	/*
	0x804A R/W  Y_Ou_Max_L  Y ����������ֵ  1024
	0x804B R/W  Y_Ou_Max_H
	*/
	0x00,0x04,//0x58,0x02,//0x00,0x10,
	/*
	0x804C R/W  �������������� 5
	*/
	0x0A,
	/*
	0x804D R/W Module_Switch1 Reserved Stretch_rank X2Y Sito INT������ʽ 
	*/
	0x7D,
	/*
	0x804E R/W Module_Switch2 Touch_key
	*/
	0x00,
	/*
	0x804F  ��ָ����/�ɿ�ȥ������
	*/
	0x02,
	/* 
	0x8050	Filter  ԭʼ���괰���˲�ֵ��ϵ��Ϊ1
	*/
	0x08,
	/*
	0x8051  Large_Touch	��������������
	*/
	0x19,
	/*
	0x8052	Noise_Reduction	��������ֵ��ϵ��Ϊ1
	*/
	0x0D,
	/* 
	0x8053	���ϴ�������޵��е���ֵ 
	*/
	0x32,
	/*
	0x8054	���ϴ�������е��޵���ֵ
	*/
	0x28,
	/*
	0x8055	���͹���ʱ�䣨0-15s��
	*/
	0x03,
	/*
	0x8056	�����ϱ��ʣ�����Ϊ5+Nms��
	*/
	0x00,
	/* 0x8057 - 0x8058 ���� */
	0x00,0x00,
	/*
	0x8059	�ٶ����Ʋ���
	*/
	0x00,0x00,
	/*
	0x805B Space ��Χ��Ŀհ���
	*/
	0x00,0x00,	
	/*
	0x805D - 0x8061 ����
	*/
	0x00,0x18,0x1A,0x32,0x14,
	/*
	0x8062 - 0x8063 Drv_GroupA_Num	Drv_GroupB_Num
	*/
	0x91,0x31,//0x90,0x30,
	/*
	0x8064	Sensor_Num
	*/
	0xCC,
	
	0x28,0x2D,0x7C,0x15,0x00,0x00,0x00,0x00,0x03,
	0x45,0x00,0x00,0x00,0x00,0x00,0x03,0x64,0x32,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1D,0x1C,0x1B,0x1A,0x19,
	0x18,0x17,0x16,0x15,0x14,0x11,0x10,0x0F,0x0E,0x0D,0x0C,0x09,0x08,
	0x07,0x06,0x05,0x04,0x01,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,
	0x02,0x04,0x06,0x07,0x08,0x0A,0x0C,0x0D,0x0E,0x0F,0x10,0x11,0x12,
	0x13,0x14,0x19,0x1B,0x1C,0x1E,0x1F,0x20,0x21,0x22,0x23,0x24,0x25,
	0x26,0x27,0x28,0x29,0x2A,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0x83,0x01
};
#endif

#define GT928_READ_XY_REG	0x814E		/* ����Ĵ��� */
#define GT928_CONFIG_REG	0x8047		/* ���ò����Ĵ��� */

static void GT928_WriteReg(uint16_t _usRegAddr, uint8_t *_pRegBuf, uint8_t _ucLen);
static void GT928_ReadReg(uint16_t _usRegAddr, uint8_t *_pRegBuf, uint8_t _ucLen);
static uint16_t GT928_ReadVersion(void);

GT928_T g_GT928;

/*
*********************************************************************************************************
*	�� �� ��: bsp_InitTouch
*	����˵��: ����STM32�ʹ�����صĿ��ߣ�Ƭѡ���������. SPI���ߵ�GPIO��bsp_spi_bus.c ��ͳһ���á�
*	��    ��:  ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void GT928_InitHard(void)
{
	uint16_t ver;
	GPIO_InitTypeDef GPIO_InitStructure;

	/* ��GPIOʱ�� */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOI, ENABLE);

	/* �Ƚ�INT�����ߡ�ʹGT928���������� */
	GPIOI->BSRRL = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;		/* ��Ϊ����� */
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;		/* ��Ϊ����ģʽ */
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;	/* ���������費ʹ�� */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	/* IO������ٶ� */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_Init(GPIOI, &GPIO_InitStructure);
	bsp_DelayUS(2000);			/* ����2ms���廽�� */
	GPIOI->BSRRH = GPIO_Pin_3;	/* INT�����͡�ʹGT928���������� */
	bsp_DelayUS(200);
	
	/* �ٽ�INT��������Ϊ����ģʽ */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;		/* ��Ϊ����� */
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;		/* ��Ϊ����ģʽ */
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;	/* ���������費ʹ�� */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	/* IO������ٶ� */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_Init(GPIOI, &GPIO_InitStructure);
	
	g_GT928.TimerCount = 0;
	
	ver = GT928_ReadVersion();	/* �����ð汾 */

	printf("GT928 Version : %04X\r\n", ver);

	/* I2C���߳�ʼ���� bsp.c ��ִ�� */
	g_GT928.Enable = 1;
}

/*
*********************************************************************************************************
*	�� �� ��: GT928_ReadVersion
*	����˵��: ���GT811��оƬ�汾
*	��    ��: ��
*	�� �� ֵ: 16λ�汾
*********************************************************************************************************
*/
static uint16_t GT928_ReadVersion(void)
{
	uint8_t buf[1];

	GT928_ReadReg(0x8047, buf, 1);

	return buf[0];
}


/*
*********************************************************************************************************
*	�� �� ��: GT928_WriteReg
*	����˵��: д1���������Ķ���Ĵ���
*	��    ��: _usRegAddr : �Ĵ�����ַ
*			  _pRegBuf : �Ĵ������ݻ�����
*			 _ucLen : ���ݳ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void GT928_WriteReg(uint16_t _usRegAddr, uint8_t *_pRegBuf, uint8_t _ucLen)
{
	uint8_t i;

    i2c_Start();					/* ���߿�ʼ�ź� */

    i2c_SendByte(GT928_I2C_ADDR);	/* �����豸��ַ+д�ź� */
	i2c_WaitAck();

    i2c_SendByte(_usRegAddr >> 8);	/* ��ַ��8λ */
	i2c_WaitAck();

    i2c_SendByte(_usRegAddr);		/* ��ַ��8λ */
	i2c_WaitAck();

	for (i = 0; i < _ucLen; i++)
	{
	    i2c_SendByte(_pRegBuf[i]);		/* �Ĵ������� */
		i2c_WaitAck();
	}

    i2c_Stop();                   			/* ����ֹͣ�ź� */
}

/*
*********************************************************************************************************
*	�� �� ��: GT928_ReadReg
*	����˵��: д1���������Ķ���Ĵ���
*	��    ��: _usRegAddr : �Ĵ�����ַ
*			  _pRegBuf : �Ĵ������ݻ�����
*			 _ucLen : ���ݳ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void GT928_ReadReg(uint16_t _usRegAddr, uint8_t *_pRegBuf, uint8_t _ucLen)
{
	uint8_t i;

    i2c_Start();					/* ���߿�ʼ�ź� */

    i2c_SendByte(GT928_I2C_ADDR);	/* �����豸��ַ+д�ź� */
	i2c_WaitAck();

    i2c_SendByte(_usRegAddr >> 8);	/* ��ַ��8λ */
	i2c_WaitAck();

    i2c_SendByte(_usRegAddr);		/* ��ַ��8λ */
	i2c_WaitAck();

	i2c_Start();
    i2c_SendByte(GT928_I2C_ADDR + 0x01);	/* �����豸��ַ+���ź� */
	i2c_WaitAck();

	for (i = 0; i < _ucLen - 1; i++)
	{
	    _pRegBuf[i] = i2c_ReadByte();	/* ���Ĵ������� */
		i2c_Ack();
	}

	/* ���һ������ */
	 _pRegBuf[i] = i2c_ReadByte();		/* ���Ĵ������� */
	i2c_NAck();

    i2c_Stop();							/* ����ֹͣ�ź� */
}

/*
*********************************************************************************************************
*	�� �� ��: GT928_Timer1ms
*	����˵��: ÿ��1ms����1��
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void GT928_Timer1ms(void)
{
	g_GT928.TimerCount++;
}

/*
*********************************************************************************************************
*	�� �� ��: GT928_Scan
*	����˵��: ��ȡGT811�������ݡ���ȡȫ�������ݣ���Ҫ 720us���ҡ�
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void GT928_Scan(void)
{
	uint8_t buf[108];
	uint8_t i;
	static uint8_t s_tp_down = 0;
	uint16_t x, y;
	static uint16_t x_save, y_save;
	
	uint8_t finger = 0, touch_num = 0;

	if (g_GT928.Enable == 0)
	{
		return;
	}
	
	/* 20ms ִ��һ�� */
	if (g_GT928.TimerCount < 20)
	{
		return;
	}

	g_GT928.TimerCount = 0;
	
/*	
	���ݴ������ֲ� P14
			BIT7	  BIT6		BIT5			BIT4			BIT3 - BIT0
	x814E   buffer   status   large detect   Reserved HaveKey   numberof touchpoint
*/
	GT928_ReadReg(GT928_READ_XY_REG, (uint8_t *)buf, 1);		/* ����������ؼĴ��� */
	
	finger = buf[0];
	touch_num = finger & 0x0f;		/* �õ����������� */
	
	if ((finger & 0x80) == 0)		/* 0x80��ʾû�д����� */
	{
		return;
	}
	else							/* �д���������Ҫ�� buffer��BIT7����0�������´β����ٶ��üĴ��� */
	{
		buf[0] = 0;
		GT928_WriteReg(GT928_READ_XY_REG, (uint8_t *)buf, 1);
	}
	
	if (touch_num == 0)
	{
		if (s_tp_down == 1)
		{
			s_tp_down = 0;
			TOUCH_PutKey(TOUCH_RELEASE, x_save, y_save);
		}
		return;
	}

	GT928_ReadReg(GT928_READ_XY_REG, &buf[0], 82);
	
	/*
	0x814E	buffer status	large detect	Reserved	HaveKey		num of touch point
	0x814F	���id
	0x8150  ������ 1��X ����� 8 λ
	0x8151  ������ 1��X ����� 8 λ
	0x8152  ������ 1��Y ����� 8 λ
	0x8153  ������ 1��Y ����� 8 λ
	0x8154	������1�ߴ� ��8λ
	0x8155	������1�ߴ� ��8λ
	
	10�������㣺8 x 10 + 1������״̬�Ĵ��� + 1��KeyValue�Ĵ��� = 82���Ĵ���
	*/
	
	g_GT928.TouchReg = buf[0];		/* ����״̬�Ĵ��� */

	for (i = 0; i < 10; i++)
	{
		g_GT928.Id[i] = buf[i * 8 + 1];		/* ���ID */
		g_GT928.X[i] = ((uint16_t)buf[i * 8 + 3] << 8) + buf[i * 8 + 2];		/* ������ X���� */
		g_GT928.Y[i] = ((uint16_t)buf[i * 8 + 5] << 8) + buf[i * 8 + 4];		/* ������ Y���� */
		g_GT928.Size[i] = ((uint16_t)buf[i * 8 + 7] << 8) + buf[i * 8 + 6];		/* ������ �ߴ� */
	}

	/* ��ⰴ�� */
	{
		/* 
		���꣺
			(���Ͻ��� (0��0), ���½��� (1023��599)
		*/		
		x = g_GT928.X[0];
		y = g_GT928.Y[0];
	}
	
	if (s_tp_down == 0)
	{
		s_tp_down = 1;
		
		TOUCH_PutKey(TOUCH_DOWN, x, y);
	}
	else
	{
		TOUCH_PutKey(TOUCH_MOVE, x, y);
	}
	x_save = x;	/* �������꣬�����ͷ��¼� */
	y_save = y;

#if 0
	for (i = 0; i < 82; i++)
	{
		static uint8_t oldbus[108];
		if (buf[i] != oldbus[i])
		{
			//printf("[%d]:  %02X    ", i, buf[i]);
			oldbus[i] = buf[i];
		}
	}
	
	/* ��ӡ���� */
	for (i = 0; i < 82; i++)
	{
		printf("%02X ", buf[i]);
	}
	printf("\r\n");

	printf("(%5d,%5d,%5d) ",   g_GT928.Id[0], g_GT928.X[0], g_GT928.Y[0]);
//	printf("(%5d,%5d,%3d) ",  g_GT928.X1, g_GT928.Y1, g_GT928.P1);
//	printf("(%5d,%5d,%3d) ",  g_GT928.X2, g_GT928.Y2, g_GT928.P2);
//	printf("(%5d,%5d,%3d) ",  g_GT928.X3, g_GT928.Y3, g_GT928.P3);
//	printf("(%5d,%5d,%3d) ",  x, y, g_GT928.P4);
	printf("\r\n");
#endif	
}

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
