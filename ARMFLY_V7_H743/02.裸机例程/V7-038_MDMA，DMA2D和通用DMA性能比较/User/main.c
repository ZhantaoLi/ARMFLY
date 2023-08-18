/*
*********************************************************************************************************
*
*	模块名称 : 主程序模块
*	文件名称 : main.c
*	版    本 : V1.0
*	说    明 : 比较MDMA，DMA2D和DMA1的性能。
*              实验目的：
*                1. 比较MDMA，DMA2D和DMA1的性能。
*              实验内容：
*                每个都测试了四种情况
*                1. 64位带宽的AXI SRAM内部做64KB数据传输。
*                2. 32位带宽的D2域SRAM1内部64KB数据传输。
*                3. AXI SRAM向SDRAM传输64KB的数据传输。
*                4. 32位带宽的SDRAM内部做64KB数据传输。
*              注意事项：
*                1. 本实验推荐使用串口软件SecureCRT查看打印信息，波特率115200，数据位8，奇偶校验位无，停止位1。
*                2. 务必将编辑器的缩进参数和TAB设置为4来阅读本文件，要不代码显示不整齐。
*
*	修改记录 :
*		版本号   日期         作者        说明
*		V1.0    2019-07-05   Eric2013     1. CMSIS软包版本 V5.4.0
*                                         2. HAL库版本 V1.3.0
*
*	Copyright (C), 2018-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/	
#include "bsp.h"			 /* 底层硬件驱动 */



/* 定义例程名和例程发布日期 */
#define EXAMPLE_NAME	"V7-比较MDMA，DMA2D和DMA1的性能"
#define EXAMPLE_DATE	"2019-07-05"
#define DEMO_VER		"1.0"

static void PrintfLogo(void);
void MDMA_SpeedTest(void);
void DMA2D_SpeedTest(void);
void DMA1_SpeedTest(void);

MDMA_HandleTypeDef  MDMA_Handle;
DMA_HandleTypeDef   DMA_Handle;
uint32_t start, end, cnt;
__IO uint32_t TransferCompleteDetected = 0;

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
	bsp_Init();		/* 硬件初始化 */
	PrintfLogo();	/* 打印例程名称和版本等信息 */
	
	MDMA_SpeedTest();
	printf("----------------------------------\n\r");
	DMA2D_SpeedTest();
	printf("----------------------------------\n\r");	
	DMA1_SpeedTest();
	
	bsp_StartAutoTimer(0, 200); /* 启动1个200ms的自动重装的定时器，软件定时器0 */
	  
	  /* 进入主程序循环体 */
	while (1)
	{
		bsp_Idle();
		
		/* 判断软件定时器0是否超时 */
		if(bsp_CheckTimer(0))
		{
			/* 每隔200ms 进来一次 */  
			bsp_LedToggle(2);
		}
	}
}

/*
*********************************************************************************************************
*	函 数 名: MDMA_SpeedTest
*	功能说明: MDMA性能测试
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void MDMA_IRQHandler(void)
{
	HAL_MDMA_IRQHandler(&MDMA_Handle);
}
static void MDMA_TransferCompleteCallback(MDMA_HandleTypeDef *hmdma)
{
	TransferCompleteDetected = 1;
}
void MDMA_SpeedTest(void)
{
	/* MDMA配置 **********************************************************************/
	__HAL_RCC_MDMA_CLK_ENABLE();  

	MDMA_Handle.Instance = MDMA_Channel0;  

	MDMA_Handle.Init.Request              = MDMA_REQUEST_SW;         /* 软件触发 */
	MDMA_Handle.Init.TransferTriggerMode  = MDMA_BLOCK_TRANSFER;     /* 块传输 */
	MDMA_Handle.Init.Priority             = MDMA_PRIORITY_HIGH;      /* 优先级高*/
	MDMA_Handle.Init.Endianness           = MDMA_LITTLE_ENDIANNESS_PRESERVE; /* 小端 */
	MDMA_Handle.Init.SourceInc            = MDMA_SRC_INC_DOUBLEWORD;         /* 源地址自增，双字，即8字节 */
	MDMA_Handle.Init.DestinationInc       = MDMA_DEST_INC_DOUBLEWORD;        /* 目的地址自增，双字，即8字节 */
	MDMA_Handle.Init.SourceDataSize       = MDMA_SRC_DATASIZE_DOUBLEWORD;    /* 源地址数据宽度双字，即8字节 */
	MDMA_Handle.Init.DestDataSize         = MDMA_DEST_DATASIZE_DOUBLEWORD;   /* 目的地址数据宽度双字，即8字节 */
	MDMA_Handle.Init.DataAlignment        = MDMA_DATAALIGN_PACKENABLE;       /* 小端，右对齐 */                    
	MDMA_Handle.Init.SourceBurst          = MDMA_SOURCE_BURST_16BEATS;      /* 源数据突发传输，SourceBurst*SourceDataSize <  BufferTransferLength*/
	MDMA_Handle.Init.DestBurst            = MDMA_DEST_BURST_16BEATS;        /* 目的数据突发传输，DestBurst*DestDataSize < BufferTransferLength */

	MDMA_Handle.Init.BufferTransferLength = 128;    /* 每次传输128个字节 */

	MDMA_Handle.Init.SourceBlockAddressOffset  = 0; /* 用于block传输，地址偏移0 */
	MDMA_Handle.Init.DestBlockAddressOffset    = 0; /* 用于block传输，地址偏移0 */

	/* 初始化MDMA */
	if(HAL_MDMA_Init(&MDMA_Handle) != HAL_OK)
	{
		 Error_Handler(__FILE__, __LINE__);
	}
	
//	MDMA_Handle.Instance->CTCR |=0x80000000;
	
	/* 设置传输完成回调和中断及其优先级配置 */
	HAL_MDMA_RegisterCallback(&MDMA_Handle, HAL_MDMA_XFER_CPLT_CB_ID, MDMA_TransferCompleteCallback);
	HAL_NVIC_SetPriority(MDMA_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(MDMA_IRQn);  

    /* AXI SRAM的64KB数据传输测试 ***********************************************/
	TransferCompleteDetected = 0;
	HAL_MDMA_Start_IT(&MDMA_Handle, 
				      (uint32_t)0x24000000, 
				      (uint32_t)(0x24000000 + 64*1024), 
				      64*1024, 
				      1);


	start = DWT_CYCCNT;
	while(TransferCompleteDetected == 0) {}
	end = DWT_CYCCNT;
	cnt = end - start;
 
	//64*1024/(cnt/400/1000/1000)/1024/1024 = 64*1000*1000*400/1024/cnt = 25000000/cnt
	printf("MDMA---AXI SRAM内部互传64KB数据耗时 =  %dus %dMB/S\r\n", cnt/400, 25000000/cnt);
		
    /* D2域SRAM1的64KB数据传输测试 ***********************************************/
	TransferCompleteDetected = 0;
	HAL_MDMA_Start_IT(&MDMA_Handle, 
				      (uint32_t)0x30000000, 
				      (uint32_t)(0x30000000 + 64*1024), 
				      64*1024, 
				      1);

	start = DWT_CYCCNT;
	while(TransferCompleteDetected == 0) {}
	end = DWT_CYCCNT;
	cnt = end - start;

	printf("MDMA---D2域SRAM1内部互传64KB数据耗时 =  %dus %dMB/S\r\n", cnt/400, 25000000/cnt);
		
	/* AXI SRAM向SDRAM的64KB数据传输测试 ***********************************************/
	TransferCompleteDetected = 0;
	HAL_MDMA_Start_IT(&MDMA_Handle, 
				      (uint32_t)0x24000000, 
				      (uint32_t)0xC0000000, 
				      64*1024, 
				      1);

	start = DWT_CYCCNT;
	while(TransferCompleteDetected == 0) {}
	end = DWT_CYCCNT;
	cnt = end - start;

	printf("MDMA---AXI SRAM传输64KB数据到SDRAM耗时 =  %dus %dMB/S\r\n", cnt/400, 25000000/cnt);
		
	/* SDRAM的64KB数据传输测试 ***********************************************/
	TransferCompleteDetected = 0;
	HAL_MDMA_Start_IT(&MDMA_Handle, 
				      (uint32_t)0xC0000000, 
				      (uint32_t)(0xC0000000 + 64*1024), 
				      64*1024, 
				      1);


	start = DWT_CYCCNT;
	while(TransferCompleteDetected == 0) {}
	end = DWT_CYCCNT;
	cnt = end - start;

	printf("MDMA---SDRAM内部互传64KB数据耗时 =  %dus %dMB/S\r\n", cnt/400, 25000000/cnt);
}

/*
*********************************************************************************************************
*	函 数 名: DMA2D_SpeedTest
*	功能说明: DMA2D性能测试
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void DMA2D_SpeedTest(void)
{
	__HAL_RCC_DMA2D_CLK_ENABLE();  
	
	/* DMA2D采用存储器到存储器模式, 这种模式是前景层作为DMA2D输入 */  
	DMA2D->CR      = 0x00000000UL;
	DMA2D->FGOR    = 0;
	DMA2D->OOR     = 0;
	
	/* 前景层和输出区域都采用的RGB565颜色格式 */
	DMA2D->FGPFCCR = LTDC_PIXEL_FORMAT_ARGB8888;
	DMA2D->OPFCCR  = LTDC_PIXEL_FORMAT_ARGB8888;
	
	DMA2D->NLR     = (uint32_t)(64 << 16) | (uint16_t)256;

	
    /* AXI SRAM的64KB数据传输测试 ***********************************************/
	DMA2D->FGMAR = (uint32_t)0x24000000;
	DMA2D->OMAR  = (uint32_t)(0x24000000 + 64*1024);
	DMA2D->CR   |= DMA2D_CR_START;   
	
	start = DWT_CYCCNT;
	/* 等待DMA2D传输完成 */
	while (DMA2D->CR & DMA2D_CR_START) {} 
	end = DWT_CYCCNT;
	cnt = end - start;
	
	printf("DMA2D---AXI SRAM内部互传64KB数据耗时 =  %dus %dMB/S\r\n", cnt/400, 25000000/cnt);
		
    /* D2域SRAM1的64KB数据传输测试 ***********************************************/
	DMA2D->FGMAR = (uint32_t)0x30000000;
	DMA2D->OMAR  = (uint32_t)(0x30000000 + 64*1024);
	DMA2D->CR   |= DMA2D_CR_START;  
	
	start = DWT_CYCCNT;
	/* 等待DMA2D传输完成 */
	while (DMA2D->CR & DMA2D_CR_START) {} 
	end = DWT_CYCCNT;
	cnt = end - start;
		
	printf("DMA2D---D2域SRAM1内部互传64KB数据耗时 =  %dus %dMB/S\r\n", cnt/400, 25000000/cnt);
		
	/* AXI SRAM向SDRAM的64KB数据传输测试 ***********************************************/
	DMA2D->FGMAR = (uint32_t)0x24000000;
	DMA2D->OMAR  = (uint32_t)0xC0000000;
	DMA2D->CR   |= DMA2D_CR_START;  
	
	start = DWT_CYCCNT;
	/* 等待DMA2D传输完成 */
	while (DMA2D->CR & DMA2D_CR_START) {} 
	end = DWT_CYCCNT;
	cnt = end - start;
		
	printf("DMA2D---AXI SRAM传输64KB数据到SDRAM耗时 =  %dus %dMB/S\r\n", cnt/400, 25000000/cnt);	

	/* SDRAM的64KB数据传输测试 ***********************************************/
	DMA2D->FGMAR = (uint32_t)0xC0000000;
	DMA2D->OMAR  = (uint32_t)(0xC0000000 + 64*1024);
	DMA2D->CR   |= DMA2D_CR_START;
	
	start = DWT_CYCCNT;
	/* 等待DMA2D传输完成 */
	while (DMA2D->CR & DMA2D_CR_START) {} 
	end = DWT_CYCCNT;
	cnt = end - start;	
		
	printf("DMA2D---SDRAM内部互传64KB数据耗时 =  %dus %dMB/S\r\n", cnt/400, 25000000/cnt);	
}

/*
*********************************************************************************************************
*	函 数 名: DMA1_SpeedTest
*	功能说明: DMA1性能测试
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void DMA1_Stream1_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&DMA_Handle);
}
static void DMA_TransferCompleteCallback(DMA_HandleTypeDef *hdma)
{
	TransferCompleteDetected = 1;
}
void DMA1_SpeedTest(void)
{
	__HAL_RCC_DMA1_CLK_ENABLE();

	DMA_Handle.Instance                 = DMA1_Stream1;
	DMA_Handle.Init.Request             = DMA_REQUEST_MEM2MEM;  
	DMA_Handle.Init.Direction           = DMA_MEMORY_TO_MEMORY;
	DMA_Handle.Init.PeriphInc           = DMA_PINC_ENABLE;
	DMA_Handle.Init.MemInc              = DMA_MINC_ENABLE;
	DMA_Handle.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
	DMA_Handle.Init.MemDataAlignment    = DMA_PDATAALIGN_WORD;
	DMA_Handle.Init.Mode                = DMA_NORMAL;
	DMA_Handle.Init.Priority            = DMA_PRIORITY_VERY_HIGH;
	DMA_Handle.Init.FIFOMode            = DMA_FIFOMODE_ENABLE;
	DMA_Handle.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
	DMA_Handle.Init.MemBurst            = DMA_MBURST_INC4;     /*WORD方式，仅支持4次突发 */
	DMA_Handle.Init.PeriphBurst         = DMA_PBURST_INC4;      /*WORD方式，仅支持4次突发 */
	DMA_Handle.XferCpltCallback         = DMA_TransferCompleteCallback;

	HAL_DMA_Init(&DMA_Handle);

	HAL_NVIC_SetPriority(DMA1_Stream1_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(DMA1_Stream1_IRQn);
	
	 /* AXI SRAM的64KB数据传输测试 ***********************************************/
	TransferCompleteDetected = 0;
	HAL_DMA_Start_IT(&DMA_Handle, (uint32_t)0x24000000, (uint32_t)(0x24000000 + 64*1024), 64*256);

	start = DWT_CYCCNT;
	while(TransferCompleteDetected == 0) {}
	end = DWT_CYCCNT;
	cnt = end - start;
 
	//64*1024/(cnt/400/1000/1000)/1024/1024 = 64*1000*1000*400/1024/cnt = 25000000/cnt
	printf("DMA1---AXI SRAM内部互传64KB数据耗时 =  %dus %dMB/S\r\n", cnt/400, 25000000/cnt);
		
	/* D2域SRAM1的64KB数据传输测试 ***********************************************/
	TransferCompleteDetected = 0;
	HAL_DMA_Start_IT(&DMA_Handle, (uint32_t)0x30000000, (uint32_t)(0x30000000 + 64*1024), 64*256);

	start = DWT_CYCCNT;
	while(TransferCompleteDetected == 0) {}
	end = DWT_CYCCNT;
	cnt = end - start;

	printf("DMA1---D2域SRAM1内部互传64KB数据耗时 =  %dus %dMB/S\r\n", cnt/400, 25000000/cnt);
		
		
	/* AXI SRAM向SDRAM的64KB数据传输测试 ***********************************************/
	TransferCompleteDetected = 0;
	HAL_DMA_Start_IT(&DMA_Handle, (uint32_t)0x24000000, (uint32_t)0xC0000000, 64*256);

	start = DWT_CYCCNT;
	while(TransferCompleteDetected == 0) {}
	end = DWT_CYCCNT;
	cnt = end - start;

	printf("DMA1---AXI SRAM传输64KB数据到SDRAM耗时 =  %dus %dMB/S\r\n", cnt/400, 25000000/cnt);
		
	/* SDRAM的64KB数据传输测试 ***********************************************/
	TransferCompleteDetected = 0;
	HAL_DMA_Start_IT(&DMA_Handle, (uint32_t)0xC0000000, (uint32_t)(0xC0000000 + 64*1024), 64*256);	

	start = DWT_CYCCNT;
	while(TransferCompleteDetected == 0) {}
	end = DWT_CYCCNT;
	cnt = end - start;

	printf("DMA1---SDRAM内部互传64KB数据耗时 =  %dus %dMB/S\r\n", cnt/400, 25000000/cnt);
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
