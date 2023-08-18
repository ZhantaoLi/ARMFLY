/*
*********************************************************************************************************
*
*	模块名称 : CAN方式固件升级
*	文件名称 : demo_canfd_update.c
*	版    本 : V1.0
*	说    明 : CAN方式固件升级
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
void can_Init(void);
void can_DeInit(void);

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
*	函 数 名: DemoCANUpdate
*	功能说明: CAN烧录
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void DemoCANUpdate(void)
{
	uint8_t cmd;
	uint32_t SectorCount = 0;
	uint32_t SectorRemain = 0;
	uint32_t i;
    uint32_t TotalSize = 0;
	uint8_t ucState;
	MSG_T msg;
	
	can_Init();	 /* 初始化CAN */
	
	bsp_StartAutoTimer(0, 500);	/* 启动1个500ms的自动重装的定时器 */
	
	while (1)
	{
		/* 判断定时器超时时间 */
		if (bsp_CheckTimer(0))	
		{            
			/* 每隔500ms 进来一次 */  
			bsp_LedToggle(2);
		}
		
		if (bsp_GetMsg(&msg))
		{
			switch (msg.MsgCode)
			{
				case MSG_CAN1_RX:		/* 接收到CAN设备的应答 */
					cmd = g_Can1RxData[0];
					printf("size = %d, cmd = %c\r\n",g_Can1RxHeader.DataLength>>16, cmd);
					/* 开始传输固件命令 **************/
					if(cmd == '$')
					{					   
						/* 接收够224个数据 */
						RecSize = g_Can1RxData[1];
						
						/* 编程内部Flash, */
						ucState = bsp_WriteCpuFlash((uint32_t)(AppAddr + TotalSize),  (uint8_t *)&g_Can1RxData[2], RecSize);
						TotalSize += RecSize;
						printf("=====%d\r\n", TotalSize);
						
						/* 如果返回非0，表示编程失败 */
						if(ucState != 0)
						{
							/* 返回0x60，表示编程失败 */
						}
						
						/* 返回0x30，表示编程成功 */   
					}
							
					/* 传输完成命令 **************/
					if(cmd == '#')
					{
						JumpToApp();
					}
							
					/* 接收固件大小命令 */
					if(cmd == '*')
					{
						filesize = g_Can1RxData[1] + (g_Can1RxData[2] << 8) + (g_Can1RxData[3] << 16) + (g_Can1RxData[4] << 24);
						uwAppSize = filesize;
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

/*
*********************************************************************************************************
*	函 数 名: can_Init
*	功能说明: 配置CAN硬件
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void can_Init(void)
{
	bsp_InitCan1();
	bsp_InitCan2();
}     

/*
*********************************************************************************************************
*	函 数 名: can_DeInit
*	功能说明: 退出CAN硬件硬质，恢复CPU相关的GPIO为缺省；关闭CAN中断
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void can_DeInit(void)
{	
	bsp_DeInitCan1();
	bsp_DeInitCan2();
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
