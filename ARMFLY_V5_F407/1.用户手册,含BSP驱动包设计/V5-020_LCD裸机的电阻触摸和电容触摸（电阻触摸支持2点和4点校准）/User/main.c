/*
*********************************************************************************************************
*
*	ģ������ : ������ģ��
*	�ļ����� : main.c
*	��    �� : V1.0
*	˵    �� : LCD�ĵ��败���͵��ݴ��������败��֧��2���4��У׼����
*              ʵ��Ŀ�ģ�
*                1. ѧϰLCD�ĵ��败���͵��ݴ�����
*              ʵ�����ݣ�
*                1. ����������У׼����������ҪУ׼��
*                2. LCD����ʵ����һ���򵥵Ļ��幦�ܣ������ⴥ���Ƿ�׼ȷ��
*                3. ����1��200ms���Զ���װ��ʱ������LED2ÿ200ms��תһ�Ρ�
*              ʵ�������
*                1. ҡ���ϼ�������LCD���������ȡ�
*                2. ҡ���¼�������LCD���������ȡ�
*                3. ҡ����������败����У׼��
*                4. ҡ��OK����������
*              ע�����
*                1. ��ʵ���Ƽ�ʹ�ô������SecureCRT�鿴��ӡ��Ϣ��������115200������λ8����żУ��λ�ޣ�ֹͣλ1��
*                2. ��ؽ��༭��������������TAB����Ϊ4���Ķ����ļ���Ҫ��������ʾ�����롣
*
*	�޸ļ�¼ :
*		�汾��   ����         ����        ˵��
*		V1.0    2020-11-15   Eric2013     1. CMSIS����汾 V5.4.0
*                                         2. HAL��汾 V1.7.6
*                                         
*	Copyright (C), 2020-2030, ���������� www.armfly.com
*
*********************************************************************************************************
*/	
#include "bsp.h"			 /* �ײ�Ӳ������ */



/* ���������������̷������� */
#define EXAMPLE_NAME	"V5-LCD�ĵ��败���͵��ݴ��������败��֧��2���4��У׼��"
#define EXAMPLE_DATE	"2020-11-15"
#define DEMO_VER		"1.0"


static void PrintfHelp(void);
static void PrintfLogo(void);
static void DispFirstPage(void);


/*
*********************************************************************************************************
*	�� �� ��: main
*	����˵��: c�������
*	��    ��: ��
*	�� �� ֵ: �������(���账��)
*********************************************************************************************************
*/
int main(void)
{
	uint16_t ucBright;	   	/* ��������(0-255) */
	uint8_t ucKeyCode;		/* �������� */
	uint8_t fRefresh;		/* ˢ�������־,1��ʾ��Ҫˢ�� */
    FONT_T tFont;		    /* ����һ������ṹ���������������������� */
	char buf[64];
    uint16_t usAdcX, usAdcY;
	int16_t tpX, tpY;
    uint8_t ucTouch;		/* �����¼� */


	/* ����������� */
	{
		tFont.FontCode = FC_ST_16;		/* ������� 16���� */
		tFont.FrontColor = CL_WHITE;	/* ������ɫ */
		tFont.BackColor = CL_BLUE;		/* ���ֱ�����ɫ */
		tFont.Space = 0;				/* ���ּ�࣬��λ = ���� */
	}	
	
	bsp_Init();		/* Ӳ����ʼ�� */
	PrintfLogo();	/* ��ӡ�������ƺͰ汾����Ϣ */
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
	fRefresh = 1;	
	while (1)
	{
       bsp_Idle();
         
		/* �ж������ʱ��0�Ƿ�ʱ */
		if(bsp_CheckTimer(0))
		{
			/* ÿ��200ms ����һ�� */  
			bsp_LedToggle(2);
		}
		
        if (fRefresh)
		{
			fRefresh = 0;

			/* ʵʱˢ�´���ADC����ֵ��ת��������� */
			{
				
				/* ��ȡ����ʾ��ǰX���Y���ADC����ֵ */
				usAdcX = TOUCH_ReadAdcX();
				usAdcY = TOUCH_ReadAdcY();
				sprintf(buf, "����ADCֵ X = %4d, Y = %4d   ", usAdcX, usAdcY);
				LCD_DispStr(5, 40, buf, &tFont);

				/* ��ȡ����ʾ��ǰ�������� */
				tpX = TOUCH_GetX();
				tpY = TOUCH_GetY();
				sprintf(buf, "��������  X = %4d, Y = %4d   ", tpX, tpY);
				LCD_DispStr(5, 60, buf, &tFont);

				/* �ڴ�������λ����ʾһ��СȦ */
				if ((tpX > 0) && (tpY > 0))
				{
					LCD_DrawCircle(tpX, tpY, 2, CL_YELLOW);
				}
			}

			/* ����Ļ���ػ���2�����ο�(���ڼ������Ե�����Ƿ�����) */
			LCD_DrawRect(0, 0, LCD_GetHeight(), LCD_GetWidth(), CL_WHITE);
			LCD_DrawRect(2, 2, LCD_GetHeight() - 4, LCD_GetWidth() - 4, CL_YELLOW);
            
            /* ��ʾ����ֵ */
			sprintf(buf, "��ǰ��������: %3d", ucBright);
			LCD_DispStr(5, 80, buf, &tFont);
		}
        
        ucTouch = TOUCH_GetKey(&tpX, &tpY);	/* ��ȡ�����¼� */
		if (ucTouch != TOUCH_NONE)
		{
			switch (ucTouch)
			{
				case TOUCH_DOWN:		/* ���ʰ����¼� */
                  
					/* �ڴ�������λ����ʾһ��СȦ */
					if ((tpX > 0) && (tpY > 0))
					{
						LCD_DrawCircle(tpX, tpY, 3, CL_RED);
					}
					break;

				case TOUCH_MOVE:		/* �����ƶ��¼� */
					/* ʵʱˢ�´���ADC����ֵ��ת��������� */
					{
						/* ��ȡ����ʾ��ǰX���Y���ADC����ֵ */
						usAdcX = TOUCH_ReadAdcX();
						usAdcY = TOUCH_ReadAdcY();
						sprintf(buf, "����ADCֵ X = %4d, Y = %4d   ", usAdcX, usAdcY);
						LCD_DispStr(5, 40, buf, &tFont);

						/* ��ȡ����ʾ��ǰ�������� */
						//tpX = TOUCH_GetX();
						//tpY = TOUCH_GetY();
						sprintf(buf, "��������  X = %4d, Y = %4d   ", tpX, tpY);
						LCD_DispStr(5, 60, buf, &tFont);

						/* �ڴ�������λ����ʾһ��СȦ */
						if ((tpX > 0) && (tpY > 0))
						{
							LCD_DrawCircle(tpX, tpY, 2, CL_YELLOW);
						}
					}
					break;

				case TOUCH_RELEASE:		/* �����ͷ��¼� */
					/* �ڴ�������λ����ʾһ��СȦ */
					if ((tpX > 0) && (tpY > 0))
					{
						LCD_DrawCircle(tpX, tpY, 4, CL_WHITE);
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
				case JOY_DOWN_L:		/* ҡ��LEFT������, 2�㴥��У׼ */
					TOUCH_Calibration(2);
					DispFirstPage();				
					fRefresh = 1;		/* ����ˢ��LCD */
					break;

				case  JOY_DOWN_OK:		/* ҡ��OK�������� */
					DispFirstPage();				
					fRefresh = 1;		/* ����ˢ��LCD */
					break;

				case JOY_DOWN_U:		/* ҡ��UP������ */
					ucBright += BRIGHT_STEP;
					if (ucBright > BRIGHT_MAX)
					{
						ucBright = BRIGHT_MAX;
					}
					LCD_SetBackLight(ucBright);
                    fRefresh = 1;		/* ����ˢ��LCD */
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
                    fRefresh = 1;		/* ����ˢ��LCD */
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
	char buf[50];

	LCD_ClrScr(CL_BLUE);  		/* ������������ɫ */
	
	
	/* ������������ */
	tFont.FontCode = FC_ST_16;		/* ����ѡ������16���󣬸�16x��15) */
	tFont.FrontColor = CL_WHITE;	/* ������ɫ����Ϊ��ɫ */
	tFont.BackColor = CL_MASK;	 	/* ���ֱ�����ɫ��͸�� */
	tFont.Space = 0;				/* �ַ�ˮƽ���, ��λ = ���� */

	y = 4;
	usLineCap = 18; /* �м�� */
	LCD_DispStr(5, y, "������STM32-V5������  www.armfly.com", &tFont);
	y += usLineCap;
	LCD_DispStr(5, y, "�������͵�����ʵ�飬����������У׼��������ҪУ׼", &tFont);
	
	y += 5 * usLineCap;
	LCD_DispStr(30, y, "������ʾ:", &tFont);
	
	y += usLineCap;
	LCD_DispStr(50, y, "ҡ���ϼ� = ���ӱ�������", &tFont);
	
	y += usLineCap;
	LCD_DispStr(50, y, "ҡ���¼� = ���ͱ�������", &tFont);
	
	y += usLineCap;
	LCD_DispStr(50, y, "ҡ����� = ����������У׼", &tFont);
	
	y += usLineCap;
	LCD_DispStr(50, y, "ҡ��OK�� = ����", &tFont);

	y += 2 * usLineCap;	
	
    
    /* ��ʾTFT�������ͺź���Ļ�ֱ��� */
    //LCD_GetChipDescribe(buf);	/* ��ȡTFT����оƬ�ͺ� */
    if (g_TouchType == CT_GT811)
    {	
        strcpy(buf, "STM32F407 + GT811");
    }
    else if (g_TouchType == CT_GT911)
    {	
        strcpy(buf, "STM32F407 + GT911");
    }
    else if (g_TouchType == CT_FT5X06)
    {
        strcpy(buf, "STM32F407 + FT5X06");
    }
    else if (g_TouchType == CT_STMPE811)
    {
        strcpy(buf, "STM32F407 + STMPE811");
    }
    else
    {					
        strcpy(buf, "STM32F407 + RA8875");				
    }	
    sprintf(&buf[strlen(buf)], "   %d x %d", LCD_GetWidth(), LCD_GetHeight());
    LCD_DispStr(5, y, (char *)buf, &tFont);
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
	printf("3. ҡ����������败����У׼\r\n");
	printf("4. ҡ��OK��������\r\n");
}

/*
*********************************************************************************************************
*	�� �� ��: PrintfLogo
*	����˵��: ��ӡ�������ƺ����̷�������, ���ϴ����ߺ󣬴�PC���ĳ����ն�������Թ۲���
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void PrintfLogo(void)
{
	printf("*************************************************************\n\r");
	
	/* ���CPU ID */
	{
		uint32_t CPU_Sn0, CPU_Sn1, CPU_Sn2;
		
		CPU_Sn0 = *(__IO uint32_t*)(0x1FFF7A10);
		CPU_Sn1 = *(__IO uint32_t*)(0x1FFF7A10 + 4);
		CPU_Sn2 = *(__IO uint32_t*)(0x1FFF7A10 + 8);

		printf("\r\nCPU : STM32F407IGT6, LQFP176, ��Ƶ: %dMHz\r\n", SystemCoreClock / 1000000);
		printf("UID = %08X %08X %08X\n\r", CPU_Sn2, CPU_Sn1, CPU_Sn0);
	}

	printf("\n\r");
	printf("*************************************************************\n\r");
	printf("* ��������   : %s\r\n", EXAMPLE_NAME);	/* ��ӡ�������� */
	printf("* ���̰汾   : %s\r\n", DEMO_VER);		/* ��ӡ���̰汾 */
	printf("* ��������   : %s\r\n", EXAMPLE_DATE);	/* ��ӡ�������� */

	/* ��ӡST��HAL��汾 */
	printf("* HAL��汾  : V1.7.6 (STM32F407 HAL Driver)\r\n");
	printf("* \r\n");	/* ��ӡһ�пո� */
	printf("* QQ    : 1295744630 \r\n");
	printf("* ����  : armfly\r\n");
	printf("* Email : armfly@qq.com \r\n");
	printf("* ΢�Ź��ں�: armfly_com \r\n");
	printf("* �Ա���: armfly.taobao.com\r\n");
	printf("* Copyright www.armfly.com ����������\r\n");
	printf("*************************************************************\n\r");
}

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
