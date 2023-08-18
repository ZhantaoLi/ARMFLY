/*
*********************************************************************************************************
*
*	模块名称 : SPI 从机
*	文件名称 : demo_spi_slave.c
*	版    本 : V1.0
*	说    明 : SPI 从机
*     
*	修改记录 :
*		版本号  日期        作者     说明
*		V1.0    2022-04-08 Eric2013  正式发布
*
*	Copyright (C), 2020-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/
#include "demo_spi_slave.h"
#include "bsp.h"


/* 仅允许本文件内调用的函数声明 */
static void sfDispMenu(void);
extern __IO uint32_t wTransferState;

/*
*********************************************************************************************************
*	                                       宏定义
*********************************************************************************************************
*/
#define AppAddr  0x08100000    /* APP地址 */

__IO uint32_t uwCRCValue;
__IO uint32_t uwExpectedCRCValue;
__IO uint32_t uwAppSize;

uint8_t buf[1024];
uint32_t RecCount = 0;
uint32_t RecCount0 = 0;
uint32_t RecSize = 0;
uint8_t RecCplt = 0;
uint32_t filesize = 0;

static void JumpToApp(void);

/*
*********************************************************************************************************
*	函 数 名: DemoSpiSlave
*	功能说明: SPI 从机通信
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void DemoSpiSlave(void)
{
	uint32_t SectorCount = 0;
	uint32_t SectorRemain = 0;
	uint32_t i;
    uint32_t TotalSize = 0;
	uint8_t ucState;
	
    
    /***************设置SPI Flash片选上拉，防止影响 ***************/
    {
        GPIO_InitTypeDef gpio_init;

        /* 打开GPIO时钟 */
        __HAL_RCC_GPIOD_CLK_ENABLE();
        
        gpio_init.Mode = GPIO_MODE_OUTPUT_PP;	/* 设置推挽输出 */
        gpio_init.Pull = GPIO_NOPULL;			/* 上下拉电阻不使能 */
        gpio_init.Speed = GPIO_SPEED_HIGH;  	/* GPIO速度等级 */	
        gpio_init.Pin = GPIO_PIN_13;	
        HAL_GPIO_Init(GPIOD, &gpio_init);

        GPIOD->BSRR = GPIO_PIN_13;
    }
    
	sfDispMenu();		/* 打印命令提示 */
	
	bsp_StartAutoTimer(0, 100);	/* 启动1个100ms的自动重装的定时器 */
    
    /* 上电后，准备接收主机命令 */
	g_spiTxBuf[69] = 0x30;
    g_spiLen = 70;
    bsp_spiTransfer();
    
	while(1)
	{
		bsp_Idle();		/* 这个函数在bsp.c文件。用户可以修改这个函数实现CPU休眠和喂狗 */
		
		/* 判断定时器超时时间 */
		if (bsp_CheckTimer(0))	
		{
			/* 每隔100ms 进来一次 */  
			bsp_LedToggle(2);
		}

        if (wTransferState != TRANSFER_WAIT)
        {
			/* 根据固件大小，做扇区擦除 ************/
			if(g_spiRxBuf[0] == '*')
			{
				/* 获取文件大小 */
				filesize = g_spiRxBuf[1] + (g_spiRxBuf[2] << 8) + (g_spiRxBuf[3] << 16) + (g_spiRxBuf[4] << 24);
				uwAppSize = filesize;
				for(int i = 0; i < 69; i++)
				{
					printf("%x ", g_spiRxBuf[i]);
				}
				
				/* 根据文件大小执行擦除 */
				SectorCount = filesize/(128*1024);
				SectorRemain = filesize%(128*1024);	
				
				printf("filesize = %d\r\n", filesize);
				for(i = 0; i < SectorCount; i++)
				{
					bsp_EraseCpuFlash((uint32_t)(AppAddr + i*128*1024));
				}
				
				if(SectorRemain)
				{
					bsp_EraseCpuFlash((uint32_t)(AppAddr + i*128*1024));
				}
				
				/* 返回0x30，表示擦除成功 */
				/* 上电后，准备接收主机命令 */
				g_spiTxBuf[69] = 0x30;
				g_spiLen = 70;
				bsp_spiTransfer();
			}
			
			/* 传输完成命令 **************/
			if(g_spiRxBuf[0]  == '#')
			{
				JumpToApp();
			}
			
			/* 开始传输固件命令 **************/
			if(g_spiRxBuf[0]  == '$')
			{					   
				/* 接收数据个数 */
				RecSize = g_spiRxBuf[1];
				
				/* 编程内部Flash, */
				ucState = bsp_WriteCpuFlash((uint32_t)(AppAddr + TotalSize),  (uint8_t *)&g_spiRxBuf[2], RecSize);
				TotalSize += RecSize;
				printf("=====%d\r\n", TotalSize);
				
				/* 如果返回非0，表示编程失败 */
				if(ucState != 0)
				{
					/* 返回0x60，表示编程失败 */
					g_spiTxBuf[69] = 0x60;
				}
				else
				{
					g_spiTxBuf[69] = 0x30;
				}
				
				/* 返回0x30，表示擦除成功 */
				/* 上电后，准备接收主机命令 */
				g_spiLen = 70;
				bsp_spiTransfer();
			}
		}
	}
}

/*
*********************************************************************************************************
*	函 数 名: JumpToApp
*	功能说明: 跳转到应用JumpToApp
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void JumpToApp(void)
{
	uint32_t i=0;
	void (*AppJump)(void);         /* 声明一个函数指针 */
    
    /* 关闭全局中断 */
	DISABLE_INT(); 
    
    /* 设置所有时钟到默认状态，使用HSI时钟 */
	HAL_RCC_DeInit();
    
	/* 关闭滴答定时器，复位到默认值 */
	SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL = 0;

	/* 关闭所有中断，清除所有中断挂起标志 */
	for (i = 0; i < 8; i++)
	{
		NVIC->ICER[i]=0xFFFFFFFF;
		NVIC->ICPR[i]=0xFFFFFFFF;
	}	

	/* 使能全局中断 */
	ENABLE_INT();

	/* 跳转到应用程序，首地址是MSP，地址+4是复位中断服务程序地址 */
	AppJump = (void (*)(void)) (*((uint32_t *) (AppAddr + 4)));

	/* 设置主堆栈指针 */
	__set_MSP(*(uint32_t *)AppAddr);
	
	/* 在RTOS工程，这条语句很重要，设置为特权级模式，使用MSP指针 */
	__set_CONTROL(0);

	/* 跳转到系统BootLoader */
	AppJump(); 

	/* 跳转成功的话，不会执行到这里，用户可以在这里添加代码 */
	while (1)
	{

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
	printf("SPI从机程序\r\n");
	printf("等待主机发送数据\r\n");
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
