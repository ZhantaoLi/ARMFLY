/*
*********************************************************************************************************
*
*	ģ������ : DAC8501����
*	�ļ����� : demo_spi_dac8501.c
*	��    �� : V1.0
*	˵    �� : DAC8501���ԡ�
*
*	�޸ļ�¼ :
*		�汾��  ����        ����     ˵��
*		V1.0    2020-04-11 armfly  ��ʽ����
*
*	Copyright (C), 2020-2030, ���������� www.armfly.com
*
*********************************************************************************************************
*/
#include "demo_spi_dac8501.h"
#include "bsp.h"



uint16_t ch1buf[100]; 

/* �������ļ��ڵ��õĺ������� */
static void sfDispMenu(void);
static void MakeSinTable(uint16_t *_pBuf, uint16_t _usSamples, uint16_t _usBottom, uint16_t _usTop);

extern void DAC8501_SetDacData(uint8_t _ch, uint16_t _dac);
/*
*********************************************************************************************************
*	�� �� ��: DemoSpiDac
*	����˵��: DAC8501����
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void DemoSpiDac(void)
{
	uint8_t i=0;
	uint8_t ucKeyCode;	/* �������� */
	
	sfDispMenu();		/* ��ӡ������ʾ */
	
	bsp_StartAutoTimer(0, 200);	/* ����1��100ms���Զ���װ�Ķ�ʱ�� */
	
	
	/* ���ɷ������� */
	MakeSinTable(ch1buf, 100, 0, 65535);
	
	DAC8501_SetDacDataDMA(1, ch1buf, sizeof(ch1buf)/sizeof(uint16_t), 1000000);
	
	while(1)
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
				case KEY_DOWN_K1:			/* K1�����£�ͨ��1������� */
					/* ���ɷ������� */
					for(i =0; i< 50; i++)
					{
						ch1buf[i] = 0;
					}
					
					for(i =50; i< 100; i++)
					{
						ch1buf[i] = 65535;
					}

					/* �����ٶ�1MHz */
					DAC8501_SetDacDataDMA(1, ch1buf, sizeof(ch1buf)/sizeof(uint16_t), 1000000);
					break;

				case KEY_DOWN_K2:			/* K2�����£�ͨ��1������Ҳ� */
					/* �������Ҳ����� */	
					MakeSinTable(ch1buf, 100, 0, 65535);
					
					/* �����ٶ�1MHz */
					DAC8501_SetDacDataDMA(1, ch1buf, sizeof(ch1buf)/sizeof(uint16_t), 1000000);
					break;

				case KEY_DOWN_K3:			/* K3�����£�ͨ��1���ֱ�� */
					
					/* ���ɷ������� */
					for(i =0; i< 100; i++)
					{
						ch1buf[i] = 65535;
					}

					/* �����ٶ�1MHz */
					DAC8501_SetDacDataDMA(1, ch1buf, sizeof(ch1buf)/sizeof(uint16_t), 1000);
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
*	�� �� ��: MakeSinTable
*	����˵��: ����������Ҳ�����
*	��    ��: _pBuf : Ŀ�껺����
*			  _usSamples : ÿ�����ڵ������� ���������32��������ż����
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void MakeSinTable(uint16_t *_pBuf, uint16_t _usSamples, uint16_t _usBottom, uint16_t _usTop)
{
	uint16_t i;
	uint16_t mid;	/* ��ֵ */
	uint16_t att;	/* ���� */

	mid = (_usBottom + _usTop) / 2;		/* 0λ��ֵ */
	att = (_usTop - _usBottom) / 2;  	/* ���Ҳ����ȣ����ֵ����2 */

	for (i = 0; i < _usSamples; i++)
	{
		_pBuf[i] = mid + (int32_t)(att * sin((i * 2 * 3.1415926f) / _usSamples));
	}
}

/*
*********************************************************************************************************
*	�� �� ��: sfDispMenu
*	����˵��: ��ʾ������ʾ�˵�
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void sfDispMenu(void)
{
	printf("������ʾ:\r\n");
	printf("1. ����һ���Զ���װ�����ʱ����ÿ100ms��תһ��LED2\r\n");
	printf("2. K1�����£�ͨ��1�������\r\n");
	printf("3. K2�����£�ͨ��1������Ҳ�\r\n");	
    printf("4. K3�����£�ͨ��1���ֱ��\r\n");
}

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
