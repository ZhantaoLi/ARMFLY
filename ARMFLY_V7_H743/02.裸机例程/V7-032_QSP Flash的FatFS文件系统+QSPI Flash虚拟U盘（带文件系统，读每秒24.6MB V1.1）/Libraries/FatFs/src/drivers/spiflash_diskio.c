/**
  ******************************************************************************
  * @file    ppp_diskio.c
  * @author  MCD Application Team
  * @version V2.0.2
  * @date    10-November-2017
  * @brief   PPP Disk I/O driver generic driver template
             this driver is not functional and is intended to show
	     how to implement a FatFs diskio driver.
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

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include "ff_gen_drv.h"
#include "bsp.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Block Size in Bytes */
/* Private variables ---------------------------------------------------------*/
/* Disk status */
static volatile DSTATUS Stat = STA_NOINIT;

/* Private function prototypes -----------------------------------------------*/
DSTATUS SPIFlash_initialize (BYTE);
DSTATUS SPIFlash_status (BYTE);
DRESULT SPIFlash_read (BYTE, BYTE*, DWORD, UINT);
#if _USE_WRITE == 1
  DRESULT SPIFlash_write (BYTE, const BYTE*, DWORD, UINT);
#endif /* _USE_WRITE == 1 */
#if _USE_IOCTL == 1
  DRESULT SPIFlash_ioctl (BYTE, BYTE, void*);
#endif  /* _USE_IOCTL == 1 */

const Diskio_drvTypeDef  SPIFlash_Driver =
{
  SPIFlash_initialize,
  SPIFlash_status,
  SPIFlash_read,
#if  _USE_WRITE == 1
  SPIFlash_write,
#endif /* _USE_WRITE == 1 */

#if  _USE_IOCTL == 1
  SPIFlash_ioctl,
#endif /* _USE_IOCTL == 1 */
};

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Initializes a Drive
  * @param  lun : not used
  * @retval DSTATUS: Operation status
  */
DSTATUS SPIFlash_initialize(BYTE lun)
{
	Stat = RES_OK;

  /* Configure the uPPP device */
   // bsp_InitQSPI_W25Q256() //在bsp.C文件里面已经初始化

  return Stat;
}

/**
  * @brief  Gets Disk Status
  * @param  lun : not used
  * @retval DSTATUS: Operation status
  */
DSTATUS SPIFlash_status(BYTE lun)
{
  Stat = RES_OK;

  return Stat;
}

/**
  * @brief  Reads Sector(s)
  * @param  lun : not used
  * @param  *buff: Data buffer to store read data
  * @param  sector: Sector address (LBA)
  * @param  count: Number of sectors to read (1..128)
  * @retval DRESULT: Operation result
  */
DRESULT SPIFlash_read(BYTE lun, BYTE *buff, DWORD sector, UINT count)
{
  DRESULT res = RES_OK;

  QSPI_ReadBuffer(buff, sector << 12, count<<12);
  return res;
}

/**
  * @brief  Writes Sector(s)
  * @param  lun : not used
  * @param  *buff: Data to be written
  * @param  sector: Sector address (LBA)
  * @param  count: Number of sectors to write (1..128)
  * @retval DRESULT: Operation result
  */
#if _USE_WRITE == 1
DRESULT SPIFlash_write(BYTE lun, const BYTE *buff, DWORD sector, UINT count)
{
	DRESULT res = RES_OK;
	uint32_t addr;
	uint16_t i;
	BYTE *p;

	p = (BYTE *)buff;
	for(i = 0; i < count; i++)
	{
		addr = (sector+i) << 12;
		QSPI_EraseSector(addr);

		QSPI_WriteBuffer(p + 256*0, addr + 256*0, 256);
		QSPI_WriteBuffer(p + 256*1, addr + 256*1, 256);
		QSPI_WriteBuffer(p + 256*2, addr + 256*2, 256);
		QSPI_WriteBuffer(p + 256*3, addr + 256*3, 256);	
		
		QSPI_WriteBuffer(p + 256*4, addr + 256*4, 256);
		QSPI_WriteBuffer(p + 256*5, addr + 256*5, 256);
		QSPI_WriteBuffer(p + 256*6, addr + 256*6, 256);
		QSPI_WriteBuffer(p + 256*7, addr + 256*7, 256);	
		
		QSPI_WriteBuffer(p + 256*8,  addr + 256*8, 256);
		QSPI_WriteBuffer(p + 256*9,  addr + 256*9, 256);
		QSPI_WriteBuffer(p + 256*10, addr + 256*10, 256);
		QSPI_WriteBuffer(p + 256*11, addr + 256*11, 256);	
		
		QSPI_WriteBuffer(p + 256*12, addr + 256*12, 256);
		QSPI_WriteBuffer(p + 256*13, addr + 256*13, 256);
		QSPI_WriteBuffer(p + 256*14, addr + 256*14, 256);
		QSPI_WriteBuffer(p + 256*15, addr + 256*15, 256);	
		
		p += 4096;					
	}

  return res;
}
#endif /* _USE_WRITE == 1 */

/**
  * @brief  I/O control operation
  * @param  lun : not used
  * @param  cmd: Control code
  * @param  *buff: Buffer to send/receive control data
  * @retval DRESULT: Operation result
  */
#if _USE_IOCTL == 1
DRESULT SPIFlash_ioctl(BYTE lun, BYTE cmd, void *buff)
{
  DRESULT res = RES_ERROR;

  switch (cmd)
  {
  /* Make sure that no pending write process */
  case CTRL_SYNC :
    res = RES_OK;
    break;

  /* Get number of sectors on the disk (DWORD) */
  case GET_SECTOR_COUNT :
    *(DWORD*)buff = 1024*8;
    res = RES_OK;
    break;

  /* Get R/W sector size (WORD) */
  case GET_SECTOR_SIZE :
    *(WORD*)buff = 4096;
    res = RES_OK;
    break;

  /* Get erase block size in unit of sector (DWORD) */
  case GET_BLOCK_SIZE :
     res = RES_OK;
    break;

  default:
    res = RES_PARERR;
  }

  return res;
}
#endif /* _USE_IOCTL == 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

