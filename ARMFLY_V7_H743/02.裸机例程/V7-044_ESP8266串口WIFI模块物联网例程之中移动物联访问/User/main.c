/*
*********************************************************************************************************
*
*	ģ������ : ������ģ��
*	�ļ����� : main.c
*	��    �� : V1.0
*	˵    �� : ѧϰ����WIFIģ��ESP8266ʵ�����ƶ��������ݴ��䡣
*              ʵ��Ŀ�ģ�
*                1. ѧϰ����WIFIģ��ESP8266ʵ�����ƶ��������ݴ��䡣
*              ʵ�����ݣ� 
*                1��K1��  : �о�AP������WIFI�ȵ�;
*                2��K2��  : ����AP�����Ǽ���WIFI�ȵ�;
*                3��K3��  : 9600�������л���115200,������ΪStationģʽ;
*                4��ҡ���ϼ�  : AT+CIFSR��ȡ����IP��ַ;
*                5��ҡ���¼�  : AT+CIPSTATUS���IP����״̬;
*                6��ҡ�����  : �������ƶ�����api.heclouds.com���˿�80;
*                7��ҡ���Ҽ�  : ��ʱÿ5�뷢��һ������;
*                8��ҡ��OK��  : ֹͣ���ݷ���;
*              ע�����
*                1. ��ʵ���Ƽ�ʹ�ô������SecureCRT�鿴��ӡ��Ϣ��������115200������λ8����żУ��λ�ޣ�ֹͣλ1��
*                2. ��ؽ��༭��������������TAB����Ϊ4���Ķ����ļ���Ҫ��������ʾ�����롣
*
*	�޸ļ�¼ :
*		�汾��   ����         ����        ˵��
*		V1.0    2019-07-19   Eric2013     1. CMSIS����汾 V5.4.0
*                                         2. HAL��汾 V1.3.0
*
*	Copyright (C), 2018-2030, ���������� www.armfly.com
*
*********************************************************************************************************
*/	
#include "bsp.h"			 /* �ײ�Ӳ������ */



/* ���������������̷������� */
#define EXAMPLE_NAME	"V7-ESP8266����WIFIģ������������֮���ƶ���������"
#define EXAMPLE_DATE	"2019-07-19"
#define DEMO_VER		"1.0"

/* ���ƶ�������DEV_ID��API_KEY��Ҫ�û�ע���ſ��Եõ� */
#define SERVER_ADDR     "api.heclouds.com"          
#define DEV_ID          "3282647"                     
#define API_KEY         "6o5Z=7BgRbdRXjm4=kPq=NEb=m8=" 

/* �������ļ��ڵ��õĺ������� */
static void PrintfLogo(void);
static void PrintfHelp(void);
uint8_t g_TCPServerOk = 0;

/* ���� */
uint8_t cmd_buf[2000];
char send_buf[2000] = {0};
char text[1024] = {0};
char tmp[512] = {0};
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
		
		/* �ж϶�ʱ����ʱʱ�� */
		if (bsp_CheckTimer(1))	
		{
			 
			/*
			   ���ݸ�ʽ��
			   POST /devices/3282647/datapoints HTTP/1.1
			   api-key:6o5Z=7BgRbdRXjm4=kPq=NEb=m8=
			   Host:api.heclouds.com
			   Connection:keep-alive
			   Content-Length:112

			   {"datastreams":[{"id":"sys_time","datapoints":[{"value":123}]},{"id":"sys_time1","datapoints":[{"value":123}]}]}
			*/
			memset(text, '\0', sizeof(text));
			memset(tmp, '\0', sizeof(tmp));
			memset(send_buf, '\0', sizeof(send_buf));		

			/* ����JSON��ʽ���� */
			strcat(text,"{\"datastreams\":[");
			
			/* ����������sys_time */
			strcat(text,"{\"id\":\"sys_time\",");
			strcat(text,"\"datapoints\":[");
			strcat(text,"{");
			sprintf(tmp, "\"value\":%d", rand()%100);
			strcat(text,tmp);
			strcat(text,"}]},");
			
			/* ����������sys_time1 */
			strcat(text,"{\"id\":\"sys_time1\",");
			strcat(text,"\"datapoints\":[");
			strcat(text,"{");
			sprintf(tmp, "\"value\":%d", rand()%100);
			strcat(text,tmp);
			strcat(text,"}]}");
			
			strcat(text,"]}");
			
			/* ׼��HTTP��ͷ */
			send_buf[0] = 0;
			strcat(send_buf,"POST /devices/");
			strcat(send_buf,DEV_ID);
			strcat(send_buf,"/datapoints HTTP/1.1\r\n");   /*ע�����������\r\n */
			strcat(send_buf,"api-key:");
			strcat(send_buf,API_KEY);
			strcat(send_buf,"\r\n");
			strcat(send_buf,"Host:");
			strcat(send_buf,SERVER_ADDR);
			strcat(send_buf,"\r\n");
			sprintf(tmp,"Content-Length:%d\r\n\r\n", strlen(text)); /* ����JSON������ */
			strcat(send_buf,tmp);
			strcat(send_buf,text);

			ret = ESP8266_SendTcpUdp(0, (uint8_t *)&send_buf, strlen(send_buf));
			
			/* �ȴ�һ�᷵������ */
			ret = ESP8266_WaitResponse("+IPD", 200);
			
			/* û�з��أ�ֱ������ */
			if(ret == 0)
			{
				ESP8266_LinkTCPServer(0, "api.heclouds.com", 80);
			}	
		}
		
		/* ��WIFI�յ������ݷ��͵�����1 */
		if (comGetChar(COM_ESP8266, &ucValue))
		{
			comSendChar(COM1, ucValue);
			//continue;
		}
		/* ������1�����ݷ��͵�MG323ģ�� */
		if (comGetChar(COM1, &ucValue))
		{
			comSendChar(COM_ESP8266, ucValue);
			//continue;
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
					//ESP8266_SendAT("AT+CWJAP=\"Netcore_7378CB\",\"512464265\"");	/*  */
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

				case JOY_DOWN_L:		              /* ҡ��������£����ӷ�������IP��ַ192.168.1.2���˿ں�1001 */
					g_TCPServerOk = 1;
					ret = ESP8266_LinkTCPServer(0, "api.heclouds.com", 80);
					if(ret == 1)
					{
					   printf("\r\nLinkTCP Success\r\n");
					}
					else
					{
						printf("\r\nLinkTCP fail\r\n");					
					}
					break;

				case JOY_DOWN_R:		            /* ҡ���Ҽ����£���TCP�ͻ��˷������� */
					g_TCPServerOk = 1;
					bsp_StartAutoTimer(1, 5000);	/* ����1��100ms���Զ���װ�Ķ�ʱ�� */
					break;

				case JOY_DOWN_OK:	             /* ҡ��OK�����£�����WIFI�ȵ� */
					g_TCPServerOk = 0;
					bsp_StopTimer(1);
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
	printf("  ҡ�����  : �������ƶ�����api.heclouds.com���˿�80\r\n");
	printf("  ҡ���Ҽ�  : ��ʱÿ�뷢��һ������\r\n");
	printf("  ҡ��OK��  : ֹͣ���ݷ���\r\n");
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
