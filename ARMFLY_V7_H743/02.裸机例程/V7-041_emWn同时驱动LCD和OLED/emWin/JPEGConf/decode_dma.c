/*
*********************************************************************************************************
*
*	模块名称 : 图片文件
*	文件名称 : decode_dma.c
*	版    本 : V1.0
*	说    明 : JPEG图片解码
*	修改记录 :
*		版本号   日期         作者       说明
*		V1.0    2019-05-11  Eric2013    正式发布
*
*	Copyright (C), 2019-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/
#include "decode_dma.h"



/*
*********************************************************************************************************
*	                                       宏定义
*********************************************************************************************************
*/
#define CHUNK_SIZE_IN  ((uint32_t)(64 * 1024))   /* 输入数据大小，单位字节 */
#define CHUNK_SIZE_OUT ((uint32_t)(64 * 1024))   /* 输出数据大小，单位字节 */


/*
*********************************************************************************************************
*	                                       变量
*********************************************************************************************************
*/
__IO uint32_t Jpeg_HWDecodingEnd = 0; /* 解码完成标志，0表示开始解码，1表示解码完成 */
JPEG_HandleTypeDef    JPEG_Handle;
JPEG_ConfTypeDef      JPEG_Info;

uint32_t FrameBufferAddress;          /* JPEG解码后数据地址 */
uint32_t JPEGSourceAddress;           /* JPEG数据源地址 */
uint32_t Input_frameSize;             /* JPEG源数据大小 */
uint32_t Input_frameIndex;            /* JPEG解码过程中，已经解码的数据大小 */


/*
*********************************************************************************************************
*	函 数 名: HAL_JPEG_MspInit
*	功能说明: 初始化JEPG所需要的底层
*	形    参: JPEG_HandleTypeDef句柄指针
*	返 回 值: 无
*********************************************************************************************************
*/
void HAL_JPEG_MspInit(JPEG_HandleTypeDef *hjpeg)
{
	/* 这两个变量务必设置为静态局部变量或者全局变量，因为退出后，JPEG句柄还要使用 */
	static MDMA_HandleTypeDef   hmdmaIn;
	static MDMA_HandleTypeDef   hmdmaOut;  

	/* 使能JPEG时钟 */
	__HAL_RCC_JPGDECEN_CLK_ENABLE();

	/* 使能MDMA时钟 */
	__HAL_RCC_MDMA_CLK_ENABLE();

	/* 使能JPEG中断并配置优先级 */
	HAL_NVIC_SetPriority(JPEG_IRQn, 0x07, 0x00);
	HAL_NVIC_EnableIRQ(JPEG_IRQn);  
	
	/* JPEG输入的MDMA配置 ###########################################*/
	hmdmaIn.Instance                = MDMA_Channel7;                   /* 使用MDMA通道7 */
	hmdmaIn.Init.Priority           = MDMA_PRIORITY_HIGH;              /* 优先级高 */
	hmdmaIn.Init.Endianness         = MDMA_LITTLE_ENDIANNESS_PRESERVE; /* 小端格式 */
	hmdmaIn.Init.SourceInc          = MDMA_SRC_INC_BYTE;               /* 源地址字节递增 */
	hmdmaIn.Init.DestinationInc     = MDMA_DEST_INC_DISABLE;           /* 目的地址无自增 */
	hmdmaIn.Init.SourceDataSize     = MDMA_SRC_DATASIZE_BYTE;          /* 源地址数据宽度字节 */
	hmdmaIn.Init.DestDataSize       = MDMA_DEST_DATASIZE_WORD;         /* 目的地址数据宽度字节 */
	hmdmaIn.Init.DataAlignment      = MDMA_DATAALIGN_PACKENABLE;       /* 小端，右对齐 */
	hmdmaIn.Init.SourceBurst        = MDMA_SOURCE_BURST_32BEATS;       /* 源数据突发传输，32次 */
	hmdmaIn.Init.DestBurst          = MDMA_DEST_BURST_16BEATS;         /* 目的数据突发传输，16次 */
	
	hmdmaIn.Init.SourceBlockAddressOffset = 0;  /* 用于block传输，buffer传输用不到 */
	hmdmaIn.Init.DestBlockAddressOffset   = 0;  /* 用于block传输，buffer传输用不到 */
  
	hmdmaIn.Init.Request = MDMA_REQUEST_JPEG_INFIFO_TH;  	 /* JPEG的FIFO阀值触发中断 */
	hmdmaIn.Init.TransferTriggerMode = MDMA_BUFFER_TRANSFER; /* 使用MDMA的buffer传输 */
	hmdmaIn.Init.BufferTransferLength = 32;                  /* 每次传输32个字节，JPEG的FIFO阀值是32字节 */

	/* 关联MDMA的句柄到JPEG */
	__HAL_LINKDMA(hjpeg, hdmain, hmdmaIn);

	/* 先复位，然后配置MDMA */
	HAL_MDMA_DeInit(&hmdmaIn); 
	HAL_MDMA_Init(&hmdmaIn);

	/* JPEG输出的MDMA配置 ###########################################*/
	hmdmaOut.Instance             = MDMA_Channel6;                   /* 使用MDMA通道6 */
	hmdmaOut.Init.Priority        = MDMA_PRIORITY_VERY_HIGH;         /* 优先级最高 */
	hmdmaOut.Init.Endianness      = MDMA_LITTLE_ENDIANNESS_PRESERVE; /* 小端格式 */
	hmdmaOut.Init.SourceInc       = MDMA_SRC_INC_DISABLE;            /* 源数据地址禁止自增 */
	hmdmaOut.Init.DestinationInc  = MDMA_DEST_INC_BYTE;              /* 目的数据地址字节自增 */
	hmdmaOut.Init.SourceDataSize  = MDMA_SRC_DATASIZE_WORD;          /* 源地址数据宽度字 */
	hmdmaOut.Init.DestDataSize    = MDMA_DEST_DATASIZE_BYTE;         /* 目的地址数据宽度字节 */
	hmdmaOut.Init.DataAlignment   = MDMA_DATAALIGN_PACKENABLE;       /* 小端，右对齐 */  
	hmdmaOut.Init.SourceBurst     = MDMA_SOURCE_BURST_32BEATS;       /* 源数据突发传输，32次 */
	hmdmaOut.Init.DestBurst       = MDMA_DEST_BURST_32BEATS;         /* 目的数据突发传输，16次 */
	
	hmdmaOut.Init.SourceBlockAddressOffset = 0;  /* 用于block传输，buffer传输用不到 */
	hmdmaOut.Init.DestBlockAddressOffset   = 0;  /* 用于block传输，buffer传输用不到 */

	hmdmaOut.Init.Request              = MDMA_REQUEST_JPEG_OUTFIFO_TH;  /* JPEG的FIFO阀值触发中断 */
	hmdmaOut.Init.TransferTriggerMode  = MDMA_BUFFER_TRANSFER;          /* 使用MDMA的buffer传输 */ 
	hmdmaOut.Init.BufferTransferLength = 32;                            /* 每次传输32个字节，JPEG的FIFO阀值是32字节 */

	/* 先复位，然后配置MDMA */
	HAL_MDMA_DeInit(&hmdmaOut);  
	HAL_MDMA_Init(&hmdmaOut);

	/* 关联MDMA的句柄到JPEG */
	__HAL_LINKDMA(hjpeg, hdmaout, hmdmaOut);

	/* 使能MDMA中断并配置优先级 */
	HAL_NVIC_SetPriority(MDMA_IRQn, 0x08, 0x00);
	HAL_NVIC_EnableIRQ(MDMA_IRQn);
}

/*
*********************************************************************************************************
*	函 数 名: HAL_JPEG_MspDeInit
*	功能说明: 复位JPEG
*	形    参: JPEG_HandleTypeDef句柄指针
*	返 回 值: 无
*********************************************************************************************************
*/
void HAL_JPEG_MspDeInit(JPEG_HandleTypeDef *hjpeg)
{
	/* 关闭MDMA中断 */
	HAL_NVIC_DisableIRQ(MDMA_IRQn);

	/* 复位MDMA的JPEG输入输出 */
	HAL_MDMA_DeInit(hjpeg->hdmain);
	HAL_MDMA_DeInit(hjpeg->hdmaout);
}

/*
*********************************************************************************************************
*	函 数 名: JPEG_Decode_DMA
*	功能说明: JPEG解码
*	形    参: hjpeg               JPEG_HandleTypeDef句柄指针
*             FrameSourceAddress  数据地址
*             FrameSize           数据大小
*             DestAddress         目的数据地址
*	返 回 值: HAL_ERROR表示配置失败，HAL_OK表示配置成功
*             HAL_BUSY表示忙（操作中），HAL_TIMEOUT表示时间溢出
*********************************************************************************************************
*/
uint32_t JPEG_Decode_DMA(JPEG_HandleTypeDef *hjpeg, uint32_t FrameSourceAddress ,uint32_t FrameSize, uint32_t DestAddress)
{
	JPEGSourceAddress =  FrameSourceAddress ;
	FrameBufferAddress = DestAddress;
	Input_frameIndex = 0;
	Input_frameSize = FrameSize;

	/* 设置标志，0表示开始解码，1表示解码完成 */
	Jpeg_HWDecodingEnd = 0;

	/* 启动JPEG解码 */
	HAL_JPEG_Decode_DMA(hjpeg ,(uint8_t *)JPEGSourceAddress ,CHUNK_SIZE_IN ,(uint8_t *)FrameBufferAddress ,CHUNK_SIZE_OUT);

	return HAL_OK;
}

/*
*********************************************************************************************************
*	函 数 名: HAL_JPEG_InfoReadyCallback
*	功能说明: JPEG中断里面的一个回调函数，本功能暂未使用
*	形    参: hjpeg    JPEG_HandleTypeDef 句柄指针
*             pInfo    JPEG_ConfTypeDef   JPEG配置信息
*	返 回 值: 无
*********************************************************************************************************
*/
void HAL_JPEG_InfoReadyCallback(JPEG_HandleTypeDef *hjpeg, JPEG_ConfTypeDef *pInfo)
{  
	
}

/*
*********************************************************************************************************
*	函 数 名: HAL_JPEG_GetDataCallback
*	功能说明: JPEG回调函数，用于从输入地址获取新数据继续解码
*	形    参: hjpeg          JPEG_HandleTypeDef 句柄指针
*             NbDecodedData  上一轮已经解码的数据大小，单位字节  
*	返 回 值: 无
*********************************************************************************************************
*/
void HAL_JPEG_GetDataCallback(JPEG_HandleTypeDef *hjpeg, uint32_t NbDecodedData)
{
	uint32_t inDataLength; 
	
	/* 更新已经解码的数据大小 */
	Input_frameIndex += NbDecodedData;
	
	/* 如果当前已经解码的数据小于总文件大小，继续解码 */
	if( Input_frameIndex < Input_frameSize)
	{
		/* 更新解码数据位置 */
		JPEGSourceAddress = JPEGSourceAddress + NbDecodedData;

		/* 更新下一轮要解码的数据大小 */
		if((Input_frameSize - Input_frameIndex) >= CHUNK_SIZE_IN)
		{
			inDataLength = CHUNK_SIZE_IN;
		}
		else
		{
			inDataLength = Input_frameSize - Input_frameIndex;
		}    
	}
	else
	{
		inDataLength = 0; 
	}
	
	/* 更新输入缓冲 */
	HAL_JPEG_ConfigInputBuffer(hjpeg,(uint8_t *)JPEGSourceAddress, inDataLength);    
}

/*
*********************************************************************************************************
*	函 数 名: HAL_JPEG_DataReadyCallback
*	功能说明: JPEG回调函数，用于输出缓冲地址更新
*	形    参: hjpeg         JPEG_HandleTypeDef 句柄指针
*             pDataOut      输出数据缓冲
*             OutDataLength 输出数据大小，单位字节
*	返 回 值: 无
*********************************************************************************************************
*/
void HAL_JPEG_DataReadyCallback (JPEG_HandleTypeDef *hjpeg, uint8_t *pDataOut, uint32_t OutDataLength)
{
	/* 更新JPEG输出地址 */  
	FrameBufferAddress += OutDataLength;

	HAL_JPEG_ConfigOutputBuffer(hjpeg, (uint8_t *)FrameBufferAddress, CHUNK_SIZE_OUT); 
}

/*
*********************************************************************************************************
*	函 数 名: HAL_JPEG_ErrorCallback
*	功能说明: JPEG错误回调
*	形    参: hjpeg    JPEG_HandleTypeDef 句柄指针
*	返 回 值: 无
*********************************************************************************************************
*/
void HAL_JPEG_ErrorCallback(JPEG_HandleTypeDef *hjpeg)
{
	Error_Handler(__FILE__, __LINE__);
}

/*
*********************************************************************************************************
*	函 数 名: HAL_JPEG_DecodeCpltCallback
*	功能说明: JPEG解码完成中断
*	形    参: hjpeg    JPEG_HandleTypeDef 句柄指针
*	返 回 值: 无
*********************************************************************************************************
*/
void HAL_JPEG_DecodeCpltCallback(JPEG_HandleTypeDef *hjpeg)
{    
	Jpeg_HWDecodingEnd = 1; 
}

/*
*********************************************************************************************************
*	函 数 名: JPEG_IRQHandler
*	功能说明: JPEG中断服务程序
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void JPEG_IRQHandler(void)
{
	HAL_JPEG_IRQHandler(&JPEG_Handle);
}

/*
*********************************************************************************************************
*	函 数 名: MDMA_IRQHandler
*	功能说明: MDMA中断服务程序
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void MDMA_IRQHandler(void)
{
	HAL_MDMA_IRQHandler(JPEG_Handle.hdmain);
	HAL_MDMA_IRQHandler(JPEG_Handle.hdmaout);  
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
