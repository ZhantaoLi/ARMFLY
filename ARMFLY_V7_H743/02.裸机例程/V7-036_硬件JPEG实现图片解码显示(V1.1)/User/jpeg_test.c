/*
*********************************************************************************************************
*
*	ģ������ : JPEG������ԡ�
*	�ļ����� : jpeg_test.c
*	��    �� : V1.0
*	˵    �� : JPEG������ԣ���ʾһ��480*272��С��ͼƬ����ʾ��
*	�޸ļ�¼ :
*		�汾��  ����         ����      ˵��
*		V1.0    2019-05-11  Eric2013   �׷�
*
*	Copyright (C), 2019-2030, ���������� www.armfly.com
*
*********************************************************************************************************
*/
#include "bsp.h"
#include "decode_dma.h"



/* Ҫ������ͼƬ���� */
extern const unsigned char _ac1[27411UL + 1];


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
	uint32_t destination = 0; 
	
	/* ����������ƫ�� */
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

	/* �����ַ���ر�ע��ĩβ����2��ӦRGB565����������ʽ��ARGB8888����Ҫ����4 */
	destination = (uint32_t)pDst + ((y * g_LcdWidth) + x) * 2;
	  
  
	/* DMA2D���ô洢�����洢��ģʽ������ִ��FPC��ɫ��ʽת��, ����ģʽ��ǰ������ΪDMA2D���� */  
	DMA2D->CR      = 0x00010000UL | (1 << 9);
	DMA2D->OOR     = g_LcdWidth - xsize;
	
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
	DMA2D->OMAR    = (uint32_t)destination;
	DMA2D->FGMAR   = (uint32_t)pSrc;  

	/* �������� */
	DMA2D->CR   |= DMA2D_CR_START;   

	/* �ȴ�DMA2D������� */
	while (DMA2D->CR & DMA2D_CR_START) {} 
}

/*
*********************************************************************************************************
*	�� �� ��: TestJpeg
*	����˵��: Ӳ��JPEG����
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void TestJpeg(void)
{
	int iTimeStart, iTimeEnd;
	FONT_T tFont;		/* ����һ������ṹ���������������������� */
	char buf0[100];
	char buf1[100];

	
	/* ������������ */
	tFont.FontCode = FC_ST_16;		/* ����ѡ������16���󣬸�16x��15) */
	tFont.FrontColor = CL_RED;		/* ������ɫ����Ϊ��ɫ */
	tFont.BackColor = CL_MASK;	 	/* ���ֱ�����ɫ��͸�� */
	tFont.Space = 0;				/* �ַ�ˮƽ���, ��λ = ���� */
	
	/* ��1����JPEG��ʼ�� ###########################################*/
	JPEG_Handle.Instance = JPEG;
	HAL_JPEG_Init(&JPEG_Handle);  

	iTimeStart = bsp_GetRunTime();
	JPEG_Decode_DMA(&JPEG_Handle, (uint32_t)_ac1, sizeof(_ac1) , SDRAM_APP_BUF);

	/* ��2�����ȴ�JPEG������� ###########################################*/
	while(Jpeg_HWDecodingEnd == 0){}	
    iTimeEnd = bsp_GetRunTime();
    sprintf(buf0, "STM32H7Ӳ��JPEG����480*272ͼƬʱ��=%dms", iTimeEnd- iTimeStart);		
		
	/* ��3������ȡJPEGͼƬ��Ϣ###########################################*/		
	HAL_JPEG_GetInfo(&JPEG_Handle, &JPEG_Info);       

	/* ��4��������JPEGͼƬ����ʾ��###########################################*/		
    iTimeStart = bsp_GetRunTime();
	DMA2D_Copy_YCbCr_To_RGB((uint32_t *)SDRAM_APP_BUF,  /* JEPG���������� */
	                        (uint32_t *)SDRAM_LCD_BUF1, /* �������Դ��ַ */
	                        0 , 
	                        0, 
	                        JPEG_Info.ImageWidth, 
	                        JPEG_Info.ImageHeight, 
					        LTDC_PIXEL_FORMAT_RGB565,
	                        JPEG_Info.ChromaSubsampling);
	
		
    iTimeEnd = bsp_GetRunTime();
	LCD_DispStr(0, 0, buf0, &tFont);
	
    sprintf(buf1, "STM32H7Ӳ��JPEG��ʾ480*272ͼƬʱ��=%dms", iTimeEnd- iTimeStart);
	LCD_DispStr(0, 18, buf1, &tFont);	
	
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

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
