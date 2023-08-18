/*
*********************************************************************************************************
*
*	ģ������ : emWin�ĵײ������ļ�
*	�ļ����� : LCDConf_Lin_Template.c
*	��    �� : V1.0 
*	˵    �� : LCD�Ĳ���������emWin�ײ�ӿڶ�������ļ�ʵ�֡�
*              ʹ��˵����
*                1. ����������Ӧ������4.3�磬5���7������
*                2. �û�ʹ��emWinǰ������ʹ��STM32��CRCʱ�ӣ�֮�����GUI_Init()����ʹ��emWin��
*                3. ��ͬ����ʾ�����õĴ���������ͬ���û��Լ�ʹ�õĻ����������Ե��ü��ɡ�
*                   �������ǣ�TOUCH_Scan()��1ms����һ�Ρ�
*                   �������ǣ�GT911_OnePiontScan()��GT811_OnePiontScan()����FT5X06_OnePiontScan()��10ms����һ�Ρ�
*              ����˵����
*                1. H7��ͼ�����ɱ����㣬ͼ��1��ͼ��2��ɡ�
*                2. �����ж�ÿ������ѡ�����ϸ˵����
*              ��ֲ˵����
*                  ע���������㼴�ɣ�
*                1. �ṩ����LCD_SetBackLight��ʵ�ֱ���Ŀ��ء�
*                2. ����ȫ�ֱ���g_LcdWidth��g_LcdHeight��
*                3. �޸ĺ���LCD_ConfigLTDC���޸ı��ļ��к���LCD_LL_Init�����ʱ�������
*
*	�޸ļ�¼ :
*		�汾��    ����         ����        ˵��
*		V1.0    2019-05-25    Eric2013     �׷�
*
*
*	Copyright (C), 2019-2030, ���������� www.armfly.com
*
*********************************************************************************************************
*/
#include "bsp.h"
#include "GUI.h"
#include "GUI_Private.h"
#include "GUIDRV_Lin.h"



/*
**********************************************************************************************************
							�����ⲿ�ļ��ı����ͺ���(bsp_tft_429.c�ļ�)
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
									�û��������õ�ѡ��
**********************************************************************************************************
*/
/* 0 ��ʱ�ò����⼸������ */
#define DrawBitmapA4Enalbe    0
#define ClearCacheHookEnalbe  0

/* 
  1. ��ʾ��������ֱ��ʣ������Ѿ�������ʾ������Ӧ��֧��4.3�磬5���7����
     ������д����Ӧ��ʾ���е����ֱ��ʡ�
*/
#define XSIZE_PHYS       800
#define YSIZE_PHYS       480

/* 2. ��ת������δʹ�ô�ѡ�ֱ���ڹ�������ʱ����ת���� */
#define ROTATION_0       0
#define ROTATION_CW      1
#define ROTATION_180     2
#define ROTATION_CCW     3

/* 
   3. STM32H7֧�ֵ���ɫģʽ��
      (1) �������ʹ��24λɫ����32λɫ����ѡ��CMS_ARGB8888�����ʹ��16λɫ����ѡ��CMS_RGB565��������ɫ��ʽ������֧�֡�
	  (2) ����û�ѡ����ARGB8888����RGB888ģʽ��LCD��˸�Ļ������Ȳ鿴�Ƿ��Ǵ��������⣺
	      http://forum.armfly.com/forum.php?mod=viewthread&tid=16892 ��������F429��H7ϵ�У�
		  �������������⣬�ٽ���LTDC���ʱ�Ӽ��ɣ��ڱ��ļ��ĺ���LCD_ConfigLTDC����    
*/
#define CMS_ARGB8888 1
#define CMS_RGB888   2
#define CMS_RGB565   3
#define CMS_ARGB1555 4
#define CMS_ARGB4444 5
#define CMS_L8       6
#define CMS_AL44     7
#define CMS_AL88     8

/* 4. �໺�� / ���������໺�������������ͬʱʹ�ã�emWin��֧�� */
#define NUM_BUFFERS      3 /* ����໺�����������������1,2��3��Ҳ�������֧�������� */
#define NUM_VSCREENS     1 /* �������������� */

/* 
   5. �ض���ͼ����������STM32H7���û�����ѡ��һ��ͼ���������ͼ�㣬��֧����ͼ�� 
      (1). ����GUI_NUM_LAYERS = 1ʱ������ʹ��ͼ��1ʱ��Ĭ�ϴ���ֵ�Ƿ��͸�ͼ��1�ġ�
	  (2). ����GUI_NUM_LAYERS = 2ʱ����ͼ��1��ͼ��2���Ѿ�ʹ�ܣ���ʱͼ��2�Ƕ��㣬
	       �û���Ҫ�����Լ���ʹ������������������ط���
		   a. ��bsp_touch.c�ļ��еĺ���TOUCH_InitHard�������ò���State.Layer = 1��1�ͱ�ʾ
		      ��ͼ��2���ʹ���ֵ��
		   b. ����GUI_Init�����󣬵��ú���GUI_SelectLayer(1), ���õ�ǰ��������ͼ��2��
*/
#undef  GUI_NUM_LAYERS
#define GUI_NUM_LAYERS 1

/* 
   6. ����ͼ��1��ͼ��2��Ӧ���Դ��ַ
      (1) EXT_SDRAM_ADDR ��SDRAM���׵�ַ��
      (2) LCD_LAYER0_FRAME_BUFFER ��ͼ��1���Դ��ַ��
	  (3) LCD_LAYER1_FRAME_BUFFER ��ͼ��2���Դ��ַ��
	  (4) ÿ��ͼ����Դ��С�ȽϿ�������������¼򵥵�˵����
	      ����û�ѡ�����ɫģʽ = 32λɫARGB8888���Դ�Ĵ�С��
	      XSIZE_PHYS * YSIZE_PHYS * 4 * NUM_VSCREENS * NUM_BUFFERS
		  
	      ��ɫģʽ = 24λɫRGB888���Դ�Ĵ�С��
	      XSIZE_PHYS * YSIZE_PHYS * 3 * NUM_VSCREENS * NUM_BUFFERS
		  
	      ��ɫģʽ = 16λɫRGB566��ARGB1555, ARGB4444��AL88����ô�Դ�Ĵ�С���ǣ�
	      XSIZE_PHYS * YSIZE_PHYS * 2 * NUM_VSCREENS * NUM_BUFFERS

	      ��ɫģʽ = 8λɫL8��AL44����ô�Դ�Ĵ�С���ǣ�
	      XSIZE_PHYS * YSIZE_PHYS * 1 * NUM_VSCREENS * NUM_BUFFERS	
      
      ����Ϊ�˷�������������������׵�16MB��SDRAMǰ8MB�����LCD�Դ�ʹ�ã���8MB����emWin��̬�ڴ档
	  ����24λɫ��16λɫ��8λɫ���û����Զ���ʹ�������壬����ʹ��˫ͼ�㡣����32λɫҲʹ���������˫
	  ͼ��Ļ��ᳬ��8MB�������û������Լ���������Դ��emWin��̬�ڴ�ķ��������
	    ��һ�����ӣ�����800*480�ֱ��ʵ���ʾ����ʹ��32λɫ�������壬��ô����һ��ͼ����Ҫ�Ĵ�С����
      800 * 480 * 4 * 3  = 4.394MB�Ŀռ䣬�����˫ͼ�㣬�Ѿ�����8MB�ķ��䷶Χ��

      (5)Ϊ�˷��������ͼ��2�ĺ궨��LCD_LAYER1_FRAME_BUFFER�еĲ���4�ǰ���32λɫ���õģ�����û���ͼ��1
         ʹ�õ���8λɫ������������1,�����16λɫ��������2�������24λɫ��������3��
*/
#define LCD_LAYER0_FRAME_BUFFER  EXT_SDRAM_ADDR
#define LCD_LAYER1_FRAME_BUFFER  (LCD_LAYER0_FRAME_BUFFER + XSIZE_PHYS * YSIZE_PHYS * 4 * NUM_VSCREENS * NUM_BUFFERS)


/* 7. ����ͼ��1����ɫģʽ�ͷֱ��ʴ�С */
#define COLOR_MODE_0  CMS_RGB565
#define ORIENTATION_0 ROTATION_0
#define XSIZE_0       XSIZE_PHYS
#define YSIZE_0       YSIZE_PHYS

/* 8. ����ͼ��2�ĵ���ɫģʽ�ͷֱ��ʴ�С */
#define COLOR_MODE_1  CMS_RGB565
#define ORIENTATION_1 ROTATION_0
#define XSIZE_1       XSIZE_PHYS
#define YSIZE_1       YSIZE_PHYS

/* 9. û��ͼ�㼤��ʱ������ɫ����, ��ʱδ�õ� */
#define BK_COLOR      GUI_DARKBLUE


/* 10. ��ͼ������£������û�ѡ�����ɫģʽ���Զ�ѡ��ͼ��1��emWin����ɫģʽ */
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

/* 11. ��ͼ������£������û�ѡ�����ɫģʽ���Զ�ѡ��ͼ��1��emWin������ */
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


/* 12. ˫ͼ������£������û�ѡ�����ɫģʽ���Զ�ѡ��ͼ��2��emWin����ɫģʽ */
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

/* 13. ˫ͼ������£������û�ѡ�����ɫģʽ���Զ�ѡ��ͼ��2��emWin������ */
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

/*14. ����ѡ���⣬��ֹ���ô������ĳЩѡ��û������ */
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
									ʹ��DMA2D�ض�����ɫ������ת��
**********************************************************************************************************
*/
#define DEFINE_DMA2D_COLORCONVERSION(PFIX, PIXELFORMAT)                                                        \
static void _Color2IndexBulk_##PFIX##_DMA2D(LCD_COLOR * pColor, void * pIndex, U32 NumItems, U8 SizeOfIndex) { \
  _DMA_Color2IndexBulk(pColor, pIndex, NumItems, SizeOfIndex, PIXELFORMAT);                                    \
}                                                                                                              \
static void _Index2ColorBulk_##PFIX##_DMA2D(void * pIndex, LCD_COLOR * pColor, U32 NumItems, U8 SizeOfIndex) { \
  _DMA_Index2ColorBulk(pIndex, pColor, NumItems, SizeOfIndex, PIXELFORMAT);                                    \
}
 
/* �������� */
static void _DMA_Index2ColorBulk(void * pIndex, LCD_COLOR * pColor, U32 NumItems, U8 SizeOfIndex, U32 PixelFormat);
static void _DMA_Color2IndexBulk(LCD_COLOR * pColor, void * pIndex, U32 NumItems, U8 SizeOfIndex, U32 PixelFormat);

/* ��ɫת�� */
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

/* �ò��ϣ��������� */
#if DrawBitmapA4Enalbe == 1
static U32 _aBuffer[XSIZE_PHYS * sizeof(U32) * 3]__attribute__((at(0x24000000)));
static U32 * _pBuffer_DMA2D = &_aBuffer[XSIZE_PHYS * sizeof(U32) * 0];

/* ���� A4 bitmaps ��ʾ */
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

/* ��ɫת��ģʽ */
static const LCD_API_COLOR_CONV *_apColorConvAPI[] = {
  COLOR_CONVERSION_0,
#if GUI_NUM_LAYERS > 1
  COLOR_CONVERSION_1,
#endif
};

/* ��ʾ���� */
static const int _aOrientation[] = 
{
  ORIENTATION_0,
#if GUI_NUM_LAYERS > 1
  ORIENTATION_1,
#endif
};

/*
*********************************************************************************************************
*	�� �� ��: _ClearCacheHook
*	����˵��: ��Cache
*	��    ��: LayerIndex  ͼ��
*	�� �� ֵ: ��
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
*	�� �� ��: _GetPixelformat
*	����˵��: ��ȡͼ��1����ͼ��2ʹ�õ���ɫ��ʽ
*	��    ��: LayerIndex  ͼ��
*	�� �� ֵ: ��ɫ��ʽ
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
	
	/* ���ô����������� */
	while (1);
}

/*
*********************************************************************************************************
*	�� �� ��: _GetPixelformat
*	����˵��: ��ȡͼ��1����ͼ��2ʹ�õ���ɫ��ʽ
*	��    ��: LayerIndex  ͼ��
*	�� �� ֵ: ��ɫ��ʽ
*********************************************************************************************************
*/
static void LCD_LL_LayerInit(U32 LayerIndex) 
{  
	LTDC_LayerCfgTypeDef  layer_cfg;  
	static uint32_t       LUT[256];
	uint32_t              i;

	if (LayerIndex < GUI_NUM_LAYERS)
	{
		/* ������ʾ������ */ 
		layer_cfg.WindowX0 = 0;
		layer_cfg.WindowX1 = g_LcdWidth;
		layer_cfg.WindowY0 = 0;
		layer_cfg.WindowY1 = g_LcdHeight;
		
		/* ������ɫ��ʽ */ 
		layer_cfg.PixelFormat = _GetPixelformat(LayerIndex);
		
		/* �Դ��ַ */
		layer_cfg.FBStartAdress = layer_prop[LayerIndex].address;
		
		/* Alpha���� (255 ��ʾ��ȫ��͸��) */
		layer_cfg.Alpha = 255;
		
		/* �ޱ���ɫ */
		layer_cfg.Alpha0 = 0;   /* ��ȫ͸�� */
		layer_cfg.Backcolor.Blue = 0;
		layer_cfg.Backcolor.Green = 0;
		layer_cfg.Backcolor.Red = 0;
		
		/* ����ͼ�������� */
		layer_cfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_PAxCA;
		layer_cfg.BlendingFactor2 = LTDC_BLENDING_FACTOR2_PAxCA;

		/* �������д�С */
		layer_cfg.ImageWidth = g_LcdWidth;
		layer_cfg.ImageHeight = g_LcdHeight;

		/* ����ͼ��1 */
		HAL_LTDC_ConfigLayer(&hltdc, &layer_cfg, LayerIndex);

		/* ʹ��LUT */
		if (LCD_GetBitsPerPixelEx(LayerIndex) <= 8)
		{
			HAL_LTDC_EnableCLUT(&hltdc, LayerIndex);
		}
		else
		{
			/*  AL88ģʽ(16bpp) */
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
*	�� �� ��: LCD_LL_Init
*	����˵��: ����LTDC
*	��    ��: ��
*	�� �� ֵ: ��
*   ��    ��:
*       LCD_TFT ͬ��ʱ�����ã������Թٷ�����һ����ͼ���Լ����ࣩ��
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
*     ÿ��LCD�豸�����Լ���ͬ��ʱ��ֵ��
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
*     LCD_TFT ����ˮƽ�ʹ�ֱ����ʼ�Լ�����λ�� :
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
	/* ����LCD��ص�GPIO */
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
		/* ʹ��LTDC��DMA2Dʱ�� */
		__HAL_RCC_LTDC_CLK_ENABLE();
		__HAL_RCC_DMA2D_CLK_ENABLE();  
		
		/* ʹ��GPIOʱ�� */
		__HAL_RCC_GPIOI_CLK_ENABLE();
		__HAL_RCC_GPIOJ_CLK_ENABLE();
		__HAL_RCC_GPIOK_CLK_ENABLE();

		/* GPIOI ���� */
		GPIO_Init_Structure.Pin       = GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15; 
		GPIO_Init_Structure.Mode      = GPIO_MODE_AF_PP;
		GPIO_Init_Structure.Pull      = GPIO_NOPULL;
		GPIO_Init_Structure.Speed     = GPIO_SPEED_FREQ_HIGH;
		GPIO_Init_Structure.Alternate = GPIO_AF14_LTDC;  
		HAL_GPIO_Init(GPIOI, &GPIO_Init_Structure);

		/* GPIOJ ���� */  
		GPIO_Init_Structure.Pin       = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | \
									  GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | \
									  GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | \
									  GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15; 
		GPIO_Init_Structure.Mode      = GPIO_MODE_AF_PP;
		GPIO_Init_Structure.Pull      = GPIO_NOPULL;
		GPIO_Init_Structure.Speed     = GPIO_SPEED_FREQ_HIGH;
		GPIO_Init_Structure.Alternate = GPIO_AF14_LTDC;  
		HAL_GPIO_Init(GPIOJ, &GPIO_Init_Structure);  

		/* GPIOK ���� */  
		GPIO_Init_Structure.Pin       = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | \
									  GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7; 
		GPIO_Init_Structure.Mode      = GPIO_MODE_AF_PP;
		GPIO_Init_Structure.Pull      = GPIO_NOPULL;
		GPIO_Init_Structure.Speed     = GPIO_SPEED_FREQ_HIGH;
		GPIO_Init_Structure.Alternate = GPIO_AF14_LTDC;  
		HAL_GPIO_Init(GPIOK, &GPIO_Init_Structure);  	
	}
	
	/*##-2- LTDC��ʼ�� #############################################################*/  
	{	
		uint16_t Width, Height, HSYNC_W, HBP, HFP, VSYNC_W, VBP, VFP;
		RCC_PeriphCLKInitTypeDef  PeriphClkInitStruct;

		/* ֧��6����� */
		switch (g_LcdType)
		{
			case LCD_35_480X320:	/* 3.5�� 480 * 320 */	
				Width = 480;
				Height = 272;
				HSYNC_W = 10;
				HBP = 20;
				HFP = 20;
				VSYNC_W = 20;
				VBP = 20;
				VFP = 20;
				break;
			
			case LCD_43_480X272:		/* 4.3�� 480 * 272 */			
				Width = 480;
				Height = 272;

				HSYNC_W = 40;
				HBP = 2;
				HFP = 2;
				VSYNC_W = 9;
				VBP = 2;
				VFP = 2;
		
				/* LCD ʱ������ */
				/* PLL3_VCO Input = HSE_VALUE/PLL3M = 25MHz/5 = 5MHz */
				/* PLL3_VCO Output = PLL3_VCO Input * PLL3N = 5MHz * 48 = 240MHz */
				/* PLLLCDCLK = PLL3_VCO Output/PLL3R = 240 / 10 = 24MHz */
				/* LTDC clock frequency = PLLLCDCLK = 24MHz */
				/*
					ˢ���� = 24MHz /((Width + HSYNC_W  + HBP  + HFP)*(Height + VSYNC_W +  VBP  + VFP))
                   		   = 24000000/((480 + 40  + 2  + 2)*(272 + 9 +  2  + 2)) 
			               = 24000000/(524*285)
                           = 160Hz	

					��ǰ������÷����û�ʹ��PLL3Q�����48MHzʱ�ӹ�USBʹ�á�
			    */
				PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LTDC;
				PeriphClkInitStruct.PLL3.PLL3M = 5;
				PeriphClkInitStruct.PLL3.PLL3N = 48;
				PeriphClkInitStruct.PLL3.PLL3P = 2;
				PeriphClkInitStruct.PLL3.PLL3Q = 5;
				PeriphClkInitStruct.PLL3.PLL3R = 10;				
				HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);     			
				break;
			
			case LCD_50_480X272:		/* 5.0�� 480 * 272 */
				Width = 480;
				Height = 272;
			
				HSYNC_W = 40;
				HBP = 2;
				HFP = 2;
				VSYNC_W = 9;
				VBP = 2;
				VFP = 2;			
				break;
			
			case LCD_50_800X480:		/* 5.0�� 800 * 480 */
			case LCD_70_800X480:		/* 7.0�� 800 * 480 */					
				Width = 800;
				Height = 480;

				HSYNC_W = 96;	/* =10ʱ����ʾ��λ��20ʱ���������Ե�,80ʱȫ��OK */
				HBP = 10;
				HFP = 10;
				VSYNC_W = 2;
				VBP = 10;
				VFP = 10;			

				/* LCD ʱ������ */
				/* PLL3_VCO Input = HSE_VALUE/PLL3M = 25MHz/5 = 5MHz */
				/* PLL3_VCO Output = PLL3_VCO Input * PLL3N = 5MHz * 48 = 240MHz */
				/* PLLLCDCLK = PLL3_VCO Output/PLL3R = 240 / 10 = 24MHz */
				/* LTDC clock frequency = PLLLCDCLK = 24MHz */
				/*
					ˢ���� = 24MHz /((Width + HSYNC_W  + HBP  + HFP)*(Height + VSYNC_W +  VBP  + VFP))
                   		   = 24000000/((800 + 96  + 10  + 10)*(480 + 2 +  10  + 10)) 
			               = 24000000/(916*502)
                           = 52Hz	
			
					������Ҫ���ԼӴ�100Hzˢ������ȫû���⣬����PeriphClkInitStruct.PLL3.PLL3N = 100����
					��ʱ��LTDCʱ����50MHz
					ˢ���� = 50MHz /(��Width + HSYNC_W  + HBP  + HFP ��*(Height + VSYNC_W +  VBP  +VFP  )) 
					       = 5000000/(916*502) 
					       = 108.7Hz

					��ǰ������÷����û�ʹ��PLL3Q�����48MHzʱ�ӹ�USBʹ�á�
			    */ 
				PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LTDC;
				PeriphClkInitStruct.PLL3.PLL3M = 5;
				PeriphClkInitStruct.PLL3.PLL3N = 48;
				PeriphClkInitStruct.PLL3.PLL3P = 2;
				PeriphClkInitStruct.PLL3.PLL3Q = 5;
				PeriphClkInitStruct.PLL3.PLL3R = 10; 
				HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);     			
				break;
			
			case LCD_70_1024X600:		/* 7.0�� 1024 * 600 */
				/* ʵ������ʱ�� = 53.7M */
				Width = 1024;
				Height = 600;

				HSYNC_W = 2;	/* =10ʱ����ʾ��λ��20ʱ���������Ե�,80ʱȫ��OK */
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

				HSYNC_W = 80;	/* =10ʱ����ʾ��λ��20ʱ���������Ե�,80ʱȫ��OK */
				HBP = 10;
				HFP = 10;
				VSYNC_W = 10;
				VBP = 10;
				VFP = 10;		
			
				/* LCD ʱ������ */
				/* PLL3_VCO Input = HSE_VALUE/PLL3M = 25MHz/5 = 5MHz */
				/* PLL3_VCO Output = PLL3_VCO Input * PLL3N = 5MHz * 48 = 240MHz */
				/* PLLLCDCLK = PLL3_VCO Output/PLL3R = 240 / 10 = 24MHz */
				/* LTDC clock frequency = PLLLCDCLK = 24MHz */
				/*
					ˢ���� = 24MHz /((Width + HSYNC_W  + HBP  + HFP)*(Height + VSYNC_W +  VBP  + VFP))
                   		   = 24000000/((800 + 96  + 10  + 10)*(480 + 2 +  10  + 10)) 
			               = 24000000/(916*502)
                           = 52Hz

					������Ҫ���ԼӴ�100Hzˢ������ȫû���⣬����PeriphClkInitStruct.PLL3.PLL3N = 100����
					��ʱ��LTDCʱ����50MHz
					ˢ���� = 50MHz /(��Width + HSYNC_W  + HBP  + HFP ��*(Height + VSYNC_W +  VBP  +VFP  )) 
					       = 5000000/(916*502) 
					       = 108.7Hz

					��ǰ������÷����û�ʹ��PLL3Q�����48MHzʱ�ӹ�USBʹ�á�
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
		
		/* �����źż��� */	
		hltdc.Init.HSPolarity = LTDC_HSPOLARITY_AL;	/* HSYNC �͵�ƽ��Ч */
		hltdc.Init.VSPolarity = LTDC_VSPOLARITY_AL; 	/* VSYNC �͵�ƽ��Ч */
		hltdc.Init.DEPolarity = LTDC_DEPOLARITY_AL; 	/* DE �͵�ƽ��Ч */
		hltdc.Init.PCPolarity = LTDC_PCPOLARITY_IPC;

		/* ʱ������ */    
		hltdc.Init.HorizontalSync = (HSYNC_W - 1);
		hltdc.Init.VerticalSync = (VSYNC_W - 1);
		hltdc.Init.AccumulatedHBP = (HSYNC_W + HBP - 1);
		hltdc.Init.AccumulatedVBP = (VSYNC_W + VBP - 1);  
		hltdc.Init.AccumulatedActiveH = (Height + VSYNC_W + VBP - 1);
		hltdc.Init.AccumulatedActiveW = (Width + HSYNC_W + HBP - 1);
		hltdc.Init.TotalHeigh = (Height + VSYNC_W + VBP + VFP - 1);
		hltdc.Init.TotalWidth = (Width + HSYNC_W + HBP + HFP - 1); 

		/* ���ñ�������ɫ */
		hltdc.Init.Backcolor.Blue = 0;
		hltdc.Init.Backcolor.Green = 0;
		hltdc.Init.Backcolor.Red = 0;

		hltdc.Instance = LTDC;

		/* ����LTDC  */  
		if (HAL_LTDC_Init(&hltdc) != HAL_OK)
		{
			/* ��ʼ������ */
			Error_Handler(__FILE__, __LINE__);
		}
	}  

	/* ʹ�����ж� */
	HAL_LTDC_ProgramLineEvent(&hltdc, 0);
  
    /* ʹ��Dither */
    HAL_LTDC_EnableDither(&hltdc);

	/* ʹ��LTDC�жϣ������������ȼ� */
	HAL_NVIC_SetPriority(LTDC_IRQn, 0x2, 0);
	HAL_NVIC_EnableIRQ(LTDC_IRQn);
}

/*
*********************************************************************************************************
*	�� �� ��: _DMA_Copy
*	����˵��: ͨ��DMA2D��ǰ���㸴��ָ���������ɫ���ݵ�Ŀ������
*	��    ��: LayerIndex    ͼ��
*             pSrc          ��ɫ����Դ��ַ
*             pDst          ��ɫ����Ŀ�ĵ�ַ
*             xSize         Ҫ���������X���С����ÿ��������
*             ySize         Ҫ���������Y���С��������
*             OffLineSrc    ǰ����ͼ�����ƫ��
*             OffLineDst    �������ƫ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void _DMA_Copy(int LayerIndex, void * pSrc, void * pDst, int xSize, int ySize, int OffLineSrc, int OffLineDst) 
{
	U32 PixelFormat;

	/* ��ȡͼ��ʹ�õ���ɫ��ʽ */
	PixelFormat    = _GetPixelformat(LayerIndex);

	/* DMA2D���� */  
	DMA2D->CR      = 0x00000000UL | (1 << 9);
	DMA2D->FGMAR   = (U32)pSrc;
	DMA2D->OMAR    = (U32)pDst;
	DMA2D->FGOR    = OffLineSrc;
	DMA2D->OOR     = OffLineDst;
	DMA2D->FGPFCCR = PixelFormat;
	DMA2D->NLR     = (U32)(xSize << 16) | (U16)ySize;

	/* �������� */
	DMA2D->CR   |= DMA2D_CR_START;   

	/* �ȴ�DMA2D������� */
	while (DMA2D->CR & DMA2D_CR_START) {} 
}

/*
*********************************************************************************************************
*	�� �� ��: _DMA_CopyRGB565
*	����˵��: ͨ��DMA2D��ǰ���㸴��ָ���������ɫ���ݵ�Ŀ������
*	��    ��: pSrc          ��ɫ����Դ��ַ
*             pDst          ��ɫ����Ŀ�ĵ�ַ
*             xSize         Ҫ���������X���С����ÿ��������
*             ySize         Ҫ���������Y���С��������
*             OffLineSrc    ǰ����ͼ�����ƫ��
*             OffLineDst    �������ƫ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void _DMA_CopyRGB565(const void * pSrc, void * pDst, int xSize, int ySize, int OffLineSrc, int OffLineDst)
{
	/* DMA2D���� */  
	DMA2D->CR      = 0x00000000UL | (1 << 9);
	DMA2D->FGMAR   = (U32)pSrc;
	DMA2D->OMAR    = (U32)pDst;
	DMA2D->FGOR    = OffLineSrc;
	DMA2D->OOR     = OffLineDst;
	DMA2D->FGPFCCR = LTDC_PIXEL_FORMAT_RGB565;
	DMA2D->NLR     = (U32)(xSize << 16) | (U16)ySize;

	/* �������� */
	DMA2D->CR   |= DMA2D_CR_START;   

	/* �ȴ�DMA2D������� */
	while (DMA2D->CR & DMA2D_CR_START) {} 
}

/*
*********************************************************************************************************
*	�� �� ��: _DMA_Fill
*	����˵��: ͨ��DMA2D����ָ�����������ɫ���
*	��    ��: LayerIndex    ͼ��
*             pDst          ��ɫ����Ŀ�ĵ�ַ
*             xSize         Ҫ���������X���С����ÿ��������
*             ySize         Ҫ���������Y���С��������
*             OffLine       ǰ����ͼ�����ƫ��
*             ColorIndex    Ҫ������ɫֵ
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void _DMA_Fill(int LayerIndex, void * pDst, int xSize, int ySize, int OffLine, U32 ColorIndex) 
{
	U32 PixelFormat;

	/* ��ȡͼ��ʹ�õ���ɫ��ʽ */
	PixelFormat = _GetPixelformat(LayerIndex);

	/* DMA2D���� */  
	DMA2D->CR      = 0x00030000UL | (1 << 9);
	DMA2D->OCOLR   = ColorIndex;
	DMA2D->OMAR    = (U32)pDst;
	DMA2D->OOR     = OffLine;
	DMA2D->OPFCCR  = PixelFormat;
	DMA2D->NLR     = (U32)(xSize << 16) | (U16)ySize;

	/* �������� */
	DMA2D->CR   |= DMA2D_CR_START;   

	/* �ȴ�DMA2D������� */
	while (DMA2D->CR & DMA2D_CR_START) {} 
}

/*
*********************************************************************************************************
*	�� �� ��: _DMA_AlphaBlendingBulk
*	����˵��: ǰ����ͱ����������
*	��    ��: pColorFG    ǰ������ɫ���ݵ�ַ
*             pColorBG    ��������ɫ���ݵ�ַ
*             pColorDst   ���Ŀ������ַ
*             NumItems    ת������
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void _DMA_AlphaBlendingBulk(LCD_COLOR * pColorFG, LCD_COLOR * pColorBG, LCD_COLOR * pColorDst, U32 NumItems) 
{  
	/* DMA2D���� */   
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

	/* �������� */
	DMA2D->CR   |= DMA2D_CR_START;   

	/* �ȴ�DMA2D������� */
	while (DMA2D->CR & DMA2D_CR_START) {} 
}

/*
*********************************************************************************************************
*	�� �� ��: _DMA_MixColorsBulk
*	����˵��: ǰ����ͱ��������������Alphaͨ������
*	��    ��: pColorFG    ǰ������ɫ���ݵ�ַ
*             pColorBG    ��������ɫ���ݵ�ַ
*             pColorDst   ���Ŀ������ַ
*             Intens      Alphaͨ������
*             NumItems    ת������
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void _DMA_MixColorsBulk(LCD_COLOR * pColorFG, LCD_COLOR * pColorBG, LCD_COLOR * pColorDst, U8 Intens, U32 NumItems) 
{
	/* ����DMA2D */
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

	/* �������� */
	DMA2D->CR   |= DMA2D_CR_START;   

	/* �ȴ�DMA2D������� */
	while (DMA2D->CR & DMA2D_CR_START) {} 
}

/*
*********************************************************************************************************
*	�� �� ��: _DMA_ConvertColor
*	����˵��: ��ɫ��ʽת��
*	��    ��: pSrc             Դ���ݵ�ַ
*             pDst             Ŀ�����ݵ�ַ
*             PixelFormatSrc   Դ������ɫ��ʽ
*             PixelFormatDst   ת������ɫ��ʽ
*             NumItems         ת������
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void _DMA_ConvertColor(void * pSrc, void * pDst,  U32 PixelFormatSrc, U32 PixelFormatDst, U32 NumItems) 
{
	/* ����DMA2D */
	DMA2D->CR      = 0x00010000UL | (1 << 9);
	DMA2D->FGMAR   = (U32)pSrc;
	DMA2D->OMAR    = (U32)pDst;
	DMA2D->FGOR    = 0;
	DMA2D->OOR     = 0;
	DMA2D->FGPFCCR = PixelFormatSrc;
	DMA2D->OPFCCR  = PixelFormatDst;
	DMA2D->NLR     = (U32)(NumItems << 16) | 1;

	/* �������� */
	DMA2D->CR   |= DMA2D_CR_START;   

	/* �ȴ�DMA2D������� */
	while (DMA2D->CR & DMA2D_CR_START) {} 
}

/*
*********************************************************************************************************
*	�� �� ��: _DMA_DrawBitmapL8
*	����˵��: ����L8��ʽλͼ
*	��    ��: pSrc             Դ���ݵ�ַ
*             pDst             Ŀ�����ݵ�ַ
*             OffSrc           Դ������ƫ��
*             OffDst           Ŀ��������ƫ��
*             PixelFormatDst   ת������ɫ��ʽ
*             xSize            λͼ��
*             ySize            λͼ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void _DMA_DrawBitmapL8(void * pSrc, void * pDst,  U32 OffSrc, U32 OffDst, U32 PixelFormatDst, U32 xSize, U32 ySize) 
{
	/* ����DMA2D */
	DMA2D->CR      = 0x00010000UL | (1 << 9);
	DMA2D->FGMAR   = (U32)pSrc;
	DMA2D->OMAR    = (U32)pDst;
	DMA2D->FGOR    = OffSrc;
	DMA2D->OOR     = OffDst;
	DMA2D->FGPFCCR = LTDC_PIXEL_FORMAT_L8;
	DMA2D->OPFCCR  = PixelFormatDst;
	DMA2D->NLR     = (U32)(xSize << 16) | ySize;

	/* �������� */
	DMA2D->CR   |= DMA2D_CR_START;   

	/* �ȴ�DMA2D������� */
	while (DMA2D->CR & DMA2D_CR_START) {}  
}

#if DrawBitmapA4Enalbe == 1
/*
*********************************************************************************************************
*	�� �� ��: _DMA_DrawBitmapA4
*	����˵��: ����A4��ʽλͼ
*	��    ��: pSrc             Դ���ݵ�ַ
*             pDst             Ŀ�����ݵ�ַ
*             OffSrc           Դ������ƫ��
*             OffDst           Ŀ��������ƫ��
*             PixelFormatDst   ת������ɫ��ʽ
*             xSize            λͼ��
*             ySize            λͼ��
*	�� �� ֵ: 0
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

	/* ����DMA2D */
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

	/* �������� */
	DMA2D->CR   |= DMA2D_CR_START;   

	/* �ȴ�DMA2D������� */
	while (DMA2D->CR & DMA2D_CR_START) {} 

	return 0;
}
#endif

/*
*********************************************************************************************************
*	�� �� ��: _DMA_DrawAlphaBitmap
*	����˵��: ���ƴ�͸��ͨ����λͼ
*	��    ��: pSrc             Դ���ݵ�ַ
*             pDst             Ŀ�����ݵ�ַ
*             xSize            λͼ��
*             ySize            λͼ��
*             OffLineSrc       Դ������ƫ��
*             OffLineDst       Ŀ��������ƫ��
*             PixelFormatDst   ת������ɫ��ʽ
*	�� �� ֵ: 0
*********************************************************************************************************
*/
static void _DMA_DrawAlphaBitmap(void * pDst, const void * pSrc, int xSize, int ySize, int OffLineSrc, int OffLineDst, int PixelFormat) 
{
	/* ����*/ 
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

	/* �������� */
	DMA2D->CR   |= DMA2D_CR_START;   

	/* �ȴ�DMA2D������� */
	while (DMA2D->CR & DMA2D_CR_START) {} 
}

/*
*********************************************************************************************************
*	�� �� ��: _DMA_LoadLUT
*	����˵��: ������ɫ��
*	��    ��: pColor     ��ɫ���ַ
*             NumItems   Ҫ���صĸ���
*	�� �� ֵ: 0
*********************************************************************************************************
*/
static void _DMA_LoadLUT(LCD_COLOR * pColor, U32 NumItems)
{
	/* ����DMA2D */
	DMA2D->FGCMAR  = (U32)pColor;
	DMA2D->FGPFCCR  = LTDC_PIXEL_FORMAT_RGB888
				  | ((NumItems - 1) & 0xFF) << 8;
	
	/* �������� */
	DMA2D->FGPFCCR |= (1 << 5);
}

/*
*********************************************************************************************************
*	�� �� ��: _DMA_AlphaBlending
*	����˵��: ǰ����ͱ����������
*	��    ��: pColorFG    ǰ������ɫ���ݵ�ַ
*             pColorBG    ��������ɫ���ݵ�ַ
*             pColorDst   ���Ŀ������ַ
*             NumItems    ת������
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void _DMA_AlphaBlending(LCD_COLOR * pColorFG, LCD_COLOR * pColorBG, LCD_COLOR * pColorDst, U32 NumItems) 
{
	_DMA_AlphaBlendingBulk(pColorFG, pColorBG, pColorDst, NumItems);
}

/*
*********************************************************************************************************
*	�� �� ��: _DMA_Index2ColorBulk
*	����˵��: ͨ��DMA2D������ǰ��ʾ������ɫ����ת��ΪemWin��32λARGB��ɫ���ݡ�
*	��    ��: pIndex       ��ʾ����ɫ��ַ
*             pColor       ת����������emWin����ɫ��ַ
*             NumItems     ת������ɫ����
*             SizeOfIndex  δʹ��
*             PixelFormat  ��ʾ����ǰʹ�õ���ɫ��ʽ
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void _DMA_Index2ColorBulk(void * pIndex, LCD_COLOR * pColor, U32 NumItems, U8 SizeOfIndex, U32 PixelFormat) 
{
	_DMA_ConvertColor(pIndex, pColor, PixelFormat, LTDC_PIXEL_FORMAT_ARGB8888, NumItems);
}

/*
*********************************************************************************************************
*	�� �� ��: _DMA_Color2IndexBulk
*	����˵��: ͨ��DMA2D����emWin��32λARGB��ɫ����ת��Ϊ�����ڵ�ǰ��ʾ������ɫ����
*	��    ��: pIndex       ��ʾ����ɫ��ַ
*             pColor       ת����������emWin����ɫ��ַ
*             NumItems     ת������ɫ����
*             SizeOfIndex  δʹ��
*             PixelFormat  ��ʾ����ǰʹ�õ���ɫ��ʽ
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void _DMA_Color2IndexBulk(LCD_COLOR * pColor, void * pIndex, U32 NumItems, U8 SizeOfIndex, U32 PixelFormat)
{
	_DMA_ConvertColor(pColor, pIndex, LTDC_PIXEL_FORMAT_ARGB8888, PixelFormat, NumItems);
}

/*
*********************************************************************************************************
*	�� �� ��: _DMA_MixColorsBulk
*	����˵��: ��һ����ʾ����ǰ��ɫ�ͱ���ɫ���л��
*	��    ��: pFG   ǰ��ɫ��ַ
*             pBG   ����ɫ��ַ
*             pDst  ��Ϻ���ɫ�洢�ĵ�ַ
*             OffFG    ǰ��ɫƫ�Ƶ�ַ
*             OffBG    ����ɫƫ�Ƶ�ַ
*             OffDest  ��Ϻ�ƫ�Ƶ�ַ
*             xSize    ��ʾ��x���С
*             ySize    ��ʾ��y���С
*             Intens   ��alphaֵ
*	�� �� ֵ: ��
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
*	�� �� ��: _GetBufferSize
*	����˵��: ��ȡָ�����Դ��С
*	��    ��: LayerIndex    ͼ��
*	�� �� ֵ: �Դ��С
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
*	�� �� ��: _LCD_CopyBuffer
*	����˵��: �˺������ڶ໺�壬��һ�������е��������ݸ��Ƶ���һ�����塣
*	��    ��: LayerIndex    ͼ��
*             IndexSrc      Դ�������
*             IndexDst      Ŀ�껺�����
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void _LCD_CopyBuffer(int LayerIndex, int IndexSrc, int IndexDst) 
{
	U32 BufferSize, AddrSrc, AddrDst;

	BufferSize = _GetBufferSize(LayerIndex);
	AddrSrc    = _aAddr[LayerIndex] + BufferSize * IndexSrc;
	AddrDst    = _aAddr[LayerIndex] + BufferSize * IndexDst;
	_DMA_Copy(LayerIndex, (void *)AddrSrc, (void *)AddrDst, _axSize[LayerIndex], _aySize[LayerIndex], 0, 0);
	
	/* ���Ʋ����л�������Buffer[IndexDst] */
	_aBufferIndex[LayerIndex] = IndexDst;
}

/*
*********************************************************************************************************
*	�� �� ��: _LCD_CopyRect
*	����˵��: �˺������ڶ໺�壬��һ��������ָ���������ݸ��Ƶ���һ�����塣
*	��    ��: LayerIndex    ͼ��
*             x0            Դ����x��λ��
*             y0            Դ����y��λ��
*             x1            Ŀ���x��λ��
*             y1            Ŀ���y��λ��
*             xSize         Ҫ���Ƶ�x���С
*             ySize         Ҫ���Ƶ�y���С
*	�� �� ֵ: ��
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
*	�� �� ��: _LCD_FillRect
*	����˵��: ��ָ�������������ɫ���
*	��    ��: LayerIndex    ͼ��
*             x0            ��ʼx��λ��
*             y0            ��ʼy��λ��
*             x1            ����x��λ��
*             y1            ����y��λ��
*             PixelIndex    Ҫ������ɫֵ
*	�� �� ֵ: ��
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
*	�� �� ��: _LCD_DrawBitmap32bpp
*	����˵��: ����32bpp��ʽλͼ
*	��    ��: LayerIndex      ͼ��   
*             x               X����ʼλ��
*             y               Y����ʼλ��
*             p               Դ���ݵ�ַ
*             xSize           λͼ��
*             ySize           λͼ��
*             BytesPerLine    ÿ���ֽ���
*	�� �� ֵ: ��
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
*	�� �� ��: _LCD_DrawBitmap16bpp
*	����˵��: ����16bpp��ʽλͼ
*	��    ��: LayerIndex      ͼ��   
*             x               X����ʼλ��
*             y               Y����ʼλ��
*             p               Դ���ݵ�ַ
*             xSize           λͼ��
*             ySize           λͼ��
*             BytesPerLine    ÿ���ֽ���
*	�� �� ֵ: ��
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
*	�� �� ��: _LCD_DrawBitmap8bpp
*	����˵��: ����8bpp��ʽλͼ
*	��    ��: LayerIndex      ͼ��   
*             x               X����ʼλ��
*             y               Y����ʼλ��
*             p               Դ���ݵ�ַ
*             xSize           λͼ��
*             ySize           λͼ��
*             BytesPerLine    ÿ���ֽ���
*	�� �� ֵ: ��
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
*	�� �� ��: _LCD_DrawBitmap4bpp
*	����˵��: ����4bpp��ʽλͼ
*	��    ��: LayerIndex      ͼ��   
*             x               X����ʼλ��
*             y               Y����ʼλ��
*             p               Դ���ݵ�ַ
*             xSize           λͼ��
*             ySize           λͼ��
*             BytesPerLine    ÿ���ֽ���
*	�� �� ֵ: ��
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
*	�� �� ��: _LCD_DrawMemdev16bpp
*	����˵��: ����16bpp�洢�豸
*	��    ��: pDst               Դ���ݵ�ַ   
*             pSrc               Ŀ�����ݵ�ַ
*             xSize              �洢�豸��
*             ySize              �洢�豸��
*             BytesPerLineDst    Դ����ÿ���ֽ���
*             BytesPerLineSrc    Ŀ������ÿ���ֽ���
*	�� �� ֵ: ��
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
*	�� �� ��: _LCD_DrawMemdevAlpha
*	����˵��: ���ƴ�Alphaͨ���Ĵ洢�豸
*	��    ��: pDst               Դ���ݵ�ַ   
*             pSrc               Ŀ�����ݵ�ַ
*             xSize              �洢�豸��
*             ySize              �洢�豸��
*             BytesPerLineDst    Դ����ÿ���ֽ���
*             BytesPerLineSrc    Ŀ������ÿ���ֽ���
*	�� �� ֵ: ��
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
*	�� �� ��: _LCD_DrawBitmapAlpha
*	����˵��: ���ƴ�Alphaͨ����λͼ
*	��    ��: LayerIndex      ͼ��   
*             x               X����ʼλ��
*             y               Y����ʼλ��
*             p               Դ���ݵ�ַ
*             xSize           λͼ��
*             ySize           λͼ��
*             BytesPerLine    ÿ���ֽ���
*	�� �� ֵ: ��
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
*	�� �� ��: _LCD_GetpPalConvTable
*	����˵��: ת����ɫ������Ӧ���������õ���ɫ��ʽ��
*	��    ��: pLogPal   Դ��ɫ���ַ
*             pBitmap   λͼ��ַ
*             LayerIndex  Դ��ɫ��ʽ
*	�� �� ֵ: ת�������ɫ���ַ
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

	/* Ĭ����ɫ����� */
	if (DoDefault) 
	{
		/* ��������ֵ */
		return LCD_GetpPalConvTable(pLogPal);
	}

	/* DMA2D����LUT */
	_DMA_LoadLUT((U32 *)pLogPal->pPalEntries, pLogPal->NumEntries);

	/* ���ط�NULL */
	return _pBuffer_DMA2D;
}

/*
*********************************************************************************************************
*	�� �� ��: LCD_X_DisplayDriver
*	����˵��: ��ʾ��������
*	��    ��: LayerIndex   ͼ��
*             Cmd          ����
*             pData        ���ݵ�ַ
*	�� �� ֵ: �ɹ�����0��ʧ�ܷ���-1
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
			/* ������ʼ�� */
			LCD_LL_LayerInit(LayerIndex);
			break;
		}
		
		case LCD_X_SETORG: 
		{
			/* �����Դ��ַ */
			LCD_X_SETORG_INFO * p;

			p = (LCD_X_SETORG_INFO *)pData;
			addr = _aAddr[LayerIndex] + p->yPos * _axSize[LayerIndex] * _aBytesPerPixels[LayerIndex];
			HAL_LTDC_SetAddress(&hltdc, addr, LayerIndex);
			break;
		}
		
		case LCD_X_SHOWBUFFER: 
		{
			/* ���ڶ໺�壬����Index����ָʾʹ�õ��Ǹ����� */
			LCD_X_SHOWBUFFER_INFO * p;

			p = (LCD_X_SHOWBUFFER_INFO *)pData;
			_aPendingBuffer[LayerIndex] = p->Index;
			break;
		}
		
		case LCD_X_SETLUTENTRY: 
		{
			/* ��ɫ���ұ����� */
			LCD_X_SETLUTENTRY_INFO * p;

			p = (LCD_X_SETLUTENTRY_INFO *)pData;
			HAL_LTDC_ConfigCLUT(&hltdc, (uint32_t*)p->Color, p->Pos, LayerIndex);
			break;
		}
		
		case LCD_X_ON: 
		{
			/* ʹ��LTDC  */
			__HAL_LTDC_ENABLE(&hltdc);
			break;
		}
		
		case LCD_X_OFF: 
		{
			/* �ر�LTDC */
			__HAL_LTDC_DISABLE(&hltdc);
			break;
		}
		
		case LCD_X_SETVIS: 
		{
			/* ͼ��Ŀ��� */
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
			/* ����ͼ����ʾλ�� */
			LCD_X_SETPOS_INFO * p;

			p = (LCD_X_SETPOS_INFO *)pData;    
			HAL_LTDC_SetWindowPosition(&hltdc, p->xPos, p->yPos, LayerIndex);
			break;
		}
		
		case LCD_X_SETSIZE:
		{
			/* ����ͼ���С */
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
			/* ����͸�� */
			LCD_X_SETALPHA_INFO * p;

			p = (LCD_X_SETALPHA_INFO *)pData;
			HAL_LTDC_SetAlpha(&hltdc, p->Alpha, LayerIndex);
			break;
		}
		
		case LCD_X_SETCHROMAMODE: 
		{
			/* ʹ�ܻ��ֹ��ɫ */
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
			/* ���ÿ�ɫ */
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
*	�� �� ��: LCD_X_DisplayDriver
*	����˵��: ��ʼ������
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void LCD_X_Config(void) 
{
	int i;
	U32 PixelFormat;

	/* �ײ��ʼ�� */
	LCD_LL_Init ();

#if ClearCacheHookEnalbe == 1
	GUI_DCACHE_SetClearCacheHook(_ClearCacheHook);
#endif

	/* ��ʼ���໺�� */
#if (NUM_BUFFERS > 1)
	for (i = 0; i < GUI_NUM_LAYERS; i++) 
	{
		GUI_MULTIBUF_ConfigEx(i, NUM_BUFFERS);
	}
#endif

	/* ������ʾ��������ɫ��ʽת�� */
	GUI_DEVICE_CreateAndLink(DISPLAY_DRIVER_0, COLOR_CONVERSION_0, 0, 0);

	/* ����ͼ��1 */
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
	/* ������ʾ��������ɫ��ʽת�� */
	GUI_DEVICE_CreateAndLink(DISPLAY_DRIVER_1, COLOR_CONVERSION_1, 0, 1);

	/* ����ͼ��2 */
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
	
	/* ����RAM��ַ��ÿ�����ص��ֽ��� */
	for (i = 0; i < GUI_NUM_LAYERS; i++) 
	{
		/* ����RAM��ַ */
		LCD_SetVRAMAddrEx(i, (void *)(_aAddr[i]));
		
		/* ÿ�����ص��ֽ��� */
		_aBytesPerPixels[i] = LCD_GetBitsPerPixelEx(i) >> 3;
	}
	
	/* ���ú����ض��� */
	for (i = 0; i < GUI_NUM_LAYERS; i++) 
	{
		PixelFormat = _GetPixelformat(i);
		
		/* CopyBuffer�ض��򣬶໺��ʹ�� */
		LCD_SetDevFunc(i, LCD_DEVFUNC_COPYBUFFER, (void(*)(void))_LCD_CopyBuffer);

		if (PixelFormat <= LTDC_PIXEL_FORMAT_ARGB4444) 
		{
			/* ���ʹ�� */
			LCD_SetDevFunc(i, LCD_DEVFUNC_FILLRECT, (void(*)(void))_LCD_FillRect);
		}

		if (_aOrientation[i] == ROTATION_0)
		{
			/* CopyRect�ض��� */
			LCD_SetDevFunc(i, LCD_DEVFUNC_COPYRECT, (void(*)(void))_LCD_CopyRect);

			/* 8bpp�ض��� */
			if (PixelFormat <= LTDC_PIXEL_FORMAT_ARGB4444) 
			{
				LCD_SetDevFunc(i, LCD_DEVFUNC_DRAWBMP_8BPP, (void(*)(void))_LCD_DrawBitmap8bpp);
			}

			/* 16bpp�ض��� */
			if (PixelFormat == LTDC_PIXEL_FORMAT_RGB565) 
			{
				LCD_SetDevFunc(i, LCD_DEVFUNC_DRAWBMP_16BPP, (void(*)(void))_LCD_DrawBitmap16bpp);
			}

			/* 32bpp�ض��� */
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

	/* Alpha����ض��� */
	GUI_SetFuncAlphaBlending(_DMA_AlphaBlending);

	GUI_SetFuncGetpPalConvTable(_LCD_GetpPalConvTable);

	/* ��ɫ����ض��� */
	GUI_SetFuncMixColorsBulk(_LCD_MixColorsBulk);

#if DrawBitmapA4Enalbe == 1
	GUI_AA_SetpfDrawCharAA4(_LCD_DrawBitmap4bpp); /* ������������⣬δʹ�� */
#endif

	/* 16bpp�洢�豸�ض��� */
	GUI_MEMDEV_SetDrawMemdev16bppFunc(_LCD_DrawMemdev16bpp);

	/* Alpha�����ض��� */
	GUI_SetFuncDrawAlpha(_LCD_DrawMemdevAlpha, _LCD_DrawBitmapAlpha);
}

#if 1
/*
*********************************************************************************************************
*	�� �� ��: LTDC_IRQHandler
*	����˵��: ��ʼ������
*	��    ��: ��
*	�� �� ֵ: ��
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
			/* ���㻺���ַ */
			Addr = _aAddr[i] + _axSize[i] * _aySize[i] * _aPendingBuffer[i] * _aBytesPerPixels[i];

			/* ����LTDC�Ĵ��� */	  
			__HAL_LTDC_LAYER(&hltdc, i)->CFBAR = Addr;     
			__HAL_LTDC_RELOAD_IMMEDIATE_CONFIG(&hltdc);   

			/* ����emWinʹ�õĻ��� */
			GUI_MULTIBUF_ConfirmEx(i, _aPendingBuffer[i]);

			/* �����־ */
			_aPendingBuffer[i] = -1;
		}
	}
}

#else
/*
*********************************************************************************************************
*	�� �� ��: LTDC_IRQHandler
*	����˵��: ��ʼ������
*	��    ��: ��
*	�� �� ֵ: ��
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
			/* ���㻺���ַ */
			Addr = _aAddr[i] + _axSize[i] * _aySize[i] * _aPendingBuffer[i] * _aBytesPerPixels[i];

			/* ����LTDC�Ĵ��� */	  
			__HAL_LTDC_LAYER(hltdc, i)->CFBAR = Addr;     
			__HAL_LTDC_RELOAD_IMMEDIATE_CONFIG(hltdc);   

			/* ����emWinʹ�õĻ��� */
			GUI_MULTIBUF_ConfirmEx(i, _aPendingBuffer[i]);

			/* �����־ */
			_aPendingBuffer[i] = -1;
		}
	}

	/* ���¿������жϣ���Ϊ����HAL_LTDC_IRQHandler�����ر� */
	HAL_LTDC_ProgramLineEvent(hltdc, 0);
}

#endif

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
