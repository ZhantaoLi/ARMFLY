/*
*********************************************************************************************************
*	                                  
*	模块名称 : GUI界面主函数
*	文件名称 : MainTask.c
*	版    本 : V1.0
*	说    明 : 主界面
*              
*	修改记录 :
*		版本号   日期         作者          说明
*		V1.0    2019-07-09   Eric2013  	    首版    
*                                     
*	Copyright (C), 2018-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/
#include "bsp.h"
#include "MainTask.h"



#include "bsp.h"
#include "MainTask.h"


/*
*********************************************************************************************************
*	                                    变量
*********************************************************************************************************
*/
static int Count = 0;


/*
*********************************************************************************************************
*	                       GUI_WIDGET_CREATE_INFO类型数组
*********************************************************************************************************
*/
static const GUI_WIDGET_CREATE_INFO _aDialogCreate[] = {
    { FRAMEWIN_CreateIndirect,  "armfly",   0,               0,  0, 128,64,0,0},
	{ TEXT_CreateIndirect,      "0000",     GUI_ID_TEXT0,    0,0,128,48, 0,0}
};


/*
*********************************************************************************************************
*	函 数 名: _cbCallback
*	功能说明: 对话框回调函数		
*	形    参: pMsg  回调参数 
*	返 回 值: 无
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
			//初始化框架窗口
			//
			FRAMEWIN_SetFont(hWin,&GUI_Font16B_ASCII);
			FRAMEWIN_SetTextAlign(hWin,GUI_TA_VCENTER|GUI_TA_CENTER);
			FRAMEWIN_SetTitleHeight(hWin,20);

			//
			//初始化文本控件
			//
			TEXT_SetFont(WM_GetDialogItem(hWin,GUI_ID_TEXT0), &GUI_FontD32);
			TEXT_SetTextColor(WM_GetDialogItem(hWin,GUI_ID_TEXT0), GUI_BLACK);
            break;

		/* 定时器消息 */
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
*  函 数 名: _cbTimer
*  功能说明: 定时器回调函数
*  形    参: pTM 消息指针
*  返 回 值: 无
*********************************************************************************************************
*/
void _cbTimer(GUI_TIMER_MESSAGE * pTM)
{
	switch (pTM->Context)
	{
		case 0x01:
			/* 100ms刷新一次OLED */
			OLED_EndDraw();
			/* 此函数一定要调用，设置重新启动，要不仅执行一次 */
			GUI_TIMER_Restart(pTM->hTimer);
			break;
			
		default:
			break;
	}
}

/*
*********************************************************************************************************
*	函 数 名: CreateOLEDDlg
*	功能说明: 创建OLED的界面
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void CreateOLEDDlg(void) 
{
    WM_HWIN hDlg;
	WM_HTIMER hTimer;
	
	/* 创建一个对话框 */
    hDlg = GUI_CreateDialogBox(_aDialogCreate, GUI_COUNTOF(_aDialogCreate), &_cbCallback, 0, 0, 0);

	/* 给对话框hDlg创建定时器，溢出时间是100ms */
	hTimer = WM_CreateTimer(WM_GetClientWindow(hDlg), 0, 100, 0);
	
	/* 未用到，防止警告 */
	(void)hTimer;
	
	/* 创建定时器 */
    hTimer = GUI_TIMER_Create(_cbTimer, /* 回调函数 */
						        1,       /* 绝对时间，设置系统上电后 1ms 作为溢出时间 */
						        0x01,   /* 可以认为此参数是区分不同定时器的 ID，方便多个定时使用同一个回调函数 */
						        0);     /* 保留，暂时未用到 */
	
	/* 设置定时器周期为 1ms */
	GUI_TIMER_SetPeriod(hTimer, 100);
}

/*
*********************************************************************************************************
*	函 数 名: MainTask
*	功能说明: GUI主函数
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void MainTask(void) 
{
	/* 初始化 */
	GUI_Init();
	
	CreateOLEDDlg();

	while(1) 
	{
		GUI_Delay(100);
	}
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
