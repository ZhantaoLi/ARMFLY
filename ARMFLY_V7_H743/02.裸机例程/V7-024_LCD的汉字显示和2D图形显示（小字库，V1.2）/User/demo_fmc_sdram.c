/*
*********************************************************************************************************
*
*	模块名称 : SDRAM性能测试文件
*	文件名称 : demo_fmc_sdram.c
*	版    本 : V1.0
*	说    明 : 外部32位带宽SDRAM性能测试。
*              1、SDRAM型号IS42S32800G-6BLI, 32位带宽, 容量32MB, 6ns速度(166MHz)。
*              2. K1键按下，测试32MB写速度;
*              3. K2键按下，测试32MB读速度;
*              4. K3键按下，读取1024字节并打印;
*              5. 摇杆OK键按下，测试SDRAM所有单元是否有异常;
*              6. 开启Cache
*                （1）使用MDK和IAR的各种优化等级测试，优化对其影响很小。
*                （2）写速度376MB/S，读速度182MB/S。
*              7. 关闭Cache
*                （1）使用MDK和IAR的各种优化等级测试，优化对其影响很小。
*                （2）写速度307MB/S，读速度116MB/S。
*              8. IAR开启最高等级优化，读速度是189MB/S，比MDK的182MB/S高点。
*              9. 对于MDK，本实验开启了最高等级优化和时间优化。
*              10. 对IAR，本实验开启了最高等级速度优化。
*	修改记录 :
*		版本号   日期         作者        说明
*		V1.0    2018-12-12   Eric2013     1. CMSIS软包版本 V5.4.0
*                                         2. HAL库版本 V1.3.0
*
*	Copyright (C), 2018-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/	
#include "bsp.h"			/* 底层硬件驱动 */



/* 供本文件内部调用函数 */
static void PrintfHelp(void);
static void WriteSpeedTest(void);
static void ReadSpeedTest(void);
static void ReadWriteTest(void);

#define TEST_ADDRESS	0    /* 从地址0开始测试 */
#define TEST_BUF_SIZE	256  /* 测试缓冲大小 */

/*
*********************************************************************************************************
*	函 数 名: DemoFmcSRAM
*	功能说明: SDRAM读写性能测试
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void DemoFmcSRAM(void)
{
	uint8_t ucKeyCode;	/* 按键代码 */
	uint32_t err;
	
	PrintfHelp();	/* 打印操作提示 */

	bsp_StartAutoTimer(0, 100);	/* 启动1个100ms的自动重装的定时器 */
	
	/* 进入主程序循环体 */
	while (1)
	{
		bsp_Idle();		/* 这个函数在bsp.c文件。用户可以修改这个函数实现CPU休眠和喂狗 */

		/* 判断定时器超时时间 */
		if (bsp_CheckTimer(0))	
		{
			/* 每隔100ms 进来一次 */  
			bsp_LedToggle(2);
		}

		/* 按键滤波和检测由后台systick中断服务程序实现，我们只需要调用bsp_GetKey读取键值即可。 */
		ucKeyCode = bsp_GetKey();	/* 读取键值, 无键按下时返回 KEY_NONE = 0 */
		if (ucKeyCode != KEY_NONE)
		{
			switch (ucKeyCode)
			{
				case KEY_DOWN_K1:			/* K1键按下，测试32MB写速度 */
					WriteSpeedTest();
					break;

				case KEY_DOWN_K2:			/* K2键按下，测试32MB读速度*/
					ReadSpeedTest();
					break;
				
				case KEY_DOWN_K3:			/* K3键按下，读取1024字节并打印 */
					ReadWriteTest();
					break;
				
				case JOY_DOWN_OK:			/* 摇杆OK键按下，测试SDRAM所有单元是否有异常*/
					err = bsp_TestExtSDRAM1();
					if (err == 0)
					{
						printf("外部SDRAM测试通过\r\n");
					}
					else
					{
						printf("外部SDRAM出错，错误单元个数：%d\r\n", err);
					}
					break;

				default:
					/* 其它的键值不处理 */
					break;
			}
		}
	}
}

/*
*********************************************************************************************************
*	函 数 名: WriteSpeedTest
*	功能说明: 写SDRAM速度测试
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void WriteSpeedTest(void)
{
	uint32_t start, end, cnt;
	uint32_t i, j;
	int32_t iTime1, iTime2;
	uint32_t *pBuf;


	/* 设置初始化值并记下开始时间 */
	j = 0;
	pBuf = (uint32_t *)EXT_SDRAM_ADDR;
	iTime1 = bsp_GetRunTime();	  
	start = DWT_CYCCNT;
	
	/* 以递增的方式写数据到SDRAM所有空间 */
	for (i = 1024*1024/4; i >0 ; i--)
	{
		*pBuf++ = j++;
		*pBuf++ = j++;
		*pBuf++ = j++;
		*pBuf++ = j++;
		*pBuf++ = j++;
		*pBuf++ = j++;
		*pBuf++ = j++;
		*pBuf++ = j++;	

		*pBuf++ = j++;
		*pBuf++ = j++;
		*pBuf++ = j++;
		*pBuf++ = j++;
		*pBuf++ = j++;
		*pBuf++ = j++;
		*pBuf++ = j++;
		*pBuf++ = j++;	

		*pBuf++ = j++;
		*pBuf++ = j++;
		*pBuf++ = j++;
		*pBuf++ = j++;
		*pBuf++ = j++;
		*pBuf++ = j++;
		*pBuf++ = j++;
		*pBuf++ = j++;	

		*pBuf++ = j++;
		*pBuf++ = j++;
		*pBuf++ = j++;
		*pBuf++ = j++;
		*pBuf++ = j++;
		*pBuf++ = j++;
		*pBuf++ = j++;
		*pBuf++ = j++;	
	}
	end = DWT_CYCCNT;
	cnt = end - start;
	iTime2 = bsp_GetRunTime();	/* 记下结束时间 */
	
    /* 读取写入的是否出错 */
	j = 0;
	pBuf = (uint32_t *)EXT_SDRAM_ADDR;
	for (i = 0; i < 1024*1024*8; i++)
	{
		if(*pBuf++ != j++)
		{
			printf("写入出错 j=%d\r\n", j);
			break;
		}
	}
		
	/* 打印速度 */
	printf("【32MB数据写耗时】: 方式一:%dms  方式二:%dms, 写速度: %dMB/s\r\n", 
	                  iTime2 - iTime1,  cnt/400000, (EXT_SDRAM_SIZE / 1024 /1024 * 1000) / (iTime2 - iTime1));
}

/*
*********************************************************************************************************
*	函 数 名: ReadSpeedTest
*	功能说明: 读SDRAM速度测试
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void ReadSpeedTest(void)
{	
	uint32_t start, end, cnt;
	uint32_t i;
	int32_t iTime1, iTime2;
	uint32_t *pBuf;
	__IO  uint32_t ulTemp; /* 设置为__IO类型，防止被MDK优化 */

	/* 设置初始化值并记下开始时间 */
	pBuf = (uint32_t *)EXT_SDRAM_ADDR;
	iTime1 = bsp_GetRunTime();	
	start = DWT_CYCCNT;
	
	/* 读取SDRAM所有空间数据 */	
	for (i = 1024*1024/4; i >0 ; i--)
	{
		ulTemp = *pBuf++;
		ulTemp = *pBuf++;
		ulTemp = *pBuf++;
		ulTemp = *pBuf++;
		ulTemp = *pBuf++;
		ulTemp = *pBuf++;
		ulTemp = *pBuf++;
		ulTemp = *pBuf++;

		ulTemp = *pBuf++;
		ulTemp = *pBuf++;
		ulTemp = *pBuf++;
		ulTemp = *pBuf++;
		ulTemp = *pBuf++;
		ulTemp = *pBuf++;
		ulTemp = *pBuf++;
		ulTemp = *pBuf++;
		
		ulTemp = *pBuf++;
		ulTemp = *pBuf++;
		ulTemp = *pBuf++;
		ulTemp = *pBuf++;
		ulTemp = *pBuf++;
		ulTemp = *pBuf++;
		ulTemp = *pBuf++;
		ulTemp = *pBuf++;
		
		ulTemp = *pBuf++;
		ulTemp = *pBuf++;
		ulTemp = *pBuf++;
		ulTemp = *pBuf++;
		ulTemp = *pBuf++;
		ulTemp = *pBuf++;
		ulTemp = *pBuf++;
		ulTemp = *pBuf++;
	}
	end = DWT_CYCCNT;
	cnt = end - start;
	iTime2 = bsp_GetRunTime();	/* 记下结束时间 */

	/* 打印速度 */
	printf("【32MB数据读耗时】: 方式一:%dms  方式二:%dms, 读速度: %dMB/s\r\n", 
	        iTime2 - iTime1,  cnt/400000, (EXT_SDRAM_SIZE / 1024 /1024 * 1000) / (iTime2 - iTime1));
}

/*
*********************************************************************************************************
*	函 数 名: ReadWriteTest
*	功能说明: 读数据，并打印出来
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void ReadWriteTest(void)
{
	uint32_t i;
	uint32_t *pBuf;
	

	/* 写入测试数据0xAAAA5555 */
	pBuf = (uint32_t *)(EXT_SDRAM_ADDR + TEST_ADDRESS);
	for (i = 0; i < TEST_BUF_SIZE; i++)		
	{
		pBuf[i] = 0xAAAA5555;
	}
	
	printf("物理地址：0x%08X  大小: %d字节  显示: %d字节  数据如下: \r\n", EXT_SDRAM_ADDR + TEST_ADDRESS, EXT_SDRAM_SIZE, TEST_BUF_SIZE*4);
	
	/* 打印数据 */
	pBuf = (uint32_t *)(EXT_SDRAM_ADDR + TEST_ADDRESS);
	for (i = 0; i < TEST_BUF_SIZE; i++)
	{
		printf(" %04X", pBuf[i]);

		if ((i & 7) == 7)
		{
			printf("\r\n");		/* 每行显示32字节数据 */
		}
		else if ((i & 7) == 3)
		{
			printf(" - ");
		}
	}
}

/*
*********************************************************************************************************
*	函 数 名: PrintfHelp
*	功能说明: 打印操作提示
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void PrintfHelp(void)
{
	printf("操作提示:\r\n");	
	printf("SDRAM型号IS42S32800G-6BLI, 32位带宽, 容量32MB, 6ns速度(166MHz)\r\n");
	printf("1. K1键按下，测试32MB写速度\r\n");
	printf("2. K2键按下，测试32MB读速度\r\n");
	printf("3. K3键按下，读取1024字节并打印\r\n");
	printf("4. 摇杆OK键按下，测试SDRAM所有单元是否有异常\r\n");
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
