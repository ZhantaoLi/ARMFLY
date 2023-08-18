/*
*********************************************************************************************************
*
*	模块名称 : GT928电容触摸芯片驱动程序
*	文件名称 : bsp_ct928.h
*	版    本 : V1.0
*	说    明 : 头文件
*
*	Copyright (C), 2015-2020, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/
#ifndef _BSP_GT928_H_
#define _BSP_GT928_H_

#define	GT928_I2C_ADDR		0x28		/* 0x28或0xBA，由INT脚上电状态决定。调试过程中,只有0x28能工作 */
	
typedef struct
{
	uint8_t Enable;
	uint8_t TimerCount;

	uint8_t TouchReg;		/* 触摸寄存器 */

	uint8_t Id[10];			/* 监测ID */
	uint16_t X[10];			/* 10个触摸的X轴坐标 */
	uint16_t Y[10];			/* 10个触摸的Y轴坐标 */
	uint16_t Size[10];		/* 触摸点尺寸 */
	
	uint8_t TouchkeyValue;	/* 按键键值 */
}GT928_T;

void GT928_Timer1ms(void);
void GT928_InitHard(void);
void GT928_Scan(void);

extern GT928_T g_GT928;

#endif

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
