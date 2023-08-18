/*
*********************************************************************************************************
*
*	模块名称 : AD7606测试
*	文件名称 : demo_fmc_ad7606.c
*	版    本 : V1.0
*	说    明 : AD7606测试。
*              数据展示方式：
*              （1）通过串口打印输出。
*              （2）通过J-Scope实时输出。
*
*	修改记录 :
*		版本号  日期        作者     说明
*		V1.0    2020-05-24 armfly  正式发布
*
*	Copyright (C), 2020-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/
#include "bsp.h"
#include "SEGGER_RTT.h"
#include "demo_fmc_ad7606.h"



/* 仅允许本文件内调用的函数声明 */
static void sfDispMenu(void);
static void AD7606_Mak(void);
static void AD7606_Disp(void);

static int16_t s_volt[8];
static int16_t s_dat[8];

uint8_t buf[20480];

/* AD7606过采样标识符 */
static const char * AD7606_SampleStr[]= 
{
	"无过采样, 最高支持采样率200Ksps",
	"2倍过采样, 最高支持采样率100Ksps",
	"4倍过采样, 最高支持采样率50Ksps",
	"8倍过采样, 最高支持采样率25Ksps",
	"16倍过采样, 最高支持采样率12.5Ksps",
	"32倍过采样, 最高支持采样率6.25Ksps",
	"64倍过采样, 最高支持采样率3.125Ksps",
};

/* AD7606不同过采样配置支持的最高采样速度 */
static const uint32_t AD7606_SampleFreq[]= 
{
	200000,  /* 无过采样最高采样率200Ksps  */
	100000,  /* 2倍过采样最高采样率100Ksps */
	50000,   /* 4倍过采样最高采样率50Ksps  */
	25000,   /* 8倍过采样最高采样率25Ksps  */
	12500,   /* 16倍过采样最高采样率12.5Ksps  */
	6250,    /* 32倍过采样最高采样率6.25Ksps  */
	3125,    /* 64倍过采样最高采样率3.125Ksps */	
};

/*
	普通的JLINK时钟速度8 - 12MHz时， J-Scope的速度基本可以达到500KB/S（注意，单位是字节）
	AD7606的最高采样率是200Ksps，16bit，那么一路采集就有400KB/S的速速，所以要根据设置的采
    样率设置要显示的J-Scope通道数，如果超出了最高通信速度，波形显示会混乱。

	200Ksps时，实时显示1路
	100Ksps时，实时显示2路
	50Ksps时， 实时显示4路
	25Ksps时， 实时显示8路

	实际速度以底栏的展示为准。

	SEGGER RTT的不同通道由函数SEGGER_RTT_ConfigUpBuffer第2个参数决定
*/
extern __align(16) int16_t g_sAd7606Buf[];   

/* DMA传输完成回调函数 */
void AD7606_DmaCplCb(DMA_HandleTypeDef *hdma)
{
	SEGGER_RTT_Write(1, &g_sAd7606Buf[0], 2);
}

/* DMA半传输完成回调函数 */
void AD7606_DmaHalfCplCb(DMA_HandleTypeDef *hdma)
{
	SEGGER_RTT_Write(1, &g_sAd7606Buf[0], 2);
}


/*
*********************************************************************************************************
*	函 数 名: DemoFmcAD7606
*	功能说明: AD7606测试
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void DemoFmcAD7606(void)
{
	uint8_t ucKeyCode;
	uint8_t ucRefresh = 0;

	
	sfDispMenu();		/* 打印命令提示 */

	ucRefresh = 0;		/* 数据在串口刷新的标志 */
	
	AD7606_SetOS(AD_OS_NO);		/* 无过采样 */
	AD7606_SetInputRange(1);	/* 0表示输入量程为正负5V, 1表示正负10V */
	AD7606_StartConvst();		/* 启动1次转换 */
	
	/* 上电默认采样率 */
	g_tAD7606.ucOS = 1;        		/* 2倍过采样 */
	AD7606_StartRecord(100000);		/* 启动100kHz采样速率 */
	AD7606_SetOS(g_tAD7606.ucOS);   /* 设置2倍过采样 */
	

	bsp_StartAutoTimer(0, 500);	/* 启动1个500ms的自动重装的定时器 */
	bsp_StartAutoTimer(3, 200);	/* 启动1个200ms的自动重装的定时器 */
	
	/*
		配置通道1，上行配置
		默认情况下，J-Scope仅显示1个通道。
		上传1个通道的波形，配置第2个参数为JScope_i2
		上传2个通道的波形，配置第2个参数为JScope_i2i2
		上传3个通道的波形，配置第2个参数为JScope_i2i2i2
		上传4个通道的波形，配置第2个参数为JScope_i2i2i2i2
		上传5个通道的波形，配置第2个参数为JScope_i2i2i2i2i2
		上传6个通道的波形，配置第2个参数为JScope_i2i2i2i2i2i2
		上传7个通道的波形，配置第2个参数为JScope_i2i2i2i2i2i2i2
		上传8个通道的波形，配置第2个参数为JScope_i2i2i2i2i2i2i2i2
	*/	
	SEGGER_RTT_ConfigUpBuffer(1, "JScope_i2", buf, 20480, SEGGER_RTT_MODE_NO_BLOCK_SKIP);

	while(1)
	{
		bsp_Idle();		/* 这个函数在bsp.c文件。用户可以修改这个函数实现CPU休眠和喂狗 */
		
		/* 判断定时器超时时间 */
		if (bsp_CheckTimer(3))	
		{
			/* 每隔100ms 进来一次 */  
			bsp_LedToggle(4);
		}

		if (bsp_CheckTimer(0))
		{
			ucRefresh = 1;	/* 刷新显示 */
		}
		
		if (ucRefresh == 1)
		{
			ucRefresh = 0;

			/* 处理数据 */
			AD7606_Mak();
										 
			/* 打印ADC采样结果 */
			AD7606_Disp();		
		}

		/* 按键检测由后台systick中断服务程序实现，我们只需要调用bsp_GetKey读取键值即可。这个函数不会
		等待按键按下，这样我们可以在while循环内做其他的事情 */
		ucKeyCode = bsp_GetKey();	/* 读取键值, 无键按下时返回 KEY_NONE = 0 */
		if (ucKeyCode != KEY_NONE)
		{
			
			switch (ucKeyCode)
			{
				case KEY_DOWN_K1:						/* K1键按下 切换量程 */
					if (g_tAD7606.ucRange == 0)
					{
						AD7606_SetInputRange(1);
					}
					else
					{
						AD7606_SetInputRange(0);
					}
					ucRefresh = 1;
					break;

				case KEY_DOWN_K2:						/* K2键按下 */
					g_tAD7606.ucOS = 1;        			/* 2倍过采样 */
					AD7606_StartRecord(100000);			/* 启动100kHz采样速率 */
					AD7606_SetOS(g_tAD7606.ucOS);       /* 设置2倍过采样 */
					break;

				case KEY_DOWN_K3:						/* K3键按下 */
					AD7606_StopRecord();				/* 停止记录 */
					break;

				case JOY_DOWN_U:						/* 摇杆UP键按下 */
					if (g_tAD7606.ucOS < 6)
					{
						g_tAD7606.ucOS++;
					}
					
					AD7606_SetOS(g_tAD7606.ucOS);

					AD7606_StartRecord(AD7606_SampleFreq[g_tAD7606.ucOS]);	/* 启动当前过采样下最高速度 */
					
					ucRefresh = 1;
					break;

				case JOY_DOWN_D:						/* 摇杆DOWN键按下 */
					if (g_tAD7606.ucOS > 0)
					{
						g_tAD7606.ucOS--;
					}
					AD7606_SetOS(g_tAD7606.ucOS);
					ucRefresh = 1;

					AD7606_StartRecord(AD7606_SampleFreq[g_tAD7606.ucOS]);	/* 启动当前过采样下最高速度 */
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
*	函 数 名: AD7606_Mak
*	功能说明: 处理采样后的数据
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void AD7606_Mak(void)
{
	uint8_t i;

	for (i = 0; i < 8; i++)
	{		
		/* 
			32767 = 5V , 这是理论值，实际可以根据5V基准的实际值进行公式矫正 
			volt[i] = ((int16_t)dat[i] * 5000) / 32767;	计算实际电压值（近似估算的），如需准确，请进行校准            
			volt[i] = dat[i] * 0.3051850947599719
		*/	
		s_dat[i] = g_sAd7606Buf[i];
		if (g_tAD7606.ucRange == 0)
		{
			s_volt[i] = (g_sAd7606Buf[i] * 5000) / 32767;
		}
		else
		{
			s_volt[i] = (g_sAd7606Buf[i] * 10000) / 32767;
		}
	}
}
 
/*
*********************************************************************************************************
*	函 数 名: AD7606_Disp
*	功能说明: 显示采样后的数据
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void AD7606_Disp(void)
{
	int16_t i;	
	int16_t iTemp;

	/* 打印采集数据 */
	printf(" OS  =  %s \r\n", AD7606_SampleStr[g_tAD7606.ucOS]);
	
	for (i = 0; i < 8; i++)
	{                
   		iTemp = s_volt[i];	/* uV  */
		
		if (s_dat[i] < 0)
		{
			iTemp = -iTemp;
            printf(" CH%d = %6d,0x%04X (-%d.%d%d%d V) \r\n", i+1, s_dat[i], (uint16_t)s_dat[i], iTemp /1000, (iTemp%1000)/100, (iTemp%100)/10,iTemp%10);
		}
		else
		{
         	printf(" CH%d = %6d,0x%04X ( %d.%d%d%d V) \r\n", i+1, s_dat[i], (uint16_t)s_dat[i] , iTemp /1000, (iTemp%1000)/100, (iTemp%100)/10,iTemp%10);                    
		}
	}
	printf("\33[%dA", (int)9);  /* 光标上移n行 */		
}

/*
*********************************************************************************************************
*	函 数 名: sfDispMenu
*	功能说明: 显示操作提示菜单
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void sfDispMenu(void)
{
	printf("操作提示:\r\n");
	printf("1. 启动一个自动重装软件定时器，每100ms翻转一次LED2\r\n");
	printf("2. K1键       : 切换量程(5V或10V)\r\n");
	printf("3. K2键       : 进入FMC DMA工作方式\r\n");
	printf("4. K3键       : 停止FMA DMA采集\r\n");
	printf("5. 摇杆上下键 : 调节过采样参数\r\n");
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
