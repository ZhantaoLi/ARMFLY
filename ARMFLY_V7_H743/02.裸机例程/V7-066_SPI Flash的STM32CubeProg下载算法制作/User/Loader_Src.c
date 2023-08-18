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


#define SPI_FLASH_MEM_ADDR         0xC0000000

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
    int result = 0;

    /* ϵͳ��ʼ�� */
    SystemInit(); 

    /* ʱ�ӳ�ʼ�� */
    result = SystemClock_Config();
    if (result == 1)
    {
        return 0;
    }

    /* SPI Flash��ʼ�� */
    bsp_InitSPIBus();
    bsp_InitSFlash();

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
    Address -= SPI_FLASH_MEM_ADDR;

    sf_WriteBuffer(buffer, Address, Size);
    
    return 1; 
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
    uint32_t BlockAddr;
    
    EraseStartAddress -= SPI_FLASH_MEM_ADDR;
    EraseEndAddress -= SPI_FLASH_MEM_ADDR;
    EraseStartAddress = EraseStartAddress -  EraseStartAddress % 0x1000; /* 4KB�׵�ַ */
    
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
*	�� �� ��: Read
*	����˵��: ��SPI Flash��ȡ����
*	��    ��: Address ��ȡ��ַ
*             Size   ��ȡ��С����λ�ֽ�
*             buffer ��ȡ��ŵ����ݻ���
*	�� �� ֵ: 1 ��ʾ�ɹ���0��ʾʧ��
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
*	�� �� ��: Verify
*	����˵��: У�飬δʹ�ã�STM32CubeProg���Զ�������У��
*	��    ��: MemoryAddr    ���ݵ�ַ
*             RAMBufferAddr RAM���ݻ����ַ
*             Size ��С�Ƚ�
*	�� �� ֵ: 1 ��ʾ�ɹ���0��ʾʧ��
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
*	�� �� ��: MassErase
*	����˵��: ����оƬ����
*	��    ��: ��
*	�� �� ֵ: 1 ��ʾ�ɹ���0��ʾʧ��
*********************************************************************************************************
*/
int MassErase(void)
{
    sf_EraseChip();

    return 1;   
}

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/

