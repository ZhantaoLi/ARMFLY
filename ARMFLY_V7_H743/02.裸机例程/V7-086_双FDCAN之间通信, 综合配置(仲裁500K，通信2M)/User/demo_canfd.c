/*
*********************************************************************************************************
*
*	ģ������ : CAN FDӦ�á�
*	�ļ����� : demo_canfd.c
*	��    �� : V1.0
*	˵    �� : CAN FDӦ��
*              1��K1�������£�CAN2������Ϣ��CAN1��
*              2��K2�������£�CAN1������Ϣ��CAN2��
*
*	�޸ļ�¼ :
*		�汾��   ����         ����        ˵��
*		V1.0    2023-05-11   Eric2013    ��ʽ����
*
*	Copyright (C), 2021-2030, ���������� www.armfly.com
*
*********************************************************************************************************
*/
#include "bsp.h"
#include "can_network.h"


/*
	����CAN1����Ҫ��V7�����ϵ�J12����ñ�̽�PA11��J13����ñ�̽�PA12��
	����CNA2��Ӳ���������ߣ���̫��������Ҫ���Σ������Ÿ��ã���
*/

/* ��ӡ���� */
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
*	�� �� ��: DemoCANFD
*	����˵��: CAN����
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/

void DemoCANFD(void)
{
	uint8_t ucKeyCode;		/* �������� */
	uint8_t tx_buf[64];
	MSG_T msg;

	bsp_StartAutoTimer(0, 500);	/* ����1��500ms���Զ���װ�Ķ�ʱ�� */
	
	while (1)
	{
		/* �ж϶�ʱ����ʱʱ�� */
		if (bsp_CheckTimer(0))	
		{            
			/* ÿ��500ms ����һ�� */  
			bsp_LedToggle(2);
		}
		
		if (bsp_GetMsg(&msg))
		{
			switch (msg.MsgCode)
			{
				case MSG_CAN1_RxFIFO0:		
					printf("CAN1 RxFIFO0�յ�����:\r\n");
					print_int_array(g_Can1RxData, 8);
					break;
				
				case MSG_CAN1_RxFIFO1:		
					printf("CAN1 RxFIFO1�յ�����:\r\n");
					print_int_array(g_Can1RxData, 8);
					break;	
				
				case MSG_CAN1_RxBuffer:		
					printf("CAN1 RxBuffer:\r\n");
					print_int_array(g_Can1RxData, 8);
					break;

				case MSG_CAN1_TxFIFO:		
					printf("CAN1 TxFIFO�����������\r\n");
					break;	

				case MSG_CAN1_TxBuffer:		
					printf("CAN1 TxBuffer�����������\r\n");
					break;	

				case MSG_CAN2_RxFIFO0:		
					printf("CAN2 RxFIFO0�յ���Ϣ\r\n");
					print_int_array(g_Can2RxData, 8);
					break;
				
				case MSG_CAN2_RxFIFO1:		
					printf("CAN2 RxFIFO1�յ���Ϣ\r\n");
					print_int_array(g_Can2RxData, 8);
					break;	
				
				case MSG_CAN2_RxBuffer:		
					printf("CAN2 RxBuffer�յ���Ϣ\r\n");
					print_int_array(g_Can2RxData, 8);
					break;

				case MSG_CAN2_TxFIFO:		
					printf("CAN2 TxFIFO�������\r\n");
					break;	

				case MSG_CAN2_TxBuffer:		
					printf("CAN2 TxBuffer�������\r\n");
					break;						
			}
		}
		
        /* �����˲��ͼ���ɺ�̨systick�жϷ������ʵ�֣�����ֻ��Ҫ����bsp_GetKey��ȡ��ֵ���ɡ� */
		ucKeyCode = bsp_GetKey();	/* ��ȡ��ֵ, �޼�����ʱ���� KEY_NONE = 0 */
		if (ucKeyCode != KEY_NONE)
		{
			switch (ucKeyCode)
			{
				case KEY_DOWN_K1:			/* K1������ */
					printf("CAN2������Ϣ��CAN1\r\n");
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

				case KEY_DOWN_K2:			/* K2������ */
					printf("CAN1������Ϣ��CAN2\r\n");
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
					/* �����ļ�ֵ������ */
					break;
			}
		
		}
	}
}

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
