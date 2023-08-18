/*
*********************************************************************************************************
*
*	模块名称 : 主程序模块
*	文件名称 : main.c
*	版    本 : V1.0
*	说    明 : Matlab的WIFI通信实现，WIFI模块使用的ESP8266。
*              实验目的：
*                1. 学习Matlab的WIFI通信。
*              实验内容： 
*                1、K1键  : 列举AP，就是WIFI热点;
*                2、K2键  : 加入AP，就是加入WIFI热点;
*                3、K3键  : 9600波特率切换到115200,并设置为Station模式;
*                4、摇杆上键  : AT+CIFSR获取本地IP地址;
*                5、摇杆下键  : AT+CIPSTATUS获得IP连接状态;
*                6、摇杆左键  : AT+CIPSTART建立TCP服务器;
*                7、摇杆右键  : 进入Maltab数据通信状态;
*              操作步骤： 
*                1、务必看第2版DSP教程第10章。   
*              注意事项：
*                1. 本实验推荐使用串口软件SecureCRT查看打印信息，波特率115200，数据位8，奇偶校验位无，停止位1。
*                2. 务必将编辑器的缩进参数和TAB设置为4来阅读本文件，要不代码显示不整齐。
*
*	修改记录 :
*		版本号   日期         作者        说明
*		V1.0    2019-09-13   Eric2013     1. CMSIS软包版本 V5.6.0
*                                         2. HAL库版本 V1.3.0
*
*	Copyright (C), 2018-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/	
#include "bsp.h"			 /* 底层硬件驱动 */



/* 定义例程名和例程发布日期 */
#define EXAMPLE_NAME	"V7-Matlab的WIFI通信实现"
#define EXAMPLE_DATE	"2019-09-13"
#define DEMO_VER		"1.0"

/* 仅允许本文件内调用的函数声明 */
static void PrintfLogo(void);
static void PrintfHelp(void);
uint8_t g_TCPServerOk = 0;

uint8_t cmd_buf[2048];	
uint8_t tcpid;	
uint8_t cmd_len;

/*
*********************************************************************************************************
*	函 数 名: main
*	功能说明: c程序入口
*	形    参：无
*	返 回 值: 错误代码(无需处理)
*********************************************************************************************************
*/
int main(void)
{
	uint8_t ucKeyCode;		/* 按键代码 */
	uint8_t ucValue;
	uint8_t ret;
	uint8_t SyncData = 36;
	uint16_t SendDATA[5];


	bsp_Init();		/* 硬件初始化 */
	PrintfLogo();	/* 打印例程信息到串口1 */

	PrintfHelp();	/* 打印操作提示信息 */

	/* 模块上电 */
	printf("\r\n【1】正在给ESP8266模块上电...(波特率: 74880bsp)\r\n");
	ESP8266_PowerOn();

	printf("\r\n【2】上电完成。波特率: 115200bsp\r\n");
	
	/* 检测模块波特率是否为115200 */
	ESP8266_SendAT("AT");
	if (ESP8266_WaitResponse("OK", 50) == 1)
	{
		printf("\r\n【3】模块应答AT成功\r\n");
		bsp_DelayMS(1000);
	}
	else
	{
		printf("\r\n【3】模块无应答, 请按K3键修改模块的波特率为115200\r\n");
		bsp_DelayMS(1000);
	}
	
	g_TCPServerOk = 0;
	
	bsp_StartAutoTimer(0, 100);	/* 启动1个100ms的自动重装的定时器 */

	/* 进入主程序循环体 */
	while (1)
	{
		bsp_Idle();		/* 这个函数在bsp.c文件。用户可以修改这个函数实现CPU休眠和喂狗 */

		/* 判断定时器超时时间 */
		if (bsp_CheckTimer(0))	
		{
			/* 每隔100ms 进来一次 */  
			bsp_LedToggle(2);
		}
		
		/* 进入Matlab通信状态执行下面程序 */
		if (g_TCPServerOk == 1)
		{
			cmd_len = ESP8266_RxNew(cmd_buf, &tcpid);
			if(cmd_len >0)
			{
				printf("\r\n接收到数据长度 = %d\r\n远程ID =%d\r\n数据内容=%s\r\n", cmd_len, tcpid, cmd_buf);
				
				/* 检索matlab发送过来的同步帧字符$，对应的ASCII数值是36 */
				if(strchr((char *)cmd_buf, 36))
				{
					/* 回复同步帧$ */
					ESP8266_SendTcpUdp(tcpid, (uint8_t *)&SyncData, 1);
					bsp_DelayMS(10);
					SendDATA[0] = rand()%65536;
					SendDATA[1] = rand()%65536;
					SendDATA[2] = rand()%65536;
					SendDATA[3] = rand()%65536;
					SendDATA[4] = rand()%65536;
					
					/* 发送数据，10个字节 */
					ESP8266_SendTcpUdp(tcpid, (uint8_t *)SendDATA, 10);
					printf("找到了相应的字符串\r\n");
				}
				else
				{
					printf("没有找到了相应的字符串\r\n");
				}
			}
		}
		/* 未进入Matlab通信状态执行下面程序 */
		else
		{			
			/* 从WIFI收到的数据发送到串口1 */
			if (comGetChar(COM_ESP8266, &ucValue))
			{
				comSendChar(COM1, ucValue);
			}
			/* 将串口1的数据发送到MG323模块 */
			if (comGetChar(COM1, &ucValue))
			{
				comSendChar(COM_ESP8266, ucValue);
			}
		}

		ucKeyCode = bsp_GetKey();	/* 读取键值, 无键按下时返回 KEY_NONE = 0 */
		if (ucKeyCode != KEY_NONE)
		{
			switch (ucKeyCode)
			{
				case KEY_DOWN_K1:			/* K1键按下，列举当前的WIFI热点 */
					g_TCPServerOk = 0;
					ESP8266_SendAT("AT+CWLAP");	
					break;

				case KEY_DOWN_K2:			/* K2键按下, 加入某个WIFI 网络*/
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

				case KEY_DOWN_K3:			            /* K3键-9600波特率切换到115200 */
					g_TCPServerOk = 0;
					ESP8266_9600to115200();
					break;

				case JOY_DOWN_U:		              /* 摇杆上键， AT+CIFSR获取本地IP地址 */
					g_TCPServerOk = 0;
					ESP8266_SendAT("AT+CIFSR");
					break;

				case JOY_DOWN_D:		              /* 摇杆下键 AT+CIPSTATUS获得IP连接状态 */
					g_TCPServerOk = 0;
					ESP8266_SendAT("AT+CIPSTATUS");
					break;

				case JOY_DOWN_L:		              /* 摇杆左键按下，创建TCP服务器 */
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

				case JOY_DOWN_R:		            /* 摇杆右键按下，进入Maltab数据通信状态 */
					g_TCPServerOk = 1;
					printf("\r\n 进入Maltab数据通信状态 \r\n");
					break;

				case JOY_DOWN_OK:	               /* 摇杆OK键按下，创建WIFI热点 */
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
					/* 其他的键值不处理 */
					break;
			}
		}
	}
}

/*
*********************************************************************************************************
*	函 数 名: PrintfHelp
*	功能说明: 显示操作提示菜单
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void PrintfHelp(void)
{
	printf("操作提示:\r\n");
	printf(" ESP8266模块  STM32-V7开发板\r\n");
	printf("  UTXD   ---  PC7/USART6_RX     CN16插座\r\n");
	printf("  GND    ---  GND               CN6插座\r\n");
	printf("  CH_PD  ---  PI0(控制模块掉电）CN16插座\r\n");
	printf("  GPIO2\r\n");
	printf("  GPIO16\r\n");
	printf("  GPIO0\r\n");
	printf("  VCC    ---  3.3  (供电)       CN6插座\r\n");
	printf("  URXD   ---  PG14/USART6_TX    CN16插座\r\n");

	printf("\r\n");
	printf("【按键操作】\r\n");
	printf("  K1键  : 列举AP\r\n");
	printf("  K2键  : 加入AP\r\n");
	printf("  K3键  : 9600波特率切换到115200,并设置为Station模式\r\n");
	printf("  摇杆上键  : AT+CIFSR获取本地IP地址\r\n");
	printf("  摇杆下键  : AT+CIPSTATUS获得IP连接状态\r\n");
	printf("  摇杆左键  : AT+CIPSTART建立TCP服务器\r\n");
	printf("  摇杆右键  : 进入Maltab数据通信状态\r\n");
	printf("\r\n");

	printf("【SecureCRT串口设置】\r\n");
	printf("  波特率: 115200\r\n");
	printf("  会话选项 - 终端 - 仿真 - 模式，勾选新行模式(W)\r\n");
	printf("\r\n");
}

/*
*********************************************************************************************************
*	函 数 名: PrintfLogo
*	功能说明: 打印例程名称和例程发布日期, 接上串口线后，打开PC机的超级终端软件可以观察结果
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void PrintfLogo(void)
{
	printf("*************************************************************\n\r");
	
	/* 检测CPU ID */
	{
		uint32_t CPU_Sn0, CPU_Sn1, CPU_Sn2;
		
		CPU_Sn0 = *(__IO uint32_t*)(0x1FF1E800);
		CPU_Sn1 = *(__IO uint32_t*)(0x1FF1E800 + 4);
		CPU_Sn2 = *(__IO uint32_t*)(0x1FF1E800 + 8);

		printf("\r\nCPU : STM32H743XIH6, BGA240, 主频: %dMHz\r\n", SystemCoreClock / 1000000);
		printf("UID = %08X %08X %08X\n\r", CPU_Sn2, CPU_Sn1, CPU_Sn0);
	}

	printf("\n\r");
	printf("*************************************************************\n\r");
	printf("* 例程名称   : %s\r\n", EXAMPLE_NAME);	/* 打印例程名称 */
	printf("* 例程版本   : %s\r\n", DEMO_VER);		/* 打印例程版本 */
	printf("* 发布日期   : %s\r\n", EXAMPLE_DATE);	/* 打印例程日期 */

	/* 打印ST的HAL库版本 */
	printf("* HAL库版本  : V1.3.0 (STM32H7xx HAL Driver)\r\n");
	printf("* \r\n");	/* 打印一行空格 */
	printf("* QQ    : 1295744630 \r\n");
	printf("* 旺旺  : armfly\r\n");
	printf("* Email : armfly@qq.com \r\n");
	printf("* 微信公众号: armfly_com \r\n");
	printf("* 淘宝店: armfly.taobao.com\r\n");
	printf("* Copyright www.armfly.com 安富莱电子\r\n");
	printf("*************************************************************\n\r");
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
