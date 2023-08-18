/*
*********************************************************************************************************
*
*	ģ������ : ������ģ��
*	�ļ����� : main.c
*	��    �� : V1.0
*	˵    �� : Matlab��WIFIͨ��ʵ�֣�WIFIģ��ʹ�õ�ESP8266��
*              ʵ��Ŀ�ģ�
*                1. ѧϰMatlab��WIFIͨ�š�
*              ʵ�����ݣ� 
*                1��K1��  : �о�AP������WIFI�ȵ�;
*                2��K2��  : ����AP�����Ǽ���WIFI�ȵ�;
*                3��K3��  : 9600�������л���115200,������ΪStationģʽ;
*                4��ҡ���ϼ�  : AT+CIFSR��ȡ����IP��ַ;
*                5��ҡ���¼�  : AT+CIPSTATUS���IP����״̬;
*                6��ҡ�����  : AT+CIPSTART����TCP������;
*                7��ҡ���Ҽ�  : ����Maltab����ͨ��״̬;
*              �������裺 
*                1����ؿ���2��DSP�̵̳�10�¡�   
*              ע�����
*                1. ��ʵ���Ƽ�ʹ�ô������SecureCRT�鿴��ӡ��Ϣ��������115200������λ8����żУ��λ�ޣ�ֹͣλ1��
*                2. ��ؽ��༭��������������TAB����Ϊ4���Ķ����ļ���Ҫ��������ʾ�����롣
*
*	�޸ļ�¼ :
*		�汾��   ����         ����        ˵��
*		V1.0    2019-09-13   Eric2013     1. CMSIS����汾 V5.6.0
*                                         2. HAL��汾 V1.3.0
*
*	Copyright (C), 2018-2030, ���������� www.armfly.com
*
*********************************************************************************************************
*/	
#include "bsp.h"			 /* �ײ�Ӳ������ */



/* ���������������̷������� */
#define EXAMPLE_NAME	"V7-Matlab��WIFIͨ��ʵ��"
#define EXAMPLE_DATE	"2019-09-13"
#define DEMO_VER		"1.0"

/* �������ļ��ڵ��õĺ������� */
static void PrintfLogo(void);
static void PrintfHelp(void);
uint8_t g_TCPServerOk = 0;

uint8_t cmd_buf[2048];	
uint8_t tcpid;	
uint8_t cmd_len;

/*
*********************************************************************************************************
*	�� �� ��: main
*	����˵��: c�������
*	��    �Σ���
*	�� �� ֵ: �������(���账��)
*********************************************************************************************************
*/
int main(void)
{
	uint8_t ucKeyCode;		/* �������� */
	uint8_t ucValue;
	uint8_t ret;
	uint8_t SyncData = 36;
	uint16_t SendDATA[5];


	bsp_Init();		/* Ӳ����ʼ�� */
	PrintfLogo();	/* ��ӡ������Ϣ������1 */

	PrintfHelp();	/* ��ӡ������ʾ��Ϣ */

	/* ģ���ϵ� */
	printf("\r\n��1�����ڸ�ESP8266ģ���ϵ�...(������: 74880bsp)\r\n");
	ESP8266_PowerOn();

	printf("\r\n��2���ϵ���ɡ�������: 115200bsp\r\n");
	
	/* ���ģ�鲨�����Ƿ�Ϊ115200 */
	ESP8266_SendAT("AT");
	if (ESP8266_WaitResponse("OK", 50) == 1)
	{
		printf("\r\n��3��ģ��Ӧ��AT�ɹ�\r\n");
		bsp_DelayMS(1000);
	}
	else
	{
		printf("\r\n��3��ģ����Ӧ��, �밴K3���޸�ģ��Ĳ�����Ϊ115200\r\n");
		bsp_DelayMS(1000);
	}
	
	g_TCPServerOk = 0;
	
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
		
		/* ����Matlabͨ��״ִ̬��������� */
		if (g_TCPServerOk == 1)
		{
			cmd_len = ESP8266_RxNew(cmd_buf, &tcpid);
			if(cmd_len >0)
			{
				printf("\r\n���յ����ݳ��� = %d\r\nԶ��ID =%d\r\n��������=%s\r\n", cmd_len, tcpid, cmd_buf);
				
				/* ����matlab���͹�����ͬ��֡�ַ�$����Ӧ��ASCII��ֵ��36 */
				if(strchr((char *)cmd_buf, 36))
				{
					/* �ظ�ͬ��֡$ */
					ESP8266_SendTcpUdp(tcpid, (uint8_t *)&SyncData, 1);
					bsp_DelayMS(10);
					SendDATA[0] = rand()%65536;
					SendDATA[1] = rand()%65536;
					SendDATA[2] = rand()%65536;
					SendDATA[3] = rand()%65536;
					SendDATA[4] = rand()%65536;
					
					/* �������ݣ�10���ֽ� */
					ESP8266_SendTcpUdp(tcpid, (uint8_t *)SendDATA, 10);
					printf("�ҵ�����Ӧ���ַ���\r\n");
				}
				else
				{
					printf("û���ҵ�����Ӧ���ַ���\r\n");
				}
			}
		}
		/* δ����Matlabͨ��״ִ̬��������� */
		else
		{			
			/* ��WIFI�յ������ݷ��͵�����1 */
			if (comGetChar(COM_ESP8266, &ucValue))
			{
				comSendChar(COM1, ucValue);
			}
			/* ������1�����ݷ��͵�MG323ģ�� */
			if (comGetChar(COM1, &ucValue))
			{
				comSendChar(COM_ESP8266, ucValue);
			}
		}

		ucKeyCode = bsp_GetKey();	/* ��ȡ��ֵ, �޼�����ʱ���� KEY_NONE = 0 */
		if (ucKeyCode != KEY_NONE)
		{
			switch (ucKeyCode)
			{
				case KEY_DOWN_K1:			/* K1�����£��оٵ�ǰ��WIFI�ȵ� */
					g_TCPServerOk = 0;
					ESP8266_SendAT("AT+CWLAP");	
					break;

				case KEY_DOWN_K2:			/* K2������, ����ĳ��WIFI ����*/
					g_TCPServerOk = 0;
					//ESP8266_SendAT("AT+CWJAP=\"Netcore_7378CB\",\"512464265\"");
					ret = ESP8266_JoinAP("Netcore_7378CB", "512464265", 15000);
					if(ret == 1)
					{
					   printf("\r\nJoinAP Success\r\n");
					}
					else
					{
						printf("\r\nJoinAP fail\r\n");					
					}
					
					break;

				case KEY_DOWN_K3:			            /* K3��-9600�������л���115200 */
					g_TCPServerOk = 0;
					ESP8266_9600to115200();
					break;

				case JOY_DOWN_U:		              /* ҡ���ϼ��� AT+CIFSR��ȡ����IP��ַ */
					g_TCPServerOk = 0;
					ESP8266_SendAT("AT+CIFSR");
					break;

				case JOY_DOWN_D:		              /* ҡ���¼� AT+CIPSTATUS���IP����״̬ */
					g_TCPServerOk = 0;
					ESP8266_SendAT("AT+CIPSTATUS");
					break;

				case JOY_DOWN_L:		              /* ҡ��������£�����TCP������ */
					g_TCPServerOk = 0;
					ret = ESP8266_CreateTCPServer(1001);
					if(ret == 1)
					{
					   printf("\r\nCreateTCP Success\r\n");
					}
					else
					{
						printf("\r\nCreateTCP fail\r\n");					
					}
					break;

				case JOY_DOWN_R:		            /* ҡ���Ҽ����£�����Maltab����ͨ��״̬ */
					g_TCPServerOk = 1;
					printf("\r\n ����Maltab����ͨ��״̬ \r\n");
					break;

				case JOY_DOWN_OK:	               /* ҡ��OK�����£�����WIFI�ȵ� */
					g_TCPServerOk = 0;
				    #if 0
				     ESP8266_SendAT("AT+CIPSTART=\"TCP\",\"WWW.ARMFLY.COM\",80");
				    #endif
				
				    #if 0
				     {
					  char ip[20], mac[32];
					  ESP8266_GetLocalIP(ip, mac);
					  printf("ip=%s, mac=%s\r\n", ip, mac);			
					 }
				    #endif
				
				    #if 1
					 ESP8266_SetWiFiMode(3);
					 ESP8266_SendAT("AT+CWSAP=\"ESP8266\",\"1234567890\",1,3");
				    #endif
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
*	�� �� ��: PrintfHelp
*	����˵��: ��ʾ������ʾ�˵�
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void PrintfHelp(void)
{
	printf("������ʾ:\r\n");
	printf(" ESP8266ģ��  STM32-V7������\r\n");
	printf("  UTXD   ---  PC7/USART6_RX     CN16����\r\n");
	printf("  GND    ---  GND               CN6����\r\n");
	printf("  CH_PD  ---  PI0(����ģ����磩CN16����\r\n");
	printf("  GPIO2\r\n");
	printf("  GPIO16\r\n");
	printf("  GPIO0\r\n");
	printf("  VCC    ---  3.3  (����)       CN6����\r\n");
	printf("  URXD   ---  PG14/USART6_TX    CN16����\r\n");

	printf("\r\n");
	printf("������������\r\n");
	printf("  K1��  : �о�AP\r\n");
	printf("  K2��  : ����AP\r\n");
	printf("  K3��  : 9600�������л���115200,������ΪStationģʽ\r\n");
	printf("  ҡ���ϼ�  : AT+CIFSR��ȡ����IP��ַ\r\n");
	printf("  ҡ���¼�  : AT+CIPSTATUS���IP����״̬\r\n");
	printf("  ҡ�����  : AT+CIPSTART����TCP������\r\n");
	printf("  ҡ���Ҽ�  : ����Maltab����ͨ��״̬\r\n");
	printf("\r\n");

	printf("��SecureCRT�������á�\r\n");
	printf("  ������: 115200\r\n");
	printf("  �Ựѡ�� - �ն� - ���� - ģʽ����ѡ����ģʽ(W)\r\n");
	printf("\r\n");
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
		
		CPU_Sn0 = *(__IO uint32_t*)(0x1FF1E800);
		CPU_Sn1 = *(__IO uint32_t*)(0x1FF1E800 + 4);
		CPU_Sn2 = *(__IO uint32_t*)(0x1FF1E800 + 8);

		printf("\r\nCPU : STM32H743XIH6, BGA240, ��Ƶ: %dMHz\r\n", SystemCoreClock / 1000000);
		printf("UID = %08X %08X %08X\n\r", CPU_Sn2, CPU_Sn1, CPU_Sn0);
	}

	printf("\n\r");
	printf("*************************************************************\n\r");
	printf("* ��������   : %s\r\n", EXAMPLE_NAME);	/* ��ӡ�������� */
	printf("* ���̰汾   : %s\r\n", DEMO_VER);		/* ��ӡ���̰汾 */
	printf("* ��������   : %s\r\n", EXAMPLE_DATE);	/* ��ӡ�������� */

	/* ��ӡST��HAL��汾 */
	printf("* HAL��汾  : V1.3.0 (STM32H7xx HAL Driver)\r\n");
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
