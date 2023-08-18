/*
*********************************************************************************************************
*
*	模块名称 : 主程序模块
*	文件名称 : main.c
*	版    本 : V1.0
*	说    明 : DMA2D功能测试。
*              实验目的：
*                1. 学习DMA2D显示色块，位图，Alpha混合和图片混合等。
*              实验内容：
*                1. 启动1个200ms的自动重装定时器，让LED2每200ms翻转一次。
*                2. 第1个图：使用DMA2D刷色块。
*                3. 第2个图：显示ARGB8888位图。
*                4. 第3个图：显示RGB565位图。
*                5. 第4个图：两个位图混合。
*                6. 第5个图：Alpha透明度200的位图显示。
*                7. 第6个图：Alpha透明度100的位图显示。
*              注意事项：
*                1. 本实验推荐使用串口软件SecureCRT查看打印信息，波特率115200，数据位8，奇偶校验位无，停止位1。
*                2. 务必将编辑器的缩进参数和TAB设置为4来阅读本文件，要不代码显示不整齐。
*
*	修改记录 :
*		版本号   日期         作者        说明
*		V1.0    2019-04-26   Eric2013     1. CMSIS软包版本 V5.4.0
*                                         2. HAL库版本 V1.3.0
*		V1.1    2020-12-31   Eric2013     1. CMSIS软包版本 V5.7.0
*                                         2. HAL库版本 V1.9.0
*                                         
*	Copyright (C), 2020-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/	
#include "bsp.h"			 /* 底层硬件驱动 */



/* 定义例程名和例程发布日期 */
#define EXAMPLE_NAME	"V7-DMA2D功能测试（显示色块，位图，Alpha混合和图片混合等）"
#define EXAMPLE_DATE	"2019-04-26"
#define DEMO_VER		"1.0"


static void PrintfHelp(void);
static void PrintfLogo(void);

/* DMA2D颜色填充功能 */
static void _DMA2D_Fill(void * pDst, 
	                    uint32_t xSize, 
                        uint32_t ySize, 
                        uint32_t OffLine, 
                        uint32_t ColorIndex, 
                        uint32_t PixelFormat);

/* 通过DMA2D从前景层复制指定区域的颜色数据到目标区域 */
static void _DMA2D_Copy(void * pSrc, 
	                    void * pDst, 
						uint32_t xSize, 
						uint32_t ySize, 
						uint32_t OffLineSrc, 
						uint32_t OffLineDst, 
						uint32_t PixelFormat);

/* 前景层和目标区域的颜色混合 */
static void _DMA2D_MixColorsBulk(uint32_t * pColorFG,  
	                             uint32_t OffLineSrcFG,
                                 uint32_t * pColorDst, 
                                 uint32_t OffLineDst,
							     uint32_t xSize, 
                                 uint32_t ySize, 
                                 uint8_t Intens);

/* 前景层和背景层的颜色混合 */
static void _DMA2D_AlphaBlendingBulk(uint32_t * pColorFG,  
	                                 uint32_t OffLineSrcFG,
	                                 uint32_t * pColorBG,  
                                     uint32_t OffLineSrcBG,
                                     uint32_t * pColorDst, 
                                     uint32_t OffLineDst,
								     uint32_t xSize, 
                                     uint32_t ySize); 
/* ARGB8888格式位图显示 */
static void _DMA2D_DrawAlphaBitmap(void  * pDst, 
	                               void  * pSrc, 
								   uint32_t xSize, 
								   uint32_t ySize, 
								   uint32_t OffLineSrc, 
								   uint32_t OffLineDst, 
								   uint32_t PixelFormat);

/* 图片数据 */
extern const uint32_t _aclufei[128*128*4];
extern const uint32_t _acsuolong[128*128*4];
extern const uint16_t _achuoying[128*128*2];
extern const uint16_t _acmickey[128*128*2];

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
	uint16_t ucBright;	   	/* 背光亮度(0-255) */
    FONT_T tFont;		    /* 定义一个字体结构体变量，用于设置字体参数 */


	/* 设置字体参数 */
	{
		tFont.FontCode = FC_ST_16;		/* 字体代码 16点阵 */
		tFont.FrontColor = CL_WHITE;	/* 字体颜色 */
		tFont.BackColor = CL_BLUE;		/* 文字背景颜色 */
		tFont.Space = 0;				/* 文字间距，单位 = 像素 */
	}	
	
	bsp_Init();		/* 硬件初始化 */
	PrintfLogo();	/* 打印例程名称和版本等信息 */
	PrintfHelp();	/* 打印操作提示 */

	/* 延迟200ms再点亮背光，避免瞬间高亮 */
	bsp_DelayMS(200); 
	
	LCD_ClrScr(CL_BLUE);

	/* 界面整体显示完毕后，再打开背光，设置为缺省亮度 */
	bsp_DelayMS(100); 
	ucBright = BRIGHT_DEFAULT;
	LCD_SetBackLight(ucBright);
	
	/* 第1个图：使用DMA2D刷色块 ##############################################################*/
	LCD_DispStr(24, 2, "DMA2D刷色块", &tFont);
	_DMA2D_Fill((void *)(SDRAM_LCD_BUF1 + g_LcdWidth*20*2 + 24*2), /* 显示起始地址(24, 20) */  
                128,                                               /* 色块长 */  
			    128,                                               /* 色块高 */
			    g_LcdWidth-128,                                    /* 色块行偏移 */
			    CL_RED,                                            /* 色块颜色 */
			    LTDC_PIXEL_FORMAT_RGB565);                         /* 色块颜色格式 */                        

	/* 第2个图：显示ARGB8888位图 ##############################################################*/
	LCD_DispStr(176, 2, "刷ARGB8888位图", &tFont);
    _DMA2D_DrawAlphaBitmap((void *)(SDRAM_LCD_BUF1 + g_LcdWidth*20*2 + 176*2), /* 显示起始地址(176, 20) */  
					   (void *)_aclufei,                                   /* 位图地址 */
					   128,                                                /* 位图长 */
					   128,                                                /* 位图高 */
					   0,                                                  /* 位图行偏移 */
					   g_LcdWidth-128,                                     /* 目标区行偏移 */
					   LTDC_PIXEL_FORMAT_RGB565);                          /* 目标区颜色格式 */

	/* 第3个图：显示RGB565位图 ##############################################################*/
	LCD_DispStr(328, 2, "刷RGB565位图", &tFont);
	_DMA2D_Copy((uint32_t *)_acmickey,                                        /* 位图地址 */
			    (uint32_t *)(SDRAM_LCD_BUF1 + g_LcdWidth*20*2 + 328*2),       /* 显示起始地址(328, 20) */  
			    128,                                                          /* 位图长 */
			    128,                                                          /* 位图高 */
			    0,                                                            /* 位图行偏移 */
			    g_LcdWidth-128,                                               /* 目标区行偏移 */
				LTDC_PIXEL_FORMAT_RGB565);                                    /* 目标区颜色格式 */


	/* 第4个图：两个位图混合 ##############################################################*/
	LCD_DispStr(24, 150, "两个位图混合", &tFont);						 
	_DMA2D_AlphaBlendingBulk((uint32_t *)_aclufei,                           /* 前景层位图地址 */
						     0,                                              /* 前景层行偏移  */  
		                     (uint32_t *)_acsuolong,                         /* 背景层位图地址  */  
							 0,                                              /* 背景层行偏移  */ 
						     (uint32_t *)(SDRAM_LCD_BUF1 +  g_LcdWidth*168*2 + 24*2), /* 显示起始地址(24, 168) */  
						     g_LcdWidth-128,                                 /* 目标区行偏移 */
						     128,                                            /* 目标区长 */
						     128);                                           /* 目标区高 */

	/* 第5个图：Alpha透明度200的位图显示 #######################################################*/
	LCD_DispStr(176, 150, "Alpha透明度200", &tFont);
	_DMA2D_MixColorsBulk((uint32_t *)_achuoying,                                  /* 位图地址 */
	                     0,                                                       /* 位图行偏移 */                     
					     (uint32_t *)(SDRAM_LCD_BUF1 + g_LcdWidth*168*2 + 176*2), /* 显示起始地址(176, 168) */
					     g_LcdWidth-128,                                          /* 目标区行偏移 */                                    
					     128,                                                     /* 目标区长 */
					     128,                                                     /* 目标区高 */          
					     200);                                                    /* 位图显示透明度200 */

	/* 第6个图：Alpha透明度100的位图显示 ####################################################*/
	LCD_DispStr(328, 150, "Alpha透明度100", &tFont);
	_DMA2D_MixColorsBulk((uint32_t *)_achuoying,                                  /* 位图地址 */
	                     0,                                                       /* 位图行偏移 */                     
					     (uint32_t *)(SDRAM_LCD_BUF1 + g_LcdWidth*168*2 + 328*2), /* 显示起始地址(328, 168) */
					     g_LcdWidth-128,                                          /* 目标区行偏移 */                                    
					     128,                                                     /* 目标区长 */
					     128,                                                     /* 目标区高 */          
					     100);                                                    /* 位图显示透明度200 */						 

	bsp_StartAutoTimer(0, 200); /* 启动1个200ms的自动重装的定时器，软件定时器0 */
	
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
*	函 数 名: _DMA2D_Fill
*	功能说明: DMA2D颜色填充功能
*	形    参: pDst          颜色数据目的地址
*             xSize         色块X轴大小，即每行像素数
*             ySize         色块Y轴大小，即行数
*             OffLine       前景层图像的行偏移
*             ColorIndex    色块颜色值
*             PixelFormat   目标区颜色格式
*	返 回 值: 无
*********************************************************************************************************
*/
static void _DMA2D_Fill(void * pDst, 
	                    uint32_t xSize, 
                        uint32_t ySize, 
                        uint32_t OffLine, 
                        uint32_t ColorIndex, 
                        uint32_t PixelFormat) 
{
	
	/* DMA2D采用寄存器到存储器模式, 这种模式用不到前景层和背景层 */  
	DMA2D->CR      = 0x00030000UL | (1 << 9);
	DMA2D->OCOLR   = ColorIndex;
	DMA2D->OMAR    = (uint32_t)pDst;
	DMA2D->OOR     = OffLine;
	DMA2D->OPFCCR  = PixelFormat;
	DMA2D->NLR     = (uint32_t)(xSize << 16) | (uint16_t)ySize;

	/* 启动传输 */
	DMA2D->CR   |= DMA2D_CR_START;   

	/* 等待DMA2D传输完成 */
	while (DMA2D->CR & DMA2D_CR_START) {} 
}

/*
*********************************************************************************************************
*	函 数 名: _DMA2D_Copy
*	功能说明: 通过DMA2D从前景层复制指定区域的颜色数据到目标区域
*	形    参: pSrc          颜色数据源地址
*             pDst          颜色数据目的地址
*             xSize         目的区域的X轴大小，即每行像素数
*             ySize         目的区域的Y轴大小，即行数
*             OffLineSrc    前景层图像的行偏移
*             OffLineDst    输出的行偏移
*             PixelFormat   目标区颜色格式
*	返 回 值: 无
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

	/* DMA2D采用存储器到存储器模式, 这种模式是前景层作为DMA2D输入 */  
	DMA2D->CR      = 0x00000000UL | (1 << 9);
	DMA2D->FGMAR   = (uint32_t)pSrc;
	DMA2D->OMAR    = (uint32_t)pDst;
	DMA2D->FGOR    = OffLineSrc;
	DMA2D->OOR     = OffLineDst;
	
	/* 前景层和输出区域都采用的RGB565颜色格式 */
	DMA2D->FGPFCCR = LTDC_PIXEL_FORMAT_RGB565;
	DMA2D->OPFCCR  = LTDC_PIXEL_FORMAT_RGB565;
	
	DMA2D->NLR     = (uint32_t)(xSize << 16) | (uint16_t)ySize;

	/* 启动传输 */
	DMA2D->CR   |= DMA2D_CR_START;   

	/* 等待DMA2D传输完成 */
	while (DMA2D->CR & DMA2D_CR_START) {} 
}

/*
*********************************************************************************************************
*	函 数 名: _DMA2D_MixColorsBulk
*	功能说明: 前景层和目标区域的颜色混合
*	形    参: pColorFG      前景层数据源地址
*             OffLineSrcFG  前景层图像的行偏移
*             pColorDst     目标区数据地址
*             OffLineDst    目标区的行偏移
*             xSize         目的区域的X轴大小，即每行像素数
*             ySize         目的区域的Y轴大小，即行数
*             Intens        设置前景层的透明度，255表示完全不透明，0表示完全透明
*	返 回 值: 无
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
	/* DMA2D采用存储器到存储器模式, 这种模式前景层和背景层作为DMA2D输入，且支持颜色格式转换和颜色混合 */  
	DMA2D->CR      = 0x00020000UL | (1 << 9);
	DMA2D->FGMAR   = (uint32_t)pColorFG;
	DMA2D->BGMAR   = (uint32_t)pColorDst;
	DMA2D->OMAR    = (uint32_t)pColorDst;
	DMA2D->FGOR    = OffLineSrcFG;
	DMA2D->BGOR    = OffLineDst;
	DMA2D->OOR     = OffLineDst;

	/* 前景层，背景层和输出区都是用的RGB565格式 */
	DMA2D->FGPFCCR = LTDC_PIXEL_FORMAT_RGB565
				 | (1UL << 16)
				 | ((uint32_t)Intens << 24);
	DMA2D->BGPFCCR = LTDC_PIXEL_FORMAT_RGB565;
	DMA2D->OPFCCR  = LTDC_PIXEL_FORMAT_RGB565;

	DMA2D->NLR     = (uint32_t)(xSize << 16) | (uint16_t)ySize;
  
	/* 启动传输 */
	DMA2D->CR   |= DMA2D_CR_START;   

	/* 等待DMA2D传输完成 */
	while (DMA2D->CR & DMA2D_CR_START) {} 
}

/*
*********************************************************************************************************
*	函 数 名: _DMA2D_AlphaBlendingBulk
*	功能说明: 前景层和背景层的颜色混合
*	形    参: pColorFG      前景层源数据地址
*             OffLineSrcFG  前景层源数据行偏移
*             pColorBG      背景层源数据地址
*             OffLineSrcBG  背景层源数据行偏移
*             pColorDst     目标区地址
*             OffLineDst    目标区行偏移
*             xSize         目标区域的X轴大小，即每行像素数
*             ySize         目标区域的Y轴大小，即行数
*	返 回 值: 无
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
	/* DMA2D采用存储器到存储器模式, 这种模式前景层和背景层作为DMA2D输入，且支持颜色格式转换和颜色混合 */  
	DMA2D->CR      = 0x00020000UL | (1 << 9);
	DMA2D->FGMAR   = (uint32_t)pColorFG;
	DMA2D->BGMAR   = (uint32_t)pColorBG;
	DMA2D->OMAR    = (uint32_t)pColorDst;
	DMA2D->FGOR    = OffLineSrcFG;
	DMA2D->BGOR    = OffLineSrcBG;
	DMA2D->OOR     = OffLineDst;

	/* 前景层，背景层采用ARGB8888格式，输出区采用RGB565格式 */
	DMA2D->FGPFCCR = LTDC_PIXEL_FORMAT_ARGB8888;
	DMA2D->BGPFCCR = LTDC_PIXEL_FORMAT_ARGB8888;
	DMA2D->OPFCCR  = LTDC_PIXEL_FORMAT_RGB565;
	DMA2D->NLR     = (uint32_t)(xSize << 16) | (uint16_t)ySize;

	/* 启动传输 */
	DMA2D->CR   |= DMA2D_CR_START;   

	/* 等待DMA2D传输完成 */
	while (DMA2D->CR & DMA2D_CR_START) {} 
}

/*
*********************************************************************************************************
*	函 数 名: _DMA2D_DrawAlphaBitmap
*	功能说明: ARGB8888格式位图显示
*	形    参: pDst        目标区地址
*             pSrc        源数据地址，即位图首地址
*             xSize       目标区域的X轴大小，即每行像素数
*             ySize       目标区域的Y轴大小，即行数
*             OffLineSrc  源数据行偏移
*             OffLineDst  目标区行偏移
*             PixelFormat 目标区颜色格式
*	返 回 值: 无
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
	/* DMA2D采用存储器到存储器模式, 这种模式前景层和背景层作为DMA2D输入，且支持颜色格式转换和颜色混合 */  
	DMA2D->CR      = 0x00020000UL | (1 << 9);
	DMA2D->FGMAR   = (uint32_t)pSrc;
	DMA2D->BGMAR   = (uint32_t)pDst;
	DMA2D->OMAR    = (uint32_t)pDst;
	DMA2D->FGOR    = OffLineSrc;
	DMA2D->BGOR    = OffLineDst;
	DMA2D->OOR     = OffLineDst;
	
	/* 前景层颜色格式是LTDC_PIXEL_FORMAT_ARGB8888，即位图的颜色格式，背景层和输出区颜色格式可配置 */
	DMA2D->FGPFCCR = LTDC_PIXEL_FORMAT_ARGB8888;
	DMA2D->BGPFCCR = PixelFormat;
	DMA2D->OPFCCR  = PixelFormat;
	DMA2D->NLR     = (uint32_t)(xSize << 16) | (uint16_t)ySize;

	/* 启动传输 */
	DMA2D->CR   |= DMA2D_CR_START;   

	/* 等待DMA2D传输完成 */
	while (DMA2D->CR & DMA2D_CR_START) {} 
}

/*
*********************************************************************************************************
*	函 数 名: PrintfHelp
*	功能说明: 打印操作提示
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void PrintfHelp(void)
{
	printf("操作提示:\r\n");
	printf("1. 第1个图：使用DMA2D刷色块\r\n");
	printf("2. 第2个图：显示ARGB8888位图\r\n");
	printf("3. 第3个图：显示RGB565位图\r\n");
	printf("4. 第4个图：两个位图混合\r\n");
	printf("5. 第5个图：Alpha透明度200\r\n");
	printf("6. 第6个图：Alpha透明度100\r\n");
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
	printf("* HAL库版本  : V1.9.0 (STM32H7xx HAL Driver)\r\n");
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
