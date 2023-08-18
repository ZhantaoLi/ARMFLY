/*
*********************************************************************************************************
*
*	模块名称 : SPI 主机
*	文件名称 : demo_spi_master.c
*	版    本 : V1.0
*	说    明 : SPI 主机
*                _________________________                        _____________________________
*                |           ______________|                      |______________________       |
*                |          |     SPI1     |                      |        SPI1          |      |
*                |          |              |                      |                      |      |
*                |          |     CLK(PB3) |______________________|(PB3)CLK              |      |
*                |          |              |                      |                      |      |
*                |          |    MISO(PB4) |______________________|(PB4)MISO             |      |
*                |          |              |                      |                      |      |
*                |          |    MOSI(PB5) |______________________|(PB5)MOSI             |      |
*                |          |              |                      |                      |      |
*                |          |    NSS(PG10) |______________________|(PG10)NSS             |      |
*                |          |______________|                      |______________________|      |
*                |                         |                      |                             |
*                |                         |                      |                             |
*                |                         |                      |                             |
*                |                      GND|______________________|GND                          |
*                |                         |                      |                             |
*                |_STM32H7 Master _________|                      |_STM32H7 Slave ______________|
*     
*	修改记录 :
*		版本号  日期        作者     说明
*		V1.0    2022-04-08 Eric2013  正式发布
*
*	Copyright (C), 2020-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/
#include "demo_spi_master.h"
#include "bsp.h"


static void sfDispMenu(void);

/*
*********************************************************************************************************
*	函 数 名: DemoSpiMaster
*	功能说明: SPI 主机通信
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void DemoSpiMaster(void)
{
	uint8_t count = 0;
    uint8_t ucKeyCode;		/* 按键代码 */
    
    /***************设置SPI Flash片选上拉，防止影响 ***************/
    {
        GPIO_InitTypeDef gpio_init;

        /* 打开GPIO时钟 */
        __HAL_RCC_GPIOD_CLK_ENABLE();
        
        gpio_init.Mode = GPIO_MODE_OUTPUT_PP;	
        gpio_init.Pull = GPIO_NOPULL;		
        gpio_init.Speed = GPIO_SPEED_HIGH;  	
        gpio_init.Pin = GPIO_PIN_13;	
        HAL_GPIO_Init(GPIOD, &gpio_init);

        GPIOD->BSRR = GPIO_PIN_13;
    }
    
	sfDispMenu();		/* 打印命令提示 */
	
	bsp_StartAutoTimer(0, 100);	/* 启动1个100ms的自动重装的定时器 */
	
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
				case KEY_DOWN_K1:			/* K1键按下，发送数据给从机*/
                    g_spiTxBuf[0] = count++;
                    g_spiTxBuf[1] = count++;
                    g_spiTxBuf[2] = count++;
                    g_spiTxBuf[3] = count++;
                    g_spiLen = 4;
                    printf("SPI主机发送数据：%d,%d,%d,%d\r\n", g_spiTxBuf[0],g_spiTxBuf[1],g_spiTxBuf[2],g_spiTxBuf[3]);
                    bsp_spiTransfer();
                    printf("SPI主机接收数据：%d,%d,%d,%d\r\n", g_spiRxBuf[0],g_spiRxBuf[1],g_spiRxBuf[2],g_spiRxBuf[3]);
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
*	函 数 名: sfDispMenu
*	功能说明: 显示操作提示菜单
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void sfDispMenu(void)
{
	printf("SPI主机程序\r\n");
	printf("K1按键按下，SPI主机开启SPI全双工通信\r\n");
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
