/*
*********************************************************************************************************
*
*	ģ������ : ����ͷ����
*	�ļ����� : camera_test.c
*	��    �� : V1.0
*	˵    �� : OV7670����ͷ���̣�ͼ��ֱ���640x480����
*              ʵ��Ŀ�ģ�
*                1. ѧϰOV7670����ͷ���̣�ͼ��ֱ���640x480����
*              ʵ�����ݣ�
*                1��OV7670����ͷ���640*480�ֱ��ʵ�RGB565��ɫ��ʽͼ��
*                2��Ĭ���ϵ������ͷ���Ǵ򿪵ġ�
*                3������ǵ��������败��У׼������ǵ�������Ҫ����У׼��
*              ʵ�������
*                1. ���������д�����ͷ�͹ر�����ͷ������ť;
*              ע�����
*                1. ��ʵ���Ƽ�ʹ�ô������SecureCRT�鿴��ӡ��Ϣ��������115200������λ8����żУ��λ�ޣ�ֹͣλ1��
*                2. ��ؽ��༭��������������TAB����Ϊ4���Ķ����ļ���Ҫ��������ʾ�����롣
*
*	�޸ļ�¼ :
*		�汾��   ����         ����        ˵��
*		V1.0    2018-12-12   Eric2013     1. CMSIS����汾 V5.4.0
*                                         2. HAL��汾 V1.3.0
*
*	Copyright (C), 2018-2030, ���������� www.armfly.com
*
*********************************************************************************************************
*/	
#include "bsp.h"
#include "camera_test.h"


/* �������ṹ */
typedef struct
{
	FONT_T FontBlack;	/* ��̬������ */
	FONT_T FontBlue;	/* �仯���������� ��ɫ */
	FONT_T FontRed;		/* �仯���������� ��ɫ */
	FONT_T FontBtn;		/* ��ť������ */
	FONT_T FontBox;		/* ������������ */

	GROUP_T Box1;

	LABEL_T Label1;	LABEL_T Label2;	/* ״̬ */
	LABEL_T Label3; LABEL_T Label4;	/* ״̬ */
	LABEL_T Label5; LABEL_T Label6;	/* ״̬ */

	BUTTON_T Btn1;		/* ������ͷ */
	BUTTON_T Btn2;		/* ��ͣ */

	BUTTON_T BtnRet;

}FormCAM_T;

/* ���屳��ɫ */
#define FORM_BACK_COLOR		CL_BTN_FACE

/* 4���������ʹ�С */
#define BOX1_X	5
#define BOX1_Y	5
#define BOX1_H	(g_LcdHeight - BOX1_Y - 10)
#define BOX1_W	(g_LcdWidth -  2 * BOX1_X)
#define BOX1_TEXT	"����ͷ0V7670���Գ���"

/* ���ذ�ť������(��Ļ���½�) */
#define BTN_RET_H	32
#define BTN_RET_W	60
#define	BTN_RET_X	((BOX1_X + BOX1_W) - BTN_RET_W - 4)
#define	BTN_RET_Y	((BOX1_Y  + BOX1_H) - BTN_RET_H - 4)
#define	BTN_RET_TEXT	"����"

#define BTN1_H	32
#define BTN1_W	100
#define	BTN1_X	(BOX1_X + 330 + 280)
#define	BTN1_Y	(BOX1_Y + 100)
#define	BTN1_TEXT	"������ͷ"

#define BTN2_H	32
#define BTN2_W	100
#define	BTN2_X	BTN1_X
#define	BTN2_Y	(BTN1_Y + BTN1_H + 10)
#define	BTN2_TEXT	"�ر�����ͷ"

/* ��ǩ */
#define LABEL1_X  	(BOX1_X + 330 + 280)
#define LABEL1_Y	(BOX1_Y + 20)
#define LABEL1_TEXT	"Chip ID : "

	#define LABEL2_X  	(LABEL1_X + 80 )
	#define LABEL2_Y	LABEL1_Y
	#define LABEL2_TEXT	"--"

#define LABEL3_X  	(LABEL1_X)
#define LABEL3_Y	(LABEL1_Y + 20)
#define LABEL3_TEXT	"״̬1   : "

	#define LABEL4_X  	(LABEL3_X + 80)
	#define LABEL4_Y	(LABEL3_Y)
	#define LABEL4_TEXT	"--"

#define LABEL5_X  	(LABEL1_X)
#define LABEL5_Y	(LABEL1_Y + 20 * 2)
#define LABEL5_TEXT	"״̬2   : "

	#define LABEL6_X  	(LABEL5_X + 80)
	#define LABEL6_Y	(LABEL5_Y)
	#define LABEL6_TEXT	"--"

/* ������ʾ����λ�úʹ�С */
#define PHOTO_X	10
#define PHOTO_Y	22
#define PHOTO_H	400
#define PHOTO_W	600

static void InitFormCam(void);
static void DispCamInitFace(void);

FormCAM_T *FormCam;

/*
*********************************************************************************************************
*	�� �� ��: TestCamera
*	����˵��: ��������ͷ
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void TestCamera(void)
{
	uint8_t ucTouch;		/* �����¼� */
	int16_t tpX, tpY;
	uint16_t usChipID;
	char buf[128];
	uint8_t fCamStop = 1;
	FormCAM_T form;
	
	
	FormCam = &form;

	/* ��ʼ������Ч�� */
	InitFormCam();

	DispCamInitFace();

	/* ��ȡ����ͷID */
	if (i2c_CheckDevice(OV7670_SLAVE_ADDRESS) == 0)
	{
		usChipID = OV_ReadID();
		sprintf(buf, "0x%04X", usChipID);

		FormCam->Label2.Font = &FormCam->FontBlue;
		FormCam->Label2.pCaption = buf;
	}
	else
	{
		sprintf(buf, "None  ");

		FormCam->Label2.Font = &FormCam->FontRed;
		FormCam->Label2.pCaption = buf;
	}
	
	/* ��ʼ����ͷ */
	bsp_InitCamera();
	
	LCD_DrawLabel(&FormCam->Label2);

	LCD_DrawRect(PHOTO_X - 1, PHOTO_Y - 1, PHOTO_H + 2, PHOTO_W + 2, CL_RED);

	/* �ϵ���Ȳɼ�һ֡ */
	fCamStop = 0;
	CAM_Start(SDRAM_APP_BUF);
	
	/* �ӳ�300ms�ٵ���LED������˲����� */
	bsp_DelayMS(300);
	
	LCD_SetBackLight(255);	 /* �򿪱��⣬����Ϊȱʡ���� */
	
	/* ����������ѭ���� */
	while (1)
	{
		
		bsp_Idle();
		
		/* �ɼ���ɺ�͸�����ʾ */
		if (g_tCam.CaptureOk == 1)
		{
			g_tCam.CaptureOk = 0;
			
			SCB_CleanInvalidateDCache();
			
			/* ��640*480�ֱ��ʲü���600*400������������м�ü��� */
			_LCD_DrawCamera16bpp(PHOTO_X, 
			                     PHOTO_Y,   
			                     (uint16_t *)(SDRAM_APP_BUF + 640*40*2 + 20*2), 
			                     600, 
			                     400, 40); 


			if (fCamStop == 0)
			{
				bsp_StartTimer(0, 1);	/* ������ʱ�� */
			}
		}

		/* �ɼ�ͼ�� */
		if (bsp_CheckTimer(0))
		{
			CAM_Start1(SDRAM_APP_BUF);
		}

		ucTouch = TOUCH_GetKey(&tpX, &tpY);	/* ��ȡ�����¼� */
		if (ucTouch != TOUCH_NONE)
		{
			switch (ucTouch)
			{
				case TOUCH_DOWN:		    /* ���ʰ����¼� */
					if (TOUCH_InRect(tpX, tpY, BTN1_X, BTN1_Y, BTN1_H, BTN1_W))
					{
						FormCam->Btn1.Focus = 1;
						LCD_DrawButton(&FormCam->Btn1);
					}
					else if (TOUCH_InRect(tpX, tpY, BTN2_X, BTN2_Y, BTN2_H, BTN2_W))
					{
						FormCam->Btn2.Focus = 1;
						LCD_DrawButton(&FormCam->Btn2);
					}
					break;

				case TOUCH_MOVE:		/* �����ƶ��¼� */
					break;

				case TOUCH_RELEASE:		/* �����ͷ��¼� */
					if (TOUCH_InRect(tpX, tpY, BTN1_X, BTN1_Y, BTN1_H, BTN1_W))	/* ������ͷ */
					{
						FormCam->Btn1.Focus = 0;
						LCD_DrawButton(&FormCam->Btn1);

						fCamStop = 0;
						CAM_Start1(SDRAM_APP_BUF);	/* ����ͷDMA��Ŀ���ַ����Ϊ�Դ� */			
					}
					else if (TOUCH_InRect(tpX, tpY, BTN2_X, BTN2_Y, BTN2_H, BTN2_W))
					{
						FormCam->Btn2.Focus = 0;
						LCD_DrawButton(&FormCam->Btn2);

						bsp_StopTimer(0);	/* ֹͣ�Զ���ʱ�� */
						fCamStop = 1;
					}
					else	/* ��ťʧȥ���� */
					{
						FormCam->BtnRet.Focus = 0;
						LCD_DrawButton(&FormCam->BtnRet);

						FormCam->Btn1.Focus = 0;
						LCD_DrawButton(&FormCam->Btn1);

						FormCam->Btn2.Focus = 0;
						LCD_DrawButton(&FormCam->Btn2);
					}
					break;
			}
		}
	}
}

/*
*********************************************************************************************************
*	�� �� ��: InitFormCam
*	����˵��: ��ʼ��GPS��ʼ����ؼ�
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void InitFormCam(void)
{
	/* ������������ */
	FormCam->FontBox.FontCode = FC_ST_16;
	FormCam->FontBox.BackColor = CL_BTN_FACE;	/* �ͱ���ɫ��ͬ */
	FormCam->FontBox.FrontColor = CL_BLACK;
	FormCam->FontBox.Space = 0;

	/* ����1 ���ھ�ֹ��ǩ */
	FormCam->FontBlack.FontCode = FC_ST_16;
	FormCam->FontBlack.BackColor = CL_MASK;		/* ͸��ɫ */
	FormCam->FontBlack.FrontColor = CL_BLACK;
	FormCam->FontBlack.Space = 0;

	/* ����2 ���ڱ仯������ */
	FormCam->FontBlue.FontCode = FC_ST_16;
	FormCam->FontBlue.BackColor = CL_BTN_FACE;
	FormCam->FontBlue.FrontColor = CL_BLUE;
	FormCam->FontBlue.Space = 0;

	/* ����3 ���ڱ仯������ */
	FormCam->FontRed.FontCode = FC_ST_16;
	FormCam->FontRed.BackColor = CL_BTN_FACE;
	FormCam->FontRed.FrontColor = CL_RED;
	FormCam->FontRed.Space = 0;

	/* ��ť���� */
	FormCam->FontBtn.FontCode = FC_ST_16;
	FormCam->FontBtn.BackColor = CL_MASK;		/* ͸������ */
	FormCam->FontBtn.FrontColor = CL_BLACK;
	FormCam->FontBtn.Space = 0;

	/* ����� */
	FormCam->Box1.Left = BOX1_X;
	FormCam->Box1.Top = BOX1_Y;
	FormCam->Box1.Height = BOX1_H;
	FormCam->Box1.Width = BOX1_W;
	FormCam->Box1.pCaption = BOX1_TEXT;
	FormCam->Box1.Font = &FormCam->FontBox;

	/* ��̬��ǩ */
	FormCam->Label1.Left = LABEL1_X;
	FormCam->Label1.Top = LABEL1_Y;
	FormCam->Label1.MaxLen = 0;
	FormCam->Label1.pCaption = LABEL1_TEXT;
	FormCam->Label1.Font = &FormCam->FontBlack;

	FormCam->Label3.Left = LABEL3_X;
	FormCam->Label3.Top = LABEL3_Y;
	FormCam->Label3.MaxLen = 0;
	FormCam->Label3.pCaption = LABEL3_TEXT;
	FormCam->Label3.Font = &FormCam->FontBlack;

	FormCam->Label5.Left = LABEL5_X;
	FormCam->Label5.Top = LABEL5_Y;
	FormCam->Label5.MaxLen = 0;
	FormCam->Label5.pCaption = LABEL5_TEXT;
	FormCam->Label5.Font = &FormCam->FontBlack;

	/* ��̬��ǩ */
	FormCam->Label2.Left = LABEL2_X;
	FormCam->Label2.Top = LABEL2_Y;
	FormCam->Label2.MaxLen = 0;
	FormCam->Label2.pCaption = LABEL2_TEXT;
	FormCam->Label2.Font = &FormCam->FontBlue;

	FormCam->Label4.Left = LABEL4_X;
	FormCam->Label4.Top = LABEL4_Y;
	FormCam->Label4.MaxLen = 0;
	FormCam->Label4.pCaption = LABEL4_TEXT;
	FormCam->Label4.Font = &FormCam->FontBlue;

	FormCam->Label6.Left = LABEL6_X;
	FormCam->Label6.Top = LABEL6_Y;
	FormCam->Label6.MaxLen = 0;
	FormCam->Label6.pCaption = LABEL6_TEXT;
	FormCam->Label6.Font = &FormCam->FontBlue;

	/* ��ť */
	FormCam->Btn1.Left = BTN1_X;
	FormCam->Btn1.Top = BTN1_Y;
	FormCam->Btn1.Height = BTN1_H;
	FormCam->Btn1.Width = BTN1_W;
	FormCam->Btn1.pCaption = BTN1_TEXT;
	FormCam->Btn1.Font = &FormCam->FontBtn;
	FormCam->Btn1.Focus = 0;

	FormCam->Btn2.Left = BTN2_X;
	FormCam->Btn2.Top = BTN2_Y;
	FormCam->Btn2.Height = BTN2_H;
	FormCam->Btn2.Width = BTN2_W;
	FormCam->Btn2.pCaption = BTN2_TEXT;
	FormCam->Btn2.Font = &FormCam->FontBtn;
	FormCam->Btn2.Focus = 0;
}

/*
*********************************************************************************************************
*	�� �� ��: DispUSBInitFace
*	����˵��: ��ʾ���еĿؼ�
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void DispCamInitFace(void)
{
	LCD_ClrScr(CL_BTN_FACE);

	/* ����� */
	LCD_DrawGroupBox(&FormCam->Box1);

	/* ��ǩ */
	LCD_DrawLabel(&FormCam->Label1);
	LCD_DrawLabel(&FormCam->Label2);
	LCD_DrawLabel(&FormCam->Label3);
	LCD_DrawLabel(&FormCam->Label4);
	LCD_DrawLabel(&FormCam->Label5);
	LCD_DrawLabel(&FormCam->Label6);

	/* ��ť */
	LCD_DrawButton(&FormCam->Btn1);
	LCD_DrawButton(&FormCam->Btn2);
}

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
