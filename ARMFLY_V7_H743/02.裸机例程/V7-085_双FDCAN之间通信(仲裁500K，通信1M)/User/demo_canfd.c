/*
*********************************************************************************************************
*
*	模块名称 : CAN FD应用。
*	文件名称 : demo_canfd.c
*	版    本 : V1.0
*	说    明 : CAN FD应用
*              1、K1按键按下，CAN2发送消息给CAN1，蜂鸣器鸣响4次。
*              2、K2按键按下，CAN1发送消息给CAN2，点亮LED1。
*              3、K3按键按下，CAN1发送消息给CAN2，熄灭LED1。
*
*	修改记录 :
*		版本号   日期         作者        说明
*		V1.0    2021-01-23   Eric2013    正式发布
*
*	Copyright (C), 2021-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/
#include "bsp.h"
#include "can_network.h"


/*
	启用CAN1，需要将V7主板上的J12跳线帽短接PA11，J13跳线帽短接PA12。
	启用CNA2，硬件无需跳线，以太网功能需要屏蔽（有引脚复用）。
*/

/*
*********************************************************************************************************
*	函 数 名: DemoCANFD
*	功能说明: CAN测试
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void DemoCANFD(void)
{
	uint8_t ucKeyCode;		/* 按键代码 */

	can_Init();	 /* 初始化CAN */
	
	bsp_StartAutoTimer(0, 500);	/* 启动1个500ms的自动重装的定时器 */
	
	while (1)
	{
        {
			MSG_T msg;

			if (bsp_GetMsg(&msg))
			{
				switch (msg.MsgCode)
				{
					case MSG_CAN1_RX:		/* 接收到CAN设备的应答 */
                        printf("CAN1接收到消息\r\n");
					can1_Analyze();
						break;	
					
					case MSG_CAN2_RX:	   /* 接收到CAN设备的应答 */
                         printf("CAN2接收到消息\r\n");
						can2_Analyze();
						break;					
				}
			}
		}
		
		/* 判断定时器超时时间 */
		if (bsp_CheckTimer(0))	
		{            
			/* 每隔500ms 进来一次 */  
			bsp_LedToggle(2);
		}
        
        /* 按键滤波和检测由后台systick中断服务程序实现，我们只需要调用bsp_GetKey读取键值即可。 */
		ucKeyCode = bsp_GetKey();	/* 读取键值, 无键按下时返回 KEY_NONE = 0 */
		if (ucKeyCode != KEY_NONE)
		{
			switch (ucKeyCode)
			{
				case KEY_DOWN_K1:			/* K1键按下 */
					printf("CAN2发送消息给CAN1，蜂鸣器鸣响4次\r\n");
                    can_BeepCtrl(1, 4);
					break;

				case KEY_DOWN_K2:			/* K2键按下 */
					printf("CAN1发送消息给CAN2，点亮LED1\r\n");
                    can_LedOn(1, 1);
					break;


				case KEY_DOWN_K3:		/* K3键按下 */
					printf("CAN1发送消息给CAN2，熄灭LED1\r\n");
                    can_LedOff(1, 1);
					break;

				default:
					/* 其它的键值不处理 */
					break;
			}
		
		}
	}
}
/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
