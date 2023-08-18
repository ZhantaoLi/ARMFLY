/*
*********************************************************************************************************
*
*	模块名称 : 主程序模块
*	文件名称 : main.c
*	版    本 : V1.0
*	说    明 : TCM，SRAM等五块内存的动态内存分配实现。
*              实验目的：
*                1. 学习TCM，SRAM等五块内存的动态内存分配实现。
*              实验内容：
*                1. 启动自动重装软件定时器0，每100ms翻转一次LED2。
*              实验操作：
*                1. K1键按下，从DTCM依次申请280字节，64字节和6111字节。
*                2. K1键松开，释放从DTCM申请的空间。
*                3. K2键按下，从AXI SRAM依次申请160字节，32字节和2333字节。
*                4. K2键松开，释放从AXI SRAM申请的空间。
*                5. K3键按下，从D2域SRAM依次申请200字节，96字节和4111字节。
*                6. K3键松开，释放从D2域SRAM申请的空间。
*                7. 摇杆OK键按下，从D3域SRAM依次申请300字节，128字节和5111字节。
*                8. 摇杆OK键松开，释放从D3域SRAM申请的空间。
*              注意事项：
*                1. 本实验推荐使用串口软件SecureCRT查看打印信息，波特率115200，数据位8，奇偶校验位无，停止位1。
*                2. 务必将编辑器的缩进参数和TAB设置为4来阅读本文件，要不代码显示不整齐。
*
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



/* 定义例程名和例程发布日期 */
#define EXAMPLE_NAME	"V7-TCM，SRAM等五块内存的动态内存分配实现"
#define EXAMPLE_DATE	"2018-12-12"
#define DEMO_VER		"1.0"

static void PrintfLogo(void);
static void PrintfHelp(void);


/* DTCM, 64KB */
mem_head_t *DTCMUsed;
uint64_t AppMallocDTCM[64*1024/8];


#if defined ( __ICCARM__ )    /* 使用的IAR */

/* D1域, AXI SRAM, 512KB */
mem_head_t *AXISRAMUsed;
#pragma location = 0x24000000
uint64_t AppMallocAXISRAM[512*1024/8];

/* D2域, 128KB SRAM1(0x30000000) + 128KB SRAM2(0x30020000) + 32KB SRAM3(0x30040000)  */
mem_head_t *SRAM1Used; 
#pragma location = 0x30000000
uint64_t AppMallocSRAM1[288*1024/8];

/* D3域, SRAM4, 64KB */
mem_head_t *SRAM4Used;
#pragma location = 0x38000000
uint64_t AppMallocSRAM4[64*1024/8];

#elif defined ( __CC_ARM )  /* 使用的MDK */
/* D1域, AXI SRAM, 512KB */
mem_head_t *AXISRAMUsed;
uint64_t AppMallocAXISRAM[512*1024/8]__attribute__((at(0x24000000)));

/* D2域, 128KB SRAM1(0x30000000) + 128KB SRAM2(0x30020000) + 32KB SRAM3(0x30040000)  */
mem_head_t *SRAM1Used; 
uint64_t AppMallocSRAM1[288*1024/8]__attribute__((at(0x30000000)));

/* D3域, SRAM4, 64KB */
mem_head_t *SRAM4Used;
uint64_t AppMallocSRAM4[64*1024/8]__attribute__((at(0x38000000)));
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
	uint8_t ucKeyCode;		/* 按键代码 */
	uint32_t *DTCM_Addres0, *AXISRAM_Addres0, *SRAM1_Addres0, *SRAM4_Addres0;
	uint16_t *DTCM_Addres1, *AXISRAM_Addres1, *SRAM1_Addres1, *SRAM4_Addres1;
	uint8_t  *DTCM_Addres2, *AXISRAM_Addres2, *SRAM1_Addres2, *SRAM4_Addres2;
    
    
	bsp_Init();		/* 硬件初始化 */
	
	/* 初始化动态内存空间 */
	osRtxMemoryInit(AppMallocDTCM,    sizeof(AppMallocDTCM));
	osRtxMemoryInit(AppMallocAXISRAM, sizeof(AppMallocAXISRAM));
	osRtxMemoryInit(AppMallocSRAM1,   sizeof(AppMallocSRAM1));
	osRtxMemoryInit(AppMallocSRAM4,   sizeof(AppMallocSRAM4));
	
	PrintfLogo();	/* 打印例程名称和版本等信息 */
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
                /* 从DTCM依次申请280字节，64字节和6111字节 */
				case KEY_DOWN_K1:	
                    /* 从DTCM申请280字节空间，使用指针变量DTCM_Addres0操作这些空间时不要超过280字节大小 */	
					printf("=========================================================\r\n");
					DTCM_Addres0 = osRtxMemoryAlloc(AppMallocDTCM, 280, 0);
					DTCMUsed = MemHeadPtr(AppMallocDTCM);
					printf("DTCM总大小 = %d字节，申请大小 = 0280字节，当前共使用大小 = %d字节\r\n", 
				                                                DTCMUsed->size, DTCMUsed->used);
				
					/* 从DTCM申请64字节空间，使用指针变量DTCM_Addres1操作这些空间时不要超过64字节大小 */	
					DTCM_Addres1 = osRtxMemoryAlloc(AppMallocDTCM, 64, 0);
					DTCMUsed = MemHeadPtr(AppMallocDTCM);
					printf("DTCM总大小 = %d字节，申请大小 = 0064字节，当前共使用大小 = %d字节\r\n", 
											                   DTCMUsed->size, DTCMUsed->used);
				
					/* 从DTCM申请6111字节空间，使用指针变量DTCM_Addres2操作这些空间时不要超过6111字节大小 */	
					DTCM_Addres2 = osRtxMemoryAlloc(AppMallocDTCM, 6111, 0);
					DTCMUsed = MemHeadPtr(AppMallocDTCM);
					printf("DTCM总大小 = %d字节，申请大小 = 6111字节，当前共使用大小 = %d字节\r\n", 
				                                                DTCMUsed->size, DTCMUsed->used);
					break;
				
				/* 释放从DTCM申请的空间 */
				case KEY_UP_K1:	
					/* 释放从DTCM申请的280字节空间 */
					osRtxMemoryFree(AppMallocDTCM, DTCM_Addres0);
					DTCMUsed = MemHeadPtr(AppMallocDTCM);
					printf("释放DTCM动态内存区申请的0280字节，当前共使用大小 = %d字节\r\n", DTCMUsed->used);
				
					/* 释放从DTCM申请的64字节空间 */
					osRtxMemoryFree(AppMallocDTCM, DTCM_Addres1);
					DTCMUsed = MemHeadPtr(AppMallocDTCM);
					printf("释放DTCM动态内存区申请的0064字节，当前共使用大小 = %d字节\r\n", DTCMUsed->used);
				
					/* 释放从DTCM申请的6111字节空间 */
					osRtxMemoryFree(AppMallocDTCM, DTCM_Addres2);
					DTCMUsed = MemHeadPtr(AppMallocDTCM);
					printf("释放DTCM动态内存区申请的6111字节，当前共使用大小 = %d字节\r\n", DTCMUsed->used);
					break;
				
				/* 从AXI SRAM依次申请160字节，32字节和2333字节 */
				case KEY_DOWN_K2:	
                    /* 从AXI SRAM 申请160字节空间，使用指针变量AXISRAM_Addres0操作这些空间时不要超过160字节大小 */	
					printf("=========================================================\r\n");				
					AXISRAM_Addres0 = osRtxMemoryAlloc(AppMallocAXISRAM, 160, 0);
					AXISRAMUsed = MemHeadPtr(AppMallocAXISRAM);
					printf("AXI SRAM总大小 = %d字节，申请大小 = 0162字节，当前共使用大小 = %d字节\r\n", 
				                                                AXISRAMUsed->size, AXISRAMUsed->used);
				
					/* 从AXI SRAM 申请32字节空间，使用指针变量AXISRAM_Addres1操作这些空间时不要超过32字节大小 */	
					AXISRAM_Addres1 = osRtxMemoryAlloc(AppMallocAXISRAM, 32, 0);
					AXISRAMUsed = MemHeadPtr(AppMallocAXISRAM);
					printf("AXI SRAM总大小 = %d字节，申请大小 = 0032字节，当前共使用大小 = %d字节\r\n", 
											                   AXISRAMUsed->size, AXISRAMUsed->used);
				
					/* 从AXI SRAM 申请2333字节空间，使用指针变量AXISRAM_Addres2操作这些空间时不要超过2333字节大小 */	
					AXISRAM_Addres2 = osRtxMemoryAlloc(AppMallocAXISRAM, 2333, 0);
					AXISRAMUsed = MemHeadPtr(AppMallocAXISRAM);
					printf("AXI SRAM总大小 = %d字节，申请大小 = 2333字节，当前共使用大小 = %d字节\r\n", 
				                                                AXISRAMUsed->size, AXISRAMUsed->used);
					break;
				
				/* 释放从AXI SRAM申请的空间 */
				case KEY_UP_K2:	
					/* 释放从AXI SRAM申请的160字节空间 */
					osRtxMemoryFree(AppMallocAXISRAM, AXISRAM_Addres0);
					AXISRAMUsed = MemHeadPtr(AppMallocAXISRAM);
					printf("释放AXI SRAM动态内存区申请的0160字节，当前共使用大小 = %d字节\r\n", AXISRAMUsed->used);
				
					/* 释放从AXI SRAM申请的32字节空间 */
					osRtxMemoryFree(AppMallocAXISRAM, AXISRAM_Addres1);
					AXISRAMUsed = MemHeadPtr(AppMallocAXISRAM);
					printf("释放AXI SRAM动态内存区申请的0032字节，当前共使用大小 = %d字节\r\n", AXISRAMUsed->used);
				
					/* 释放从AXI SRAM申请的2333字节空间 */
					osRtxMemoryFree(AppMallocAXISRAM, AXISRAM_Addres2);
					AXISRAMUsed = MemHeadPtr(AppMallocAXISRAM);
					printf("释放AXI SRAM动态内存区申请的2333字节，当前共使用大小 = %d字节\r\n", AXISRAMUsed->used);
					break;
				
				/* 从D2域SRAM依次申请200字节，96字节和4111字节 */
				case KEY_DOWN_K3:	
                    /* 从D2域的SRAM申请200字节空间，使用指针变量SRAM1_Addres0操作这些空间时不要超过200字节大小 */	
					printf("=========================================================\r\n");				
					SRAM1_Addres0 = osRtxMemoryAlloc(AppMallocSRAM1, 200, 0);
					SRAM1Used = MemHeadPtr(AppMallocSRAM1);
					printf("D2域SRAM总大小 = %d字节，申请大小 = 0200字节，当前共使用大小 = %d字节\r\n", 
				                                                SRAM1Used->size, SRAM1Used->used);
				
					/* 从D2域的SRAM申请96字节空间，使用指针变量SRAM1_Addres1操作这些空间时不要超过96字节大小 */	
					SRAM1_Addres1 = osRtxMemoryAlloc(AppMallocSRAM1, 96, 0);
					SRAM1Used = MemHeadPtr(AppMallocSRAM1);
					printf("D2域SRAM总大小 = %d字节，申请大小 = 0096字节，当前共使用大小 = %d字节\r\n", 
											                   SRAM1Used->size, SRAM1Used->used);
				
					/* 从D2域的SRAM申请4111字节空间，使用指针变量SRAM1_Addres2操作这些空间时不要超过4111字节大小 */	
					SRAM1_Addres2 = osRtxMemoryAlloc(AppMallocSRAM1, 4111, 0);
					SRAM1Used = MemHeadPtr(AppMallocSRAM1);
					printf("D2域SRAM总大小 = %d字节，申请大小 = 4111字节，当前共使用大小 = %d字节\r\n", 
				                                                SRAM1Used->size, SRAM1Used->used);
					break;
				
				/* 释放从D2域SRAM申请的空间 */
				case KEY_UP_K3:	
					/* 释放从D2域的SRAM申请的200字节空间 */
					osRtxMemoryFree(AppMallocSRAM1, SRAM1_Addres0);
					SRAM1Used = MemHeadPtr(AppMallocSRAM1);
					printf("释放D2域SRAM动态内存区申请的0200字节，当前共使用大小 = %d字节\r\n", SRAM1Used->used);
				
					/* 释放从D2域的SRAM申请的96字节空间 */
					osRtxMemoryFree(AppMallocSRAM1, SRAM1_Addres1);
					SRAM1Used = MemHeadPtr(AppMallocSRAM1);
					printf("释放D2域SRAM动态内存区申请的0096字节，当前共使用大小 = %d字节\r\n", SRAM1Used->used);
				
					/* 释放从D2域的SRAM申请的4111字节空间 */
					osRtxMemoryFree(AppMallocSRAM1, SRAM1_Addres2);
					SRAM1Used = MemHeadPtr(AppMallocSRAM1);
					printf("释放D2域SRAM动态内存区申请的4111字节，当前共使用大小 = %d字节\r\n", SRAM1Used->used);
					break;
				
				/* 从D3域SRAM依次申请300字节，128字节和5111字节 */
				case JOY_DOWN_OK:	
                    /* 从D3域的SRAM申请300字节空间，使用指针变量SRAM4_Addres0操作这些空间时不要超过300字节大小 */	
					printf("=========================================================\r\n");				
					SRAM4_Addres0 = osRtxMemoryAlloc(AppMallocSRAM4, 300, 0);
					SRAM4Used = MemHeadPtr(AppMallocSRAM4);
					printf("D3域SRAM总大小 = %d字节，申请大小 = 0300字节，当前共使用大小 = %d字节\r\n", 
				                                                SRAM4Used->size, SRAM4Used->used);
				
					/* 从D3域的SRAM申请96字节空间，使用指针变量SRAM4_Addres1操作这些空间时不要超过96字节大小 */	
					SRAM4_Addres1 = osRtxMemoryAlloc(AppMallocSRAM4, 128, 0);
					SRAM4Used = MemHeadPtr(AppMallocSRAM4);
					printf("D3域SRAM总大小 = %d字节，申请大小 = 0128字节，当前共使用大小 = %d字节\r\n", 
											                   SRAM4Used->size, SRAM4Used->used);
				
					/* 从D3域的SRAM申请5111字节空间，使用指针变量SRAM4_Addres2操作这些空间时不要超过5111字节大小 */	
					SRAM4_Addres2 = osRtxMemoryAlloc(AppMallocSRAM4, 5111, 0);
					SRAM4Used = MemHeadPtr(AppMallocSRAM4);
					printf("D3域SRAM总大小 = %d字节，申请大小 = 5111字节，当前共使用大小 = %d字节\r\n", 
				                                                SRAM4Used->size, SRAM4Used->used);
					break;
				
				/* 释放从D3域SRAM申请的空间 */
				case JOY_UP_OK:	
					/* 释放从D3域的SRAM申请的300字节空间 */
					osRtxMemoryFree(AppMallocSRAM4, SRAM4_Addres0);
					SRAM4Used = MemHeadPtr(AppMallocSRAM4);
					printf("释放D3域SRAM动态内存区申请的0300字节，当前共使用大小 = %d字节\r\n", SRAM4Used->used);
				
					/* 释放从D3域的SRAM申请的128字节空间 */
					osRtxMemoryFree(AppMallocSRAM4, SRAM4_Addres1);
					SRAM4Used = MemHeadPtr(AppMallocSRAM4);
					printf("释放D3域SRAM动态内存区申请的0128字节，当前共使用大小 = %d字节\r\n", SRAM4Used->used);
				
					/* 释放从D3域的SRAM申请的5111字节空间 */
					osRtxMemoryFree(AppMallocSRAM4, SRAM4_Addres2);
					SRAM4Used = MemHeadPtr(AppMallocSRAM4);
					printf("释放D3域SRAM动态内存区申请的5111字节，当前共使用大小 = %d字节\r\n", SRAM4Used->used);
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
	printf("1. K1键按下，从DTCM依次申请280字节，64字节和6111字节\r\n");
	printf("2. K1键松开，释放从DTCM申请的空间\r\n");
	printf("3. K2键按下，从AXI SRAM依次申请160字节，32字节和2333字节\r\n");
	printf("4. K2键松开，释放从AXI SRAM申请的空间\r\n");
	printf("5. K3键按下，从D2域SRAM依次申请200字节，96字节和4111字节\r\n");
	printf("6. K3键松开，释放从D2域SRAM申请的空间\r\n");
	printf("7. 摇杆OK键按下，从D3域SRAM依次申请300字节，128字节和5111字节\r\n");
	printf("8. 摇杆OK键松开，释放从D3域SRAM申请的空间\r\n");
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
	printf("* HAL库版本  : V1.3.0 (STM32H7xx HAL Driver)\r\n");
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
