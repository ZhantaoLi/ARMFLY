/*
*********************************************************************************************************
*
*	ģ������ : ������ģ��
*	�ļ����� : main.c
*	��    �� : V1.0
*	˵    �� : DMA2D���ܲ��ԡ�
*              ʵ��Ŀ�ģ�
*                1. ѧϰDMA2D��ʾɫ�飬λͼ��Alpha��Ϻ�ͼƬ��ϵȡ�
*              ʵ�����ݣ�
*                1. ����1��200ms���Զ���װ��ʱ������LED2ÿ200ms��תһ�Ρ�
*                2. ��1��ͼ��ʹ��DMA2Dˢɫ�顣
*                3. ��2��ͼ����ʾARGB8888λͼ��
*                4. ��3��ͼ����ʾRGB565λͼ��
*                5. ��4��ͼ������λͼ��ϡ�
*                6. ��5��ͼ��Alpha͸����200��λͼ��ʾ��
*                7. ��6��ͼ��Alpha͸����100��λͼ��ʾ��
*              ע�����
*                1. ��ʵ���Ƽ�ʹ�ô������SecureCRT�鿴��ӡ��Ϣ��������115200������λ8����żУ��λ�ޣ�ֹͣλ1��
*                2. ��ؽ��༭��������������TAB����Ϊ4���Ķ����ļ���Ҫ��������ʾ�����롣
*
*	�޸ļ�¼ :
*		�汾��   ����         ����        ˵��
*		V1.0    2019-04-26   Eric2013     1. CMSIS����汾 V5.4.0
*                                         2. HAL��汾 V1.3.0
*		V1.1    2020-12-31   Eric2013     1. CMSIS����汾 V5.7.0
*                                         2. HAL��汾 V1.9.0
*                                         
*	Copyright (C), 2020-2030, ���������� www.armfly.com
*
*********************************************************************************************************
*/	
#include "bsp.h"			 /* �ײ�Ӳ������ */



/* ���������������̷������� */
#define EXAMPLE_NAME	"V7-DMA2D���ܲ��ԣ���ʾɫ�飬λͼ��Alpha��Ϻ�ͼƬ��ϵȣ�"
#define EXAMPLE_DATE	"2019-04-26"
#define DEMO_VER		"1.0"


static void PrintfHelp(void);
static void PrintfLogo(void);

/* DMA2D��ɫ��书�� */
static void _DMA2D_Fill(void * pDst, 
	                    uint32_t xSize, 
                        uint32_t ySize, 
                        uint32_t OffLine, 
                        uint32_t ColorIndex, 
                        uint32_t PixelFormat);

/* ͨ��DMA2D��ǰ���㸴��ָ���������ɫ���ݵ�Ŀ������ */
static void _DMA2D_Copy(void * pSrc, 
	                    void * pDst, 
						uint32_t xSize, 
						uint32_t ySize, 
						uint32_t OffLineSrc, 
						uint32_t OffLineDst, 
						uint32_t PixelFormat);

/* ǰ�����Ŀ���������ɫ��� */
static void _DMA2D_MixColorsBulk(uint32_t * pColorFG,  
	                             uint32_t OffLineSrcFG,
                                 uint32_t * pColorDst, 
                                 uint32_t OffLineDst,
							     uint32_t xSize, 
                                 uint32_t ySize, 
                                 uint8_t Intens);

/* ǰ����ͱ��������ɫ��� */
static void _DMA2D_AlphaBlendingBulk(uint32_t * pColorFG,  
	                                 uint32_t OffLineSrcFG,
	                                 uint32_t * pColorBG,  
                                     uint32_t OffLineSrcBG,
                                     uint32_t * pColorDst, 
                                     uint32_t OffLineDst,
								     uint32_t xSize, 
                                     uint32_t ySize); 
/* ARGB8888��ʽλͼ��ʾ */
static void _DMA2D_DrawAlphaBitmap(void  * pDst, 
	                               void  * pSrc, 
								   uint32_t xSize, 
								   uint32_t ySize, 
								   uint32_t OffLineSrc, 
								   uint32_t OffLineDst, 
								   uint32_t PixelFormat);

/* ͼƬ���� */
extern const uint32_t _aclufei[128*128*4];
extern const uint32_t _acsuolong[128*128*4];
extern const uint16_t _achuoying[128*128*2];
extern const uint16_t _acmickey[128*128*2];

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
	uint16_t ucBright;	   	/* ��������(0-255) */
    FONT_T tFont;		    /* ����һ������ṹ���������������������� */


	/* ����������� */
	{
		tFont.FontCode = FC_ST_16;		/* ������� 16���� */
		tFont.FrontColor = CL_WHITE;	/* ������ɫ */
		tFont.BackColor = CL_BLUE;		/* ���ֱ�����ɫ */
		tFont.Space = 0;				/* ���ּ�࣬��λ = ���� */
	}	
	
	bsp_Init();		/* Ӳ����ʼ�� */
	PrintfLogo();	/* ��ӡ�������ƺͰ汾����Ϣ */
	PrintfHelp();	/* ��ӡ������ʾ */

	/* �ӳ�200ms�ٵ������⣬����˲����� */
	bsp_DelayMS(200); 
	
	LCD_ClrScr(CL_BLUE);

	/* ����������ʾ��Ϻ��ٴ򿪱��⣬����Ϊȱʡ���� */
	bsp_DelayMS(100); 
	ucBright = BRIGHT_DEFAULT;
	LCD_SetBackLight(ucBright);
	
	/* ��1��ͼ��ʹ��DMA2Dˢɫ�� ##############################################################*/
	LCD_DispStr(24, 2, "DMA2Dˢɫ��", &tFont);
	_DMA2D_Fill((void *)(SDRAM_LCD_BUF1 + g_LcdWidth*20*2 + 24*2), /* ��ʾ��ʼ��ַ(24, 20) */  
                128,                                               /* ɫ�鳤 */  
			    128,                                               /* ɫ��� */
			    g_LcdWidth-128,                                    /* ɫ����ƫ�� */
			    CL_RED,                                            /* ɫ����ɫ */
			    LTDC_PIXEL_FORMAT_RGB565);                         /* ɫ����ɫ��ʽ */                        

	/* ��2��ͼ����ʾARGB8888λͼ ##############################################################*/
	LCD_DispStr(176, 2, "ˢARGB8888λͼ", &tFont);
    _DMA2D_DrawAlphaBitmap((void *)(SDRAM_LCD_BUF1 + g_LcdWidth*20*2 + 176*2), /* ��ʾ��ʼ��ַ(176, 20) */  
					   (void *)_aclufei,                                   /* λͼ��ַ */
					   128,                                                /* λͼ�� */
					   128,                                                /* λͼ�� */
					   0,                                                  /* λͼ��ƫ�� */
					   g_LcdWidth-128,                                     /* Ŀ������ƫ�� */
					   LTDC_PIXEL_FORMAT_RGB565);                          /* Ŀ������ɫ��ʽ */

	/* ��3��ͼ����ʾRGB565λͼ ##############################################################*/
	LCD_DispStr(328, 2, "ˢRGB565λͼ", &tFont);
	_DMA2D_Copy((uint32_t *)_acmickey,                                        /* λͼ��ַ */
			    (uint32_t *)(SDRAM_LCD_BUF1 + g_LcdWidth*20*2 + 328*2),       /* ��ʾ��ʼ��ַ(328, 20) */  
			    128,                                                          /* λͼ�� */
			    128,                                                          /* λͼ�� */
			    0,                                                            /* λͼ��ƫ�� */
			    g_LcdWidth-128,                                               /* Ŀ������ƫ�� */
				LTDC_PIXEL_FORMAT_RGB565);                                    /* Ŀ������ɫ��ʽ */


	/* ��4��ͼ������λͼ��� ##############################################################*/
	LCD_DispStr(24, 150, "����λͼ���", &tFont);						 
	_DMA2D_AlphaBlendingBulk((uint32_t *)_aclufei,                           /* ǰ����λͼ��ַ */
						     0,                                              /* ǰ������ƫ��  */  
		                     (uint32_t *)_acsuolong,                         /* ������λͼ��ַ  */  
							 0,                                              /* ��������ƫ��  */ 
						     (uint32_t *)(SDRAM_LCD_BUF1 +  g_LcdWidth*168*2 + 24*2), /* ��ʾ��ʼ��ַ(24, 168) */  
						     g_LcdWidth-128,                                 /* Ŀ������ƫ�� */
						     128,                                            /* Ŀ������ */
						     128);                                           /* Ŀ������ */

	/* ��5��ͼ��Alpha͸����200��λͼ��ʾ #######################################################*/
	LCD_DispStr(176, 150, "Alpha͸����200", &tFont);
	_DMA2D_MixColorsBulk((uint32_t *)_achuoying,                                  /* λͼ��ַ */
	                     0,                                                       /* λͼ��ƫ�� */                     
					     (uint32_t *)(SDRAM_LCD_BUF1 + g_LcdWidth*168*2 + 176*2), /* ��ʾ��ʼ��ַ(176, 168) */
					     g_LcdWidth-128,                                          /* Ŀ������ƫ�� */                                    
					     128,                                                     /* Ŀ������ */
					     128,                                                     /* Ŀ������ */          
					     200);                                                    /* λͼ��ʾ͸����200 */

	/* ��6��ͼ��Alpha͸����100��λͼ��ʾ ####################################################*/
	LCD_DispStr(328, 150, "Alpha͸����100", &tFont);
	_DMA2D_MixColorsBulk((uint32_t *)_achuoying,                                  /* λͼ��ַ */
	                     0,                                                       /* λͼ��ƫ�� */                     
					     (uint32_t *)(SDRAM_LCD_BUF1 + g_LcdWidth*168*2 + 328*2), /* ��ʾ��ʼ��ַ(328, 168) */
					     g_LcdWidth-128,                                          /* Ŀ������ƫ�� */                                    
					     128,                                                     /* Ŀ������ */
					     128,                                                     /* Ŀ������ */          
					     100);                                                    /* λͼ��ʾ͸����200 */						 

	bsp_StartAutoTimer(0, 200); /* ����1��200ms���Զ���װ�Ķ�ʱ���������ʱ��0 */
	
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
*	�� �� ��: _DMA2D_Fill
*	����˵��: DMA2D��ɫ��书��
*	��    ��: pDst          ��ɫ����Ŀ�ĵ�ַ
*             xSize         ɫ��X���С����ÿ��������
*             ySize         ɫ��Y���С��������
*             OffLine       ǰ����ͼ�����ƫ��
*             ColorIndex    ɫ����ɫֵ
*             PixelFormat   Ŀ������ɫ��ʽ
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void _DMA2D_Fill(void * pDst, 
	                    uint32_t xSize, 
                        uint32_t ySize, 
                        uint32_t OffLine, 
                        uint32_t ColorIndex, 
                        uint32_t PixelFormat) 
{
	
	/* DMA2D���üĴ������洢��ģʽ, ����ģʽ�ò���ǰ����ͱ����� */  
	DMA2D->CR      = 0x00030000UL | (1 << 9);
	DMA2D->OCOLR   = ColorIndex;
	DMA2D->OMAR    = (uint32_t)pDst;
	DMA2D->OOR     = OffLine;
	DMA2D->OPFCCR  = PixelFormat;
	DMA2D->NLR     = (uint32_t)(xSize << 16) | (uint16_t)ySize;

	/* �������� */
	DMA2D->CR   |= DMA2D_CR_START;   

	/* �ȴ�DMA2D������� */
	while (DMA2D->CR & DMA2D_CR_START) {} 
}

/*
*********************************************************************************************************
*	�� �� ��: _DMA2D_Copy
*	����˵��: ͨ��DMA2D��ǰ���㸴��ָ���������ɫ���ݵ�Ŀ������
*	��    ��: pSrc          ��ɫ����Դ��ַ
*             pDst          ��ɫ����Ŀ�ĵ�ַ
*             xSize         Ŀ�������X���С����ÿ��������
*             ySize         Ŀ�������Y���С��������
*             OffLineSrc    ǰ����ͼ�����ƫ��
*             OffLineDst    �������ƫ��
*             PixelFormat   Ŀ������ɫ��ʽ
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void _DMA2D_Copy(void * pSrc, 
	                    void * pDst, 
						uint32_t xSize, 
						uint32_t ySize, 
						uint32_t OffLineSrc, 
						uint32_t OffLineDst, 
						uint32_t PixelFormat) 
{

	/* DMA2D���ô洢�����洢��ģʽ, ����ģʽ��ǰ������ΪDMA2D���� */  
	DMA2D->CR      = 0x00000000UL | (1 << 9);
	DMA2D->FGMAR   = (uint32_t)pSrc;
	DMA2D->OMAR    = (uint32_t)pDst;
	DMA2D->FGOR    = OffLineSrc;
	DMA2D->OOR     = OffLineDst;
	
	/* ǰ�����������򶼲��õ�RGB565��ɫ��ʽ */
	DMA2D->FGPFCCR = LTDC_PIXEL_FORMAT_RGB565;
	DMA2D->OPFCCR  = LTDC_PIXEL_FORMAT_RGB565;
	
	DMA2D->NLR     = (uint32_t)(xSize << 16) | (uint16_t)ySize;

	/* �������� */
	DMA2D->CR   |= DMA2D_CR_START;   

	/* �ȴ�DMA2D������� */
	while (DMA2D->CR & DMA2D_CR_START) {} 
}

/*
*********************************************************************************************************
*	�� �� ��: _DMA2D_MixColorsBulk
*	����˵��: ǰ�����Ŀ���������ɫ���
*	��    ��: pColorFG      ǰ��������Դ��ַ
*             OffLineSrcFG  ǰ����ͼ�����ƫ��
*             pColorDst     Ŀ�������ݵ�ַ
*             OffLineDst    Ŀ��������ƫ��
*             xSize         Ŀ�������X���С����ÿ��������
*             ySize         Ŀ�������Y���С��������
*             Intens        ����ǰ�����͸���ȣ�255��ʾ��ȫ��͸����0��ʾ��ȫ͸��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void _DMA2D_MixColorsBulk(uint32_t * pColorFG,  
	                             uint32_t OffLineSrcFG,
                                 uint32_t * pColorDst, 
                                 uint32_t OffLineDst,
							     uint32_t xSize, 
                                 uint32_t ySize, 
                                 uint8_t Intens)
{
	/* DMA2D���ô洢�����洢��ģʽ, ����ģʽǰ����ͱ�������ΪDMA2D���룬��֧����ɫ��ʽת������ɫ��� */  
	DMA2D->CR      = 0x00020000UL | (1 << 9);
	DMA2D->FGMAR   = (uint32_t)pColorFG;
	DMA2D->BGMAR   = (uint32_t)pColorDst;
	DMA2D->OMAR    = (uint32_t)pColorDst;
	DMA2D->FGOR    = OffLineSrcFG;
	DMA2D->BGOR    = OffLineDst;
	DMA2D->OOR     = OffLineDst;

	/* ǰ���㣬�����������������õ�RGB565��ʽ */
	DMA2D->FGPFCCR = LTDC_PIXEL_FORMAT_RGB565
				 | (1UL << 16)
				 | ((uint32_t)Intens << 24);
	DMA2D->BGPFCCR = LTDC_PIXEL_FORMAT_RGB565;
	DMA2D->OPFCCR  = LTDC_PIXEL_FORMAT_RGB565;

	DMA2D->NLR     = (uint32_t)(xSize << 16) | (uint16_t)ySize;
  
	/* �������� */
	DMA2D->CR   |= DMA2D_CR_START;   

	/* �ȴ�DMA2D������� */
	while (DMA2D->CR & DMA2D_CR_START) {} 
}

/*
*********************************************************************************************************
*	�� �� ��: _DMA2D_AlphaBlendingBulk
*	����˵��: ǰ����ͱ��������ɫ���
*	��    ��: pColorFG      ǰ����Դ���ݵ�ַ
*             OffLineSrcFG  ǰ����Դ������ƫ��
*             pColorBG      ������Դ���ݵ�ַ
*             OffLineSrcBG  ������Դ������ƫ��
*             pColorDst     Ŀ������ַ
*             OffLineDst    Ŀ������ƫ��
*             xSize         Ŀ�������X���С����ÿ��������
*             ySize         Ŀ�������Y���С��������
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void _DMA2D_AlphaBlendingBulk(uint32_t * pColorFG,  
	                                 uint32_t OffLineSrcFG,
	                                 uint32_t * pColorBG,  
                                     uint32_t OffLineSrcBG,
                                     uint32_t * pColorDst, 
                                     uint32_t OffLineDst,
								     uint32_t xSize, 
                                     uint32_t ySize) 
{  
	/* DMA2D���ô洢�����洢��ģʽ, ����ģʽǰ����ͱ�������ΪDMA2D���룬��֧����ɫ��ʽת������ɫ��� */  
	DMA2D->CR      = 0x00020000UL | (1 << 9);
	DMA2D->FGMAR   = (uint32_t)pColorFG;
	DMA2D->BGMAR   = (uint32_t)pColorBG;
	DMA2D->OMAR    = (uint32_t)pColorDst;
	DMA2D->FGOR    = OffLineSrcFG;
	DMA2D->BGOR    = OffLineSrcBG;
	DMA2D->OOR     = OffLineDst;

	/* ǰ���㣬���������ARGB8888��ʽ�����������RGB565��ʽ */
	DMA2D->FGPFCCR = LTDC_PIXEL_FORMAT_ARGB8888;
	DMA2D->BGPFCCR = LTDC_PIXEL_FORMAT_ARGB8888;
	DMA2D->OPFCCR  = LTDC_PIXEL_FORMAT_RGB565;
	DMA2D->NLR     = (uint32_t)(xSize << 16) | (uint16_t)ySize;

	/* �������� */
	DMA2D->CR   |= DMA2D_CR_START;   

	/* �ȴ�DMA2D������� */
	while (DMA2D->CR & DMA2D_CR_START) {} 
}

/*
*********************************************************************************************************
*	�� �� ��: _DMA2D_DrawAlphaBitmap
*	����˵��: ARGB8888��ʽλͼ��ʾ
*	��    ��: pDst        Ŀ������ַ
*             pSrc        Դ���ݵ�ַ����λͼ�׵�ַ
*             xSize       Ŀ�������X���С����ÿ��������
*             ySize       Ŀ�������Y���С��������
*             OffLineSrc  Դ������ƫ��
*             OffLineDst  Ŀ������ƫ��
*             PixelFormat Ŀ������ɫ��ʽ
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void _DMA2D_DrawAlphaBitmap(void  * pDst, 
	                               void  * pSrc, 
								   uint32_t xSize, 
								   uint32_t ySize, 
								   uint32_t OffLineSrc, 
								   uint32_t OffLineDst, 
								   uint32_t PixelFormat) 
{
	/* DMA2D���ô洢�����洢��ģʽ, ����ģʽǰ����ͱ�������ΪDMA2D���룬��֧����ɫ��ʽת������ɫ��� */  
	DMA2D->CR      = 0x00020000UL | (1 << 9);
	DMA2D->FGMAR   = (uint32_t)pSrc;
	DMA2D->BGMAR   = (uint32_t)pDst;
	DMA2D->OMAR    = (uint32_t)pDst;
	DMA2D->FGOR    = OffLineSrc;
	DMA2D->BGOR    = OffLineDst;
	DMA2D->OOR     = OffLineDst;
	
	/* ǰ������ɫ��ʽ��LTDC_PIXEL_FORMAT_ARGB8888����λͼ����ɫ��ʽ����������������ɫ��ʽ������ */
	DMA2D->FGPFCCR = LTDC_PIXEL_FORMAT_ARGB8888;
	DMA2D->BGPFCCR = PixelFormat;
	DMA2D->OPFCCR  = PixelFormat;
	DMA2D->NLR     = (uint32_t)(xSize << 16) | (uint16_t)ySize;

	/* �������� */
	DMA2D->CR   |= DMA2D_CR_START;   

	/* �ȴ�DMA2D������� */
	while (DMA2D->CR & DMA2D_CR_START) {} 
}

/*
*********************************************************************************************************
*	�� �� ��: PrintfHelp
*	����˵��: ��ӡ������ʾ
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void PrintfHelp(void)
{
	printf("������ʾ:\r\n");
	printf("1. ��1��ͼ��ʹ��DMA2Dˢɫ��\r\n");
	printf("2. ��2��ͼ����ʾARGB8888λͼ\r\n");
	printf("3. ��3��ͼ����ʾRGB565λͼ\r\n");
	printf("4. ��4��ͼ������λͼ���\r\n");
	printf("5. ��5��ͼ��Alpha͸����200\r\n");
	printf("6. ��6��ͼ��Alpha͸����100\r\n");
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
	printf("* HAL��汾  : V1.9.0 (STM32H7xx HAL Driver)\r\n");
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
