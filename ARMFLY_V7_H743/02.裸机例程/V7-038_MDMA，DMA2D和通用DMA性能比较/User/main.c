/*
*********************************************************************************************************
*
*	ģ������ : ������ģ��
*	�ļ����� : main.c
*	��    �� : V1.0
*	˵    �� : �Ƚ�MDMA��DMA2D��DMA1�����ܡ�
*              ʵ��Ŀ�ģ�
*                1. �Ƚ�MDMA��DMA2D��DMA1�����ܡ�
*              ʵ�����ݣ�
*                ÿ�����������������
*                1. 64λ�����AXI SRAM�ڲ���64KB���ݴ��䡣
*                2. 32λ�����D2��SRAM1�ڲ�64KB���ݴ��䡣
*                3. AXI SRAM��SDRAM����64KB�����ݴ��䡣
*                4. 32λ�����SDRAM�ڲ���64KB���ݴ��䡣
*              ע�����
*                1. ��ʵ���Ƽ�ʹ�ô������SecureCRT�鿴��ӡ��Ϣ��������115200������λ8����żУ��λ�ޣ�ֹͣλ1��
*                2. ��ؽ��༭��������������TAB����Ϊ4���Ķ����ļ���Ҫ��������ʾ�����롣
*
*	�޸ļ�¼ :
*		�汾��   ����         ����        ˵��
*		V1.0    2019-07-05   Eric2013     1. CMSIS����汾 V5.4.0
*                                         2. HAL��汾 V1.3.0
*
*	Copyright (C), 2018-2030, ���������� www.armfly.com
*
*********************************************************************************************************
*/	
#include "bsp.h"			 /* �ײ�Ӳ������ */



/* ���������������̷������� */
#define EXAMPLE_NAME	"V7-�Ƚ�MDMA��DMA2D��DMA1������"
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
*	�� �� ��: main
*	����˵��: c�������
*	��    ��: ��
*	�� �� ֵ: �������(���账��)
*********************************************************************************************************
*/
int main(void)
{
	bsp_Init();		/* Ӳ����ʼ�� */
	PrintfLogo();	/* ��ӡ�������ƺͰ汾����Ϣ */
	
	MDMA_SpeedTest();
	printf("----------------------------------\n\r");
	DMA2D_SpeedTest();
	printf("----------------------------------\n\r");	
	DMA1_SpeedTest();
	
	bsp_StartAutoTimer(0, 200); /* ����1��200ms���Զ���װ�Ķ�ʱ���������ʱ��0 */
	  
	  /* ����������ѭ���� */
	while (1)
	{
		bsp_Idle();
		
		/* �ж������ʱ��0�Ƿ�ʱ */
		if(bsp_CheckTimer(0))
		{
			/* ÿ��200ms ����һ�� */  
			bsp_LedToggle(2);
		}
	}
}

/*
*********************************************************************************************************
*	�� �� ��: MDMA_SpeedTest
*	����˵��: MDMA���ܲ���
*	��    ��: ��
*	�� �� ֵ: ��
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
	/* MDMA���� **********************************************************************/
	__HAL_RCC_MDMA_CLK_ENABLE();  

	MDMA_Handle.Instance = MDMA_Channel0;  

	MDMA_Handle.Init.Request              = MDMA_REQUEST_SW;         /* ������� */
	MDMA_Handle.Init.TransferTriggerMode  = MDMA_BLOCK_TRANSFER;     /* �鴫�� */
	MDMA_Handle.Init.Priority             = MDMA_PRIORITY_HIGH;      /* ���ȼ���*/
	MDMA_Handle.Init.Endianness           = MDMA_LITTLE_ENDIANNESS_PRESERVE; /* С�� */
	MDMA_Handle.Init.SourceInc            = MDMA_SRC_INC_DOUBLEWORD;         /* Դ��ַ������˫�֣���8�ֽ� */
	MDMA_Handle.Init.DestinationInc       = MDMA_DEST_INC_DOUBLEWORD;        /* Ŀ�ĵ�ַ������˫�֣���8�ֽ� */
	MDMA_Handle.Init.SourceDataSize       = MDMA_SRC_DATASIZE_DOUBLEWORD;    /* Դ��ַ���ݿ��˫�֣���8�ֽ� */
	MDMA_Handle.Init.DestDataSize         = MDMA_DEST_DATASIZE_DOUBLEWORD;   /* Ŀ�ĵ�ַ���ݿ��˫�֣���8�ֽ� */
	MDMA_Handle.Init.DataAlignment        = MDMA_DATAALIGN_PACKENABLE;       /* С�ˣ��Ҷ��� */                    
	MDMA_Handle.Init.SourceBurst          = MDMA_SOURCE_BURST_16BEATS;      /* Դ����ͻ�����䣬SourceBurst*SourceDataSize <  BufferTransferLength*/
	MDMA_Handle.Init.DestBurst            = MDMA_DEST_BURST_16BEATS;        /* Ŀ������ͻ�����䣬DestBurst*DestDataSize < BufferTransferLength */

	MDMA_Handle.Init.BufferTransferLength = 128;    /* ÿ�δ���128���ֽ� */

	MDMA_Handle.Init.SourceBlockAddressOffset  = 0; /* ����block���䣬��ַƫ��0 */
	MDMA_Handle.Init.DestBlockAddressOffset    = 0; /* ����block���䣬��ַƫ��0 */

	/* ��ʼ��MDMA */
	if(HAL_MDMA_Init(&MDMA_Handle) != HAL_OK)
	{
		 Error_Handler(__FILE__, __LINE__);
	}
	
//	MDMA_Handle.Instance->CTCR |=0x80000000;
	
	/* ���ô�����ɻص����жϼ������ȼ����� */
	HAL_MDMA_RegisterCallback(&MDMA_Handle, HAL_MDMA_XFER_CPLT_CB_ID, MDMA_TransferCompleteCallback);
	HAL_NVIC_SetPriority(MDMA_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(MDMA_IRQn);  

    /* AXI SRAM��64KB���ݴ������ ***********************************************/
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
	printf("MDMA---AXI SRAM�ڲ�����64KB���ݺ�ʱ =  %dus %dMB/S\r\n", cnt/400, 25000000/cnt);
		
    /* D2��SRAM1��64KB���ݴ������ ***********************************************/
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

	printf("MDMA---D2��SRAM1�ڲ�����64KB���ݺ�ʱ =  %dus %dMB/S\r\n", cnt/400, 25000000/cnt);
		
	/* AXI SRAM��SDRAM��64KB���ݴ������ ***********************************************/
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

	printf("MDMA---AXI SRAM����64KB���ݵ�SDRAM��ʱ =  %dus %dMB/S\r\n", cnt/400, 25000000/cnt);
		
	/* SDRAM��64KB���ݴ������ ***********************************************/
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

	printf("MDMA---SDRAM�ڲ�����64KB���ݺ�ʱ =  %dus %dMB/S\r\n", cnt/400, 25000000/cnt);
}

/*
*********************************************************************************************************
*	�� �� ��: DMA2D_SpeedTest
*	����˵��: DMA2D���ܲ���
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void DMA2D_SpeedTest(void)
{
	__HAL_RCC_DMA2D_CLK_ENABLE();  
	
	/* DMA2D���ô洢�����洢��ģʽ, ����ģʽ��ǰ������ΪDMA2D���� */  
	DMA2D->CR      = 0x00000000UL;
	DMA2D->FGOR    = 0;
	DMA2D->OOR     = 0;
	
	/* ǰ�����������򶼲��õ�RGB565��ɫ��ʽ */
	DMA2D->FGPFCCR = LTDC_PIXEL_FORMAT_ARGB8888;
	DMA2D->OPFCCR  = LTDC_PIXEL_FORMAT_ARGB8888;
	
	DMA2D->NLR     = (uint32_t)(64 << 16) | (uint16_t)256;

	
    /* AXI SRAM��64KB���ݴ������ ***********************************************/
	DMA2D->FGMAR = (uint32_t)0x24000000;
	DMA2D->OMAR  = (uint32_t)(0x24000000 + 64*1024);
	DMA2D->CR   |= DMA2D_CR_START;   
	
	start = DWT_CYCCNT;
	/* �ȴ�DMA2D������� */
	while (DMA2D->CR & DMA2D_CR_START) {} 
	end = DWT_CYCCNT;
	cnt = end - start;
	
	printf("DMA2D---AXI SRAM�ڲ�����64KB���ݺ�ʱ =  %dus %dMB/S\r\n", cnt/400, 25000000/cnt);
		
    /* D2��SRAM1��64KB���ݴ������ ***********************************************/
	DMA2D->FGMAR = (uint32_t)0x30000000;
	DMA2D->OMAR  = (uint32_t)(0x30000000 + 64*1024);
	DMA2D->CR   |= DMA2D_CR_START;  
	
	start = DWT_CYCCNT;
	/* �ȴ�DMA2D������� */
	while (DMA2D->CR & DMA2D_CR_START) {} 
	end = DWT_CYCCNT;
	cnt = end - start;
		
	printf("DMA2D---D2��SRAM1�ڲ�����64KB���ݺ�ʱ =  %dus %dMB/S\r\n", cnt/400, 25000000/cnt);
		
	/* AXI SRAM��SDRAM��64KB���ݴ������ ***********************************************/
	DMA2D->FGMAR = (uint32_t)0x24000000;
	DMA2D->OMAR  = (uint32_t)0xC0000000;
	DMA2D->CR   |= DMA2D_CR_START;  
	
	start = DWT_CYCCNT;
	/* �ȴ�DMA2D������� */
	while (DMA2D->CR & DMA2D_CR_START) {} 
	end = DWT_CYCCNT;
	cnt = end - start;
		
	printf("DMA2D---AXI SRAM����64KB���ݵ�SDRAM��ʱ =  %dus %dMB/S\r\n", cnt/400, 25000000/cnt);	

	/* SDRAM��64KB���ݴ������ ***********************************************/
	DMA2D->FGMAR = (uint32_t)0xC0000000;
	DMA2D->OMAR  = (uint32_t)(0xC0000000 + 64*1024);
	DMA2D->CR   |= DMA2D_CR_START;
	
	start = DWT_CYCCNT;
	/* �ȴ�DMA2D������� */
	while (DMA2D->CR & DMA2D_CR_START) {} 
	end = DWT_CYCCNT;
	cnt = end - start;	
		
	printf("DMA2D---SDRAM�ڲ�����64KB���ݺ�ʱ =  %dus %dMB/S\r\n", cnt/400, 25000000/cnt);	
}

/*
*********************************************************************************************************
*	�� �� ��: DMA1_SpeedTest
*	����˵��: DMA1���ܲ���
*	��    ��: ��
*	�� �� ֵ: ��
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
	DMA_Handle.Init.MemBurst            = DMA_MBURST_INC4;     /*WORD��ʽ����֧��4��ͻ�� */
	DMA_Handle.Init.PeriphBurst         = DMA_PBURST_INC4;      /*WORD��ʽ����֧��4��ͻ�� */
	DMA_Handle.XferCpltCallback         = DMA_TransferCompleteCallback;

	HAL_DMA_Init(&DMA_Handle);

	HAL_NVIC_SetPriority(DMA1_Stream1_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(DMA1_Stream1_IRQn);
	
	 /* AXI SRAM��64KB���ݴ������ ***********************************************/
	TransferCompleteDetected = 0;
	HAL_DMA_Start_IT(&DMA_Handle, (uint32_t)0x24000000, (uint32_t)(0x24000000 + 64*1024), 64*256);

	start = DWT_CYCCNT;
	while(TransferCompleteDetected == 0) {}
	end = DWT_CYCCNT;
	cnt = end - start;
 
	//64*1024/(cnt/400/1000/1000)/1024/1024 = 64*1000*1000*400/1024/cnt = 25000000/cnt
	printf("DMA1---AXI SRAM�ڲ�����64KB���ݺ�ʱ =  %dus %dMB/S\r\n", cnt/400, 25000000/cnt);
		
	/* D2��SRAM1��64KB���ݴ������ ***********************************************/
	TransferCompleteDetected = 0;
	HAL_DMA_Start_IT(&DMA_Handle, (uint32_t)0x30000000, (uint32_t)(0x30000000 + 64*1024), 64*256);

	start = DWT_CYCCNT;
	while(TransferCompleteDetected == 0) {}
	end = DWT_CYCCNT;
	cnt = end - start;

	printf("DMA1---D2��SRAM1�ڲ�����64KB���ݺ�ʱ =  %dus %dMB/S\r\n", cnt/400, 25000000/cnt);
		
		
	/* AXI SRAM��SDRAM��64KB���ݴ������ ***********************************************/
	TransferCompleteDetected = 0;
	HAL_DMA_Start_IT(&DMA_Handle, (uint32_t)0x24000000, (uint32_t)0xC0000000, 64*256);

	start = DWT_CYCCNT;
	while(TransferCompleteDetected == 0) {}
	end = DWT_CYCCNT;
	cnt = end - start;

	printf("DMA1---AXI SRAM����64KB���ݵ�SDRAM��ʱ =  %dus %dMB/S\r\n", cnt/400, 25000000/cnt);
		
	/* SDRAM��64KB���ݴ������ ***********************************************/
	TransferCompleteDetected = 0;
	HAL_DMA_Start_IT(&DMA_Handle, (uint32_t)0xC0000000, (uint32_t)(0xC0000000 + 64*1024), 64*256);	

	start = DWT_CYCCNT;
	while(TransferCompleteDetected == 0) {}
	end = DWT_CYCCNT;
	cnt = end - start;

	printf("DMA1---SDRAM�ڲ�����64KB���ݺ�ʱ =  %dus %dMB/S\r\n", cnt/400, 25000000/cnt);
}

/*
*********************************************************************************************************
*	�� �� ��: PrintfLogo
*	����˵��: ��ӡ�������ƺ����̷�������, ���ϴ����ߺ󣬴�PC���ĳ����ն�������Թ۲���
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void PrintfLogo(void)
{
	printf("*************************************************************\n\r");
	
	/* ���CPU ID */
	{
		uint32_t CPU_Sn0, CPU_Sn1, CPU_Sn2;
		
		CPU_Sn0 = *(__IO uint32_t*)(0x1FF1E800);
		CPU_Sn1 = *(__IO uint32_t*)(0x1FF1E800 + 4);
		CPU_Sn2 = *(__IO uint32_t*)(0x1FF1E800 + 8);

		printf("\r\nCPU : STM32H743XIH6, BGA240, ��Ƶ: %dMHz\r\n", SystemCoreClock / 1000000);
		printf("UID = %08X %08X %08X\n\r", CPU_Sn2, CPU_Sn1, CPU_Sn0);
	}

	printf("\n\r");
	printf("*************************************************************\n\r");
	printf("* ��������   : %s\r\n", EXAMPLE_NAME);	/* ��ӡ�������� */
	printf("* ���̰汾   : %s\r\n", DEMO_VER);		/* ��ӡ���̰汾 */
	printf("* ��������   : %s\r\n", EXAMPLE_DATE);	/* ��ӡ�������� */

	/* ��ӡST��HAL��汾 */
	printf("* HAL��汾  : V1.3.0 (STM32H7xx HAL Driver)\r\n");
	printf("* \r\n");	/* ��ӡһ�пո� */
	printf("* QQ    : 1295744630 \r\n");
	printf("* ����  : armfly\r\n");
	printf("* Email : armfly@qq.com \r\n");
	printf("* ΢�Ź��ں�: armfly_com \r\n");
	printf("* �Ա���: armfly.taobao.com\r\n");
	printf("* Copyright www.armfly.com ����������\r\n");
	printf("*************************************************************\n\r");
}

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
