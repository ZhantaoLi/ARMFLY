/*
*********************************************************************************************************
*
*	ģ������ : ������ģ��
*	�ļ����� : main.c
*	��    �� : V1.0
*	˵    �� : LCD�ĺ���С�ֿ��ȫ�ֿ�����ʵ�顣
*              ʵ��Ŀ�ģ�
*                1. ѧϰLCD�ĺ���С�ֿ��ȫ�ֿ�����ʵ�顣
*              ʵ�����ݣ�
*                1. С�ֿ��ȫ�ֿ�ͨ����������ɣ�
*                   http://forum.armfly.com/forum.php?mod=viewthread&tid=202 �� 
*                2. LCD������չʾASCII�ַ���GB2312���뺺�֡�
*                3. ����1��200ms���Զ���װ��ʱ������LED2ÿ200ms��תһ�Ρ�
*              ʵ�������
*                1. ҡ���ϼ�������LCD���������ȡ�
*                2. ҡ���¼�������LCD���������ȡ�
*                3. ҡ���������ʾ��һҳ���֡�
*                4. ҡ���Ҽ�����ʾ��һҳ���֡�
*                5. ҡ��OK����������ҳ��
*              ע�����
*                1. ��ʵ���Ƽ�ʹ�ô������SecureCRT�鿴��ӡ��Ϣ��������115200������λ8����żУ��λ�ޣ�ֹͣλ1��
*                2. ��ؽ��༭��������������TAB����Ϊ4���Ķ����ļ���Ҫ��������ʾ�����롣
*
*	�޸ļ�¼ :
*		�汾��   ����         ����        ˵��
*		V1.0    2019-04-08   Eric2013     1. CMSIS����汾 V5.4.0
*                                         2. HAL��汾 V1.3.0
*                                         
*	Copyright (C), 2018-2030, ���������� www.armfly.com
*
*********************************************************************************************************
*/	
#include "bsp.h"			 /* �ײ�Ӳ������ */


/* ���������������̷������� */
#define EXAMPLE_NAME	"V7-LCD�ĺ���С�ֿ��ȫ�ֿ�����ʵ��"
#define EXAMPLE_DATE	"2019-04-08"
#define DEMO_VER		"1.0"

#define	DEMO_PAGE_COUNT	89	/* Demo������� */

static void PrintfHelp(void);
static void DispFirstPage(void);
static void DispAsciiDot(void);
static void DispHZK16(uint8_t _ucIndex);

/*
*********************************************************************************************************
*	�� �� ��: demo_tft_lcd
*	����˵��: lcd����
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void demo_tft_lcd(void)
{
	uint16_t ucBright;	   	/* ��������(0-255) */
	uint8_t ucKeyCode;		/* �������� */
	uint8_t ucStatus;		/* ������״̬�� */
	uint8_t fRefresh;		/* ˢ�������־,1��ʾ��Ҫˢ�� */
	

	PrintfHelp();	/* ��ӡ������ʾ */

	/* �ӳ�200ms�ٵ������⣬����˲����� */
	bsp_DelayMS(200); 
	
	DispFirstPage();	/* ��ʾ��1ҳ */
	
	/* ����������ʾ��Ϻ��ٴ򿪱��⣬����Ϊȱʡ���� */
	bsp_DelayMS(100); 
	ucBright = BRIGHT_DEFAULT;
	LCD_SetBackLight(ucBright);
	
	bsp_StartAutoTimer(0, 200); /* ����1��200ms���Զ���װ�Ķ�ʱ���������ʱ��0 */
	
	/* ����������ѭ���� */
	ucStatus = 0;
	fRefresh = 0;	
	while (1)
	{
		/* �ж������ʱ��0�Ƿ�ʱ */
		if(bsp_CheckTimer(0))
		{
			/* ÿ��200ms ����һ�� */  
			bsp_LedToggle(2);
		}
		
		if (fRefresh == 1)
		{
			fRefresh = 0;

			switch (ucStatus)
			{
				case 0:
					DispFirstPage();	/* ��ʾ��1ҳ */
					break;

				case 1:
					DispAsciiDot();		/* ��ʾASCII���� */
					break;

				default:
					/* ���뷶Χ ��1 - 87 */
					if (ucStatus <= 89)
					{
						DispHZK16(ucStatus);	/* ��ʾһ������94������ */
					}
					break;
			}
		}

		ucKeyCode = bsp_GetKey();	/* ��ȡ��ֵ, �޼�����ʱ���� KEY_NONE = 0 */
		if (ucKeyCode != KEY_NONE)
		{
			/* �м����� */
			switch (ucKeyCode)
			{
				case JOY_DOWN_L:		/* ҡ��LEFT������ */
					if (ucStatus > 0)
					{
						ucStatus--;
					}
					fRefresh = 1;		/* ����ˢ��LCD */
					break;

				case JOY_DOWN_R:		/* ҡ��RIGHT������ */
					if (ucStatus < DEMO_PAGE_COUNT - 1)
					{
						ucStatus++;
					}
					fRefresh = 1;		/* ����ˢ��LCD */
					break;

				case  JOY_DOWN_OK:		/* ҡ��OK�� */
					ucStatus = 0;		/* ������ҳ */					
					fRefresh = 1;		/* ����ˢ��LCD */
					break;

				case JOY_DOWN_U:		/* ҡ��UP������ */
					ucBright += BRIGHT_STEP;
					if (ucBright > BRIGHT_MAX)
					{
						ucBright = BRIGHT_MAX;
					}
					LCD_SetBackLight(ucBright);
					printf("��ǰ���������� : %d\r\n", ucBright);
					break;

				case JOY_DOWN_D:		/* ҡ��DOWN������ */
					if (ucBright < BRIGHT_STEP)
					{
						ucBright = 0;
					}
					else
					{
						ucBright -= BRIGHT_STEP;
					}
					LCD_SetBackLight(ucBright);
					printf("��ǰ���������� : %d\r\n", ucBright);
					break;

				default:
					break;
			}
		}
	}
}

/*
*********************************************************************************************************
*	�� �� ��: DispFirstPage
*	����˵��: ��ʾ������ʾ
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void DispFirstPage(void)
{
	FONT_T tFont;		/* ����һ������ṹ���������������������� */
	uint16_t y;			/* Y���� */
	uint16_t usLineCap;	/* �и� */
	uint8_t buf[50];

	LCD_ClrScr(CL_BLUE);  		/* ������������ɫ */
	
	
	/* ������������ */
	tFont.FontCode = FC_ST_16;		/* ����ѡ������16���󣬸�16x��15) */
	tFont.FrontColor = CL_WHITE;	/* ������ɫ����Ϊ��ɫ */
	tFont.BackColor = CL_MASK;	 	/* ���ֱ�����ɫ��͸�� */
	tFont.Space = 0;				/* �ַ�ˮƽ���, ��λ = ���� */

	y = 0;
	usLineCap = 18; /* �м�� */
	LCD_DispStr(5, y, "������STM32-V7������  www.armfly.com", &tFont);
	y += usLineCap;
	LCD_DispStr(5, y, "����С�ֿ��ȫ�ֿ����ʵ��", &tFont);
	
	y += 2 * usLineCap;
	LCD_DispStr(30, y, "������ʾ:", &tFont);
	
	y += usLineCap;
	LCD_DispStr(50, y, "ҡ���ϼ� = ���ӱ�������", &tFont);
	
	y += usLineCap;
	LCD_DispStr(50, y, "ҡ���¼� = ���ͱ�������", &tFont);
	
	y += usLineCap;
	LCD_DispStr(50, y, "ҡ����� = ��ǰ��ҳ", &tFont);
	
	y += usLineCap;
	LCD_DispStr(50, y, "ҡ���Ҽ� = ���ҳ", &tFont);
	
	y += usLineCap;
	LCD_DispStr(50, y, "ҡ��OK�� = ������ҳ", &tFont);

	y += 2 * usLineCap;	
	
	sprintf((char *)buf, "��ʾ���ֱ��� ��%dx%d", g_LcdWidth, g_LcdHeight);
	LCD_DispStr(5, y, (char *)buf, &tFont);
	
	y += usLineCap;
	LCD_DispStr(5, y, "ÿ�п�����ʾ25�����֣���50���ַ�", &tFont);
}

/*
*********************************************************************************************************
*	�� �� ��: DispAsciiDot
*	����˵��: ��ʾASCII����
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void DispAsciiDot(void)
{
	uint8_t i,k;
	FONT_T tFont;		/* ����һ������ṹ���������������������� */
	uint16_t x;			/* X���� */
	uint16_t y;			/* Y���� */	
	char buf[32 + 1];
	uint8_t ascii;

	LCD_ClrScr(CL_BLUE);  		/* ������������ɫ */
	
	/* ������������ */
	tFont.FontCode = FC_ST_16;		/* ����ѡ������16���󣬸�16x��15) */
	tFont.FrontColor = CL_WHITE;	/* ������ɫ����Ϊ��ɫ */
	tFont.BackColor = CL_MASK;	 	/* ���ֱ�����ɫ��͸�� */
	tFont.Space = 2;				/* �ַ�ˮƽ���, ��λ = ���� */

	LCD_DispStr(50, 0, "16����ASCII���ֿ⣬����1-127", &tFont);

	x = 50;
	y = 40;
	ascii = 0;
	for (i = 0; i < 4; i++)
	{
		for (k = 0; k < 32; k++)
		{
			buf[k] = ascii++;
		}
		buf[32] = 0;
		if (buf[0] == 0)
		{
			buf[0] = ' ';
		}
		LCD_DispStr(x, y, buf, &tFont);
		y += 20;
	}
}

/*
*********************************************************************************************************
*	�� �� ��: DispHZK16
*	����˵��: ��ʾ16��������
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void DispHZK16(uint8_t _ucIndex)
{
	uint8_t i,k;
	uint16_t x,y;
	char buf[50 + 1];
	uint8_t code1,code2;	/* �������� */
	uint8_t usLineCap = 18;
	FONT_T tFont;			/* ����һ������ṹ���������������������� */

	printf(" Display HZK Area Code = %d\r\n", _ucIndex - 1);

	if (_ucIndex == 2)
	{
		/* ��1���������Ժ���ʾλ�ò��䣬���Բ�������������˸ */
		LCD_ClrScr(CL_BLUE);  		/* ������������ɫ */
	}
	
	/* ������������ */
	tFont.FontCode = FC_ST_16;		/* ����ѡ������16���󣬸�16x��15) */
	tFont.FrontColor = CL_WHITE;	/* ������ɫ����Ϊ��ɫ */
	tFont.BackColor = CL_BLUE;	 	/* ���ֱ�����ɫ����ɫ */
	tFont.Space = 0;				/* �ַ�ˮƽ���, ��λ = ���� */

	y = 0;
	LCD_DispStr(20, y, "����GB2312 16�����ֿ�(����1-87��λ��1-94)", &tFont);

	code1 = _ucIndex - 1; /* �õ����� */
	code2 = 1;	/* λ���1��ʼ */

	y += usLineCap;
	sprintf((char *)buf, (char *)"��ǰ����: %2d, ��ҳλ��:1-94, ��10-15�����ַ�", code1);
	LCD_DispStr(20, y, buf, &tFont);
	y += (2 * usLineCap);

	/*
		�������λ = ���� + 0xA0
		�������λ = λ�� + 0xA0

		���뷶Χ ��1 - 87
		λ�뷶Χ : 1 - 94

		ÿ����ʾ20�����֣�һ������94�����֣���Ҫ5����ʾ,��5����ʾ14������
	*/
	
	x = 40;
	code1 += 0xA0;	/* ���㵽�����λ */
	code2 = 0xA1;	/* �����λ��ʼ */
	for (i = 0; i < 5; i++)
	{
		for (k = 0; k < 20; k++)
		{
			buf[2 * k] = code1;
			buf[2 * k + 1] = code2;
			code2++;
			if ((i == 4) && (k == 13))
			{
				k++;
				break;
			}
		}
		buf[2 * k] = 0;
		LCD_DispStr(x, y, buf, &tFont);
		y += usLineCap;
	}
}

/*
*********************************************************************************************************
*	�� �� ��: PrintfHelp
*	����˵��: ��ӡ������ʾ
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void PrintfHelp(void)
{
	printf("������ʾ:\r\n");
	printf("1. ҡ���ϼ�������LCD����������\r\n");
	printf("2. ҡ���¼�������LCD����������\r\n");
	printf("3. ҡ���������ʾ��һҳ����\r\n");
	printf("4. ҡ���Ҽ�����ʾ��һҳ����\r\n");
	printf("5. ҡ��OK����������ҳ\r\n");
}

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
