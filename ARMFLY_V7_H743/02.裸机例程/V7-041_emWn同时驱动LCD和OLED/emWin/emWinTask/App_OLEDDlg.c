/*
*********************************************************************************************************
*	                                  
*	ģ������ : OLED����
*	�ļ����� : MainTask.c
*	��    �� : V1.0
*	˵    �� : OLED����
*              
*	�޸ļ�¼ :
*		�汾��   ����         ����          ˵��
*		V1.0    2019-07-10   Eric2013  	    �װ�    
*                                     
*	Copyright (C), 2018-2030, ���������� www.armfly.com
*
*********************************************************************************************************
*/
#include "bsp.h"
#include "MainTask.h"


/*
*********************************************************************************************************
*	                                    ����
*********************************************************************************************************
*/
static int Count = 0;


/*
*********************************************************************************************************
*	                       GUI_WIDGET_CREATE_INFO��������
*********************************************************************************************************
*/
static const GUI_WIDGET_CREATE_INFO _aDialogCreate[] = {
    { FRAMEWIN_CreateIndirect,  "armfly",   0,               0,  0, 128,64,0,0},
	{ TEXT_CreateIndirect,      "0000",     GUI_ID_TEXT0,    0,0,128,48, 0,0}
};


/*
*********************************************************************************************************
*	�� �� ��: _cbCallback
*	����˵��: �Ի���ص�����		
*	��    ��: pMsg  �ص����� 
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void _cbCallback(WM_MESSAGE * pMsg) 
{
    int NCode, Id;
	char buf[10];
    WM_HWIN hWin = pMsg->hWin;

    switch (pMsg->MsgId) 
    {
        case WM_INIT_DIALOG:

			//
			//��ʼ����ܴ���
			//
			FRAMEWIN_SetFont(hWin,&GUI_Font16B_ASCII);
			FRAMEWIN_SetTextAlign(hWin,GUI_TA_VCENTER|GUI_TA_CENTER);
			FRAMEWIN_SetTitleHeight(hWin,20);

			//
			//��ʼ���ı��ؼ�
			//
			TEXT_SetFont(WM_GetDialogItem(hWin,GUI_ID_TEXT0), &GUI_FontD32);
			TEXT_SetTextColor(WM_GetDialogItem(hWin,GUI_ID_TEXT0), GUI_BLACK);
            break;

		/* ��ʱ����Ϣ */
		case WM_TIMER:
            Count++;
			sprintf(buf, "%04d", Count);
			TEXT_SetText(WM_GetDialogItem(hWin,GUI_ID_TEXT0), (const char *)buf);
			WM_RestartTimer(pMsg->Data.v, 100);
            break;
			
        case WM_KEY:
            switch (((WM_KEY_INFO*)(pMsg->Data.p))->Key) 
            {
                case GUI_KEY_ESCAPE:
                    GUI_EndDialog(hWin, 1);
                    break;
                case GUI_KEY_ENTER:
                    GUI_EndDialog(hWin, 0);
                    break;
            }
            break;
			
        case WM_NOTIFY_PARENT:
            Id = WM_GetId(pMsg->hWinSrc); 
            NCode = pMsg->Data.v;        
            switch (Id) 
            {
                case GUI_ID_OK:
                    if(NCode==WM_NOTIFICATION_RELEASED)
                        GUI_EndDialog(hWin, 0);
                    break;
                case GUI_ID_CANCEL:
                    if(NCode==WM_NOTIFICATION_RELEASED)
                        GUI_EndDialog(hWin, 0);
                    break;
            }
            break;
			
        default:
            WM_DefaultProc(pMsg);
    }
}

/*
*********************************************************************************************************
*  �� �� ��: _cbTimer
*  ����˵��: ��ʱ���ص�����
*  ��    ��: pTM ��Ϣָ��
*  �� �� ֵ: ��
*********************************************************************************************************
*/
void _cbTimer(GUI_TIMER_MESSAGE * pTM)
{
	switch (pTM->Context)
	{
		case 0x01:
			/* 100msˢ��һ��OLED */
			OLED_EndDraw();
			/* �˺���һ��Ҫ���ã���������������Ҫ����ִ��һ�� */
			GUI_TIMER_Restart(pTM->hTimer);
			break;
			
		default:
			break;
	}
}

/*
*********************************************************************************************************
*	�� �� ��: CreateOLEDDlg
*	����˵��: ����OLED�Ľ���
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void CreateOLEDDlg(void) 
{
    WM_HWIN hDlg;
	WM_HTIMER hTimer;
	
	/* ����һ���Ի��� */
    hDlg = GUI_CreateDialogBox(_aDialogCreate, GUI_COUNTOF(_aDialogCreate), &_cbCallback, 0, 0, 0);

	/* ���Ի���hDlg������ʱ�������ʱ����100ms */
	hTimer = WM_CreateTimer(WM_GetClientWindow(hDlg), 0, 100, 0);
	
	/* δ�õ�����ֹ���� */
	(void)hTimer;
	
	/* ������ʱ�� */
    hTimer = GUI_TIMER_Create(_cbTimer, /* �ص����� */
						        1,       /* ����ʱ�䣬����ϵͳ�ϵ�� 1ms ��Ϊ���ʱ�� */
						        0x01,   /* ������Ϊ�˲��������ֲ�ͬ��ʱ���� ID����������ʱʹ��ͬһ���ص����� */
						        0);     /* ��������ʱδ�õ� */
	
	/* ���ö�ʱ������Ϊ 1ms */
	GUI_TIMER_SetPeriod(hTimer, 100);
}

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
