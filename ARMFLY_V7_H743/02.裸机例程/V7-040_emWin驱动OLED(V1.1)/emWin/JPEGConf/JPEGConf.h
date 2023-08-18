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

#ifndef JPEGCONF_H
#define JPEGCONF_H

#define JPEG_ARGB8888 0
#define JPEG_RGB565   1

typedef struct {
  U32                 WorkBufferSize;
  void              * (* pfMalloc)(size_t size);
  void                (* pfFree)  (void * ptr);
  int                 ColorFormat;
  U8                * pWorkBuffer;
  GUI_HMEM            hWorkBuffer;
  U8                * pOutBuffer;
  GUI_HMEM            hOutBuffer;
  U32                 OutBufferSize;
  U8                * pInBuffer;
  U32                 NumBytesInBuffer;
  int                 BitsPerPixel;
  int                 BytesPerLine;
  GUI_GET_DATA_FUNC * pfGetData;
  void              * pVoid;
  U32                 Off;
  U32                 xPos;
  U32                 yPos;
  U32                 xSize;
  U32                 ySize;
  U32                 BlockSize;
  U32                 TotalMCUs;
  U32                 IndexMCU;
  U32                 (* pfConvert)(U32 BlockIndex);
  //U8                  HFactor;
  U8                  VFactor;
  U32                 xSizeExtended;
  U8                  Error;
  U8                  IRQFlag;
#if GUI_SUPPORT_MEMDEV
  GUI_MEMDEV_Handle   hMem;
#endif
} JPEG_X_CONTEXT;

int  JPEG_X_Draw        (GUI_GET_DATA_FUNC * pfGetData, void * p, int x0, int y0);
void JPEG_X_DeInit      (void);
void JPEG_X_Init(JPEG_X_CONTEXT * pContext);
void JPEG_X_SetStayAlive(int OnOff);
void JPEG_X_IncStayAlive(void);
void JPEG_X_DecStayAlive(void);

void MOVIE_X_cbNotify    (GUI_MOVIE_HANDLE hMovie, int Notification, U32 CurrentFrame);

#endif 

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
