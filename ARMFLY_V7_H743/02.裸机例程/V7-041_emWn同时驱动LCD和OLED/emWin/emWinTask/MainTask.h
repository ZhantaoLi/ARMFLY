/*
*********************************************************************************************************
*	                                  
*	ģ������ : GUI����������
*	�ļ����� : MainTask.c
*	��    �� : V1.0
*	˵    �� : GUI����������
*		�汾��   ����         ����            ˵��
*		v1.0    2019-05-23  Eric2013  	      �װ�
*
*	Copyright (C), 2019-2030, ���������� www.armfly.com
*
*********************************************************************************************************
*/

#ifndef __MainTask_H
#define __MainTask_H

#include "GUI.h"
#include "DIALOG.h"
#include "WM.h"
#include "BUTTON.h"
#include "CHECKBOX.h"
#include "DROPDOWN.h"
#include "EDIT.h"
#include "FRAMEWIN.h"
#include "LISTBOX.h"
#include "MULTIEDIT.h"
#include "RADIO.h"
#include "SLIDER.h"
#include "TEXT.h"
#include "PROGBAR.h"
#include "SCROLLBAR.h"
#include "LISTVIEW.h"
#include "GRAPH.h"
#include "MENU.h"
#include "MULTIPAGE.h"
#include "ICONVIEW.h"
#include "TREEVIEW.h"


#include "JPEGConf.h"
#include "decode_dma.h"

/*
************************************************************************
*						���ⲿ�ļ�����
************************************************************************
*/
extern void MainTask(void);
extern void TOUCH_Calibration(uint8_t _PointCount);
extern const unsigned char _ac1[87294UL + 1];

#endif

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
