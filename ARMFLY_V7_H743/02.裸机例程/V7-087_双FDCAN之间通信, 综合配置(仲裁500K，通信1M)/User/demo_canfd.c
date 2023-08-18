/*
*********************************************************************************************************
*
*	模块名称 : CAN FD应用。
*	文件名称 : demo_canfd.c
*	版    本 : V1.0
*	说    明 : CAN FD应用
*              1、K1按键按下，CAN2发送消息给CAN1。
*              2、K2按键按下，CAN1发送消息给CAN2。
*
*	修改记录 :
*		版本号   日期         作者        说明
*		V1.0    2023-05-11   Eric2013    正式发布
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

/* 打印数组 */
void print_int_array(uint8_t *arr, uint32_t size) 
{
    if (size == 0) 
	{
        printf("[]");
    } 
	else
	{     
        putchar('[');
        for (int i = 0; i < size - 1; i++)
            printf("%d, ", arr[i]);
		
        printf("%d]\r\n", arr[size - 1]);
    }
}

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
	uint8_t tx_buf[64];
	MSG_T msg;

	bsp_StartAutoTimer(0, 500);	/* 启动1个500ms的自动重装的定时器 */
	
	while (1)
	{
		/* 判断定时器超时时间 */
		if (bsp_CheckTimer(0))	
		{            
			/* 每隔500ms 进来一次 */  
			bsp_LedToggle(2);
		}
		
		if (bsp_GetMsg(&msg))
		{
			switch (msg.MsgCode)
			{
				case MSG_CAN1_RxFIFO0:		
					printf("CAN1 RxFIFO0收到数据:\r\n");
					print_int_array(g_Can1RxData, 8);
					break;
				
				case MSG_CAN1_RxFIFO1:		
					printf("CAN1 RxFIFO1收到数据:\r\n");
					print_int_array(g_Can1RxData, 8);
					break;	
				
				case MSG_CAN1_RxBuffer:		
					printf("CAN1 RxBuffer:\r\n");
					print_int_array(g_Can1RxData, 8);
					break;

				case MSG_CAN1_TxFIFO:		
					printf("CAN1 TxFIFO发送完成数据\r\n");
					break;	

				case MSG_CAN1_TxBuffer:		
					printf("CAN1 TxBuffer发送完成数据\r\n");
					break;	

				case MSG_CAN2_RxFIFO0:		
					printf("CAN2 RxFIFO0收到消息\r\n");
					print_int_array(g_Can2RxData, 8);
					break;
				
				case MSG_CAN2_RxFIFO1:		
					printf("CAN2 RxFIFO1收到消息\r\n");
					print_int_array(g_Can2RxData, 8);
					break;	
				
				case MSG_CAN2_RxBuffer:		
					printf("CAN2 RxBuffer收到消息\r\n");
					print_int_array(g_Can2RxData, 8);
					break;

				case MSG_CAN2_TxFIFO:		
					printf("CAN2 TxFIFO发送完成\r\n");
					break;	

				case MSG_CAN2_TxBuffer:		
					printf("CAN2 TxBuffer发送完成\r\n");
					break;						
			}
		}
		
        /* 按键滤波和检测由后台systick中断服务程序实现，我们只需要调用bsp_GetKey读取键值即可。 */
		ucKeyCode = bsp_GetKey();	/* 读取键值, 无键按下时返回 KEY_NONE = 0 */
		if (ucKeyCode != KEY_NONE)
		{
			switch (ucKeyCode)
			{
				case KEY_DOWN_K1:			/* K1键按下 */
					printf("CAN2发送消息给CAN1\r\n");
					tx_buf[0] = 0;
					tx_buf[1] = 1;
					tx_buf[2] = 2;
					tx_buf[3] = 3;
					tx_buf[4] = 4;
					tx_buf[5] = 5;
					tx_buf[6] = 6;
					tx_buf[7] = 7;
                    can2_SendPacket(tx_buf, FDCAN_DLC_BYTES_8);
					break;

				case KEY_DOWN_K2:			/* K2键按下 */
					printf("CAN1发送消息给CAN2\r\n");
					tx_buf[0] = 7;
					tx_buf[1] = 6;
					tx_buf[2] = 5;
					tx_buf[3] = 4;
					tx_buf[4] = 3;
					tx_buf[5] = 2;
					tx_buf[6] = 1;
					tx_buf[7] = 0;
                    can1_SendPacket(tx_buf, FDCAN_DLC_BYTES_8);
					break;

				default:
					/* 其它的键值不处理 */
					break;
			}
		
		}
	}
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
