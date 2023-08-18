/*
*********************************************************************************************************
*
*	ģ������ : FDCAN����ģ��
*	�ļ����� : bsp_can.c
*	��    �� : V1.1
*	˵    �� : CAN����. 
*
*	�޸ļ�¼ :
*		�汾��  ����        ����     ˵��
*		V1.0    2018-11-14  armfly  ��ʽ����
*		V1.1    2021-01-23  armfly  ����
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
uint8_t g_Can1RxData[8];

FDCAN_RxHeaderTypeDef g_Can2RxHeader;
uint8_t g_Can2RxData[8];
	
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
	/*                    λʱ����������
		Bit time parameter         | Nominal      |  Data
		---------------------------|--------------|----------------
		fdcan_ker_ck               | 20 MHz       | 20 MHz
		Time_quantum (tq)          | 50 ns        | 50 ns
		Synchronization_segment    | 1 tq         | 1 tq
		Propagation_segment        | 23 tq        | 1 tq
		Phase_segment_1            | 8 tq         | 4 tq
		Phase_segment_2            | 8 tq         | 4 tq
		Synchronization_Jump_width | 8 tq         | 4 tq
		Bit_length                 | 40 tq = 2us  | 10 tq = 0.5us
		Bit_rate                   | 0.5 MBit/s   | 1 MBit/s
	*/
	hfdcan1.Instance = FDCAN1;                     /* ����FDCAN1 */             
	hfdcan1.Init.FrameFormat = FDCAN_FRAME_FD_BRS; /* ����ʹ��FDCAN�ɱ䲨���� */  
	hfdcan1.Init.Mode = FDCAN_MODE_NORMAL;         /* ����ʹ������ģʽ */ 
	hfdcan1.Init.AutoRetransmission = ENABLE;      /*ʹ���Զ��ط� */ 
	hfdcan1.Init.TransmitPause = DISABLE;          /* ���ý�ֹ������ͣ���� */
	hfdcan1.Init.ProtocolException = ENABLE;       /* Э���쳣����ʹ�� */
	
	/* 
		�����ٲý׶β����� 
		CANʱ��20MHzʱ���ٲý׶εĲ����ʾ���
		CAN FD Freq / (Sync_Seg + Pro_Seg + Phase_Seg1 + Phase_Seg2) = 20MHz / ��1+0x1F + 8�� = 0.5Mbps	
		
		����Sync_Seg�ǹ̶�ֵ = 1 �� Pro_Seg + Phase_Seg1 = NominalTimeSeg1�� Phase_Seg2 = NominalTimeSeg2
	*/
	hfdcan1.Init.NominalPrescaler = 0x01; 	  /* CANʱ�ӷ������ã�һ������Ϊ1���ɣ�ȫ����PLL���úã�tq = NominalPrescaler x (1/ fdcan_ker_ck) */
	hfdcan1.Init.NominalSyncJumpWidth = 0x08; /* ���ڶ�̬����  Phase_Seg1�� Phase_Seg1�����Բ����Ա�Phase_Seg1�� Phase_Seg1�� */
	hfdcan1.Init.NominalTimeSeg1 = 0x1F; 	  /* �ر�ע�������Seg1����������������֮�ͣ���Ӧλʱ������ͼ�� Pro_Seg + Phase_Seg1 */
	hfdcan1.Init.NominalTimeSeg2 = 0x08;      /* ��Ӧλʱ������ͼ�� Phase_Seg2 */


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
	
	
	hfdcan1.Init.MessageRAMOffset = 0;      /* CAN1��CAN2����2560����, ����CAN1����ǰ1280�� */
	
	
	hfdcan1.Init.StdFiltersNbr = 1;         			/* ���ñ�׼ID��������������Χ0-128 */       
	hfdcan1.Init.ExtFiltersNbr = 0;         			/* ������չID��������������Χ0-64 */   
	hfdcan1.Init.RxFifo0ElmtsNbr = 2;                   /* ����Rx FIFO0��Ԫ�ظ�������Χ0-64 */  
	hfdcan1.Init.RxFifo0ElmtSize = FDCAN_DATA_BYTES_8; 	/* ����Rx FIFO0��ÿ��Ԫ�ش�С��֧��8,12,16,20,24,32,48����64�ֽ� */   
	hfdcan1.Init.RxFifo1ElmtsNbr = 0;                   /* ����Rx FIFO1��Ԫ�ظ�������Χ0-64 */
	hfdcan1.Init.RxFifo1ElmtSize = FDCAN_DATA_BYTES_8;  /* ����Rx FIFO1��ÿ��Ԫ�ش�С��֧��8,12,16,20,24,32,48����64�ֽ� */	
	hfdcan1.Init.RxBuffersNbr = 0;                      /* ����Rx Buffer��������Χ0-64 */
	hfdcan1.Init.RxBufferSize = 0;                      /* ����Rx Buffer��ÿ��Ԫ�ش�С��֧��8,12,16,20,24,32,48����64�ֽ� */	


	hfdcan1.Init.TxEventsNbr = 0;							/* ����Tx Event FIFO��Ԫ�ظ�������Χ0-32 */	
	hfdcan1.Init.TxBuffersNbr = 0;							/* ����Tx Buffer��Ԫ�ظ�������Χ0-32 */
	hfdcan1.Init.TxFifoQueueElmtsNbr = 2;                   /* ��������Tx FIFO/Queue��Tx Buffers��������Χ0��32 */
	hfdcan1.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION; /* ����FIFOģʽ����QUEUE����ģʽ */
	hfdcan1.Init.TxElmtSize = FDCAN_DATA_BYTES_8;           /* ����Tx Element�е��������С��֧��8,12,16,20,24,32,48����64�ֽ� */
	HAL_FDCAN_Init(&hfdcan1);


	/* 
		���ù�����, ��������Ҫ���ڽ��գ������������λģʽ��
		FilterID1 = filter
		FilterID2 = mask
		
		FilterID2��maskÿ��bit����
		0: �����ģ���λ�����ڱȽϣ�
		1: ����ƥ�䣬���յ���ID�������˲�����Ӧ��IDλ��һ�¡�
		
		����˵����
		FilterID1 = 0x111
		FilterID2 = 0x7FF 
		��ʾ������IDΪ0x111��FDCAN֡��
		
	*/
	sFilterConfig1.IdType = FDCAN_STANDARD_ID;              /* ���ñ�׼ID������չID */
	sFilterConfig1.FilterIndex = 0;   						/* ���ڹ�������������Ǳ�׼ID����Χ0��127���������չID����Χ0��64 */
	sFilterConfig1.FilterType = FDCAN_FILTER_MASK;          /* ��������������λģʽ */
	sFilterConfig1.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;  /* �������ƥ�䣬�����ݱ��浽Rx FIFO 0 */
	sFilterConfig1.FilterID1 = 0x111;                       /* ����λģʽ�£�FilterID1����ϢID */
	sFilterConfig1.FilterID2 = 0x7FF; 						/* ����λģʽ�£�FilterID2����Ϣ����λ */
	HAL_FDCAN_ConfigFilter(&hfdcan1, &sFilterConfig1);      /* ���ù����� */
	
	/* ����Rx FIFO0��wartermarkΪ1 */
	HAL_FDCAN_ConfigFifoWatermark(&hfdcan1, FDCAN_CFG_RX_FIFO0, 1);

	/* ����RX FIFO0��watermark֪ͨ�жϣ�λ����Tx Buffer�ж�*/
	HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_IT_RX_FIFO0_WATERMARK, 0);

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
	/*                    λʱ����������
		Bit time parameter         | Nominal      |  Data
		---------------------------|--------------|----------------
		fdcan_ker_ck               | 20 MHz       | 20 MHz
		Time_quantum (tq)          | 50 ns        | 50 ns
		Synchronization_segment    | 1 tq         | 1 tq
		Propagation_segment        | 23 tq        | 1 tq
		Phase_segment_1            | 8 tq         | 4 tq
		Phase_segment_2            | 8 tq         | 4 tq
		Synchronization_Jump_width | 8 tq         | 4 tq
		Bit_length                 | 40 tq = 2us  | 10 tq = 0.5us
		Bit_rate                   | 0.5 MBit/s   | 1 MBit/s
	*/
	hfdcan2.Instance = FDCAN2;                     /* ����FDCAN2 */             
	hfdcan2.Init.FrameFormat = FDCAN_FRAME_FD_BRS; /* ����ʹ��FDCAN�ɱ䲨���� */  
	hfdcan2.Init.Mode = FDCAN_MODE_NORMAL;         /* ����ʹ������ģʽ */ 
	hfdcan2.Init.AutoRetransmission = ENABLE;      /*ʹ���Զ��ط� */ 
	hfdcan2.Init.TransmitPause = DISABLE;          /* ���ý�ֹ������ͣ���� */
	hfdcan2.Init.ProtocolException = ENABLE;       /* Э���쳣����ʹ�� */
	
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
	
	 
	hfdcan2.Init.MessageRAMOffset = 500;   /* CAN1��CAN2����2560����, ����CAN2�����1280�� */
	
	hfdcan2.Init.StdFiltersNbr = 1;         			/* ���ñ�׼ID��������������Χ0-128 */       
	hfdcan2.Init.ExtFiltersNbr = 0;         			/* ������չID��������������Χ0-64 */   
	hfdcan2.Init.RxFifo0ElmtsNbr = 2;                   /* ����Rx FIFO0��Ԫ�ظ�������Χ0-64 */  
	hfdcan2.Init.RxFifo0ElmtSize = FDCAN_DATA_BYTES_8; 	/* ����Rx FIFO0��ÿ��Ԫ�ش�С��֧��8,12,16,20,24,32,48����64�ֽ� */   
	hfdcan2.Init.RxFifo1ElmtsNbr = 0;                   /* ����Rx FIFO1��Ԫ�ظ�������Χ0-64 */
	hfdcan2.Init.RxFifo1ElmtSize = FDCAN_DATA_BYTES_8;  /* ����Rx FIFO1��ÿ��Ԫ�ش�С��֧��8,12,16,20,24,32,48����64�ֽ� */	
	hfdcan2.Init.RxBuffersNbr = 0;                      /* ����Rx Buffer��������Χ0-64 */
	hfdcan2.Init.RxBufferSize = 0;                      /* ����Rx Buffer��ÿ��Ԫ�ش�С��֧��8,12,16,20,24,32,48����64�ֽ� */	


	hfdcan2.Init.TxEventsNbr = 0;							/* ����Tx Event FIFO��Ԫ�ظ�������Χ0-32 */	
	hfdcan2.Init.TxBuffersNbr = 0;							/* ����Tx Buffer��Ԫ�ظ�������Χ0-32 */
	hfdcan2.Init.TxFifoQueueElmtsNbr = 2;                   /* ��������Tx FIFO/Queue��Tx Buffers��������Χ0��32 */
	hfdcan2.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION; /* ����FIFOģʽ����QUEUE����ģʽ */
	hfdcan2.Init.TxElmtSize = FDCAN_DATA_BYTES_8;           /* ����Tx Element�е��������С��֧��8,12,16,20,24,32,48����64�ֽ� */
	HAL_FDCAN_Init(&hfdcan2);

	/* 
		���ù�����, ��������Ҫ���ڽ��գ������������λģʽ��
		FilterID1 = filter
		FilterID2 = mask
		
		FilterID2��maskÿ��bit����
		0: �����ģ���λ�����ڱȽϣ�
		1: ����ƥ�䣬���յ���ID�������˲�����Ӧ��IDλ��һ�¡�
		
		����˵����
		FilterID1 = 0x222
		FilterID2 = 0x7FF 
		��ʾ������IDΪ0x222��FDCAN֡��
		
	*/
	sFilterConfig2.IdType = FDCAN_STANDARD_ID;              /* ���ñ�׼ID������չID */
	sFilterConfig2.FilterIndex = 0;   						/* ���ڹ�������������Ǳ�׼ID����Χ0��127���������չID����Χ0��64 */
	sFilterConfig2.FilterType = FDCAN_FILTER_MASK;          /* ��������������λģʽ */
	sFilterConfig2.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;  /* �������ƥ�䣬�����ݱ��浽Rx FIFO 0 */
	sFilterConfig2.FilterID1 = 0x222;                       /* ����λģʽ�£�FilterID1����ϢID */
	sFilterConfig2.FilterID2 = 0x7FF; 						/* ����λģʽ�£�FilterID2����Ϣ����λ */
	HAL_FDCAN_ConfigFilter(&hfdcan2, &sFilterConfig2);      /* ���ù����� */
	
	/* ����Rx FIFO0��wartermarkΪ1 */
	HAL_FDCAN_ConfigFifoWatermark(&hfdcan2, FDCAN_CFG_RX_FIFO0, 1);

	/* ����RX FIFO0��watermark֪ͨ�жϣ�λ����Tx Buffer�ж�*/
	HAL_FDCAN_ActivateNotification(&hfdcan2, FDCAN_IT_RX_FIFO0_WATERMARK, 0);

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
*	����˵��: CAN�жϷ������-�ص�����
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

			/* ����Rx FIFO0 watermark notification */
			HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_RX_FIFO0_WATERMARK, 0);
			
			if (g_Can1RxHeader.Identifier == 0x111 && g_Can1RxHeader.IdType == FDCAN_STANDARD_ID)
			{
				bsp_PutMsg(MSG_CAN1_RX, 0);	/* ����Ϣ�յ����ݰ��������g_Can1RxHeader�� g_Can1RxData */
			}
		}
	}

	if (hfdcan == &hfdcan2)
	{
		if ((RxFifo0ITs & FDCAN_IT_RX_FIFO0_WATERMARK) != RESET)
		{
			/* ��RX FIFO0��ȡ���� */
			HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &g_Can2RxHeader, g_Can2RxData);

			/* ����Rx FIFO0 watermark notification */
			HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_RX_FIFO0_WATERMARK, 0);
			
			if (g_Can2RxHeader.Identifier == 0x222 && g_Can2RxHeader.IdType == FDCAN_STANDARD_ID)
			{			
				bsp_PutMsg(MSG_CAN2_RX, 0);	/* ����Ϣ�յ����ݰ��������g_Can1RxHeader�� g_Can1RxData */
			}
		}
	}
}

/*
*********************************************************************************************************
*	�� �� ��: can1_SendPacket
*	����˵��: ����һ������
*	��    �Σ�_DataBuf ���ݻ�����
*			  _Len ���ݳ���, ֧��8,12,16,20,24,32,48����64�ֽ�
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void can1_SendPacket(uint8_t *_DataBuf, uint8_t _Len)
{		
	FDCAN_TxHeaderTypeDef TxHeader = {0};

	
	/* ���÷��Ͳ��� */
	TxHeader.Identifier = 0x222;             		 /* ���ý���֡��Ϣ��ID */
	TxHeader.IdType = FDCAN_STANDARD_ID;     		 /* ��׼ID */
	TxHeader.TxFrameType = FDCAN_DATA_FRAME;		 /* ����֡ */
	TxHeader.DataLength = (uint32_t)_Len << 16;      /* �������ݳ��� */
	TxHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE; /* ���ô���״ָ̬ʾ */
	TxHeader.BitRateSwitch = FDCAN_BRS_ON;           /* �����ɱ䲨���� */
	TxHeader.FDFormat = FDCAN_FD_CAN;                /* FDCAN��ʽ */
	TxHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;/* ���ڷ����¼�FIFO����, ���洢 */
	TxHeader.MessageMarker = 0;                      /* ���ڸ��Ƶ�TX EVENT FIFO����ϢMaker��ʶ����Ϣ״̬����Χ0��0xFF */
	
    /* ������ݵ�TX FIFO */
    HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &TxHeader, _DataBuf);
}

/*
*********************************************************************************************************
*	�� �� ��: can2_SendPacket
*	����˵��: ����һ������
*	��    �Σ�_DataBuf ���ݻ�����
*			  _Len ���ݳ���, ֧��8,12,16,20,24,32,48����64�ֽ�
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void can2_SendPacket(uint8_t *_DataBuf, uint8_t _Len)
{		
	FDCAN_TxHeaderTypeDef TxHeader = {0};
	
	/* ���÷��Ͳ��� */
	TxHeader.Identifier = 0x111;            		/* ���ý���֡��Ϣ��ID */
	TxHeader.IdType = FDCAN_STANDARD_ID;			/* ��׼ID */
	TxHeader.TxFrameType = FDCAN_DATA_FRAME;		/* ����֡ */
	TxHeader.DataLength = (uint32_t)_Len << 16;		/* �������ݳ��� */
	TxHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;/* ���ô���״ָ̬ʾ */
	TxHeader.BitRateSwitch = FDCAN_BRS_ON;			/* �����ɱ䲨���� */
	TxHeader.FDFormat = FDCAN_FD_CAN;				/* FDCAN��ʽ */
	TxHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;/* ���ڷ����¼�FIFO����, ���洢 */
	TxHeader.MessageMarker = 0;
	
    /* ������ݵ�TX */
    HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan2, &TxHeader, _DataBuf);
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
