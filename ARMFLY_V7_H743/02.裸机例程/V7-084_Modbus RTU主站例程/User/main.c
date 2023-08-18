/*
*********************************************************************************************************
*
*	模块名称 : 主程序模块
*	文件名称 : main.c
*	版    本 : V1.0
*	说    明 : RS485 MODBUS从站例程（使用的是串口3）。
*              本例程主要讲解MODBUS协议从站的命令处理方法,包含了常用的命令。
*   实验内容：
*              1. 接好硬件,(1)串口1(打印实验数据)  (2)485接口(收发命令)
*              2. 按键定义:
*								  读	  从机地址 功能码	寄存器首地址   寄存器数量	校验码
*					KEY_DOWN_K1 : 发送 0x 	01 		 01		   01 01 		00 04 		6D F5
*					KEY_DOWN_K2	: 发送 0x   01       03        03 01        00 02       95 8F	
* 					JOY_DOWN_OK	: 发送 0x   01       02        02 01        00 03       68 73  				
*					JOY_UP_OK   : 发送 0x   01       04        04 01        00 01       61 3A
*								  写(1个) 从机地址 功能码    寄存器地址	   写入的值		校验码
*					JOY_DOWN_U	: 发送 0x   01       06        03 01        00 01       19 8E
*					JOY_DOWN_D	: 发送 0x   01       06        03 01        00 00       D8 4E
*					JOY_DOWN_L	: 发送 0x   01       05        01 01        00 01       5C 36
*					JOY_DOWN_R	: 发送 0x   01       05        01 01        00 00       9D F6
*								  写(多个)从机地址 功能码    寄存器地址    寄存器数量  字节数   写入的值1   写入的值2   校验码
*					KEY_DOWN_K3 : 发送 0x   01       10        03 01        00 02        04      00 01       02 03      36 32
*
*   注意事项：
*              1. 本实验推荐使用串口软件SecureCRT或者H7-TOOL上位机软件查看打印信息，波特率115200，数据位8，奇偶校验位无，停止位2。
*              2. 务必将编辑器的缩进参数和TAB设置为4来阅读本文件，要不代码显示不整齐。
*
*	修改记录 :
*		版本号   日期         作者        说明
*		V1.0    2022-10-02   Eric2013     1. CMSIS软包版本 V5.7.0
*                                         2. HAL库版本 V1.9.0
*                                         
*	Copyright (C), 2020-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/	
#include "bsp.h"			 /* 底层硬件驱动 */
#include "modbus_host.h"


/* 定义例程名和例程发布日期 */
#define EXAMPLE_NAME	"V7-RS485 MODBUS RTU主站例程"
#define EXAMPLE_DATE	"2022-10-02"
#define DEMO_VER		"1.0"

static void PrintfLogo(void);


/*
*********************************************************************************************************
*	函 数 名: main
*	功能说明: c程序入口
*	形    参: 无
*	返 回 值: 错误代码(无需处理)
*********************************************************************************************************
*/
int main(void)
{
	uint8_t ucKeyCode;				/* 按键代码 */
	MSG_T ucMsg;					/* 消息代码 */

	bsp_Init();						/* 硬件初始化 */
	bsp_InitMsg();
	PrintfLogo();					/* 打印例程信息到串口1 */
	
	bsp_StartAutoTimer(0, 100); 	/* 启动 1 个 100ms 的自动重装的定时器 */
	
	/* 进入主程序循环体 */
	while (1)
	{
		bsp_Idle();
		
		if (bsp_CheckTimer(0)) /* 判断定时器超时时间 */
		{
			/* 每隔 100ms 进来一次 */
			bsp_LedToggle(2); /* 翻转 LED 的状态 */
		}
		
		if (bsp_GetMsg(&ucMsg))
		{
			switch (ucMsg.MsgCode)
			{
				case MSG_MODS_05H:		/* 打印 发送的命令 和 应答的命令  刷新LED状态 */
					break;
				
				default:
					break;
			}
		}

		/* 按键滤波和检测由后台systick中断服务程序实现，我们只需要调用bsp_GetKey读取键值即可。 */
		ucKeyCode = bsp_GetKey();	/* 读取键值, 无键按下时返回 KEY_NONE = 0 */
		if (ucKeyCode != KEY_NONE)
		{
			bsp_PutMsg(MSG_MODS, 0);
			
			switch (ucKeyCode)
			{			
				case KEY_DOWN_K1:				/* K1键按下 */
					if (MODH_ReadParam_01H(REG_D01, 4) == 1) ;else ;
					break;
				
				case KEY_DOWN_K2:				/* K2键按下 */
					if (MODH_ReadParam_03H(REG_P01, 2) == 1) ;else ;
					break;
				
				case KEY_DOWN_K3:				/* K3键按下 */
					{
						uint8_t buf[4];
						
						buf[0] = 0x01;
						buf[1] = 0x02;
						buf[2] = 0x03;
						buf[3] = 0x04;
						if (MODH_WriteParam_10H(REG_P01, 2, buf) == 1) ;else ;
					}
					break;
				
				case JOY_DOWN_U:				/* 摇杆UP键弹起 */
					if (MODH_WriteParam_06H(REG_P01, 1) == 1) ;else ;
					break;
				
				case JOY_DOWN_D:				/* 摇杆DOWN键按下 */
					if (MODH_WriteParam_06H(REG_P01, 0) == 1) ;else ;
					break;
				
				case JOY_DOWN_L:				/* 摇杆LEFT键弹起 */
					if (MODH_WriteParam_05H(REG_D01, 1) == 1) ;else ;
					break;
				
				case JOY_DOWN_R:				/* 摇杆RIGHT键弹起 */
					if (MODH_WriteParam_05H(REG_D01, 0) == 1) ;else ;
					break;
				
				case JOY_DOWN_OK:				/* 摇杆OK键按下 */
					if (MODH_ReadParam_02H(REG_T01, 3) == 1) ;else ;
					break;

				case JOY_UP_OK:					/* 摇杆OK键弹起 */
					if (MODH_ReadParam_04H(REG_A01, 1) == 1) ;else ;	
					break;
				
				default:						/* 其它的键值不处理 */
					break;
			}
		}
		
	}
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
	printf("*************************************************************\r\n");
	
	/* 检测CPU ID */
	{
		uint32_t CPU_Sn0, CPU_Sn1, CPU_Sn2;
		
		CPU_Sn0 = *(__IO uint32_t*)(0x1FF1E800);
		CPU_Sn1 = *(__IO uint32_t*)(0x1FF1E800 + 4);
		CPU_Sn2 = *(__IO uint32_t*)(0x1FF1E800 + 8);

		printf("CPU : STM32H743XIH6, BGA240, 主频: %dMHz\r\n", SystemCoreClock / 1000000);
		printf("UID = %08X %08X %08X\r\n", CPU_Sn2, CPU_Sn1, CPU_Sn0);
	}

	printf("*************************************************************\r\n");
	printf("* 例程名称   : %s\r\n", EXAMPLE_NAME);	/* 打印例程名称 */
	printf("* 例程版本   : %s\r\n", DEMO_VER);		/* 打印例程版本 */
	printf("* 发布日期   : %s\r\n", EXAMPLE_DATE);	/* 打印例程日期 */

	/* 打印ST的HAL库版本 */
	printf("* HAL库版本  : V1.9.0 (STM32H7xx HAL Driver)\r\n");
	printf("* \r\n");	/* 打印一行空格 */
	printf("* QQ    : 1295744630 \r\n");
	printf("* 旺旺  : armfly\r\n");
	printf("* Email : armfly@qq.com \r\n");
	printf("* 微信公众号: anfulai_com \r\n");
	printf("* 淘宝店: anfulai.taobao.com\r\n");
	printf("* Copyright www.armfly.com 安富莱电子\r\n");
	printf("*************************************************************\r\n");
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
