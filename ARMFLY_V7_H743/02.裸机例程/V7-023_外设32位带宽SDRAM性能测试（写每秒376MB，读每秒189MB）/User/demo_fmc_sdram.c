/*
*********************************************************************************************************
*
*	ģ������ : SDRAM���ܲ����ļ�
*	�ļ����� : demo_fmc_sdram.c
*	��    �� : V1.0
*	˵    �� : �ⲿ32λ����SDRAM���ܲ��ԡ�
*              1��SDRAM�ͺ�IS42S32800G-6BLI, 32λ����, ����32MB, 6ns�ٶ�(166MHz)��
*              2. K1�����£�����32MBд�ٶ�;
*              3. K2�����£�����32MB���ٶ�;
*              4. K3�����£���ȡ1024�ֽڲ���ӡ;
*              5. ҡ��OK�����£�����SDRAM���е�Ԫ�Ƿ����쳣;
*              6. ����Cache
*                ��1��ʹ��MDK��IAR�ĸ����Ż��ȼ����ԣ��Ż�����Ӱ���С��
*                ��2��д�ٶ�376MB/S�����ٶ�182MB/S��
*              7. �ر�Cache
*                ��1��ʹ��MDK��IAR�ĸ����Ż��ȼ����ԣ��Ż�����Ӱ���С��
*                ��2��д�ٶ�307MB/S�����ٶ�116MB/S��
*              8. IAR������ߵȼ��Ż������ٶ���189MB/S����MDK��182MB/S�ߵ㡣
*              9. ����MDK����ʵ�鿪������ߵȼ��Ż���ʱ���Ż���
*              10. ��IAR����ʵ�鿪������ߵȼ��ٶ��Ż���
*	�޸ļ�¼ :
*		�汾��   ����         ����        ˵��
*		V1.0    2018-12-12   Eric2013     1. CMSIS����汾 V5.4.0
*                                         2. HAL��汾 V1.3.0
*
*	Copyright (C), 2018-2030, ���������� www.armfly.com
*
*********************************************************************************************************
*/	
#include "bsp.h"			/* �ײ�Ӳ������ */



/* �����ļ��ڲ����ú��� */
static void PrintfHelp(void);
static void WriteSpeedTest(void);
static void ReadSpeedTest(void);
static void ReadWriteTest(void);

#define TEST_ADDRESS	0    /* �ӵ�ַ0��ʼ���� */
#define TEST_BUF_SIZE	256  /* ���Ի����С */

/*
*********************************************************************************************************
*	�� �� ��: DemoFmcSDRAM
*	����˵��: SDRAM��д���ܲ���
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void DemoFmcSDRAM(void)
{
	uint8_t ucKeyCode;	/* �������� */
	uint32_t err;
	
	PrintfHelp();	/* ��ӡ������ʾ */

	bsp_StartAutoTimer(0, 100);	/* ����1��100ms���Զ���װ�Ķ�ʱ�� */
	
	/* ����������ѭ���� */
	while (1)
	{
		bsp_Idle();		/* ���������bsp.c�ļ����û������޸��������ʵ��CPU���ߺ�ι�� */

		/* �ж϶�ʱ����ʱʱ�� */
		if (bsp_CheckTimer(0))	
		{
			/* ÿ��100ms ����һ�� */  
			bsp_LedToggle(2);
		}

		/* �����˲��ͼ���ɺ�̨systick�жϷ������ʵ�֣�����ֻ��Ҫ����bsp_GetKey��ȡ��ֵ���ɡ� */
		ucKeyCode = bsp_GetKey();	/* ��ȡ��ֵ, �޼�����ʱ���� KEY_NONE = 0 */
		if (ucKeyCode != KEY_NONE)
		{
			switch (ucKeyCode)
			{
				case KEY_DOWN_K1:			/* K1�����£�����32MBд�ٶ� */
					WriteSpeedTest();
					break;

				case KEY_DOWN_K2:			/* K2�����£�����32MB���ٶ�*/
					ReadSpeedTest();
					break;
				
				case KEY_DOWN_K3:			/* K3�����£���ȡ1024�ֽڲ���ӡ */
					ReadWriteTest();
					break;
				
				case JOY_DOWN_OK:			/* ҡ��OK�����£�����SDRAM���е�Ԫ�Ƿ����쳣*/
					err = bsp_TestExtSDRAM1();
					if (err == 0)
					{
						printf("�ⲿSDRAM����ͨ��\r\n");
					}
					else
					{
						printf("�ⲿSDRAM��������Ԫ������%d\r\n", err);
					}
					break;

				default:
					/* �����ļ�ֵ������ */
					break;
			}
		}
	}
}

/*
*********************************************************************************************************
*	�� �� ��: WriteSpeedTest
*	����˵��: дSDRAM�ٶȲ���
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void WriteSpeedTest(void)
{
	uint32_t start, end, cnt;
	uint32_t i, j;
	int32_t iTime1, iTime2;
	uint32_t *pBuf;


	/* ���ó�ʼ��ֵ�����¿�ʼʱ�� */
	j = 0;
	pBuf = (uint32_t *)EXT_SDRAM_ADDR;
	iTime1 = bsp_GetRunTime();	  
	start = DWT_CYCCNT;
	
	/* �Ե����ķ�ʽд���ݵ�SDRAM���пռ� */
	for (i = 1024*1024/4; i >0 ; i--)
	{
		*pBuf++ = j++;
		*pBuf++ = j++;
		*pBuf++ = j++;
		*pBuf++ = j++;
		*pBuf++ = j++;
		*pBuf++ = j++;
		*pBuf++ = j++;
		*pBuf++ = j++;	

		*pBuf++ = j++;
		*pBuf++ = j++;
		*pBuf++ = j++;
		*pBuf++ = j++;
		*pBuf++ = j++;
		*pBuf++ = j++;
		*pBuf++ = j++;
		*pBuf++ = j++;	

		*pBuf++ = j++;
		*pBuf++ = j++;
		*pBuf++ = j++;
		*pBuf++ = j++;
		*pBuf++ = j++;
		*pBuf++ = j++;
		*pBuf++ = j++;
		*pBuf++ = j++;	

		*pBuf++ = j++;
		*pBuf++ = j++;
		*pBuf++ = j++;
		*pBuf++ = j++;
		*pBuf++ = j++;
		*pBuf++ = j++;
		*pBuf++ = j++;
		*pBuf++ = j++;	
	}
	end = DWT_CYCCNT;
	cnt = end - start;
	iTime2 = bsp_GetRunTime();	/* ���½���ʱ�� */
	
    /* ��ȡд����Ƿ���� */
	j = 0;
	pBuf = (uint32_t *)EXT_SDRAM_ADDR;
	for (i = 0; i < 1024*1024*8; i++)
	{
		if(*pBuf++ != j++)
		{
			printf("д����� j=%d\r\n", j);
			break;
		}
	}
		
	/* ��ӡ�ٶ� */
	printf("��32MB����д��ʱ��: ��ʽһ:%dms  ��ʽ��:%dms, д�ٶ�: %dMB/s\r\n", 
	                  iTime2 - iTime1,  cnt/400000, (EXT_SDRAM_SIZE / 1024 /1024 * 1000) / (iTime2 - iTime1));
}

/*
*********************************************************************************************************
*	�� �� ��: ReadSpeedTest
*	����˵��: ��SDRAM�ٶȲ���
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void ReadSpeedTest(void)
{	
	uint32_t start, end, cnt;
	uint32_t i;
	int32_t iTime1, iTime2;
	uint32_t *pBuf;
	__IO  uint32_t ulTemp; /* ����Ϊ__IO���ͣ���ֹ��MDK�Ż� */

	/* ���ó�ʼ��ֵ�����¿�ʼʱ�� */
	pBuf = (uint32_t *)EXT_SDRAM_ADDR;
	iTime1 = bsp_GetRunTime();	
	start = DWT_CYCCNT;
	
	/* ��ȡSDRAM���пռ����� */	
	for (i = 1024*1024/4; i >0 ; i--)
	{
		ulTemp = *pBuf++;
		ulTemp = *pBuf++;
		ulTemp = *pBuf++;
		ulTemp = *pBuf++;
		ulTemp = *pBuf++;
		ulTemp = *pBuf++;
		ulTemp = *pBuf++;
		ulTemp = *pBuf++;

		ulTemp = *pBuf++;
		ulTemp = *pBuf++;
		ulTemp = *pBuf++;
		ulTemp = *pBuf++;
		ulTemp = *pBuf++;
		ulTemp = *pBuf++;
		ulTemp = *pBuf++;
		ulTemp = *pBuf++;
		
		ulTemp = *pBuf++;
		ulTemp = *pBuf++;
		ulTemp = *pBuf++;
		ulTemp = *pBuf++;
		ulTemp = *pBuf++;
		ulTemp = *pBuf++;
		ulTemp = *pBuf++;
		ulTemp = *pBuf++;
		
		ulTemp = *pBuf++;
		ulTemp = *pBuf++;
		ulTemp = *pBuf++;
		ulTemp = *pBuf++;
		ulTemp = *pBuf++;
		ulTemp = *pBuf++;
		ulTemp = *pBuf++;
		ulTemp = *pBuf++;
	}
	end = DWT_CYCCNT;
	cnt = end - start;
	iTime2 = bsp_GetRunTime();	/* ���½���ʱ�� */

	/* ��ӡ�ٶ� */
	printf("��32MB���ݶ���ʱ��: ��ʽһ:%dms  ��ʽ��:%dms, ���ٶ�: %dMB/s\r\n", 
	        iTime2 - iTime1,  cnt/400000, (EXT_SDRAM_SIZE / 1024 /1024 * 1000) / (iTime2 - iTime1));
}

/*
*********************************************************************************************************
*	�� �� ��: ReadWriteTest
*	����˵��: �����ݣ�����ӡ����
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void ReadWriteTest(void)
{
	uint32_t i;
	uint32_t *pBuf;
	

	/* д���������0xAAAA5555 */
	pBuf = (uint32_t *)(EXT_SDRAM_ADDR + TEST_ADDRESS);
	for (i = 0; i < TEST_BUF_SIZE; i++)		
	{
		pBuf[i] = 0xAAAA5555;
	}
	
	printf("�����ַ��0x%08X  ��С: %d�ֽ�  ��ʾ: %d�ֽ�  ��������: \r\n", EXT_SDRAM_ADDR + TEST_ADDRESS, EXT_SDRAM_SIZE, TEST_BUF_SIZE*4);
	
	/* ��ӡ���� */
	pBuf = (uint32_t *)(EXT_SDRAM_ADDR + TEST_ADDRESS);
	for (i = 0; i < TEST_BUF_SIZE; i++)
	{
		printf(" %04X", pBuf[i]);

		if ((i & 7) == 7)
		{
			printf("\r\n");		/* ÿ����ʾ32�ֽ����� */
		}
		else if ((i & 7) == 3)
		{
			printf(" - ");
		}
	}
}

/*
*********************************************************************************************************
*	�� �� ��: PrintfHelp
*	����˵��: ��ӡ������ʾ
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void PrintfHelp(void)
{
	printf("������ʾ:\r\n");	
	printf("SDRAM�ͺ�IS42S32800G-6BLI, 32λ����, ����32MB, 6ns�ٶ�(166MHz)\r\n");
	printf("1. K1�����£�����32MBд�ٶ�\r\n");
	printf("2. K2�����£�����32MB���ٶ�\r\n");
	printf("3. K3�����£���ȡ1024�ֽڲ���ӡ\r\n");
	printf("4. ҡ��OK�����£�����SDRAM���е�Ԫ�Ƿ����쳣\r\n");
}

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
