/*
*********************************************************************************************************
*
*	模块名称 : emWin的底层驱动文件
*	文件名称 : LCDConf_Lin_Template.c
*	版    本 : V1.0 
*	说    明 : LCD的部分驱动和emWin底层接口都在这个文件实现。
*              使用说明：
*                1. 此驱动自适应安富莱4.3寸，5寸和7寸屏。
*                2. 用户使用emWin前，请先使能STM32的CRC时钟，之后调用GUI_Init()即可使用emWin。
*                3. 不同的显示屏对用的触摸驱动不同，用户自己使用的话，请周期性调用即可。
*                   电阻屏是：TOUCH_Scan()，1ms调用一次。
*                   电容屏是：GT911_OnePiontScan()，GT811_OnePiontScan()或者FT5X06_OnePiontScan()，10ms调用一次。
*              配置说明：
*                1. H7的图层是由背景层，图层1和图层2组成。
*                2. 程序中对每个配置选项都有详细说明。
*              移植说明：
*                  注意以下三点即可：
*                1. 提供函数LCD_SetBackLight，实现背光的开关。
*                2. 定义全局变量g_LcdWidth和g_LcdHeight。
*                3. 修改函数LCD_ConfigLTDC，修改本文件中函数LCD_LL_Init里面的时序参数。
*
*	修改记录 :
*		版本号    日期         作者        说明
*		V1.0    2019-05-25    Eric2013     首发
*
*
*	Copyright (C), 2019-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/
#include "bsp.h"
#include "GUI.h"
#include "GUI_Private.h"
#include "GUIDRV_Lin.h"



/*
**********************************************************************************************************
							调用外部文件的变量和函数(bsp_tft_429.c文件)
**********************************************************************************************************
*/
typedef struct
{
  int32_t      address;          
  __IO int32_t      pending_buffer;   
  int32_t      buffer_index;     
  int32_t      xSize;            
  int32_t      ySize;            
  int32_t      BytesPerPixel;
  LCD_API_COLOR_CONV   *pColorConvAPI;
}LCD_LayerPropTypedef;

/*
**********************************************************************************************************
									用户可以配置的选项
**********************************************************************************************************
*/
/* 0 暂时用不到这几个功能 */
#define DrawBitmapA4Enalbe    0
#define ClearCacheHookEnalbe  0

/* 
  1. 显示屏的物理分辨率，驱动已经做了显示屏自适应，支持4.3寸，5寸和7寸屏
     这里填写自适应显示屏中的最大分辨率。
*/
#define XSIZE_PHYS       800
#define YSIZE_PHYS       480

/* 2. 旋转方向，暂未使用此选项，直接在工程运行时做旋转即可 */
#define ROTATION_0       0
#define ROTATION_CW      1
#define ROTATION_180     2
#define ROTATION_CCW     3

/* 
   3. STM32H7支持的颜色模式。
      (1) 如果打算使用24位色或者32位色，请选择CMS_ARGB8888，如果使用16位色，请选择CMS_RGB565，其它颜色格式不再做支持。
	  (2) 如果用户选择了ARGB8888或者RGB888模式，LCD闪烁的话，优先查看是否是此贴的问题：
	      http://forum.armfly.com/forum.php?mod=viewthread&tid=16892 （适用于F429和H7系列）
		  如果不是这个问题，再降低LTDC输出时钟即可，在本文件的函数LCD_ConfigLTDC里面    
*/
#define CMS_ARGB8888 1
#define CMS_RGB888   2
#define CMS_RGB565   3
#define CMS_ARGB1555 4
#define CMS_ARGB4444 5
#define CMS_L8       6
#define CMS_AL44     7
#define CMS_AL88     8

/* 4. 多缓冲 / 虚拟屏，多缓冲和虚拟屏不可同时使用，emWin不支持 */
#define NUM_BUFFERS      3 /* 定义多缓冲个数，仅可以设置1,2和3，也就是最大支持三缓冲 */
#define NUM_VSCREENS     1 /* 定义虚拟屏个数 */

/* 
   5. 重定义图层数，对于STM32H7，用户可以选择一个图层或者两个图层，不支持三图层 
      (1). 设置GUI_NUM_LAYERS = 1时，即仅使用图层1时，默认触摸值是发送给图层1的。
	  (2). 设置GUI_NUM_LAYERS = 2时，即图层1和图层2都已经使能，此时图层2是顶层，
	       用户需要根据自己的使用情况设置如下两个地方。
		   a. 在bsp_touch.c文件中的函数TOUCH_InitHard里面设置参数State.Layer = 1，1就表示
		      给图层2发送触摸值。
		   b. 调用GUI_Init函数后，调用函数GUI_SelectLayer(1), 设置当前操作的是图层2。
*/
#undef  GUI_NUM_LAYERS
#define GUI_NUM_LAYERS 1

/* 
   6. 设置图层1和图层2对应的显存地址
      (1) EXT_SDRAM_ADDR 是SDRAM的首地址。
      (2) LCD_LAYER0_FRAME_BUFFER 是图层1的显存地址。
	  (3) LCD_LAYER1_FRAME_BUFFER 是图层2的显存地址。
	  (4) 每个图层的显存大小比较考究，这里进行下简单的说明。
	      如果用户选择的颜色模式 = 32位色ARGB8888，显存的大小：
	      XSIZE_PHYS * YSIZE_PHYS * 4 * NUM_VSCREENS * NUM_BUFFERS
		  
	      颜色模式 = 24位色RGB888，显存的大小：
	      XSIZE_PHYS * YSIZE_PHYS * 3 * NUM_VSCREENS * NUM_BUFFERS
		  
	      颜色模式 = 16位色RGB566，ARGB1555, ARGB4444，AL88，那么显存的大小就是：
	      XSIZE_PHYS * YSIZE_PHYS * 2 * NUM_VSCREENS * NUM_BUFFERS

	      颜色模式 = 8位色L8，AL44，那么显存的大小就是：
	      XSIZE_PHYS * YSIZE_PHYS * 1 * NUM_VSCREENS * NUM_BUFFERS	
      
      这里为了方便起见，将开发板配套的16MB的SDRAM前8MB分配给LCD显存使用，后8MB用于emWin动态内存。
	  对于24位色，16位色，8位色，用户可以对其使能三缓冲，并且使能双图层。但是32位色也使能三缓冲和双
	  图层的话会超出8MB，所以用户根据自己的情况做显存和emWin动态内存的分配调整。
	    举一个例子，对于800*480分辨率的显示屏，使能32位色，三缓冲，那么最终一个图层需要的大小就是
      800 * 480 * 4 * 3  = 4.394MB的空间，如果是双图层，已经超出8MB的分配范围。

      (5)为了方便起见，图层2的宏定义LCD_LAYER1_FRAME_BUFFER中的参数4是按照32位色设置的，如果用户的图层1
         使用的是8位色，这里填数字1,如果是16位色，这里填2，如果是24位色，这里填3。
*/
#define LCD_LAYER0_FRAME_BUFFER  EXT_SDRAM_ADDR
#define LCD_LAYER1_FRAME_BUFFER  (LCD_LAYER0_FRAME_BUFFER + XSIZE_PHYS * YSIZE_PHYS * 4 * NUM_VSCREENS * NUM_BUFFERS)


/* 7. 配置图层1的颜色模式和分辨率大小 */
#define COLOR_MODE_0  CMS_RGB565
#define ORIENTATION_0 ROTATION_0
#define XSIZE_0       XSIZE_PHYS
#define YSIZE_0       YSIZE_PHYS

/* 8. 配置图层2的的颜色模式和分辨率大小 */
#define COLOR_MODE_1  CMS_RGB565
#define ORIENTATION_1 ROTATION_0
#define XSIZE_1       XSIZE_PHYS
#define YSIZE_1       YSIZE_PHYS

/* 9. 没有图层激活时，背景色设置, 暂时未用到 */
#define BK_COLOR      GUI_DARKBLUE


/* 10. 单图层情况下，根据用户选择的颜色模式可自动选择图层1的emWin的颜色模式 */
#if   (COLOR_MODE_0 == CMS_ARGB8888)
  #define COLOR_CONVERSION_0 GUICC_M8888I
#elif (COLOR_MODE_0 == CMS_RGB888)
  #define COLOR_CONVERSION_0 GUICC_M888
#elif (COLOR_MODE_0 == CMS_RGB565)
  #define COLOR_CONVERSION_0 GUICC_M565
#elif (COLOR_MODE_0 == CMS_ARGB1555)
  #define COLOR_CONVERSION_0 GUICC_M1555I
#elif (COLOR_MODE_0 == CMS_ARGB4444)
  #define COLOR_CONVERSION_0 GUICC_M4444I
#elif (COLOR_MODE_0 == CMS_L8)
  #define COLOR_CONVERSION_0 GUICC_8666
#elif (COLOR_MODE_0 == CMS_AL44)
  #define COLOR_CONVERSION_0 GUICC_1616I
#elif (COLOR_MODE_0 == CMS_AL88)
  #define COLOR_CONVERSION_0 GUICC_88666I
#else
  #error Illegal color mode 0!
#endif

/* 11. 单图层情况下，根据用户选择的颜色模式可自动选择图层1的emWin的驱动 */
#if   (COLOR_MODE_0 == CMS_ARGB8888)
  #if   (ORIENTATION_0 == ROTATION_0)
    #define DISPLAY_DRIVER_0   GUIDRV_LIN_32
  #elif (ORIENTATION_0 == ROTATION_CW)
    #define DISPLAY_DRIVER_0   GUIDRV_LIN_OSX_32
  #elif (ORIENTATION_0 == ROTATION_180)
    #define DISPLAY_DRIVER_0   GUIDRV_LIN_OXY_32
  #elif (ORIENTATION_0 == ROTATION_CCW)
    #define DISPLAY_DRIVER_0   GUIDRV_LIN_OSY_32
  #endif
#elif (COLOR_MODE_0 == CMS_RGB888)
  #if   (ORIENTATION_0 == ROTATION_0)
    #define DISPLAY_DRIVER_0   GUIDRV_LIN_24
  #elif (ORIENTATION_0 == ROTATION_CW)
    #define DISPLAY_DRIVER_0   GUIDRV_LIN_OSX_24
  #elif (ORIENTATION_0 == ROTATION_180)
    #define DISPLAY_DRIVER_0   GUIDRV_LIN_OXY_24
  #elif (ORIENTATION_0 == ROTATION_CCW)
    #define DISPLAY_DRIVER_0   GUIDRV_LIN_OSY_24
  #endif
#elif (COLOR_MODE_0 == CMS_RGB565)   \
   || (COLOR_MODE_0 == CMS_ARGB1555) \
   || (COLOR_MODE_0 == CMS_ARGB4444) \
   || (COLOR_MODE_0 == CMS_AL88)
  #if   (ORIENTATION_0 == ROTATION_0)
    #define DISPLAY_DRIVER_0   GUIDRV_LIN_16
  #elif (ORIENTATION_0 == ROTATION_CW)
    #define DISPLAY_DRIVER_0   GUIDRV_LIN_OSX_16
  #elif (ORIENTATION_0 == ROTATION_180)
    #define DISPLAY_DRIVER_0   GUIDRV_LIN_OXY_16
  #elif (ORIENTATION_0 == ROTATION_CCW)
    #define DISPLAY_DRIVER_0   GUIDRV_LIN_OSY_16
  #endif
#elif (COLOR_MODE_0 == CMS_L8)   \
   || (COLOR_MODE_0 == CMS_AL44)
  #if   (ORIENTATION_0 == ROTATION_0)
    #define DISPLAY_DRIVER_0   GUIDRV_LIN_8
  #elif (ORIENTATION_0 == ROTATION_CW)
    #define DISPLAY_DRIVER_0   GUIDRV_LIN_OSX_8
  #elif (ORIENTATION_0 == ROTATION_180)
    #define DISPLAY_DRIVER_0   GUIDRV_LIN_OXY_8
  #elif (ORIENTATION_0 == ROTATION_CCW)
    #define DISPLAY_DRIVER_0   GUIDRV_LIN_OSY_8
  #endif
#endif


/* 12. 双图层情况下，根据用户选择的颜色模式可自动选择图层2的emWin的颜色模式 */
#if (GUI_NUM_LAYERS > 1)
#if   (COLOR_MODE_1 == CMS_ARGB8888)
  #define COLOR_CONVERSION_1 GUICC_M8888I
#elif (COLOR_MODE_1 == CMS_RGB888)
  #define COLOR_CONVERSION_1 GUICC_M888
#elif (COLOR_MODE_1 == CMS_RGB565)
  #define COLOR_CONVERSION_1 GUICC_M565
#elif (COLOR_MODE_1 == CMS_ARGB1555)
  #define COLOR_CONVERSION_1 GUICC_M1555I
#elif (COLOR_MODE_1 == CMS_ARGB4444)
  #define COLOR_CONVERSION_1 GUICC_M4444I
#elif (COLOR_MODE_1 == CMS_L8)
  #define COLOR_CONVERSION_1 GUICC_8666
#elif (COLOR_MODE_1 == CMS_AL44)
  #define COLOR_CONVERSION_1 GUICC_1616I
#elif (COLOR_MODE_1 == CMS_AL88)
  #define COLOR_CONVERSION_1 GUICC_88666I
#else
  #error Illegal color mode 0!
#endif

/* 13. 双图层情况下，根据用户选择的颜色模式可自动选择图层2的emWin的驱动 */
#if   (COLOR_MODE_1 == CMS_ARGB8888)
  #if   (ORIENTATION_1 == ROTATION_0)
    #define DISPLAY_DRIVER_1   GUIDRV_LIN_32
  #elif (ORIENTATION_1 == ROTATION_CW)
    #define DISPLAY_DRIVER_1   GUIDRV_LIN_OSX_32
  #elif (ORIENTATION_1 == ROTATION_180)
    #define DISPLAY_DRIVER_1   GUIDRV_LIN_OXY_32
  #elif (ORIENTATION_1 == ROTATION_CCW)
    #define DISPLAY_DRIVER_1   GUIDRV_LIN_OSY_32
  #endif
#elif (COLOR_MODE_1 == CMS_RGB888)
  #if   (ORIENTATION_1 == ROTATION_0)
    #define DISPLAY_DRIVER_1   GUIDRV_LIN_24
  #elif (ORIENTATION_1 == ROTATION_CW)
    #define DISPLAY_DRIVER_1   GUIDRV_LIN_OSX_24
  #elif (ORIENTATION_1 == ROTATION_180)
    #define DISPLAY_DRIVER_1   GUIDRV_LIN_OXY_24
  #elif (ORIENTATION_1 == ROTATION_CCW)
    #define DISPLAY_DRIVER_1   GUIDRV_LIN_OSY_24
  #endif
#elif (COLOR_MODE_1 == CMS_RGB565)   \
   || (COLOR_MODE_1 == CMS_ARGB1555) \
   || (COLOR_MODE_1 == CMS_ARGB4444) \
   || (COLOR_MODE_1 == CMS_AL88)
  #if   (ORIENTATION_1 == ROTATION_0)
    #define DISPLAY_DRIVER_1   GUIDRV_LIN_16
  #elif (ORIENTATION_1 == ROTATION_CW)
    #define DISPLAY_DRIVER_1   GUIDRV_LIN_OSX_16
  #elif (ORIENTATION_1 == ROTATION_180)
    #define DISPLAY_DRIVER_1   GUIDRV_LIN_OXY_16
  #elif (ORIENTATION_1 == ROTATION_CCW)
    #define DISPLAY_DRIVER_1   GUIDRV_LIN_OSY_16
  #endif
#elif (COLOR_MODE_1 == CMS_L8)   \
   || (COLOR_MODE_1 == CMS_AL44)
  #if   (ORIENTATION_1 == ROTATION_0)
    #define DISPLAY_DRIVER_1   GUIDRV_LIN_8
  #elif (ORIENTATION_1 == ROTATION_CW)
    #define DISPLAY_DRIVER_1   GUIDRV_LIN_OSX_8
  #elif (ORIENTATION_1 == ROTATION_180)
    #define DISPLAY_DRIVER_1   GUIDRV_LIN_OXY_8
  #elif (ORIENTATION_1 == ROTATION_CCW)
    #define DISPLAY_DRIVER_1   GUIDRV_LIN_OSY_8
  #endif
#endif

#else

#undef XSIZE_0
#undef YSIZE_0
#define XSIZE_0 XSIZE_PHYS
#define YSIZE_0 YSIZE_PHYS
     
#endif

/*14. 配置选项检测，防止配置错误或者某些选项没有配置 */
#if NUM_BUFFERS > 3
  #error More than 3 buffers make no sense and are not supported in this configuration file!
#endif
#ifndef   XSIZE_PHYS
  #error Physical X size of display is not defined!
#endif
#ifndef   YSIZE_PHYS
  #error Physical Y size of display is not defined!
#endif
#ifndef   NUM_BUFFERS
  #define NUM_BUFFERS 1
#else
  #if (NUM_BUFFERS <= 0)
    #error At least one buffer needs to be defined!
  #endif
#endif
#ifndef   NUM_VSCREENS
  #define NUM_VSCREENS 1
#else
  #if (NUM_VSCREENS <= 0)
    #error At least one screeen needs to be defined!
  #endif
#endif
#if (NUM_VSCREENS > 1) && (NUM_BUFFERS > 1)
  #error Virtual screens together with multiple buffers are not allowed!
#endif
     
/*
**********************************************************************************************************
									使用DMA2D重定向颜色的批量转换
**********************************************************************************************************
*/
#define DEFINE_DMA2D_COLORCONVERSION(PFIX, PIXELFORMAT)                                                        \
static void _Color2IndexBulk_##PFIX##_DMA2D(LCD_COLOR * pColor, void * pIndex, U32 NumItems, U8 SizeOfIndex) { \
  _DMA_Color2IndexBulk(pColor, pIndex, NumItems, SizeOfIndex, PIXELFORMAT);                                    \
}                                                                                                              \
static void _Index2ColorBulk_##PFIX##_DMA2D(void * pIndex, LCD_COLOR * pColor, U32 NumItems, U8 SizeOfIndex) { \
  _DMA_Index2ColorBulk(pIndex, pColor, NumItems, SizeOfIndex, PIXELFORMAT);                                    \
}
 
/* 函数声明 */
static void _DMA_Index2ColorBulk(void * pIndex, LCD_COLOR * pColor, U32 NumItems, U8 SizeOfIndex, U32 PixelFormat);
static void _DMA_Color2IndexBulk(LCD_COLOR * pColor, void * pIndex, U32 NumItems, U8 SizeOfIndex, U32 PixelFormat);

/* 颜色转换 */
DEFINE_DMA2D_COLORCONVERSION(M8888I, LTDC_PIXEL_FORMAT_ARGB8888)
DEFINE_DMA2D_COLORCONVERSION(M888,   LTDC_PIXEL_FORMAT_ARGB8888) 
DEFINE_DMA2D_COLORCONVERSION(M565,   LTDC_PIXEL_FORMAT_RGB565)
DEFINE_DMA2D_COLORCONVERSION(M1555I, LTDC_PIXEL_FORMAT_ARGB1555)
DEFINE_DMA2D_COLORCONVERSION(M4444I, LTDC_PIXEL_FORMAT_ARGB4444)


static LTDC_HandleTypeDef     hltdc;  
static LCD_LayerPropTypedef   layer_prop[GUI_NUM_LAYERS];
static const U32   _aAddr[]   = {LCD_LAYER0_FRAME_BUFFER, LCD_LAYER1_FRAME_BUFFER};
static int _aPendingBuffer[2] = { -1, -1 };
static int _aBufferIndex[GUI_NUM_LAYERS];
static int _axSize[GUI_NUM_LAYERS];
static int _aySize[GUI_NUM_LAYERS];
static int _aBytesPerPixels[GUI_NUM_LAYERS];

/* 用不上，留作备用 */
#if DrawBitmapA4Enalbe == 1
static U32 _aBuffer[XSIZE_PHYS * sizeof(U32) * 3]__attribute__((at(0x24000000)));
static U32 * _pBuffer_DMA2D = &_aBuffer[XSIZE_PHYS * sizeof(U32) * 0];

/* 加速 A4 bitmaps 显示 */
static const U8 _aMirror[] = {
  0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80, 0x90, 0xA0, 0xB0, 0xC0, 0xD0, 0xE0, 0xF0,
  0x01, 0x11, 0x21, 0x31, 0x41, 0x51, 0x61, 0x71, 0x81, 0x91, 0xA1, 0xB1, 0xC1, 0xD1, 0xE1, 0xF1,
  0x02, 0x12, 0x22, 0x32, 0x42, 0x52, 0x62, 0x72, 0x82, 0x92, 0xA2, 0xB2, 0xC2, 0xD2, 0xE2, 0xF2,
  0x03, 0x13, 0x23, 0x33, 0x43, 0x53, 0x63, 0x73, 0x83, 0x93, 0xA3, 0xB3, 0xC3, 0xD3, 0xE3, 0xF3,
  0x04, 0x14, 0x24, 0x34, 0x44, 0x54, 0x64, 0x74, 0x84, 0x94, 0xA4, 0xB4, 0xC4, 0xD4, 0xE4, 0xF4,
  0x05, 0x15, 0x25, 0x35, 0x45, 0x55, 0x65, 0x75, 0x85, 0x95, 0xA5, 0xB5, 0xC5, 0xD5, 0xE5, 0xF5,
  0x06, 0x16, 0x26, 0x36, 0x46, 0x56, 0x66, 0x76, 0x86, 0x96, 0xA6, 0xB6, 0xC6, 0xD6, 0xE6, 0xF6,
  0x07, 0x17, 0x27, 0x37, 0x47, 0x57, 0x67, 0x77, 0x87, 0x97, 0xA7, 0xB7, 0xC7, 0xD7, 0xE7, 0xF7,
  0x08, 0x18, 0x28, 0x38, 0x48, 0x58, 0x68, 0x78, 0x88, 0x98, 0xA8, 0xB8, 0xC8, 0xD8, 0xE8, 0xF8,
  0x09, 0x19, 0x29, 0x39, 0x49, 0x59, 0x69, 0x79, 0x89, 0x99, 0xA9, 0xB9, 0xC9, 0xD9, 0xE9, 0xF9,
  0x0A, 0x1A, 0x2A, 0x3A, 0x4A, 0x5A, 0x6A, 0x7A, 0x8A, 0x9A, 0xAA, 0xBA, 0xCA, 0xDA, 0xEA, 0xFA,
  0x0B, 0x1B, 0x2B, 0x3B, 0x4B, 0x5B, 0x6B, 0x7B, 0x8B, 0x9B, 0xAB, 0xBB, 0xCB, 0xDB, 0xEB, 0xFB,
  0x0C, 0x1C, 0x2C, 0x3C, 0x4C, 0x5C, 0x6C, 0x7C, 0x8C, 0x9C, 0xAC, 0xBC, 0xCC, 0xDC, 0xEC, 0xFC,
  0x0D, 0x1D, 0x2D, 0x3D, 0x4D, 0x5D, 0x6D, 0x7D, 0x8D, 0x9D, 0xAD, 0xBD, 0xCD, 0xDD, 0xED, 0xFD,
  0x0E, 0x1E, 0x2E, 0x3E, 0x4E, 0x5E, 0x6E, 0x7E, 0x8E, 0x9E, 0xAE, 0xBE, 0xCE, 0xDE, 0xEE, 0xFE,
  0x0F, 0x1F, 0x2F, 0x3F, 0x4F, 0x5F, 0x6F, 0x7F, 0x8F, 0x9F, 0xAF, 0xBF, 0xCF, 0xDF, 0xEF, 0xFF,
};
#else
static U32 _aBuffer[1];
static U32 * _pBuffer_DMA2D = &_aBuffer[0];
#endif

/* 颜色转换模式 */
static const LCD_API_COLOR_CONV *_apColorConvAPI[] = {
  COLOR_CONVERSION_0,
#if GUI_NUM_LAYERS > 1
  COLOR_CONVERSION_1,
#endif
};

/* 显示方向 */
static const int _aOrientation[] = 
{
  ORIENTATION_0,
#if GUI_NUM_LAYERS > 1
  ORIENTATION_1,
#endif
};

/*
*********************************************************************************************************
*	函 数 名: _ClearCacheHook
*	功能说明: 清Cache
*	形    参: LayerIndex  图层
*	返 回 值: 无
*********************************************************************************************************
*/
#if ClearCacheHookEnalbe == 1
static void _ClearCacheHook(U32 LayerMask) 
{
	int i;
	for (i = 0; i < GUI_NUM_LAYERS; i++) 
	{
		if (LayerMask & (1 << i)) 
		{
			SCB_CleanDCache_by_Addr ((uint32_t *)_aAddr[i], XSIZE_PHYS * YSIZE_PHYS * sizeof(U32));
		}
	}
}
#endif

/*
*********************************************************************************************************
*	函 数 名: _GetPixelformat
*	功能说明: 获取图层1或者图层2使用的颜色格式
*	形    参: LayerIndex  图层
*	返 回 值: 颜色格式
*********************************************************************************************************
*/
static U32 _GetPixelformat(int LayerIndex) 
{
	const LCD_API_COLOR_CONV * pColorConvAPI;

	if (LayerIndex >= GUI_COUNTOF(_apColorConvAPI)) 
	{
		return 0;
	}
	
	pColorConvAPI = _apColorConvAPI[LayerIndex];
	
	if (pColorConvAPI == GUICC_M8888I) 
	{
		return LTDC_PIXEL_FORMAT_ARGB8888;
	}
	else if (pColorConvAPI == GUICC_M888) 
	{
		return LTDC_PIXEL_FORMAT_RGB888;
	}
	else if (pColorConvAPI == GUICC_M565) 
	{
		return LTDC_PIXEL_FORMAT_RGB565;
	}
	else if (pColorConvAPI == GUICC_M1555I)
	{
		return LTDC_PIXEL_FORMAT_ARGB1555;
	}
	else if (pColorConvAPI == GUICC_M4444I) 
	{
	return LTDC_PIXEL_FORMAT_ARGB4444;
	}
	else if (pColorConvAPI == GUICC_8666  ) 
	{
		return LTDC_PIXEL_FORMAT_L8;
	}
	else if (pColorConvAPI == GUICC_1616I ) 
	{
		return LTDC_PIXEL_FORMAT_AL44;
	}
	else if (pColorConvAPI == GUICC_88666I) 
	{
		return LTDC_PIXEL_FORMAT_AL88;
	}
	
	/* 配置错误会进入这里 */
	while (1);
}

/*
*********************************************************************************************************
*	函 数 名: _GetPixelformat
*	功能说明: 获取图层1或者图层2使用的颜色格式
*	形    参: LayerIndex  图层
*	返 回 值: 颜色格式
*********************************************************************************************************
*/
static void LCD_LL_LayerInit(U32 LayerIndex) 
{  
	LTDC_LayerCfgTypeDef  layer_cfg;  
	static uint32_t       LUT[256];
	uint32_t              i;

	if (LayerIndex < GUI_NUM_LAYERS)
	{
		/* 窗口显示区设置 */ 
		layer_cfg.WindowX0 = 0;
		layer_cfg.WindowX1 = g_LcdWidth;
		layer_cfg.WindowY0 = 0;
		layer_cfg.WindowY1 = g_LcdHeight;
		
		/* 配置颜色格式 */ 
		layer_cfg.PixelFormat = _GetPixelformat(LayerIndex);
		
		/* 显存地址 */
		layer_cfg.FBStartAdress = layer_prop[LayerIndex].address;
		
		/* Alpha常数 (255 表示完全不透明) */
		layer_cfg.Alpha = 255;
		
		/* 无背景色 */
		layer_cfg.Alpha0 = 0;   /* 完全透明 */
		layer_cfg.Backcolor.Blue = 0;
		layer_cfg.Backcolor.Green = 0;
		layer_cfg.Backcolor.Red = 0;
		
		/* 配置图层混合因数 */
		layer_cfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_PAxCA;
		layer_cfg.BlendingFactor2 = LTDC_BLENDING_FACTOR2_PAxCA;

		/* 配置行列大小 */
		layer_cfg.ImageWidth = g_LcdWidth;
		layer_cfg.ImageHeight = g_LcdHeight;

		/* 配置图层1 */
		HAL_LTDC_ConfigLayer(&hltdc, &layer_cfg, LayerIndex);

		/* 使能LUT */
		if (LCD_GetBitsPerPixelEx(LayerIndex) <= 8)
		{
			HAL_LTDC_EnableCLUT(&hltdc, LayerIndex);
		}
		else
		{
			/*  AL88模式(16bpp) */
			if (layer_prop[LayerIndex].pColorConvAPI == GUICC_88666I)
			{
				HAL_LTDC_EnableCLUT(&hltdc, LayerIndex);

				for (i = 0; i < 256; i++)
				{
					LUT[i] = LCD_API_ColorConv_8666.pfIndex2Color(i);
				}
				
				HAL_LTDC_ConfigCLUT(&hltdc, LUT, 256, LayerIndex);
			}
		}
	}  
}

/*
*********************************************************************************************************
*	函 数 名: LCD_LL_Init
*	功能说明: 配置LTDC
*	形    参: 无
*	返 回 值: 无
*   笔    记:
*       LCD_TFT 同步时序配置（整理自官方做的一个截图，言简意赅）：
*       ----------------------------------------------------------------------------
*    
*                                                 Total Width
*                             <--------------------------------------------------->
*                       Hsync width HBP             Active Width                HFP
*                             <---><--><--------------------------------------><-->
*                         ____    ____|_______________________________________|____ 
*                             |___|   |                                       |    |
*                                     |                                       |    |
*                         __|         |                                       |    |
*            /|\    /|\  |            |                                       |    |
*             | VSYNC|   |            |                                       |    |
*             |Width\|/  |__          |                                       |    |
*             |     /|\     |         |                                       |    |
*             |  VBP |      |         |                                       |    |
*             |     \|/_____|_________|_______________________________________|    |
*             |     /|\     |         | / / / / / / / / / / / / / / / / / / / |    |
*             |      |      |         |/ / / / / / / / / / / / / / / / / / / /|    |
*    Total    |      |      |         |/ / / / / / / / / / / / / / / / / / / /|    |
*    Heigh    |      |      |         |/ / / / / / / / / / / / / / / / / / / /|    |
*             |Active|      |         |/ / / / / / / / / / / / / / / / / / / /|    |
*             |Heigh |      |         |/ / / / / / Active Display Area / / / /|    |
*             |      |      |         |/ / / / / / / / / / / / / / / / / / / /|    |
*             |      |      |         |/ / / / / / / / / / / / / / / / / / / /|    |
*             |      |      |         |/ / / / / / / / / / / / / / / / / / / /|    |
*             |      |      |         |/ / / / / / / / / / / / / / / / / / / /|    |
*             |      |      |         |/ / / / / / / / / / / / / / / / / / / /|    |
*             |     \|/_____|_________|_______________________________________|    |
*             |     /|\     |                                                      |
*             |  VFP |      |                                                      |
*            \|/    \|/_____|______________________________________________________|
*            
*     
*     每个LCD设备都有自己的同步时序值：
*     Horizontal Synchronization (Hsync) 
*     Horizontal Back Porch (HBP)       
*     Active Width                      
*     Horizontal Front Porch (HFP)     
*   
*     Vertical Synchronization (Vsync)  
*     Vertical Back Porch (VBP)         
*     Active Heigh                       
*     Vertical Front Porch (VFP)         
*     
*     LCD_TFT 窗口水平和垂直的起始以及结束位置 :
*     ----------------------------------------------------------------
*   
*     HorizontalStart = (Offset_X + Hsync + HBP);
*     HorizontalStop  = (Offset_X + Hsync + HBP + Window_Width - 1); 
*     VarticalStart   = (Offset_Y + Vsync + VBP);
*     VerticalStop    = (Offset_Y + Vsync + VBP + Window_Heigh - 1);
*
*********************************************************************************************************
*/
static void LCD_LL_Init(void) 
{
	/* 配置LCD相关的GPIO */
	{
		/* GPIOs Configuration */
		/*
		+------------------------+-----------------------+----------------------------+
		+                       LCD pins assignment                                   +
		+------------------------+-----------------------+----------------------------+
		|  LCDH7_TFT R0 <-> PI.15  |  LCDH7_TFT G0 <-> PJ.07 |  LCDH7_TFT B0 <-> PJ.12      |
		|  LCDH7_TFT R1 <-> PJ.00  |  LCDH7_TFT G1 <-> PJ.08 |  LCDH7_TFT B1 <-> PJ.13      |
		|  LCDH7_TFT R2 <-> PJ.01  |  LCDH7_TFT G2 <-> PJ.09 |  LCDH7_TFT B2 <-> PJ.14      |
		|  LCDH7_TFT R3 <-> PJ.02  |  LCDH7_TFT G3 <-> PJ.10 |  LCDH7_TFT B3 <-> PJ.15      |
		|  LCDH7_TFT R4 <-> PJ.03  |  LCDH7_TFT G4 <-> PJ.11 |  LCDH7_TFT B4 <-> PK.03      |
		|  LCDH7_TFT R5 <-> PJ.04  |  LCDH7_TFT G5 <-> PK.00 |  LCDH7_TFT B5 <-> PK.04      |
		|  LCDH7_TFT R6 <-> PJ.05  |  LCDH7_TFT G6 <-> PK.01 |  LCDH7_TFT B6 <-> PK.05      |
		|  LCDH7_TFT R7 <-> PJ.06  |  LCDH7_TFT G7 <-> PK.02 |  LCDH7_TFT B7 <-> PK.06      |
		-------------------------------------------------------------------------------
		|  LCDH7_TFT HSYNC <-> PI.12  | LCDTFT VSYNC <->  PI.13 |
		|  LCDH7_TFT CLK   <-> PI.14  | LCDH7_TFT DE   <->  PK.07 |
		-----------------------------------------------------
		*/		
		GPIO_InitTypeDef GPIO_Init_Structure;

		/*##-1- Enable peripherals and GPIO Clocks #################################*/  
		/* 使能LTDC和DMA2D时钟 */
		__HAL_RCC_LTDC_CLK_ENABLE();
		__HAL_RCC_DMA2D_CLK_ENABLE();  
		
		/* 使能GPIO时钟 */
		__HAL_RCC_GPIOI_CLK_ENABLE();
		__HAL_RCC_GPIOJ_CLK_ENABLE();
		__HAL_RCC_GPIOK_CLK_ENABLE();

		/* GPIOI 配置 */
		GPIO_Init_Structure.Pin       = GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15; 
		GPIO_Init_Structure.Mode      = GPIO_MODE_AF_PP;
		GPIO_Init_Structure.Pull      = GPIO_NOPULL;
		GPIO_Init_Structure.Speed     = GPIO_SPEED_FREQ_HIGH;
		GPIO_Init_Structure.Alternate = GPIO_AF14_LTDC;  
		HAL_GPIO_Init(GPIOI, &GPIO_Init_Structure);

		/* GPIOJ 配置 */  
		GPIO_Init_Structure.Pin       = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | \
									  GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | \
									  GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | \
									  GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15; 
		GPIO_Init_Structure.Mode      = GPIO_MODE_AF_PP;
		GPIO_Init_Structure.Pull      = GPIO_NOPULL;
		GPIO_Init_Structure.Speed     = GPIO_SPEED_FREQ_HIGH;
		GPIO_Init_Structure.Alternate = GPIO_AF14_LTDC;  
		HAL_GPIO_Init(GPIOJ, &GPIO_Init_Structure);  

		/* GPIOK 配置 */  
		GPIO_Init_Structure.Pin       = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | \
									  GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7; 
		GPIO_Init_Structure.Mode      = GPIO_MODE_AF_PP;
		GPIO_Init_Structure.Pull      = GPIO_NOPULL;
		GPIO_Init_Structure.Speed     = GPIO_SPEED_FREQ_HIGH;
		GPIO_Init_Structure.Alternate = GPIO_AF14_LTDC;  
		HAL_GPIO_Init(GPIOK, &GPIO_Init_Structure);  	
	}
	
	/*##-2- LTDC初始化 #############################################################*/  
	{	
		uint16_t Width, Height, HSYNC_W, HBP, HFP, VSYNC_W, VBP, VFP;
		RCC_PeriphCLKInitTypeDef  PeriphClkInitStruct;

		/* 支持6种面板 */
		switch (g_LcdType)
		{
			case LCD_35_480X320:	/* 3.5寸 480 * 320 */	
				Width = 480;
				Height = 272;
				HSYNC_W = 10;
				HBP = 20;
				HFP = 20;
				VSYNC_W = 20;
				VBP = 20;
				VFP = 20;
				break;
			
			case LCD_43_480X272:		/* 4.3寸 480 * 272 */			
				Width = 480;
				Height = 272;

				HSYNC_W = 40;
				HBP = 2;
				HFP = 2;
				VSYNC_W = 9;
				VBP = 2;
				VFP = 2;
		
				/* LCD 时钟配置 */
				/* PLL3_VCO Input = HSE_VALUE/PLL3M = 25MHz/5 = 5MHz */
				/* PLL3_VCO Output = PLL3_VCO Input * PLL3N = 5MHz * 48 = 240MHz */
				/* PLLLCDCLK = PLL3_VCO Output/PLL3R = 240 / 10 = 24MHz */
				/* LTDC clock frequency = PLLLCDCLK = 24MHz */
				/*
					刷新率 = 24MHz /((Width + HSYNC_W  + HBP  + HFP)*(Height + VSYNC_W +  VBP  + VFP))
                   		   = 24000000/((480 + 40  + 2  + 2)*(272 + 9 +  2  + 2)) 
			               = 24000000/(524*285)
                           = 160Hz	

					当前这个配置方便用户使用PLL3Q输出的48MHz时钟供USB使用。
			    */
				PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LTDC;
				PeriphClkInitStruct.PLL3.PLL3M = 5;
				PeriphClkInitStruct.PLL3.PLL3N = 48;
				PeriphClkInitStruct.PLL3.PLL3P = 2;
				PeriphClkInitStruct.PLL3.PLL3Q = 5;
				PeriphClkInitStruct.PLL3.PLL3R = 10;				
				HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);     			
				break;
			
			case LCD_50_480X272:		/* 5.0寸 480 * 272 */
				Width = 480;
				Height = 272;
			
				HSYNC_W = 40;
				HBP = 2;
				HFP = 2;
				VSYNC_W = 9;
				VBP = 2;
				VFP = 2;			
				break;
			
			case LCD_50_800X480:		/* 5.0寸 800 * 480 */
			case LCD_70_800X480:		/* 7.0寸 800 * 480 */					
				Width = 800;
				Height = 480;

				HSYNC_W = 96;	/* =10时，显示错位，20时部分屏可以的,80时全部OK */
				HBP = 10;
				HFP = 10;
				VSYNC_W = 2;
				VBP = 10;
				VFP = 10;			

				/* LCD 时钟配置 */
				/* PLL3_VCO Input = HSE_VALUE/PLL3M = 25MHz/5 = 5MHz */
				/* PLL3_VCO Output = PLL3_VCO Input * PLL3N = 5MHz * 48 = 240MHz */
				/* PLLLCDCLK = PLL3_VCO Output/PLL3R = 240 / 10 = 24MHz */
				/* LTDC clock frequency = PLLLCDCLK = 24MHz */
				/*
					刷新率 = 24MHz /((Width + HSYNC_W  + HBP  + HFP)*(Height + VSYNC_W +  VBP  + VFP))
                   		   = 24000000/((800 + 96  + 10  + 10)*(480 + 2 +  10  + 10)) 
			               = 24000000/(916*502)
                           = 52Hz	
			
					根据需要可以加大，100Hz刷新率完全没问题，设置PeriphClkInitStruct.PLL3.PLL3N = 100即可
					此时的LTDC时钟是50MHz
					刷新率 = 50MHz /(（Width + HSYNC_W  + HBP  + HFP ）*(Height + VSYNC_W +  VBP  +VFP  )) 
					       = 5000000/(916*502) 
					       = 108.7Hz

					当前这个配置方便用户使用PLL3Q输出的48MHz时钟供USB使用。
			    */ 
				PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LTDC;
				PeriphClkInitStruct.PLL3.PLL3M = 5;
				PeriphClkInitStruct.PLL3.PLL3N = 48;
				PeriphClkInitStruct.PLL3.PLL3P = 2;
				PeriphClkInitStruct.PLL3.PLL3Q = 5;
				PeriphClkInitStruct.PLL3.PLL3R = 10; 
				HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);     			
				break;
			
			case LCD_70_1024X600:		/* 7.0寸 1024 * 600 */
				/* 实测像素时钟 = 53.7M */
				Width = 1024;
				Height = 600;

				HSYNC_W = 2;	/* =10时，显示错位，20时部分屏可以的,80时全部OK */
				HBP = 157;
				HFP = 160;
				VSYNC_W = 2;
				VBP = 20;
				VFP = 12;		
			
				PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LTDC;
				PeriphClkInitStruct.PLL3.PLL3M = 5;
				PeriphClkInitStruct.PLL3.PLL3N = 48;
				PeriphClkInitStruct.PLL3.PLL3P = 2;
				PeriphClkInitStruct.PLL3.PLL3Q = 5;
				PeriphClkInitStruct.PLL3.PLL3R = 10;
				HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct); 			
				break;
			
			default:
				Width = 800;
				Height = 480;

				HSYNC_W = 80;	/* =10时，显示错位，20时部分屏可以的,80时全部OK */
				HBP = 10;
				HFP = 10;
				VSYNC_W = 10;
				VBP = 10;
				VFP = 10;		
			
				/* LCD 时钟配置 */
				/* PLL3_VCO Input = HSE_VALUE/PLL3M = 25MHz/5 = 5MHz */
				/* PLL3_VCO Output = PLL3_VCO Input * PLL3N = 5MHz * 48 = 240MHz */
				/* PLLLCDCLK = PLL3_VCO Output/PLL3R = 240 / 10 = 24MHz */
				/* LTDC clock frequency = PLLLCDCLK = 24MHz */
				/*
					刷新率 = 24MHz /((Width + HSYNC_W  + HBP  + HFP)*(Height + VSYNC_W +  VBP  + VFP))
                   		   = 24000000/((800 + 96  + 10  + 10)*(480 + 2 +  10  + 10)) 
			               = 24000000/(916*502)
                           = 52Hz

					根据需要可以加大，100Hz刷新率完全没问题，设置PeriphClkInitStruct.PLL3.PLL3N = 100即可
					此时的LTDC时钟是50MHz
					刷新率 = 50MHz /(（Width + HSYNC_W  + HBP  + HFP ）*(Height + VSYNC_W +  VBP  +VFP  )) 
					       = 5000000/(916*502) 
					       = 108.7Hz

					当前这个配置方便用户使用PLL3Q输出的48MHz时钟供USB使用。
			    */ 
				PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LTDC;
				PeriphClkInitStruct.PLL3.PLL3M = 5;
				PeriphClkInitStruct.PLL3.PLL3N = 48;
				PeriphClkInitStruct.PLL3.PLL3P = 2;
				PeriphClkInitStruct.PLL3.PLL3Q = 5;
				PeriphClkInitStruct.PLL3.PLL3R = 10;  
				HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct); 			
				break;
		}		

		g_LcdHeight = Height;
		g_LcdWidth = Width;
		
		/* 配置信号极性 */	
		hltdc.Init.HSPolarity = LTDC_HSPOLARITY_AL;	/* HSYNC 低电平有效 */
		hltdc.Init.VSPolarity = LTDC_VSPOLARITY_AL; 	/* VSYNC 低电平有效 */
		hltdc.Init.DEPolarity = LTDC_DEPOLARITY_AL; 	/* DE 低电平有效 */
		hltdc.Init.PCPolarity = LTDC_PCPOLARITY_IPC;

		/* 时序配置 */    
		hltdc.Init.HorizontalSync = (HSYNC_W - 1);
		hltdc.Init.VerticalSync = (VSYNC_W - 1);
		hltdc.Init.AccumulatedHBP = (HSYNC_W + HBP - 1);
		hltdc.Init.AccumulatedVBP = (VSYNC_W + VBP - 1);  
		hltdc.Init.AccumulatedActiveH = (Height + VSYNC_W + VBP - 1);
		hltdc.Init.AccumulatedActiveW = (Width + HSYNC_W + HBP - 1);
		hltdc.Init.TotalHeigh = (Height + VSYNC_W + VBP + VFP - 1);
		hltdc.Init.TotalWidth = (Width + HSYNC_W + HBP + HFP - 1); 

		/* 配置背景层颜色 */
		hltdc.Init.Backcolor.Blue = 0;
		hltdc.Init.Backcolor.Green = 0;
		hltdc.Init.Backcolor.Red = 0;

		hltdc.Instance = LTDC;

		/* 配置LTDC  */  
		if (HAL_LTDC_Init(&hltdc) != HAL_OK)
		{
			/* 初始化错误 */
			Error_Handler(__FILE__, __LINE__);
		}
	}  

	/* 使能行中断 */
	HAL_LTDC_ProgramLineEvent(&hltdc, 0);
  
    /* 使能Dither */
    HAL_LTDC_EnableDither(&hltdc);

	/* 使能LTDC中断，并配置其优先级 */
	HAL_NVIC_SetPriority(LTDC_IRQn, 0x2, 0);
	HAL_NVIC_EnableIRQ(LTDC_IRQn);
}

/*
*********************************************************************************************************
*	函 数 名: _DMA_Copy
*	功能说明: 通过DMA2D从前景层复制指定区域的颜色数据到目标区域
*	形    参: LayerIndex    图层
*             pSrc          颜色数据源地址
*             pDst          颜色数据目的地址
*             xSize         要复制区域的X轴大小，即每行像素数
*             ySize         要复制区域的Y轴大小，即行数
*             OffLineSrc    前景层图像的行偏移
*             OffLineDst    输出的行偏移
*	返 回 值: 无
*********************************************************************************************************
*/
static void _DMA_Copy(int LayerIndex, void * pSrc, void * pDst, int xSize, int ySize, int OffLineSrc, int OffLineDst) 
{
	U32 PixelFormat;

	/* 获取图层使用的颜色格式 */
	PixelFormat    = _GetPixelformat(LayerIndex);

	/* DMA2D配置 */  
	DMA2D->CR      = 0x00000000UL | (1 << 9);
	DMA2D->FGMAR   = (U32)pSrc;
	DMA2D->OMAR    = (U32)pDst;
	DMA2D->FGOR    = OffLineSrc;
	DMA2D->OOR     = OffLineDst;
	DMA2D->FGPFCCR = PixelFormat;
	DMA2D->NLR     = (U32)(xSize << 16) | (U16)ySize;

	/* 启动传输 */
	DMA2D->CR   |= DMA2D_CR_START;   

	/* 等待DMA2D传输完成 */
	while (DMA2D->CR & DMA2D_CR_START) {} 
}

/*
*********************************************************************************************************
*	函 数 名: _DMA_CopyRGB565
*	功能说明: 通过DMA2D从前景层复制指定区域的颜色数据到目标区域
*	形    参: pSrc          颜色数据源地址
*             pDst          颜色数据目的地址
*             xSize         要复制区域的X轴大小，即每行像素数
*             ySize         要复制区域的Y轴大小，即行数
*             OffLineSrc    前景层图像的行偏移
*             OffLineDst    输出的行偏移
*	返 回 值: 无
*********************************************************************************************************
*/
static void _DMA_CopyRGB565(const void * pSrc, void * pDst, int xSize, int ySize, int OffLineSrc, int OffLineDst)
{
	/* DMA2D配置 */  
	DMA2D->CR      = 0x00000000UL | (1 << 9);
	DMA2D->FGMAR   = (U32)pSrc;
	DMA2D->OMAR    = (U32)pDst;
	DMA2D->FGOR    = OffLineSrc;
	DMA2D->OOR     = OffLineDst;
	DMA2D->FGPFCCR = LTDC_PIXEL_FORMAT_RGB565;
	DMA2D->NLR     = (U32)(xSize << 16) | (U16)ySize;

	/* 启动传输 */
	DMA2D->CR   |= DMA2D_CR_START;   

	/* 等待DMA2D传输完成 */
	while (DMA2D->CR & DMA2D_CR_START) {} 
}

/*
*********************************************************************************************************
*	函 数 名: _DMA_Fill
*	功能说明: 通过DMA2D对于指定区域进行颜色填充
*	形    参: LayerIndex    图层
*             pDst          颜色数据目的地址
*             xSize         要复制区域的X轴大小，即每行像素数
*             ySize         要复制区域的Y轴大小，即行数
*             OffLine       前景层图像的行偏移
*             ColorIndex    要填充的颜色值
*	返 回 值: 无
*********************************************************************************************************
*/
static void _DMA_Fill(int LayerIndex, void * pDst, int xSize, int ySize, int OffLine, U32 ColorIndex) 
{
	U32 PixelFormat;

	/* 获取图层使用的颜色格式 */
	PixelFormat = _GetPixelformat(LayerIndex);

	/* DMA2D配置 */  
	DMA2D->CR      = 0x00030000UL | (1 << 9);
	DMA2D->OCOLR   = ColorIndex;
	DMA2D->OMAR    = (U32)pDst;
	DMA2D->OOR     = OffLine;
	DMA2D->OPFCCR  = PixelFormat;
	DMA2D->NLR     = (U32)(xSize << 16) | (U16)ySize;

	/* 启动传输 */
	DMA2D->CR   |= DMA2D_CR_START;   

	/* 等待DMA2D传输完成 */
	while (DMA2D->CR & DMA2D_CR_START) {} 
}

/*
*********************************************************************************************************
*	函 数 名: _DMA_AlphaBlendingBulk
*	功能说明: 前景层和背景层混合输出
*	形    参: pColorFG    前景层颜色数据地址
*             pColorBG    背景层颜色数据地址
*             pColorDst   输出目标区地址
*             NumItems    转换次数
*	返 回 值: 无
*********************************************************************************************************
*/
static void _DMA_AlphaBlendingBulk(LCD_COLOR * pColorFG, LCD_COLOR * pColorBG, LCD_COLOR * pColorDst, U32 NumItems) 
{  
	/* DMA2D配置 */   
	DMA2D->CR      = 0x00020000UL | (1 << 9);
	DMA2D->FGMAR   = (U32)pColorFG;
	DMA2D->BGMAR   = (U32)pColorBG;
	DMA2D->OMAR    = (U32)pColorDst;
	DMA2D->FGOR    = 0;
	DMA2D->BGOR    = 0;
	DMA2D->OOR     = 0;
	DMA2D->FGPFCCR = LTDC_PIXEL_FORMAT_ARGB8888;
	DMA2D->BGPFCCR = LTDC_PIXEL_FORMAT_ARGB8888;
	DMA2D->OPFCCR  = LTDC_PIXEL_FORMAT_ARGB8888;
	DMA2D->NLR     = (U32)(NumItems << 16) | 1;

	/* 启动传输 */
	DMA2D->CR   |= DMA2D_CR_START;   

	/* 等待DMA2D传输完成 */
	while (DMA2D->CR & DMA2D_CR_START) {} 
}

/*
*********************************************************************************************************
*	函 数 名: _DMA_MixColorsBulk
*	功能说明: 前景层和背景层混合输出，带Alpha通道设置
*	形    参: pColorFG    前景层颜色数据地址
*             pColorBG    背景层颜色数据地址
*             pColorDst   输出目标区地址
*             Intens      Alpha通道设置
*             NumItems    转换次数
*	返 回 值: 无
*********************************************************************************************************
*/
static void _DMA_MixColorsBulk(LCD_COLOR * pColorFG, LCD_COLOR * pColorBG, LCD_COLOR * pColorDst, U8 Intens, U32 NumItems) 
{
	/* 配置DMA2D */
	DMA2D->CR      = 0x00020000UL | (1 << 9);
	DMA2D->FGMAR   = (U32)pColorFG;
	DMA2D->BGMAR   = (U32)pColorBG;
	DMA2D->OMAR    = (U32)pColorDst;
	DMA2D->FGPFCCR = LTDC_PIXEL_FORMAT_ARGB8888
				 | (1UL << 16)
				 | ((U32)Intens << 24);
	DMA2D->BGPFCCR = LTDC_PIXEL_FORMAT_ARGB8888
				 | (0UL << 16)
				 | ((U32)(255 - Intens) << 24);
	DMA2D->OPFCCR  = LTDC_PIXEL_FORMAT_ARGB8888;
	DMA2D->NLR     = (U32)(NumItems << 16) | 1;

	/* 启动传输 */
	DMA2D->CR   |= DMA2D_CR_START;   

	/* 等待DMA2D传输完成 */
	while (DMA2D->CR & DMA2D_CR_START) {} 
}

/*
*********************************************************************************************************
*	函 数 名: _DMA_ConvertColor
*	功能说明: 颜色格式转换
*	形    参: pSrc             源数据地址
*             pDst             目的数据地址
*             PixelFormatSrc   源数据颜色格式
*             PixelFormatDst   转换的颜色格式
*             NumItems         转换次数
*	返 回 值: 无
*********************************************************************************************************
*/
static void _DMA_ConvertColor(void * pSrc, void * pDst,  U32 PixelFormatSrc, U32 PixelFormatDst, U32 NumItems) 
{
	/* 配置DMA2D */
	DMA2D->CR      = 0x00010000UL | (1 << 9);
	DMA2D->FGMAR   = (U32)pSrc;
	DMA2D->OMAR    = (U32)pDst;
	DMA2D->FGOR    = 0;
	DMA2D->OOR     = 0;
	DMA2D->FGPFCCR = PixelFormatSrc;
	DMA2D->OPFCCR  = PixelFormatDst;
	DMA2D->NLR     = (U32)(NumItems << 16) | 1;

	/* 启动传输 */
	DMA2D->CR   |= DMA2D_CR_START;   

	/* 等待DMA2D传输完成 */
	while (DMA2D->CR & DMA2D_CR_START) {} 
}

/*
*********************************************************************************************************
*	函 数 名: _DMA_DrawBitmapL8
*	功能说明: 绘制L8格式位图
*	形    参: pSrc             源数据地址
*             pDst             目的数据地址
*             OffSrc           源数据行偏移
*             OffDst           目的数据行偏移
*             PixelFormatDst   转换的颜色格式
*             xSize            位图长
*             ySize            位图高
*	返 回 值: 无
*********************************************************************************************************
*/
static void _DMA_DrawBitmapL8(void * pSrc, void * pDst,  U32 OffSrc, U32 OffDst, U32 PixelFormatDst, U32 xSize, U32 ySize) 
{
	/* 配置DMA2D */
	DMA2D->CR      = 0x00010000UL | (1 << 9);
	DMA2D->FGMAR   = (U32)pSrc;
	DMA2D->OMAR    = (U32)pDst;
	DMA2D->FGOR    = OffSrc;
	DMA2D->OOR     = OffDst;
	DMA2D->FGPFCCR = LTDC_PIXEL_FORMAT_L8;
	DMA2D->OPFCCR  = PixelFormatDst;
	DMA2D->NLR     = (U32)(xSize << 16) | ySize;

	/* 启动传输 */
	DMA2D->CR   |= DMA2D_CR_START;   

	/* 等待DMA2D传输完成 */
	while (DMA2D->CR & DMA2D_CR_START) {}  
}

#if DrawBitmapA4Enalbe == 1
/*
*********************************************************************************************************
*	函 数 名: _DMA_DrawBitmapA4
*	功能说明: 绘制A4格式位图
*	形    参: pSrc             源数据地址
*             pDst             目的数据地址
*             OffSrc           源数据行偏移
*             OffDst           目的数据行偏移
*             PixelFormatDst   转换的颜色格式
*             xSize            位图长
*             ySize            位图高
*	返 回 值: 0
*********************************************************************************************************
*/
static int _DMA_DrawBitmapA4(void * pSrc, void * pDst,  U32 OffSrc, U32 OffDst, U32 PixelFormatDst, U32 xSize, U32 ySize) 
{
	U8 * pRD;
	U8 * pWR;
	U32 NumBytes, Color, Index;

	NumBytes = ((xSize + 1) & ~1) * ySize;
	if ((NumBytes > sizeof(_aBuffer)) || (NumBytes == 0)) 
	{
		return 1;
	}
	
	pWR = (U8 *)_aBuffer;
	pRD = (U8 *)pSrc;
	do 
	{
		*pWR++ = _aMirror[*pRD++];
	} while (--NumBytes);

	Index = LCD_GetColorIndex();
	Color = LCD_Index2Color(Index);

	/* 配置DMA2D */
	DMA2D->CR = 0x00020000UL;
	DMA2D->FGCOLR  = ((Color & 0xFF) << 16)
			       |  (Color & 0xFF00)
			       | ((Color >> 16) & 0xFF);
	DMA2D->FGMAR   = (U32)_aBuffer;
	DMA2D->FGOR    = 0;
	DMA2D->FGPFCCR = 0xA;
	DMA2D->NLR     = (U32)((xSize + OffSrc) << 16) | ySize;
	DMA2D->BGMAR   = (U32)pDst;
	DMA2D->BGOR    = OffDst - OffSrc;
	DMA2D->BGPFCCR = PixelFormatDst;
	DMA2D->OMAR    = DMA2D->BGMAR;
	DMA2D->OOR     = DMA2D->BGOR;
	DMA2D->OPFCCR  = DMA2D->BGPFCCR;

	/* 启动传输 */
	DMA2D->CR   |= DMA2D_CR_START;   

	/* 等待DMA2D传输完成 */
	while (DMA2D->CR & DMA2D_CR_START) {} 

	return 0;
}
#endif

/*
*********************************************************************************************************
*	函 数 名: _DMA_DrawAlphaBitmap
*	功能说明: 绘制带透明通道的位图
*	形    参: pSrc             源数据地址
*             pDst             目的数据地址
*             xSize            位图长
*             ySize            位图高
*             OffLineSrc       源数据行偏移
*             OffLineDst       目的数据行偏移
*             PixelFormatDst   转换的颜色格式
*	返 回 值: 0
*********************************************************************************************************
*/
static void _DMA_DrawAlphaBitmap(void * pDst, const void * pSrc, int xSize, int ySize, int OffLineSrc, int OffLineDst, int PixelFormat) 
{
	/* 配置*/ 
	DMA2D->CR      = 0x00020000UL | (1 << 9);
	DMA2D->FGMAR   = (U32)pSrc;
	DMA2D->BGMAR   = (U32)pDst;
	DMA2D->OMAR    = (U32)pDst;
	DMA2D->FGOR    = OffLineSrc;
	DMA2D->BGOR    = OffLineDst;
	DMA2D->OOR     = OffLineDst;
	DMA2D->FGPFCCR = LTDC_PIXEL_FORMAT_ARGB8888;
	DMA2D->BGPFCCR = PixelFormat;
	DMA2D->OPFCCR  = PixelFormat;
	DMA2D->NLR     = (U32)(xSize << 16) | (U16)ySize;

	/* 启动传输 */
	DMA2D->CR   |= DMA2D_CR_START;   

	/* 等待DMA2D传输完成 */
	while (DMA2D->CR & DMA2D_CR_START) {} 
}

/*
*********************************************************************************************************
*	函 数 名: _DMA_LoadLUT
*	功能说明: 加载颜色表
*	形    参: pColor     颜色表地址
*             NumItems   要加载的个数
*	返 回 值: 0
*********************************************************************************************************
*/
static void _DMA_LoadLUT(LCD_COLOR * pColor, U32 NumItems)
{
	/* 配置DMA2D */
	DMA2D->FGCMAR  = (U32)pColor;
	DMA2D->FGPFCCR  = LTDC_PIXEL_FORMAT_RGB888
				  | ((NumItems - 1) & 0xFF) << 8;
	
	/* 启动加载 */
	DMA2D->FGPFCCR |= (1 << 5);
}

/*
*********************************************************************************************************
*	函 数 名: _DMA_AlphaBlending
*	功能说明: 前景层和背景层混合输出
*	形    参: pColorFG    前景层颜色数据地址
*             pColorBG    背景层颜色数据地址
*             pColorDst   输出目标区地址
*             NumItems    转换次数
*	返 回 值: 无
*********************************************************************************************************
*/
static void _DMA_AlphaBlending(LCD_COLOR * pColorFG, LCD_COLOR * pColorBG, LCD_COLOR * pColorDst, U32 NumItems) 
{
	_DMA_AlphaBlendingBulk(pColorFG, pColorBG, pColorDst, NumItems);
}

/*
*********************************************************************************************************
*	函 数 名: _DMA_Index2ColorBulk
*	功能说明: 通过DMA2D，将当前显示屏的颜色数据转换为emWin的32位ARGB颜色数据。
*	形    参: pIndex       显示屏颜色地址
*             pColor       转换成适用于emWin的颜色地址
*             NumItems     转换的颜色数量
*             SizeOfIndex  未使用
*             PixelFormat  显示屏当前使用的颜色格式
*	返 回 值: 无
*********************************************************************************************************
*/
static void _DMA_Index2ColorBulk(void * pIndex, LCD_COLOR * pColor, U32 NumItems, U8 SizeOfIndex, U32 PixelFormat) 
{
	_DMA_ConvertColor(pIndex, pColor, PixelFormat, LTDC_PIXEL_FORMAT_ARGB8888, NumItems);
}

/*
*********************************************************************************************************
*	函 数 名: _DMA_Color2IndexBulk
*	功能说明: 通过DMA2D，将emWin的32位ARGB颜色数据转换为适用于当前显示屏的颜色数据
*	形    参: pIndex       显示屏颜色地址
*             pColor       转换成适用于emWin的颜色地址
*             NumItems     转换的颜色数量
*             SizeOfIndex  未使用
*             PixelFormat  显示屏当前使用的颜色格式
*	返 回 值: 无
*********************************************************************************************************
*/
static void _DMA_Color2IndexBulk(LCD_COLOR * pColor, void * pIndex, U32 NumItems, U8 SizeOfIndex, U32 PixelFormat)
{
	_DMA_ConvertColor(pColor, pIndex, LTDC_PIXEL_FORMAT_ARGB8888, PixelFormat, NumItems);
}

/*
*********************************************************************************************************
*	函 数 名: _DMA_MixColorsBulk
*	功能说明: 将一块显示区的前景色和背景色进行混合
*	形    参: pFG   前景色地址
*             pBG   背景色地址
*             pDst  混合后颜色存储的地址
*             OffFG    前景色偏移地址
*             OffBG    背景色偏移地址
*             OffDest  混合后偏移地址
*             xSize    显示区x轴大小
*             ySize    显示区y轴大小
*             Intens   即alpha值
*	返 回 值: 无
*********************************************************************************************************
*/
static void _LCD_MixColorsBulk(U32 * pFG, U32 * pBG, U32 * pDst, unsigned OffFG, unsigned OffBG, unsigned OffDest, unsigned xSize, unsigned ySize, U8 Intens) 
{
	int y;

	GUI_USE_PARA(OffFG);
	GUI_USE_PARA(OffDest);

	for (y = 0; y < ySize; y++) 
	{
		_DMA_MixColorsBulk(pFG, pBG, pDst, Intens, xSize);
		pFG  += xSize + OffFG;
		pBG  += xSize + OffBG;
		pDst += xSize + OffDest;
	}
}

/*
*********************************************************************************************************
*	函 数 名: _GetBufferSize
*	功能说明: 获取指定层显存大小
*	形    参: LayerIndex    图层
*	返 回 值: 显存大小
*********************************************************************************************************
*/
static U32 _GetBufferSize(int LayerIndex) 
{
	U32 BufferSize;

	BufferSize = _axSize[LayerIndex] * _aySize[LayerIndex] * _aBytesPerPixels[LayerIndex];

	return BufferSize;
}

/*
*********************************************************************************************************
*	函 数 名: _LCD_CopyBuffer
*	功能说明: 此函数用于多缓冲，将一个缓冲中的所有数据复制到另一个缓冲。
*	形    参: LayerIndex    图层
*             IndexSrc      源缓冲序号
*             IndexDst      目标缓冲序号
*	返 回 值: 无
*********************************************************************************************************
*/
static void _LCD_CopyBuffer(int LayerIndex, int IndexSrc, int IndexDst) 
{
	U32 BufferSize, AddrSrc, AddrDst;

	BufferSize = _GetBufferSize(LayerIndex);
	AddrSrc    = _aAddr[LayerIndex] + BufferSize * IndexSrc;
	AddrDst    = _aAddr[LayerIndex] + BufferSize * IndexDst;
	_DMA_Copy(LayerIndex, (void *)AddrSrc, (void *)AddrDst, _axSize[LayerIndex], _aySize[LayerIndex], 0, 0);
	
	/* 绘制操作切换到缓冲Buffer[IndexDst] */
	_aBufferIndex[LayerIndex] = IndexDst;
}

/*
*********************************************************************************************************
*	函 数 名: _LCD_CopyRect
*	功能说明: 此函数用于多缓冲，将一个缓冲中指定区域数据复制到另一个缓冲。
*	形    参: LayerIndex    图层
*             x0            源缓冲x轴位置
*             y0            源缓冲y轴位置
*             x1            目标冲x轴位置
*             y1            目标冲y轴位置
*             xSize         要复制的x轴大小
*             ySize         要复制的y轴大小
*	返 回 值: 无
*********************************************************************************************************
*/
static void _LCD_CopyRect(int LayerIndex, int x0, int y0, int x1, int y1, int xSize, int ySize)
{
	U32 BufferSize, AddrSrc, AddrDst;
	int OffLine;

	BufferSize = _GetBufferSize(LayerIndex);
	AddrSrc = _aAddr[LayerIndex] + BufferSize * _aBufferIndex[LayerIndex] + (y0 * _axSize[LayerIndex] + x0) * _aBytesPerPixels[LayerIndex];
	AddrDst = _aAddr[LayerIndex] + BufferSize * _aBufferIndex[LayerIndex] + (y1 * _axSize[LayerIndex] + x1) * _aBytesPerPixels[LayerIndex];
	OffLine = _axSize[LayerIndex] - xSize;
	_DMA_Copy(LayerIndex, (void *)AddrSrc, (void *)AddrDst, xSize, ySize, OffLine, OffLine);
}

/*
*********************************************************************************************************
*	函 数 名: _LCD_FillRect
*	功能说明: 对指定的区域进行颜色填充
*	形    参: LayerIndex    图层
*             x0            起始x轴位置
*             y0            起始y轴位置
*             x1            结束x轴位置
*             y1            结束y轴位置
*             PixelIndex    要填充的颜色值
*	返 回 值: 无
*********************************************************************************************************
*/
static void _LCD_FillRect(int LayerIndex, int x0, int y0, int x1, int y1, U32 PixelIndex) 
{
	U32 BufferSize, AddrDst;
	int xSize, ySize;

	if (GUI_GetDrawMode() == GUI_DM_XOR) 
	{
		LCD_SetDevFunc(LayerIndex, LCD_DEVFUNC_FILLRECT, NULL);
		LCD_FillRect(x0, y0, x1, y1);
		LCD_SetDevFunc(LayerIndex, LCD_DEVFUNC_FILLRECT, (void(*)(void))_LCD_FillRect);
	}
	else
	{
		xSize = x1 - x0 + 1;
		ySize = y1 - y0 + 1;
		BufferSize = _GetBufferSize(LayerIndex);
		AddrDst = _aAddr[LayerIndex] + BufferSize * _aBufferIndex[LayerIndex] + (y0 * _axSize[LayerIndex] + x0) * _aBytesPerPixels[LayerIndex];
		_DMA_Fill(LayerIndex, (void *)AddrDst, xSize, ySize, _axSize[LayerIndex] - xSize, PixelIndex);
	}
}

/*
*********************************************************************************************************
*	函 数 名: _LCD_DrawBitmap32bpp
*	功能说明: 绘制32bpp格式位图
*	形    参: LayerIndex      图层   
*             x               X轴起始位置
*             y               Y轴起始位置
*             p               源数据地址
*             xSize           位图长
*             ySize           位图高
*             BytesPerLine    每行字节数
*	返 回 值: 无
*********************************************************************************************************
*/
static void _LCD_DrawBitmap32bpp(int LayerIndex, int x, int y, U16 const * p, int xSize, int ySize, int BytesPerLine) 
{
	U32 BufferSize, AddrDst;
	int OffLineSrc, OffLineDst;

	BufferSize = _GetBufferSize(LayerIndex);
	AddrDst = _aAddr[LayerIndex] + BufferSize * _aBufferIndex[LayerIndex] + (y * _axSize[LayerIndex] + x) * _aBytesPerPixels[LayerIndex];
	OffLineSrc = (BytesPerLine / 4) - xSize;
	OffLineDst = _axSize[LayerIndex] - xSize;
	_DMA_Copy(LayerIndex, (void *)p, (void *)AddrDst, xSize, ySize, OffLineSrc, OffLineDst);
}

/*
*********************************************************************************************************
*	函 数 名: _LCD_DrawBitmap16bpp
*	功能说明: 绘制16bpp格式位图
*	形    参: LayerIndex      图层   
*             x               X轴起始位置
*             y               Y轴起始位置
*             p               源数据地址
*             xSize           位图长
*             ySize           位图高
*             BytesPerLine    每行字节数
*	返 回 值: 无
*********************************************************************************************************
*/
void _LCD_DrawBitmap16bpp(int LayerIndex, int x, int y, U16 const * p, int xSize, int ySize, int BytesPerLine) 
{
	U32 BufferSize, AddrDst;
	int OffLineSrc, OffLineDst;

	BufferSize = _GetBufferSize(LayerIndex);
	AddrDst = _aAddr[LayerIndex] + BufferSize * _aBufferIndex[LayerIndex] + (y * _axSize[LayerIndex] + x) * _aBytesPerPixels[LayerIndex];
	OffLineSrc = (BytesPerLine / 2) - xSize;
	OffLineDst = _axSize[LayerIndex] - xSize;
	_DMA_Copy(LayerIndex, (void *)p, (void *)AddrDst, xSize, ySize, OffLineSrc, OffLineDst);
}

/*
*********************************************************************************************************
*	函 数 名: _LCD_DrawBitmap8bpp
*	功能说明: 绘制8bpp格式位图
*	形    参: LayerIndex      图层   
*             x               X轴起始位置
*             y               Y轴起始位置
*             p               源数据地址
*             xSize           位图长
*             ySize           位图高
*             BytesPerLine    每行字节数
*	返 回 值: 无
*********************************************************************************************************
*/
static void _LCD_DrawBitmap8bpp(int LayerIndex, int x, int y, U8 const * p, int xSize, int ySize, int BytesPerLine) {
	U32 BufferSize, AddrDst;
	int OffLineSrc, OffLineDst;
	U32 PixelFormat;

	PixelFormat = _GetPixelformat(LayerIndex);
	BufferSize = _GetBufferSize(LayerIndex);
	AddrDst = _aAddr[LayerIndex] + BufferSize * _aBufferIndex[LayerIndex] + (y * _axSize[LayerIndex] + x) * _aBytesPerPixels[LayerIndex];
	OffLineSrc = BytesPerLine - xSize;
	OffLineDst = _axSize[LayerIndex] - xSize;
	_DMA_DrawBitmapL8((void *)p, (void *)AddrDst, OffLineSrc, OffLineDst, PixelFormat, xSize, ySize);
}

#if DrawBitmapA4Enalbe == 1
/*
*********************************************************************************************************
*	函 数 名: _LCD_DrawBitmap4bpp
*	功能说明: 绘制4bpp格式位图
*	形    参: LayerIndex      图层   
*             x               X轴起始位置
*             y               Y轴起始位置
*             p               源数据地址
*             xSize           位图长
*             ySize           位图高
*             BytesPerLine    每行字节数
*	返 回 值: 无
*********************************************************************************************************
*/
static int _LCD_DrawBitmap4bpp(int LayerIndex, int x, int y, U8 const * p, int xSize, int ySize, int BytesPerLine) {
	U32 BufferSize, AddrDst;
	int OffLineSrc, OffLineDst;
	U32 PixelFormat;

	if (x < 0) 
	{
		return 1;
	}
	
	if ((x + xSize) >= _axSize[LayerIndex]) 
	{
		return 1;
	}
	
	if (y < 0) 
	{
		return 1;
	}
	
	if ((y + ySize) >= _aySize[LayerIndex]) 
	{
		return 1;
	}
	
	PixelFormat = _GetPixelformat(LayerIndex);

	if (PixelFormat > LTDC_PIXEL_FORMAT_ARGB4444) 
	{
		return 1;
	}
	
	BufferSize = _GetBufferSize(LayerIndex);
	AddrDst = _aAddr[LayerIndex] + BufferSize * _aBufferIndex[LayerIndex] + (y * _axSize[LayerIndex] + x) * _aBytesPerPixels[LayerIndex];
	OffLineSrc = (BytesPerLine * 2) - xSize;
	OffLineDst = _axSize[LayerIndex] - xSize;
	return _DMA_DrawBitmapA4((void *)p, (void *)AddrDst, OffLineSrc, OffLineDst, PixelFormat, xSize, ySize);;
}
#endif

/*
*********************************************************************************************************
*	函 数 名: _LCD_DrawMemdev16bpp
*	功能说明: 绘制16bpp存储设备
*	形    参: pDst               源数据地址   
*             pSrc               目的数据地址
*             xSize              存储设备长
*             ySize              存储设备高
*             BytesPerLineDst    源数据每行字节数
*             BytesPerLineSrc    目的数据每行字节数
*	返 回 值: 无
*********************************************************************************************************
*/
static void _LCD_DrawMemdev16bpp(void * pDst, const void * pSrc, int xSize, int ySize, int BytesPerLineDst, int BytesPerLineSrc) 
{
	int OffLineSrc, OffLineDst;

	OffLineSrc = (BytesPerLineSrc / 2) - xSize;
	OffLineDst = (BytesPerLineDst / 2) - xSize;
	_DMA_CopyRGB565(pSrc, pDst, xSize, ySize, OffLineSrc, OffLineDst);
}

/*
*********************************************************************************************************
*	函 数 名: _LCD_DrawMemdevAlpha
*	功能说明: 绘制带Alpha通道的存储设备
*	形    参: pDst               源数据地址   
*             pSrc               目的数据地址
*             xSize              存储设备长
*             ySize              存储设备高
*             BytesPerLineDst    源数据每行字节数
*             BytesPerLineSrc    目的数据每行字节数
*	返 回 值: 无
*********************************************************************************************************
*/
static void _LCD_DrawMemdevAlpha(void * pDst, const void * pSrc, int xSize, int ySize, int BytesPerLineDst, int BytesPerLineSrc) 
{
	int OffLineSrc, OffLineDst;

	OffLineSrc = (BytesPerLineSrc / 4) - xSize;
	OffLineDst = (BytesPerLineDst / 4) - xSize;
	_DMA_DrawAlphaBitmap(pDst, pSrc, xSize, ySize, OffLineSrc, OffLineDst, LTDC_PIXEL_FORMAT_ARGB8888);
}

/*
*********************************************************************************************************
*	函 数 名: _LCD_DrawBitmapAlpha
*	功能说明: 绘制带Alpha通道的位图
*	形    参: LayerIndex      图层   
*             x               X轴起始位置
*             y               Y轴起始位置
*             p               源数据地址
*             xSize           位图长
*             ySize           位图高
*             BytesPerLine    每行字节数
*	返 回 值: 无
*********************************************************************************************************
*/
static void _LCD_DrawBitmapAlpha(int LayerIndex, int x, int y, const void * p, int xSize, int ySize, int BytesPerLine) 
{
	U32 BufferSize, AddrDst;
	int OffLineSrc, OffLineDst;
	U32 PixelFormat;

	PixelFormat = _GetPixelformat(LayerIndex);
	BufferSize = _GetBufferSize(LayerIndex);
	AddrDst = _aAddr[LayerIndex] + BufferSize * _aBufferIndex[LayerIndex] + (y * _axSize[LayerIndex] + x) * _aBytesPerPixels[LayerIndex];
	OffLineSrc = (BytesPerLine / 4) - xSize;
	OffLineDst = _axSize[LayerIndex] - xSize;
	_DMA_DrawAlphaBitmap((void *)AddrDst, p, xSize, ySize, OffLineSrc, OffLineDst, PixelFormat);
}

/*
*********************************************************************************************************
*	函 数 名: _LCD_GetpPalConvTable
*	功能说明: 转换颜色板以适应控制器设置的颜色格式。
*	形    参: pLogPal   源颜色板地址
*             pBitmap   位图地址
*             LayerIndex  源颜色格式
*	返 回 值: 转换后的颜色板地址
*********************************************************************************************************
*/
static LCD_PIXELINDEX * _LCD_GetpPalConvTable(const LCD_LOGPALETTE GUI_UNI_PTR * pLogPal, const GUI_BITMAP GUI_UNI_PTR * pBitmap, int LayerIndex) 
{
	void (* pFunc)(void);
	int DoDefault = 0;

	/* 8bpp */
	if (pBitmap->BitsPerPixel == 8) 
	{
		pFunc = LCD_GetDevFunc(LayerIndex, LCD_DEVFUNC_DRAWBMP_8BPP);
		if (pFunc) 
		{
			if (pBitmap->pPal) 
			{
				if (pBitmap->pPal->HasTrans) 
				{
					DoDefault = 1;
				}
			}
			else
			{
				DoDefault = 1;
			}
		}
		else
		{
			DoDefault = 1;
		}
	}
	else 
	{
	DoDefault = 1;
	}

	/* 默认颜色板管理 */
	if (DoDefault) 
	{
		/* 返回索引值 */
		return LCD_GetpPalConvTable(pLogPal);
	}

	/* DMA2D加载LUT */
	_DMA_LoadLUT((U32 *)pLogPal->pPalEntries, pLogPal->NumEntries);

	/* 返回非NULL */
	return _pBuffer_DMA2D;
}

/*
*********************************************************************************************************
*	函 数 名: LCD_X_DisplayDriver
*	功能说明: 显示驱动设置
*	形    参: LayerIndex   图层
*             Cmd          命令
*             pData        数据地址
*	返 回 值: 成功返回0，失败返回-1
*********************************************************************************************************
*/
int LCD_X_DisplayDriver(unsigned LayerIndex, unsigned Cmd, void * pData) 
{
	int r = 0;
	U32 addr;

	switch (Cmd) 
	{
		case LCD_X_INITCONTROLLER: 
		{
			/* 驱动初始化 */
			LCD_LL_LayerInit(LayerIndex);
			break;
		}
		
		case LCD_X_SETORG: 
		{
			/* 设置显存地址 */
			LCD_X_SETORG_INFO * p;

			p = (LCD_X_SETORG_INFO *)pData;
			addr = _aAddr[LayerIndex] + p->yPos * _axSize[LayerIndex] * _aBytesPerPixels[LayerIndex];
			HAL_LTDC_SetAddress(&hltdc, addr, LayerIndex);
			break;
		}
		
		case LCD_X_SHOWBUFFER: 
		{
			/* 用于多缓冲，参数Index用于指示使用的那个缓冲 */
			LCD_X_SHOWBUFFER_INFO * p;

			p = (LCD_X_SHOWBUFFER_INFO *)pData;
			_aPendingBuffer[LayerIndex] = p->Index;
			break;
		}
		
		case LCD_X_SETLUTENTRY: 
		{
			/* 颜色查找表设置 */
			LCD_X_SETLUTENTRY_INFO * p;

			p = (LCD_X_SETLUTENTRY_INFO *)pData;
			HAL_LTDC_ConfigCLUT(&hltdc, (uint32_t*)p->Color, p->Pos, LayerIndex);
			break;
		}
		
		case LCD_X_ON: 
		{
			/* 使能LTDC  */
			__HAL_LTDC_ENABLE(&hltdc);
			break;
		}
		
		case LCD_X_OFF: 
		{
			/* 关闭LTDC */
			__HAL_LTDC_DISABLE(&hltdc);
			break;
		}
		
		case LCD_X_SETVIS: 
		{
			/* 图层的开关 */
			LCD_X_SETVIS_INFO * p;

			p = (LCD_X_SETVIS_INFO *)pData;
			if(p->OnOff == ENABLE )
			{
				__HAL_LTDC_LAYER_ENABLE(&hltdc, LayerIndex); 
			}
			else
			{
				__HAL_LTDC_LAYER_DISABLE(&hltdc, LayerIndex);
			}
			
			__HAL_LTDC_RELOAD_IMMEDIATE_CONFIG(&hltdc);
			break;
		}
		
		case LCD_X_SETPOS:
		{
			/* 设置图层显示位置 */
			LCD_X_SETPOS_INFO * p;

			p = (LCD_X_SETPOS_INFO *)pData;    
			HAL_LTDC_SetWindowPosition(&hltdc, p->xPos, p->yPos, LayerIndex);
			break;
		}
		
		case LCD_X_SETSIZE:
		{
			/* 设置图层大小 */
			LCD_X_SETSIZE_INFO * p;
			int xPos, yPos;

			GUI_GetLayerPosEx(LayerIndex, &xPos, &yPos);
			p = (LCD_X_SETSIZE_INFO *)pData;
			if (LCD_GetSwapXYEx(LayerIndex))
			{
				_axSize[LayerIndex] = p->ySize;
				_aySize[LayerIndex] = p->xSize;
			}
			else
			{
				_axSize[LayerIndex] = p->xSize;
				_aySize[LayerIndex] = p->ySize;
			}
			
			HAL_LTDC_SetWindowPosition(&hltdc, xPos, yPos, LayerIndex);
			break;
		}
		
		case LCD_X_SETALPHA: 
		{
			/* 设置透明 */
			LCD_X_SETALPHA_INFO * p;

			p = (LCD_X_SETALPHA_INFO *)pData;
			HAL_LTDC_SetAlpha(&hltdc, p->Alpha, LayerIndex);
			break;
		}
		
		case LCD_X_SETCHROMAMODE: 
		{
			/* 使能或禁止扣色 */
			LCD_X_SETCHROMAMODE_INFO * p;

			p = (LCD_X_SETCHROMAMODE_INFO *)pData;
			if(p->ChromaMode != 0)
			{
				HAL_LTDC_EnableColorKeying(&hltdc, LayerIndex);
			}
			else
			{
				HAL_LTDC_DisableColorKeying(&hltdc, LayerIndex);      
			}
			break;
		}
		
		case LCD_X_SETCHROMA: 
		{
			/* 设置扣色 */
			LCD_X_SETCHROMA_INFO * p;
			U32 Color;

			p = (LCD_X_SETCHROMA_INFO *)pData;
			Color = ((p->ChromaMin & 0xFF0000) >> 16) | (p->ChromaMin & 0x00FF00) | ((p->ChromaMin & 0x0000FF) << 16);
			HAL_LTDC_ConfigColorKeying(&hltdc, Color, LayerIndex);
			break;
		}
		
		default:
			r = -1;
			break;
	}

	return r;
}

/*
*********************************************************************************************************
*	函 数 名: LCD_X_DisplayDriver
*	功能说明: 初始化配置
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD_X_Config(void) 
{
	int i;
	U32 PixelFormat;

	/* 底层初始化 */
	LCD_LL_Init ();

#if ClearCacheHookEnalbe == 1
	GUI_DCACHE_SetClearCacheHook(_ClearCacheHook);
#endif

	/* 初始化多缓冲 */
#if (NUM_BUFFERS > 1)
	for (i = 0; i < GUI_NUM_LAYERS; i++) 
	{
		GUI_MULTIBUF_ConfigEx(i, NUM_BUFFERS);
	}
#endif

	/* 设置显示驱动和颜色格式转换 */
	GUI_DEVICE_CreateAndLink(DISPLAY_DRIVER_0, COLOR_CONVERSION_0, 0, 0);

	/* 设置图层1 */
	if (LCD_GetSwapXYEx(0)) 
	{
		LCD_SetSizeEx (0,  g_LcdHeight, g_LcdWidth);
		LCD_SetVSizeEx(0,  g_LcdHeight * NUM_VSCREENS, g_LcdWidth);
	}
	else
	{
		LCD_SetSizeEx (0, g_LcdWidth, g_LcdHeight);
		LCD_SetVSizeEx(0, g_LcdWidth, g_LcdHeight * NUM_VSCREENS);
	}
	
#if (GUI_NUM_LAYERS > 1)
	/* 设置显示驱动和颜色格式转换 */
	GUI_DEVICE_CreateAndLink(DISPLAY_DRIVER_1, COLOR_CONVERSION_1, 0, 1);

	/* 设置图层2 */
	if (LCD_GetSwapXYEx(1)) 
	{
		LCD_SetSizeEx (1, g_LcdHeight, g_LcdWidth);
		LCD_SetVSizeEx(1,  g_LcdHeight * NUM_VSCREENS, g_LcdWidth);
	}
	else 
	{
		LCD_SetSizeEx (1, g_LcdWidth, g_LcdHeight);
		LCD_SetVSizeEx(1, g_LcdWidth, g_LcdHeight * NUM_VSCREENS);
	}
#endif
	
	/* 设置RAM地址和每个像素的字节数 */
	for (i = 0; i < GUI_NUM_LAYERS; i++) 
	{
		/* 设置RAM地址 */
		LCD_SetVRAMAddrEx(i, (void *)(_aAddr[i]));
		
		/* 每个像素的字节数 */
		_aBytesPerPixels[i] = LCD_GetBitsPerPixelEx(i) >> 3;
	}
	
	/* 设置函数重定向 */
	for (i = 0; i < GUI_NUM_LAYERS; i++) 
	{
		PixelFormat = _GetPixelformat(i);
		
		/* CopyBuffer重定向，多缓冲使用 */
		LCD_SetDevFunc(i, LCD_DEVFUNC_COPYBUFFER, (void(*)(void))_LCD_CopyBuffer);

		if (PixelFormat <= LTDC_PIXEL_FORMAT_ARGB4444) 
		{
			/* 填充使用 */
			LCD_SetDevFunc(i, LCD_DEVFUNC_FILLRECT, (void(*)(void))_LCD_FillRect);
		}

		if (_aOrientation[i] == ROTATION_0)
		{
			/* CopyRect重定向 */
			LCD_SetDevFunc(i, LCD_DEVFUNC_COPYRECT, (void(*)(void))_LCD_CopyRect);

			/* 8bpp重定向 */
			if (PixelFormat <= LTDC_PIXEL_FORMAT_ARGB4444) 
			{
				LCD_SetDevFunc(i, LCD_DEVFUNC_DRAWBMP_8BPP, (void(*)(void))_LCD_DrawBitmap8bpp);
			}

			/* 16bpp重定向 */
			if (PixelFormat == LTDC_PIXEL_FORMAT_RGB565) 
			{
				LCD_SetDevFunc(i, LCD_DEVFUNC_DRAWBMP_16BPP, (void(*)(void))_LCD_DrawBitmap16bpp);
			}

			/* 32bpp重定向 */
			if (PixelFormat == LTDC_PIXEL_FORMAT_ARGB8888) 
			{
				LCD_SetDevFunc(i, LCD_DEVFUNC_DRAWBMP_32BPP, (void(*)(void))_LCD_DrawBitmap32bpp);
			}
		}
	}
	/* DMA2D for ARGB1555 */
	GUICC_M1555I_SetCustColorConv(_Color2IndexBulk_M1555I_DMA2D, _Index2ColorBulk_M1555I_DMA2D);

	/* DMA2D for RGB565 */  
	GUICC_M565_SetCustColorConv  (_Color2IndexBulk_M565_DMA2D,   _Index2ColorBulk_M565_DMA2D);

	/* DMA2D for ARGB4444 */
	GUICC_M4444I_SetCustColorConv(_Color2IndexBulk_M4444I_DMA2D, _Index2ColorBulk_M4444I_DMA2D);

	/* DMA2D for RGB888 */
	GUICC_M888_SetCustColorConv  (_Color2IndexBulk_M888_DMA2D,   _Index2ColorBulk_M888_DMA2D);

	/* DMA2D for ARGB8888 */
	GUICC_M8888I_SetCustColorConv(_Color2IndexBulk_M8888I_DMA2D, _Index2ColorBulk_M8888I_DMA2D);

	/* Alpha混合重定向 */
	GUI_SetFuncAlphaBlending(_DMA_AlphaBlending);

	GUI_SetFuncGetpPalConvTable(_LCD_GetpPalConvTable);

	/* 颜色混合重定向 */
	GUI_SetFuncMixColorsBulk(_LCD_MixColorsBulk);

#if DrawBitmapA4Enalbe == 1
	GUI_AA_SetpfDrawCharAA4(_LCD_DrawBitmap4bpp); /* 这个函数有问题，未使用 */
#endif

	/* 16bpp存储设备重定向 */
	GUI_MEMDEV_SetDrawMemdev16bppFunc(_LCD_DrawMemdev16bpp);

	/* Alpha绘制重定向 */
	GUI_SetFuncDrawAlpha(_LCD_DrawMemdevAlpha, _LCD_DrawBitmapAlpha);
}

#if 1
/*
*********************************************************************************************************
*	函 数 名: LTDC_IRQHandler
*	功能说明: 初始化配置
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void LTDC_IRQHandler(void) 
{
	U32 Addr;
	int i;

	LTDC->ICR = (U32)LTDC_IER_LIE;
	
	for (i = 0; i < GUI_NUM_LAYERS; i++) 
	{
		if (_aPendingBuffer[i] >= 0) 
		{
			/* 计算缓冲地址 */
			Addr = _aAddr[i] + _axSize[i] * _aySize[i] * _aPendingBuffer[i] * _aBytesPerPixels[i];

			/* 更新LTDC寄存器 */	  
			__HAL_LTDC_LAYER(&hltdc, i)->CFBAR = Addr;     
			__HAL_LTDC_RELOAD_IMMEDIATE_CONFIG(&hltdc);   

			/* 告诉emWin使用的缓冲 */
			GUI_MULTIBUF_ConfirmEx(i, _aPendingBuffer[i]);

			/* 清除标志 */
			_aPendingBuffer[i] = -1;
		}
	}
}

#else
/*
*********************************************************************************************************
*	函 数 名: LTDC_IRQHandler
*	功能说明: 初始化配置
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void LTDC_IRQHandler(void)
{
	HAL_LTDC_IRQHandler(&hltdc);
}

void HAL_LTDC_LineEvenCallback(LTDC_HandleTypeDef *hltdc) 
{
	U32 Addr;
	int i;

	for (i = 0; i < GUI_NUM_LAYERS; i++) 
	{
		if (_aPendingBuffer[i] >= 0) 
		{
			/* 计算缓冲地址 */
			Addr = _aAddr[i] + _axSize[i] * _aySize[i] * _aPendingBuffer[i] * _aBytesPerPixels[i];

			/* 更新LTDC寄存器 */	  
			__HAL_LTDC_LAYER(hltdc, i)->CFBAR = Addr;     
			__HAL_LTDC_RELOAD_IMMEDIATE_CONFIG(hltdc);   

			/* 告诉emWin使用的缓冲 */
			GUI_MULTIBUF_ConfirmEx(i, _aPendingBuffer[i]);

			/* 清除标志 */
			_aPendingBuffer[i] = -1;
		}
	}

	/* 重新开启行中断，因为函数HAL_LTDC_IRQHandler里面会关闭 */
	HAL_LTDC_ProgramLineEvent(hltdc, 0);
}

#endif

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
