/**
  ******************************************************************************
  * @file    USB_Device/MSC_Standalone/Src/usbd_storage.c
  * @author  MCD Application Team
  * @brief   Memory management layer
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2017 STMicroelectronics International N.V.
  * All rights reserved.</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice,
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other
  *    contributors to this software may be used to endorse or promote products
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under
  *    this license is void and will automatically terminate your rights under
  *    this license.
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------ */
#include "usbd_storage.h"
#include "bsp_sdio_sd.h"
#include "bsp_fmc_sdram.h"
#include "bsp_fmc_nand_flash.h"

//#define printf_ok			printf
#define printf_ok(...)

#define printf_err			printf
//#define printf_err(...)

#define STORAGE_LUN_NBR                  1
//#define STORAGE_BLK_NBR                  0x10000
//#define STORAGE_BLK_SIZ                  0x200

#define LUN_SD		0
#define LUN_NAND	1
#define LUN_SDRAM	2

/* 定义SDRAM 虚拟磁盘的地址和空间。 4M字节 */
#define SDRAM_DISK_ADDR		SDRAM_APP_BUF
#define SDRAM_DISK_SIZE		(4 * 1024 * 1024)

/* Private macro ------------------------------------------------------------- */
/* Private variables --------------------------------------------------------- */
/* USB Mass storage Standard Inquiry Data */
int8_t STORAGE_Inquirydata[] = {  /* 36 */
	/* LUN 0 */
	0x00,
	0x80,
	0x02,
	0x02,
	(STANDARD_INQUIRY_DATA_LEN - 5),
	0x00,
	0x00,
	0x00,
	'S', 'T', 'M', ' ', ' ', ' ', ' ', ' ', /* Manufacturer: 8 bytes */
	'P', 'r', 'o', 'd', 'u', 'c', 't', ' ', /* Product : 16 Bytes */
	' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
	'0', '.', '0', '1',           /* Version : 4 Bytes */
	
	/* LUN 1 NAND Flash */
	0x00,
	0x80,
	0x02,
	0x02,
	(STANDARD_INQUIRY_DATA_LEN - 5),
	0x00,
	0x00,
	0x00,
	'A', 'R', 'M', 'F', 'L', 'Y', ' ', ' ', /* Manufacturer : 8 bytes */
	'N', 'A', 'N', 'D', ' ', 'F', 'l', 'a', /* Product      : 16 Bytes */
	' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
	'1', '.', '0' ,'0',                     /* Version      : 4 Bytes */	

	/* LUN 2 SDRAM */
	0x00,
	0x80,
	0x02,
	0x02,
	(STANDARD_INQUIRY_DATA_LEN - 5),
	0x00,
	0x00,
	0x00,
	'A', 'R', 'M', 'F', 'L', 'Y', ' ', ' ', /* Manufacturer : 8 bytes */
	'S', 'D', 'R', 'A', 'M', ' ', ' ', ' ', /* Product      : 16 Bytes */
	's', 'h', ' ', ' ', ' ', ' ', ' ', ' ',
	'1', '.', '0' ,'0',                     /* Version      : 4 Bytes */	
};

/* Private function prototypes ----------------------------------------------- */
int8_t STORAGE_Init(uint8_t lun);
int8_t STORAGE_GetCapacity(uint8_t lun, uint32_t * block_num,
                           uint16_t * block_size);
int8_t STORAGE_IsReady(uint8_t lun);
int8_t STORAGE_IsWriteProtected(uint8_t lun);
int8_t STORAGE_Read(uint8_t lun, uint8_t * buf, uint32_t blk_addr,
                    uint16_t blk_len);
int8_t STORAGE_Write(uint8_t lun, uint8_t * buf, uint32_t blk_addr,
                     uint16_t blk_len);
int8_t STORAGE_GetMaxLun(void);

USBD_StorageTypeDef USBD_DISK_fops = {
  STORAGE_Init,
  STORAGE_GetCapacity,
  STORAGE_IsReady,
  STORAGE_IsWriteProtected,
  STORAGE_Read,
  STORAGE_Write,
  STORAGE_GetMaxLun,
  STORAGE_Inquirydata,
};

/* Private functions --------------------------------------------------------- */

/**
  * @brief  Initializes the storage unit (medium)       
  * @param  lun: Logical unit number
  * @retval Status (0 : OK / -1 : Error)
  */
int8_t STORAGE_Init(uint8_t lun)
{
	int8_t ret = -1;
	
	switch (lun)
	{
		case LUN_SD:	
			BSP_SD_Init();
			ret = 0;
			break;
		
		case LUN_NAND:
//			if (NAND_Init() == NAND_OK)
//			{
//				ret = 0;
//			}
			break;
			
		case LUN_SDRAM:
			break;
	}
	return ret;	
}

/**
  * @brief  Returns the medium capacity.      
  * @param  lun: Logical unit number
  * @param  block_num: Number of total block number
  * @param  block_size: Block size
  * @retval Status (0: OK / -1: Error)
  */
int8_t STORAGE_GetCapacity(uint8_t lun, uint32_t * block_num,
                           uint16_t * block_size)
{
	int8_t ret = -1;
	
	switch (lun)
	{
		case LUN_SD:	
			{
				HAL_SD_CardInfoTypeDef info;
				

				if (BSP_SD_IsDetected() != SD_NOT_PRESENT)
				{
					BSP_SD_GetCardInfo(&info);

					*block_num = info.LogBlockNbr - 1;
					*block_size = info.LogBlockSize;
					ret = 0;
				}
			}
			break;
		
		case LUN_NAND:
//			#if 1
//				*block_size =  512;
//				//*block_num =  (NAND_PAGE_SIZE / 512) * NAND_BLOCK_SIZE * NAND_ZONE_SIZE * NAND_MAX_ZONE;		
//				*block_num = NAND_FormatCapacity()/512;		/* 必须为可用的扇区个数，不是芯片的理论容量 */
//			#else
//				*block_size =  NAND_PAGE_SIZE;
//				*block_num =  NAND_BLOCK_SIZE * NAND_ZONE_SIZE * NAND_MAX_ZONE;
//			#endif
//			ret = 0;	
			break;
			
		case LUN_SDRAM:
			*block_num =  SDRAM_DISK_SIZE / 512 - 1;
			*block_size = 512;
			ret = 0;			
			break;
	}	
	return ret; 
}

/**
  * @brief  Checks whether the medium is ready.  
  * @param  lun: Logical unit number
  * @retval Status (0: OK / -1: Error)
  */
int8_t STORAGE_IsReady(uint8_t lun)
{
	int8_t ret = -1;
	switch (lun)
	{
		case LUN_SD:	
			{
				static int8_t prev_status = 0;

				if (BSP_SD_IsDetected() != SD_NOT_PRESENT)
				{
					if (prev_status < 0)
					{
						BSP_SD_Init();
						prev_status = 0;
					}

					if (BSP_SD_GetCardState() == SD_TRANSFER_OK)
					{
						ret = 0;
					}
				}
				else if (prev_status == 0)
				{
					prev_status = -1;
				}
			}
			break;

		case LUN_NAND:
			ret = 0;
			break;
		
		case LUN_SDRAM:
			ret = 0;
			break;
	}
	return ret;
}

/**
  * @brief  Checks whether the medium is write protected.
  * @param  lun: Logical unit number
  * @retval Status (0: write enabled / -1: otherwise)
  */
int8_t STORAGE_IsWriteProtected(uint8_t lun)
{
  return 0;
}

/**
  * @brief  Reads data from the medium.
  * @param  lun: Logical unit number
  * @param  blk_addr: Logical block address
  * @param  blk_len: Blocks number
  * @retval Status (0: OK / -1: Error)
  */
int8_t STORAGE_Read(uint8_t lun, uint8_t * buf, uint32_t blk_addr,
                    uint16_t blk_len)
{
	int8_t ret = -1;
	switch (lun)
	{
		case LUN_SD:	
			{
				if (BSP_SD_IsDetected() != SD_NOT_PRESENT)
				{
					BSP_SD_ReadBlocks((uint32_t *) buf, blk_addr, blk_len, 1000);

					/* Wait until SD card is ready to use for new operation */
					while (BSP_SD_GetCardState() != SD_TRANSFER_OK)
					{
					}

					ret = 0;
				}
			}
			break;

		case LUN_NAND:
			break;
			
		case LUN_SDRAM:
			break;
	}
	return ret;
}

/**
  * @brief  Writes data into the medium.
  * @param  lun: Logical unit number
  * @param  blk_addr: Logical block address
  * @param  blk_len: Blocks number
  * @retval Status (0 : OK / -1 : Error)
  */
int8_t STORAGE_Write(uint8_t lun, uint8_t * buf, uint32_t blk_addr,
                     uint16_t blk_len)
{
	int8_t ret = -1;
	switch (lun)
	{
		case LUN_SD:	
			{
				if (BSP_SD_IsDetected() != SD_NOT_PRESENT)
				{
					BSP_SD_WriteBlocks((uint32_t *) buf, blk_addr, blk_len, 1000);

					/* Wait until SD card is ready to use for new operation */
					while (BSP_SD_GetCardState() != SD_TRANSFER_OK)
					{
					}

					ret = 0;
				}
			}
			break;

		case LUN_NAND:		
			break;
			
		case LUN_SDRAM:
			break;
	}
	return ret;
}

/**
  * @brief  Returns the Max Supported LUNs.   
  * @param  None
  * @retval Lun(s) number
  */
int8_t STORAGE_GetMaxLun(void)
{
  return (STORAGE_LUN_NBR - 1);
}


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
