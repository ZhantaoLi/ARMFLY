/*
*********************************************************************************************************
*
*	模块名称 : 主程序模块
*	文件名称 : main.c
*	版    本 : V1.0
*	说    明 : 内部Flash模拟EEPROM。
*              实验目的：
*                1、学习内部Flash模拟EEPROM。
*              实验内容：
*                1、使用内部Flash模拟EEPROM，务必告诉编译要使用的存储空间，防止这个空间存入了程序。
*                2、对于同一个地址空间，仅支持一次编程(不推荐二次编程，即使是将相应bit由数值1编程0)。
*                3、只能对已经擦除的空间做编程，擦除1个扇区是128KB。
*                4、H7的Flash编程时，务必保证要编程的地址是32字节对齐的，即此地址对32求余为0。
*                   并且编程的数据必须32字节整数倍，函数bsp_WriteCpuFlash对字节数不够32字节整数倍的情况自动补0。                   
*              实验操作：
*                1、K1键按下，将8bit，16bit和32bit数据写入到内部Flash。
*                2、K2键按下，将结构体数据写入到内部Flash。
*              注意事项：
*                1. 本实验推荐使用串口软件SecureCRT查看打印信息，波特率115200，数据位8，奇偶校验位无，停止位1。
*                2. 务必将编辑器的缩进参数和TAB设置为4来阅读本文件，要不代码显示不整齐。
*
*	修改记录 :
*		版本号   日期         作者        说明
*		V1.0    2020-02-29   Eric2013     1. CMSIS软包版本 V5.6.0
*                                         2. HAL库版本 V1.7.0
*
*	Copyright (C), 2020-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/	
#include "bsp.h"			 /* 底层硬件驱动 */



/* 定义例程名和例程发布日期 */
#define EXAMPLE_NAME	"V7-内部Flash模拟EEPROM"
#define EXAMPLE_DATE	"2020-02-29"
#define DEMO_VER		"1.0"

static void PrintfLogo(void);
static void PrintfHelp(void);


/* 全局参数 */
typedef struct
{
	uint8_t   ParamVer;			
	uint16_t  ucBackLight;
	uint32_t  Baud485;
	float     ucRadioMode;		
}
PARAM_T;


/* 
   1、将一个扇区的空间预留出来做为参数区，这里是将第2个扇区作为参数区，
      默认情况下不要将第1个扇区做参数区，因为第1个扇区是默认的boot启动地址。
   2、通过这种定义方式告诉编译器，此空间已经被占用，不让编译器再为这个空间编写程序。
*/
#if defined ( __CC_ARM )       /* MDK编译器 */
	const uint8_t para_flash_area[128*1024] __attribute__((at(0x08000000 + 128*1024)));
#elif defined ( __ICCARM__ )   /* IAR编译器 */
	#pragma location=0x08000000 + 128*1024
	const uint8_t para_flash_area[128*1024];
#endif

/*
*********************************************************************************************************
*	函 数 名: main
*	功能说明: c程序入口
*	形    参: 无
*	返 回 值: 错误代码(无需处理)
*********************************************************************************************************
*/
int main(void)
{
	uint8_t ucKeyCode;	/* 按键代码 */
	uint8_t  ucTest, *ptr8;
	uint16_t uiTest, *ptr16;
	uint32_t ulTest, *ptr32;
	PARAM_T tPara, *paraptr;

	
	/* 初始化数据 */
	tPara.Baud485 = 0x5555AAAA;
	tPara.ParamVer = 0x99;
	tPara.ucBackLight = 0x7788;
	tPara.ucRadioMode = 99.99f;
	
	
	bsp_Init();		/* 硬件初始化 */
	PrintfLogo();	/* 打印例程名称和版本等信息 */
	PrintfHelp();	/* 打印操作提示 */
	
	bsp_StartAutoTimer(0, 100);	/* 启动1个100ms的自动重装的定时器 */
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
				case KEY_DOWN_K1:			/* K1键按下，将8bit，16bit和32bit数据写入到内部Flash */
					
					/*
						1、对于同一个地址空间，仅支持一次编程(不推荐二次编程，即使是将相应bit由数值1编程0)。
				        2、只能对已经擦除的空间做编程，擦除1个扇区是128KB。
				        3、H7的Flash编程时，务必保证要编程的地址是32字节对齐的，即此地址对32求余为0。并且编程的数据必须32字节整数倍。
				           函数bsp_WriteCpuFlash对字节数不够32字节整数倍的情况自动补0。
					*/
					/* 擦除扇区 */
					bsp_EraseCpuFlash((uint32_t)para_flash_area);
				
					ucTest = 0xAA;
					uiTest = 0x55AA;
					ulTest = 0x11223344;
					
					/* 扇区写入数据 */
					bsp_WriteCpuFlash((uint32_t)para_flash_area + 32*0,  (uint8_t *)&ucTest, sizeof(ucTest));
					bsp_WriteCpuFlash((uint32_t)para_flash_area + 32*1,  (uint8_t *)&uiTest, sizeof(uiTest));
					bsp_WriteCpuFlash((uint32_t)para_flash_area + 32*2,  (uint8_t *)&ulTest, sizeof(ulTest));				
					
					/* 读出数据并打印 */
					ptr8  = (uint8_t  *)(para_flash_area + 32*0);
					ptr16 = (uint16_t *)(para_flash_area + 32*1);
					ptr32 = (uint32_t *)(para_flash_area + 32*2);
				
					printf("写入数据：ucTest = %x, uiTest = %x, ulTest = %x\r\n", ucTest, uiTest, ulTest);
					printf("读取数据：ptr8 = %x, ptr16 = %x, ptr32 = %x\r\n", *ptr8, *ptr16, *ptr32);
					
					break;
				
				case KEY_DOWN_K2:			/* K2键按下， 将结构体数据写入到内部Flash */
					/* 擦除扇区 */
					bsp_EraseCpuFlash((uint32_t)para_flash_area);

					/* 扇区写入数据 */
					bsp_WriteCpuFlash((uint32_t)para_flash_area,  (uint8_t *)&tPara, sizeof(tPara));			
					
					/* 读出数据并打印 */
					paraptr  = (PARAM_T  *)((uint32_t)para_flash_area);
				

					printf("写入数据：Baud485=%x, ParamVer=%x, ucBackLight=%x, ucRadioMode=%f\r\n", 
																				tPara.Baud485,
																				tPara.ParamVer,
																				tPara.ucBackLight,
																				tPara.ucRadioMode);
				
					printf("读取数据：Baud485=%x, ParamVer=%x, ucBackLight=%x, ucRadioMode=%f\r\n", 
																				paraptr->Baud485,
																				paraptr->ParamVer,
																				paraptr->ucBackLight,
																				paraptr->ucRadioMode);
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
*	函 数 名: PrintfHelp
*	功能说明: 打印操作提示
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void PrintfHelp(void)
{
	printf("操作提示:\r\n");	
	printf("1. 上电启动了一个软件定时器，每100ms翻转一次LED2\r\n");
	printf("2. K1键按下，将8bit，16bit和32bit数据写入到内部Flash\r\n");
	printf("3. K2键按下，将结构体数据写入到内部Flash\r\n");	
}

/*
*********************************************************************************************************
*	函 数 名: PrintfLogo
*	功能说明: 打印例程名称和例程发布日期, 接上串口线后，打开PC机的超级终端软件可以观察结果
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void PrintfLogo(void)
{
	printf("*************************************************************\n\r");
	
	/* 检测CPU ID */
	{
		uint32_t CPU_Sn0, CPU_Sn1, CPU_Sn2;
		
		CPU_Sn0 = *(__IO uint32_t*)(0x1FF1E800);
		CPU_Sn1 = *(__IO uint32_t*)(0x1FF1E800 + 4);
		CPU_Sn2 = *(__IO uint32_t*)(0x1FF1E800 + 8);

		printf("\r\nCPU : STM32H743XIH6, BGA240, 主频: %dMHz\r\n", SystemCoreClock / 1000000);
		printf("UID = %08X %08X %08X\n\r", CPU_Sn2, CPU_Sn1, CPU_Sn0);
	}

	printf("\n\r");
	printf("*************************************************************\n\r");
	printf("* 例程名称   : %s\r\n", EXAMPLE_NAME);	/* 打印例程名称 */
	printf("* 例程版本   : %s\r\n", DEMO_VER);		/* 打印例程版本 */
	printf("* 发布日期   : %s\r\n", EXAMPLE_DATE);	/* 打印例程日期 */

	/* 打印ST的HAL库版本 */
	printf("* HAL库版本  : V1.7.0 (STM32H7xx HAL Driver)\r\n");
	printf("* \r\n");	/* 打印一行空格 */
	printf("* QQ    : 1295744630 \r\n");
	printf("* 旺旺  : armfly\r\n");
	printf("* Email : armfly@qq.com \r\n");
	printf("* 微信公众号: armfly_com \r\n");
	printf("* 淘宝店: armfly.taobao.com\r\n");
	printf("* Copyright www.armfly.com 安富莱电子\r\n");
	printf("*************************************************************\n\r");
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
