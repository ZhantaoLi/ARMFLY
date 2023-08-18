/*
*********************************************************************************************************
*
*	模块名称 : Flash算法接口
*	文件名称 : Loader_Src.c
*	版    本 : V1.0
*	说    明 : Flash编程接口
*
*	修改记录 :
*		版本号  日期         作者       说明
*		V1.0    2020-11-06  Eric2013   正式发布
*
*	Copyright (C), 2020-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/
#include "bsp.h"



/*
*********************************************************************************************************
*	函 数 名: Init
*	功能说明: Flash编程初始化
*	形    参: 无
*	返 回 值: 0 表示失败， 1表示成功
*********************************************************************************************************
*/
int Init(void)
{   
    int result = 1;
 
    /* 系统初始化 */
    SystemInit(); 

    /* 时钟初始化 */
    result = SystemClock_Config();
    if (result == 1)
    {
        return 0;        
    }

    /* W25Q256初始化 */
    result = bsp_InitQSPI_W25Q256();
    if (result == 1)
    {
        return 0;
    }
    
    /* 内存映射 */    
    result = QSPI_MemoryMapped(); 
    if (result == 1)
    {
        return 0;
    }

    return 1;
}

/*
*********************************************************************************************************
*	函 数 名: Write
*	功能说明: 写数据到Device
*	形    参: Address 写入地址
*             Size   写入大小，单位字节
*             buffer 要写入的数据地址
*	返 回 值: 1 表示成功，0表示失败
*********************************************************************************************************
*/
int Write(uint32_t Address, uint32_t Size, uint8_t* buffer)
{ 
    int result = 0;
    uint32_t end_addr, current_size, current_addr;
    

    if (Address < QSPI_FLASH_MEM_ADDR || Address >= QSPI_FLASH_MEM_ADDR + QSPI_FLASH_SIZES)
    {
        return 0;
    }
    
    Address -= QSPI_FLASH_MEM_ADDR;

    /* 计算写入地址和页末的字节大小 */
    current_size = QSPI_PAGE_SIZE - (Address % QSPI_PAGE_SIZE);

    /* 检测页剩余空间是否够用 */
    if (current_size > Size)
    {
        current_size = Size;
    }
    
    /* W25Q256初始化 */
    result = bsp_InitQSPI_W25Q256();
    if (result == 1)
    {
        return 0;
    }

    current_addr = Address;
    end_addr = Address + Size;

    do{
        if (QSPI_WriteBuffer(buffer, current_addr, current_size) == 1)
        {
            QSPI_MemoryMapped();  
            return 0;  
        }

        /* 更新地址 */
        current_addr += current_size;
        buffer += current_size;
        current_size = ((current_addr + QSPI_PAGE_SIZE) > end_addr) ? (end_addr - current_addr) : QSPI_PAGE_SIZE;
        
    }while(current_addr < end_addr);

    /* 内存映射 */    
    result = QSPI_MemoryMapped(); 
    if (result == 1)
    {
        return 0;
    }

    return (1);  
}

/*
*********************************************************************************************************
*	函 数 名: SectorErase
*	功能说明: EraseStartAddress 擦除起始地址
*             EraseEndAddress   擦除结束地址
*	形    参: adr 擦除地址
*	返 回 值: 1 表示成功，0表示失败
*********************************************************************************************************
*/
int SectorErase (uint32_t EraseStartAddress ,uint32_t EraseEndAddress)
{
    uint32_t BlockAddr, result;
    
    EraseStartAddress -= QSPI_FLASH_MEM_ADDR;
    EraseEndAddress -= QSPI_FLASH_MEM_ADDR;
    EraseStartAddress = EraseStartAddress -  EraseStartAddress % 0x10000; /* 64KB首地址 */
    
    /* W25Q256初始化 */
    result = bsp_InitQSPI_W25Q256();
    if (result == 1)
    {
        return 0;        
    }

    while (EraseEndAddress >= EraseStartAddress)
    {
        /* 防止超出256MB空间 */
        BlockAddr = EraseStartAddress & 0x0FFFFFFF;

        result = QSPI_EraseSector(BlockAddr);    
        if (result == 1)
        {
            QSPI_MemoryMapped(); 
            return 0;        
        }

        EraseStartAddress += 0x10000;
    }

    /* 内存映射 */    
    result = QSPI_MemoryMapped(); 
    if (result == 1)
    {
        return 0;
    }

    return 1; 
}

/*
*********************************************************************************************************
*	函 数 名: Verify
*	功能说明: 校验，未使用，STM32CubeProg会自动做读回校验
*	形    参: MemoryAddr    数据地址
*             RAMBufferAddr RAM数据缓冲地址
*             Size 大小比较
*	返 回 值: 1 表示成功，0表示失败
*********************************************************************************************************
*/
//int Verify(uint32_t MemoryAddr, uint32_t RAMBufferAddr, uint32_t Size)
//{   
//    return 1;
//}

/*
*********************************************************************************************************
*	函 数 名: MassErase
*	功能说明: 整个芯片擦除
*	形    参: 无
*	返 回 值: 1 表示成功，0表示失败
*********************************************************************************************************
*/
int MassErase(void)
{
    int result = 1;
    
    /* W25Q256初始化 */
    result = bsp_InitQSPI_W25Q256();
    if (result == 1)
    {
        return 0;
    }
                                             
    result = QSPI_EraseChip(); 
    if (result == 1)
    {
        return 0;
    }    

    /* 内存映射 */    
    result = QSPI_MemoryMapped(); 
    if (result == 1)
    {
        return 0;
    }
    
    return result;          
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/

