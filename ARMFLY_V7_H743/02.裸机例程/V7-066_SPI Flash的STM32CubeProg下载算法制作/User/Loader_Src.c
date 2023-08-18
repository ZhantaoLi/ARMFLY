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


#define SPI_FLASH_MEM_ADDR         0xC0000000

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
    int result = 0;

    /* 系统初始化 */
    SystemInit(); 

    /* 时钟初始化 */
    result = SystemClock_Config();
    if (result == 1)
    {
        return 0;
    }

    /* SPI Flash初始化 */
    bsp_InitSPIBus();
    bsp_InitSFlash();

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
    Address -= SPI_FLASH_MEM_ADDR;

    sf_WriteBuffer(buffer, Address, Size);
    
    return 1; 
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
    uint32_t BlockAddr;
    
    EraseStartAddress -= SPI_FLASH_MEM_ADDR;
    EraseEndAddress -= SPI_FLASH_MEM_ADDR;
    EraseStartAddress = EraseStartAddress -  EraseStartAddress % 0x1000; /* 4KB首地址 */
    
    while (EraseEndAddress >= EraseStartAddress)
    {
        BlockAddr = EraseStartAddress & 0x0FFFFFFF;

        sf_EraseSector(BlockAddr);    

        EraseStartAddress += 0x1000;
    }

    return 1;
}

/*
*********************************************************************************************************
*	函 数 名: Read
*	功能说明: 从SPI Flash读取数据
*	形    参: Address 读取地址
*             Size   读取大小，单位字节
*             buffer 读取存放的数据缓冲
*	返 回 值: 1 表示成功，0表示失败
*********************************************************************************************************
*/
int Read(uint32_t Address, uint32_t Size, uint8_t* Buffer)
{ 
    Address -= SPI_FLASH_MEM_ADDR;
    sf_ReadBuffer(Buffer, Address, Size);

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
//unsigned char aux_buf[4096];
//int Verify(uint32_t MemoryAddr, uint32_t RAMBufferAddr, uint32_t Size)
//{   
//    int i;

//    uint8_t *buf = (uint8_t *)RAMBufferAddr;
//    MemoryAddr -= SPI_FLASH_MEM_ADDR;
//    Size*=4;

//    sf_ReadBuffer(aux_buf, MemoryAddr, Size);

//    for (i = 0; i< Size; i++) 
//    {
//        if (aux_buf[i] != buf[i]) 
//        return (0);                   
//    }

//    return (1);
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
    sf_EraseChip();

    return 1;   
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/

