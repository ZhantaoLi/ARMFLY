/*
*********************************************************************************************************
*
*	模块名称 : 主程序模块
*	文件名称 : main.c
*	版    本 : V1.0
*	说    明 : RS485多机通讯，使用的USART3。
*              实验目的：
*                1. 学习RS485多机通讯的实现。
*              实验内容：
*                1. 由于通信距离较短，SP3485E芯片上缺省未贴的电阻不需要贴上，大家可以根据需要贴上电阻做测试。
*                2. 本例子支持多个485节点，不需要设置主设备和从设备，所有节点下载此程序即可。
*                3. 开发板的485-A端子连接到一起，485-B端子连接到一起，具体连接看工程Doc文件夹中的截图。
*              实验操作：
*                1. 按下开发板上的K1键点亮LED1，松开熄灭LED1，同时打印按键事件到串口1。485总线上的其它开
*                   发板做相同的动作。
*                2. 按下开发板上的K2键，启动50ms的自动重装定时器，每隔50ms翻转LED2，并向485总线上的其它开
*                   发板发送按键K2按下消息，从而也实现每隔50ms翻转LED2。
*                3. 按下开发板上的K3按键，停止K2按键启动的50ms自动重载定时器，485总线上的其它开发板做相同
*                   的动作。
*                4. 按下开发板上的摇杆（上下左右，OK共5种），会通过串口1打印摇杆事件。485总线上的其它开
*                   发板做相同的动作。
*              注意事项：
*                1. RS485 PHY的发送使能用的引脚PB11，测试前需要将板子J5处的跳线帽短接到PB11端。
*                2. 本实验推荐使用串口软件SecureCRT查看打印信息，波特率115200，数据位8，奇偶校验位无，停止位1。
*                3. 务必将编辑器的缩进参数和TAB设置为4来阅读本文件，要不代码显示不整齐。
*
*	修改记录 :
*		版本号   日期         作者        说明
*		V1.0    2018-12-12   Eric2013     1. CMSIS软包版本 V5.4.0
*                                         2. HAL库版本 V1.3.0
*
*	Copyright (C), 2018-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/	
#include "bsp.h"			/* 底层硬件驱动 */



/* 定义例程名和例程发布日期 */
#define EXAMPLE_NAME	"V7-RS485多机通信"
#define EXAMPLE_DATE	"2018-12-12"
#define DEMO_VER		"1.0"

static void PrintfLogo(void);
static void PrintfHelp(void);

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
	uint8_t ucKeyCode;		/* 按键代码 */
	uint8_t ucDataTravel;	/* 发送变量 */
	uint8_t ucDataRec;	    /* 接收变量 */
    

	bsp_Init();		/* 硬件初始化 */
	
	PrintfLogo();	/* 打印例程名称和版本等信息 */
	PrintfHelp();	/* 打印操作提示 */

	/* 进入主程序循环体 */
	while (1)
	{
		bsp_Idle();		/* 这个函数在bsp.c文件。用户可以修改这个函数实现CPU休眠和喂狗 */
		
		/* 获取其它开发板通过485总线发来的数据 */
		if(comGetChar(COM3, &ucDataRec))
		{
			switch (ucDataRec)
			{
				case KEY_DOWN_K1:			/* 获得K1键按下消息 */
					bsp_LedOn(1);
					printf("K1键按下, LED1点亮\r\n");
					break;

				case KEY_UP_K1:		  		/* 获得K1键释放消息 */
					bsp_LedOff(1);
					printf("K1键弹起, LED1熄灭\r\n");
					break;

				case KEY_DOWN_K2:			/* 获得K2键按下消息 */
					bsp_LedToggle(2);
					break;

				case JOY_DOWN_U:			/* 获得摇杆UP键按下 */
					printf("摇杆上键按下\r\n");
					break;

				case JOY_DOWN_D:			/* 获得摇杆DOWN键按下 */
					printf("摇杆下键按下\r\n");
					break;

				case JOY_DOWN_L:			/* 获得摇杆LEFT键按下 */
					printf("摇杆左键按下\r\n");
					break;

				case JOY_DOWN_R:			/* 获得摇杆RIGHT键按下 */
					printf("摇杆右键按下\r\n");
					break;

				case JOY_DOWN_OK:			/* 获得摇杆OK键按下 */
					printf("摇杆OK键按下\r\n");
					break;

				case JOY_UP_OK:				/* 获得摇杆OK键弹起 */
					printf("摇杆OK键弹起\r\n");
					break;

				default:
					/* 其它的键值不处理 */
					break;
			}
		}

		/* 判断定时器超时时间 */
		if (bsp_CheckTimer(0))	
		{
			/* 每隔50ms 进来一次 */  
			bsp_LedToggle(2);
			/* 向其它开发板发送按键K2按下的消息 */
			ucDataTravel = KEY_DOWN_K2;
			comSendChar(COM3, ucDataTravel);
		}

		/* 按键滤波和检测由后台systick中断服务程序实现，我们只需要调用bsp_GetKey读取键值即可。 */
		ucKeyCode = bsp_GetKey();	/* 读取键值, 无键按下时返回 KEY_NONE = 0 */
		if (ucKeyCode != KEY_NONE)
		{
			switch (ucKeyCode)
			{
				case KEY_DOWN_K1:			/* K1键按下 */
					ucDataTravel = KEY_DOWN_K1;
					comSendChar(COM3, ucDataTravel);
					bsp_LedOn(1);
					printf("K1键按下, LED1点亮\r\n");
					break;

				case KEY_UP_K1:				/* K1键弹起 */
					ucDataTravel = KEY_UP_K1;
					comSendChar(COM3, ucDataTravel);
					bsp_LedOff(1);
					printf("K1键弹起, LED1熄灭\r\n");
					break;

				case KEY_DOWN_K2:			/* K2键按下 */
					bsp_StartAutoTimer(0, 50);	/* 启动1个50ms的自动重装的定时器 */
					break;

				case KEY_DOWN_K3:			/* K3键按下 */
					bsp_StopTimer(0);       /* 停止自动重装的定时器 */
					break;

				case JOY_DOWN_U:			/* 摇杆UP键按下 */
					ucDataTravel = JOY_DOWN_U;
					comSendChar(COM3, ucDataTravel);
					printf("摇杆上键按下\r\n");
					break;

				case JOY_DOWN_D:			/* 摇杆DOWN键按下 */
					ucDataTravel = JOY_DOWN_D;
					comSendChar(COM3, ucDataTravel);
					printf("摇杆下键按下\r\n");
					break;

				case JOY_DOWN_L:			/* 摇杆LEFT键按下 */
					ucDataTravel = JOY_DOWN_L;
					comSendChar(COM3, ucDataTravel);
					printf("摇杆左键按下\r\n");
					break;

				case JOY_DOWN_R:			/* 摇杆RIGHT键按下 */
					ucDataTravel = JOY_DOWN_R;
					comSendChar(COM3, ucDataTravel);
					printf("摇杆右键按下\r\n");
					break;

				case JOY_DOWN_OK:			/* 摇杆OK键按下 */
					ucDataTravel = JOY_DOWN_OK;
					comSendChar(COM3, ucDataTravel);
					printf("摇杆OK键按下\r\n");
					break;

				case JOY_UP_OK:				/* 摇杆OK键弹起 */
					ucDataTravel = JOY_UP_OK;
					comSendChar(COM3, ucDataTravel);
					printf("摇杆OK键弹起\r\n");
					break;

				default:
					/* 其它的键值不处理 */
					break;
			}
		}
	}
}

/*
*********************************************************************************************************
*	函 数 名: PrintfHelp
*	功能说明: 打印操作提示
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void PrintfHelp(void)
{
	printf("操作提示, 485总线上的其它开发板做相同的动作:\r\n");
	printf("1. K1按键按下点亮LED1，松开熄灭LED1，同时串口打印消息\r\n");
	printf("2. K2按键按下启动50ms的自动重装定时器，每50ms翻转LED2\r\n");
	printf("3. K3按键按下，停止K2按键启动的50ms自动重载定时器\r\n");
	printf("4. 摇杆（上下左右，OK共5种）按下，会通过串口1打印摇杆事件\r\n");
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
