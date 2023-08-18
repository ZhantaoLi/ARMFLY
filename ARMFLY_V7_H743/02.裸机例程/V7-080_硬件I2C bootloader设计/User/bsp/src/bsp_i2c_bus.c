/*
*********************************************************************************************************
*
*	ģ������ : Ӳ��I2C��������
*	�ļ����� : bsp_i2c_bus.c
*	��    �� : V1.0
*	˵    �� : 
*	�޸ļ�¼ :
*		�汾��  ����         ����      ˵��
*       v1.0    2022-10-24  Eric2013   �װ档
*
*	Copyright (C), 2020-2030, ���������� www.armfly.com
*
*********************************************************************************************************
*/
#include "bsp.h"



/*
	������STM32-V7��������߷���
	PB6/I2C1 CLK
	PB9/I2C1 SDA
*/

/*
*********************************************************************************************************
*	                            ʱ�ӣ����ţ�DMA���жϵȺ궨��
*********************************************************************************************************
*/
#define I2C_ADDRESS        0x20

/* I2C TIMING Register define when I2C clock source is APB1 (SYSCLK/4) */
/* I2C TIMING is calculated in case of the I2C Clock source is the APB1CLK = 100 MHz */
/* This example use TIMING to 0x00901954 to reach 400 kHz speed (Rise time = 100 ns, Fall time = 10 ns) */
#define I2C_TIMING      0x00901954

/* ʹ�õ�I2C1 */
#define I2Cx                            I2C1
#define I2Cx_CLK_ENABLE()               __HAL_RCC_I2C1_CLK_ENABLE()
#define I2Cx_SDA_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOB_CLK_ENABLE()
#define I2Cx_SCL_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOB_CLK_ENABLE() 

#define I2Cx_FORCE_RESET()              __HAL_RCC_I2C1_FORCE_RESET()
#define I2Cx_RELEASE_RESET()            __HAL_RCC_I2C1_RELEASE_RESET()

/* ����ʹ�õ����� */
#define I2Cx_SCL_PIN                    GPIO_PIN_6
#define I2Cx_SCL_GPIO_PORT              GPIOB
#define I2Cx_SDA_PIN                    GPIO_PIN_9
#define I2Cx_SDA_GPIO_PORT              GPIOB
#define I2Cx_SCL_SDA_AF                 GPIO_AF4_I2C1

/* ����NVIC�ж���� */
#define I2Cx_EV_IRQn                    I2C1_EV_IRQn
#define I2Cx_ER_IRQn                    I2C1_ER_IRQn
#define I2Cx_EV_IRQHandler              I2C1_EV_IRQHandler
#define I2Cx_ER_IRQHandler              I2C1_ER_IRQHandler


/*
*********************************************************************************************************
*	                                           ����
*********************************************************************************************************
*/
__IO uint32_t wTransferState = TRANSFER_WAIT;

I2C_HandleTypeDef I2cHandle;

uint8_t g_i2cTxBuf[I2C_BUFFER_SIZE]; /* ���ͻ��� */
uint8_t g_i2cRxBuf[I2C_BUFFER_SIZE]; /* ���ջ��� */

uint32_t g_i2cLen;	

/*
*********************************************************************************************************
*	�� �� ��: bsp_InitI2CBus
*	����˵��: ����I2C���ߡ�
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void bsp_InitI2CBus(void)
{	
	I2cHandle.Instance             = I2Cx;
	I2cHandle.Init.Timing          = I2C_TIMING;
	I2cHandle.Init.OwnAddress1     = I2C_ADDRESS;
	I2cHandle.Init.AddressingMode  = I2C_ADDRESSINGMODE_7BIT;
	I2cHandle.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	I2cHandle.Init.OwnAddress2     = 0xF0;
	I2cHandle.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
	I2cHandle.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	I2cHandle.Init.NoStretchMode   = I2C_NOSTRETCH_DISABLE;  

	if(HAL_I2C_Init(&I2cHandle) != HAL_OK)
	{
		Error_Handler(__FILE__, __LINE__);
	}

	/* ʹ��I2C��ģ���˲� */
	HAL_I2CEx_ConfigAnalogFilter(&I2cHandle,I2C_ANALOGFILTER_ENABLE);
}

/*
*********************************************************************************************************
*	�� �� ��: HAL_I2C_MspInit
*	����˵��: ����I2C���GPIO��ʱ�Ӻ�NVIC��
*	��    ��: --
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void HAL_I2C_MspInit(I2C_HandleTypeDef *hi2c)
{
	GPIO_InitTypeDef  GPIO_InitStruct;

	/* ʱ�� */
	I2Cx_SCL_GPIO_CLK_ENABLE();
	I2Cx_SDA_GPIO_CLK_ENABLE();
	I2Cx_CLK_ENABLE(); 

	/* ����GPIO */
	GPIO_InitStruct.Pin       = I2Cx_SCL_PIN;
	GPIO_InitStruct.Mode      = GPIO_MODE_AF_OD;
	GPIO_InitStruct.Pull      = GPIO_PULLUP;
	GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.Alternate = I2Cx_SCL_SDA_AF;
	HAL_GPIO_Init(I2Cx_SCL_GPIO_PORT, &GPIO_InitStruct);

	GPIO_InitStruct.Pin       = I2Cx_SDA_PIN;
	GPIO_InitStruct.Alternate = I2Cx_SCL_SDA_AF;
	HAL_GPIO_Init(I2Cx_SDA_GPIO_PORT, &GPIO_InitStruct);

	/* ����NVIC */
	HAL_NVIC_SetPriority(I2Cx_ER_IRQn, 2, 0);
	HAL_NVIC_EnableIRQ(I2Cx_ER_IRQn);
	HAL_NVIC_SetPriority(I2Cx_EV_IRQn, 2, 0);
	HAL_NVIC_EnableIRQ(I2Cx_EV_IRQn);
}

/*
*********************************************************************************************************
*	�� �� ��: HAL_I2C_MspDeInit
*	����˵��: ��λI2C
*	��    ��: --
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void HAL_I2C_MspDeInit(I2C_HandleTypeDef *hi2c)
{
	I2Cx_FORCE_RESET();
	I2Cx_RELEASE_RESET();

	HAL_GPIO_DeInit(I2Cx_SCL_GPIO_PORT, I2Cx_SCL_PIN);
	HAL_GPIO_DeInit(I2Cx_SDA_GPIO_PORT, I2Cx_SDA_PIN);

	HAL_NVIC_DisableIRQ(I2Cx_ER_IRQn);
	HAL_NVIC_DisableIRQ(I2Cx_EV_IRQn);
}

/*
*********************************************************************************************************
*	�� �� ��: bsp_i2cTransfer
*	����˵��: �������ݴ���
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void bsp_i2cTransfer(void)
{
	if (g_i2cLen > I2C_BUFFER_SIZE)
	{
		return;
	}
	
	/* �жϷ�ʽ���� */	
	wTransferState = TRANSFER_WAIT;
	
	if(HAL_I2C_Slave_Transmit_IT(&I2cHandle, (uint8_t*)g_i2cTxBuf, g_i2cLen)!= HAL_OK)
	{
		Error_Handler(__FILE__, __LINE__);  
	}

	/* �����ǵȴ�������ɣ���ҿ��Ը��������޸�Ϊ������ */
	while (wTransferState == TRANSFER_WAIT)
	{
		;
	}

}

/*
*********************************************************************************************************
*	�� �� ��: bsp_i2cTransfer
*	����˵��: �������ݴ���
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void bsp_i2cReceive(void)
{
	if (g_i2cLen > I2C_BUFFER_SIZE)
	{
		return;
	}
	
	/* �жϷ�ʽ���� */	
	wTransferState = TRANSFER_WAIT;
	
	if(HAL_I2C_Slave_Receive_IT(&I2cHandle, (uint8_t *)g_i2cRxBuf, g_i2cLen) != HAL_OK)
	{
		Error_Handler(__FILE__, __LINE__);
	}
  
//	while (wTransferState == TRANSFER_WAIT)
//	{
//		;
//	}
		
}

/*
*********************************************************************************************************
*	�� �� ��: HAL_I2C_SlaveTxCpltCallback
*             HAL_I2C_SlaveRxCpltCallback
*	����˵��: I2C�ӻ�ģʽ���ͺͽ�����ɻص�
*	��    ��: --
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void HAL_I2C_SlaveTxCpltCallback(I2C_HandleTypeDef *I2cHandle)
{
	wTransferState = TRANSFER_COMPLETE;
}

void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *I2cHandle)
{
	wTransferState = TRANSFER_COMPLETE;
}

/*
*********************************************************************************************************
*	�� �� ��: HAL_I2C_ErrorCallback
*	����˵��: I2Cͨ�Ŵ���ص�
*	��    ��: --
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *I2cHandle)
{
	/** Error_Handler() function is called when error occurs.
	* 1- When Slave don't acknowledge it's address, Master restarts communication.
	* 2- When Master don't acknowledge the last data transferred, Slave don't care in this example.
	*/
	if (HAL_I2C_GetError(I2cHandle) != HAL_I2C_ERROR_AF)
	{
		wTransferState = TRANSFER_ERROR;
	}
}

/*
*********************************************************************************************************
*	�� �� ��: I2Cx_EV_IRQHandler
*             I2Cx_ER_IRQHandler
*	����˵��: �жϷ������
*	��    ��: --
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void I2Cx_EV_IRQHandler(void)
{
	HAL_I2C_EV_IRQHandler(&I2cHandle);
}

void I2Cx_ER_IRQHandler(void)
{
	HAL_I2C_ER_IRQHandler(&I2cHandle);
}

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
