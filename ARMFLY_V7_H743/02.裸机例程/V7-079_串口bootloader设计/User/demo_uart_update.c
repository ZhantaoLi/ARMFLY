/*
*********************************************************************************************************
*
*	模块名称 : 串口方式固件升级
*	文件名称 : demo_uart_update.c
*	版    本 : V1.0
*	说    明 : 串口方式固件升级
*
*	修改记录 :
*		版本号   日期         作者        说明
*		V1.0    2022-06-15   Eric2013    正式发布
*
*	Copyright (C), 2022-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/
#include "bsp.h"


/*
*********************************************************************************************************
*	                                        函数
*********************************************************************************************************
*/
/* 仅允许本文件内调用的函数声明 */
static void JumpToApp(void);

/*
*********************************************************************************************************
*	                                       宏定义
*********************************************************************************************************
*/
#define AppAddr  0x08100000    /* APP地址 */


/*
*********************************************************************************************************
*	                                       变量
*********************************************************************************************************
*/
__IO uint32_t uwCRCValue;
__IO uint32_t uwExpectedCRCValue;
__IO uint32_t uwAppSize;

uint8_t buf[1024];
uint32_t RecCount = 0;
uint32_t RecCount0 = 0;
uint32_t RecSize = 0;
uint8_t RecCplt = 0;
uint32_t filesize = 0;

/*
*********************************************************************************************************
*	函 数 名: DemoUartUpdate
*	功能说明: 串口烧录
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void DemoUartUpdate(void)
{
	uint8_t cmd;
	uint8_t ucStatus = 0;  /* 状态机标志 */
	uint32_t SectorCount = 0;
	uint32_t SectorRemain = 0;
	uint32_t i;
    uint32_t TotalSize = 0;
	uint8_t ucState;
	
	
	bsp_StartAutoTimer(0, 500);	/* 启动1个500ms的自动重装的定时器 */
	
	while (1)
	{
		/* 判断定时器超时时间 */
		if (bsp_CheckTimer(0))	
		{            
			/* 每隔500ms 进来一次 */  
			bsp_LedToggle(2);
		}

		if (comGetChar(COM1, &cmd))	/* 从串口读入一个字符(非阻塞方式) */
		{
			switch (ucStatus)
			{
				case 0:
					/* 开始传输固件命令 **************/
					if(cmd == '$')
					{
						RecCplt = 0;
						ucStatus = 1;        
					}
					
					/* 传输完成命令 **************/
					if(cmd == '#')
					{
						RecCount = 0;
						RecCplt = 1;
						JumpToApp();
					}
					
					/* 接收固件大小命令 */
					if(cmd == '*')
					{
						ucStatus = 3; 
					}
					break;
					
				/* 设置每帧传输字节数，默认设置的224字节 */
				case 1:
					RecSize = cmd;
					ucStatus = 2; 
					break;
					   
				/* 将接收到的数据编程到内部Flash */
				case 2:
					buf[RecCount0] = cmd;
                                       
					/* 接收够224个数据 */
					if(RecCount0 == (RecSize - 1))
					{
						ucStatus = 0;
						RecCount0 = 0;
						
						/* 编程内部Flash, */
						ucState = bsp_WriteCpuFlash((uint32_t)(AppAddr + TotalSize),  (uint8_t *)buf, RecSize);
						TotalSize += RecSize;
						
						/* 如果返回非0，表示编程失败 */
						if(ucState != 0)
						{
							/* 返回0x60，表示编程失败 */
							comSendChar(COM1, 0x60);
						}
						
						/* 返回0x30，表示编程成功 */
						comSendChar(COM1, 0x30);
					}
					else
					{
						RecCount++;
						RecCount0++;
					}
					break;
					
				/* 根据接收到的文件大小，擦除相应大小的扇区 */
				case 3:
					buf[RecCount0] = cmd;
                                       
					if(RecCount0 == 3)
					{
						ucStatus = 0;
						RecCount0 = 0;
						filesize = buf[0] + (buf[1] << 8) + (buf[2] << 16) + (buf[3] << 24);
						uwAppSize = filesize;
						SectorCount = filesize/(128*1024);
						SectorRemain = filesize%(128*1024);	
						
						for(i = 0; i < SectorCount; i++)
						{
							bsp_EraseCpuFlash((uint32_t)(AppAddr + i*128*1024));
						}
						
						if(SectorRemain)
						{
							bsp_EraseCpuFlash((uint32_t)(AppAddr + i*128*1024));
						}
						
						/* 返回0x30，表示擦除成功 */
						comSendChar(COM1, 0x30);
					}
					else
					{
						RecCount0++;
					}
					break;
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

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
