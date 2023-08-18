/*
*********************************************************************************************************
*
*	ģ������ : Flash�㷨�ӿ�
*	�ļ����� : Loader_Src.c
*	��    �� : V1.0
*	˵    �� : Flash��̽ӿ�
*
*	�޸ļ�¼ :
*		�汾��  ����         ����       ˵��
*		V1.0    2020-11-06  Eric2013   ��ʽ����
*
*	Copyright (C), 2020-2030, ���������� www.armfly.com
*
*********************************************************************************************************
*/
#include "bsp.h"



/*
*********************************************************************************************************
*	�� �� ��: Init
*	����˵��: Flash��̳�ʼ��
*	��    ��: ��
*	�� �� ֵ: 0 ��ʾʧ�ܣ� 1��ʾ�ɹ�
*********************************************************************************************************
*/
int Init(void)
{   
    int result = 1;
 
    /* ϵͳ��ʼ�� */
    SystemInit(); 

    /* ʱ�ӳ�ʼ�� */
    result = SystemClock_Config();
    if (result == 1)
    {
        return 0;        
    }

    /* W25Q256��ʼ�� */
    result = bsp_InitQSPI_W25Q256();
    if (result == 1)
    {
        return 0;
    }
    
    /* �ڴ�ӳ�� */    
    result = QSPI_MemoryMapped(); 
    if (result == 1)
    {
        return 0;
    }

    return 1;
}

/*
*********************************************************************************************************
*	�� �� ��: Write
*	����˵��: д���ݵ�Device
*	��    ��: Address д���ַ
*             Size   д���С����λ�ֽ�
*             buffer Ҫд������ݵ�ַ
*	�� �� ֵ: 1 ��ʾ�ɹ���0��ʾʧ��
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

    /* ����д���ַ��ҳĩ���ֽڴ�С */
    current_size = QSPI_PAGE_SIZE - (Address % QSPI_PAGE_SIZE);

    /* ���ҳʣ��ռ��Ƿ��� */
    if (current_size > Size)
    {
        current_size = Size;
    }
    
    /* W25Q256��ʼ�� */
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

        /* ���µ�ַ */
        current_addr += current_size;
        buffer += current_size;
        current_size = ((current_addr + QSPI_PAGE_SIZE) > end_addr) ? (end_addr - current_addr) : QSPI_PAGE_SIZE;
        
    }while(current_addr < end_addr);

    /* �ڴ�ӳ�� */    
    result = QSPI_MemoryMapped(); 
    if (result == 1)
    {
        return 0;
    }

    return (1);  
}

/*
*********************************************************************************************************
*	�� �� ��: SectorErase
*	����˵��: EraseStartAddress ������ʼ��ַ
*             EraseEndAddress   ����������ַ
*	��    ��: adr ������ַ
*	�� �� ֵ: 1 ��ʾ�ɹ���0��ʾʧ��
*********************************************************************************************************
*/
int SectorErase (uint32_t EraseStartAddress ,uint32_t EraseEndAddress)
{
    uint32_t BlockAddr, result;
    
    EraseStartAddress -= QSPI_FLASH_MEM_ADDR;
    EraseEndAddress -= QSPI_FLASH_MEM_ADDR;
    EraseStartAddress = EraseStartAddress -  EraseStartAddress % 0x10000; /* 64KB�׵�ַ */
    
    /* W25Q256��ʼ�� */
    result = bsp_InitQSPI_W25Q256();
    if (result == 1)
    {
        return 0;        
    }

    while (EraseEndAddress >= EraseStartAddress)
    {
        /* ��ֹ����256MB�ռ� */
        BlockAddr = EraseStartAddress & 0x0FFFFFFF;

        result = QSPI_EraseSector(BlockAddr);    
        if (result == 1)
        {
            QSPI_MemoryMapped(); 
            return 0;        
        }

        EraseStartAddress += 0x10000;
    }

    /* �ڴ�ӳ�� */    
    result = QSPI_MemoryMapped(); 
    if (result == 1)
    {
        return 0;
    }

    return 1; 
}

/*
*********************************************************************************************************
*	�� �� ��: Verify
*	����˵��: У�飬δʹ�ã�STM32CubeProg���Զ�������У��
*	��    ��: MemoryAddr    ���ݵ�ַ
*             RAMBufferAddr RAM���ݻ����ַ
*             Size ��С�Ƚ�
*	�� �� ֵ: 1 ��ʾ�ɹ���0��ʾʧ��
*********************************************************************************************************
*/
//int Verify(uint32_t MemoryAddr, uint32_t RAMBufferAddr, uint32_t Size)
//{   
//    return 1;
//}

/*
*********************************************************************************************************
*	�� �� ��: MassErase
*	����˵��: ����оƬ����
*	��    ��: ��
*	�� �� ֵ: 1 ��ʾ�ɹ���0��ʾʧ��
*********************************************************************************************************
*/
int MassErase(void)
{
    int result = 1;
    
    /* W25Q256��ʼ�� */
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

    /* �ڴ�ӳ�� */    
    result = QSPI_MemoryMapped(); 
    if (result == 1)
    {
        return 0;
    }
    
    return result;          
}

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/

