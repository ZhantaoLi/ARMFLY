/*
*********************************************************************************************************
*
*	模块名称 : DAC8563/DAC8562测试
*	文件名称 : demo_spi_dac8562.c
*	版    本 : V1.0
*	说    明 : DAC8563测试。
*
*	修改记录 :
*		版本号  日期        作者     说明
*		V1.0    2020-04-03 armfly  正式发布
*
*	Copyright (C), 2020-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/
#include "demo_spi_dac8562.h"
#include "bsp.h"



uint16_t ch1buf[100]; /* 通道1，即通道A缓冲 */
uint16_t ch2buf[100]; /* 通道2，即通道B缓冲 */


/* 仅允许本文件内调用的函数声明 */
static void sfDispMenu(void);
static void MakeSinTable(uint16_t *_pBuf, uint16_t _usSamples, uint16_t _usBottom, uint16_t _usTop);

/*
*********************************************************************************************************
*	函 数 名: DemoSpiDac
*	功能说明: DAC8562测试
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void DemoSpiDac(void)
{
	uint8_t i=0;
	uint8_t ucKeyCode;	/* 按键代码 */
	
	sfDispMenu();		/* 打印命令提示 */
	
	bsp_StartAutoTimer(0, 200);	/* 启动1个100ms的自动重装的定时器 */
	
	
	/* 生成方波数据 */
	for(i =0; i< 50; i++)
	{
		ch1buf[i] = 0;
	}
	
	for(i =50; i< 100; i++)
	{
		ch1buf[i] = 65535;
	}

	/* 生成正弦波数据 */	
	MakeSinTable(ch2buf, 100, 0, 65535);
	
	/* 
	   上电默认双通道输出：
	    第1个参数：
		  1 表示通道1输出
		  2 表示通道2输出
		  3 表示通道1和2都输出
          4 表示通道1和2都输出，并且附加一个控制命令，有效防止传输错误时恢复，即使插拔模块也不影响。
	    最后一个参数：
	      定时器触发速度1MHz，触发1次是一组24bit数据的传输。
	      推荐范围100Hz - 1MHz。
	*/
	DAC8562_SetDacDataDMA(4, ch1buf, ch2buf, sizeof(ch1buf)/sizeof(uint16_t), sizeof(ch2buf)/sizeof(uint16_t), 1000000);
	
	while(1)
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
				case KEY_DOWN_K1:			/* K1键按下，双通道输出，通道1输出方波，通道2输出正弦波 */
					/* 生成方波数据 */
					for(i =0; i< 50; i++)
					{
						ch1buf[i] = 0;
					}
					
					for(i =50; i< 100; i++)
					{
						ch1buf[i] = 65535;
					}

					/* 生成正弦波数据 */	
					MakeSinTable(ch2buf, 100, 0, 65535);
					
					/* 上电默认双通道输出，触发速度1MHz */
					DAC8562_SetDacDataDMA(3, ch1buf, ch2buf, sizeof(ch1buf)/sizeof(uint16_t), sizeof(ch2buf)/sizeof(uint16_t), 1000000);
					break;

				case KEY_DOWN_K2:			/* K2键按下，双通道输出方波 */
					/* 生成正弦波数据 */	
					MakeSinTable(ch2buf, 100, 0, 65535);
					
					/* 上电默认双通道输出，触发速度1MHz */
					DAC8562_SetDacDataDMA(3, ch2buf, ch2buf, sizeof(ch2buf)/sizeof(uint16_t), sizeof(ch2buf)/sizeof(uint16_t), 1000000);
					break;

				case KEY_DOWN_K3:			/* K3键按下，双通道输出正弦波 */
					/* 生成方波数据 */
					for(i =0; i< 50; i++)
					{
						ch1buf[i] = 0;
					}
					
					for(i =50; i< 100; i++)
					{
						ch1buf[i] = 65535;
					}
					
					/* 上电默认双通道输出，触发速度1MHz */
					DAC8562_SetDacDataDMA(3, ch1buf, ch1buf, sizeof(ch1buf)/sizeof(uint16_t), sizeof(ch1buf)/sizeof(uint16_t), 1000000);	
					break;
				
				case JOY_DOWN_U:			/* 摇杆上键按下，通道1停止方波，通道2停止输出 */
					/* 通道1输出方波 */
					for(i =0; i< 50; i++)
					{
						ch1buf[i] = 0;
					}
					
					for(i =50; i< 100; i++)
					{
						ch1buf[i] = 65535;
					}
					
					/* 仅通道1输出 */
					DAC8562_SetDacDataDMA(1, ch1buf, ch2buf, sizeof(ch1buf)/sizeof(uint16_t), sizeof(ch2buf)/sizeof(uint16_t), 1000000);	
					break;
				
				case JOY_DOWN_D:			/* 摇杆下键按下，双通道输出直流 */
					/* 通道1输出负数10V */
					DAC8562_SetDacData(0, 0);	

					/* 通道2输出正10V */
					DAC8562_SetDacData(1, 65535);		
					break;
				
				case JOY_DOWN_OK:			/* 摇杆OK键按下，重新初始化  */
					/* 初始化配置DAC8562/8563 */
					//bsp_InitDAC8562();	
					DAC8562_WriteCmd((7 << 19) | (0 << 16) | (1 << 0));
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
*	函 数 名: MakeSinTable
*	功能说明: 计算产生正弦波数组
*	形    参: _pBuf : 目标缓冲区
*			  _usSamples : 每个周期的样本数 （建议大于32，并且是偶数）
*	返 回 值: 无
*********************************************************************************************************
*/
static void MakeSinTable(uint16_t *_pBuf, uint16_t _usSamples, uint16_t _usBottom, uint16_t _usTop)
{
	uint16_t i;
	uint16_t mid;	/* 中值 */
	uint16_t att;	/* 幅度 */

	mid = (_usBottom + _usTop) / 2;		/* 0位的值 */
	att = (_usTop - _usBottom) / 2;  	/* 正弦波幅度，峰峰值除以2 */

	for (i = 0; i < _usSamples; i++)
	{
		_pBuf[i] = mid + (int32_t)(att * sin((i * 2 * 3.1415926f) / _usSamples));
	}
}

/*
*********************************************************************************************************
*	函 数 名: sfDispMenu
*	功能说明: 显示操作提示菜单
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void sfDispMenu(void)
{
	printf("操作提示:\r\n");
	printf("1. 启动一个自动重装软件定时器，每100ms翻转一次LED2\r\n");
	printf("2. K1键按下，双通道输出，通道1输出方波，通道2输出正弦波\r\n");
	printf("3. K2键按下，双通道输出方波\r\n");	
    printf("4. K3键按下，双通道输出正弦波\r\n");
	printf("5. 摇杆上键按下，通道1停止方波，通道2停止输出\r\n");
	printf("6. 摇杆下键按下，双通道输出直流\r\n");
	printf("7. 摇杆OK键按下，重新初始化 \r\n");
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
