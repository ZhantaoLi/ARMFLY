/*
*********************************************************************************************************
*
*	ģ������ : DAC8563/DAC8562����
*	�ļ����� : demo_spi_dac8562.c
*	��    �� : V1.0
*	˵    �� : DAC8563���ԡ�
*
*	�޸ļ�¼ :
*		�汾��  ����        ����     ˵��
*		V1.0    2020-04-03 armfly  ��ʽ����
*
*	Copyright (C), 2020-2030, ���������� www.armfly.com
*
*********************************************************************************************************
*/
#include "demo_spi_dac8562.h"
#include "bsp.h"



uint16_t ch1buf[100]; /* ͨ��1����ͨ��A���� */
uint16_t ch2buf[100]; /* ͨ��2����ͨ��B���� */


/* �������ļ��ڵ��õĺ������� */
static void sfDispMenu(void);
static void MakeSinTable(uint16_t *_pBuf, uint16_t _usSamples, uint16_t _usBottom, uint16_t _usTop);

/*
*********************************************************************************************************
*	�� �� ��: DemoSpiDac
*	����˵��: DAC8562����
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
	for(i =0; i< 50; i++)
	{
		ch1buf[i] = 0;
	}
	
	for(i =50; i< 100; i++)
	{
		ch1buf[i] = 65535;
	}

	/* �������Ҳ����� */	
	MakeSinTable(ch2buf, 100, 0, 65535);
	
	/* 
	   �ϵ�Ĭ��˫ͨ�������
	    ��1��������
		  1 ��ʾͨ��1���
		  2 ��ʾͨ��2���
		  3 ��ʾͨ��1��2�����
          4 ��ʾͨ��1��2����������Ҹ���һ�����������Ч��ֹ�������ʱ�ָ�����ʹ���ģ��Ҳ��Ӱ�졣
	    ���һ��������
	      ��ʱ�������ٶ�1MHz������1����һ��24bit���ݵĴ��䡣
	      �Ƽ���Χ100Hz - 1MHz��
	*/
	DAC8562_SetDacDataDMA(4, ch1buf, ch2buf, sizeof(ch1buf)/sizeof(uint16_t), sizeof(ch2buf)/sizeof(uint16_t), 1000000);
	
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
				case KEY_DOWN_K1:			/* K1�����£�˫ͨ�������ͨ��1���������ͨ��2������Ҳ� */
					/* ���ɷ������� */
					for(i =0; i< 50; i++)
					{
						ch1buf[i] = 0;
					}
					
					for(i =50; i< 100; i++)
					{
						ch1buf[i] = 65535;
					}

					/* �������Ҳ����� */	
					MakeSinTable(ch2buf, 100, 0, 65535);
					
					/* �ϵ�Ĭ��˫ͨ������������ٶ�1MHz */
					DAC8562_SetDacDataDMA(3, ch1buf, ch2buf, sizeof(ch1buf)/sizeof(uint16_t), sizeof(ch2buf)/sizeof(uint16_t), 1000000);
					break;

				case KEY_DOWN_K2:			/* K2�����£�˫ͨ��������� */
					/* �������Ҳ����� */	
					MakeSinTable(ch2buf, 100, 0, 65535);
					
					/* �ϵ�Ĭ��˫ͨ������������ٶ�1MHz */
					DAC8562_SetDacDataDMA(3, ch2buf, ch2buf, sizeof(ch2buf)/sizeof(uint16_t), sizeof(ch2buf)/sizeof(uint16_t), 1000000);
					break;

				case KEY_DOWN_K3:			/* K3�����£�˫ͨ��������Ҳ� */
					/* ���ɷ������� */
					for(i =0; i< 50; i++)
					{
						ch1buf[i] = 0;
					}
					
					for(i =50; i< 100; i++)
					{
						ch1buf[i] = 65535;
					}
					
					/* �ϵ�Ĭ��˫ͨ������������ٶ�1MHz */
					DAC8562_SetDacDataDMA(3, ch1buf, ch1buf, sizeof(ch1buf)/sizeof(uint16_t), sizeof(ch1buf)/sizeof(uint16_t), 1000000);	
					break;
				
				case JOY_DOWN_U:			/* ҡ���ϼ����£�ͨ��1ֹͣ������ͨ��2ֹͣ��� */
					/* ͨ��1������� */
					for(i =0; i< 50; i++)
					{
						ch1buf[i] = 0;
					}
					
					for(i =50; i< 100; i++)
					{
						ch1buf[i] = 65535;
					}
					
					/* ��ͨ��1��� */
					DAC8562_SetDacDataDMA(1, ch1buf, ch2buf, sizeof(ch1buf)/sizeof(uint16_t), sizeof(ch2buf)/sizeof(uint16_t), 1000000);	
					break;
				
				case JOY_DOWN_D:			/* ҡ���¼����£�˫ͨ�����ֱ�� */
					/* ͨ��1�������10V */
					DAC8562_SetDacData(0, 0);	

					/* ͨ��2�����10V */
					DAC8562_SetDacData(1, 65535);		
					break;
				
				case JOY_DOWN_OK:			/* ҡ��OK�����£����³�ʼ��  */
					/* ��ʼ������DAC8562/8563 */
					//bsp_InitDAC8562();	
					DAC8562_WriteCmd((7 << 19) | (0 << 16) | (1 << 0));
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
	printf("2. K1�����£�˫ͨ�������ͨ��1���������ͨ��2������Ҳ�\r\n");
	printf("3. K2�����£�˫ͨ���������\r\n");	
    printf("4. K3�����£�˫ͨ��������Ҳ�\r\n");
	printf("5. ҡ���ϼ����£�ͨ��1ֹͣ������ͨ��2ֹͣ���\r\n");
	printf("6. ҡ���¼����£�˫ͨ�����ֱ��\r\n");
	printf("7. ҡ��OK�����£����³�ʼ�� \r\n");
}

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
