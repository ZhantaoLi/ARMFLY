/*
*********************************************************************************************************
*
*	模块名称 : JPEG硬件解码文件【原创】
*	文件名称 : JPEGConf.c
*	版    本 : V1.0
*	说    明 : JPEG硬件解码文件，用于emWin的JPEG接口函数
*	修改记录 :
*		版本号   日期         作者       说明
*		V1.0    2019-05-11  Eric2013    正式发布
*
*	Copyright (C), 2019-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/
#include "bsp.h"
#include "stm32h7xx_hal.h"
#include <stdlib.h>
#include "GUI_Private.h"
#include "JPEGConf.h"
#include "decode_dma.h"



/*
*********************************************************************************************************
*	                                       宏定义
*********************************************************************************************************
*/
#define AutoMalloc     0                           /* 0 申请后不释放, 1 使用完毕后释放 */
#define LoadPicSize    1024*600*4                  /* 最大支持的加载的图片大小 */
#define DrawPicSize    1024*600*4                  /* 图片解码出来后，可以使用的缓冲大小 */
#define PicPixelFormat LTDC_PIXEL_FORMAT_RGB565    /* 当前显示屏使用的颜色格式 */

/*
*********************************************************************************************************
*	                                       变量
*********************************************************************************************************
*/
static int _IsInitialized;
static JPEG_X_CONTEXT _Context;
extern __IO uint32_t Jpeg_HWDecodingEnd;
extern JPEG_HandleTypeDef    JPEG_Handle;
extern  JPEG_ConfTypeDef       JPEG_Info;
static DMA2D_HandleTypeDef    DMA2D_Handle;


/*
*********************************************************************************************************
*	函 数 名: _DrawBitmap
*	功能说明: 绘制位图
*	形    参: ----
*	返 回 值: 无
*********************************************************************************************************
*/
static void _DrawBitmap(int x, int y, void const * p, int xSize, int ySize, int BytesPerLine, int BitsPerPixel) 
{
#if (GUI_WINSUPPORT)
	GUI_RECT r;
#endif
	
#if (GUI_WINSUPPORT)
	WM_ADDORG(x,y);
	r.x1 = (r.x0 = x) + xSize-1;
	r.y1 = (r.y0 = y) + ySize-1;
	WM_ITERATE_START(&r) {
#endif
		
	LCD_DrawBitmap(x, y, xSize, ySize, 1, 1, BitsPerPixel, BytesPerLine, p, NULL);
	
#if (GUI_WINSUPPORT)
	} WM_ITERATE_END();
#endif
}

/*
*********************************************************************************************************
*	函 数 名: DMA2D_Copy_YCbCr_To_RGB
*	功能说明: YCbCr转RGB输出
*	形    参: pSrc:    数据源地址
*	          pDst:    数据目的地址
*	          x:       X轴首地址
*	          y:       Y轴首地址 
*	          xsize:   目的区域的X轴大小，即每行像素数
*	          ysize:   目的区域的Y轴大小，即行数
*	          PixelFormat：   目标区颜色格式
*	          ChromaSampling : YCbCr Chroma sampling : 4:2:0, 4:2:2 or 4:4:4  
*	返 回 值: 无
*********************************************************************************************************
*/
static void DMA2D_Copy_YCbCr_To_RGB(uint32_t *pSrc, 
	                                uint32_t *pDst, 
                                    uint16_t x, 
                                    uint16_t y, 
                                    uint16_t xsize, 
                                    uint16_t ysize, 
                                    uint32_t PixelFormat,
                                    uint32_t ChromaSampling)
{   
	uint32_t cssMode = DMA2D_CSS_420;
	uint32_t inputLineOffset = 0;  

	/* 处理输出行偏移 */
	if(ChromaSampling == JPEG_420_SUBSAMPLING)
	{
		cssMode = DMA2D_CSS_420;

		inputLineOffset = xsize % 16;
		if(inputLineOffset != 0)
		{
			inputLineOffset = 16 - inputLineOffset;
		}    
	}
	else if(ChromaSampling == JPEG_444_SUBSAMPLING)
	{
		cssMode = DMA2D_NO_CSS;

		inputLineOffset = xsize % 8;
		if(inputLineOffset != 0)
		{
			inputLineOffset = 8 - inputLineOffset;
		}    
	}
	else if(ChromaSampling == JPEG_422_SUBSAMPLING)
	{
		cssMode = DMA2D_CSS_422;

		inputLineOffset = xsize % 16;
		if(inputLineOffset != 0)
		{
			inputLineOffset = 16 - inputLineOffset;
		}      
	}  

	/* DMA2D采用存储器到存储器模式，并且执行FPC颜色格式转换, 这种模式是前景层作为DMA2D输入 */  
	DMA2D->CR      = 0x00010000UL | (1 << 9);
	DMA2D->OOR     = 0;

	/* 输出格式 */
	DMA2D->OPFCCR  = PixelFormat 
					 | (DMA2D_REGULAR_ALPHA << 20) 
					 | (DMA2D_RB_REGULAR << 21);  

	/* 前景层输入格式 */	
	DMA2D->FGPFCCR = DMA2D_INPUT_YCBCR 
					 | (DMA2D_REPLACE_ALPHA << 16) 
					 | (DMA2D_REGULAR_ALPHA << 20)
					 | (DMA2D_RB_REGULAR << 21)   
					 | (0xFFU << 24)              
					 | (cssMode << 18);		

	DMA2D->FGOR    = inputLineOffset;
	DMA2D->NLR     = (uint32_t)(xsize << 16) | (uint16_t)ysize;      
	DMA2D->OMAR    = (uint32_t)pDst;
	DMA2D->FGMAR   = (uint32_t)pSrc;  

	/* 启动传输 */
	DMA2D->CR   |= DMA2D_CR_START;   

	/* 等待DMA2D传输完成 */
	while (DMA2D->CR & DMA2D_CR_START) {} 
}

/*
*********************************************************************************************************
*	函 数 名: JPEG_X_Draw
*	功能说明: 硬件JPEG绘制
*	形    参: ---
*	返 回 值: 绘制是否成功
*********************************************************************************************************
*/
int JPEG_X_Draw(GUI_GET_DATA_FUNC * pfGetData, void * p, int x0, int y0) 
{
	U8 *ppData;
	GUI_LOCK();

	_Context.xPos      = x0;
	_Context.yPos      = y0;
	_Context.pfGetData = pfGetData;
	_Context.pVoid     = p;
	_Context.Error     = 0;

	/* 初始化硬件JPEG，并申请空间  */
	if (_IsInitialized == 0) 
	{
		_IsInitialized = 1;
		JPEG_Handle.Instance = JPEG;
		HAL_JPEG_Init(&JPEG_Handle);  
		
#if AutoMalloc == 0
		/* 申请一块内存空间，用于加载JPEG图片 */
		_Context.hWorkBuffer = GUI_ALLOC_AllocNoInit(LoadPicSize);
		_Context.pWorkBuffer = GUI_ALLOC_h2p(_Context.hWorkBuffer);

		/* 申请一块内存空间，用于存放解码完成的数据 */
		_Context.hOutBuffer = GUI_ALLOC_AllocNoInit(DrawPicSize);
		_Context.pOutBuffer = GUI_ALLOC_h2p(_Context.hOutBuffer);
#endif
	}

#if AutoMalloc == 1
	/* 申请一块内存空间，用于加载JPEG图片 */
	_Context.hWorkBuffer = GUI_ALLOC_AllocNoInit(LoadPicSize);
	_Context.pWorkBuffer = GUI_ALLOC_h2p(_Context.hWorkBuffer);

	/* 申请一块内存空间，用于存放解码完成的数据 */
	_Context.hOutBuffer = GUI_ALLOC_AllocNoInit(DrawPicSize);
	_Context.pOutBuffer = GUI_ALLOC_h2p(_Context.hOutBuffer);	
#endif
	
	/* 读取JPEG数据，并解码 */
	_Context.NumBytesInBuffer  = _Context.pfGetData(_Context.pVoid, (const U8 **)&ppData, LoadPicSize, 0);
	JPEG_Decode_DMA(&JPEG_Handle, (uint32_t)ppData,  _Context.NumBytesInBuffer, (uint32_t)_Context.pWorkBuffer);

	/* 解码完成 */
	while(Jpeg_HWDecodingEnd == 0)
	{
	}
	
	/* 获取JPEG图片格式信息后，做颜色格式转换 */
	HAL_JPEG_GetInfo(&JPEG_Handle, &JPEG_Info);    
	DMA2D_Copy_YCbCr_To_RGB((uint32_t *)_Context.pWorkBuffer, 
							(uint32_t *)_Context.pOutBuffer , 
							0, 
							0, 
							JPEG_Info.ImageWidth, 
							JPEG_Info.ImageHeight, 
							PicPixelFormat,
							JPEG_Info.ChromaSubsampling);

	/* 绘制JPEG图片 */
	_DrawBitmap(_Context.xPos, _Context.yPos, (void const *)_Context.pOutBuffer , JPEG_Info.ImageWidth, JPEG_Info.ImageHeight, JPEG_Info.ImageWidth*2, 16);

#if AutoMalloc == 1
	/* 释放动态内存hMem */
	GUI_ALLOC_Free(_Context.hWorkBuffer);
	GUI_ALLOC_Free(_Context.hOutBuffer );
#endif
	
	GUI_UNLOCK();
	return _Context.Error;
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
