/*
*********************************************************************************************************
*
*	ģ������ : QSPI Flash��д��ʾ����
*	�ļ����� : demo_qspi_flash.c
*	��    �� : V1.0
*	˵    �� : ������STM32-V7����������QSPI Flash�ͺ�Ϊ W25Q256JV, 32M�ֽ�
*
*	�޸ļ�¼ :
*		�汾��  ����        ����     ˵��
*		V1.0    2020-11-01 Eric2013  ��ʽ����
*
*	Copyright (C), 2020-2030, ���������� www.armfly.com
*
*********************************************************************************************************
*/
#include "demo_spi_flash.h"
#include "bsp.h"

#define TEST_ADDR		0		/* ��д���Ե�ַ */
#define TEST_SIZE		1024	/* ��д�������ݴ�С */

/* �������ļ��ڵ��õĺ������� */
static void sfDispMenu(void);
static void sfReadTest(void);
static void sfWriteTest(void);
static void sfErase(void);
static void sfViewData(uint32_t _uiAddr);
static void sfWriteAll(uint8_t _ch);
static void sfTestReadSpeed(void);

ALIGN_32BYTES(uint8_t buf[TEST_SIZE]);

ALIGN_32BYTES(uint8_t SpeedTestbuf[16*1024]); /* �����ڶ��ٶȲ���Ŀ�� */

/*
*********************************************************************************************************
*	�� �� ��: DemoSpiFlash
*	����˵��: QSPI��д����
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void DemoSpiFlash(void)
{
	uint8_t cmd;
	uint32_t uiReadPageNo = 0, id;

	/* ��⴮��Flash OK */
	id = QSPI_ReadID();
	printf("��⵽����Flash, ID = %08X, �ͺ�: WM25Q256JV\r\n", id);
	printf(" ���� : 32M�ֽ�, ������С : 4096�ֽ�, ҳ��С��256�ֽ�\r\n");

	sfDispMenu();		/* ��ӡ������ʾ */
    
    bsp_StartAutoTimer(0, 200); /* ����1��200ms���Զ���װ�Ķ�ʱ���������ʱ��0 */
	while(1)
	{
		bsp_Idle();		/* ���������bsp.c�ļ����û������޸��������ʵ��CPU���ߺ�ι�� */
		
        /* �ж������ʱ��0�Ƿ�ʱ */
        if(bsp_CheckTimer(0))
        {
            /* ÿ��200ms ����һ�� */  
            bsp_LedToggle(2);
        }
        
		if (comGetChar(COM1, &cmd))	/* �Ӵ��ڶ���һ���ַ�(��������ʽ) */
		{
			switch (cmd)
			{
				case '1':
					printf("\r\n��1 - ��QSPI Flash, ��ַ:0x%X ,����:%d�ֽڡ�\r\n", TEST_ADDR, TEST_SIZE);
					sfReadTest();	/* ������Flash���ݣ�����ӡ������������ */
					break;

				case '2':
					printf("\r\n��2 - дQSPFlash, ��ַ:0x%X,����:%d�ֽڡ�\r\n", TEST_ADDR, TEST_SIZE);
					sfWriteTest();	/* д����Flash���ݣ�����ӡд���ٶ� */
					break;

				case '3':
					printf("\r\n��3 - дQSPI Flashǰ10KB�ռ�, ȫ0x55��\r\n");
					sfWriteAll(0x55);/* ��������Flash���ݣ�ʵ���Ͼ���д��ȫ0xFF */
					break;

				case '4':
					printf("\r\n��4 - ������QSPI Flash, %dM�ֽڡ�\r\n", QSPI_FLASH_SIZES/(1024*1024));
					sfTestReadSpeed(); /* ����������Flash���ݣ������ٶ� */
					break;
				
				case 'y':
				case 'Y':
					printf("\r\n��Y - ��������QSPI Flash��\r\n");
					printf("����Flash������ϴ����Ҫ300�����ң������ĵȴ�");
					sfErase();		/* ��������Flash���ݣ�ʵ���Ͼ���д��ȫ0xFF */
					break;

				case 'z':
				case 'Z': /* ��ȡǰ1K */
					if (uiReadPageNo > 0)
					{
						uiReadPageNo--;
					}
					else
					{
						printf("�Ѿ�����ǰ\r\n");
					}
					sfViewData(uiReadPageNo * 1024);
					break;

				case 'x':
				case 'X': /* ��ȡ��1K */
					if (uiReadPageNo < QSPI_FLASH_SIZES / 1024 - 1)
					{
						uiReadPageNo++;
					}
					else
					{
						printf("�Ѿ������\r\n");
					}
					sfViewData(uiReadPageNo * 1024);
					break;

				default:
					sfDispMenu();	/* ��Ч������´�ӡ������ʾ */
					break;
			}
		}
	}
}

/*
*********************************************************************************************************
*	�� �� ��: sfReadTest
*	����˵��: ������Flash����
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void sfReadTest(void)
{
	uint32_t i;

	/* �建����Ϊ0x55 */
	memset(buf, 0x55, TEST_SIZE);
	QSPI_ReadBuffer(buf, 0, 1);
	printf("��ȡ��1���ֽڣ���������\r\n");
	printf(" %02X \r\n", buf[0]);
	
	/* ��ջ�����ΪAA */
	memset(buf, 0xAA, TEST_SIZE);
	QSPI_ReadBuffer(buf, 0, 10);
	printf("��ȡǰ10���ֽڣ���������\r\n");	
	for (i = 0; i < 10; i++)
	{
		printf(" %02X", buf[i]);
	}
	printf("\r\n");
	
	/* ��ջ����� */
	memset(buf, 0, TEST_SIZE);
	QSPI_ReadBuffer(buf, 0, TEST_SIZE);
	printf("������Flash�ɹ�����������\r\n");

	/* ��ӡ���� */
	for (i = 0; i < TEST_SIZE; i++)
	{
		printf(" %02X", buf[i]);

		if ((i & 31) == 31)
		{
			printf("\r\n");	/* ÿ����ʾ16�ֽ����� */
		}
		else if ((i & 31) == 15)
		{
			printf(" - ");
		}
	}
}

/*
*********************************************************************************************************
*	�� �� ��: sfWriteTest
*	����˵��: д����Flash����
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void sfWriteTest(void)
{
	uint32_t i;
	int32_t iTime1, iTime2;

	/* �����Ի����� */
	for (i = 0; i < TEST_SIZE; i++)
	{
		buf[i] = i;
	}

	QSPI_EraseSector(TEST_ADDR);
	
	iTime1 = bsp_GetRunTime();	/* ���¿�ʼʱ�� */
	for(i = 0; i< TEST_SIZE; i += QSPI_PAGE_SIZE)
	{
		if (QSPI_WriteBuffer(buf, TEST_ADDR + i, QSPI_PAGE_SIZE) == 0)
		{
			printf("д����Flash����\r\n");
			return;
		}		
	}
	
	iTime2 = bsp_GetRunTime();	/* ���½���ʱ�� */
	printf("д����Flash�ɹ���\r\n");
	
	/* ��ӡ���ٶ� */
	printf("���ݳ���: %d�ֽ�, д��ʱ: %dms, д�ٶ�: %dB/s\r\n", TEST_SIZE, iTime2 - iTime1, (TEST_SIZE * 1000) / (iTime2 - iTime1));
}

/*
*********************************************************************************************************
*	�� �� ��: sfTestReadSpeed
*	����˵��: ���Դ���Flash���ٶȡ���ȡ��������Flash�����ݣ�����ӡ���
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void sfTestReadSpeed(void)
{
	uint32_t i;
	int32_t iTime1, iTime2;
	uint32_t uiAddr;

	
	iTime1 = bsp_GetRunTime();	/* ���¿�ʼʱ�� */
	uiAddr = 0;
	for (i = 0; i < QSPI_FLASH_SIZES / (16*1024); i++, uiAddr += 16*1024)
	{
		QSPI_ReadBuffer(SpeedTestbuf, uiAddr, 16*1024);
	}
	iTime2 = bsp_GetRunTime();	/* ���½���ʱ�� */

	/* ��ӡ���ٶ� */
	printf("���ݳ���: %d�ֽ�, ����ʱ: %dms, ���ٶ�: %d KB/s\r\n", QSPI_FLASH_SIZES, iTime2 - iTime1, QSPI_FLASH_SIZES / (iTime2 - iTime1));
}

/*
*********************************************************************************************************
*	�� �� ��: sfWriteAll
*	����˵��: дQSPIȫ������
*	��    �Σ�_ch : д�������
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void sfWriteAll(uint8_t _ch)
{
	uint32_t i;
	int32_t iTime1, iTime2;
	uint32_t uiAddr;

	/* �����Ի����� */
	for (i = 0; i < TEST_SIZE; i++)
	{
		buf[i] = _ch;
	}
	
	/* �Ȳ���ǰ12KB�ռ� */
	QSPI_EraseSector(0);
	QSPI_EraseSector(4096);
	QSPI_EraseSector(8192);
	
	iTime1 = bsp_GetRunTime();	/* ���¿�ʼʱ�� */
	uiAddr = 0;
	for (i = 0; i < 10*1024/QSPI_PAGE_SIZE; i++, uiAddr += QSPI_PAGE_SIZE)
	{
		
		QSPI_WriteBuffer(buf, uiAddr, QSPI_PAGE_SIZE);
		printf(".");
		if (((i + 1) % 128) == 0)
		{
			printf("\r\n");
		}
	}
	
	iTime2 = bsp_GetRunTime();	/* ���½���ʱ�� */
	printf("\r\n");
	
	/* ��ӡ���ٶ� */
	printf("���ݳ���: %d�ֽ�, д��ʱ: %dms, д�ٶ�: %dKB/s\r\n", 10*1024, iTime2 - iTime1, (10 * 1024) / (iTime2 - iTime1));
}

/*
*********************************************************************************************************
*	�� �� ��: sfErase
*	����˵��: ��������Flash
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void sfErase(void)
{
	uint32_t i;
	int32_t iTime1, iTime2;
	uint32_t uiAddr;
	
	
	iTime1 = bsp_GetRunTime();	/* ���¿�ʼʱ�� */
	
	uiAddr = 0;
	printf("\r\n");
	for (i = 0; i < QSPI_FLASH_SIZES / QSPI_SECTOR_SIZE; i++, uiAddr += QSPI_SECTOR_SIZE)
	{
		QSPI_EraseSector(uiAddr);
		
		printf(".");
		if (((i + 1) % 128) == 0)
		{
			printf("\r\n");
		}
	}
	
	iTime2 = bsp_GetRunTime();	/* ���½���ʱ�� */

	/* ��ӡ���ٶ� */
	printf("��������Flash��ɣ�, ��ʱ: %dms\r\n", iTime2 - iTime1);
	return;
}

/*
*********************************************************************************************************
*	�� �� ��: sfViewData
*	����˵��: ������Flash����ʾ��ÿ����ʾ1K������
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void sfViewData(uint32_t _uiAddr)
{
	uint32_t i;

	QSPI_ReadBuffer(buf, _uiAddr,  1024);		/* ������ */
	printf("��ַ��0x%08X; ���ݳ��� = 1024\r\n", _uiAddr);

	/* ��ӡ���� */
	for (i = 0; i < 1024; i++)
	{
		printf(" %02X", buf[i]);

		if ((i & 31) == 31)
		{
			printf("\r\n");	/* ÿ����ʾ16�ֽ����� */
		}
		else if ((i & 31) == 15)
		{
			printf(" - ");
		}
	}
}

/*
*********************************************************************************************************
*	�� �� ��: sfDispMenu
*	����˵��: ��ʾ������ʾ�˵�
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void sfDispMenu(void)
{
	printf("\r\n*******************************************\r\n");
	printf("��ѡ���������:\r\n");
	printf("��1 - ��QSPI Flash, ��ַ:0x%X,����:%d�ֽڡ�\r\n", TEST_ADDR, TEST_SIZE);
	printf("��2 - дQSPI Flash, ��ַ:0x%X,����:%d�ֽڡ�\r\n", TEST_ADDR, TEST_SIZE);
	printf("��3 - дQSPI Flashǰ10KB�ռ�, ȫ0x55��\r\n");
	printf("��4 - ����������Flash, ���Զ��ٶȡ�\r\n");
	printf("��Z - ��ȡǰ1K����ַ�Զ����١�\r\n");
	printf("��X - ��ȡ��1K����ַ�Զ����ӡ�\r\n");
	printf("��Y - ������������Flash����Ƭ32MB�������300�����ҡ�\r\n");
	printf("��������� - ��ʾ������ʾ\r\n");
	printf("\r\n");
}

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
