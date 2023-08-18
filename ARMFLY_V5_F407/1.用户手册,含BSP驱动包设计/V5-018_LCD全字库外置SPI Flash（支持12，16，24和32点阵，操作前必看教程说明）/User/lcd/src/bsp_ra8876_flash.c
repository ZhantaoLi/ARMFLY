/*
*********************************************************************************************************
*
*	模块名称 : RA8876芯片外挂的串行Flash驱动模块
*	文件名称 : bsp_ra8876_flash.c
*	版    本 : V1.0
*	说    明 : 访问RA8875外挂的串行Flash （字库芯片和图库芯片），支持 SST25VF016B、MX25L1606E 和
*			   W25Q64BVSSIG。 通过TFT显示接口中SPI总线和PWM口线控制7寸新屏上的串行Flash。
*				【备注： RA8875本身不支持外挂串行Flash的写操作，必须增加额外的电子开关电路才能实现】
*	修改记录 :
*		版本号  日期       作者    说明
*		V1.0    2012-06-25 armfly  发布首版
*
*	Copyright (C), 2015-2020, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/
#include "bsp.h"

#define CMD_AAI       0xAD  	/* AAI 连续编程指令(FOR SST25VF016B) */
#define CMD_DISWR	  0x04		/* 禁止写, 退出AAI状态 */
#define CMD_EWRSR	  0x50		/* 允许写状态寄存器的命令 */
#define CMD_WRSR      0x01  	/* 写状态寄存器命令 */
#define CMD_WREN      0x06		/* 写使能命令 */
#define CMD_READ      0x03  	/* 读数据区命令 */
#define CMD_RDSR      0x05		/* 读状态寄存器命令 */
#define CMD_RDID      0x9F		/* 读器件ID命令 */
#define CMD_SE        0x20		/* 擦除扇区命令 */
#define CMD_BE        0xC7		/* 批量擦除命令 */
#define DUMMY_BYTE    0xA5		/* 哑命令，可以为任意值，用于读操作 */

#define WIP_FLAG      0x01		/* 状态寄存器中的正在编程标志（WIP) */

/*
	PWM口线置低选中
	PWM = 1  这个模式支持STM32读写RA8875外挂的串行Flash
	PWM = 0 这是正常工作模式，由RA8875 DMA读取外挂的串行Flash
*/
#define	RCC_CS			(RCC_AHB1Periph_GPIOI | RCC_AHB1Periph_GPIOF)

/* 片选1  */
#define W25_CS1_GPIO	GPIOI
#define W25_CS1_PIN		GPIO_Pin_10

/* 片选2 */
#define PWM_CS2_GPIO	GPIOF
#define PWM_CS2_PIN		GPIO_Pin_6

/* 设置片选1 */
#define W25_CS1_0()     W25_CS1_GPIO->BSRRH = W25_CS1_PIN
#define W25_CS1_1()     W25_CS1_GPIO->BSRRL = W25_CS1_PIN

/* 设置片选2 */
#define PWM_CS2_0()     PWM_CS2_GPIO->BSRRH = PWM_CS2_PIN
#define PWM_CS2_1()     PWM_CS2_GPIO->BSRRL = PWM_CS2_PIN

static void RA8876_W25_SetCS(uint8_t _Value);
static uint32_t RA8876_w25_ReadID(void);
static void RA8876_w25_WaitForWriteEnd(void);
static void RA8876_w25_WriteStatus(uint8_t _ucValue);
static void RA8876_w25_ConfigGPIO(void);
static void RA8876_w25_WriteEnable(void);
static void RA8876_w25_ReadInfo(void);

uint8_t s_Ra8876CS = 0;			/* RA8876片选 1表示选中第一片spi flash， 2表示选中第二片spi flash */

/*
*********************************************************************************************************
*	函 数 名: bsp_InitRA8876Flash
*	功能说明: 初始化串行Flash硬件接口（配置STM32的SPI时钟、GPIO)
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_InitRA8876Flash(void)
{
	RA8876_w25_ConfigGPIO();	/* 1.配置片选1、2 */
	/* 配置SPI硬件参数用于访问串行Flash */
	bsp_CfgSPIForW25();			/* 2.配置SPI引脚功能 */

	/* 识别串行FLASH型号 */
	RA8876_w25_CtrlByMCU();		/* (必须先执行RA8876_w25_CtrlByMCU()切换SPI控制权)  */
	bsp_DelayMS(10);			/* 2016-04-24 发现部分显示模块不延迟，导致串行Flash ID识别错误 */
	s_Ra8876CS = 1;				/* 默认为第一片spi flash */
	RA8876_w25_ReadInfo();		/* 自动识别芯片型号,RA8876有两片串行flash 所以片选有两种 */
	RA8876_w25_CtrlByRA8876();	/* 将flash控制权交给RA8876 */
}

/*
*********************************************************************************************************
*	函 数 名: RA8876_w25_ConfigGPIO
*	功能说明: 配置GPIO。 不包括 SCK  MOSI  MISO 共享的SPI总线。
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void RA8876_w25_ConfigGPIO(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* 使能GPIO 时钟 */
	RCC_AHB1PeriphClockCmd(RCC_CS, ENABLE);

	/* 配置片选1口线为推挽输出模式 */
	W25_CS1_1();		/* 片选置高，不选中 */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Pin = W25_CS1_PIN;
	GPIO_Init(W25_CS1_GPIO, &GPIO_InitStructure);
	
	/* 配置片选2口线为推挽输出模式 */
	PWM_CS2_1();
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Pin = PWM_CS2_PIN;
	GPIO_Init(PWM_CS2_GPIO, &GPIO_InitStructure);
}

/*
*********************************************************************************************************
*	函 数 名: RA8876_w25_CtrlByMCU
*	功能说明: 串行Flash控制权交给MCU （STM32）
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
void RA8876_w25_CtrlByMCU(void)
{
	uint8_t temp;

	/* RA8876 GPIOD[3] = 0 */
	RA8876_CmdWrite(0xF6);
	temp = RA8876_DataRead();
	temp &= cClrb3;
	RA8876_DataWrite(temp);
	
	/* RA8876 锁 SPI 功能 */
	Disable_SFlash_SPI();
}

/*
*********************************************************************************************************
*	函 数 名: w25_CtrlByRA8875
*	功能说明: 串行Flash控制权交给RA8875
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
void RA8876_w25_CtrlByRA8876(void)
{
	uint8_t temp;

	/* RA8876 GPIOD[3] = 0 */
	RA8876_CmdWrite(0xF6);
	temp = RA8876_DataRead();
	temp |= cSetb3;
	RA8876_DataWrite(temp);
	
	/* RA8876 致能 SPI 功能 */
	Enable_SFlash_SPI();
}


/*
*********************************************************************************************************
*	函 数 名: RA8876_W25_CS_1
*	功能说明: RA8876两片Flash都不选中
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void RA8876_W25_SetCS(uint8_t _Value)
{
	if (_Value == 0)			/* 选中其中一个片选 */
	{
		if (s_Ra8876CS == 1)			/* 选中第一片flash */
		{
			W25_CS1_0();
			PWM_CS2_1();	
		}
		else							/* 选中第二片flash */
		{
			W25_CS1_1();
			PWM_CS2_0();	
		}
	}
	else if (_Value == 1)		/* 片选全部置1 */
	{
		W25_CS1_1();
		PWM_CS2_1();	
	}
}

/*
*********************************************************************************************************
*	函 数 名: RA8876_w25_SelectChip
*	功能说明: 选择即将操作的芯片
*	形    参: _idex = FONT_CHIP 表示字库芯片;  idex = BMP_CHIP 表示图库芯片
*	返 回 值: 无
*********************************************************************************************************
*/
void RA8876_w25_SelectChip(uint8_t _idex)
{
/* 	
	写字库，片选选中 串行flash1
	写图库，片选选中 串行flash2
*/	
	if (_idex == FONT_CHIP)
	{
		/* 设置为第1个spi flash */
		s_Ra8876CS = 1;
	}
	else	/* BMP图片芯片 */
	{
		/* 设置为第2个spi flash */	
		s_Ra8876CS = 2;
	}

	RA8876_w25_ReadInfo();		/* 自动识别芯片型号 */
	
	RA8876_W25_SetCS(0);				/* 软件方式，使能串行Flash片选 */
	bsp_spiWrite1(CMD_DISWR);			/* 发送禁止写入的命令,即使能软件写保护 */
	RA8876_W25_SetCS(1);					/* 软件方式，禁能串行Flash片选 */

	RA8876_w25_WaitForWriteEnd();		/* 等待串行Flash内部操作完成 */

	RA8876_w25_WriteStatus(0);			/* 解除所有BLOCK的写保护 */
}

/*
*********************************************************************************************************
*	函 数 名: RA8876_w25_ReadInfo
*	功能说明: 读取器件ID,并填充器件参数
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
static void RA8876_w25_ReadInfo(void)
{
	/* 自动识别串行Flash型号 */
	{
		g_tW25.ChipID = RA8876_w25_ReadID();	/* 芯片ID */

		switch (g_tW25.ChipID)
		{
			case SST25VF016B:
				g_tW25.TotalSize = 2 * 1024 * 1024;	/* 总容量 = 2M */
				g_tW25.PageSize = 4 * 1024;			/* 页面大小 = 4K */
				break;

			case MX25L1606E:
				g_tW25.TotalSize = 2 * 1024 * 1024;	/* 总容量 = 2M */
				g_tW25.PageSize = 4 * 1024;			/* 页面大小 = 4K */
				break;

			case W25Q64BV:
				g_tW25.TotalSize = 8 * 1024 * 1024;	/* 总容量 = 8M */
				g_tW25.PageSize = 4 * 1024;			/* 页面大小 = 4K */
				break;

			case W25Q128:
				g_tW25.TotalSize = 16 * 1024 * 1024;	/* 总容量 = 16M */
				g_tW25.PageSize = 4 * 1024;			/* 页面大小 = 4K */
				break;

			default:		/* 集通字库不支持ID读取 */
				g_tW25.TotalSize = 2 * 1024 * 1024;
				g_tW25.PageSize = 4 * 1024;
				break;
		}
	}
}

/*
*********************************************************************************************************
*	函 数 名: w25_WriteEnable
*	功能说明: 向器件发送写使能命令
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
static void RA8876_w25_WriteEnable(void)
{
	RA8876_W25_SetCS(0);									/* 使能片选 */
	bsp_spiWrite1(CMD_WREN);								/* 发送命令 */
	RA8876_W25_SetCS(1);									/* 禁能片选 */
}

/*
*********************************************************************************************************
*	函 数 名: RA8876_w25_ReadID
*	功能说明: 读取器件ID
*	形    参:  无
*	返 回 值: 32bit的器件ID (最高8bit填0，有效ID位数为24bit）
*********************************************************************************************************
*/
static uint32_t RA8876_w25_ReadID(void)
{
	uint32_t uiID;
	uint8_t id1, id2, id3;

	RA8876_W25_SetCS(0);						/* 选中相应的spi flash */
	bsp_spiWrite1(CMD_RDID);				/* 发送读ID命令 */
	id1 = bsp_spiRead1();					/* 读ID的第1个字节 */
	id2 = bsp_spiRead1();					/* 读ID的第2个字节 */
	id3 = bsp_spiRead1();					/* 读ID的第3个字节 */
	RA8876_W25_SetCS(1);						/* 选中相应的spi flash */
	
	uiID = ((uint32_t)id1 << 16) | ((uint32_t)id2 << 8) | id3;

	return uiID;
}

/*
*********************************************************************************************************
*	函 数 名: RA8876_w25_WaitForWriteEnd
*	功能说明: 采用循环查询的方式等待器件内部写操作完成
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
static void RA8876_w25_WaitForWriteEnd()
{
	RA8876_W25_SetCS(0);								/* 使能片选_cs */
	bsp_spiWrite1(CMD_RDSR);						/* 发送命令， 读状态寄存器 */
	while((bsp_spiRead1() & WIP_FLAG) == SET);		/* 判断状态寄存器的忙标志位 */
	RA8876_W25_SetCS(1);									/* 禁能片选 */
}

/*
*********************************************************************************************************
*	函 数 名: RA8876_w25_WriteStatus
*	功能说明: 写状态寄存器
*	形    参:  _ucValue : 状态寄存器的值
*	返 回 值: 无
*********************************************************************************************************
*/
static void RA8876_w25_WriteStatus(uint8_t _ucValue)
{

	if (g_tW25.ChipID == SST25VF016B)
	{
		/* 第1步：先使能写状态寄存器 */
		RA8876_W25_SetCS(0);									/* 使能片选_cs */
		bsp_spiWrite1(CMD_EWRSR);							/* 发送命令， 允许写状态寄存器 */
		RA8876_W25_SetCS(1);										/* 禁能片选 */

		/* 第2步：再写状态寄存器 */
		RA8876_W25_SetCS(0);								/* 使能片选 */
		bsp_spiWrite1(CMD_WRSR);							/* 发送命令， 写状态寄存器 */
		bsp_spiWrite1(_ucValue);							/* 发送数据：状态寄存器的值 */
		RA8876_W25_SetCS(1);											/* 禁能片选 */
	}
	else
	{
		RA8876_W25_SetCS(0);										/* 使能片选 */
		bsp_spiWrite1(CMD_WRSR);							/* 发送命令， 写状态寄存器 */
		bsp_spiWrite1(_ucValue);							/* 发送数据：状态寄存器的值 */
		RA8876_W25_SetCS(1);									/* 禁能片选 */
	}
}

/*
*********************************************************************************************************
*	函 数 名: w25_EraseSector
*	功能说明: 擦除指定的扇区
*	形    参:  _uiSectorAddr : 扇区地址
*	返 回 值: 无
*********************************************************************************************************
*/
void RA8876_w25_EraseSector(uint32_t _uiSectorAddr)
{
	RA8876_w25_WriteEnable();								/* 发送写使能命令 */

	/* 擦除扇区操作 */
	RA8876_W25_SetCS(0);									/* 使能片选 */
	bsp_spiWrite1(CMD_SE);								/* 发送擦除命令 */
	bsp_spiWrite1((_uiSectorAddr & 0xFF0000) >> 16);	/* 发送扇区地址的高8bit */
	bsp_spiWrite1((_uiSectorAddr & 0xFF00) >> 8);		/* 发送扇区地址中间8bit */
	bsp_spiWrite1(_uiSectorAddr & 0xFF);				/* 发送扇区地址低8bit */
	RA8876_W25_SetCS(1);									/* 禁能片选 */

	RA8876_w25_WaitForWriteEnd();							/* 等待串行Flash内部写操作完成 */
}

/*
*********************************************************************************************************
*	函 数 名: w25_EraseChip
*	功能说明: 擦除整个芯片
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
void RA8876_w25_EraseChip(void)
{
	RA8876_w25_WriteEnable();								/* 发送写使能命令 */

	/* 擦除扇区操作 */
	RA8876_W25_SetCS(0);									/* 使能片选 */
	bsp_spiWrite1(CMD_BE);							/* 发送整片擦除命令 */
	RA8876_W25_SetCS(1);									/* 禁能片选 */

	RA8876_w25_WaitForWriteEnd();							/* 等待串行Flash内部写操作完成 */
}

/*
*********************************************************************************************************
*	函 数 名: RA8876_w25_WritePage
*	功能说明: 向一个page内写入若干字节。字节个数不能超出页面大小（4K)
*	形    参:  	_pBuf : 数据源缓冲区；
*				_uiWriteAddr ：目标区域首地址
*				_usSize ：数据个数，不能超过页面大小
*	返 回 值: 无
*********************************************************************************************************
*/
void RA8876_w25_WritePage(uint8_t * _pBuf, uint32_t _uiWriteAddr, uint16_t _usSize)
{
	uint32_t i, j;

	if (g_tW25.ChipID == SST25VF016B)
	{
		/* AAI指令要求传入的数据个数是偶数 */
		if ((_usSize < 2) && (_usSize % 2))
		{
			return ;
		}

		RA8876_w25_WriteEnable();								/* 发送写使能命令 */

		RA8876_W25_SetCS(0);									/* 使能片选 */
		bsp_spiWrite1(CMD_AAI);							/* 发送AAI命令(地址自动增加编程) */
		bsp_spiWrite1((_uiWriteAddr & 0xFF0000) >> 16);	/* 发送扇区地址的高8bit */
		bsp_spiWrite1((_uiWriteAddr & 0xFF00) >> 8);		/* 发送扇区地址中间8bit */
		bsp_spiWrite1(_uiWriteAddr & 0xFF);				/* 发送扇区地址低8bit */
		bsp_spiWrite1(*_pBuf++);							/* 发送第1个数据 */
		bsp_spiWrite1(*_pBuf++);							/* 发送第2个数据 */
		RA8876_W25_SetCS(1);									/* 禁能片选 */

		RA8876_w25_WaitForWriteEnd();							/* 等待串行Flash内部写操作完成 */

		_usSize -= 2;									/* 计算剩余字节数 */

		for (i = 0; i < _usSize / 2; i++)
		{
			RA8876_W25_SetCS(0);								/* 使能片选 */
			bsp_spiWrite1(CMD_AAI);						/* 发送AAI命令(地址自动增加编程) */
			bsp_spiWrite1(*_pBuf++);						/* 发送数据 */
			bsp_spiWrite1(*_pBuf++);						/* 发送数据 */
			RA8876_W25_SetCS(1);								/* 禁能片选 */
			RA8876_w25_WaitForWriteEnd();						/* 等待串行Flash内部写操作完成 */
		}

		/* 进入写保护状态 */
		RA8876_W25_SetCS(0);
		bsp_spiWrite1(CMD_DISWR);
		RA8876_W25_SetCS(1);

		RA8876_w25_WaitForWriteEnd();							/* 等待串行Flash内部写操作完成 */
	}
	else	/* for MX25L1606E 、 W25Q64BV */
	{
		for (j = 0; j < _usSize / 256; j++)
		{
			RA8876_w25_WriteEnable();								/* 发送写使能命令 */

			RA8876_W25_SetCS(0);									/* 使能片选 */
			bsp_spiWrite1(0x02);								/* 发送AAI命令(地址自动增加编程) */
			bsp_spiWrite1((_uiWriteAddr & 0xFF0000) >> 16);	/* 发送扇区地址的高8bit */
			bsp_spiWrite1((_uiWriteAddr & 0xFF00) >> 8);		/* 发送扇区地址中间8bit */
			bsp_spiWrite1(_uiWriteAddr & 0xFF);				/* 发送扇区地址低8bit */

			for (i = 0; i < 256; i++)
			{
				bsp_spiWrite1(*_pBuf++);					/* 发送数据 */
			}

			RA8876_W25_SetCS(1);								/* 禁止片选 */

			RA8876_w25_WaitForWriteEnd();						/* 等待串行Flash内部写操作完成 */

			_uiWriteAddr += 256;
		}

		/* 进入写保护状态 */
		RA8876_W25_SetCS(0);
		bsp_spiWrite1(CMD_DISWR);
		RA8876_W25_SetCS(1);

		RA8876_w25_WaitForWriteEnd();							/* 等待串行Flash内部写操作完成 */
	}
}

/*
*********************************************************************************************************
*	函 数 名: RA8876_w25_ReadBuffer
*	功能说明: 连续读取若干字节。字节个数不能超出芯片容量。
*	形    参:  	_pBuf : 数据源缓冲区；
*				_uiReadAddr ：首地址
*				_usSize ：数据个数, 可以大于PAGE_SIZE,但是不能超出芯片总容量
*	返 回 值: 无
*********************************************************************************************************
*/
void RA8876_w25_ReadBuffer(uint8_t * _pBuf, uint32_t _uiReadAddr, uint32_t _uiSize)
{
	/* 如果读取的数据长度为0或者超出串行Flash地址空间，则直接返回 */
	if ((_uiSize == 0) ||(_uiReadAddr + _uiSize) > g_tW25.TotalSize)
	{
		return;
	}

	/* 擦除扇区操作 */
	RA8876_W25_SetCS(0);									/* 使能片选 */
	bsp_spiWrite1(CMD_READ);							/* 发送读命令 */
	bsp_spiWrite1((_uiReadAddr & 0xFF0000) >> 16);	/* 发送扇区地址的高8bit */
	bsp_spiWrite1((_uiReadAddr & 0xFF00) >> 8);		/* 发送扇区地址中间8bit */
	bsp_spiWrite1(_uiReadAddr & 0xFF);				/* 发送扇区地址低8bit */
	while (_uiSize--)
	{
		*_pBuf++ = bsp_spiRead1();			/* 读一个字节并存储到pBuf，读完后指针自加1 */
	}
	RA8876_W25_SetCS(1);									/* 禁能片选 */
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
