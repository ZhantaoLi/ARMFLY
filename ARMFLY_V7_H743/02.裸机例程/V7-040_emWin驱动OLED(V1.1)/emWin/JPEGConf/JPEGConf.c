/*
*********************************************************************************************************
*
*	ģ������ : JPEGӲ�������ļ���ԭ����
*	�ļ����� : JPEGConf.c
*	��    �� : V1.0
*	˵    �� : JPEGӲ�������ļ�������emWin��JPEG�ӿں���
*	�޸ļ�¼ :
*		�汾��   ����         ����       ˵��
*		V1.0    2019-05-11  Eric2013    ��ʽ����
*
*	Copyright (C), 2019-2030, ���������� www.armfly.com
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
*	                                       �궨��
*********************************************************************************************************
*/
#define AutoMalloc     0                           /* 0 ������ͷ�, 1 ʹ����Ϻ��ͷ� */
#define LoadPicSize    1024*600*4                  /* ���֧�ֵļ��ص�ͼƬ��С */
#define DrawPicSize    1024*600*4                  /* ͼƬ��������󣬿���ʹ�õĻ����С */
#define PicPixelFormat LTDC_PIXEL_FORMAT_RGB565    /* ��ǰ��ʾ��ʹ�õ���ɫ��ʽ */

/*
*********************************************************************************************************
*	                                       ����
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
*	�� �� ��: _DrawBitmap
*	����˵��: ����λͼ
*	��    ��: ----
*	�� �� ֵ: ��
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
*	�� �� ��: DMA2D_Copy_YCbCr_To_RGB
*	����˵��: YCbCrתRGB���
*	��    ��: pSrc:    ����Դ��ַ
*	          pDst:    ����Ŀ�ĵ�ַ
*	          x:       X���׵�ַ
*	          y:       Y���׵�ַ 
*	          xsize:   Ŀ�������X���С����ÿ��������
*	          ysize:   Ŀ�������Y���С��������
*	          PixelFormat��   Ŀ������ɫ��ʽ
*	          ChromaSampling : YCbCr Chroma sampling : 4:2:0, 4:2:2 or 4:4:4  
*	�� �� ֵ: ��
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

	/* ���������ƫ�� */
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

	/* DMA2D���ô洢�����洢��ģʽ������ִ��FPC��ɫ��ʽת��, ����ģʽ��ǰ������ΪDMA2D���� */  
	DMA2D->CR      = 0x00010000UL | (1 << 9);
	DMA2D->OOR     = 0;

	/* �����ʽ */
	DMA2D->OPFCCR  = PixelFormat 
					 | (DMA2D_REGULAR_ALPHA << 20) 
					 | (DMA2D_RB_REGULAR << 21);  

	/* ǰ���������ʽ */	
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

	/* �������� */
	DMA2D->CR   |= DMA2D_CR_START;   

	/* �ȴ�DMA2D������� */
	while (DMA2D->CR & DMA2D_CR_START) {} 
}

/*
*********************************************************************************************************
*	�� �� ��: JPEG_X_Draw
*	����˵��: Ӳ��JPEG����
*	��    ��: ---
*	�� �� ֵ: �����Ƿ�ɹ�
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

	/* ��ʼ��Ӳ��JPEG��������ռ�  */
	if (_IsInitialized == 0) 
	{
		_IsInitialized = 1;
		JPEG_Handle.Instance = JPEG;
		HAL_JPEG_Init(&JPEG_Handle);  
		
#if AutoMalloc == 0
		/* ����һ���ڴ�ռ䣬���ڼ���JPEGͼƬ */
		_Context.hWorkBuffer = GUI_ALLOC_AllocNoInit(LoadPicSize);
		_Context.pWorkBuffer = GUI_ALLOC_h2p(_Context.hWorkBuffer);

		/* ����һ���ڴ�ռ䣬���ڴ�Ž�����ɵ����� */
		_Context.hOutBuffer = GUI_ALLOC_AllocNoInit(DrawPicSize);
		_Context.pOutBuffer = GUI_ALLOC_h2p(_Context.hOutBuffer);
#endif
	}

#if AutoMalloc == 1
	/* ����һ���ڴ�ռ䣬���ڼ���JPEGͼƬ */
	_Context.hWorkBuffer = GUI_ALLOC_AllocNoInit(LoadPicSize);
	_Context.pWorkBuffer = GUI_ALLOC_h2p(_Context.hWorkBuffer);

	/* ����һ���ڴ�ռ䣬���ڴ�Ž�����ɵ����� */
	_Context.hOutBuffer = GUI_ALLOC_AllocNoInit(DrawPicSize);
	_Context.pOutBuffer = GUI_ALLOC_h2p(_Context.hOutBuffer);	
#endif
	
	/* ��ȡJPEG���ݣ������� */
	_Context.NumBytesInBuffer  = _Context.pfGetData(_Context.pVoid, (const U8 **)&ppData, LoadPicSize, 0);
	JPEG_Decode_DMA(&JPEG_Handle, (uint32_t)ppData,  _Context.NumBytesInBuffer, (uint32_t)_Context.pWorkBuffer);

	/* ������� */
	while(Jpeg_HWDecodingEnd == 0)
	{
	}
	
	/* ��ȡJPEGͼƬ��ʽ��Ϣ������ɫ��ʽת�� */
	HAL_JPEG_GetInfo(&JPEG_Handle, &JPEG_Info);    
	DMA2D_Copy_YCbCr_To_RGB((uint32_t *)_Context.pWorkBuffer, 
							(uint32_t *)_Context.pOutBuffer , 
							0, 
							0, 
							JPEG_Info.ImageWidth, 
							JPEG_Info.ImageHeight, 
							PicPixelFormat,
							JPEG_Info.ChromaSubsampling);

	/* ����JPEGͼƬ */
	_DrawBitmap(_Context.xPos, _Context.yPos, (void const *)_Context.pOutBuffer , JPEG_Info.ImageWidth, JPEG_Info.ImageHeight, JPEG_Info.ImageWidth*2, 16);

#if AutoMalloc == 1
	/* �ͷŶ�̬�ڴ�hMem */
	GUI_ALLOC_Free(_Context.hWorkBuffer);
	GUI_ALLOC_Free(_Context.hOutBuffer );
#endif
	
	GUI_UNLOCK();
	return _Context.Error;
}

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
