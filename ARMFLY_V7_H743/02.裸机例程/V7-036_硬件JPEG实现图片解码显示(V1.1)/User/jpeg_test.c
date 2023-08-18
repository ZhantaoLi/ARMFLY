/*
*********************************************************************************************************
*
*	模块名称 : JPEG解码测试。
*	文件名称 : jpeg_test.c
*	版    本 : V1.0
*	说    明 : JPEG解码测试，显示一张480*272大小的图片到显示屏
*	修改记录 :
*		版本号  日期         作者      说明
*		V1.0    2019-05-11  Eric2013   首发
*
*	Copyright (C), 2019-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/
#include "bsp.h"
#include "decode_dma.h"



/* 要解析的图片数据 */
extern const unsigned char _ac1[27411UL + 1];


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
	uint32_t destination = 0; 
	
	/* 处理输入行偏移 */
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

	/* 输出地址，特别注意末尾乘以2对应RGB565，如果输出格式是ARGB8888，需要乘以4 */
	destination = (uint32_t)pDst + ((y * g_LcdWidth) + x) * 2;
	  
  
	/* DMA2D采用存储器到存储器模式，并且执行FPC颜色格式转换, 这种模式是前景层作为DMA2D输入 */  
	DMA2D->CR      = 0x00010000UL | (1 << 9);
	DMA2D->OOR     = g_LcdWidth - xsize;
	
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
	DMA2D->OMAR    = (uint32_t)destination;
	DMA2D->FGMAR   = (uint32_t)pSrc;  

	/* 启动传输 */
	DMA2D->CR   |= DMA2D_CR_START;   

	/* 等待DMA2D传输完成 */
	while (DMA2D->CR & DMA2D_CR_START) {} 
}

/*
*********************************************************************************************************
*	函 数 名: TestJpeg
*	功能说明: 硬件JPEG测试
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void TestJpeg(void)
{
	int iTimeStart, iTimeEnd;
	FONT_T tFont;		/* 定义一个字体结构体变量，用于设置字体参数 */
	char buf0[100];
	char buf1[100];

	
	/* 设置字体属性 */
	tFont.FontCode = FC_ST_16;		/* 字体选择宋体16点阵，高16x宽15) */
	tFont.FrontColor = CL_RED;		/* 字体颜色设置为红色 */
	tFont.BackColor = CL_MASK;	 	/* 文字背景颜色，透明 */
	tFont.Space = 0;				/* 字符水平间距, 单位 = 像素 */
	
	/* 第1步：JPEG初始化 ###########################################*/
	JPEG_Handle.Instance = JPEG;
	HAL_JPEG_Init(&JPEG_Handle);  

	iTimeStart = bsp_GetRunTime();
	JPEG_Decode_DMA(&JPEG_Handle, (uint32_t)_ac1, sizeof(_ac1) , SDRAM_APP_BUF);

	/* 第2步：等待JPEG解码完成 ###########################################*/
	while(Jpeg_HWDecodingEnd == 0){}	
    iTimeEnd = bsp_GetRunTime();
    sprintf(buf0, "STM32H7硬件JPEG解码480*272图片时间=%dms", iTimeEnd- iTimeStart);		
		
	/* 第3步：获取JPEG图片信息###########################################*/		
	HAL_JPEG_GetInfo(&JPEG_Handle, &JPEG_Info);       

	/* 第4步：绘制JPEG图片到显示屏###########################################*/		
    iTimeStart = bsp_GetRunTime();
	DMA2D_Copy_YCbCr_To_RGB((uint32_t *)SDRAM_APP_BUF,  /* JEPG解码后的数据 */
	                        (uint32_t *)SDRAM_LCD_BUF1, /* 这里是显存地址 */
	                        0 , 
	                        0, 
	                        JPEG_Info.ImageWidth, 
	                        JPEG_Info.ImageHeight, 
					        LTDC_PIXEL_FORMAT_RGB565,
	                        JPEG_Info.ChromaSubsampling);
	
		
    iTimeEnd = bsp_GetRunTime();
	LCD_DispStr(0, 0, buf0, &tFont);
	
    sprintf(buf1, "STM32H7硬件JPEG显示480*272图片时间=%dms", iTimeEnd- iTimeStart);
	LCD_DispStr(0, 18, buf1, &tFont);	
	
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

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
