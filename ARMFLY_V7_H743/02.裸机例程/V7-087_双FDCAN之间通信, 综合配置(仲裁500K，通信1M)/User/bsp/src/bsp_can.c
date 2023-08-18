/*
*********************************************************************************************************
*
*	ģ������ : FDCAN����ģ��
*	�ļ����� : bsp_can.c
*	��    �� : V1.1
*	˵    �� : CAN����. 
*
*	�޸ļ�¼ :
*		�汾��  ����        ����      ˵��
*		V1.0    2018-11-14  armfly   ��ʽ����
*		V1.1    2021-01-23  armfly   ����
*		V1.2    2023-05-12  Eric2013 ����
*
*	Copyright (C), 2021-2030, ���������� www.armfly.com
*
*********************************************************************************************************
*/

#include "bsp.h"



/*
	����CAN1����Ҫ��V7�����ϵ�J12����ñ�̽�PA11��J13����ñ�̽�PA12��
	����CNA2��Ӳ���������ߣ���̫��������Ҫ���Σ������Ÿ��ã���
*/

/*
*********************************************************************************************************
*                                             FDCAN1����
*********************************************************************************************************
*/
/* FDCAN1 GPIO���� */
#define FDCAN1_TX_PIN       GPIO_PIN_12
#define FDCAN1_TX_GPIO_PORT GPIOA
#define FDCAN1_TX_AF        GPIO_AF9_FDCAN1
#define FDCAN1_TX_GPIO_CLK_ENABLE() __HAL_RCC_GPIOA_CLK_ENABLE()

#define FDCAN1_RX_PIN       GPIO_PIN_11
#define FDCAN1_RX_GPIO_PORT GPIOA
#define FDCAN1_RX_AF        GPIO_AF9_FDCAN1
#define FDCAN1_RX_GPIO_CLK_ENABLE() __HAL_RCC_GPIOA_CLK_ENABLE()

FDCAN_HandleTypeDef hfdcan1;
FDCAN_FilterTypeDef sFilterConfig1;

/*
*********************************************************************************************************
*                                             FDCAN2����
*********************************************************************************************************
*/
/* FDCAN1 GPIO���� */
#define FDCAN2_TX_PIN       GPIO_PIN_13
#define FDCAN2_TX_GPIO_PORT GPIOB
#define FDCAN2_TX_AF        GPIO_AF9_FDCAN2
#define FDCAN2_TX_GPIO_CLK_ENABLE() __HAL_RCC_GPIOB_CLK_ENABLE()

#define FDCAN2_RX_PIN       GPIO_PIN_12
#define FDCAN2_RX_GPIO_PORT GPIOB
#define FDCAN2_RX_AF        GPIO_AF9_FDCAN2
#define FDCAN2_RX_GPIO_CLK_ENABLE() __HAL_RCC_GPIOB_CLK_ENABLE()

FDCAN_HandleTypeDef hfdcan2;
FDCAN_FilterTypeDef sFilterConfig2;

FDCAN_RxHeaderTypeDef g_Can1RxHeader;
uint8_t g_Can1RxData[64];

FDCAN_RxHeaderTypeDef g_Can2RxHeader;
uint8_t g_Can2RxData[64];
	
/*
*********************************************************************************************************
*	�� �� ��: bsp_InitCan1
*	����˵��: ��ʼCAN1
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void bsp_InitCan1(void)
{	 
	/* ��1������������ ==================================================================================*/
	hfdcan1.Instance = FDCAN1;                     /* ����FDCAN1 */             
	hfdcan1.Init.FrameFormat = FDCAN_FRAME_FD_BRS; /* ����ʹ��FDCAN�ɱ䲨���� */  
	hfdcan1.Init.Mode = FDCAN_MODE_NORMAL;         /* ����ʹ������ģʽ */ 
	hfdcan1.Init.AutoRetransmission = ENABLE;      /* ʹ���Զ��ط� */ 
	hfdcan1.Init.TransmitPause = DISABLE;          /* ���ý�ֹ������ͣ���� */
	hfdcan1.Init.ProtocolException = ENABLE;       /* Э���쳣����ʹ�� */
	
	/* ��2�������������� ==================================================================================*/
	/* 
		�����ٲý׶β����� 
		CANʱ��20MHzʱ���ٲý׶εĲ����ʾ���
		CAN FD Freq / (Sync_Seg + Pro_Seg + Phase_Seg1 + Phase_Seg2) = 20MHz / ��1+0x1F + 8�� = 0.5Mbps	
		
		����Sync_Seg�ǹ̶�ֵ = 1 �� Pro_Seg + Phase_Seg1 = NominalTimeSeg1�� Phase_Seg2 = NominalTimeSeg2
	*/
	hfdcan1.Init.NominalPrescaler = 0x01; 	  /* CANʱ�ӷ������ã�һ������Ϊ1���ɣ�ȫ����PLL���úã�tq = NominalPrescaler x (1/ fdcan_ker_ck), ��Χ1-512 */
	hfdcan1.Init.NominalSyncJumpWidth = 0x08; /* ���ڶ�̬����  Phase_Seg1�� Phase_Seg1�����Բ����Ա�Phase_Seg1�� Phase_Seg1�󣬷�Χ1-128 */
	hfdcan1.Init.NominalTimeSeg1 = 0x1F; 	  /* �ر�ע�������Seg1����������������֮�ͣ���Ӧλʱ������ͼ�� Pro_Seg + Phase_Seg1����Χ2-256 */
	hfdcan1.Init.NominalTimeSeg2 = 0x08;      /* ��Ӧλʱ������ͼ�� Phase_Seg2����Χ2- 128 */


	/* 
		�������ݽ׶β����� 
		CANʱ��20MHzʱ�����ݽ׶εĲ����ʾ���
		CAN FD Freq / (Sync_Seg + Pro_Seg + Phase_Seg1 + Phase_Seg2) = 20MHz / 2 / ��1+5+ 4�� = 1Mbps
		
		����Sync_Seg�ǹ̶�ֵ = 1 �� Pro_Seg + Phase_Seg1 = DataTimeSeg1�� Phase_Seg2 = DataTimeSeg2
	*/
	hfdcan1.Init.DataPrescaler = 0x02;      /* CANʱ�ӷ������ã�һ������Ϊ1���ɣ�ȫ����PLL���úã�tq = NominalPrescaler x (1/ fdcan_ker_ck)����Χ1-32 */
	hfdcan1.Init.DataSyncJumpWidth = 0x04;  /* ���ڶ�̬����  Phase_Seg1�� Phase_Seg1�����Բ����Ա�Phase_Seg1�� Phase_Seg1�󣬷�Χ1-16 */
	hfdcan1.Init.DataTimeSeg1 = 0x05; 		/* �ر�ע�������Seg1����������������֮�ͣ���Ӧλʱ������ͼ�� Pro_Seg + Phase_Seg1����Χ */
	hfdcan1.Init.DataTimeSeg2 = 0x04;       /* ��Ӧλʱ������ͼ�� Phase_Seg2 */    
	
	/* ��3����RAM���������� ==================================================================================*/
	hfdcan1.Init.MessageRAMOffset = 0;      			/* CAN1��CAN2����2560����, ����CAN1����ǰ1280�� */
	hfdcan1.Init.StdFiltersNbr = 4;         			/* ���ñ�׼ID��������������Χ0-128 */       
	hfdcan1.Init.ExtFiltersNbr = 4;         			/* ������չID��������������Χ0-64 */   
	hfdcan1.Init.RxFifo0ElmtsNbr = 3;                   /* ����Rx FIFO0��Ԫ�ظ�������Χ0-64 */  
	hfdcan1.Init.RxFifo0ElmtSize = FDCAN_DATA_BYTES_8; 	/* ����Rx FIFO0��ÿ��Ԫ�ش�С��֧��8,12,16,20,24,32,48����64�ֽ� */   
	hfdcan1.Init.RxFifo1ElmtsNbr = 3;                   /* ����Rx FIFO1��Ԫ�ظ�������Χ0-64 */
	hfdcan1.Init.RxFifo1ElmtSize = FDCAN_DATA_BYTES_8;  /* ����Rx FIFO1��ÿ��Ԫ�ش�С��֧��8,12,16,20,24,32,48����64�ֽ� */	
	hfdcan1.Init.RxBuffersNbr = 2;                      /* ����Rx Buffer��������Χ0-64 */
	hfdcan1.Init.RxBufferSize = FDCAN_DATA_BYTES_8;     /* ����Rx Buffer��ÿ��Ԫ�ش�С��֧��8,12,16,20,24,32,48����64�ֽ� */	

	hfdcan1.Init.TxEventsNbr = 16;							/* ����Tx Event FIFO��Ԫ�ظ�������Χ0-32 */	
	hfdcan1.Init.TxBuffersNbr = 8;							/* ����Tx Buffer��Ԫ�ظ�������Χ0-32 */
	hfdcan1.Init.TxFifoQueueElmtsNbr = 8;                   /* ��������Tx FIFO/Queue��Tx Buffers��������Χ0��32 */
	hfdcan1.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION; /* ����FIFOģʽ����QUEUE����ģʽ */
	hfdcan1.Init.TxElmtSize = FDCAN_DATA_BYTES_8;           /* ����Tx Element�е��������С��֧��8,12,16,20,24,32,48����64�ֽ� */
	HAL_FDCAN_Init(&hfdcan1);

	/* 
		���ù�����, ��������Ҫ���ڽ��գ���������λģʽ��
		FilterID1 = filter
		FilterID2 = mask
		
		FilterID2��maskÿ��bit����
		0: �����ģ���λ�����ڱȽϣ�
		1: ����ƥ�䣬���յ���ID�������˲�����Ӧ��IDλ��һ�¡�
		
		����˵����
		FilterID1 = 0x111  0001 0001 0001 
		FilterID2 = 0x7FF  0111 1111 1111
		��ʾ������IDΪ0x111��FDCAN֡��
	*/
	
	/* ��4������չID�������� ==================================================================================*/
	sFilterConfig1.IdType = FDCAN_EXTENDED_ID;              /* ��չID */
	sFilterConfig1.FilterIndex = 0;   						/* ���ڹ�����������չID����Χ0��64 */
	sFilterConfig1.FilterType = FDCAN_FILTER_RANGE;         /* ��Χ���� -----------------*/
	sFilterConfig1.FilterConfig = FDCAN_FILTER_TO_RXFIFO1;  /* �������ƥ�䣬�����ݱ��浽Rx FIFO 1 */
	sFilterConfig1.FilterID1 = 0x12345678;                  /* ��ʼID:0x12345678 */
	sFilterConfig1.FilterID2 = 0x12345680; 					/* ����ID:0x12345680 */
	HAL_FDCAN_ConfigFilter(&hfdcan1, &sFilterConfig1);      /* ���ù����� */
	
	sFilterConfig1.IdType = FDCAN_EXTENDED_ID;              /* ��չID */
	sFilterConfig1.FilterIndex = 1;   						/* ���ڹ�����������չID����Χ0��64 */
	sFilterConfig1.FilterType = FDCAN_FILTER_DUAL;          /* ר��ID���� --------------*/
	sFilterConfig1.FilterConfig = FDCAN_FILTER_TO_RXFIFO1;  /* �������ƥ�䣬�����ݱ��浽Rx FIFO 1 */
	sFilterConfig1.FilterID1 = 0x12345681;                  /* ָ��ID */
	sFilterConfig1.FilterID2 = 0x12345682; 					/* ָ��ID */
	HAL_FDCAN_ConfigFilter(&hfdcan1, &sFilterConfig1);      /* ���ù����� */
	
	sFilterConfig1.IdType = FDCAN_EXTENDED_ID;              /* ��չID */
	sFilterConfig1.FilterIndex = 2;   						/* ���ڹ�����������չID����Χ0��64 */
	sFilterConfig1.FilterType = FDCAN_FILTER_MASK;          /* ��������������λģʽ ----*/
	sFilterConfig1.FilterConfig = FDCAN_FILTER_TO_RXFIFO1;  /* �������ƥ�䣬�����ݱ��浽Rx FIFO 1 */
	sFilterConfig1.FilterID1 = 0x12345683;                  /* ����λģʽ�£�FilterID1����ϢID */
	sFilterConfig1.FilterID2 = 0x1FFFFFFF; 					/* ����λģʽ�£�FilterID2����Ϣ����λ */
	HAL_FDCAN_ConfigFilter(&hfdcan1, &sFilterConfig1);      /* ���ù����� */
	
	sFilterConfig1.IdType = FDCAN_EXTENDED_ID;              /* ��չID */
	sFilterConfig1.FilterIndex = 3;   						/* ���ڹ�����������չID����Χ0��64 */
	sFilterConfig1.FilterType = FDCAN_FILTER_MASK;          /* ���� --------------------*/
	sFilterConfig1.FilterConfig = FDCAN_FILTER_TO_RXBUFFER; /* �������ƥ�䣬�����ݱ��浽Rx Buffer */
	sFilterConfig1.FilterID1 = 0x12345684;                  /* ����ָ��ID��Ϣ */
	sFilterConfig1.FilterID2 = 0x1FFFFFFF; 					/* ���� */
	sFilterConfig1.IsCalibrationMsg = 0;                    /* ������Ϣ */
	sFilterConfig1.RxBufferIndex = 0;                       /* Rx bufferλ�� */
	HAL_FDCAN_ConfigFilter(&hfdcan1, &sFilterConfig1);      /* ���ù����� */

	/* ��5������׼ID�������� ==================================================================================*/	
	sFilterConfig1.IdType = FDCAN_STANDARD_ID;              /* ���ñ�׼ID������չID */
	sFilterConfig1.FilterIndex = 0;   						/* ���ڹ�����������׼ID����Χ0��127 */
	sFilterConfig1.FilterType = FDCAN_FILTER_RANGE;         /* ��Χ���� -------------------------*/
	sFilterConfig1.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;  /* �������ƥ�䣬�����ݱ��浽Rx FIFO 0 */
	sFilterConfig1.FilterID1 = 0x000;                       /* ��ʼID:0x000 */
	sFilterConfig1.FilterID2 = 0x010; 						/* ����ID:0x010 */
	HAL_FDCAN_ConfigFilter(&hfdcan1, &sFilterConfig1);      /* ���ù����� */
	
	sFilterConfig1.IdType = FDCAN_STANDARD_ID;              /* ���ñ�׼ID������չID */
	sFilterConfig1.FilterIndex = 1;   						/* ���ڹ�����������׼ID����Χ0��127 */
	sFilterConfig1.FilterType = FDCAN_FILTER_DUAL;          /* ר��ID���� ----------------------*/
	sFilterConfig1.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;  /* �������ƥ�䣬�����ݱ��浽Rx FIFO 0 */
	sFilterConfig1.FilterID1 = 0x011;                       /* ָ��ID */
	sFilterConfig1.FilterID2 = 0x012; 						/* ָ��ID */
	HAL_FDCAN_ConfigFilter(&hfdcan1, &sFilterConfig1);      /* ���ù����� */
	
	sFilterConfig1.IdType = FDCAN_STANDARD_ID;              /* ���ñ�׼ID������չID */
	sFilterConfig1.FilterIndex = 2;   						/* ���ڹ�����������׼ID����Χ0��127 */
	sFilterConfig1.FilterType = FDCAN_FILTER_MASK;          /* ��������������λģʽ -----------*/
	sFilterConfig1.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;  /* �������ƥ�䣬�����ݱ��浽Rx FIFO 0 */
	sFilterConfig1.FilterID1 = 0x013;                       /* ����λģʽ�£�FilterID1����ϢID */
	sFilterConfig1.FilterID2 = 0x7FF; 						/* ����λģʽ�£�FilterID2����Ϣ����λ */
	HAL_FDCAN_ConfigFilter(&hfdcan1, &sFilterConfig1);      /* ���ù����� */
	
	sFilterConfig1.IdType = FDCAN_STANDARD_ID;              /* ���ñ�׼ID������չID */
	sFilterConfig1.FilterIndex = 3;   						/* ���ڹ�����������չID����Χ0��64 */
	sFilterConfig1.FilterType = FDCAN_FILTER_MASK;          /* ���� ---------------------------*/
	sFilterConfig1.FilterConfig = FDCAN_FILTER_TO_RXBUFFER; /* �������ƥ�䣬�����ݱ��浽Rx BUFFER */
	sFilterConfig1.FilterID1 = 0x014;                       /* ����ָ��ID��Ϣ */
	sFilterConfig1.FilterID2 = 0x7FF; 					    /* ���� */
	sFilterConfig1.IsCalibrationMsg = 0;                    /* ������Ϣ */
	sFilterConfig1.RxBufferIndex = 1;                       /* Rx bufferλ�� */
	HAL_FDCAN_ConfigFilter(&hfdcan1, &sFilterConfig1);      /* ���ù����� */
	
	HAL_FDCAN_ConfigGlobalFilter(&hfdcan1,                  /* ȫ�ֹ������� */
	                              FDCAN_REJECT,             /* ���յ���ϢID���׼ID���˲�ƥ�䣬������ */
								  FDCAN_REJECT,             /* ���յ���ϢID����չID���˲�ƥ�䣬������ */
								  FDCAN_FILTER_REMOTE,      /* ���˱�׼IDԶ��֡ */ 
								  FDCAN_FILTER_REMOTE);     /* ������չIDԶ��֡ */ 
	

	/* ��5����Rx FIFO0�ж����� ===================================================================*/	
	/* ����Rx FIFO0����ֵΪ1 ---------------------------------------*/
	HAL_FDCAN_ConfigFifoWatermark(&hfdcan1, FDCAN_CFG_RX_FIFO0, 1);

	/* ����RX FIFO0����ֵ�ж� */
	HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_IT_RX_FIFO0_WATERMARK, 0);
	
	/* ��6����Rx FIFO1�ж����� ===================================================================*/	
	/* ����RX FIFO1���������ж� */
	HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_IT_RX_FIFO1_NEW_MESSAGE, 0);
	
	/* ��7����Rx Buffer�ж����� ==================================================================*/	
	/* Rx Buffer���յ�����Ϣ�ʹ����ж� */
	HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_IT_RX_BUFFER_NEW_MESSAGE, 0);	
	
	/* ����FDCAN */
	HAL_FDCAN_Start(&hfdcan1);	
}

/*
*********************************************************************************************************
*	�� �� ��: bsp_InitCan2
*	����˵��: ��ʼCAN2
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void bsp_InitCan2(void)
{	
	/* ��1������������ ==================================================================================*/
	hfdcan2.Instance = FDCAN2;                     /* ����FDCAN1 */             
	hfdcan2.Init.FrameFormat = FDCAN_FRAME_FD_BRS; /* ����ʹ��FDCAN�ɱ䲨���� */  
	hfdcan2.Init.Mode = FDCAN_MODE_NORMAL;         /* ����ʹ������ģʽ */ 
	hfdcan2.Init.AutoRetransmission = ENABLE;      /* ʹ���Զ��ط� */ 
	hfdcan2.Init.TransmitPause = DISABLE;          /* ���ý�ֹ������ͣ���� */
	hfdcan2.Init.ProtocolException = ENABLE;       /* Э���쳣����ʹ�� */
	
	/* ��2�������������� ==================================================================================*/
	/* 
		�����ٲý׶β����� 
		CANʱ��20MHzʱ���ٲý׶εĲ����ʾ���
		CAN FD Freq / (Sync_Seg + Pro_Seg + Phase_Seg1 + Phase_Seg2) = 20MHz / ��1+0x1F + 8�� = 0.5Mbps	
		
		����Sync_Seg�ǹ̶�ֵ = 1 �� Pro_Seg + Phase_Seg1 = NominalTimeSeg1�� Phase_Seg2 = NominalTimeSeg2
	*/
	hfdcan2.Init.NominalPrescaler = 0x01; 	  /* CANʱ�ӷ������ã�һ������Ϊ1���ɣ�ȫ����PLL���úã�tq = NominalPrescaler x (1/ fdcan_ker_ck) */
	hfdcan2.Init.NominalSyncJumpWidth = 0x08; /* ���ڶ�̬����  Phase_Seg1�� Phase_Seg1�����Բ����Ա�Phase_Seg1�� Phase_Seg1�� */
	hfdcan2.Init.NominalTimeSeg1 = 0x1F; 	  /* �ر�ע�������Seg1����������������֮�ͣ���Ӧλʱ������ͼ�� Pro_Seg + Phase_Seg1 */
	hfdcan2.Init.NominalTimeSeg2 = 0x08;      /* ��Ӧλʱ������ͼ�� Phase_Seg2 */


	/* 
		�������ݽ׶β����� 
		CANʱ��20MHzʱ�����ݽ׶εĲ����ʾ���
		CAN FD Freq / (Sync_Seg + Pro_Seg + Phase_Seg1 + Phase_Seg2) = 20MHz / 2 / ��1+5+ 4�� = 1Mbps
		
		����Sync_Seg�ǹ̶�ֵ = 1 �� Pro_Seg + Phase_Seg1 = DataTimeSeg1�� Phase_Seg2 = DataTimeSeg2
	*/
	hfdcan2.Init.DataPrescaler = 0x02;      /* CANʱ�ӷ������ã�һ������Ϊ1���ɣ�ȫ����PLL���úã�tq = NominalPrescaler x (1/ fdcan_ker_ck)����Χ1-32 */
	hfdcan2.Init.DataSyncJumpWidth = 0x04;  /* ���ڶ�̬����  Phase_Seg1�� Phase_Seg1�����Բ����Ա�Phase_Seg1�� Phase_Seg1�󣬷�Χ1-16 */
	hfdcan2.Init.DataTimeSeg1 = 0x05; 		/* �ر�ע�������Seg1����������������֮�ͣ���Ӧλʱ������ͼ�� Pro_Seg + Phase_Seg1����Χ */
	hfdcan2.Init.DataTimeSeg2 = 0x04;       /* ��Ӧλʱ������ͼ�� Phase_Seg2 */    
	
	/* ��3����RAM���������� ==================================================================================*/
	hfdcan2.Init.MessageRAMOffset = 1280;      			/* CAN1��CAN2����2560����, ����CAN1�����1280�� */
	hfdcan2.Init.StdFiltersNbr = 4;         			/* ���ñ�׼ID��������������Χ0-128 */       
	hfdcan2.Init.ExtFiltersNbr = 4;         			/* ������չID��������������Χ0-64 */   
	hfdcan2.Init.RxFifo0ElmtsNbr = 3;                   /* ����Rx FIFO0��Ԫ�ظ�������Χ0-64 */  
	hfdcan2.Init.RxFifo0ElmtSize = FDCAN_DATA_BYTES_8; 	/* ����Rx FIFO0��ÿ��Ԫ�ش�С��֧��8,12,16,20,24,32,48����64�ֽ� */   
	hfdcan2.Init.RxFifo1ElmtsNbr = 3;                   /* ����Rx FIFO1��Ԫ�ظ�������Χ0-64 */
	hfdcan2.Init.RxFifo1ElmtSize = FDCAN_DATA_BYTES_8;  /* ����Rx FIFO1��ÿ��Ԫ�ش�С��֧��8,12,16,20,24,32,48����64�ֽ� */	
	hfdcan2.Init.RxBuffersNbr = 2;                      /* ����Rx Buffer��������Χ0-64 */
	hfdcan2.Init.RxBufferSize = FDCAN_DATA_BYTES_8;     /* ����Rx Buffer��ÿ��Ԫ�ش�С��֧��8,12,16,20,24,32,48����64�ֽ� */	

	hfdcan2.Init.TxEventsNbr = 16;							/* ����Tx Event FIFO��Ԫ�ظ�������Χ0-32 */	
	hfdcan2.Init.TxBuffersNbr = 8;							/* ����Tx Buffer��Ԫ�ظ�������Χ0-32 */
	hfdcan2.Init.TxFifoQueueElmtsNbr = 8;                   /* ��������Tx FIFO/Queue��Tx Buffers��������Χ0��32 */
	hfdcan2.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION; /* ����FIFOģʽ����QUEUE����ģʽ */
	hfdcan2.Init.TxElmtSize = FDCAN_DATA_BYTES_8;           /* ����Tx Element�е��������С��֧��8,12,16,20,24,32,48����64�ֽ� */
	HAL_FDCAN_Init(&hfdcan2);

	/* 
		���ù�����, ��������Ҫ���ڽ��գ���������λģʽ��
		FilterID1 = filter
		FilterID2 = mask
		
		FilterID2��maskÿ��bit����
		0: �����ģ���λ�����ڱȽϣ�
		1: ����ƥ�䣬���յ���ID�������˲�����Ӧ��IDλ��һ�¡�
		
		����˵����
		FilterID1 = 0x111  0001 0001 0001 
		FilterID2 = 0x7FF  0111 1111 1111
		��ʾ������IDΪ0x111��FDCAN֡��
	*/
	
	/* ��4������չID�������� ==================================================================================*/
	sFilterConfig2.IdType = FDCAN_EXTENDED_ID;              /* ��չID */
	sFilterConfig2.FilterIndex = 0;   						/* ���ڹ�����������չID����Χ0��64 */
	sFilterConfig2.FilterType = FDCAN_FILTER_RANGE;         /* ��Χ���� -----------------*/
	sFilterConfig2.FilterConfig = FDCAN_FILTER_TO_RXFIFO1;  /* �������ƥ�䣬�����ݱ��浽Rx FIFO 1 */
	sFilterConfig2.FilterID1 = 0x12345678;                  /* ��ʼID:0x12345678 */
	sFilterConfig2.FilterID2 = 0x12345680; 					/* ����ID:0x12345680 */
	HAL_FDCAN_ConfigFilter(&hfdcan2, &sFilterConfig2);      /* ���ù����� */
	
	sFilterConfig2.IdType = FDCAN_EXTENDED_ID;              /* ��չID */
	sFilterConfig2.FilterIndex = 1;   						/* ���ڹ�����������չID����Χ0��64 */
	sFilterConfig2.FilterType = FDCAN_FILTER_DUAL;          /* ר��ID���� --------------*/
	sFilterConfig2.FilterConfig = FDCAN_FILTER_TO_RXFIFO1;  /* �������ƥ�䣬�����ݱ��浽Rx FIFO 1 */
	sFilterConfig2.FilterID1 = 0x12345681;                  /* ָ��ID */
	sFilterConfig2.FilterID2 = 0x12345682; 					/* ָ��ID */
	HAL_FDCAN_ConfigFilter(&hfdcan2, &sFilterConfig2);      /* ���ù����� */
	
	sFilterConfig2.IdType = FDCAN_EXTENDED_ID;              /* ��չID */
	sFilterConfig2.FilterIndex = 2;   						/* ���ڹ�����������չID����Χ0��64 */
	sFilterConfig2.FilterType = FDCAN_FILTER_MASK;          /* ��������������λģʽ ----*/
	sFilterConfig2.FilterConfig = FDCAN_FILTER_TO_RXFIFO1;  /* �������ƥ�䣬�����ݱ��浽Rx FIFO 1 */
	sFilterConfig2.FilterID1 = 0x12345683;                  /* ����λģʽ�£�FilterID1����ϢID */
	sFilterConfig2.FilterID2 = 0x1FFFFFFF; 					/* ����λģʽ�£�FilterID2����Ϣ����λ */
	HAL_FDCAN_ConfigFilter(&hfdcan2, &sFilterConfig2);      /* ���ù����� */
	
	sFilterConfig2.IdType = FDCAN_EXTENDED_ID;              /* ��չID */
	sFilterConfig2.FilterIndex = 3;   						/* ���ڹ�����������չID����Χ0��64 */
	sFilterConfig2.FilterType = FDCAN_FILTER_MASK;          /* ���� --------------------*/
	sFilterConfig2.FilterConfig = FDCAN_FILTER_TO_RXBUFFER; /* �������ƥ�䣬�����ݱ��浽Rx Buffer */
	sFilterConfig2.FilterID1 = 0x12345684;                  /* ����ָ��ID��Ϣ */
	sFilterConfig2.FilterID2 = 0x1FFFFFFF; 					/* ���� */
	sFilterConfig2.IsCalibrationMsg = 0;                    /* ������Ϣ */
	sFilterConfig2.RxBufferIndex = 0;                       /* Rx bufferλ�� */
	HAL_FDCAN_ConfigFilter(&hfdcan2, &sFilterConfig2);      /* ���ù����� */

	/* ��5������׼ID�������� ==================================================================================*/	
	sFilterConfig2.IdType = FDCAN_STANDARD_ID;              /* ���ñ�׼ID������չID */
	sFilterConfig2.FilterIndex = 0;   						/* ���ڹ�����������׼ID����Χ0��127*/
	sFilterConfig2.FilterType = FDCAN_FILTER_RANGE;         /* ��Χ���� -------------------------*/
	sFilterConfig2.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;  /* �������ƥ�䣬�����ݱ��浽Rx FIFO 0 */
	sFilterConfig2.FilterID1 = 0x000;                       /* ��ʼID:0x000 */
	sFilterConfig2.FilterID2 = 0x010; 						/* ����ID:0x010 */
	HAL_FDCAN_ConfigFilter(&hfdcan2, &sFilterConfig2);      /* ���ù����� */
	
	sFilterConfig2.IdType = FDCAN_STANDARD_ID;              /* ���ñ�׼ID������չID */
	sFilterConfig2.FilterIndex = 1;   						/* ���ڹ�����������׼ID����Χ0��127*/
	sFilterConfig2.FilterType = FDCAN_FILTER_DUAL;          /* ר��ID���� ----------------------*/
	sFilterConfig2.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;  /* �������ƥ�䣬�����ݱ��浽Rx FIFO 0 */
	sFilterConfig2.FilterID1 = 0x011;                       /* ָ��ID */
	sFilterConfig2.FilterID2 = 0x012; 						/* ָ��ID */
	HAL_FDCAN_ConfigFilter(&hfdcan2, &sFilterConfig2);      /* ���ù����� */
	
	sFilterConfig2.IdType = FDCAN_STANDARD_ID;              /* ���ñ�׼ID������չID */
	sFilterConfig2.FilterIndex = 2;   						/* ���ڹ�����������׼ID����Χ0��127*/
	sFilterConfig2.FilterType = FDCAN_FILTER_MASK;          /* ��������������λģʽ -----------*/
	sFilterConfig2.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;  /* �������ƥ�䣬�����ݱ��浽Rx FIFO 0 */
	sFilterConfig2.FilterID1 = 0x013;                       /* ����λģʽ�£�FilterID1����ϢID */
	sFilterConfig2.FilterID2 = 0x7FF; 						/* ����λģʽ�£�FilterID2����Ϣ����λ */
	HAL_FDCAN_ConfigFilter(&hfdcan2, &sFilterConfig2);      /* ���ù����� */
	
	sFilterConfig2.IdType = FDCAN_STANDARD_ID;              /* ���ñ�׼ID������չID */
	sFilterConfig2.FilterIndex = 3;   						/* ���ڹ�����������չID����Χ0��64 */
	sFilterConfig2.FilterType = FDCAN_FILTER_MASK;          /* ���� ---------------------------*/
	sFilterConfig2.FilterConfig = FDCAN_FILTER_TO_RXBUFFER; /* �������ƥ�䣬�����ݱ��浽Rx BUFFER */
	sFilterConfig2.FilterID1 = 0x014;                       /* ����ָ��ID��Ϣ */
	sFilterConfig2.FilterID2 = 0x7FF; 					    /* ���� */
	sFilterConfig2.IsCalibrationMsg = 0;                    /* ������Ϣ */
	sFilterConfig2.RxBufferIndex = 1;                       /* Rx bufferλ�� */
	HAL_FDCAN_ConfigFilter(&hfdcan2, &sFilterConfig2);      /* ���ù����� */
	
	HAL_FDCAN_ConfigGlobalFilter(&hfdcan2,                  /* ȫ�ֹ������� */
	                              FDCAN_REJECT,             /* ���յ���ϢID���׼ID���˲�ƥ�䣬������ */
								  FDCAN_REJECT,             /* ���յ���ϢID����չID���˲�ƥ�䣬������ */
								  FDCAN_FILTER_REMOTE,      /* ���˱�׼IDԶ��֡ */ 
								  FDCAN_FILTER_REMOTE);     /* ������չIDԶ��֡ */ 
	

	/* ��5����Rx FIFO0�ж����� ===================================================================*/	
	/* ����Rx FIFO0����ֵΪ1 ---------------------------------------*/
	HAL_FDCAN_ConfigFifoWatermark(&hfdcan2, FDCAN_CFG_RX_FIFO0, 1);

	/* ����RX FIFO0����ֵ�ж� */
	HAL_FDCAN_ActivateNotification(&hfdcan2, FDCAN_IT_RX_FIFO0_WATERMARK, 0);
	
	/* ��6����Rx FIFO1�ж����� ===================================================================*/	
	/* ����RX FIFO1���������ж� */
	HAL_FDCAN_ActivateNotification(&hfdcan2, FDCAN_IT_RX_FIFO1_NEW_MESSAGE, 0);
	
	/* ��7����Rx Buffer�ж����� ==================================================================*/	
	/* Rx Buffer���յ�����Ϣ�ʹ����ж� */
	HAL_FDCAN_ActivateNotification(&hfdcan2, FDCAN_IT_RX_BUFFER_NEW_MESSAGE, 0);	
	
	/* ����FDCAN */
	HAL_FDCAN_Start(&hfdcan2);	
}

/*
*********************************************************************************************************
*	�� �� ��: bsp_DeInitCan1
*	����˵��: �ͷ�CAN1
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void bsp_DeInitCan1(void)
{
	HAL_FDCAN_MspDeInit(&hfdcan1);
}

/*
*********************************************************************************************************
*	�� �� ��: bsp_DeInitCan2
*	����˵��: �ͷ�CAN2
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void bsp_DeInitCan2(void)
{
	HAL_FDCAN_MspDeInit(&hfdcan2);
}

/*
*********************************************************************************************************
*	�� �� ��: HAL_FDCAN_MspInit
*	����˵��: ����CAN gpio
*	��    ��: hfdcan
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void HAL_FDCAN_MspInit(FDCAN_HandleTypeDef* hfdcan)
{
	GPIO_InitTypeDef  GPIO_InitStruct;
	RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

	if (hfdcan == &hfdcan1)
	{
		/*##-1- ʹ���������GPIOʱ�� #################################*/
		FDCAN1_TX_GPIO_CLK_ENABLE();
		FDCAN1_RX_GPIO_CLK_ENABLE();

		/* ѡ��PLL2Q��ΪFDCANxʱ�� */
        PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_FDCAN;
        PeriphClkInitStruct.PLL2.PLL2M = 5;
        PeriphClkInitStruct.PLL2.PLL2N = 80;
        PeriphClkInitStruct.PLL2.PLL2P = 2;
        PeriphClkInitStruct.PLL2.PLL2Q = 20;
        PeriphClkInitStruct.PLL2.PLL2R = 2;
        PeriphClkInitStruct.PLL2.PLL2RGE = RCC_PLL2VCIRANGE_2;
        PeriphClkInitStruct.PLL2.PLL2VCOSEL = RCC_PLL2VCOWIDE;
        PeriphClkInitStruct.PLL2.PLL2FRACN = 0;
        PeriphClkInitStruct.FdcanClockSelection = RCC_FDCANCLKSOURCE_PLL2;
        if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
        {
            Error_Handler(__FILE__, __LINE__);
        }

		__HAL_RCC_FDCAN_CLK_ENABLE();

		GPIO_InitStruct.Pin       = FDCAN1_TX_PIN;
		GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull      = GPIO_PULLUP;
		GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
		GPIO_InitStruct.Alternate = FDCAN1_TX_AF;
		HAL_GPIO_Init(FDCAN1_TX_GPIO_PORT, &GPIO_InitStruct);

		GPIO_InitStruct.Pin       = FDCAN1_RX_PIN;
		GPIO_InitStruct.Alternate = FDCAN1_RX_AF;
		HAL_GPIO_Init(FDCAN1_RX_GPIO_PORT, &GPIO_InitStruct);

		HAL_NVIC_SetPriority(FDCAN1_IT0_IRQn, 2, 0);
		HAL_NVIC_SetPriority(FDCAN1_IT1_IRQn, 2, 0);
		HAL_NVIC_SetPriority(FDCAN_CAL_IRQn, 2, 0);
		HAL_NVIC_EnableIRQ(FDCAN1_IT0_IRQn);
		HAL_NVIC_EnableIRQ(FDCAN1_IT1_IRQn);
		HAL_NVIC_EnableIRQ(FDCAN_CAL_IRQn);
	}
	
	if (hfdcan == &hfdcan2)
	{
		/* ѡ��PLL2Q��ΪFDCANxʱ�� */
		FDCAN2_TX_GPIO_CLK_ENABLE();
		FDCAN2_RX_GPIO_CLK_ENABLE();

        PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_FDCAN;
        PeriphClkInitStruct.PLL2.PLL2M = 5;
        PeriphClkInitStruct.PLL2.PLL2N = 80;
        PeriphClkInitStruct.PLL2.PLL2P = 2;
        PeriphClkInitStruct.PLL2.PLL2Q = 20;
        PeriphClkInitStruct.PLL2.PLL2R = 2;
        PeriphClkInitStruct.PLL2.PLL2RGE = RCC_PLL2VCIRANGE_2;
        PeriphClkInitStruct.PLL2.PLL2VCOSEL = RCC_PLL2VCOWIDE;
        PeriphClkInitStruct.PLL2.PLL2FRACN = 0;
        PeriphClkInitStruct.FdcanClockSelection = RCC_FDCANCLKSOURCE_PLL2;
        if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
        {
            Error_Handler(__FILE__, __LINE__);
        }

		__HAL_RCC_FDCAN_CLK_ENABLE();

		GPIO_InitStruct.Pin       = FDCAN2_TX_PIN;
		GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull      = GPIO_PULLUP;
		GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
		GPIO_InitStruct.Alternate = FDCAN2_TX_AF;
		HAL_GPIO_Init(FDCAN2_TX_GPIO_PORT, &GPIO_InitStruct);

		GPIO_InitStruct.Pin       = FDCAN2_RX_PIN;
		GPIO_InitStruct.Alternate = FDCAN2_RX_AF;
		HAL_GPIO_Init(FDCAN2_RX_GPIO_PORT, &GPIO_InitStruct);

		
		HAL_NVIC_SetPriority(FDCAN2_IT0_IRQn, 2, 0);
		HAL_NVIC_SetPriority(FDCAN2_IT1_IRQn, 2, 0);
		HAL_NVIC_SetPriority(FDCAN_CAL_IRQn, 2, 0);
		
		HAL_NVIC_EnableIRQ(FDCAN2_IT0_IRQn);
		HAL_NVIC_EnableIRQ(FDCAN2_IT1_IRQn);
		HAL_NVIC_EnableIRQ(FDCAN_CAL_IRQn);
	}	
}

/*
*********************************************************************************************************
*	�� �� ��: HAL_FDCAN_MspInit
*	����˵��: ����CAN gpio, �ָ�Ϊ��ͨGPIO��ȡ���ж�
*	��    ��: hfdcan
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void HAL_FDCAN_MspDeInit(FDCAN_HandleTypeDef* hfdcan)
{
	if (hfdcan == &hfdcan1)
	{	
		__HAL_RCC_FDCAN_FORCE_RESET();
		__HAL_RCC_FDCAN_RELEASE_RESET();

		HAL_GPIO_DeInit(FDCAN1_TX_GPIO_PORT, FDCAN1_TX_PIN);

		HAL_GPIO_DeInit(FDCAN1_RX_GPIO_PORT, FDCAN1_RX_PIN);

		HAL_NVIC_DisableIRQ(FDCAN1_IT0_IRQn);
		HAL_NVIC_DisableIRQ(FDCAN1_IT1_IRQn);
		HAL_NVIC_DisableIRQ(FDCAN_CAL_IRQn);
	}
	
	if (hfdcan == &hfdcan2)
	{	
		__HAL_RCC_FDCAN_FORCE_RESET();
		__HAL_RCC_FDCAN_RELEASE_RESET();

		HAL_GPIO_DeInit(FDCAN2_TX_GPIO_PORT, FDCAN2_TX_PIN);

		HAL_GPIO_DeInit(FDCAN2_RX_GPIO_PORT, FDCAN2_RX_PIN);

		HAL_NVIC_DisableIRQ(FDCAN2_IT0_IRQn);
		HAL_NVIC_DisableIRQ(FDCAN2_IT1_IRQn);
		HAL_NVIC_DisableIRQ(FDCAN_CAL_IRQn);
	}	
}

/*
*********************************************************************************************************
*	�� �� ��: HAL_FDCAN_RxFifo0Callback
*	����˵��: Rx FIFO0�ص�����
*	��    ��: hfdcan
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
	if (hfdcan == &hfdcan1)
	{
		if ((RxFifo0ITs & FDCAN_IT_RX_FIFO0_WATERMARK) != RESET)
		{
			/* ��RX FIFO0��ȡ���� */
			HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &g_Can1RxHeader, g_Can1RxData);

			/* ����ʹ��RX FIFO0��ֵ�ж� */
			HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_RX_FIFO0_WATERMARK, 0);

			/* ����Ϣ�յ����ݰ��������g_Can1RxHeader�� g_Can1RxData */
			bsp_PutMsg(MSG_CAN1_RxFIFO0, 0);	
		}
	}

	if (hfdcan == &hfdcan2)
	{
		if ((RxFifo0ITs & FDCAN_IT_RX_FIFO0_WATERMARK) != RESET)
		{
			/* ��RX FIFO0��ȡ���� */
			HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &g_Can2RxHeader, g_Can2RxData);

			/* ����ʹ��RX FIFO0��ֵ�ж� */
			HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_RX_FIFO0_WATERMARK, 0);
					
			/* ����Ϣ�յ����ݰ��������g_Can1RxHeader�� g_Can1RxData */
			bsp_PutMsg(MSG_CAN2_RxFIFO0, 0);	
		}
	}
}

/*
*********************************************************************************************************
*	�� �� ��: HAL_FDCAN_RxFifo0Callback
*	����˵��: Rx FIFO0�ص�����
*	��    ��: hfdcan
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void HAL_FDCAN_RxFifo1Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo1ITs)
{
	if (hfdcan == &hfdcan1)
	{
		if ((RxFifo1ITs & FDCAN_IT_RX_FIFO1_NEW_MESSAGE) != RESET)
		{
			/* ��RX FIFO1��ȡ���� */
			HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO1, &g_Can1RxHeader, g_Can1RxData);
			
			/* ʹ��Rx FIFO1�յ���������ֵ�ж� */
			HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_IT_RX_FIFO1_NEW_MESSAGE, 0);

			/* ����Ϣ�յ����ݰ��������g_Can1RxHeader�� g_Can1RxData */
			bsp_PutMsg(MSG_CAN1_RxFIFO1, 0);	

		}
	}

	if (hfdcan == &hfdcan2)
	{
		if ((RxFifo1ITs & FDCAN_IT_RX_FIFO1_NEW_MESSAGE) != RESET)
		{
			/* ��RX FIFO0��ȡ���� */
			HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO1, &g_Can2RxHeader, g_Can2RxData);
			
			/* ʹ��Rx FIFO1�յ���������ֵ�ж� */
			HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_IT_RX_FIFO1_NEW_MESSAGE, 0);
	
			/* ����Ϣ�յ����ݰ��������g_Can1RxHeader�� g_Can1RxData */
			bsp_PutMsg(MSG_CAN2_RxFIFO1, 0);	
		}
	}
}

/*
*********************************************************************************************************
*	�� �� ��: HAL_FDCAN_RxBufferNewMessageCallback
*	����˵��: Rx Buffer�ص�����
*	��    ��: hfdcan
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void HAL_FDCAN_RxBufferNewMessageCallback(FDCAN_HandleTypeDef *hfdcan)
{
	HAL_StatusTypeDef Status;
	uint8_t i, off;
	
	if (hfdcan == &hfdcan1)
	{
		for(i = 0; i < 32; i++)
		{
			off = (hfdcan->Instance->NDAT1 & (1<<i) ) >> i;
			if(off == 1)
			{
				/* ��RX Buffer0��ȡ����, iֵ��ͬ��FDCAN_RX_BUFFER */
				Status = HAL_FDCAN_GetRxMessage(hfdcan, i, &g_Can1RxHeader, g_Can1RxData);

				if(Status == HAL_OK)
				{
					/* ����Ϣ�յ����ݰ��������g_Can1RxHeader�� g_Can1RxData */
					bsp_PutMsg(MSG_CAN1_RxBuffer, 0);				
				}		
			}
		}
		
		for(i = 0; i < 32; i++)
		{
			off = (hfdcan->Instance->NDAT2 & (1<<i) ) >> i;
			if(off == 1)
			{
				/* ��RX Buffer0��ȡ����, iֵ��ͬ��FDCAN_RX_BUFFER */
				Status = HAL_FDCAN_GetRxMessage(hfdcan, i+31, &g_Can1RxHeader, g_Can1RxData);

				if(Status == HAL_OK)
				{
					/* ����Ϣ�յ����ݰ��������g_Can1RxHeader�� g_Can1RxData */		
					bsp_PutMsg(MSG_CAN1_RxBuffer, 0);		
				}		
			}
		}
	}

	if (hfdcan == &hfdcan2)
	{
		for(i = 0; i < 32; i++)
		{
			off = (hfdcan->Instance->NDAT1 & (1<<i) ) >> i;
			if(off == 1)
			{
				/* ��RX Buffer0��ȡ����, iֵ��ͬ��FDCAN_RX_BUFFER */
				Status = HAL_FDCAN_GetRxMessage(hfdcan, i, &g_Can1RxHeader, g_Can1RxData);

				if(Status == HAL_OK)
				{
					/* ����Ϣ�յ����ݰ��������g_Can1RxHeader�� g_Can1RxData */
					bsp_PutMsg(MSG_CAN2_RxBuffer, 0);				
				}		
			}
		}
		
		for(i = 0; i < 32; i++)
		{
			off = (hfdcan->Instance->NDAT2 & (1<<i) ) >> i;
			if(off == 1)
			{
				/* ��RX Buffer0��ȡ����, iֵ��ͬ��FDCAN_RX_BUFFER */
				Status = HAL_FDCAN_GetRxMessage(hfdcan, i+31, &g_Can1RxHeader, g_Can1RxData);

				if(Status == HAL_OK)
				{
					/* ����Ϣ�յ����ݰ��������g_Can1RxHeader�� g_Can1RxData */		
					bsp_PutMsg(MSG_CAN2_RxBuffer, 0);		
				}		
			}
		}
	}	
}

/*
*********************************************************************************************************
*	�� �� ��: HAL_FDCAN_TxFifoEmptyCallback
*	����˵��: Tx FIFO������ɻص�����
*	��    ��: hfdcan
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void HAL_FDCAN_TxFifoEmptyCallback(FDCAN_HandleTypeDef *hfdcan)
{
	if (hfdcan == &hfdcan1)
	{
		bsp_PutMsg(MSG_CAN1_TxFIFO, 0);
	}
	
	if (hfdcan == &hfdcan2)
	{
		bsp_PutMsg(MSG_CAN2_TxFIFO, 0);		
	}
}

/*
*********************************************************************************************************
*	�� �� ��: HAL_FDCAN_TxBufferCompleteCallback
*	����˵��: Tx Buffer������ɱ�־
*	��    ��: hfdcan
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void HAL_FDCAN_TxBufferCompleteCallback(FDCAN_HandleTypeDef *hfdcan, uint32_t BufferIndexes)
{
	if (hfdcan == &hfdcan1)
	{
		bsp_PutMsg(MSG_CAN1_TxBuffer, 0);
	}
	
	if (hfdcan == &hfdcan2)
	{
		bsp_PutMsg(MSG_CAN2_TxBuffer, 0);		
	}	
}

/*
*********************************************************************************************************
*	�� �� ��: can1_SendPacket
*	����˵��: ����һ������
*	��    �Σ�_DataBuf ���ݻ�����
*			  _Len ���ݳ���
*             FDCAN_DLC_BYTES_0  
*             FDCAN_DLC_BYTES_1  
*             FDCAN_DLC_BYTES_2   
*             FDCAN_DLC_BYTES_3   
*             FDCAN_DLC_BYTES_4 
*             FDCAN_DLC_BYTES_5 
*             FDCAN_DLC_BYTES_6 
*             FDCAN_DLC_BYTES_7 
*             FDCAN_DLC_BYTES_8 
*             FDCAN_DLC_BYTES_12
*             FDCAN_DLC_BYTES_16 
*             FDCAN_DLC_BYTES_20 
*             FDCAN_DLC_BYTES_24
*             FDCAN_DLC_BYTES_32 
*             FDCAN_DLC_BYTES_48
*             FDCAN_DLC_BYTES_64 
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void can1_SendPacket(uint8_t *_DataBuf, uint32_t _Len)
{		
	FDCAN_TxHeaderTypeDef TxHeader = {0};

	/* ��1�������÷��Ͳ��� ===================================================================*/
	TxHeader.Identifier = 0x001;             		     /* ���ý���֡��Ϣ��ID */
	TxHeader.IdType = FDCAN_STANDARD_ID;     		     /* ��׼ID */
	TxHeader.TxFrameType = FDCAN_DATA_FRAME;		     /* ����֡ */
	TxHeader.DataLength = _Len;      				     /* �������ݳ��� */
	TxHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;     /* ���ô���״ָ̬ʾ */
	TxHeader.BitRateSwitch = FDCAN_BRS_ON;               /* �����ɱ䲨���� */
	TxHeader.FDFormat = FDCAN_FD_CAN;                    /* FDCAN��ʽ */
	TxHeader.TxEventFifoControl = FDCAN_STORE_TX_EVENTS; /* ���ڷ����¼�FIFO����, ���洢 */
	TxHeader.MessageMarker = 1;                          /* ���ڸ��Ƶ�TX EVENT FIFO����ϢMaker��ʶ����Ϣ״̬����Χ0��0xFF */
	
#if 0 /* �򵥷��ͷ�ʽ */ 
	
	HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &TxHeader, _DataBuf); /* ������Ҫ�������޸�_DataBuf��ֵ */
	
#else /* ���ӷ��ͷ�ʽ���������ж� */
    /* ��2����������ݵ�TX FIFO, ����������� ================================================*/
    HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &TxHeader, _DataBuf); /* ������Ҫ�������޸�_DataBuf��ֵ */
	TxHeader.Identifier = 0x011;             		       		     
	TxHeader.MessageMarker = 2;   
	HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &TxHeader, _DataBuf); /* ������Ҫ�������޸�_DataBuf��ֵ */
	TxHeader.Identifier = 0x013;             		    
	TxHeader.MessageMarker = 3;   
	HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &TxHeader, _DataBuf); /* ������Ҫ�������޸�_DataBuf��ֵ */
	
	/* ʹ��Tx FIFO���Ϳ��ж� */
	HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_IT_TX_FIFO_EMPTY, 0);	
	
    /* ��2����������ݵ�TX BUFFER, ����������� ================================================*/
	TxHeader.Identifier = 0x12345678;             		    
	TxHeader.IdType = FDCAN_EXTENDED_ID;  
	TxHeader.MessageMarker = 4;
	HAL_FDCAN_AddMessageToTxBuffer(&hfdcan1,  &TxHeader, _DataBuf, FDCAN_TX_BUFFER0);/* ������Ҫ�������޸�_DataBuf��ֵ */
	TxHeader.Identifier = 0x12345681;             		    
	TxHeader.IdType = FDCAN_EXTENDED_ID;
	TxHeader.MessageMarker = 5;
	HAL_FDCAN_AddMessageToTxBuffer(&hfdcan1,  &TxHeader, _DataBuf, FDCAN_TX_BUFFER1);/* ������Ҫ�������޸�_DataBuf��ֵ */
	TxHeader.Identifier = 0x12345683;             		    
	TxHeader.IdType = FDCAN_EXTENDED_ID;
	TxHeader.MessageMarker = 6;
	HAL_FDCAN_AddMessageToTxBuffer(&hfdcan1,  &TxHeader, _DataBuf, FDCAN_TX_BUFFER2);/* ������Ҫ�������޸�_DataBuf��ֵ */	
	
	/* ʹ��Tx Buffer��������ж� */	
	HAL_FDCAN_EnableTxBufferRequest(&hfdcan1, FDCAN_TX_BUFFER0|FDCAN_TX_BUFFER1|FDCAN_TX_BUFFER2);
	HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_IT_TX_COMPLETE, FDCAN_TX_BUFFER0|FDCAN_TX_BUFFER1|FDCAN_TX_BUFFER2);
#endif
}

/*
*********************************************************************************************************
*	�� �� ��: can2_SendPacket
*	����˵��: ����һ������
*	��    �Σ�_DataBuf ���ݻ�����
*			  _Len ���ݳ���
*             FDCAN_DLC_BYTES_0  
*             FDCAN_DLC_BYTES_1  
*             FDCAN_DLC_BYTES_2   
*             FDCAN_DLC_BYTES_3   
*             FDCAN_DLC_BYTES_4 
*             FDCAN_DLC_BYTES_5 
*             FDCAN_DLC_BYTES_6 
*             FDCAN_DLC_BYTES_7 
*             FDCAN_DLC_BYTES_8 
*             FDCAN_DLC_BYTES_12
*             FDCAN_DLC_BYTES_16 
*             FDCAN_DLC_BYTES_20 
*             FDCAN_DLC_BYTES_24
*             FDCAN_DLC_BYTES_32 
*             FDCAN_DLC_BYTES_48
*             FDCAN_DLC_BYTES_64 
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void can2_SendPacket(uint8_t *_DataBuf, uint32_t _Len)
{		
	FDCAN_TxHeaderTypeDef TxHeader = {0};

	/* ��1�������÷��Ͳ��� ===================================================================*/
	TxHeader.Identifier = 0x001;             		     /* ���ý���֡��Ϣ��ID */
	TxHeader.IdType = FDCAN_STANDARD_ID;     		     /* ��׼ID */
	TxHeader.TxFrameType = FDCAN_DATA_FRAME;		     /* ����֡ */
	TxHeader.DataLength = _Len;      				     /* �������ݳ��� */
	TxHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;     /* ���ô���״ָ̬ʾ */
	TxHeader.BitRateSwitch = FDCAN_BRS_ON;               /* �����ɱ䲨���� */
	TxHeader.FDFormat = FDCAN_FD_CAN;                    /* FDCAN��ʽ */
	TxHeader.TxEventFifoControl = FDCAN_STORE_TX_EVENTS; /* ���ڷ����¼�FIFO����, ���洢 */
	TxHeader.MessageMarker = 1;                          /* ���ڸ��Ƶ�TX EVENT FIFO����ϢMaker��ʶ����Ϣ״̬����Χ0��0xFF */
	
#if 0 /* �򵥷��ͷ�ʽ */ 
	
	HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &TxHeader, _DataBuf); /* ������Ҫ�������޸�_DataBuf��ֵ */
	
#else /* ���ӷ��ͷ�ʽ���������ж� */
    /* ��2����������ݵ�TX FIFO, ����������� ================================================*/
    HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan2, &TxHeader, _DataBuf); /* ������Ҫ�������޸�_DataBuf��ֵ */
	TxHeader.Identifier = 0x011;             		       		     
	TxHeader.MessageMarker = 2;   
	HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan2, &TxHeader, _DataBuf); /* ������Ҫ�������޸�_DataBuf��ֵ */
	TxHeader.Identifier = 0x013;             		    
	TxHeader.MessageMarker = 3;   
	HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan2, &TxHeader, _DataBuf); /* ������Ҫ�������޸�_DataBuf��ֵ */
	
	/* ʹ��Tx FIFO���Ϳ��ж� */
	HAL_FDCAN_ActivateNotification(&hfdcan2, FDCAN_IT_TX_FIFO_EMPTY, 0);	
	
    /* ��2����������ݵ�TX BUFFER, ����������� ================================================*/
	TxHeader.Identifier = 0x12345678;             		    
	TxHeader.IdType = FDCAN_EXTENDED_ID;  
	TxHeader.MessageMarker = 4;
	HAL_FDCAN_AddMessageToTxBuffer(&hfdcan2,  &TxHeader, _DataBuf, FDCAN_TX_BUFFER0);/* ������Ҫ�������޸�_DataBuf��ֵ */
	TxHeader.Identifier = 0x12345681;             		    
	TxHeader.IdType = FDCAN_EXTENDED_ID;
	TxHeader.MessageMarker = 5;
	HAL_FDCAN_AddMessageToTxBuffer(&hfdcan2,  &TxHeader, _DataBuf, FDCAN_TX_BUFFER1);/* ������Ҫ�������޸�_DataBuf��ֵ */
	TxHeader.Identifier = 0x12345683;             		    
	TxHeader.IdType = FDCAN_EXTENDED_ID;
	TxHeader.MessageMarker = 6;
	HAL_FDCAN_AddMessageToTxBuffer(&hfdcan2,  &TxHeader, _DataBuf, FDCAN_TX_BUFFER2);/* ������Ҫ�������޸�_DataBuf��ֵ */	
	
	/* ʹ��Tx Buffer��������ж� */	
	HAL_FDCAN_EnableTxBufferRequest(&hfdcan2, FDCAN_TX_BUFFER0|FDCAN_TX_BUFFER1|FDCAN_TX_BUFFER2);
	HAL_FDCAN_ActivateNotification(&hfdcan2, FDCAN_IT_TX_COMPLETE, FDCAN_TX_BUFFER0|FDCAN_TX_BUFFER1|FDCAN_TX_BUFFER2);
#endif
}

/*
*********************************************************************************************************
*	�� �� ��: FDCAN1_IT0_IRQHandler
*	����˵��: CAN�жϷ������
*	��    ��: hfdcan
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void FDCAN1_IT0_IRQHandler(void)
{
	HAL_FDCAN_IRQHandler(&hfdcan1);
}

void FDCAN2_IT0_IRQHandler(void)
{
	HAL_FDCAN_IRQHandler(&hfdcan2);
}

void FDCAN1_IT1_IRQHandler(void)
{
	HAL_FDCAN_IRQHandler(&hfdcan1);
}

void FDCAN2_IT1_IRQHandler(void)
{
	HAL_FDCAN_IRQHandler(&hfdcan2);
}

void FDCAN_CAL_IRQHandler(void)
{
	HAL_FDCAN_IRQHandler(&hfdcan1);
	HAL_FDCAN_IRQHandler(&hfdcan2);
}

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
