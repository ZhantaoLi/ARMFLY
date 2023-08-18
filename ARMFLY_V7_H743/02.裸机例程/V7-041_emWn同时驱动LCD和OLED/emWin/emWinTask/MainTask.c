/*
*********************************************************************************************************
*	                                  
*	模块名称 : GUI界面主函数
*	文件名称 : MainTask.c
*	版    本 : V1.0
*	说    明 : LCD界面
*              
*	修改记录 :
*		版本号   日期         作者          说明
*		V1.0    2019-07-10   Eric2013  	    首版    
*                                     
*	Copyright (C), 2018-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/
#include "bsp.h"
#include "MainTask.h"


/*
*********************************************************************************************************
*	                                    变量
*********************************************************************************************************
*/
static int Count = 0;
extern void CreateOLEDDlg(void);


/*
*********************************************************************************************************
*	                       GUI_WIDGET_CREATE_INFO类型数组
*********************************************************************************************************
*/
static const GUI_WIDGET_CREATE_INFO _aDialogCreate[] = {
    { FRAMEWIN_CreateIndirect,  "armfly",   0,               0,  0,  480,272,0,0},
	{ TEXT_CreateIndirect,      "0000",     GUI_ID_TEXT0,    30,30,300,48, 0,0}
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
			FRAMEWIN_SetFont(hWin,&GUI_Font24B_ASCII);
			FRAMEWIN_SetTextAlign(hWin,GUI_TA_VCENTER|GUI_TA_CENTER);
			FRAMEWIN_SetTitleHeight(hWin,30);

			//
			//初始化文本控件
			//
			TEXT_SetFont(WM_GetDialogItem(hWin,GUI_ID_TEXT0), &GUI_FontD36x48);
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
*	函 数 名: MainTask
*	功能说明: GUI主函数
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void MainTask(void) 
{
	WM_HWIN hDlg;
	WM_HTIMER hTimer;
	
	/* 初始化 */
	GUI_Init();
	
	/*
	 关于多缓冲和窗口内存设备的设置说明
	   1. 使能多缓冲是调用的如下函数，用户要在LCDConf_Lin_Template.c文件中配置了多缓冲，调用此函数才有效：
		  WM_MULTIBUF_Enable(1);
	   2. 窗口使能使用内存设备是调用函数：WM_SetCreateFlags(WM_CF_MEMDEV);
	   3. 如果emWin的配置多缓冲和窗口内存设备都支持，二选一即可，且务必优先选择使用多缓冲，实际使用
		  STM32H7XI + 32位SDRAM + RGB565/RGB888平台测试，多缓冲可以有效的降低窗口移动或者滑动时的撕裂
		  感，并有效的提高流畅性，通过使能窗口使用内存设备是做不到的。
	   4. 所有emWin例子默认是开启三缓冲。
	*/
	WM_MULTIBUF_Enable(1);

	/* 操作LCD-------------------------------------*/
	GUI_SelectLayer(0);
	/* 避免上电后瞬间的撕裂感 */
	LCD_SetBackLight(0);
	GUI_SetBkColor(GUI_BLACK);
	GUI_Clear();
	GUI_Delay(200);
	LCD_SetBackLight(255);
	
	/*
       触摸校准函数默认是注释掉的，电阻屏需要校准，电容屏无需校准。如果用户需要校准电阻屏的话，执行
	   此函数即可，会将触摸校准参数保存到EEPROM里面，以后系统上电会自动从EEPROM里面加载。
	*/
    //TOUCH_Calibration();
	
	/* 创建一个对话框 */
    hDlg = GUI_CreateDialogBox(_aDialogCreate, GUI_COUNTOF(_aDialogCreate), &_cbCallback, 0, 0, 0);

	/* 给对话框hDlg创建定时器，溢出时间是100ms */
	hTimer = WM_CreateTimer(WM_GetClientWindow(hDlg), 0, 100, 0);
	
	/* 未用到，防止警告 */
	(void)hTimer;
	
	/* 操作OLED-------------------------------------*/
	GUI_SelectLayer(1);
	CreateOLEDDlg();
	
	GUI_SelectLayer(0);
	while(1) 
	{
		GUI_Delay(1);
	}
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
