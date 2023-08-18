/*
*********************************************************************************************************
*
*	模块名称 : 电容触摸芯片GT928驱动程序
*	文件名称 : bsp_ct928.c
*	版    本 : V1.0
*	说    明 : GT928触摸芯片驱动程序。
*	修改记录 :
*		版本号   日期        作者     说明
*		V1.0    2016-07-18  armfly   正式发布
*
*	Copyright (C), 2015-2020, 安富莱电子 www.armfly.com
*********************************************************************************************************
*/
#include "bsp.h"

#if 0		/* 配置已被固化到GT928中 */
/* gt928 cfg */
uint8_t s_GT928_CfgParams[] = {
	/* 
	0x8047 R/W  配置文件版本号
	*/
	0x41,
	/* 
	0x8048 R/W  X_Ou_Max_L X 坐标输出最大值  600
	0x8049 R/W  X_Ou_Max_H
	*/
	0x58,0x02,	//0x00,0x04,//0x00,0x10,
	/*
	0x804A R/W  Y_Ou_Max_L  Y 坐标输出最大值  1024
	0x804B R/W  Y_Ou_Max_H
	*/
	0x00,0x04,//0x58,0x02,//0x00,0x10,
	/*
	0x804C R/W  输出触点个数上限 5
	*/
	0x0A,
	/*
	0x804D R/W Module_Switch1 Reserved Stretch_rank X2Y Sito INT触发方式 
	*/
	0x7D,
	/*
	0x804E R/W Module_Switch2 Touch_key
	*/
	0x00,
	/*
	0x804F  手指按下/松开去抖次数
	*/
	0x02,
	/* 
	0x8050	Filter  原始坐标窗口滤波值，系数为1
	*/
	0x08,
	/*
	0x8051  Large_Touch	大面积触摸点个数
	*/
	0x19,
	/*
	0x8052	Noise_Reduction	噪声消除值，系数为1
	*/
	0x0D,
	/* 
	0x8053	屏上触摸点从无到有的阈值 
	*/
	0x32,
	/*
	0x8054	屏上触摸点从有到无的阈值
	*/
	0x28,
	/*
	0x8055	进低功耗时间（0-15s）
	*/
	0x03,
	/*
	0x8056	坐标上报率（周期为5+Nms）
	*/
	0x00,
	/* 0x8057 - 0x8058 保留 */
	0x00,0x00,
	/*
	0x8059	速度限制参数
	*/
	0x00,0x00,
	/*
	0x805B Space 周围框的空白区
	*/
	0x00,0x00,	
	/*
	0x805D - 0x8061 保留
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

#define GT928_READ_XY_REG	0x814E		/* 坐标寄存器 */
#define GT928_CONFIG_REG	0x8047		/* 配置参数寄存器 */

static void GT928_WriteReg(uint16_t _usRegAddr, uint8_t *_pRegBuf, uint8_t _ucLen);
static void GT928_ReadReg(uint16_t _usRegAddr, uint8_t *_pRegBuf, uint8_t _ucLen);
static uint16_t GT928_ReadVersion(void);

GT928_T g_GT928;

/*
*********************************************************************************************************
*	函 数 名: bsp_InitTouch
*	功能说明: 配置STM32和触摸相关的口线，片选由软件控制. SPI总线的GPIO在bsp_spi_bus.c 中统一配置。
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
void GT928_InitHard(void)
{
	uint16_t ver;
	GPIO_InitTypeDef GPIO_InitStructure;

	/* 打开GPIO时钟 */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOI, ENABLE);

	/* 先将INT脚拉高。使GT928能正常工作 */
	GPIOI->BSRRL = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;		/* 设为输出口 */
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;		/* 设为推挽模式 */
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;	/* 上下拉电阻不使能 */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	/* IO口最大速度 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_Init(GPIOI, &GPIO_InitStructure);
	bsp_DelayUS(2000);			/* 产生2ms脉冲唤醒 */
	GPIOI->BSRRH = GPIO_Pin_3;	/* INT脚拉低。使GT928能正常工作 */
	bsp_DelayUS(200);
	
	/* 再将INT引脚配置为输入模式 */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;		/* 设为输出口 */
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;		/* 设为推挽模式 */
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;	/* 上下拉电阻不使能 */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	/* IO口最大速度 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_Init(GPIOI, &GPIO_InitStructure);
	
	g_GT928.TimerCount = 0;
	
	ver = GT928_ReadVersion();	/* 读配置版本 */

	printf("GT928 Version : %04X\r\n", ver);

	/* I2C总线初始化在 bsp.c 中执行 */
	g_GT928.Enable = 1;
}

/*
*********************************************************************************************************
*	函 数 名: GT928_ReadVersion
*	功能说明: 获得GT811的芯片版本
*	形    参: 无
*	返 回 值: 16位版本
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
*	函 数 名: GT928_WriteReg
*	功能说明: 写1个或连续的多个寄存器
*	形    参: _usRegAddr : 寄存器地址
*			  _pRegBuf : 寄存器数据缓冲区
*			 _ucLen : 数据长度
*	返 回 值: 无
*********************************************************************************************************
*/
static void GT928_WriteReg(uint16_t _usRegAddr, uint8_t *_pRegBuf, uint8_t _ucLen)
{
	uint8_t i;

    i2c_Start();					/* 总线开始信号 */

    i2c_SendByte(GT928_I2C_ADDR);	/* 发送设备地址+写信号 */
	i2c_WaitAck();

    i2c_SendByte(_usRegAddr >> 8);	/* 地址高8位 */
	i2c_WaitAck();

    i2c_SendByte(_usRegAddr);		/* 地址低8位 */
	i2c_WaitAck();

	for (i = 0; i < _ucLen; i++)
	{
	    i2c_SendByte(_pRegBuf[i]);		/* 寄存器数据 */
		i2c_WaitAck();
	}

    i2c_Stop();                   			/* 总线停止信号 */
}

/*
*********************************************************************************************************
*	函 数 名: GT928_ReadReg
*	功能说明: 写1个或连续的多个寄存器
*	形    参: _usRegAddr : 寄存器地址
*			  _pRegBuf : 寄存器数据缓冲区
*			 _ucLen : 数据长度
*	返 回 值: 无
*********************************************************************************************************
*/
static void GT928_ReadReg(uint16_t _usRegAddr, uint8_t *_pRegBuf, uint8_t _ucLen)
{
	uint8_t i;

    i2c_Start();					/* 总线开始信号 */

    i2c_SendByte(GT928_I2C_ADDR);	/* 发送设备地址+写信号 */
	i2c_WaitAck();

    i2c_SendByte(_usRegAddr >> 8);	/* 地址高8位 */
	i2c_WaitAck();

    i2c_SendByte(_usRegAddr);		/* 地址低8位 */
	i2c_WaitAck();

	i2c_Start();
    i2c_SendByte(GT928_I2C_ADDR + 0x01);	/* 发送设备地址+读信号 */
	i2c_WaitAck();

	for (i = 0; i < _ucLen - 1; i++)
	{
	    _pRegBuf[i] = i2c_ReadByte();	/* 读寄存器数据 */
		i2c_Ack();
	}

	/* 最后一个数据 */
	 _pRegBuf[i] = i2c_ReadByte();		/* 读寄存器数据 */
	i2c_NAck();

    i2c_Stop();							/* 总线停止信号 */
}

/*
*********************************************************************************************************
*	函 数 名: GT928_Timer1ms
*	功能说明: 每隔1ms调用1次
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void GT928_Timer1ms(void)
{
	g_GT928.TimerCount++;
}

/*
*********************************************************************************************************
*	函 数 名: GT928_Scan
*	功能说明: 读取GT811触摸数据。读取全部的数据，需要 720us左右。
*	形    参: 无
*	返 回 值: 无
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
	
	/* 20ms 执行一次 */
	if (g_GT928.TimerCount < 20)
	{
		return;
	}

	g_GT928.TimerCount = 0;
	
/*	
	电容触摸板手册 P14
			BIT7	  BIT6		BIT5			BIT4			BIT3 - BIT0
	x814E   buffer   status   large detect   Reserved HaveKey   numberof touchpoint
*/
	GT928_ReadReg(GT928_READ_XY_REG, (uint8_t *)buf, 1);		/* 读触摸板相关寄存器 */
	
	finger = buf[0];
	touch_num = finger & 0x0f;		/* 得到触摸点数量 */
	
	if ((finger & 0x80) == 0)		/* 0x80表示没有触摸键 */
	{
		return;
	}
	else							/* 有触摸键，则要把 buffer（BIT7）置0，这样下次才能再读该寄存器 */
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
	0x814F	监测id
	0x8150  触摸点 1，X 坐标低 8 位
	0x8151  触摸点 1，X 坐标高 8 位
	0x8152  触摸点 1，Y 坐标低 8 位
	0x8153  触摸点 1，Y 坐标高 8 位
	0x8154	触摸点1尺寸 低8位
	0x8155	触摸点1尺寸 高8位
	
	10个触摸点：8 x 10 + 1个触摸状态寄存器 + 1个KeyValue寄存器 = 82个寄存器
	*/
	
	g_GT928.TouchReg = buf[0];		/* 触摸状态寄存器 */

	for (i = 0; i < 10; i++)
	{
		g_GT928.Id[i] = buf[i * 8 + 1];		/* 监测ID */
		g_GT928.X[i] = ((uint16_t)buf[i * 8 + 3] << 8) + buf[i * 8 + 2];		/* 触摸点 X坐标 */
		g_GT928.Y[i] = ((uint16_t)buf[i * 8 + 5] << 8) + buf[i * 8 + 4];		/* 触摸点 Y坐标 */
		g_GT928.Size[i] = ((uint16_t)buf[i * 8 + 7] << 8) + buf[i * 8 + 6];		/* 触摸点 尺寸 */
	}

	/* 检测按下 */
	{
		/* 
		坐标：
			(左上角是 (0，0), 右下角是 (1023，599)
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
	x_save = x;	/* 保存坐标，用于释放事件 */
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
	
	/* 打印测试 */
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

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
