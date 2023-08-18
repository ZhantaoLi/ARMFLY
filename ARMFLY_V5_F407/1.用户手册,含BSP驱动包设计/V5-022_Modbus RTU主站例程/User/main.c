/*
*********************************************************************************************************
*
*	ģ������ : ������ģ��
*	�ļ����� : main.c
*	��    �� : V1.0
*	˵    �� : RS485 MODBUS��վ���̣�ʹ�õ��Ǵ���3����
*              ��������Ҫ����MODBUSЭ���վ���������,�����˳��õ����
*   ʵ�����ݣ�
*              1. �Ӻ�Ӳ��,(1)����1(��ӡʵ������)  (2)485�ӿ�(�շ�����)
*              2. ��������:
*								  ��	  �ӻ���ַ ������	�Ĵ����׵�ַ   �Ĵ�������	У����
*					KEY_DOWN_K1 : ���� 0x 	01 		 01		   01 01 		00 04 		6D F5
*					KEY_DOWN_K2	: ���� 0x   01       03        03 01        00 02       95 8F	
* 					JOY_DOWN_OK	: ���� 0x   01       02        02 01        00 03       68 73  				
*					JOY_UP_OK   : ���� 0x   01       04        04 01        00 01       61 3A
*								  д(1��) �ӻ���ַ ������    �Ĵ�����ַ	   д���ֵ		У����
*					JOY_DOWN_U	: ���� 0x   01       06        03 01        00 01       19 8E
*					JOY_DOWN_D	: ���� 0x   01       06        03 01        00 00       D8 4E
*					JOY_DOWN_L	: ���� 0x   01       05        01 01        00 01       5C 36
*					JOY_DOWN_R	: ���� 0x   01       05        01 01        00 00       9D F6
*								  д(���)�ӻ���ַ ������    �Ĵ�����ַ    �Ĵ�������  �ֽ���   д���ֵ1   д���ֵ2   У����
*					KEY_DOWN_K3 : ���� 0x   01       10        03 01        00 02        04      00 01       02 03      36 32
*
*   ע�����
*              1. ��ʵ���Ƽ�ʹ�ô������SecureCRT����H7-TOOL��λ������鿴��ӡ��Ϣ��������115200������λ8����żУ��λ�ޣ�ֹͣλ2��
*              2. ��ؽ��༭��������������TAB����Ϊ4���Ķ����ļ���Ҫ��������ʾ�����롣
*
*	�޸ļ�¼ :
*		�汾��   ����         ����        ˵��
*		V1.0    2022-10-02   Eric2013     1. CMSIS����汾 V5.4.0
*                                         2. HAL��汾 V1.7.6
*                                         
*	Copyright (C), 2020-2030, ���������� www.armfly.com
*
*********************************************************************************************************
*/	
#include "bsp.h"			 /* �ײ�Ӳ������ */
#include "modbus_host.h"


/* ���������������̷������� */
#define EXAMPLE_NAME	"V7-RS485 MODBUS RTU��վ����"
#define EXAMPLE_DATE	"2022-10-02"
#define DEMO_VER		"1.0"

static void PrintfLogo(void);


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
	uint8_t ucKeyCode;				/* �������� */
	MSG_T ucMsg;					/* ��Ϣ���� */

	bsp_Init();						/* Ӳ����ʼ�� */
	bsp_InitMsg();
	PrintfLogo();					/* ��ӡ������Ϣ������1 */
	
	bsp_StartAutoTimer(0, 100); 	/* ���� 1 �� 100ms ���Զ���װ�Ķ�ʱ�� */
	
	/* ����������ѭ���� */
	while (1)
	{
		bsp_Idle();
		
		if (bsp_CheckTimer(0)) /* �ж϶�ʱ����ʱʱ�� */
		{
			/* ÿ�� 100ms ����һ�� */
			bsp_LedToggle(2); /* ��ת LED ��״̬ */
		}
		
		if (bsp_GetMsg(&ucMsg))
		{
			switch (ucMsg.MsgCode)
			{
				case MSG_MODS_05H:		/* ��ӡ ���͵����� �� Ӧ�������  ˢ��LED״̬ */
					break;
				
				default:
					break;
			}
		}

		/* �����˲��ͼ���ɺ�̨systick�жϷ������ʵ�֣�����ֻ��Ҫ����bsp_GetKey��ȡ��ֵ���ɡ� */
		ucKeyCode = bsp_GetKey();	/* ��ȡ��ֵ, �޼�����ʱ���� KEY_NONE = 0 */
		if (ucKeyCode != KEY_NONE)
		{
			bsp_PutMsg(MSG_MODS, 0);
			
			switch (ucKeyCode)
			{			
				case KEY_DOWN_K1:				/* K1������ */
					if (MODH_ReadParam_01H(REG_D01, 4) == 1) ;else ;
					break;
				
				case KEY_DOWN_K2:				/* K2������ */
					if (MODH_ReadParam_03H(REG_P01, 2) == 1) ;else ;
					break;
				
				case KEY_DOWN_K3:				/* K3������ */
					{
						uint8_t buf[4];
						
						buf[0] = 0x01;
						buf[1] = 0x02;
						buf[2] = 0x03;
						buf[3] = 0x04;
						if (MODH_WriteParam_10H(REG_P01, 2, buf) == 1) ;else ;
					}
					break;
				
				case JOY_DOWN_U:				/* ҡ��UP������ */
					if (MODH_WriteParam_06H(REG_P01, 1) == 1) ;else ;
					break;
				
				case JOY_DOWN_D:				/* ҡ��DOWN������ */
					if (MODH_WriteParam_06H(REG_P01, 0) == 1) ;else ;
					break;
				
				case JOY_DOWN_L:				/* ҡ��LEFT������ */
					if (MODH_WriteParam_05H(REG_D01, 1) == 1) ;else ;
					break;
				
				case JOY_DOWN_R:				/* ҡ��RIGHT������ */
					if (MODH_WriteParam_05H(REG_D01, 0) == 1) ;else ;
					break;
				
				case JOY_DOWN_OK:				/* ҡ��OK������ */
					if (MODH_ReadParam_02H(REG_T01, 3) == 1) ;else ;
					break;

				case JOY_UP_OK:					/* ҡ��OK������ */
					if (MODH_ReadParam_04H(REG_A01, 1) == 1) ;else ;	
					break;
				
				default:						/* �����ļ�ֵ������ */
					break;
			}
		}		
	}
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
	printf("*************************************************************\r\n");
	
	/* ���CPU ID */
	{
		uint32_t CPU_Sn0, CPU_Sn1, CPU_Sn2;
		
		CPU_Sn0 = *(__IO uint32_t*)(0x1FFF7A10);
		CPU_Sn1 = *(__IO uint32_t*)(0x1FFF7A10 + 4);
		CPU_Sn2 = *(__IO uint32_t*)(0x1FFF7A10 + 8);

		printf("CPU : STM32F407IGT6, LQFP176, ��Ƶ: %dMHz\r\n", SystemCoreClock / 1000000);
		printf("UID = %08X %08X %08X\r\n", CPU_Sn2, CPU_Sn1, CPU_Sn0);
	}

	printf("*************************************************************\r\n");
	printf("* ��������   : %s\r\n", EXAMPLE_NAME);	/* ��ӡ�������� */
	printf("* ���̰汾   : %s\r\n", DEMO_VER);		/* ��ӡ���̰汾 */
	printf("* ��������   : %s\r\n", EXAMPLE_DATE);	/* ��ӡ�������� */

	/* ��ӡST��HAL��汾 */
	printf("* HAL��汾  : V1.7.6 (STM32F407 HAL Driver)\r\n");
	printf("* \r\n");	/* ��ӡһ�пո� */
	printf("* QQ    : 1295744630 \r\n");
	printf("* ����  : armfly\r\n");
	printf("* Email : armfly@qq.com \r\n");
	printf("* ΢�Ź��ں�: anfulai_com \r\n");
	printf("* �Ա���: anfulai.taobao.com\r\n");
	printf("* Copyright www.armfly.com ����������\r\n");
	printf("*************************************************************\r\n");
}

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
