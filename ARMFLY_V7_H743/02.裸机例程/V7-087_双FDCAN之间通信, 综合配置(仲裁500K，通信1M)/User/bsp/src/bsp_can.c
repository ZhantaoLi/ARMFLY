/*
*********************************************************************************************************
*
*	模块名称 : FDCAN驱动模块
*	文件名称 : bsp_can.c
*	版    本 : V1.1
*	说    明 : CAN驱动. 
*
*	修改记录 :
*		版本号  日期        作者      说明
*		V1.0    2018-11-14  armfly   正式发布
*		V1.1    2021-01-23  armfly   升级
*		V1.2    2023-05-12  Eric2013 升级
*
*	Copyright (C), 2021-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#include "bsp.h"



/*
	启用CAN1，需要将V7主板上的J12跳线帽短接PA11，J13跳线帽短接PA12。
	启用CNA2，硬件无需跳线，以太网功能需要屏蔽（有引脚复用）。
*/

/*
*********************************************************************************************************
*                                             FDCAN1配置
*********************************************************************************************************
*/
/* FDCAN1 GPIO定义 */
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
*                                             FDCAN2配置
*********************************************************************************************************
*/
/* FDCAN1 GPIO定义 */
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
*	函 数 名: bsp_InitCan1
*	功能说明: 初始CAN1
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_InitCan1(void)
{	 
	/* 第1步：基础配置 ==================================================================================*/
	hfdcan1.Instance = FDCAN1;                     /* 配置FDCAN1 */             
	hfdcan1.Init.FrameFormat = FDCAN_FRAME_FD_BRS; /* 配置使用FDCAN可变波特率 */  
	hfdcan1.Init.Mode = FDCAN_MODE_NORMAL;         /* 配置使用正常模式 */ 
	hfdcan1.Init.AutoRetransmission = ENABLE;      /* 使能自动重发 */ 
	hfdcan1.Init.TransmitPause = DISABLE;          /* 配置禁止传输暂停特性 */
	hfdcan1.Init.ProtocolException = ENABLE;       /* 协议异常处理使能 */
	
	/* 第2步：波特率配置 ==================================================================================*/
	/* 
		配置仲裁阶段波特率 
		CAN时钟20MHz时，仲裁阶段的波特率就是
		CAN FD Freq / (Sync_Seg + Pro_Seg + Phase_Seg1 + Phase_Seg2) = 20MHz / （1+0x1F + 8） = 0.5Mbps	
		
		其中Sync_Seg是固定值 = 1 ， Pro_Seg + Phase_Seg1 = NominalTimeSeg1， Phase_Seg2 = NominalTimeSeg2
	*/
	hfdcan1.Init.NominalPrescaler = 0x01; 	  /* CAN时钟分配设置，一般设置为1即可，全部由PLL配置好，tq = NominalPrescaler x (1/ fdcan_ker_ck), 范围1-512 */
	hfdcan1.Init.NominalSyncJumpWidth = 0x08; /* 用于动态调节  Phase_Seg1和 Phase_Seg1，所以不可以比Phase_Seg1和 Phase_Seg1大，范围1-128 */
	hfdcan1.Init.NominalTimeSeg1 = 0x1F; 	  /* 特别注意这里的Seg1，这里是两个参数之和，对应位时间特性图的 Pro_Seg + Phase_Seg1，范围2-256 */
	hfdcan1.Init.NominalTimeSeg2 = 0x08;      /* 对应位时间特性图的 Phase_Seg2，范围2- 128 */


	/* 
		配置数据阶段波特率 
		CAN时钟20MHz时，数据阶段的波特率就是
		CAN FD Freq / (Sync_Seg + Pro_Seg + Phase_Seg1 + Phase_Seg2) = 20MHz / 2 / （1+5+ 4） = 1Mbps
		
		其中Sync_Seg是固定值 = 1 ， Pro_Seg + Phase_Seg1 = DataTimeSeg1， Phase_Seg2 = DataTimeSeg2
	*/
	hfdcan1.Init.DataPrescaler = 0x02;      /* CAN时钟分配设置，一般设置为1即可，全部由PLL配置好，tq = NominalPrescaler x (1/ fdcan_ker_ck)，范围1-32 */
	hfdcan1.Init.DataSyncJumpWidth = 0x04;  /* 用于动态调节  Phase_Seg1和 Phase_Seg1，所以不可以比Phase_Seg1和 Phase_Seg1大，范围1-16 */
	hfdcan1.Init.DataTimeSeg1 = 0x05; 		/* 特别注意这里的Seg1，这里是两个参数之和，对应位时间特性图的 Pro_Seg + Phase_Seg1，范围 */
	hfdcan1.Init.DataTimeSeg2 = 0x04;       /* 对应位时间特性图的 Phase_Seg2 */    
	
	/* 第3步：RAM管理器配置 ==================================================================================*/
	hfdcan1.Init.MessageRAMOffset = 0;      			/* CAN1和CAN2共享2560个字, 这里CAN1分配前1280字 */
	hfdcan1.Init.StdFiltersNbr = 4;         			/* 设置标准ID过滤器个数，范围0-128 */       
	hfdcan1.Init.ExtFiltersNbr = 4;         			/* 设置扩展ID过滤器个数，范围0-64 */   
	hfdcan1.Init.RxFifo0ElmtsNbr = 3;                   /* 设置Rx FIFO0的元素个数，范围0-64 */  
	hfdcan1.Init.RxFifo0ElmtSize = FDCAN_DATA_BYTES_8; 	/* 设置Rx FIFO0中每个元素大小，支持8,12,16,20,24,32,48或者64字节 */   
	hfdcan1.Init.RxFifo1ElmtsNbr = 3;                   /* 设置Rx FIFO1的元素个数，范围0-64 */
	hfdcan1.Init.RxFifo1ElmtSize = FDCAN_DATA_BYTES_8;  /* 设置Rx FIFO1中每个元素大小，支持8,12,16,20,24,32,48或者64字节 */	
	hfdcan1.Init.RxBuffersNbr = 2;                      /* 设置Rx Buffer个数，范围0-64 */
	hfdcan1.Init.RxBufferSize = FDCAN_DATA_BYTES_8;     /* 设置Rx Buffer中每个元素大小，支持8,12,16,20,24,32,48或者64字节 */	

	hfdcan1.Init.TxEventsNbr = 16;							/* 设置Tx Event FIFO中元素个数，范围0-32 */	
	hfdcan1.Init.TxBuffersNbr = 8;							/* 设置Tx Buffer中元素个数，范围0-32 */
	hfdcan1.Init.TxFifoQueueElmtsNbr = 8;                   /* 设置用于Tx FIFO/Queue的Tx Buffers个数。范围0到32 */
	hfdcan1.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION; /* 设置FIFO模式或者QUEUE队列模式 */
	hfdcan1.Init.TxElmtSize = FDCAN_DATA_BYTES_8;           /* 设置Tx Element中的数据域大小，支持8,12,16,20,24,32,48或者64字节 */
	HAL_FDCAN_Init(&hfdcan1);

	/* 
		配置过滤器, 过滤器主要用于接收，比如屏蔽位模式。
		FilterID1 = filter
		FilterID2 = mask
		
		FilterID2的mask每个bit含义
		0: 不关心，该位不用于比较；
		1: 必须匹配，接收到的ID必须与滤波器对应的ID位相一致。
		
		举例说明：
		FilterID1 = 0x111  0001 0001 0001 
		FilterID2 = 0x7FF  0111 1111 1111
		表示仅接收ID为0x111的FDCAN帧。
	*/
	
	/* 第4步：扩展ID过滤设置 ==================================================================================*/
	sFilterConfig1.IdType = FDCAN_EXTENDED_ID;              /* 扩展ID */
	sFilterConfig1.FilterIndex = 0;   						/* 用于过滤索引，扩展ID，范围0到64 */
	sFilterConfig1.FilterType = FDCAN_FILTER_RANGE;         /* 范围过滤 -----------------*/
	sFilterConfig1.FilterConfig = FDCAN_FILTER_TO_RXFIFO1;  /* 如果过滤匹配，将数据保存到Rx FIFO 1 */
	sFilterConfig1.FilterID1 = 0x12345678;                  /* 起始ID:0x12345678 */
	sFilterConfig1.FilterID2 = 0x12345680; 					/* 结束ID:0x12345680 */
	HAL_FDCAN_ConfigFilter(&hfdcan1, &sFilterConfig1);      /* 配置过滤器 */
	
	sFilterConfig1.IdType = FDCAN_EXTENDED_ID;              /* 扩展ID */
	sFilterConfig1.FilterIndex = 1;   						/* 用于过滤索引，扩展ID，范围0到64 */
	sFilterConfig1.FilterType = FDCAN_FILTER_DUAL;          /* 专用ID过滤 --------------*/
	sFilterConfig1.FilterConfig = FDCAN_FILTER_TO_RXFIFO1;  /* 如果过滤匹配，将数据保存到Rx FIFO 1 */
	sFilterConfig1.FilterID1 = 0x12345681;                  /* 指定ID */
	sFilterConfig1.FilterID2 = 0x12345682; 					/* 指定ID */
	HAL_FDCAN_ConfigFilter(&hfdcan1, &sFilterConfig1);      /* 配置过滤器 */
	
	sFilterConfig1.IdType = FDCAN_EXTENDED_ID;              /* 扩展ID */
	sFilterConfig1.FilterIndex = 2;   						/* 用于过滤索引，扩展ID，范围0到64 */
	sFilterConfig1.FilterType = FDCAN_FILTER_MASK;          /* 过滤器采样屏蔽位模式 ----*/
	sFilterConfig1.FilterConfig = FDCAN_FILTER_TO_RXFIFO1;  /* 如果过滤匹配，将数据保存到Rx FIFO 1 */
	sFilterConfig1.FilterID1 = 0x12345683;                  /* 屏蔽位模式下，FilterID1是消息ID */
	sFilterConfig1.FilterID2 = 0x1FFFFFFF; 					/* 屏蔽位模式下，FilterID2是消息屏蔽位 */
	HAL_FDCAN_ConfigFilter(&hfdcan1, &sFilterConfig1);      /* 配置过滤器 */
	
	sFilterConfig1.IdType = FDCAN_EXTENDED_ID;              /* 扩展ID */
	sFilterConfig1.FilterIndex = 3;   						/* 用于过滤索引，扩展ID，范围0到64 */
	sFilterConfig1.FilterType = FDCAN_FILTER_MASK;          /* 忽略 --------------------*/
	sFilterConfig1.FilterConfig = FDCAN_FILTER_TO_RXBUFFER; /* 如果过滤匹配，将数据保存到Rx Buffer */
	sFilterConfig1.FilterID1 = 0x12345684;                  /* 接收指定ID消息 */
	sFilterConfig1.FilterID2 = 0x1FFFFFFF; 					/* 忽略 */
	sFilterConfig1.IsCalibrationMsg = 0;                    /* 正常消息 */
	sFilterConfig1.RxBufferIndex = 0;                       /* Rx buffer位置 */
	HAL_FDCAN_ConfigFilter(&hfdcan1, &sFilterConfig1);      /* 配置过滤器 */

	/* 第5步：标准ID过滤设置 ==================================================================================*/	
	sFilterConfig1.IdType = FDCAN_STANDARD_ID;              /* 设置标准ID或者扩展ID */
	sFilterConfig1.FilterIndex = 0;   						/* 用于过滤索引，标准ID，范围0到127 */
	sFilterConfig1.FilterType = FDCAN_FILTER_RANGE;         /* 范围过滤 -------------------------*/
	sFilterConfig1.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;  /* 如果过滤匹配，将数据保存到Rx FIFO 0 */
	sFilterConfig1.FilterID1 = 0x000;                       /* 起始ID:0x000 */
	sFilterConfig1.FilterID2 = 0x010; 						/* 结束ID:0x010 */
	HAL_FDCAN_ConfigFilter(&hfdcan1, &sFilterConfig1);      /* 配置过滤器 */
	
	sFilterConfig1.IdType = FDCAN_STANDARD_ID;              /* 设置标准ID或者扩展ID */
	sFilterConfig1.FilterIndex = 1;   						/* 用于过滤索引，标准ID，范围0到127 */
	sFilterConfig1.FilterType = FDCAN_FILTER_DUAL;          /* 专用ID过滤 ----------------------*/
	sFilterConfig1.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;  /* 如果过滤匹配，将数据保存到Rx FIFO 0 */
	sFilterConfig1.FilterID1 = 0x011;                       /* 指定ID */
	sFilterConfig1.FilterID2 = 0x012; 						/* 指定ID */
	HAL_FDCAN_ConfigFilter(&hfdcan1, &sFilterConfig1);      /* 配置过滤器 */
	
	sFilterConfig1.IdType = FDCAN_STANDARD_ID;              /* 设置标准ID或者扩展ID */
	sFilterConfig1.FilterIndex = 2;   						/* 用于过滤索引，标准ID，范围0到127 */
	sFilterConfig1.FilterType = FDCAN_FILTER_MASK;          /* 过滤器采样屏蔽位模式 -----------*/
	sFilterConfig1.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;  /* 如果过滤匹配，将数据保存到Rx FIFO 0 */
	sFilterConfig1.FilterID1 = 0x013;                       /* 屏蔽位模式下，FilterID1是消息ID */
	sFilterConfig1.FilterID2 = 0x7FF; 						/* 屏蔽位模式下，FilterID2是消息屏蔽位 */
	HAL_FDCAN_ConfigFilter(&hfdcan1, &sFilterConfig1);      /* 配置过滤器 */
	
	sFilterConfig1.IdType = FDCAN_STANDARD_ID;              /* 设置标准ID或者扩展ID */
	sFilterConfig1.FilterIndex = 3;   						/* 用于过滤索引，扩展ID，范围0到64 */
	sFilterConfig1.FilterType = FDCAN_FILTER_MASK;          /* 忽略 ---------------------------*/
	sFilterConfig1.FilterConfig = FDCAN_FILTER_TO_RXBUFFER; /* 如果过滤匹配，将数据保存到Rx BUFFER */
	sFilterConfig1.FilterID1 = 0x014;                       /* 接收指定ID消息 */
	sFilterConfig1.FilterID2 = 0x7FF; 					    /* 忽略 */
	sFilterConfig1.IsCalibrationMsg = 0;                    /* 正常消息 */
	sFilterConfig1.RxBufferIndex = 1;                       /* Rx buffer位置 */
	HAL_FDCAN_ConfigFilter(&hfdcan1, &sFilterConfig1);      /* 配置过滤器 */
	
	HAL_FDCAN_ConfigGlobalFilter(&hfdcan1,                  /* 全局过滤设置 */
	                              FDCAN_REJECT,             /* 接收到消息ID与标准ID过滤不匹配，不接受 */
								  FDCAN_REJECT,             /* 接收到消息ID与扩展ID过滤不匹配，不接受 */
								  FDCAN_FILTER_REMOTE,      /* 过滤标准ID远程帧 */ 
								  FDCAN_FILTER_REMOTE);     /* 过滤扩展ID远程帧 */ 
	

	/* 第5步：Rx FIFO0中断配置 ===================================================================*/	
	/* 设置Rx FIFO0的阈值为1 ---------------------------------------*/
	HAL_FDCAN_ConfigFifoWatermark(&hfdcan1, FDCAN_CFG_RX_FIFO0, 1);

	/* 开启RX FIFO0的阈值中断 */
	HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_IT_RX_FIFO0_WATERMARK, 0);
	
	/* 第6步：Rx FIFO1中断配置 ===================================================================*/	
	/* 开启RX FIFO1的新数据中断 */
	HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_IT_RX_FIFO1_NEW_MESSAGE, 0);
	
	/* 第7步：Rx Buffer中断配置 ==================================================================*/	
	/* Rx Buffer接收到新消息就触发中断 */
	HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_IT_RX_BUFFER_NEW_MESSAGE, 0);	
	
	/* 启动FDCAN */
	HAL_FDCAN_Start(&hfdcan1);	
}

/*
*********************************************************************************************************
*	函 数 名: bsp_InitCan2
*	功能说明: 初始CAN2
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_InitCan2(void)
{	
	/* 第1步：基础配置 ==================================================================================*/
	hfdcan2.Instance = FDCAN2;                     /* 配置FDCAN1 */             
	hfdcan2.Init.FrameFormat = FDCAN_FRAME_FD_BRS; /* 配置使用FDCAN可变波特率 */  
	hfdcan2.Init.Mode = FDCAN_MODE_NORMAL;         /* 配置使用正常模式 */ 
	hfdcan2.Init.AutoRetransmission = ENABLE;      /* 使能自动重发 */ 
	hfdcan2.Init.TransmitPause = DISABLE;          /* 配置禁止传输暂停特性 */
	hfdcan2.Init.ProtocolException = ENABLE;       /* 协议异常处理使能 */
	
	/* 第2步：波特率配置 ==================================================================================*/
	/* 
		配置仲裁阶段波特率 
		CAN时钟20MHz时，仲裁阶段的波特率就是
		CAN FD Freq / (Sync_Seg + Pro_Seg + Phase_Seg1 + Phase_Seg2) = 20MHz / （1+0x1F + 8） = 0.5Mbps	
		
		其中Sync_Seg是固定值 = 1 ， Pro_Seg + Phase_Seg1 = NominalTimeSeg1， Phase_Seg2 = NominalTimeSeg2
	*/
	hfdcan2.Init.NominalPrescaler = 0x01; 	  /* CAN时钟分配设置，一般设置为1即可，全部由PLL配置好，tq = NominalPrescaler x (1/ fdcan_ker_ck) */
	hfdcan2.Init.NominalSyncJumpWidth = 0x08; /* 用于动态调节  Phase_Seg1和 Phase_Seg1，所以不可以比Phase_Seg1和 Phase_Seg1大 */
	hfdcan2.Init.NominalTimeSeg1 = 0x1F; 	  /* 特别注意这里的Seg1，这里是两个参数之和，对应位时间特性图的 Pro_Seg + Phase_Seg1 */
	hfdcan2.Init.NominalTimeSeg2 = 0x08;      /* 对应位时间特性图的 Phase_Seg2 */


	/* 
		配置数据阶段波特率 
		CAN时钟20MHz时，数据阶段的波特率就是
		CAN FD Freq / (Sync_Seg + Pro_Seg + Phase_Seg1 + Phase_Seg2) = 20MHz / 2 / （1+5+ 4） = 1Mbps
		
		其中Sync_Seg是固定值 = 1 ， Pro_Seg + Phase_Seg1 = DataTimeSeg1， Phase_Seg2 = DataTimeSeg2
	*/
	hfdcan2.Init.DataPrescaler = 0x02;      /* CAN时钟分配设置，一般设置为1即可，全部由PLL配置好，tq = NominalPrescaler x (1/ fdcan_ker_ck)，范围1-32 */
	hfdcan2.Init.DataSyncJumpWidth = 0x04;  /* 用于动态调节  Phase_Seg1和 Phase_Seg1，所以不可以比Phase_Seg1和 Phase_Seg1大，范围1-16 */
	hfdcan2.Init.DataTimeSeg1 = 0x05; 		/* 特别注意这里的Seg1，这里是两个参数之和，对应位时间特性图的 Pro_Seg + Phase_Seg1，范围 */
	hfdcan2.Init.DataTimeSeg2 = 0x04;       /* 对应位时间特性图的 Phase_Seg2 */    
	
	/* 第3步：RAM管理器配置 ==================================================================================*/
	hfdcan2.Init.MessageRAMOffset = 1280;      			/* CAN1和CAN2共享2560个字, 这里CAN1分配后1280字 */
	hfdcan2.Init.StdFiltersNbr = 4;         			/* 设置标准ID过滤器个数，范围0-128 */       
	hfdcan2.Init.ExtFiltersNbr = 4;         			/* 设置扩展ID过滤器个数，范围0-64 */   
	hfdcan2.Init.RxFifo0ElmtsNbr = 3;                   /* 设置Rx FIFO0的元素个数，范围0-64 */  
	hfdcan2.Init.RxFifo0ElmtSize = FDCAN_DATA_BYTES_8; 	/* 设置Rx FIFO0中每个元素大小，支持8,12,16,20,24,32,48或者64字节 */   
	hfdcan2.Init.RxFifo1ElmtsNbr = 3;                   /* 设置Rx FIFO1的元素个数，范围0-64 */
	hfdcan2.Init.RxFifo1ElmtSize = FDCAN_DATA_BYTES_8;  /* 设置Rx FIFO1中每个元素大小，支持8,12,16,20,24,32,48或者64字节 */	
	hfdcan2.Init.RxBuffersNbr = 2;                      /* 设置Rx Buffer个数，范围0-64 */
	hfdcan2.Init.RxBufferSize = FDCAN_DATA_BYTES_8;     /* 设置Rx Buffer中每个元素大小，支持8,12,16,20,24,32,48或者64字节 */	

	hfdcan2.Init.TxEventsNbr = 16;							/* 设置Tx Event FIFO中元素个数，范围0-32 */	
	hfdcan2.Init.TxBuffersNbr = 8;							/* 设置Tx Buffer中元素个数，范围0-32 */
	hfdcan2.Init.TxFifoQueueElmtsNbr = 8;                   /* 设置用于Tx FIFO/Queue的Tx Buffers个数。范围0到32 */
	hfdcan2.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION; /* 设置FIFO模式或者QUEUE队列模式 */
	hfdcan2.Init.TxElmtSize = FDCAN_DATA_BYTES_8;           /* 设置Tx Element中的数据域大小，支持8,12,16,20,24,32,48或者64字节 */
	HAL_FDCAN_Init(&hfdcan2);

	/* 
		配置过滤器, 过滤器主要用于接收，比如屏蔽位模式。
		FilterID1 = filter
		FilterID2 = mask
		
		FilterID2的mask每个bit含义
		0: 不关心，该位不用于比较；
		1: 必须匹配，接收到的ID必须与滤波器对应的ID位相一致。
		
		举例说明：
		FilterID1 = 0x111  0001 0001 0001 
		FilterID2 = 0x7FF  0111 1111 1111
		表示仅接收ID为0x111的FDCAN帧。
	*/
	
	/* 第4步：扩展ID过滤设置 ==================================================================================*/
	sFilterConfig2.IdType = FDCAN_EXTENDED_ID;              /* 扩展ID */
	sFilterConfig2.FilterIndex = 0;   						/* 用于过滤索引，扩展ID，范围0到64 */
	sFilterConfig2.FilterType = FDCAN_FILTER_RANGE;         /* 范围过滤 -----------------*/
	sFilterConfig2.FilterConfig = FDCAN_FILTER_TO_RXFIFO1;  /* 如果过滤匹配，将数据保存到Rx FIFO 1 */
	sFilterConfig2.FilterID1 = 0x12345678;                  /* 起始ID:0x12345678 */
	sFilterConfig2.FilterID2 = 0x12345680; 					/* 结束ID:0x12345680 */
	HAL_FDCAN_ConfigFilter(&hfdcan2, &sFilterConfig2);      /* 配置过滤器 */
	
	sFilterConfig2.IdType = FDCAN_EXTENDED_ID;              /* 扩展ID */
	sFilterConfig2.FilterIndex = 1;   						/* 用于过滤索引，扩展ID，范围0到64 */
	sFilterConfig2.FilterType = FDCAN_FILTER_DUAL;          /* 专用ID过滤 --------------*/
	sFilterConfig2.FilterConfig = FDCAN_FILTER_TO_RXFIFO1;  /* 如果过滤匹配，将数据保存到Rx FIFO 1 */
	sFilterConfig2.FilterID1 = 0x12345681;                  /* 指定ID */
	sFilterConfig2.FilterID2 = 0x12345682; 					/* 指定ID */
	HAL_FDCAN_ConfigFilter(&hfdcan2, &sFilterConfig2);      /* 配置过滤器 */
	
	sFilterConfig2.IdType = FDCAN_EXTENDED_ID;              /* 扩展ID */
	sFilterConfig2.FilterIndex = 2;   						/* 用于过滤索引，扩展ID，范围0到64 */
	sFilterConfig2.FilterType = FDCAN_FILTER_MASK;          /* 过滤器采样屏蔽位模式 ----*/
	sFilterConfig2.FilterConfig = FDCAN_FILTER_TO_RXFIFO1;  /* 如果过滤匹配，将数据保存到Rx FIFO 1 */
	sFilterConfig2.FilterID1 = 0x12345683;                  /* 屏蔽位模式下，FilterID1是消息ID */
	sFilterConfig2.FilterID2 = 0x1FFFFFFF; 					/* 屏蔽位模式下，FilterID2是消息屏蔽位 */
	HAL_FDCAN_ConfigFilter(&hfdcan2, &sFilterConfig2);      /* 配置过滤器 */
	
	sFilterConfig2.IdType = FDCAN_EXTENDED_ID;              /* 扩展ID */
	sFilterConfig2.FilterIndex = 3;   						/* 用于过滤索引，扩展ID，范围0到64 */
	sFilterConfig2.FilterType = FDCAN_FILTER_MASK;          /* 忽略 --------------------*/
	sFilterConfig2.FilterConfig = FDCAN_FILTER_TO_RXBUFFER; /* 如果过滤匹配，将数据保存到Rx Buffer */
	sFilterConfig2.FilterID1 = 0x12345684;                  /* 接收指定ID消息 */
	sFilterConfig2.FilterID2 = 0x1FFFFFFF; 					/* 忽略 */
	sFilterConfig2.IsCalibrationMsg = 0;                    /* 正常消息 */
	sFilterConfig2.RxBufferIndex = 0;                       /* Rx buffer位置 */
	HAL_FDCAN_ConfigFilter(&hfdcan2, &sFilterConfig2);      /* 配置过滤器 */

	/* 第5步：标准ID过滤设置 ==================================================================================*/	
	sFilterConfig2.IdType = FDCAN_STANDARD_ID;              /* 设置标准ID或者扩展ID */
	sFilterConfig2.FilterIndex = 0;   						/* 用于过滤索引，标准ID，范围0到127*/
	sFilterConfig2.FilterType = FDCAN_FILTER_RANGE;         /* 范围过滤 -------------------------*/
	sFilterConfig2.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;  /* 如果过滤匹配，将数据保存到Rx FIFO 0 */
	sFilterConfig2.FilterID1 = 0x000;                       /* 起始ID:0x000 */
	sFilterConfig2.FilterID2 = 0x010; 						/* 结束ID:0x010 */
	HAL_FDCAN_ConfigFilter(&hfdcan2, &sFilterConfig2);      /* 配置过滤器 */
	
	sFilterConfig2.IdType = FDCAN_STANDARD_ID;              /* 设置标准ID或者扩展ID */
	sFilterConfig2.FilterIndex = 1;   						/* 用于过滤索引，标准ID，范围0到127*/
	sFilterConfig2.FilterType = FDCAN_FILTER_DUAL;          /* 专用ID过滤 ----------------------*/
	sFilterConfig2.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;  /* 如果过滤匹配，将数据保存到Rx FIFO 0 */
	sFilterConfig2.FilterID1 = 0x011;                       /* 指定ID */
	sFilterConfig2.FilterID2 = 0x012; 						/* 指定ID */
	HAL_FDCAN_ConfigFilter(&hfdcan2, &sFilterConfig2);      /* 配置过滤器 */
	
	sFilterConfig2.IdType = FDCAN_STANDARD_ID;              /* 设置标准ID或者扩展ID */
	sFilterConfig2.FilterIndex = 2;   						/* 用于过滤索引，标准ID，范围0到127*/
	sFilterConfig2.FilterType = FDCAN_FILTER_MASK;          /* 过滤器采样屏蔽位模式 -----------*/
	sFilterConfig2.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;  /* 如果过滤匹配，将数据保存到Rx FIFO 0 */
	sFilterConfig2.FilterID1 = 0x013;                       /* 屏蔽位模式下，FilterID1是消息ID */
	sFilterConfig2.FilterID2 = 0x7FF; 						/* 屏蔽位模式下，FilterID2是消息屏蔽位 */
	HAL_FDCAN_ConfigFilter(&hfdcan2, &sFilterConfig2);      /* 配置过滤器 */
	
	sFilterConfig2.IdType = FDCAN_STANDARD_ID;              /* 设置标准ID或者扩展ID */
	sFilterConfig2.FilterIndex = 3;   						/* 用于过滤索引，扩展ID，范围0到64 */
	sFilterConfig2.FilterType = FDCAN_FILTER_MASK;          /* 忽略 ---------------------------*/
	sFilterConfig2.FilterConfig = FDCAN_FILTER_TO_RXBUFFER; /* 如果过滤匹配，将数据保存到Rx BUFFER */
	sFilterConfig2.FilterID1 = 0x014;                       /* 接收指定ID消息 */
	sFilterConfig2.FilterID2 = 0x7FF; 					    /* 忽略 */
	sFilterConfig2.IsCalibrationMsg = 0;                    /* 正常消息 */
	sFilterConfig2.RxBufferIndex = 1;                       /* Rx buffer位置 */
	HAL_FDCAN_ConfigFilter(&hfdcan2, &sFilterConfig2);      /* 配置过滤器 */
	
	HAL_FDCAN_ConfigGlobalFilter(&hfdcan2,                  /* 全局过滤设置 */
	                              FDCAN_REJECT,             /* 接收到消息ID与标准ID过滤不匹配，不接受 */
								  FDCAN_REJECT,             /* 接收到消息ID与扩展ID过滤不匹配，不接受 */
								  FDCAN_FILTER_REMOTE,      /* 过滤标准ID远程帧 */ 
								  FDCAN_FILTER_REMOTE);     /* 过滤扩展ID远程帧 */ 
	

	/* 第5步：Rx FIFO0中断配置 ===================================================================*/	
	/* 设置Rx FIFO0的阈值为1 ---------------------------------------*/
	HAL_FDCAN_ConfigFifoWatermark(&hfdcan2, FDCAN_CFG_RX_FIFO0, 1);

	/* 开启RX FIFO0的阈值中断 */
	HAL_FDCAN_ActivateNotification(&hfdcan2, FDCAN_IT_RX_FIFO0_WATERMARK, 0);
	
	/* 第6步：Rx FIFO1中断配置 ===================================================================*/	
	/* 开启RX FIFO1的新数据中断 */
	HAL_FDCAN_ActivateNotification(&hfdcan2, FDCAN_IT_RX_FIFO1_NEW_MESSAGE, 0);
	
	/* 第7步：Rx Buffer中断配置 ==================================================================*/	
	/* Rx Buffer接收到新消息就触发中断 */
	HAL_FDCAN_ActivateNotification(&hfdcan2, FDCAN_IT_RX_BUFFER_NEW_MESSAGE, 0);	
	
	/* 启动FDCAN */
	HAL_FDCAN_Start(&hfdcan2);	
}

/*
*********************************************************************************************************
*	函 数 名: bsp_DeInitCan1
*	功能说明: 释放CAN1
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_DeInitCan1(void)
{
	HAL_FDCAN_MspDeInit(&hfdcan1);
}

/*
*********************************************************************************************************
*	函 数 名: bsp_DeInitCan2
*	功能说明: 释放CAN2
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_DeInitCan2(void)
{
	HAL_FDCAN_MspDeInit(&hfdcan2);
}

/*
*********************************************************************************************************
*	函 数 名: HAL_FDCAN_MspInit
*	功能说明: 配置CAN gpio
*	形    参: hfdcan
*	返 回 值: 无
*********************************************************************************************************
*/
void HAL_FDCAN_MspInit(FDCAN_HandleTypeDef* hfdcan)
{
	GPIO_InitTypeDef  GPIO_InitStruct;
	RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

	if (hfdcan == &hfdcan1)
	{
		/*##-1- 使能外设这个GPIO时钟 #################################*/
		FDCAN1_TX_GPIO_CLK_ENABLE();
		FDCAN1_RX_GPIO_CLK_ENABLE();

		/* 选择PLL2Q作为FDCANx时钟 */
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
		/* 选择PLL2Q作为FDCANx时钟 */
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
*	函 数 名: HAL_FDCAN_MspInit
*	功能说明: 配置CAN gpio, 恢复为普通GPIO，取消中断
*	形    参: hfdcan
*	返 回 值: 无
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
*	函 数 名: HAL_FDCAN_RxFifo0Callback
*	功能说明: Rx FIFO0回调函数
*	形    参: hfdcan
*	返 回 值: 无
*********************************************************************************************************
*/
void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
	if (hfdcan == &hfdcan1)
	{
		if ((RxFifo0ITs & FDCAN_IT_RX_FIFO0_WATERMARK) != RESET)
		{
			/* 从RX FIFO0读取数据 */
			HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &g_Can1RxHeader, g_Can1RxData);

			/* 重新使能RX FIFO0阈值中断 */
			HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_RX_FIFO0_WATERMARK, 0);

			/* 发消息收到数据包，结果在g_Can1RxHeader， g_Can1RxData */
			bsp_PutMsg(MSG_CAN1_RxFIFO0, 0);	
		}
	}

	if (hfdcan == &hfdcan2)
	{
		if ((RxFifo0ITs & FDCAN_IT_RX_FIFO0_WATERMARK) != RESET)
		{
			/* 从RX FIFO0读取数据 */
			HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &g_Can2RxHeader, g_Can2RxData);

			/* 重新使能RX FIFO0阈值中断 */
			HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_RX_FIFO0_WATERMARK, 0);
					
			/* 发消息收到数据包，结果在g_Can1RxHeader， g_Can1RxData */
			bsp_PutMsg(MSG_CAN2_RxFIFO0, 0);	
		}
	}
}

/*
*********************************************************************************************************
*	函 数 名: HAL_FDCAN_RxFifo0Callback
*	功能说明: Rx FIFO0回调函数
*	形    参: hfdcan
*	返 回 值: 无
*********************************************************************************************************
*/
void HAL_FDCAN_RxFifo1Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo1ITs)
{
	if (hfdcan == &hfdcan1)
	{
		if ((RxFifo1ITs & FDCAN_IT_RX_FIFO1_NEW_MESSAGE) != RESET)
		{
			/* 从RX FIFO1读取数据 */
			HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO1, &g_Can1RxHeader, g_Can1RxData);
			
			/* 使能Rx FIFO1收到新数据阈值中断 */
			HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_IT_RX_FIFO1_NEW_MESSAGE, 0);

			/* 发消息收到数据包，结果在g_Can1RxHeader， g_Can1RxData */
			bsp_PutMsg(MSG_CAN1_RxFIFO1, 0);	

		}
	}

	if (hfdcan == &hfdcan2)
	{
		if ((RxFifo1ITs & FDCAN_IT_RX_FIFO1_NEW_MESSAGE) != RESET)
		{
			/* 从RX FIFO0读取数据 */
			HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO1, &g_Can2RxHeader, g_Can2RxData);
			
			/* 使能Rx FIFO1收到新数据阈值中断 */
			HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_IT_RX_FIFO1_NEW_MESSAGE, 0);
	
			/* 发消息收到数据包，结果在g_Can1RxHeader， g_Can1RxData */
			bsp_PutMsg(MSG_CAN2_RxFIFO1, 0);	
		}
	}
}

/*
*********************************************************************************************************
*	函 数 名: HAL_FDCAN_RxBufferNewMessageCallback
*	功能说明: Rx Buffer回调函数
*	形    参: hfdcan
*	返 回 值: 无
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
				/* 从RX Buffer0读取数据, i值等同于FDCAN_RX_BUFFER */
				Status = HAL_FDCAN_GetRxMessage(hfdcan, i, &g_Can1RxHeader, g_Can1RxData);

				if(Status == HAL_OK)
				{
					/* 发消息收到数据包，结果在g_Can1RxHeader， g_Can1RxData */
					bsp_PutMsg(MSG_CAN1_RxBuffer, 0);				
				}		
			}
		}
		
		for(i = 0; i < 32; i++)
		{
			off = (hfdcan->Instance->NDAT2 & (1<<i) ) >> i;
			if(off == 1)
			{
				/* 从RX Buffer0读取数据, i值等同于FDCAN_RX_BUFFER */
				Status = HAL_FDCAN_GetRxMessage(hfdcan, i+31, &g_Can1RxHeader, g_Can1RxData);

				if(Status == HAL_OK)
				{
					/* 发消息收到数据包，结果在g_Can1RxHeader， g_Can1RxData */		
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
				/* 从RX Buffer0读取数据, i值等同于FDCAN_RX_BUFFER */
				Status = HAL_FDCAN_GetRxMessage(hfdcan, i, &g_Can1RxHeader, g_Can1RxData);

				if(Status == HAL_OK)
				{
					/* 发消息收到数据包，结果在g_Can1RxHeader， g_Can1RxData */
					bsp_PutMsg(MSG_CAN2_RxBuffer, 0);				
				}		
			}
		}
		
		for(i = 0; i < 32; i++)
		{
			off = (hfdcan->Instance->NDAT2 & (1<<i) ) >> i;
			if(off == 1)
			{
				/* 从RX Buffer0读取数据, i值等同于FDCAN_RX_BUFFER */
				Status = HAL_FDCAN_GetRxMessage(hfdcan, i+31, &g_Can1RxHeader, g_Can1RxData);

				if(Status == HAL_OK)
				{
					/* 发消息收到数据包，结果在g_Can1RxHeader， g_Can1RxData */		
					bsp_PutMsg(MSG_CAN2_RxBuffer, 0);		
				}		
			}
		}
	}	
}

/*
*********************************************************************************************************
*	函 数 名: HAL_FDCAN_TxFifoEmptyCallback
*	功能说明: Tx FIFO发送完成回调函数
*	形    参: hfdcan
*	返 回 值: 无
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
*	函 数 名: HAL_FDCAN_TxBufferCompleteCallback
*	功能说明: Tx Buffer发送完成标志
*	形    参: hfdcan
*	返 回 值: 无
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
*	函 数 名: can1_SendPacket
*	功能说明: 发送一包数据
*	形    参：_DataBuf 数据缓冲区
*			  _Len 数据长度
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
*	返 回 值: 无
*********************************************************************************************************
*/
void can1_SendPacket(uint8_t *_DataBuf, uint32_t _Len)
{		
	FDCAN_TxHeaderTypeDef TxHeader = {0};

	/* 第1步：配置发送参数 ===================================================================*/
	TxHeader.Identifier = 0x001;             		     /* 设置接收帧消息的ID */
	TxHeader.IdType = FDCAN_STANDARD_ID;     		     /* 标准ID */
	TxHeader.TxFrameType = FDCAN_DATA_FRAME;		     /* 数据帧 */
	TxHeader.DataLength = _Len;      				     /* 发送数据长度 */
	TxHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;     /* 设置错误状态指示 */
	TxHeader.BitRateSwitch = FDCAN_BRS_ON;               /* 开启可变波特率 */
	TxHeader.FDFormat = FDCAN_FD_CAN;                    /* FDCAN格式 */
	TxHeader.TxEventFifoControl = FDCAN_STORE_TX_EVENTS; /* 用于发送事件FIFO控制, 不存储 */
	TxHeader.MessageMarker = 1;                          /* 用于复制到TX EVENT FIFO的消息Maker来识别消息状态，范围0到0xFF */
	
#if 0 /* 简单发送方式 */ 
	
	HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &TxHeader, _DataBuf); /* 根据需要，可以修改_DataBuf数值 */
	
#else /* 复杂发送方式，并开启中断 */
    /* 第2步：添加数据到TX FIFO, 连续添加三组 ================================================*/
    HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &TxHeader, _DataBuf); /* 根据需要，可以修改_DataBuf数值 */
	TxHeader.Identifier = 0x011;             		       		     
	TxHeader.MessageMarker = 2;   
	HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &TxHeader, _DataBuf); /* 根据需要，可以修改_DataBuf数值 */
	TxHeader.Identifier = 0x013;             		    
	TxHeader.MessageMarker = 3;   
	HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &TxHeader, _DataBuf); /* 根据需要，可以修改_DataBuf数值 */
	
	/* 使能Tx FIFO发送空中断 */
	HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_IT_TX_FIFO_EMPTY, 0);	
	
    /* 第2步：添加数据到TX BUFFER, 连续添加三组 ================================================*/
	TxHeader.Identifier = 0x12345678;             		    
	TxHeader.IdType = FDCAN_EXTENDED_ID;  
	TxHeader.MessageMarker = 4;
	HAL_FDCAN_AddMessageToTxBuffer(&hfdcan1,  &TxHeader, _DataBuf, FDCAN_TX_BUFFER0);/* 根据需要，可以修改_DataBuf数值 */
	TxHeader.Identifier = 0x12345681;             		    
	TxHeader.IdType = FDCAN_EXTENDED_ID;
	TxHeader.MessageMarker = 5;
	HAL_FDCAN_AddMessageToTxBuffer(&hfdcan1,  &TxHeader, _DataBuf, FDCAN_TX_BUFFER1);/* 根据需要，可以修改_DataBuf数值 */
	TxHeader.Identifier = 0x12345683;             		    
	TxHeader.IdType = FDCAN_EXTENDED_ID;
	TxHeader.MessageMarker = 6;
	HAL_FDCAN_AddMessageToTxBuffer(&hfdcan1,  &TxHeader, _DataBuf, FDCAN_TX_BUFFER2);/* 根据需要，可以修改_DataBuf数值 */	
	
	/* 使能Tx Buffer发送完成中断 */	
	HAL_FDCAN_EnableTxBufferRequest(&hfdcan1, FDCAN_TX_BUFFER0|FDCAN_TX_BUFFER1|FDCAN_TX_BUFFER2);
	HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_IT_TX_COMPLETE, FDCAN_TX_BUFFER0|FDCAN_TX_BUFFER1|FDCAN_TX_BUFFER2);
#endif
}

/*
*********************************************************************************************************
*	函 数 名: can2_SendPacket
*	功能说明: 发送一包数据
*	形    参：_DataBuf 数据缓冲区
*			  _Len 数据长度
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
*	返 回 值: 无
*********************************************************************************************************
*/
void can2_SendPacket(uint8_t *_DataBuf, uint32_t _Len)
{		
	FDCAN_TxHeaderTypeDef TxHeader = {0};

	/* 第1步：配置发送参数 ===================================================================*/
	TxHeader.Identifier = 0x001;             		     /* 设置接收帧消息的ID */
	TxHeader.IdType = FDCAN_STANDARD_ID;     		     /* 标准ID */
	TxHeader.TxFrameType = FDCAN_DATA_FRAME;		     /* 数据帧 */
	TxHeader.DataLength = _Len;      				     /* 发送数据长度 */
	TxHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;     /* 设置错误状态指示 */
	TxHeader.BitRateSwitch = FDCAN_BRS_ON;               /* 开启可变波特率 */
	TxHeader.FDFormat = FDCAN_FD_CAN;                    /* FDCAN格式 */
	TxHeader.TxEventFifoControl = FDCAN_STORE_TX_EVENTS; /* 用于发送事件FIFO控制, 不存储 */
	TxHeader.MessageMarker = 1;                          /* 用于复制到TX EVENT FIFO的消息Maker来识别消息状态，范围0到0xFF */
	
#if 0 /* 简单发送方式 */ 
	
	HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &TxHeader, _DataBuf); /* 根据需要，可以修改_DataBuf数值 */
	
#else /* 复杂发送方式，并开启中断 */
    /* 第2步：添加数据到TX FIFO, 连续添加三组 ================================================*/
    HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan2, &TxHeader, _DataBuf); /* 根据需要，可以修改_DataBuf数值 */
	TxHeader.Identifier = 0x011;             		       		     
	TxHeader.MessageMarker = 2;   
	HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan2, &TxHeader, _DataBuf); /* 根据需要，可以修改_DataBuf数值 */
	TxHeader.Identifier = 0x013;             		    
	TxHeader.MessageMarker = 3;   
	HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan2, &TxHeader, _DataBuf); /* 根据需要，可以修改_DataBuf数值 */
	
	/* 使能Tx FIFO发送空中断 */
	HAL_FDCAN_ActivateNotification(&hfdcan2, FDCAN_IT_TX_FIFO_EMPTY, 0);	
	
    /* 第2步：添加数据到TX BUFFER, 连续添加三组 ================================================*/
	TxHeader.Identifier = 0x12345678;             		    
	TxHeader.IdType = FDCAN_EXTENDED_ID;  
	TxHeader.MessageMarker = 4;
	HAL_FDCAN_AddMessageToTxBuffer(&hfdcan2,  &TxHeader, _DataBuf, FDCAN_TX_BUFFER0);/* 根据需要，可以修改_DataBuf数值 */
	TxHeader.Identifier = 0x12345681;             		    
	TxHeader.IdType = FDCAN_EXTENDED_ID;
	TxHeader.MessageMarker = 5;
	HAL_FDCAN_AddMessageToTxBuffer(&hfdcan2,  &TxHeader, _DataBuf, FDCAN_TX_BUFFER1);/* 根据需要，可以修改_DataBuf数值 */
	TxHeader.Identifier = 0x12345683;             		    
	TxHeader.IdType = FDCAN_EXTENDED_ID;
	TxHeader.MessageMarker = 6;
	HAL_FDCAN_AddMessageToTxBuffer(&hfdcan2,  &TxHeader, _DataBuf, FDCAN_TX_BUFFER2);/* 根据需要，可以修改_DataBuf数值 */	
	
	/* 使能Tx Buffer发送完成中断 */	
	HAL_FDCAN_EnableTxBufferRequest(&hfdcan2, FDCAN_TX_BUFFER0|FDCAN_TX_BUFFER1|FDCAN_TX_BUFFER2);
	HAL_FDCAN_ActivateNotification(&hfdcan2, FDCAN_IT_TX_COMPLETE, FDCAN_TX_BUFFER0|FDCAN_TX_BUFFER1|FDCAN_TX_BUFFER2);
#endif
}

/*
*********************************************************************************************************
*	函 数 名: FDCAN1_IT0_IRQHandler
*	功能说明: CAN中断服务程序
*	形    参: hfdcan
*	返 回 值: 无
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

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
