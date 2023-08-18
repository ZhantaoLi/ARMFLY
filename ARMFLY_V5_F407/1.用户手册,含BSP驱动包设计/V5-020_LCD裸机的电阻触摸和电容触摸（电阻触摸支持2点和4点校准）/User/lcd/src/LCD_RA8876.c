/*
*********************************************************************************************************
*
*	ģ������ : RA8876оƬ����ģ��
*	�ļ����� : LCD_RA8876.c
*	��    �� : V1.0
*	˵    �� : RA8876�ײ�������������
*	�޸ļ�¼ :
*		�汾��  ����       ����    ˵��
*		V1.0    2016-06-28 armfly  �����װ�
*	Copyright (C), 2016-2020, ���������� www.armfly.com
*
*********************************************************************************************************
*/

#include "bsp.h"
#include "math.h"
#include "lcd_ra8876.h"
#include "main_menu.h"

/* RA8876��Ч���ȫ�ֱ��� */
uint8_t g_Drawing = 0;			/* �Ƿ�������Ч���������� */
uint8_t g_Interface = 0;		/* ��ǰ���� 0:������(��1���ڴ�) 1:�ӽ���(��2���ڴ�) */
uint16_t CanvasWidth;			/* �ڴ��ͼ��� */

extern uint8_t g_TouchType;		/* �������� */
extern uint8_t g_LcdType;		/* LCD������ */
/*
*********************************************************************************************************
*	�� �� ��: RA8876_InitHard
*	����˵��: ��ʼ��RA8876�����������������⣬������Ӧ���ڳ�ʼ����Ϻ��ٵ��� RA8876_SetBackLight()����������
*	��    ��:  ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
/*
  ������3�鲢�ŵ��ڴ棬��һ���������棬�ڶ����Ŵӽ��棬������Ϊ��ת�ڴ棨ÿ��ˢ�½�����д�����ڴ棬�ٸ��Ƶ� ��/�ӽ��� �ڴ棩

			 ---------------->---  ---------------->---   ---------------->---
			|(_usX��_usY)        |					    |					   |
 _usHeight	V     ��1���ڴ�      V 		��2���ڴ�      	V       ��3���ڴ�	   V 
			|                    |						|	                   |
			 ---------------->---  ---------------->---   ---------------->---
				  _usWidth
*/

void RA8876_InitHard(void)
{
	uint8_t id;		
	uint8_t i;
		
	/* ���败���� */	
	for (i = 0; i < 5; i++)
	{
		if (i2c_CheckDevice(STMPE811_I2C_ADDRESS) == 0)
		{
			/*	RA8876 �������ͺ�ʶ��	
			0  = 3.5������480X320��
			1  = 5.0������800X480��
			2  = 7.0������800X480��
			3  = 7.0������1024X600��
			4  = 10.1���� ��1024X600��
			*/							
			STMPE811_InitHard();				/* ���������ò��ܶ�ȡID */
			
			id = STMPE811_ReadIO();				/* ʶ��LCDӲ������ */	
			switch (id)
			{
				case 0:
					g_LcdType = LCD_35_480X320;
					/* �������Ŀ�Ⱥ͸߶� */
					g_LcdHeight = 320;
					g_LcdWidth = 480;
					break;

				case 1:
					g_LcdType = LCD_50_800X480;
					/* �������Ŀ�Ⱥ͸߶� */
					g_LcdHeight = 480;
					g_LcdWidth = 800;
					break;

				case 2:
					g_LcdType = LCD_70_800X480;
					/* �������Ŀ�Ⱥ͸߶� */
					g_LcdHeight = 480;
					g_LcdWidth = 800;
					break;

				case 3:
					g_LcdType = LCD_70_1024X600;
					/* �������Ŀ�Ⱥ͸߶� */
					g_LcdHeight = 600;
					g_LcdWidth = 1024;
					break;

				case 4:
					g_LcdType = LCD_101_1024X600;
					/* �������Ŀ�Ⱥ͸߶� */
					g_LcdHeight = 600;
					g_LcdWidth = 1024;
					break;
				
				default:
					g_LcdType = LCD_70_800X480;
					/* �������Ŀ�Ⱥ͸߶� */
					g_LcdHeight = 480;
					g_LcdWidth = 800;
					break;
			}		
		}
		else		/* Ĭ��Ϊ10.1���� */
		{
			g_LcdType = LCD_101_1024X600;
			/* �������Ŀ�Ⱥ͸߶� */
			g_LcdHeight = 600;
			g_LcdWidth = 1024;
		}
	}
	
	STMPE811_ResetRA8876();			/* ����STMPE811����λRA8876 */
	
	CanvasWidth = g_LcdWidth * 3;
	
	/* ����V5�����������IDʶ��ķ�ʽ����λ��ʶ��֮ǰ���ˡ�ʵ����Ŀ�п��ڴ˴�Ӳ����λ */
//	RA8876_HW_Reset();				/* �ⲿ��λ�������MCU */
//	Check_IC_ready();				/* ���MCU�Ƿ�׼����  */
//	RA8876_SW_Reset();				/* �����λ���ٷ�������û���� */
//	Check_IC_ready();	  

	RA8876_PLL();					/* ����RA8876��PLLʱ�� */
	RA8876_SDRAM_initial();			/* ����SDRAM������ */ 

//	TFT_24bit();//RA8876 only
//	TFT_18bit();//RA8876 only	
	TFT_16bit();					/* ����RA8876Ϊ16λ���������TFT���������������ã� */
//	Without_TFT();//RA8876 only					
	
	/* ���ض���������ѡ�� */
#if defined (MCU_8bit_ColorDepth_8bpp) || defined (MCU_8bit_ColorDepth_16bpp) || defined (MCU_8bit_ColorDepth_24bpp)	
	Host_Bus_8bit();
#endif
#if defined (MCU_16bit_ColorDepth_16bpp) || defined (MCU_16bit_ColorDepth_24bpp_Mode_1) || defined (MCU_16bit_ColorDepth_24bpp_Mode_2)	
	Host_Bus_16bit();				/* ����Ϊ16bit */
#endif

	/* MPU ����ڴ�Ķ�д���ݸ�ʽ */
#ifdef MCU_8bit_ColorDepth_8bpp	
	Data_Format_8b_8bpp();
#endif
#ifdef MCU_8bit_ColorDepth_16bpp	
	Data_Format_8b_16bpp();
#endif
#ifdef MCU_8bit_ColorDepth_24bpp	
	Data_Format_8b_24bpp();
#endif
#ifdef MCU_16bit_ColorDepth_16bpp	
	Data_Format_16b_16bpp();
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_1	
	Data_Format_16b_24bpp_mode1();
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_2	
	Data_Format_16b_24bpp_mode2();
#endif	
	
	MemWrite_Left_Right_Top_Down();	/* �����ڴ�д�뷽�򣨽���ͼ��ģʽ��Ч�� */
//	MemWrite_Top_Down_Left_Right();
	
	
	Graphic_Mode();					/* ͼ��ģʽ,������ȷ�� core task busy �Ƿ�����æµ�������У��� core task busy ��״̬������ */
//	Text_Mode();					/* �ı�ģʽ */	

/* ѡ�� SDRAM Ϊ image/pattern/ʹ�����Զ����͵�����д��Ŀ�ģ�֧�� Read-modify-Write�� */
	Memory_Select_SDRAM();			
/* ѡ�� RGB ɫ�� Gamma table Ϊд��Ŀ�ġ� ÿ����ɫ�Ķ��� 256 bytes��
ʹ������Ҫָ����Ҫд��� gamma table Ȼ��������д�� 256 bytes�� */
//	Memory_Select_Gamma_Table();

	/* ������ɫ��� */
#ifdef MCU_8bit_ColorDepth_8bpp	
	Select_Main_Window_8bpp();		//[10h] Set main window color depth
	Memory_8bpp_Mode();				//[5Eh] Set active memory color depth

	Select_PIP1_Window_8bpp();		//[11h] PIP 1 Window Color Depth
	Select_PIP2_Window_8bpp();		//[11h] PIP 2 Window Color Depth

	BTE_S0_Color_8bpp();			//[92h] Source_0 Color Depth
	BTE_S1_Color_8bpp();			//[92h] Source_1 Color Depth
	BTE_Destination_Color_8bpp();	//[92h] Destination Color Depth	
#endif

#if defined (MCU_8bit_ColorDepth_16bpp) || defined (MCU_16bit_ColorDepth_16bpp)
	Select_Main_Window_16bpp();		//[10h]Set main window color depth
	Memory_16bpp_Mode();			//[5Eh]Set active memory color depth

	Select_PIP1_Window_16bpp();		//[11h] PIP 1 Window Color Depth
	Select_PIP2_Window_16bpp();		//[11h] PIP 2 Window Color Depth

	BTE_S0_Color_16bpp();			//[92h] Source_0 Color Depth
	BTE_S1_Color_16bpp();			//[92h] Source_1 Color Depth
	BTE_Destination_Color_16bpp();	//[92h] Destination Color Depth	
#endif

#if defined (MCU_8bit_ColorDepth_24bpp) || defined (MCU_16bit_ColorDepth_24bpp_Mode_1) || defined (MCU_16bit_ColorDepth_24bpp_Mode_2)	
	Select_Main_Window_24bpp();		//[10h]Set main window color depth
	Memory_24bpp_Mode();			//[5Eh]Set active memory color depth

	Select_PIP1_Window_24bpp();		//[11h] PIP 1 Window Color Depth
	Select_PIP2_Window_24bpp();		//[11h] PIP 2 Window Color Depth

	BTE_S0_Color_24bpp();			//[92h] Source_0 Color Depth
	BTE_S1_Color_24bpp();			//[92h] Source_1 Color Depth
	BTE_Destination_Color_24bpp();	//[92h] Destination Color Depth	
#endif

	Set_LCD_Panel(); 					/* ����LCD��� */
	
/* RA8876�ֲ� P157  LCD ��ʾ���ƻ����� */
	Main_Image_Start_Address(0);		/* ����ʾ���ڵ���ʼ��ַ���Դ棬ȡSDRAM�ڴ�����ݣ����ɵ�����ʾͼƬ��λ�� */				
	Main_Image_Width(CanvasWidth);		/* ����ʾ���ڿ�ȣ��͵�ͼ�ڴ������ó�һ�������ݲ��ܶ�Ӧ�� */
	Main_Window_Start_XY(0,0);			/* ������ʾ�Դ����ʼ���꿪ʼ��ȡSDRAM�ڴ��е����� */
	
/* RA8876�ֲ� P51 */
	Canvas_Image_Start_address(0);		/* �ӵ�ַ0��ʼд��SDRAM�ڴ� */
	Canvas_image_width(CanvasWidth);	/* SDRAM�ڴ��ͼ��� */
	
/* ��������������꣬�����������������ͼ��Χ�� */	
	Active_Window_XY(0,0);				/* �趨���������������,x��y֮ǰ�����ݶ�����д���ڴ� */
/* CanvasWidth * g_LcdHeight Ϊ������ͼ��Χ����ȱ�ʾ����������һ�����ú��ڴ��ͼ���CanvasWidth��� */
	Active_Window_WH(CanvasWidth, g_LcdHeight);		/* �趨�������ڵĸ߶ȺͿ�� */

	Memory_XY_Mode();					/* �ڴ�����Ѱַ���� */
//	Memory_Linear_Mode();

	Set_Serial_Flash_IF();				/* ���ô���Flash�ӿ� */

	Goto_Pixel_XY(0,0);					/* ������������Ϊ��0,0�� */

	Display_ON();						/* ����ʾ */
	
	bsp_DelayMS(1);						/* ��Ҫ��ʱһ�� */
	
/* ����LCD_ONOFF������������ʱû�õ��� */
	Set_GPIO_D_In_Out((1<<0) | (1<<1)| (1<<2) | (0<<3)| (0<<4)| (0<<5)| (1<<6)| (0<<7));
	Write_GPIO_D_7_0((1<<0) | (1<<1)| (1<<2) | (1<<3)| (1<<4)| (1<<5)| (1<<6)| (1<<7));
}

/*
*********************************************************************************************************
*	�� �� ��: RA8876_DispOn
*	����˵��: ����ʾ
*	��    ��:  ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void RA8876_DispOn(void)
{
	Display_ON();
}

/*
*********************************************************************************************************
*	�� �� ��: RA8876_DispOff
*	����˵��: �ر���ʾ
*	��    ��:  ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void RA8876_DispOff(void)
{
	Display_OFF();
}

/*
*********************************************************************************************************
*	�� �� ��: RA8876_ClrScr
*	����˵��: �����������ɫֵ����.RA8875֧��Ӳ����ɫ��䡣�ú������Ե�ǰ�������ʾ���ڽ�������. ��ʾ
*			 ���ڵ�λ�úʹ�С�� RA8875_SetDispWin() ������������
*	��    ��:  _usColor : ����ɫ
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void RA8876_ClrScr(uint16_t _usColor)
{
	uint16_t width, height;
	
	if (g_LcdDirection > 1)			/* ���� */
	{
		width = g_LcdHeight;
		height = g_LcdWidth;
	}
	else							/* ���� */
	{
		width = g_LcdWidth;
		height = g_LcdHeight;
	}
	
	/* ������ڵ���LCD_BeginDrawAll����������,������������һ��SDRAM�ڴ�顣
	   ���һ�����ʾ���������������嵱ǰSDRAM�ڴ�顣
	*/

	if (g_Drawing == 1)		/* ������Ч��ͼ */
	{
		BTE_Solid_Fill(0, CanvasWidth, width * 2, 0, _usColor, width, height);		/* ����3���ڴ� */
	}
	else
	{
		if (g_Interface == 0)
		{
			BTE_Solid_Fill(0, CanvasWidth, 0, 0, _usColor, width, height);			/* ����1���ڴ� */
		}
		else
		{
			BTE_Solid_Fill(0, CanvasWidth, width, 0, _usColor, width, height);		/* ����2���ڴ� */
		}
	}
}

/*
*********************************************************************************************************
*	�� �� ��: RA8876_PutPixel
*	����˵��: ��1������
*	��    ��:
*			_usX,_usY : ��������
*			_usColor  :������ɫ
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void RA8876_PutPixel(uint16_t _usX, uint16_t _usY, uint16_t _usColor)
{
	if (g_LcdDirection == 0)					/* ���� */
	{
		Goto_Pixel_XY(_usX, _usY);				/* ���õ����� */
	}
	else if (g_LcdDirection == 1)				/* ��������ת180�� */
	{
		Goto_Pixel_XY(g_LcdWidth - _usX, _usY);				
	}
	else if (g_LcdDirection == 2)				/* ���� */
	{
		Goto_Pixel_XY(_usY, _usX);			
	}
	else if (g_LcdDirection == 3)				/* ��������ת180�� */
	{
		Goto_Pixel_XY(g_LcdHeight - _usY, _usX);				
	}
	
	RA8876_CmdWrite(0x04);						/* �ڴ�SDRAM���ݶ�/д�ӿ� */
  	Check_Mem_WR_FIFO_not_Full();

#ifdef MCU_8bit_ColorDepth_8bpp	
	RA8876_DataWrite(_usColor);
#endif
	
#ifdef MCU_8bit_ColorDepth_16bpp	
	RA8876_DataWrite(_usColor);
	Check_Mem_WR_FIFO_not_Full();
	RA8876_DataWrite(_usColor >> 8);
#endif
	
#ifdef MCU_8bit_ColorDepth_24bpp	
	RA8876_DataWrite(_usColor);
	Check_Mem_WR_FIFO_not_Full();
	RA8876_DataWrite(_usColor >> 8);
	Check_Mem_WR_FIFO_not_Full();
	RA8876_DataWrite(_usColor >> 16);
#endif

#ifdef MCU_16bit_ColorDepth_16bpp			/* ѡ��16λ16bpp��ɫ��� */
	RA8876_DataWrite(_usColor);				/* д����ɫ */
#endif

#ifdef MCU_16bit_ColorDepth_24bpp_Mode_2	
	RA8876_DataWrite(_usColor);
	Check_Mem_WR_FIFO_not_Full();
	RA8876_DataWrite(_usColor >> 16);
#endif
}

/*
*********************************************************************************************************
*	�� �� ��: RA8876_GetPixel
*	����˵��: ��ȡ1������
*	��    ��:
*			_usX,_usY : ��������
*			_usColor  :������ɫ
*	�� �� ֵ: RGB��ɫֵ
*********************************************************************************************************
*/
uint16_t RA8876_GetPixel(uint16_t _usX, uint16_t _usY)
{
	uint16_t usRGB;

	if (g_LcdDirection == 0)					/* ���� */
	{
		Goto_Pixel_XY(_usX, _usY);				/* ���õ����� */
	}
	else if (g_LcdDirection == 1)				/* ��������ת180�� */
	{
		Goto_Pixel_XY(g_LcdWidth - _usX, _usY);				
	}
	else if (g_LcdDirection == 2)				/* ���� */
	{
		Goto_Pixel_XY(_usY, _usX);			
	}
	else if (g_LcdDirection == 3)				/* ��������ת180�� */
	{
		Goto_Pixel_XY(g_LcdHeight - _usY, _usX);				
	}
	
	RA8876_CmdWrite(0x04);			/* �ڴ��д�˿� */
	usRGB = RA8876_DataRead();		/* ��һ�ζ�ȡ���ݶ��� */
	bsp_DelayUS(1);				/* �������Ҫ��ʱ��������Ҫ��ʱ��û������ */
	usRGB = RA8876_DataRead();	

	return usRGB;
}

/*
*********************************************************************************************************
*	�� �� ��: RA8876_DrawLine
*	����˵��: ����RA8876��Ӳ�����ܣ���2��仭һ��ֱ�ߡ�
*	��    ��:
*			_usX1, _usY1 :��ʼ������
*			_usX2, _usY2 :��ֹ��Y����
*			_usColor     :��ɫ
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void RA8876_DrawLine(uint16_t _usX1 , uint16_t _usY1 , uint16_t _usX2 , uint16_t _usY2 , uint16_t _usColor)
{
	uint16_t temp;
	
	if (g_LcdDirection == 0)
	{
	}
	else if (g_LcdDirection == 1) 
	{
		/* X�᾵��Y�᾵�� */
		_usX1 = g_LcdWidth - _usX1;
		_usY1 = _usY1; 
		
		_usX2 = g_LcdWidth - _usX2;
		_usY2 = _usY2; 
	}
	else if (g_LcdDirection == 2)
	{
		temp = _usX1;
		_usX1 = _usY1;
		_usY1 = temp;
		
		temp = _usX2;
		_usX2 = _usY2;
		_usY2 = temp;
	}
	else if (g_LcdDirection == 3)
	{
		temp = _usY1;
		_usY1 = _usX1;
		_usX1 = g_LcdHeight - temp;
		
		temp = _usY2;
		_usY2 = _usX2;
		_usX2 = g_LcdHeight - temp;
	}
	
	Draw_Line(_usColor, _usX1 , _usY1 , _usX2 , _usY2);
}

/*
*********************************************************************************************************
*	�� �� ��: RA8876_DrawRect
*	����˵��: ����RA8876Ӳ�����ܻ��ƾ���
*	��    ��:
*			_usX,_usY:�������Ͻǵ�����
*			_usHeight :���εĸ߶�
*			_usWidth  :���εĿ��
*			_usColor  :��ɫ
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void RA8876_DrawRect(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth, uint16_t _usColor)
{
	uint16_t temp;
	
	if (g_LcdDirection > 1)	/* ����  */
	{
		temp = _usX;
		_usX = _usY;
		_usY = temp;
		
		temp = _usHeight;
		_usHeight = _usWidth;
		_usWidth = temp;
	}	
	
	if (g_LcdDirection == 0)				/* ������ */
	{
		Draw_Square(_usColor, _usX, _usY, _usX + _usWidth - 1, _usY + _usHeight - 1);
	}
	else if (g_LcdDirection == 1) 			/* 180����ת */
	{
		_usX = g_LcdWidth - _usX;
		_usY = _usY; 
		Draw_Square(_usColor, _usX - 1, _usY, _usX - (_usWidth), _usY + (_usHeight - 1));
	}
	else if (g_LcdDirection == 2)			/* 90����ת */
	{
		Draw_Square(_usColor, _usX, _usY, _usX + _usWidth - 1, _usY + _usHeight - 1);
	}
	else if (g_LcdDirection == 3)			/* 270����ת */
	{
		_usX = _usX;
		_usY = g_LcdWidth - (_usY + _usHeight);
		
		_usX = g_LcdHeight - _usX;
		_usY = g_LcdWidth - _usY;
		
		Draw_Square(_usColor, _usX - 1, _usY - 1, _usX - (_usWidth), _usY - (_usHeight));
	}
}

/*
*********************************************************************************************************
*	�� �� ��: RA8875_DrawCircle
*	����˵��: ����һ��Բ���ʿ�Ϊ1������
*	��    ��:
*			_usX,_usY  :Բ�ĵ�����
*			_usRadius  :Բ�İ뾶
*			_usColor  :��ɫ
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void RA8876_DrawCircle(uint16_t _usX, uint16_t _usY, uint16_t _usRadius, uint16_t _usColor)
{
	uint16_t temp;
	
	if (g_LcdDirection > 1)	/* ����  */
	{	
		temp = _usX;
		_usX = _usY;
		_usY = temp;
	}

	if (g_LcdDirection == 0)				/* ������ */
	{
	}
	else if (g_LcdDirection == 1) 			/* 180����ת */
	{
		_usX = g_LcdWidth - _usX;
		_usY = _usY;
	}
	else if (g_LcdDirection == 2)			/* 90����ת */
	{
	}
	else if (g_LcdDirection == 3)			/* 270����ת */
	{
		_usX = g_LcdHeight - _usX;
		_usY = _usY; 
	}	
	
	Draw_Circle(_usColor, _usX, _usY, _usRadius);
}

/*
*********************************************************************************************************
*	�� �� ��: RA8876_FillRect
*	����˵��: ����RA8876Ӳ���������һ������Ϊ��ɫ
*	��    ��:
*			_usX,_usY:�������Ͻǵ�����
*			_usHeight :���εĸ߶�
*			_usWidth  :���εĿ��
*			_usColor  :��ɫ
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void RA8876_FillRect(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth, uint16_t _usColor)
{
	uint16_t temp;
	
	if (g_LcdDirection > 1)	/* ����  */
	{
		temp = _usX;
		_usX = _usY;
		_usY = temp;
		
		temp = _usHeight;
		_usHeight = _usWidth;
		_usWidth = temp;
	}	
	
	if (g_LcdDirection == 0)				/* ������ */
	{
		Draw_Square_Fill(_usColor, _usX, _usY, _usX + _usWidth - 1, _usY + _usHeight - 1);
	}
	else if (g_LcdDirection == 1) 			/* 180����ת */
	{
		_usX = g_LcdWidth - _usX;
		_usY = _usY; 
		Draw_Square_Fill(_usColor, _usX, _usY, _usX - (_usWidth), _usY + (_usHeight - 1));
	}
	else if (g_LcdDirection == 2)			/* 90����ת */
	{
		Draw_Square_Fill(_usColor, _usX, _usY, _usX + _usWidth - 1, _usY + _usHeight - 1);
	}
	else if (g_LcdDirection == 3)			/* 270����ת */
	{
		_usX = _usX;
		_usY = g_LcdWidth - (_usY + _usHeight);
		
		_usX = g_LcdHeight - _usX;
		_usY = g_LcdWidth - _usY;
		
		Draw_Square_Fill(_usColor, _usX, _usY, _usX - (_usWidth), _usY - (_usHeight));
	}
}

/*
*********************************************************************************************************
*	�� �� ��: RA8876_DrawBMP
*	����˵��: ��LCD����ʾһ��BMPλͼ��λͼ����ɨ�����:�����ң����ϵ���
*	��    ��:
*			_usX, _usY : ͼƬ������
*			_usHeight  :ͼƬ�߶�
*			_usWidth   :ͼƬ���
*			_ptr       :ͼƬ����ָ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void RA8876_DrawBMP(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth, uint16_t *_ptr)
{
	uint16_t temp;
	
	if (g_LcdDirection == 0)
	{
	}
	else if (g_LcdDirection == 1)	/* ������ת180�� */
	{
		_usX = g_LcdWidth - _usWidth;
		_usY = _usY;
	}
	else if (g_LcdDirection == 2)	/* ������ת90�� */
	{
		temp = _usHeight;
		_usHeight = _usWidth;
		_usWidth = temp;
	}
	MPU16_16bpp_Memory_Write(_usX, _usY, _usWidth, _usHeight, _ptr);
}
	
/*
*********************************************************************************************************
*	�� �� ��: RA8876_FillCircle
*	����˵��: ���һ��Բ
*	��    ��:
*			_usX,_usY  :Բ�ĵ�����
*			_usRadius  :Բ�İ뾶
*			_usColor   :��ɫ
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void RA8876_FillCircle(uint16_t _usX, uint16_t _usY, uint16_t _usRadius, uint16_t _usColor)
{
	uint16_t temp;
	
	if (g_LcdDirection > 1)	/* ����  */
	{	
		temp = _usX;
		_usX = _usY;
		_usY = temp;
	}

	if (g_LcdDirection == 0)				/* ������ */
	{
	}
	else if (g_LcdDirection == 1) 			/* 180����ת */
	{
		_usX = g_LcdWidth - _usX;
		_usY = _usY;
	}
	else if (g_LcdDirection == 2)			/* 90����ת */
	{
	}
	else if (g_LcdDirection == 3)			/* 270����ת */
	{
		_usX = g_LcdHeight - _usX;
		_usY = _usY; 
	}	
	
	Draw_Circle_Fill(_usColor, _usX, _usY, _usRadius);
}

/*
*********************************************************************************************************
*	�� �� ��: RA8875_DispBmpInFlash
*	����˵��: ��ʾRA8875��Ҵ���Flash�е�BMPλͼ.
*			    λͼ��ʽ:������ ���ϵ���ɨ��, ÿ������2�ֽ�, RGB = 565 ��ʽ, ��λ��ǰ.
*	��    ��:
*			_usX, _usY : ͼƬ������
*			_usHeight  :ͼƬ�߶�
*			_usWidth   :ͼƬ���
*			_uiFlashAddr       :����Flash��ַ
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void RA8876_DispBmpInFlash(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth,
	uint32_t _uiFlashAddr)
{	
	/* pdf 179ҳ 7-10-1 �����ڴ�ֱ�Ӵ�ȡģʽ
		1. �趨�������ڷ�Χ (REG[30h] ~REG[37h])���ڴ�д��λ�� (REG[46h] ~REG[49h])
		2. �趨Serial Flash/ROM ��̬ (REG[05h])
		3. �趨 �ڴ�ֱ�Ӵ�ȡ������Դ��ʼλ�� (REG[B0h] ~REG[B2h])
		4. �趨 �ڴ�ֱ�Ӵ�ȡ������ (REG[B4h] ��REG[B5h])
		5. �趨 �ڴ�ֱ�Ӵ�ȡ����߶� (REG[B6h] �� REG[B7h])
		6. �趨�ڴ�ֱ�Ӵ�ȡ��ԴͼƬ��� (REG[B8h] �� REG[B9h])
		7. �����ڴ�ֱ�Ӵ�ȡΪ�������ģʽ (REG[BFh] bit 1)
		8. �����ڴ�ֱ�Ӵ�ȡ��ʼѶ���Ҽ���ڴ�ֱ�Ӵ�ȡæµѶ�� (REG[BFh] bit 0)
	*/
	
/*
	
. S0 �ĵ�ַ�������� REG [93h], REG[94h], REG[95h], REG[96h], REG[97h], REG[98h],
REG[99h], REG[9Ah], REG[9Bh], REG[9Ch]
2. S1 �ĵ�ַ��������[9Dh], REG[9Eh], REG[9Fh], REG[A0h], REG[A1h] , REG[A2h],
REG[A3h], REG[A4h], REG[A5h], REG[A6h]
3. D �ĵ�ַ�������� REG [A7h], REG[A8h], REG[A9h], REG[AAh], REG [ABh], REG[ACh],
REG[ADh], REG[AEh], REG[AFh], REG[B0h]	
*/
#ifdef MCU_8bit_ColorDepth_8bpp					   //setting in UserDef.h
	DMA_24bit(1,0,0,0,1024,640,1024,0);
#endif
#ifdef MCU_8bit_ColorDepth_16bpp				   //setting in UserDef.h
	DMA_24bit(1,0,0,0,1024,640,1024,655360);
#endif
#ifdef MCU_8bit_ColorDepth_24bpp				   //setting in UserDef.h
	DMA_24bit(1,0,0,0,1024,640,1024,1966080);
#endif
#ifdef MCU_16bit_ColorDepth_16bpp				   //setting in UserDef.h
//	DMA_24bit(1,0,0,0,1024,640,1024,655360);
	DMA_24bit(1,0,_usX,_usY,_usWidth,_usHeight,_usWidth,_uiFlashAddr);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_1		   //setting in UserDef.h
	DMA_24bit(1,0,0,0,1024,640,1024,1966080);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_2		   //setting in UserDef.h
	DMA_24bit(1,0,0,0,1024,640,1024,1966080);
#endif
}

/*
*********************************************************************************************************
*	�� �� ��: RA8875_DispStr
*	����˵��: ��ʾ�ַ������ַ�����������RA8875��ӵ��ֿ�оƬ��֧�ֺ���.
*			������ɫ��������ɫ���Ƿ�ͨ͸�������ĺ�����������
*			void RA8875_SetFrontColor(uint16_t _usColor);
*			void RA8875_SetBackColor(uint16_t _usColor);
*	��    ��:  _usX, _usY : ��ʾλ�ã����꣩
*			 _ptr : AascII�ַ�������0����
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void RA8876_DispStr(uint8_t SCS, uint16_t _usX, uint16_t _usY, uint32_t FontColor, uint32_t BackGroundColor, char *_ptr)
{	
	uint16_t temp;
	
	if (g_LcdDirection == 0)
	{
		Font_0_degree();		/* ���ֲ���ת */
	}
	else if (g_LcdDirection == 2)
	{
		temp = _usX;
		_usX = _usY;
		_usY = temp;
		
		Font_90_degree();		/* ������ת90�� */
	}
	
	Select_SFI_Font_Mode();			/* [B7H]bit6����Flashѡ��ģʽ���ַ�ģʽ-ʹ����CGROM */

	/* �����ⲿ����оƬ�ͺ�Ϊ GT23L32S4W, ����ΪGB2312 */
	CGROM_Select_Genitop_FontROM();	/* [CCH]bit7-6�ַ�Դѡ���ⲿCGROMΪ�ַ���Դ����ͨ���棩 */

	SPI_Clock_Period(0);		/* [BBH]SPIʱ�����ڣ�SPI CLK = System Clock / 2*(Clk+1) 8M����Flash�ķ����ٶ�:SPI ʱ��Ƶ��:80MHz(max.) */
	Set_GTFont_Decoder(0x01);   /* [CFH]��ͨ�ַ����룺GB2312 */
	if(SCS == 0)
	{
		Select_SFI_0();				/* ����flashѡ�񣺴�������/ROM 0 ��ѡ�� */
	}

	if(SCS == 1)
	{
		Select_SFI_1();
	}
#ifdef MCU_8bit_ColorDepth_8bpp	
	Foreground_color_256(FontColor);		/* ��������ɫ */
	Background_color_256(BackGroundColor);	/* ���ñ���ɫ */
#endif
#ifdef MCU_8bit_ColorDepth_16bpp	
	Foreground_color_65k(FontColor);
	Background_color_65k(BackGroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_24bpp	
	Foreground_color_16M(FontColor);
	Background_color_16M(BackGroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_16bpp	
	Foreground_color_65k(FontColor);		/* ����������ɫ */
	Background_color_65k(BackGroundColor);	/* ���ñ�����ɫ */
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_1	
	Foreground_color_16M(FontColor);
	Background_color_16M(BackGroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_2	
	Foreground_color_16M(FontColor);
	Background_color_16M(BackGroundColor);
#endif
	Text_Mode();					/* ����Ϊ�ı�ģʽ */
	//Active_Window_XY(0, 0);		/* �������������� */
	if (g_LcdDirection > 1)			/* ���� */
	{
		Active_Window_WH(g_LcdHeight, g_LcdWidth);
	}
	else
	{
		Active_Window_WH(g_LcdWidth, g_LcdHeight);
	}
	Goto_Text_XY(_usX, _usY);		/* �����ı���ʾλ�ã�ע���ı�ģʽ��д�����ͼ��ģʽ��д�����ǲ�ͬ�ļĴ��� */
	//  sprintf(tmp1,"%s",tmp2); 
	//  Show_String(tmp1);
	Show_String(_ptr);		/* ��ʾ�ַ�����������Ϊͼ��ģʽ */
}

/*
*********************************************************************************************************
*	�� �� ��: RA8876_SetFont
*	����˵��: �ı�ģʽ�������������塢�о���־�
*	��    ��:
*			_ucFontType : ��������: RA_FONT_16, RA_FONT_24, RA_FONT_32
*			_ucLineSpace : �о࣬���ص�λ (0-31)
*			_ucCharSpace : �־࣬���ص�λ (0-63)
*
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void RA8876_SetFont(uint8_t _ucFontType, uint8_t _ucLineSpace, uint8_t _ucCharSpace)
{
	/*
		[D0H]������ģʽ�£������趨���ּ���о� (��λ: ����) ��
		ֻ�е�5��bit��Ч��0-31
	*/
	if (_ucLineSpace >31)
	{
		_ucLineSpace = 31;
	}
	Font_Line_Distance(_ucLineSpace);		/* �����м�� */
	
	/*
		[D1H] �����ַ���ࣨ���ص�λ��0-63���������壨16*16��24*24��32*32��
	*/
	if (_ucCharSpace > 63)
	{
		_ucCharSpace = 63;	
	}
	Set_Font_to_Font_Width(_ucCharSpace);	/* �����ַ���� */
	
	if (_ucFontType > RA_FONT_32)
	{
		_ucFontType = RA_FONT_16;
	}
	
	if (_ucFontType == RA_FONT_16)
	{
		Font_Select_8x16_16x16();			/* ��������ߴ� */
	}
	else if (_ucFontType == RA_FONT_24)
	{
		Font_Select_12x24_24x24();
	}
	else if (_ucFontType == RA_FONT_32)
	{
		Font_Select_16x32_32x32();
	}
}

/*
*********************************************************************************************************
*	�� �� ��: RA8876_SetDirection
*	����˵��: ������ʾ����
*	��    ��:  _ucDir : ��ʾ������� 0 ��������, 1=����180�ȷ�ת, 2=����, 3=����180�ȷ�ת
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void RA8876_SetDirection(uint8_t _ucDir)
{
/*
	Display Configuration Register[12H]
bit3:
	VDIR
	Vertical Scan direction
	0 : ���ϵ��¡�
	1 : ���µ���
*/
	if (_ucDir == 0)
	{
		VSCAN_T_to_B();		/* ���������� */
		MemWrite_Left_Right_Top_Down();
	}
	else if (_ucDir == 1)
	{
		VSCAN_B_to_T();		/* ������180�� */
		MemWrite_Right_Left_Top_Down();		/* 01b */
	}
	else if (_ucDir == 2)
	{
		VSCAN_B_to_T();		/* ������90�� */
		MemWrite_Top_Down_Left_Right();		/* 10b */
	}
	else if (_ucDir == 3)
	{
		VSCAN_T_to_B();		/* ������270�� */
		MemWrite_Left_Right_Top_Down();
	}
	
	if (_ucDir > 1)	/* ����  */
	{
		uint16_t temp;
		
		if (g_LcdHeight < g_LcdWidth)
		{
			temp = g_LcdHeight;
			g_LcdHeight = g_LcdWidth;
			g_LcdWidth = temp;
		}
		
	}		
	else	/* ���� */
	{
		uint16_t temp;
		
		if (g_LcdHeight > g_LcdWidth)
		{
			temp = g_LcdHeight;
			g_LcdHeight = g_LcdWidth;
			g_LcdWidth = temp;
		}
	}
}

/*
*********************************************************************************************************
*	�� �� ��: RA8876_SetBackLight
*	����˵��: ����RA8875оƬ��PWM1��ؼĴ���������LCD����
*	��    ��:  _bright ���ȣ�0����255������
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void RA8876_SetBackLight(uint8_t _bright)
{
/* 
	PWM CLK = (Core CLK / Prescalar ) /2^ divided clock 
	PWM CLK = (120*1000*1000) / 10 /2 = 6M
	PWMƵ�� = PWM CLK / TCNTBn = 6*1000*1000/255 = 23529 = 23.5KHz
	ռ�ձ� = TCMPBn / TCNTBn
*/
	/* 1: ��PWM    1: 1��Ƶ   10: Prescalar   255: TCNTBn   _bright: TCMPBn */
	
	if (_bright == 0)
	{
		PWM1(0,1,10,255,_bright); 					/* ��PWM���� */
	}
	else
	{
		PWM1(1,1,10,255,_bright); 					/* ��PWM���� */
	}
}

/*
*********************************************************************************************************
*	�� �� ��: RA8876_FillRoundRect
*	����˵��: ����RA8876Ӳ���������Բ�Ǿ���
*	��    ��:
*			_usX,_usY:�������Ͻǵ�����
*			_usHeight :���εĸ߶�
*			_usWidth  :���εĿ��
*			_usArc    :Բ�ǵĻ�
*			_usColor  :��ɫ
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void RA8876_FillRoundRect(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth, 
	uint16_t _usArc, uint16_t _usColor)
{
/*
	RA8876 ֧�ֻ���Բ�Ǿ��Σ�ʹ���߿���ʹ�������� MPU ���ڴ�ɻ���Բ�Ǿ��Ρ������趨Բ�Ǿ�����ʼ
�� REG[68h~6Ch] �������� REG[6Dh~6Fh]��Բ�Ǿ��γ�����뾶 REG[77h~7Ah]����ɫ REG[D2h~D4h]��
����趨����ͼ��ΪԲ�Ǿ��� REG[76h] Bit5~4 Ϊ 11b���������� REG[76h] Bit7 = 1����ô RA8876 ������
��ͼ�ϻ���Բ�Ǿ��Σ�����һ���ģ�ʹ���߿����趨�������� REG[76h] Bit6 = 1��
	
	 ---------------->---
	|(_usX��_usY)        |
	V                    V  _usHeight
	|                    |
	 ---------------->---
		  _usWidth
*/
	uint16_t temp;
		
	if (g_LcdDirection > 1)	/* ����  */
	{
		temp = _usX;
		_usX = _usY;
		_usY = temp;

		temp = _usHeight;
		_usHeight = _usWidth;
		_usWidth = temp;		
	}
	
	if (g_LcdDirection == 0)			/* ���� */
	{
		Draw_Circle_Square_Fill(_usColor, _usX, _usY,  _usX + _usWidth - 1, _usY + _usHeight - 1, _usArc, _usArc);
	}
	else if (g_LcdDirection == 1)		/* ����,��ת180��*/
	{
		_usX = g_LcdWidth - _usX;
		_usY = _usY;
		
		Draw_Circle_Square_Fill(_usColor, _usX, _usY,  _usX - (_usWidth - 1), _usY + (_usHeight - 1), _usArc, _usArc);
	}
	else if (g_LcdDirection == 2)		/* ��������ת90�� */
	{	
		Draw_Circle_Square_Fill(_usColor, _usX, _usY,  _usX + (_usWidth - 1), _usY + (_usHeight - 1), _usArc, _usArc);
	}
	else if (g_LcdDirection == 3)		/* ��������ת270�� */
	{
		_usX = g_LcdHeight - (_usX + _usWidth - 1);
		_usY = _usY;
		
		Draw_Circle_Square_Fill(_usColor, _usX, _usY,  _usX + _usWidth - 1, _usY + _usHeight - 1, _usArc, _usArc);
	}
}

/*
*********************************************************************************************************
*	�� �� ��: RA8876_DrawRoundRect
*	����˵��: ����RA8875Ӳ�����ܻ���Բ�Ǿ���
*	��    ��:
*			_usX,_usY:�������Ͻǵ�����
*			_usHeight :���εĸ߶�
*			_usWidth  :���εĿ��
*			_usArc    :Բ�ǵĻ�
*			_usColor  :��ɫ
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void RA8876_DrawRoundRect(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth, 
	uint16_t _usArc, uint16_t _usColor)
{
/*
	RA8876 ֧�ֻ���Բ�Ǿ��Σ�ʹ���߿���ʹ�������� MPU ���ڴ�ɻ���Բ�Ǿ��Ρ������趨Բ�Ǿ�����ʼ
�� REG[68h~6Ch] �������� REG[6Dh~6Fh]��Բ�Ǿ��γ�����뾶 REG[77h~7Ah]����ɫ REG[D2h~D4h]��
����趨����ͼ��ΪԲ�Ǿ��� REG[76h] Bit5~4 Ϊ 11b���������� REG[76h] Bit7 = 1����ô RA8876 ������
��ͼ�ϻ���Բ�Ǿ��Σ�����һ���ģ�ʹ���߿����趨�������� REG[76h] Bit6 = 1��
	
	---------------->---
	|(_usX��_usY)        |
	V                    V  _usHeight
	|                    |
	 ---------------->---
		  _usWidth
*/
	uint16_t temp;
		
	if (g_LcdDirection > 1)	/* ����  */
	{	
		temp = _usX;
		_usX = _usY;
		_usY = temp;

		temp = _usHeight;
		_usHeight = _usWidth;
		_usWidth = temp;		
	}
	
	if (g_LcdDirection == 0)			/* ���� */
	{
		Draw_Circle_Square(_usColor, _usX, _usY,  _usX + _usWidth - 1, _usY + _usHeight - 1, _usArc, _usArc);
	}
	else if (g_LcdDirection == 1)		/* ����,��ת180��*/
	{
		_usX = g_LcdWidth - _usX;
		_usY = _usY;
		
		Draw_Circle_Square(_usColor, _usX, _usY,  _usX - (_usWidth - 1), _usY + (_usHeight - 1), _usArc, _usArc);
	}
	else if (g_LcdDirection == 2)		/* ��������ת90�� */
	{	
		Draw_Circle_Square(_usColor, _usX, _usY,  _usX + (_usWidth - 1), _usY + (_usHeight - 1), _usArc, _usArc);
	}
	else if (g_LcdDirection == 3)		/* ��������ת270�� */
	{
		_usX = g_LcdHeight - (_usX + _usWidth - 1);
		_usY = _usY;
		
		Draw_Circle_Square(_usColor, _usX, _usY,  _usX + _usWidth - 1, _usY + _usHeight - 1, _usArc, _usArc);
	}
}

/*
*********************************************************************************************************
*	�� �� ��: RA8876_SetTextTransp
*	����˵��: �ı�ģʽ���������ֱ����Ƿ�ͨ͸
*	��    ��:_Enable : 0��ʾ��ͨ͸�� 1��ʾͨ͸
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void RA8876_SetTextTransp(uint8_t _Enable)
{
	/*
		pdf ��203ҳ		[CDH]
		bit7 ���ڶ��룬һ�㲻�ã�ȱʡ��0
		bit6 0 : �ַ������� 0 ����ʾΪָ������ɫ 1 : �ַ������� 0 ����ʾΪ��ͼ (Canvas)
		bit4 ������ת90����һ�㲻�ã�ȱʡ��0
		bit3-2 ˮƽ�Ŵ���
		bit1-0 ��ֱ�Ŵ���
	*/
	if (_Enable == 0)
	{
		Font_Background_select_Color();
	}
	else
	{
		Font_Background_select_Original_Canvas();
	}
}

/*
*********************************************************************************************************
*	�� �� ��: RA8876_SetTextZoom
*	����˵��: �ı�ģʽ���������ֵķŴ�ģʽ��1X,2X,3X, 4X
*	��    ��:
*			_ucHSize : ����ˮƽ�Ŵ�����RA_SIZE_X1��RA_SIZE_X2��RA_SIZE_X3��RA_SIZE_X4
*			_ucVSize : ���ִ��÷Ŵ�����RA_SIZE_X1��RA_SIZE_X2��RA_SIZE_X3��RA_SIZE_X4		
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void RA8876_SetTextZoom(uint8_t _ucHSize, uint8_t _ucVSize)
{
	/*
		pdf ��22ҳ		[22H]
		bit7 ���ڶ��룬һ�㲻�ã�ȱʡ��0
		bit6 ����ͨ͸��0 ��ʾ���ֱ�����ͨ͸�� 1��ʾͨ͸
		bit4 ������ת90����һ�㲻�ã�ȱʡ��0
		bit3-2 ˮƽ�Ŵ���
		bit1-0 ��ֱ�Ŵ���
	*/
	/* ˮƽ�ߴ� */
	if (_ucHSize == RA_SIZE_X1)
	{
		Font_Width_X1();
	}
	else if (_ucHSize == RA_SIZE_X2)
	{
		Font_Width_X2();
	}
	else if (_ucHSize == RA_SIZE_X3)
	{
		Font_Width_X3();
	}
	else if (_ucHSize == RA_SIZE_X4)
	{
		Font_Width_X4();
	}
	
	/* ��ֱ�ߴ� */
	if (_ucVSize == RA_SIZE_X1)
	{
		Font_Height_X1();
	}
	else if (_ucVSize == RA_SIZE_X2)
	{
		Font_Height_X2();
	}
	else if (_ucVSize == RA_SIZE_X3)
	{
		Font_Height_X3();
	}
	else if (_ucVSize == RA_SIZE_X4)
	{
		Font_Height_X4();
	}
}


/********************************************************************************************************
*																										*
*							����Ϊ�ٷ������ṩ��API����													*
*																										*
*********************************************************************************************************/

/*
*********************************************************************************************************
*	�� �� ��: RA8876_HW_Reset
*	����˵��: RA8876Ӳ����λ����
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void RA8876_HW_Reset(void)
{
/*
	RA8876_RST = 0;
	delay_ms(1);
	RA8876_RST = 1;
	Check_IC_ready();
*/
	GPIO_SetBits(GPIOI, GPIO_Pin_8);
	bsp_DelayMS(1);

	GPIO_ResetBits(GPIOI, GPIO_Pin_8);
	bsp_DelayMS(1); // XnRST have to keep low at least 256 OSC clocks. 

	GPIO_SetBits(GPIOI, GPIO_Pin_8 );	
	bsp_DelayMS(1);
	
	//=====
	System_Check_Temp();	//double check RA8876_HW_Reset(); 
}

/*
*********************************************************************************************************
*	�� �� ��: System_Check_Temp
*	����˵��: ���RA8876�Ƿ�OK
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void System_Check_Temp(void)
{
	uint8_t i=0;
	uint8_t temp=0;
	uint8_t system_ok=0;
	
/*
	״̬������ bit1
Operation mode status
	0: Normal ������
	1: Inhibit ������
	Inhibit ������ʾ RA8876 �ڲ����ڽ����ڲ���λ���ǿ�����ʾ��
	�ǽ�����ʡ��ģʽ���С�
	��ʡ��ģʽģʽ����λ��ά���� 1 ֱ�� PLL Ƶ�ʱ�ֹͣ���������
	bit �� REG([DFh]bit[7]) ����һ����ʱ��
*/	
	do
	{
		if((RA8876_StatusRead()&0x02)==0x00)
		{
			bsp_DelayMS(1); 					/* ����MCU�ӿ�̫�죬Ҳ����ҪһЩ��ʱ */
			RA8876_CmdWrite(0x01);
			bsp_DelayMS(1);  					/* ����MCU�ӿ�̫�죬Ҳ����ҪһЩ��ʱ */
			temp =RA8876_DataRead();
			if((temp & 0x80)==0x80)
			{
				system_ok=1;
				i=0;
			}
			else
			{
				bsp_DelayMS(1); 					/* ����MCU�ӿ�̫�죬Ҳ����ҪһЩ��ʱ */
				RA8876_CmdWrite(0x01);
				bsp_DelayMS(1); 					/* ����MCU�ӿ�̫�죬Ҳ����ҪһЩ��ʱ */
				RA8876_DataWrite(0x80);
			}
		}
		else
		{
			system_ok=0;
			i++;
		}
		if(system_ok==0 && i==5)
		{
			RA8876_HW_Reset(); //note1
			i=0;
		}
	}while(system_ok==0);
}
//==============================================================================
void LCD_RegisterWrite(uint8_t Cmd,uint8_t Data)
{
	Check_2D_Busy();
//	bsp_DelayUS(1);
	RA8876_CmdWrite(Cmd);
//	RA8875_Delaly1ms();
	RA8876_DataWrite(Data);
}  
//---------------------//
uint8_t LCD_RegisterRead(uint8_t Cmd)
{
	uint8_t temp;
	Check_2D_Busy();
	RA8876_CmdWrite(Cmd);
	temp=RA8876_DataRead();

	return temp;
}
//==============================================================================
void Check_Mem_WR_FIFO_not_Full(void)
{
/*[Status Register] bit7
Host Memory Write FIFO full
0: Memory Write FIFO is not full.
1: Memory Write FIFO is full.
Only when Memory Write FIFO is not full, MPU may write another
one pixel.
*/ 
	uint16_t i;

	/* you can using the following */
	// Please according to your MCU speed to modify. // �Ш̾ڱz��MCU�t�׭ק�ɶ�
	for(i=0;i<1000;i++)
	{
		if( (RA8876_StatusRead()&0x80)==0 ){break;}
	}
	/* or the following */
	do{	}
	while( RA8876_StatusRead()&0x80 );
}
//---------------------//
void Check_Mem_WR_FIFO_Empty(void)
{
/*[Status Register] bit6
Host Memory Write FIFO empty
0: Memory Write FIFO is not empty.
1: Memory Write FIFO is empty.
When Memory Write FIFO is empty, MPU may write 8bpp data 64
pixels, or 16bpp data 32 pixels, 24bpp data 16 pixels directly.
*/	
	uint16_t i;		 

	/* you can using the following */
	// Please according to your MCU speed to modify. // �Ш̾ڱz��MCU�t�׭ק�ɶ�
	for(i=0;i<1000;i++)
	{
		if( (RA8876_StatusRead()&0x40)==0x40 ){break;}
	}
	/* or the following */
	do{	}
	while( (RA8876_StatusRead()&0x40) == 0x00 );
}
//---------------------//
void Check_Mem_RD_FIFO_not_Full(void)
{
/*	[Status Register] bit5
Host Memory Read FIFO full
0: Memory Read FIFO is not full.
1: Memory Read FIFO is full.
When Memory Read FIFO is full, MPU may read 8bpp data 32
pixels, or 16bpp data 16 pixels, 24bpp data 8 pixels directly.
*/
	uint16_t i;

	/* you can using the following */
	// Please according to your MCU speed to modify. // �Ш̾ڱz��MCU�t�׭ק�ɶ�
	for(i=0;i<1000;i++)
	{
		if( (RA8876_StatusRead()&0x20)==0x00 ){break;}
	}
	/* or the following */
	do{	}
	while( RA8876_StatusRead()&0x20 );
}
//---------------------//
void Check_Mem_RD_FIFO_Full(void)
{
/*	[Status Register] bit5
Host Memory Read FIFO full
0: Memory Read FIFO is not full.
1: Memory Read FIFO is full.
When Memory Read FIFO is full, MPU may read 8bpp data 32
pixels, or 16bpp data 16 pixels, 24bpp data 8 pixels directly.
*/
	uint16_t i;

	/* you can using the following */
	// Please according to your MCU speed to modify. // �Ш̾ڱz��MCU�t�׭ק�ɶ�
	for(i=0;i<1000;i++)
	{
		if( (RA8876_StatusRead()&0x20)==0x20 ){break;}
	}
	/* or the following */
	do{	}
	while( (RA8876_StatusRead()&0x20)==0x00  );
}
//---------------------//
void Check_Mem_RD_FIFO_not_Empty(void)
{
/*	[Status Register] bit4
Host Memory Read FIFO empty
0: Memory Read FIFO is not empty.
1: Memory Read FIFO is empty.
*/
	uint16_t i;

	/* you can using the following */
	// Please according to your MCU speed to modify. // �Ш̾ڱz��MCU�t�׭ק�ɶ�
	for(i=0;i<1000;i++)
	{
		if( (RA8876_StatusRead()&0x10)==0x00 ){break;}
	}
	/* or the following */
	do{	}
	while( RA8876_StatusRead()&0x10 );
}

/*
*********************************************************************************************************
*	�� �� ��: Check_Mem_RD_FIFO_Empty
*	����˵��: ����ڴ��fifo�Ƿ�Ϊ��
*	��    �Σ���
*	�� �� ֵ: 1��Ϊ��   0����Ϊ��
*********************************************************************************************************
*/
uint8_t Check_Mem_RD_FIFO_Empty(void)
{
/*	[Status Register] bit4
Host Memory Read FIFO empty
0: Memory Read FIFO is not empty.
1: Memory Read FIFO is empty.
*/
	unsigned short i;

	/* you can using the following */
	// Please according to your MCU speed to modify. // �Ш̾ڱz��MCU�t�׭ק�ɶ�
	for(i=0;i<1000;i++)
	{
		if( (RA8876_StatusRead()&0x10)==0x10 ){return 1;}
	}
	
	return 0;
}

//---------------------//
void Check_2D_Busy(void)
{
/*	[Status Register] bit3
Core task is busy
Following task is running:
BTE, Geometry engine, Serial flash DMA, Text write or Graphic write
0: task is done or idle.
1: task is busy.													
*/
	uint16_t i;
	
	/* you can using the following */
	// Please according to your MCU speed to modify. // �Ш̾ڱz��MCU�t�׭ק�ɶ�
		for(i=0;i<10000;i++)
		{
			if( (RA8876_StatusRead()&0x08)==0x00 ){break;}
		}

	/* or the following */
		do{	}
		while(( RA8876_StatusRead()&0x08)==0x08 );

}  

/*
*********************************************************************************************************
*	�� �� ��: Check_SDRAM_Ready
*	����˵��: ���SDRAM�Ƿ�׼������
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/ 
void Check_SDRAM_Ready(void)
{
/*	[Status Register] bit2
SDRAM ready for access
0: SDRAM is not ready for access
1: SDRAM is ready for access		
*/	
	uint16_t i;
	/* you can using the following */
	// Please according to your MCU speed to modify. // �Ш̾ڱz��MCU�t�׭ק�ɶ�
	for(i=0;i<10000;i++)
	{
		if( (RA8876_StatusRead()&0x04)==0x04 ){break;}
	}
	/* or the following */
	do{	}
	while( (RA8876_StatusRead()&0x04) == 0x00 );
}

/*
*********************************************************************************************************
*	�� �� ��: Check_IC_ready
*	����˵��: ���оƬ�Ƿ�׼����
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/ 
void Check_IC_ready(void)
{
/*	״̬������ bit1
Operation mode status
0: Normal ������
1: Inhibit ������
Inhibit ������ʾ RA8876 �ڲ����ڽ����ڲ���λ���ǿ�����ʾ��
�ǽ�����ʡ��ģʽ���С�
��ʡ��ģʽģʽ����λ��ά���� 1 ֱ�� PLL Ƶ�ʱ�ֹͣ���������
bit �� REG([DFh]bit[7]) ����һ����ʱ��	
*/	
	uint16_t i;
	/* you can using the following */
	// Please according to your MCU speed to modify. // �Ш̾ڱz��MCU�t�׭ק�ɶ�
	for(i=0;i<10000;i++)
	{
		if( (RA8876_StatusRead()&0x02)==0x00 ){break;}
	}
	/* or the following */
	do{	}
	while(RA8876_StatusRead() & 0x02);
}

//---------------------//
uint8_t Power_Saving_Status(void)
{
/*	[Status Register] bit1
Operation mode status
0: Normal operation state
1: Inhibit operation state
Inhibit operation state means internal reset event keep running or
initial display still running or chip enter power saving state.		
*/
  	  uint8_t temp;

	  if((RA8876_StatusRead()&0x02)==0x02)
	    temp = 1;
	  else
	    temp = 0;

	  return temp;
}

void Check_Power_is_Saving(void)
{  
/*	[Status Register] bit1
Operation mode status
0: Normal operation state
1: Inhibit operation state
Inhibit operation state means internal reset event keep running or
initial display still running or chip enter power saving state.		
*/
	uint16_t i;
	/* you can using the following */
	// Please according to your MCU speed to modify. // �Ш̾ڱz��MCU�t�׭ק�ɶ�
	for(i=0;i<1000;i++)
	{
		if( (RA8876_StatusRead()&0x02)==0x02 ){break;}
	}
	/* or the following */
	do{	}
	while( (RA8876_StatusRead()&0x02) == 0x00 );
}
//---------------------//
void Check_NO_Interrupt(void)
{
/*	[Status Register] bit0
Interrupt pin state
0: without interrupt active
1: interrupt active		
*/	
	uint16_t i;
	/* you can using the following */
	// Please according to your MCU speed to modify. // �Ш̾ڱz��MCU�t�׭ק�ɶ�
	for(i=0;i<1000;i++)
	{
		if( (RA8876_StatusRead()&0x01)==0x00 ){break;}
	}
	/* or the following */
	do{	}
	while( RA8876_StatusRead()&0x01 );
}
void Check_Interrupt_Occur(void)
{
/*	[Status Register] bit0
Interrupt pin state
0: without interrupt active
1: interrupt active		
*/	
	uint16_t i;
	/* you can using the following */
	// Please according to your MCU speed to modify. // �Ш̾ڱz��MCU�t�׭ק�ɶ�
	for(i=0;i<1000;i++)
	{
		if( (RA8876_StatusRead()&0x01)==0x01 ){break;}
	}
	/* or the following */
	do{	}
	while( (RA8876_StatusRead()&0x01) == 0x00 );
}

//==============================================================================



/*
*********************************************************************************************************
*	�� �� ��: RA8876_SW_Reset
*	����˵��: RA8876�����λ
*	��    �Σ���
*	�� �� ֵ: �������(���账��)
*********************************************************************************************************
*/
void RA8876_SW_Reset(void)
{/*
Software Reset
0: Normal operation.
1: Software Reset.
Software Reset only reset internal state machine. Configuration
Registers value won��t be reset. So all read-only flag in the
register will return to its initial value. User should have proper
action to make sure flag is in desired state.
Note: The bit will auto clear after reset.
*/	
	uint16_t i;
	uint8_t temp;

	RA8876_CmdWrite(0x00);
	temp = RA8876_DataRead();
	temp |= 0x01;
	RA8876_DataWrite(temp);
	bsp_DelayUS(100);			// it must wait 100us after Software_Reset.

	// Please according to your MCU speed to modify. // �Ш̾ڱz��MCU�t�׭ק�ɶ�
	for(i=0;i<100;i++)
	{
		if( (LCD_RegisterRead(0x01)&0x80)==0x80 )
		{break;}
	}

}
/*
*********************************************************************************************************
*	�� �� ��: Enable_PLL
*	����˵��: RA8876ʱ��ʹ��
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void Enable_PLL(void)
{
/*  0: PLL disable; allow change PLL parameter.
    1: PLL enable; cannot change PLL parameter.*/
/*
	оƬ���üĴ���[01H]
	
bit7��
	Reconfigure PLL frequency
	����� bit д��1�����������趨 PLL Ƶ�ʡ�
	ע
	a. ��ʹ���߸��� PLL ��ز�����PLL Ƶ�ʲ������ϸı䣬ʹ��
		�߻������ٴν���� bit �趨Ϊ 1��PLL Ƶ�ʲŻ�ı䡣
	b. ʹ���߿��Զ�ȡ(���)��� bit ��֪��ϵͳ�Ƿ��Ѿ��л���
		PLL Ƶ�ʣ���1����ʾ PLL Ƶ���Ѿ����������л��ɹ�	
	
*/
	static uint8_t temp;
	uint16_t i;

	RA8876_CmdWrite(0x01);
	temp = RA8876_DataRead();
	temp |= cSetb7;
	RA8876_DataWrite(temp);

	bsp_DelayUS(100);// PLL lock time = 1024 T OSC clocks, if OSC=10MHz, PLL lock time = 10 us.  

	/*check PLL was ready ( Please according to your MCU speed to modify. �Ш̾ڱz��MCU�t�׭ק�ɶ�)	 */
	for(i=0;i<10000;i++)
	{
		RA8876_CmdWrite(0x01);
		temp=RA8876_DataRead();
		if( (temp&0x80)==0x80 ){break;}
	}
}

void Sent_XnWAIT_Status_When_CS_Low(void)
{/*  
	Mask XnWAIT on XnCS deassert
	0 : No mask
		XnWAIT keep assert if internal state keep busy and cannot 
		accept next R/W cycle, no matter XnCS assert/deassert. If
		MCU cycle cannot be extended while XnWAIT keep low, user
		should poll XnWAIT and wait it goes high then start next
		access.
	1 : Mask
		XnWAIT deassert when XnCS deassert. Use in MCU cycle can
		be extended by XnWAIT automatically.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x01);
	temp = RA8876_DataRead();
	temp |= cSetb6;
	RA8876_DataWrite(temp);    
}
void Sent_XnWAIT_Status_At_Any_Time(void)
{/*  
	Mask XnWAIT on XnCS deassert
	0 : No mask
		XnWAIT keep assert if internal state keep busy and cannot 
		accept next R/W cycle, no matter XnCS assert/deassert. If
		MCU cycle cannot be extended while XnWAIT keep low, user
		should poll XnWAIT and wait it goes high then start next
		access.
	1 : Mask
		XnWAIT deassert when XnCS deassert. Use in MCU cycle can
		be extended by XnWAIT automatically.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x01);
	temp = RA8876_DataRead();
	temp &= cClrb6;
	RA8876_DataWrite(temp);    
}
void Key_Scan_Enable(void)
{
/*  0: Disable.
    1: Enable.*/
	uint8_t temp;
	RA8876_CmdWrite(0x01);
	temp = RA8876_DataRead();
	temp |= cSetb5;
	RA8876_DataWrite(temp);    
}
void Key_Scan_Disable(void)
{
/*  0: Disable.
    1: Enable.*/
	uint8_t temp;
	RA8876_CmdWrite(0x01);
	temp = RA8876_DataRead();
	temp &= cClrb5;
	RA8876_DataWrite(temp);    
}

/*
*********************************************************************************************************
*	�� �� ��: TFT_24bit
*	����˵��: 24bits TFT���
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void TFT_24bit(void)//RA8876 only
{
/*  TFT Panel I/F Output pin Setting
00b: 24-bits TFT output.
01b: 18-bits TFT output.
10b: 16-bits TFT output.
11b: w/o TFT output.
Other unused TFT output pins are set as GPIO or Key function.*/
	uint8_t temp;
	RA8876_CmdWrite(0x01);
	temp = RA8876_DataRead();
	temp &= cClrb4;
    temp &= cClrb3;
	RA8876_DataWrite(temp);    
}

/*
*********************************************************************************************************
*	�� �� ��: TFT_18bit
*	����˵��: 18bits TFT���
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void TFT_18bit(void)//RA8876 only
{
/*  TFT Panel I/F Output pin Setting
00b: 24-bits TFT output.
01b: 18-bits TFT output.
10b: 16-bits TFT output.
11b: w/o TFT output.
Other unused TFT output pins are set as GPIO or Key function.*/
	uint8_t temp;
	RA8876_CmdWrite(0x01);
	temp = RA8876_DataRead();
	temp &= cClrb4;
    temp |= cSetb3;
	RA8876_DataWrite(temp);  
}

/*
*********************************************************************************************************
*	�� �� ��: TFT_16bit
*	����˵��: 16bits TFT���
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void TFT_16bit(void)//RA8876 only
{
/*  
	TFT Panel I/F Output pin Setting
00b: 24-bits TFT output.
01b: 18-bits TFT output.
10b: 16-bits TFT output.
11b: w/o TFT output.
Other unused TFT output pins are set as GPIO or Key function.*/
/*
	оƬ���üĴ���[]01H
bit4-3
	For RA8876 TFT Panel I/F Output pin Setting
	00b: 24-bits TFT output��
	01b: 18-bits TFT output��
	10b: 16-bits TFT output��
	11b: w/o TFT output��
	����δʹ�õ� TFT ������ű��趨Ϊ GPIO �밴������ Key��	
*/	

	uint8_t temp;
	RA8876_CmdWrite(0x01);
	temp = RA8876_DataRead();
	temp |= cSetb4;
    temp &= cClrb3;
	RA8876_DataWrite(temp);  
}

void Without_TFT(void)//RA8876 only
{
/*  TFT Panel I/F Output pin Setting
00b: 24-bits TFT output.
01b: 18-bits TFT output.
10b: 16-bits TFT output.
11b: w/o TFT output.
Other unused TFT output pins are set as GPIO or Key function.*/
	uint8_t temp;
	RA8876_CmdWrite(0x01);
	temp = RA8876_DataRead();
	temp |= cSetb4;
    temp |= cSetb3;
	RA8876_DataWrite(temp);  
}

void RA8876_I2CM_Enable(void)
{
/*  I2C master Interface Enable/Disable
		0: Disable (GPIO function)
		1: Enable (I2C master function)*/
	uint8_t temp;
	RA8876_CmdWrite(0x01);
	temp = RA8876_DataRead();
	temp |= cSetb2;
	RA8876_DataWrite(temp);    
}

void RA8876_I2CM_Disable(void)
{
/*  I2C master Interface Enable/Disable
		0: Disable (GPIO function)
		1: Enable (I2C master function)*/
	uint8_t temp;
	RA8876_CmdWrite(0x01);
	temp = RA8876_DataRead();
	temp &= cClrb2;
	RA8876_DataWrite(temp);     
}

/*
*********************************************************************************************************
*	�� �� ��: Enable_SFlash_SPI
*	����˵��: ����SPI flash���п���
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void Enable_SFlash_SPI(void)
{
/*	Serial Flash or SPI Interface Enable/Disable
		0: Disable (GPIO function)
		1: Enable (SPI master function)
		When SDR SDRAM 32bits bus function enable, this bit is ignored
		& Serial flash pins become SDR SDRAM bus function.*/
/*
	оƬ���ƼĴ���[01h]
bit1:
	Serial Flash or SPI Interface Enable/Disable
		0: ���� (GPIO function)��
		1: ���� (SPI master function)��
	�� SDR SDRAM 32bits ���߹��ܱ�����ʱ����� bit �ᱻ���ԣ���
	�� Serial flash pins ������ SDR SDRAM bus �Ĺ��ܡ�	
*/	
	uint8_t temp;
	RA8876_CmdWrite(0x01);
	temp = RA8876_DataRead();
	temp |= cSetb1;
	RA8876_DataWrite(temp);     
}

/*
*********************************************************************************************************
*	�� �� ��: Disable_SFlash_SPI
*	����˵��: ����SPI flash���п���
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void Disable_SFlash_SPI(void)
{
/*	Serial Flash or SPI Interface Enable/Disable
		0: Disable (GPIO function)
		1: Enable (SPI master function)
		When SDR SDRAM 32bits bus function enable, this bit is ignored
		& Serial flash pins become SDR SDRAM bus function.*/
/*
	оƬ���ƼĴ���[01h]
bit1:
	Serial Flash or SPI Interface Enable/Disable
		0: ���� (GPIO function)��
		1: ���� (SPI master function)��
	�� SDR SDRAM 32bits ���߹��ܱ�����ʱ����� bit �ᱻ���ԣ���
	�� Serial flash pins ������ SDR SDRAM bus �Ĺ��ܡ�	
*/	
	uint8_t temp;
	RA8876_CmdWrite(0x01);
	temp = RA8876_DataRead();
	temp &= cClrb1;
	RA8876_DataWrite(temp);     
}

/*
*********************************************************************************************************
*	�� �� ��: Host_Bus_8bit
*	����˵��: 8λ��������
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void Host_Bus_8bit(void)
{
/*  Parallel Host Data Bus Width Selection
    0: 8-bit Parallel Host Data Bus.
    1: 16-bit Parallel Host Data Bus.*/
	uint8_t temp;
	RA8876_CmdWrite(0x01);
	temp = RA8876_DataRead();
	temp &= cClrb0;
	RA8876_DataWrite(temp);
}

/*
*********************************************************************************************************
*	�� �� ��: Host_Bus_16bit
*	����˵��: 16λ��������
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void Host_Bus_16bit(void)
{
/*  Parallel Host Data Bus Width Selection
    0: 8-bit Parallel Host Data Bus.
    1: 16-bit Parallel Host Data Bus.*/
	
/*
	оƬ���üĴ���[01H]
bit0��
	Host Data Bus Width Selection
		0: 8-bit ���ض��������ߡ�
		1: 16-bit ���ض��������š�
	*** ��� Serial host I/F ��ѡ������ڿ�����ʾ�Ĳ������ڣ�
	RA8876 ���Ὣ��� bit ��Ϊ 0������ֻ���� 8-bit ��ȵĴ�ȡ��
	�� SDR SDRAM 32bit ���趨����Ȩ�Ƚϣ���� bit ������Ȩ�ƽ�
	�͵ġ�	
*/
	uint8_t temp;
	RA8876_CmdWrite(0x01);
	temp = RA8876_DataRead();
	temp |= cSetb0;
	RA8876_DataWrite(temp);
}

//[02h][02h][02h][02h][02h][02h][02h][02h][02h][02h][02h][02h][02h][02h][02h][02h]

void Data_Format_8b_8bpp(void)
{
/* MPU read/write data format when access memory data port.
0xb: Direct write for 
	all 8 bits MPU I/F,
	16 bits MPU I/F with 8bpp data mode 2, 
	16 bits MPU I/F with 16bpp, 
	16 bits MPU I/F with 24bpp data mode 1, 
	and serial host interface.				 				*/
	uint8_t temp;
	RA8876_CmdWrite(0x02);
	temp = RA8876_DataRead();
	temp &= cClrb7;
	RA8876_DataWrite(temp);
}
void Data_Format_8b_16bpp(void)
{  
/* MPU read/write data format when access memory data port.
0xb: Direct write for 
	all 8 bits MPU I/F,
	16 bits MPU I/F with 8bpp data mode 2, 
	16 bits MPU I/F with 16bpp, 
	16 bits MPU I/F with 24bpp data mode 1, 
	and serial host interface.				 				*/
	uint8_t temp;
	RA8876_CmdWrite(0x02);
	temp = RA8876_DataRead();
	temp &= cClrb7;
	RA8876_DataWrite(temp);
}
void Data_Format_8b_24bpp(void)
{
/* MPU read/write data format when access memory data port.
0xb: Direct write for 
	all 8 bits MPU I/F,
	16 bits MPU I/F with 8bpp data mode 2, 
	16 bits MPU I/F with 16bpp, 
	16 bits MPU I/F with 24bpp data mode 1, 
	and serial host interface.				 				*/
	uint8_t temp;
	RA8876_CmdWrite(0x02);
	temp = RA8876_DataRead();
	temp &= cClrb7;
	RA8876_DataWrite(temp);
}

void Data_Format_16b_8bpp(void)
{
/* MPU read/write data format when access memory data port.
10b: Mask high byte of each data (ex. 16 bit MPU I/F with 8-bpp data mode 1)	*/
	uint8_t temp;
	RA8876_CmdWrite(0x02);
	temp = RA8876_DataRead();
	temp |= cSetb7;
	temp &= cClrb6;
	RA8876_DataWrite(temp);
}

/*
*********************************************************************************************************
*	�� �� ��: Data_Format_16b_16bpp
*	����˵��: MPU����ڴ�Ķ�д���ݸ�ʽ
*	��    �Σ���
*	�� �� ֵ: �������(���账��)
*********************************************************************************************************
*/
void Data_Format_16b_16bpp(void)
{
/* MPU read/write data format when access memory data port.
0xb: Direct write for 
	all 8 bits MPU I/F,
	16 bits MPU I/F with 8bpp data mode 2, 
	16 bits MPU I/F with 16bpp, 
	16 bits MPU I/F with 24bpp data mode 1, 
	and serial host interface.				 				*/
/*
	�洢�����ʿ��ƼĴ���[02H]
bit7-6��
	Host Read/Write image Data Format
	MPU ����ڴ�Ķ�д���ݸ�ʽ��
	0xb:ֱ��д�룬����ʹ�ø�ʽ����:
		1. 8 bits MPU I/F
		2. 16 bits MPU I/F with 8bpp data mode 1 & 2
		3. 16 bits MPU I/F with 16/24-bpp data mode 1
		4. serial host interface
	10b: ��ÿ�����ݽ����� high byte(�� 16 bit MPU I/F ʹ�õ���
	8-bpp data mode 1 ���ݸ�ʽ)��
	11b: ��ż���������� high byte(�� 16 bit MPU I/F ʹ�� 24-bpp
	data mode 2)��	
*/
	uint8_t temp;
	RA8876_CmdWrite(0x02);
	temp = RA8876_DataRead();
	temp &= cClrb7;
//	temp |= cSetb6;
	RA8876_DataWrite(temp);
}
/*
*********************************************************************************************************
*	�� �� ��: Data_Format_16b_16bpp
*	����˵��: MPU����ڴ�Ķ�д���ݸ�ʽ
*	��    �Σ���
*	�� �� ֵ: �������(���账��)
*********************************************************************************************************
*/
void Data_Format_16b_24bpp_mode1(void)
{
/* MPU read/write data format when access memory data port.
0xb: Direct write for 
	all 8 bits MPU I/F,
	16 bits MPU I/F with 8bpp data mode 2, 
	16 bits MPU I/F with 16bpp, 
	16 bits MPU I/F with 24bpp data mode 1, 
	and serial host interface.				 				*/
	uint8_t temp;
	RA8876_CmdWrite(0x02);
	temp = RA8876_DataRead();
	temp &= cClrb7;
//	temp &= cClrb6;
	RA8876_DataWrite(temp);
}
/*
*********************************************************************************************************
*	�� �� ��: Data_Format_16b_16bpp
*	����˵��: MPU����ڴ�Ķ�д���ݸ�ʽ
*	��    �Σ���
*	�� �� ֵ: �������(���账��)
*********************************************************************************************************
*/
void Data_Format_16b_24bpp_mode2(void)
{
/* MPU read/write data format when access memory data port.
11b: Mask high byte of even data (ex. 16 bit MPU I/F with 24-bpp data mode 2)	*/
	uint8_t temp;
	RA8876_CmdWrite(0x02);
	temp = RA8876_DataRead();
	temp |= cSetb7;
	temp |= cSetb6;
	RA8876_DataWrite(temp);
}

void MemRead_Left_Right_Top_Down(void)
{
/* Host Read Memory Direction (Only for Graphic Mode)
00b: Left .. Right then Top .. Bottom.
Ignored if canvas in linear addressing mode.		*/
	uint8_t temp;
	RA8876_CmdWrite(0x02);
	temp = RA8876_DataRead();
	temp &= cClrb5;
	temp &= cClrb4;
	RA8876_DataWrite(temp);
}
void MemRead_Right_Left_Top_Down(void)
{
/* Host Read Memory Direction (Only for Graphic Mode)
01b: Right .. Left then Top ..Bottom.
Ignored if canvas in linear addressing mode.		*/
	uint8_t temp;
	RA8876_CmdWrite(0x02);
	temp = RA8876_DataRead();
	temp &= cClrb5;
	temp |= cSetb4;
	RA8876_DataWrite(temp);
}
void MemRead_Top_Down_Left_Right(void)
{
/* Host Read Memory Direction (Only for Graphic Mode)
10b: Top .. Bottom then Left .. Right.
Ignored if canvas in linear addressing mode.		*/
	uint8_t temp;
	RA8876_CmdWrite(0x02);
	temp = RA8876_DataRead();
	temp |= cSetb5;
    temp &= cClrb4;
	RA8876_DataWrite(temp);
}
void MemRead_Down_Top_Left_Right(void)
{
/* Host Read Memory Direction (Only for Graphic Mode)
11b: Bottom .. Top then Left .. Right.
Ignored if canvas in linear addressing mode.		*/
	uint8_t temp;
	RA8876_CmdWrite(0x02);
	temp = RA8876_DataRead();
	temp |= cSetb5;
	temp |= cSetb4;
	RA8876_DataWrite(temp);
}

/*
*********************************************************************************************************
*	�� �� ��: MemWrite_Left_Right_Top_Down
*	����˵��: д�Ĵ�������Only for Graphic Mode�� 
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
/* �ر�˵�����м����ı��ڴ�д�뷽��ĺ�����ֻ��Ч��ֱ��д�ڴ档��2D��ͼ��Ч�� 
	���磺MPU16_16bpp_Memory_Write() ����ʵ��ͼƬ��ת��
		  Draw_Square_Fill()		������
*/
void MemWrite_Left_Right_Top_Down(void)
{
/* Host Write Memory Direction (Only for Graphic Mode)
00b: Left .. Right then Top ..Bottom.
Ignored if canvas in linear addressing mode.		*/
/*
	�洢�����ʼĴ���[02H]
	Host Write Memory Direction (Only for Graphic Mode)
	00b: ��->�� Ȼ�� ��->��. (Original)��
	01b: ��->�� Ȼ�� ��->��. (Horizontal flip)��
	10b: ��->�� Ȼ�� ��->��. (Rotate right 90��& Horizontal flip)��
	11b: ��->�� Ȼ�� ��->��. (Rotate left 90��)��
	�����ͼ�趨������Ѱַģʽ������� bit �ɺ��ԡ�
*/
	uint8_t temp;
	RA8876_CmdWrite(0x02);
	temp = RA8876_DataRead();
	temp &= cClrb2;
	temp &= cClrb1;
	RA8876_DataWrite(temp);
}
void MemWrite_Right_Left_Top_Down(void)
{
/* Host Write Memory Direction (Only for Graphic Mode)
01b: Right .. Left then Top .. Bottom.
Ignored if canvas in linear addressing mode.		*/
	uint8_t temp;
	RA8876_CmdWrite(0x02);
	temp = RA8876_DataRead();
	temp &= cClrb2;
	temp |= cSetb1;
	RA8876_DataWrite(temp);
}
void MemWrite_Top_Down_Left_Right(void)
{
/* Host Write Memory Direction (Only for Graphic Mode)
10b: Top .. Bottom then Left .. Right.
Ignored if canvas in linear addressing mode.		*/
	uint8_t temp;
	RA8876_CmdWrite(0x02);
	temp = RA8876_DataRead();
	temp |= cSetb2;
    temp &= cClrb1;
	RA8876_DataWrite(temp);
}
void MemWrite_Down_Top_Left_Right(void)
{
/* Host Write Memory Direction (Only for Graphic Mode)
11b: Bottom .. Top then Left .. Right.
Ignored if canvas in linear addressing mode.		*/
	uint8_t temp;
	RA8876_CmdWrite(0x02);
	temp = RA8876_DataRead();
	temp |= cSetb2;
	temp |= cSetb1;
	RA8876_DataWrite(temp);
}
//[03h][03h][03h][03h][03h][03h][03h][03h][03h][03h][03h][03h][03h][03h][03h][03h]
void Interrupt_Active_Low(void)
{
/*  Output to MPU Interrupt active level
	0 : active low.
	1 : active high.						*/
	uint8_t temp;
	RA8876_CmdWrite(0x03);
	temp = RA8876_DataRead();
	temp &= cClrb7;
	RA8876_DataWrite(temp);
}
void Interrupt_Active_High(void)
{
/*  Output to MPU Interrupt active level
	0 : active low.
	1 : active high.						*/
	uint8_t temp;
	RA8876_CmdWrite(0x03);
	temp = RA8876_DataRead();
	temp |= cSetb7;
	RA8876_DataWrite(temp);
}
void ExtInterrupt_Debounce(void)
{
/*  External interrupt input (XPS[0] pin) de-bounce
    0 : without de-bounce
    1 : enable de-bounce (1024 OSC clock)			*/
	uint8_t temp;
	RA8876_CmdWrite(0x03);
	temp = RA8876_DataRead();
	temp |= cSetb6;
	RA8876_DataWrite(temp);
}
void ExtInterrupt_Nodebounce(void)
{
/*  External interrupt input (XPS[0] pin) de-bounce
    0 : without de-bounce
    1 : enable de-bounce (1024 OSC clock)			*/
	uint8_t temp;
	RA8876_CmdWrite(0x03);
	temp = RA8876_DataRead();
	temp &= cClrb6;
	RA8876_DataWrite(temp);
}
void ExtInterrupt_Input_Low_Level_Trigger(void)
{
/* External interrupt input (XPS[0] pin) trigger type
00 : low level trigger
*/
	uint8_t temp;
	RA8876_CmdWrite(0x03);
	temp = RA8876_DataRead();
	temp &= cClrb5;
    temp &= cClrb4;
	RA8876_DataWrite(temp);
}
void ExtInterrupt_Input_High_Level_Trigger(void)
{
/* External interrupt input (XPS[0] pin) trigger type
01 : falling edge trigger
*/
	uint8_t temp;
	RA8876_CmdWrite(0x03);

	temp = RA8876_DataRead();
	temp |= cSetb5;
    temp &= cClrb4;
	RA8876_DataWrite(temp);
}
void ExtInterrupt_Input_Falling_Edge_Trigger(void)
{
/* External interrupt input (XPS[0] pin) trigger type
10 : high level trigger
*/
	uint8_t temp;
	RA8876_CmdWrite(0x03);
	temp = RA8876_DataRead();
    temp &= cClrb5;
    temp |= cSetb4;
	RA8876_DataWrite(temp);
}
void ExtInterrupt_Input_Rising_Edge_Trigger(void)
{
/* External interrupt input (XPS[0] pin) trigger type
11 : rising edge trigger
*/
	uint8_t temp;
	RA8876_CmdWrite(0x03);
	temp = RA8876_DataRead();
	temp |= cSetb5;
    temp |= cSetb4;
	RA8876_DataWrite(temp);
}
void LVDS_Format1(void)//RA8877 only
{
/* FPD-Link Data Format / LVDS Data Format
0 : Format 1 (VESA format) 
1 : Format 2 (JEIDA format) 				*/
	uint8_t temp;
	RA8876_CmdWrite(0x03);
	temp = RA8876_DataRead();
    temp &= cClrb3;
	RA8876_DataWrite(temp);
}
void LVDS_Format2(void)//RA8877 only
{
/* FPD-Link Data Format / LVDS Data Format
0 : Format 1 (VESA format) 
1 : Format 2 (JEIDA format) 				*/
	uint8_t temp;
	RA8876_CmdWrite(0x03);
	temp = RA8876_DataRead();
    temp |= cSetb3;
	RA8876_DataWrite(temp);
}

/*
*********************************************************************************************************
*	�� �� ��: Graphic_Mode
*	����˵��: ����ͼ��ģʽ
*	��    �Σ���
*	�� �� ֵ: �������(���账��)
*********************************************************************************************************
*/
void Graphic_Mode(void)
{
/*
	������ƼĴ���[03H]
bit2��
	
	Text Mode Enable
		0 : ͼ��ģʽ��
		1 : ����ģʽ��
	���趨��� bit ֮ǰ��������ȷ�� core task busy �Ƿ�����æµ��
	�����У��� core task busy ��״̬��������
	����� linear Ѱַģʽ�У���� bit ʼ��Ϊ 0��
		
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x03);
	temp = RA8876_DataRead();
    temp &= cClrb2;
	RA8876_DataWrite(temp);
}


/*
*********************************************************************************************************
*	�� �� ��: Text_Mode
*	����˵��: �����ı�ģʽ
*	��    �Σ���
*	�� �� ֵ: �������(���账��)
*********************************************************************************************************
*/
void Text_Mode(void)
{
/*
	������ƼĴ���[03H]
bit2��
	
	Text Mode Enable
		0 : ͼ��ģʽ��
		1 : ����ģʽ��
	���趨��� bit ֮ǰ��������ȷ�� core task busy �Ƿ�����æµ��
	�����У��� core task busy ��״̬��������
	����� linear Ѱַģʽ�У���� bit ʼ��Ϊ 0��
		
*/
	uint8_t temp;
	RA8876_CmdWrite(0x03);
	temp = RA8876_DataRead();
    temp |= cSetb2;
	RA8876_DataWrite(temp);
}

/*
*********************************************************************************************************
*	�� �� ��: Memory_Select_SDRAM
*	����˵��: �ڴ�˿ڶ�дĿ��ѡ��
*	��    �Σ���
*	�� �� ֵ: �������(���账��)
*********************************************************************************************************
*/
void Memory_Select_SDRAM(void)
{
/*
	������ƼĴ���[03H]
bit1-0��
	Memory port Read/Write Destination Selection
		00b: ѡ�� SDRAM Ϊ image/pattern/ʹ�����Զ����͵�����д��
		Ŀ�ģ�֧�� Read-modify-Write��
		01b: ѡ�� RGB ɫ�� Gamma table Ϊд��Ŀ�ġ� ÿ����ɫ�Ķ�
		�� 256 bytes��ʹ������Ҫָ����Ҫд��� gamma table Ȼ������
		��д�� 256 bytes��
	
*/
	uint8_t temp;

	RA8876_CmdWrite(0x03);
	temp = RA8876_DataRead();
    temp &= cClrb1;
    temp &= cClrb0;
	RA8876_DataWrite(temp);

}
void Memory_Select_Gamma_Table(void)
{
/*
	������ƼĴ���[03H]
bit1-0��
	Memory port Read/Write Destination Selection
		00b: ѡ�� SDRAM Ϊ image/pattern/ʹ�����Զ����͵�����д��
		Ŀ�ģ�֧�� Read-modify-Write��
		01b: ѡ�� RGB ɫ�� Gamma table Ϊд��Ŀ�ġ� ÿ����ɫ�Ķ�
		�� 256 bytes��ʹ������Ҫָ����Ҫд��� gamma table Ȼ������
		��д�� 256 bytes��
	
*/
	uint8_t temp;
	RA8876_CmdWrite(0x03);
	temp = RA8876_DataRead();
    temp &= cClrb1;
    temp |= cSetb0;
	RA8876_DataWrite(temp);  

}
void Memory_Select_Graphic_Cursor_RAM(void)
{
	uint8_t temp;
	RA8876_CmdWrite(0x03);
	temp = RA8876_DataRead();
    temp |= cSetb1;
    temp &= cClrb0;
	RA8876_DataWrite(temp);
}
void Memory_Select_Color_Palette_RAM(void)
{
	uint8_t temp;
	RA8876_CmdWrite(0x03);
	temp = RA8876_DataRead();
    temp |= cSetb1;
    temp |= cSetb0;
	RA8876_DataWrite(temp);
}

//[05h]=========================================================================
//[06h]=========================================================================
//[07h]=========================================================================
//[08h]=========================================================================
//[09h]=========================================================================
//[0Ah]=========================================================================
//[0Bh]=========================================================================

void Enable_Resume_Interrupt(void)
{
/*
Wakeup/resume Interrupt Enable
0: Disable.
1: Enable.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0B);
	temp = RA8876_DataRead();
    temp |= cSetb7;
	RA8876_DataWrite(temp);
}
void Disable_Resume_Interrupt(void)
{
/*
Wakeup/resume Interrupt Enable
0: Disable.
1: Enable.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0B);
	temp = RA8876_DataRead();
    temp &= cClrb7;
	RA8876_DataWrite(temp);
}
void Enable_ExtInterrupt_Input(void)
{
/*
External Interrupt (PS[0] pin) Enable
0: Disable.
1: Enable.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0B);
	temp = RA8876_DataRead();
    temp |= cSetb6;
	RA8876_DataWrite(temp);
}
void Disable_ExtInterrupt_Input(void)
{
/*
External Interrupt (PS[0] pin) Enable
0: Disable.
1: Enable.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0B);
	temp = RA8876_DataRead();
    temp &= cClrb6;
	RA8876_DataWrite(temp);
}
void Enable_I2CM_Interrupt(void)
{
/*
I2C Master Interrupt Enable
0: Disable.
1: Enable.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0B);
	temp = RA8876_DataRead();
    temp |= cSetb5;
	RA8876_DataWrite(temp);
}
void Disable_I2CM_Interrupt(void)
{
/*
I2C Master Interrupt Enable
0: Disable.
1: Enable.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0B);
	temp = RA8876_DataRead();
    temp &= cClrb5;
	RA8876_DataWrite(temp);
}
void Enable_Vsync_Interrupt(void)
{
/*
Vsync time base interrupt Enable Bit
0: Disable Interrupt.
1: Enable Interrupt.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0B);
	temp = RA8876_DataRead();
    temp |= cSetb4;
	RA8876_DataWrite(temp);
}
void Disable_Vsync_Interrupt(void)
{
/*
Vsync time base interrupt Enable Bit
0: Disable Interrupt.
1: Enable Interrupt.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0B);
	temp = RA8876_DataRead();
    temp &= cClrb4;
	RA8876_DataWrite(temp);
}
void Enable_KeyScan_Interrupt(void)
{
/*
Key Scan Interrupt Enable Bit
0: Disable Key scan interrupt.
1: Enable Key scan interrupt.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0B);
	temp = RA8876_DataRead();
    temp |= cSetb3;
	RA8876_DataWrite(temp);
}
void Disable_KeyScan_Interrupt(void)
{
/*
Key Scan Interrupt Enable Bit
0: Disable Key scan interrupt.
1: Enable Key scan interrupt.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0B);
	temp = RA8876_DataRead();
    temp &= cClrb3;
	RA8876_DataWrite(temp);
}
void Enable_DMA_Draw_BTE_Interrupt(void)
{
/*
Serial flash DMA Complete | Draw task finished | BTE Process
Complete etc. Interrupt Enable
0: Disable Interrupt.
1: Enable Interrupt.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0B);
	temp = RA8876_DataRead();
    temp |= cSetb2;
	RA8876_DataWrite(temp);
}
void Disable_DMA_Draw_BTE_Interrupt(void)
{
/*
Serial flash DMA Complete | Draw task finished | BTE Process
Complete etc. Interrupt Enable
0: Disable Interrupt.
1: Enable Interrupt.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0B);
	temp = RA8876_DataRead();
    temp &= cClrb2;
	RA8876_DataWrite(temp);
}
void Enable_PWM1_Interrupt(void)
{
/*
PWM timer 1 Interrupt Enable Bit
0: Disable Interrupt.
1: Enable Interrupt.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0B);
	temp = RA8876_DataRead();
    temp |= cSetb1;
	RA8876_DataWrite(temp);
}
void Disable_PWM1_Interrupt(void)
{
/*
PWM timer 1 Interrupt Enable Bit
0: Disable Interrupt.
1: Enable Interrupt.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0B);
	temp = RA8876_DataRead();
    temp &= cClrb1;
	RA8876_DataWrite(temp);
}
void Enable_PWM0_Interrupt(void)
{
/*
PWM timer 0 Interrupt Enable Bit
0: Disable Interrupt.
1: Enable Interrupt.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0B);
	temp = RA8876_DataRead();
    temp |= cSetb0;
	RA8876_DataWrite(temp);
}
void Disable_PWM0_Interrupt(void)
{
/*
PWM timer 0 Interrupt Enable Bit
0: Disable Interrupt.
1: Enable Interrupt.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0B);
	temp = RA8876_DataRead();
    temp &= cClrb0;
	RA8876_DataWrite(temp);
}

//[0Ch]=========================================================================
uint8_t Read_Interrupt_status(void)
{
/*
[Bit7]Read Function ..Resume Interrupt Status
0: No Resume interrupt happens.
1: Resume interrupt happens.
[Bit6]Read Function .. PS[0] pin Interrupt Status
0: No PS[0] pin interrupt happens.
1: PS[0] pin interrupt happens.
[Bit5]Read Function .. I2C master Interrupt Status
0: No I2C master interrupt happens.
1: I2C master interrupt happens.
[Bit4]Read Function .. Vsync Interrupt Status
0: No interrupt happens.
1: interrupt happens.
[Bit3]Read Function ..Key Scan Interrupt Status
0: No Key Scan interrupt happens.
1: Key Scan interrupt happens.
[Bit2]Read Function..Interrupt Status
0: No interrupt happens.
1: interrupt happens.
[Bit1]Read Function..Interrupt Status
0: No interrupt happens.
1: interrupt happens.
[Bit0]Read Function..Interrupt Status
0: No interrupt happens.
1: interrupt happens.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0C);
	temp = RA8876_DataRead();
    return temp;
}

void Check_Vsync_finished(void)
{
/*[Bit4]
Vsync Time base interrupt flag
Read Function .. Vsync Interrupt Status
0: No interrupt happens.
1: interrupt happens.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0C);
	RA8876_DataWrite(0x10);
	do{	   
		temp=RA8876_DataRead();
	
	}while((temp&0x10) == 0x00); 

}

void Clear_Resume_Interrupt_Flag(void)
{
/*
Resume Interrupt flag
Write Function .. Resume Interrupt Clear Bit
0: No operation.
1: Clear Resume interrupt.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0C);
	temp = RA8876_DataRead();
    temp |= cSetb7;
	RA8876_DataWrite(temp);
}
void Clear_ExtInterrupt_Input_Flag(void)
{
/*
External Interrupt (PS[0] pin) flag
Write Function .. PS[0] pin edge Interrupt Clear Bit
0: No operation.
1: Clear the PS[0] pin edge interrupt.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0C);
	temp = RA8876_DataRead();
    temp |= cSetb6;
	RA8876_DataWrite(temp);
}
void Clear_I2CM_Interrupt_Flag(void)
{
/*
I2C master Interrupt flag
Write Function.. I2C master Interrupt Clear Bit
0: No operation.
1: Clear the I2C master interrupt.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0C);
	temp = RA8876_DataRead();
    temp |= cSetb5;
	RA8876_DataWrite(temp);
}
void Clear_Vsync_Interrupt_Flag(void)
{
/*
Vsync Time base interrupt flag
Write Function ..Vsync Interrupt Clear Bit
0: No operation.
1: Clear the interrupt.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0C);
	temp = RA8876_DataRead();
    temp |= cSetb4;
	RA8876_DataWrite(temp);
}
void Clear_KeyScan_Interrupt_Flag(void)
{
/*
Key Scan Interrupt flag
Write Function..Key Scan Interrupt Clear Bit
0: No operation.
1: Clear the Key Scan interrupt.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0C);
	temp = RA8876_DataRead();
    temp |= cSetb3;
	RA8876_DataWrite(temp);
}
void Clear_DMA_Draw_BTE_Interrupt_Flag(void)
{
/*
Serial flash DMA Complete | Draw task finished | BTE
Process Complete etc. Interrupt flag
Write Function.. Interrupt Clear Bit
0: No operation.
1: Clear interrupt.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0C);
	temp = RA8876_DataRead();
    temp |= cSetb2;
	RA8876_DataWrite(temp);
}
void Clear_PWM1_Interrupt_Flag(void)
{
/*
PWM 1 timer Interrupt flag
Write Function..Interrupt Clear Bit
0: No operation.
1: Clear interrupt.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0C);
	temp = RA8876_DataRead();
    temp |= cSetb1;
	RA8876_DataWrite(temp);
}
void Clear_PWM0_Interrupt_Flag(void)
{
/*
PWM 0 timer Interrupt flag
Write Function..Interrupt Clear Bit
0: No operation.
1: Clear interrupt.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0C);
	temp = RA8876_DataRead();
    temp |= cSetb0;
	RA8876_DataWrite(temp);
}
//[0Dh]=========================================================================
void XnINTR_Mask_Resume_Interrupt_Flag(void)
{
/*
Mask Resume Interrupt Flag
0: Enable.
1: Mask.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0D);
	temp = RA8876_DataRead();
    temp |= cSetb7;
	RA8876_DataWrite(temp);
}
void XnINTR_Mask_ExtInterrupt_Input_Flag(void)
{
/*
Mask External Interrupt (PS[0] pin) Flag
0: Enable.
1: Mask.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0D);
	temp = RA8876_DataRead();
    temp |= cSetb6;
	RA8876_DataWrite(temp);
}
void XnINTR_Mask_I2CM_Interrupt_Flag(void)
{
/*
Mask I2C Master Interrupt Flag
0: Enable.
1: Mask.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0D);
	temp = RA8876_DataRead();
    temp |= cSetb5;
	RA8876_DataWrite(temp);
}
void XnINTR_Mask_Vsync_Interrupt_Flag(void)
{
/*
Mask Vsync time base interrupt Flag
0: Enable.
1: Mask.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0D);
	temp = RA8876_DataRead();
    temp |= cSetb4;
	RA8876_DataWrite(temp);
}
void XnINTR_Mask_KeyScan_Interrupt_Flag(void)
{
/*
Mask Key Scan Interrupt Flag
0: Enable.
1: Mask.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0D);
	temp = RA8876_DataRead();
    temp |= cSetb3;
	RA8876_DataWrite(temp);
}
void XnINTR_Mask_DMA_Draw_BTE_Interrupt_Flag(void)
{
/*
Mask Serial flash DMA Complete | Draw task finished | BTE
Process Complete etc. Interrupt Flag
0: Enable.
1: Mask.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0D);
	temp = RA8876_DataRead();
    temp |= cSetb2;
	RA8876_DataWrite(temp);
}
void XnINTR_Mask_PWM1_Interrupt_Flag(void)
{
/*
Mask PWM timer 1 Interrupt Flag
0: Enable.
1: Mask.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0D);
	temp = RA8876_DataRead();
    temp |= cSetb1;
	RA8876_DataWrite(temp);
}
void XnINTR_Mask_PWM0_Interrupt_Flag(void)
{
/*
Mask PWM timer 0 Interrupt Flag
0: Enable.
1: Mask.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0D);
	temp = RA8876_DataRead();
    temp |= cSetb0;
	RA8876_DataWrite(temp);
}

void XnINTR_Unmask_Resume_Interrupt_Flag(void)
{
/*
Mask Resume Interrupt Flag
0: Enable.
1: Mask.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0D);
	temp = RA8876_DataRead();
    temp &= cClrb7;
	RA8876_DataWrite(temp);
}
void XnINTR_Unmask_ExtInterrupt_Input_Flag(void)
{
/*
Mask External Interrupt (PS[0] pin) Flag
0: Enable.
1: Mask.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0D);
	temp = RA8876_DataRead();
    temp &= cClrb6;
	RA8876_DataWrite(temp);
}
void XnINTR_Unmask_I2CM_Interrupt_Flag(void)
{
/*
Mask I2C Master Interrupt Flag
0: Enable.
1: Mask.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0D);
	temp = RA8876_DataRead();
    temp &= cClrb5;
	RA8876_DataWrite(temp);
}
void XnINTR_Unmask_Vsync_Interrupt_Flag(void)
{
/*
Mask Vsync time base interrupt Flag
0: Enable.
1: Mask.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0D);
	temp = RA8876_DataRead();
    temp &= cClrb4;
	RA8876_DataWrite(temp);
}
void XnINTR_Unmask_KeyScan_Interrupt_Flag(void)
{
/*
Mask Key Scan Interrupt Flag
0: Enable.
1: Mask.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0D);
	temp = RA8876_DataRead();
    temp &= cClrb3;
	RA8876_DataWrite(temp);
}
void XnINTR_Unmask_DMA_Draw_BTE_Interrupt_Flag(void)
{
/*
Mask Serial flash DMA Complete | Draw task finished | BTE
Process Complete etc. Interrupt Flag
0: Enable.
1: Mask.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0D);
	temp = RA8876_DataRead();
    temp &= cClrb2;
	RA8876_DataWrite(temp);
}
void XnINTR_Unmask_PWM1_Interrupt_Flag(void)
{
/*
Mask PWM timer 1 Interrupt Flag
0: Enable.
1: Mask.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0D);
	temp = RA8876_DataRead();
    temp &= cClrb1;
	RA8876_DataWrite(temp);
}
void XnINTR_Unmask_PWM0_Interrupt_Flag(void)
{
/*
Mask PWM timer 0 Interrupt Flag
0: Enable.
1: Mask.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0D);
	temp = RA8876_DataRead();
    temp &= cClrb0;
	RA8876_DataWrite(temp);
}

//[0Eh]=========================================================================
void Enable_GPIOF_PullUp(void)
{
/*
GPIO_F[7:0] Pull-Up Enable (XPDAT[23:19, 15:13])
0: Pull-Up Disable
1: Pull-Up Enable
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0E);
	temp = RA8876_DataRead();
    temp |= cSetb5;
	RA8876_DataWrite(temp);
}
void Enable_GPIOE_PullUp(void)
{
/*
GPIO_E[7:0] Pull-Up Enable (XPDAT[12:10, 7:3])
0: Pull-Up Disable
1: Pull-Up Enable
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0E);
	temp = RA8876_DataRead();
    temp |= cSetb4;
	RA8876_DataWrite(temp);
}
void Enable_GPIOD_PullUp(void)
{
/*
GPIO_D[7:0] Pull-Up Enable (XPDAT[18, 2, 17, 16, 9, 8, 1,0])
0: Pull-Up Disable
1: Pull-Up Enable
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0E);
	temp = RA8876_DataRead();
    temp |= cSetb3;
	RA8876_DataWrite(temp);
}
void Enable_GPIOC_PullUp(void)
{
/*
GPIO_C[6:0] Pull-Up Enable (XSDA, XSCL, XnSFCS1,
XnSFCS0, XMISO, XMOSI , XSCK)
0: Pull-Up Disable
1: Pull-Up Enable
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0E);
	temp = RA8876_DataRead();
    temp |= cSetb2;
	RA8876_DataWrite(temp);
}
void Enable_XDB15_8_PullUp(void)
{
/*
XDB[15:8] Pull-Up Enable
0: Pull-Up Disable
1: Pull-Up Enable
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0E);
	temp = RA8876_DataRead();
    temp |= cSetb1;
	RA8876_DataWrite(temp);
}
void Enable_XDB7_0_PullUp(void)
{
/*
XDB[7:0] Pull-Up Enable
0: Pull-Up Disable
1: Pull-Up Enable
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0E);
	temp = RA8876_DataRead();
    temp |= cSetb0;
	RA8876_DataWrite(temp);
}
void Disable_GPIOF_PullUp(void)
{
/*
GPIO_F[7:0] Pull-Up Enable (XPDAT[23:19, 15:13])
0: Pull-Up Disable
1: Pull-Up Enable
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0E);
	temp = RA8876_DataRead();
    temp &= cClrb5;
	RA8876_DataWrite(temp);
}
void Disable_GPIOE_PullUp(void)
{
/*
GPIO_E[7:0] Pull-Up Enable (XPDAT[12:10, 7:3])
0: Pull-Up Disable
1: Pull-Up Enable
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0E);
	temp = RA8876_DataRead();
    temp &= cClrb4;
	RA8876_DataWrite(temp);
}
void Disable_GPIOD_PullUp(void)
{
/*
GPIO_D[7:0] Pull-Up Enable (XPDAT[18, 2, 17, 16, 9, 8, 1,0])
0: Pull-Up Disable
1: Pull-Up Enable
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0E);
	temp = RA8876_DataRead();
    temp &= cClrb3;
	RA8876_DataWrite(temp);
}
void Disable_GPIOC_PullUp(void)
{
/*
GPIO_C[6:0] Pull-Up Enable (XSDA, XSCL, XnSFCS1,
XnSFCS0, XMISO, XMOSI , XSCK)
0: Pull-Up Disable
1: Pull-Up Enable
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0E);
	temp = RA8876_DataRead();
    temp &= cClrb2;
	RA8876_DataWrite(temp);
}
void Disable_XDB15_8_PullUp(void)
{
/*
XDB[15:8] Pull-Up Enable
0: Pull-Up Disable
1: Pull-Up Enable
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0E);
	temp = RA8876_DataRead();
    temp &= cClrb1;
	RA8876_DataWrite(temp);
}
void Disable_XDB7_0_PullUp(void)
{
/*
XDB[7:0] Pull-Up Enable
0: Pull-Up Disable
1: Pull-Up Enable
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0E);
	temp = RA8876_DataRead();
    temp &= cClrb0;
	RA8876_DataWrite(temp);
}
//[0Fh]=========================================================================
void XPDAT18_Set_GPIO_D7(void)
{
/*
XPDAT[18] �V not scan function select
0: GPIO-D7
1: KOUT[4]
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0F);
	temp = RA8876_DataRead();
    temp &= cClrb7;
	RA8876_DataWrite(temp);
}
void XPDAT18_Set_KOUT4(void)
{
/*
XPDAT[18] �V not scan function select
0: GPIO-D7
1: KOUT[4]
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0F);
	temp = RA8876_DataRead();
    temp |= cSetb7;
	RA8876_DataWrite(temp);
}
void XPDAT17_Set_GPIO_D5(void)
{
/*
XPDAT[17] �V not scan function select
0: GPIO-D5
1: KOUT[2]
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0F);
	temp = RA8876_DataRead();
    temp &= cClrb6;
	RA8876_DataWrite(temp);
}
void XPDAT17_Set_KOUT2(void)
{
/*
XPDAT[17] �V not scan function select
0: GPIO-D5
1: KOUT[2]
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0F);
	temp = RA8876_DataRead();
    temp |= cSetb6;
	RA8876_DataWrite(temp);
}
void XPDAT16_Set_GPIO_D4(void)
{
/*
XPDAT[16] �V not scan function select
0: GPIO-D4
1: KOUT[1]
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0F);
	temp = RA8876_DataRead();
    temp &= cClrb5;
	RA8876_DataWrite(temp);
}
void XPDAT16_Set_KOUT1(void)
{
/*
XPDAT[16] �V not scan function select
0: GPIO-D4
1: KOUT[1]
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0F);
	temp = RA8876_DataRead();
    temp |= cSetb5;
	RA8876_DataWrite(temp);
}
void XPDAT9_Set_GPIO_D3(void)
{
/*
XPDAT[9] �V not scan function select
0: GPIO-D3
1: KOUT[3]
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0F);
	temp = RA8876_DataRead();
    temp &= cClrb4;
	RA8876_DataWrite(temp);
}
void XPDAT9_Set_KOUT3(void)
{
/*
XPDAT[9] �V not scan function select
0: GPIO-D3
1: KOUT[3]
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0F);
	temp = RA8876_DataRead();
    temp |= cSetb4;
	RA8876_DataWrite(temp);
}
void XPDAT8_Set_GPIO_D2(void)
{
/*
XPDAT[8] �V not scan function select
0: GPIO-D2
1: KIN[3]
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0F);
	temp = RA8876_DataRead();
    temp &= cClrb3;
	RA8876_DataWrite(temp);
}
void XPDAT8_Set_KIN3(void)
{
/*
XPDAT[8] �V not scan function select
0: GPIO-D2
1: KIN[3]
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0F);
	temp = RA8876_DataRead();
    temp |= cSetb3;
	RA8876_DataWrite(temp);
}
void XPDAT2_Set_GPIO_D6(void)
{
/*
XPDAT[2] �V not scan function select
0: GPIO-D6
1: KIN[4]
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0F);
	temp = RA8876_DataRead();
    temp &= cClrb2;
	RA8876_DataWrite(temp);
}
void XPDAT2_Set_KIN4(void)
{
/*
XPDAT[2] �V not scan function select
0: GPIO-D6
1: KIN[4]
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0F);
	temp = RA8876_DataRead();
    temp |= cSetb2;
	RA8876_DataWrite(temp);
}
void XPDAT1_Set_GPIO_D1(void)
{
/*
XPDAT[1] �V not scan function select
0: GPIO-D1
1: KIN[2]
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0F);
	temp = RA8876_DataRead();
    temp &= cClrb1;
	RA8876_DataWrite(temp);
}
void XPDAT1_Set_KIN2(void)
{
/*
XPDAT[1] �V not scan function select
0: GPIO-D1
1: KIN[2]
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0F);
	temp = RA8876_DataRead();
    temp |= cSetb1;
	RA8876_DataWrite(temp);
}
void XPDAT0_Set_GPIO_D0(void)
{
/*
XPDAT[0] �V not scan function select
0: GPIO-D0
1: KIN[1]
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0F);
	temp = RA8876_DataRead();
    temp &= cClrb0;
	RA8876_DataWrite(temp);
}
void XPDAT0_Set_KIN1(void)
{
/*
XPDAT[0] �V not scan function select
0: GPIO-D0
1: KIN[1]
*/
	uint8_t temp;
	RA8876_CmdWrite(0x0F);
	temp = RA8876_DataRead();
    temp |= cSetb0;
	RA8876_DataWrite(temp);
}

//[10h]=========================================================================
void Enable_PIP1(void)
{
/*
PIP 1 window Enable/Disable
0 : PIP 1 window disable.
1 : PIP 1 window enable
PIP 1 window always on top of PIP 2 window.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x10);
	temp = RA8876_DataRead();
    temp |= cSetb7;
	RA8876_DataWrite(temp);
}
void Disable_PIP1(void)
{
/*
PIP 1 window Enable/Disable
0 : PIP 1 window disable.
1 : PIP 1 window enable
PIP 1 window always on top of PIP 2 window.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x10);
	temp = RA8876_DataRead();
    temp &= cClrb7;
	RA8876_DataWrite(temp);
}
void Enable_PIP2(void)
{
/*
PIP 2 window Enable/Disable
0 : PIP 2 window disable.
1 : PIP 2 window enable
PIP 1 window always on top of PIP 2 window.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x10);
	temp = RA8876_DataRead();
    temp |= cSetb6;
	RA8876_DataWrite(temp);
}
void Disable_PIP2(void)
{
/*
PIP 2 window Enable/Disable
0 : PIP 2 window disable.
1 : PIP 2 window enable
PIP 1 window always on top of PIP 2 window.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x10);
	temp = RA8876_DataRead();
    temp &= cClrb6;
	RA8876_DataWrite(temp);
}
void Select_PIP1_Parameter(void)
{
/*
0: To configure PIP 1��s parameters.
1: To configure PIP 2��s parameters..
*/
	uint8_t temp;
	RA8876_CmdWrite(0x10);
	temp = RA8876_DataRead();
    temp &= cClrb4;
	RA8876_DataWrite(temp);
}
void Select_PIP2_Parameter(void)
{
/*
0: To configure PIP 1��s parameters.
1: To configure PIP 2��s parameters..
*/
	uint8_t temp;
	RA8876_CmdWrite(0x10);
	temp = RA8876_DataRead();
    temp |= cSetb4;
	RA8876_DataWrite(temp);
}
void Select_Main_Window_8bpp(void)
{
/*
Main Window Color Depth Setting
00b: 8-bpp generic TFT, i.e. 256 colors.
01b: 16-bpp generic TFT, i.e. 65K colors.
1xb: 24-bpp generic TFT, i.e. 1.67M colors.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x10);
	temp = RA8876_DataRead();
    temp &= cClrb3;
    temp &= cClrb2;
	RA8876_DataWrite(temp);
}

/*
*********************************************************************************************************
*	�� �� ��: Select_Main_Window_16bpp
*	����˵��: ��������ɫ�趨
*	��    �Σ���
*	�� �� ֵ: �������(���账��)
*********************************************************************************************************
*/
void Select_Main_Window_16bpp(void)
{
/*
Main Window Color Depth Setting
00b: 8-bpp generic TFT, i.e. 256 colors.
01b: 16-bpp generic TFT, i.e. 65K colors.
1xb: 24-bpp generic TFT, i.e. 1.67M colors.
*/
/*
	��/PIP���ڿ��ƼĴ���[10H]
bit3-2��
	Main Image Color Depth Setting
	00b: 8-bpp generic TFT, i.e. 256 ɫ��
	01b: 16-bpp generic TFT, i.e. 65K ɫ��
	1xb: 24-bpp generic TFT, i.e. 1.67 ɫ��
*/
	uint8_t temp;
	RA8876_CmdWrite(0x10);
	temp = RA8876_DataRead();
    temp &= cClrb3;
    temp |= cSetb2;
	RA8876_DataWrite(temp);
}

/*
*********************************************************************************************************
*	�� �� ��: Select_Main_Window_16bpp
*	����˵��: ��������ɫ�趨
*	��    �Σ���
*	�� �� ֵ: �������(���账��)
*********************************************************************************************************
*/
void Select_Main_Window_24bpp(void)
{
/*
Main Window Color Depth Setting
00b: 8-bpp generic TFT, i.e. 256 colors.
01b: 16-bpp generic TFT, i.e. 65K colors.
1xb: 24-bpp generic TFT, i.e. 1.67M colors.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x10);
	temp = RA8876_DataRead();
    temp |= cSetb3;
	RA8876_DataWrite(temp);
}

void Select_LCD_Sync_Mode(void)
{
/*
To Control panel's synchronous signals
0: Sync Mode: Enable XVSYNC, XHSYNC, XDE
1: DE Mode: Only XDE enable, XVSYNC & XHSYNC in idle state
*/
	uint8_t temp;
	RA8876_CmdWrite(0x10);
	temp = RA8876_DataRead();
    temp &= cClrb0;
	RA8876_DataWrite(temp);
}
void Select_LCD_DE_Mode(void)
{
/*
To Control panel's synchronous signals
0: Sync Mode: Enable XVSYNC, XHSYNC, XDE
1: DE Mode: Only XDE enable, XVSYNC & XHSYNC in idle state
*/
	uint8_t temp;
	RA8876_CmdWrite(0x10);
	temp = RA8876_DataRead();
    temp |= cSetb0;
	RA8876_DataWrite(temp);
}

//[11h]=========================================================================
void Select_PIP1_Window_8bpp(void)
{
/*
PIP 1 Window Color Depth Setting
00b: 8-bpp generic TFT, i.e. 256 colors.
01b: 16-bpp generic TFT, i.e. 65K colors.
1xb: 24-bpp generic TFT, i.e. 1.67M colors.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x11);
	temp = RA8876_DataRead();
    temp &= cClrb3;
    temp &= cClrb2;
	RA8876_DataWrite(temp);
}
void Select_PIP1_Window_16bpp(void)
{
/*
PIP 1 Window Color Depth Setting
00b: 8-bpp generic TFT, i.e. 256 colors.
01b: 16-bpp generic TFT, i.e. 65K colors.
1xb: 24-bpp generic TFT, i.e. 1.67M colors.
*/
/*
	PIP������ɫ����趨�Ĵ���[11H]
bit3-2��
	PIP 1 Window Color Depth Setting
	00b: 8-bpp generic TFT, i.e. 256 ɫ��
	01b: 16-bpp generic TFT, i.e. 65K ɫ��
	1xb: 24-bpp generic TFT, i.e. 1.67M ɫ
*/
	uint8_t temp;
	RA8876_CmdWrite(0x11);
	temp = RA8876_DataRead();
    temp &= cClrb3;
    temp |= cSetb2;
	RA8876_DataWrite(temp);
}
void Select_PIP1_Window_24bpp(void)
{
/*
PIP 1 Window Color Depth Setting
00b: 8-bpp generic TFT, i.e. 256 colors.
01b: 16-bpp generic TFT, i.e. 65K colors.
1xb: 24-bpp generic TFT, i.e. 1.67M colors.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x11);
	temp = RA8876_DataRead();
    temp |= cSetb3;
//    temp &= cClrb2;
	RA8876_DataWrite(temp);
}
void Select_PIP2_Window_8bpp(void)
{
/*
PIP 2 Window Color Depth Setting
00b: 8-bpp generic TFT, i.e. 256 colors.
01b: 16-bpp generic TFT, i.e. 65K colors.
1xb: 24-bpp generic TFT, i.e. 1.67M colors.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x11);
	temp = RA8876_DataRead();
    temp &= cClrb1;
    temp &= cClrb0;
	RA8876_DataWrite(temp);
}
void Select_PIP2_Window_16bpp(void)
{
/*
PIP 2 Window Color Depth Setting
00b: 8-bpp generic TFT, i.e. 256 colors.
01b: 16-bpp generic TFT, i.e. 65K colors.
1xb: 24-bpp generic TFT, i.e. 1.67M colors.
*/
/*
	PIP������ɫ����趨�Ĵ���[11H]
bit1-0��
	PIP 2 Window Color Depth Setting
	00b: 8-bpp generic TFT, i.e. 256 ɫ��
	01b: 16-bpp generic TFT, i.e. 65K ɫ��
	1xb: 24-bpp generic TFT, i.e. 1.67M ɫ��
*/
	uint8_t temp;
	RA8876_CmdWrite(0x11);
	temp = RA8876_DataRead();
    temp &= cClrb1;
    temp |= cSetb0;
	RA8876_DataWrite(temp);
}
void Select_PIP2_Window_24bpp(void)
{
/*
PIP 2 Window Color Depth Setting
00b: 8-bpp generic TFT, i.e. 256 colors.
01b: 16-bpp generic TFT, i.e. 65K colors.
1xb: 24-bpp generic TFT, i.e. 1.67M colors.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x11);
	temp = RA8876_DataRead();
    temp |= cSetb1;
//    temp &= cClrb0;
	RA8876_DataWrite(temp);
}

//[12h]=========================================================================
void PCLK_Rising(void)	 
{
/*
PCLK Inversion
0: PDAT, DE, HSYNC etc. Drive(/ change) at PCLK falling edge.
1: PDAT, DE, HSYNC etc. Drive(/ change) at PCLK rising edge.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x12);
	temp = RA8876_DataRead();
    temp &= cClrb7;
	RA8876_DataWrite(temp);
}
void PCLK_Falling(void)
{
/*
PCLK Inversion
0: PDAT, DE, HSYNC etc. Drive(/ change) at PCLK falling edge.
1: PDAT, DE, HSYNC etc. Drive(/ change) at PCLK rising edge.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x12);
	temp = RA8876_DataRead();
    temp |= cSetb7;
	RA8876_DataWrite(temp);
}

/*
*********************************************************************************************************
*	�� �� ��: Display_ON
*	����˵��: ����ʾ
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void Display_ON(void)
{
/*	
Display ON/OFF
0b: Display Off.
1b: Display On.
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x12);
	temp = RA8876_DataRead();
	temp |= cSetb6;
	RA8876_DataWrite(temp);
}

/*
*********************************************************************************************************
*	�� �� ��: Display_OFF
*	����˵��: ����ʾ
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void Display_OFF(void)
{
/*	
Display ON/OFF
0b: Display Off.
1b: Display On.
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x12);
	temp = RA8876_DataRead();
	temp &= cClrb6;
	RA8876_DataWrite(temp);
}

/*
*********************************************************************************************************
*	�� �� ��: Color_Bar_ON
*	����˵��: ��ʾ����ɫ��
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void Color_Bar_ON(void)
{
/*	
Display Test Color Bar
0b: Disable.
1b: Enable.
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x12);
	temp = RA8876_DataRead();
	temp |= cSetb5;
	RA8876_DataWrite(temp);
}


/*
*********************************************************************************************************
*	�� �� ��: Color_Bar_OFF
*	����˵��: �ز���ɫ��
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void Color_Bar_OFF(void)
{
/*	
Display Test Color Bar
0b: Disable.
1b: Enable.
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x12);
	temp = RA8876_DataRead();
	temp &= cClrb5;
	RA8876_DataWrite(temp);
}


void VSCAN_T_to_B(void)
{
/*	
Vertical Scan direction
0 : From Top to Bottom
1 : From bottom to Top
PIP window will be disabled when VDIR set as 1.
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x12);
	temp = RA8876_DataRead();
	temp &= cClrb3;
	RA8876_DataWrite(temp);
}
void VSCAN_B_to_T(void)
{
/*	
Vertical Scan direction
0 : From Top to Bottom
1 : From bottom to Top
PIP window will be disabled when VDIR set as 1.
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x12);
	temp = RA8876_DataRead();
	temp |= cSetb3;
	RA8876_DataWrite(temp);
}
void PDATA_Set_RGB(void)
{
/*parallel PDATA[23:0] Output Sequence
000b : RGB
001b : RBG
010b : GRB
011b : GBR
100b : BRG
101b : BGR
110b : Gray			
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x12);
	temp = RA8876_DataRead();
    temp &=0xf8;
	RA8876_DataWrite(temp);
}
void PDATA_Set_RBG(void)
{
/*parallel PDATA[23:0] Output Sequence
000b : RGB
001b : RBG
010b : GRB
011b : GBR
100b : BRG
101b : BGR
110b : Gray			
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x12);
	temp = RA8876_DataRead();
	temp &=0xf8;
    temp |= cSetb0;
	RA8876_DataWrite(temp);
}
void PDATA_Set_GRB(void)
{
/*parallel PDATA[23:0] Output Sequence
000b : RGB
001b : RBG
010b : GRB
011b : GBR
100b : BRG
101b : BGR
110b : Gray			
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x12);
	temp = RA8876_DataRead();
	temp &=0xf8;
    temp |= cSetb1;
	RA8876_DataWrite(temp);
}
void PDATA_Set_GBR(void)
{
/*parallel PDATA[23:0] Output Sequence
000b : RGB
001b : RBG
010b : GRB
011b : GBR
100b : BRG
101b : BGR
110b : Gray			
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x12);
	temp = RA8876_DataRead();
	temp &=0xf8;
    temp |= cSetb1;
    temp |= cSetb0;
	RA8876_DataWrite(temp);
}
void PDATA_Set_BRG(void)
{
/*parallel PDATA[23:0] Output Sequence
000b : RGB
001b : RBG
010b : GRB
011b : GBR
100b : BRG
101b : BGR
110b : Gray			
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x12);
	temp = RA8876_DataRead();
	temp &=0xf8;
	temp |= cSetb2;
	RA8876_DataWrite(temp);
}
void PDATA_Set_BGR(void)
{
/*parallel PDATA[23:0] Output Sequence
000b : RGB
001b : RBG
010b : GRB
011b : GBR
100b : BRG
101b : BGR
110b : Gray			
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x12);
	temp = RA8876_DataRead();
	temp &=0xf8;
	temp |= cSetb2;
    temp |= cSetb0;
	RA8876_DataWrite(temp);
}
void PDATA_Set_Gray(void)
{
/*parallel PDATA[23:0] Output Sequence
000b : RGB
001b : RBG
010b : GRB
011b : GBR
100b : BRG
101b : BGR
110b : Gray			
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x12);
	temp = RA8876_DataRead();
	temp &=0xf8;
	temp |= cSetb2;
    temp |= cSetb1;
	RA8876_DataWrite(temp);
}

void PDATA_IDLE_STATE(void)
{
/*
111b : XPDAT pins send out PDAT idle state (all 0 or 1, black or white color).
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x12);
	temp = RA8876_DataRead();
	temp |=0x07;
	RA8876_DataWrite(temp);
}


//[13h]=========================================================================
void HSYNC_Low_Active(void)
{
/*	
HSYNC Polarity
0 : Low active.
1 : High active.
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x13);
	temp = RA8876_DataRead();
    temp &= cClrb7;
	RA8876_DataWrite(temp);
}
void HSYNC_High_Active(void)
{
/*	
HSYNC Polarity
0 : Low active.
1 : High active.
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x13);
	temp = RA8876_DataRead();
	temp |= cSetb7;
	RA8876_DataWrite(temp);
}
void VSYNC_Low_Active(void)
{
/*	
VSYNC Polarity
0 : Low active.
1 : High active.
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x13);
	temp = RA8876_DataRead();
    temp &= cClrb6;
	RA8876_DataWrite(temp);
}
void VSYNC_High_Active(void)
{
/*	
VSYNC Polarity
0 : Low active.
1 : High active.
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x13);
	temp = RA8876_DataRead();
	temp |= cSetb6;
	RA8876_DataWrite(temp);
}
void DE_Low_Active(void)
{
/*	
DE Polarity
0 : High active.
1 : Low active.
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x13);
	temp = RA8876_DataRead();
    temp |= cSetb5;
	RA8876_DataWrite(temp);
}
void DE_High_Active(void)
{
/*	
DE Polarity
0 : High active.
1 : Low active.
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x13);
	temp = RA8876_DataRead();
	temp &= cClrb5;
	RA8876_DataWrite(temp);
}
void Idle_DE_Low(void)
{
/*	
DE IDLE STATE(When STANDBY or DISPLAY OFF )
0 : Pin ��DE�� output is low.
1 : Pin ��DE�� output is high.
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x13);
	temp = RA8876_DataRead();
    temp &= cClrb4;
	RA8876_DataWrite(temp);
}
void Idle_DE_High(void)
{
/*	
DE IDLE STATE(When STANDBY or DISPLAY OFF )
0 : Pin ��DE�� output is low.
1 : Pin ��DE�� output is high.
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x13);
	temp = RA8876_DataRead();
	temp |= cSetb4;
	RA8876_DataWrite(temp);
}
void Idle_PCLK_Low(void)
{
/*	
PCLK IDLE STATE(When STANDBY or DISPLAY OFF )
0 : Pin ��PCLK�� output is low.
1 : Pin ��PCLK�� output is high.
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x13);
	temp = RA8876_DataRead();
    temp &= cClrb3;
	RA8876_DataWrite(temp);
}
void Idle_PCLK_High(void)
{
/*	
PCLK IDLE STATE(When STANDBY or DISPLAY OFF )
0 : Pin ��PCLK�� output is low.
1 : Pin ��PCLK�� output is high.
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x13);
	temp = RA8876_DataRead();
	temp |= cSetb3;
	RA8876_DataWrite(temp);
}
void Idle_PDAT_Low(void)
{
/*	
PDAT IDLE STATE(When STANDBY or DISPLAY OFF )
0 : Pins ��PDAT[23:0]�� output is low.
1 : Pins ��PCLK[23:0]�� output is high.
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x13);
	temp = RA8876_DataRead();
    temp &= cClrb2;
	RA8876_DataWrite(temp);
}
void Idle_PDAT_High(void)
{
/*	
PDAT IDLE STATE(When STANDBY or DISPLAY OFF )
0 : Pins ��PDAT[23:0]�� output is low.
1 : Pins ��PCLK[23:0]�� output is high.
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x13);
	temp = RA8876_DataRead();
	temp |= cSetb2;
	RA8876_DataWrite(temp);
}
void Idle_HSYNC_Low(void)
{
/*	
HSYNC IDLE STATE(When STANDBY or DISPLAY OFF )
0 : Pin ��HSYNC�� output is low.
1 : Pin ��HSYNC�� output is high.
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x13);
	temp = RA8876_DataRead();
    temp &= cClrb1;
	RA8876_DataWrite(temp);
}
void Idle_HSYNC_High(void)
{
/*	
HSYNC IDLE STATE(When STANDBY or DISPLAY OFF )
0 : Pin ��HSYNC�� output is low.
1 : Pin ��HSYNC�� output is high.
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x13);
	temp = RA8876_DataRead();
	temp |= cSetb1;
	RA8876_DataWrite(temp);
}
void Idle_VSYNC_Low(void)
{
/*	
VSYNC IDLE STATE(When STANDBY or DISPLAY OFF )
0 : Pin ��VSYNC�� output is low.
1 : Pin ��VSYNC�� output is high.
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x13);
	temp = RA8876_DataRead();
    temp &= cClrb0;
	RA8876_DataWrite(temp);
}
void Idle_VSYNC_High(void)
{
/*	
VSYNC IDLE STATE(When STANDBY or DISPLAY OFF )
0 : Pin ��VSYNC�� output is low.
1 : Pin ��VSYNC�� output is high.
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x13);
	temp = RA8876_DataRead();
	temp |= cSetb0;
	RA8876_DataWrite(temp);
}

//[14h][15h][1Ah][1Bh]=========================================================================
void LCD_HorizontalWidth_VerticalHeight(uint16_t WX,uint16_t HY)
{
/* ˮƽ��Ⱥʹ�ֱ�߶��趨����LCD���ķֱ����йأ� */
/*
[14h] Horizontal Display Width Setting Bit[7:0]
[15h] Horizontal Display Width Fine Tuning (HDWFT) [3:0]
The register specifies the LCD panel horizontal display width in
the unit of 8 pixels resolution.
Horizontal display width(pixels) = (HDWR + 1) * 8 + HDWFTR

[1Ah] Vertical Display Height Bit[7:0]
Vertical Display Height(Line) = VDHR + 1
[1Bh] Vertical Display Height Bit[10:8]
Vertical Display Height(Line) = VDHR + 1
*/
	uint8_t temp;
	if(WX<8)
    {
	RA8876_CmdWrite(0x14);
	RA8876_DataWrite(0x00);
    
	RA8876_CmdWrite(0x15);
	RA8876_DataWrite(WX);
    
    temp=HY-1;
	RA8876_CmdWrite(0x1A);
	RA8876_DataWrite(temp);
	    
	temp=(HY-1)>>8;
	RA8876_CmdWrite(0x1B);
	RA8876_DataWrite(temp);
	}
	else
	{
    temp=(WX/8)-1;
	RA8876_CmdWrite(0x14);
	RA8876_DataWrite(temp);
    
    temp=WX%8;
	RA8876_CmdWrite(0x15);
	RA8876_DataWrite(temp);
    
    temp=HY-1;
	RA8876_CmdWrite(0x1A);
	RA8876_DataWrite(temp);
	    
	temp=(HY-1)>>8;
	RA8876_CmdWrite(0x1B);
	RA8876_DataWrite(temp);
	}
}
//[16h][17h]=========================================================================
void LCD_Horizontal_Non_Display(uint16_t WX)
{
/*
[16h] Horizontal Non-Display Period(HNDR) Bit[4:0]
This register specifies the horizontal non-display period. Also
called back porch.
Horizontal non-display period(pixels) = (HNDR + 1) * 8 + HNDFTR

[17h] Horizontal Non-Display Period Fine Tuning(HNDFT) [3:0]
This register specifies the fine tuning for horizontal non-display
period; it is used to support the SYNC mode panel. Each level of
this modulation is 1-pixel.
Horizontal non-display period(pixels) = (HNDR + 1) * 8 + HNDFTR
*/
	uint8_t temp;
	if(WX<8)
	{
	RA8876_CmdWrite(0x16);
	RA8876_DataWrite(0x00);
    
	RA8876_CmdWrite(0x17);
	RA8876_DataWrite(WX);
	}
	else
	{
    temp=(WX/8)-1;
	RA8876_CmdWrite(0x16);
	RA8876_DataWrite(temp);
    
    temp=WX%8;
	RA8876_CmdWrite(0x17);
	RA8876_DataWrite(temp);
	}	
}
//[18h]=========================================================================
void LCD_HSYNC_Start_Position(uint16_t WX)
{
/*
[18h] HSYNC Start Position[4:0]
The starting position from the end of display area to the
beginning of HSYNC. Each level of this modulation is 8-pixel.
Also called front porch.
HSYNC Start Position(pixels) = (HSTR + 1)x8
*/
	uint8_t temp;
	if(WX<8)
	{
	RA8876_CmdWrite(0x18);
	RA8876_DataWrite(0x00);
	}
	else
	{
    temp=(WX/8)-1;
	RA8876_CmdWrite(0x18);
	RA8876_DataWrite(temp);	
	}
}
//[19h]=========================================================================
void LCD_HSYNC_Pulse_Width(uint16_t WX)
{
/* HSYNC���ڿ�� */
/*
[19h] HSYNC Pulse Width(HPW) [4:0]
The period width of HSYNC.
HSYNC Pulse Width(pixels) = (HPW + 1)x8
*/
	uint8_t temp;
	if(WX<8)
	{
	RA8876_CmdWrite(0x19);
	RA8876_DataWrite(0x00);
	}
	else
	{
    temp=(WX/8)-1;
	RA8876_CmdWrite(0x19);
	RA8876_DataWrite(temp);	
	}
}
//[1Ch][1Dh]=========================================================================
void LCD_Vertical_Non_Display(uint16_t HY)
{
/*
[1Ch] Vertical Non-Display Period Bit[7:0]
Vertical Non-Display Period(Line) = (VNDR + 1)

[1Dh] Vertical Non-Display Period Bit[9:8]
Vertical Non-Display Period(Line) = (VNDR + 1)
*/
	uint16_t temp;
    temp=HY-1;
	RA8876_CmdWrite(0x1C);
	RA8876_DataWrite(temp);

	RA8876_CmdWrite(0x1D);
	RA8876_DataWrite(temp>>8);
}
//[1Eh]=========================================================================
void LCD_VSYNC_Start_Position(uint16_t HY)
{
/*
[1Eh] VSYNC Start Position[7:0]
The starting position from the end of display area to the beginning of VSYNC.
VSYNC Start Position(Line) = (VSTR + 1)
*/
	uint8_t temp;
    temp=HY-1;
	RA8876_CmdWrite(0x1E);
	RA8876_DataWrite(temp);
}
//[1Fh]=========================================================================
void LCD_VSYNC_Pulse_Width(uint16_t HY)
{
/*
[1Fh] VSYNC Pulse Width[5:0]
The pulse width of VSYNC in lines.
VSYNC Pulse Width(Line) = (VPWR + 1)
*/
	uint8_t temp;
    temp=HY-1;
	RA8876_CmdWrite(0x1F);
	RA8876_DataWrite(temp);
}

/*
*********************************************************************************************************
*	�� �� ��: Main_Image_Start_Address
*	����˵��: ��Ӱ����ʼ��ַ
*	��    �Σ���
*	�� �� ֵ: �������(���账��)
*********************************************************************************************************
*/
void Main_Image_Start_Address(unsigned long Addr)	
{
/*
[20h] Main Image Start Address[7:2]
[21h] Main Image Start Address[15:8]
[22h] Main Image Start Address [23:16]
[23h] Main Image Start Address [31:24]
*/
	RA8876_CmdWrite(0x20);	RA8876_DataWrite(Addr);
	RA8876_CmdWrite(0x21);	RA8876_DataWrite(Addr>>8);
	RA8876_CmdWrite(0x22);	RA8876_DataWrite(Addr>>16);
	RA8876_CmdWrite(0x23);	RA8876_DataWrite(Addr>>24);
}

/*
*********************************************************************************************************
*	�� �� ��: Main_Image_Width
*	����˵��: ��Ӱ����
*	��    �Σ���
*	�� �� ֵ: �������(���账��)
*********************************************************************************************************
*/
void Main_Image_Width(uint16_t WX)	
{
/*
[24h] Main Image Width [7:0]
[25h] Main Image Width [12:8]
Unit: Pixel.
It must be divisible by 4. MIW Bit [1:0] tie to ��0�� internally.
The value is physical pixel number. Maximum value is 8188 pixels
*/
	RA8876_CmdWrite(0x24);	RA8876_DataWrite(WX);
	RA8876_CmdWrite(0x25);	RA8876_DataWrite(WX>>8);
}

/*
*********************************************************************************************************
*	�� �� ��: Main_Window_Start_XY
*	����˵��: ��������ʼλ��
*	��    �Σ���
*	�� �� ֵ: �������(���账��)
*********************************************************************************************************
*/
void Main_Window_Start_XY(uint16_t WX,uint16_t HY)	
{
/*
[26h] Main Window Upper-Left corner X-coordination [7:0]
[27h] Main Window Upper-Left corner X-coordination [12:8]
Reference Main Image coordination.
Unit: Pixel
It must be divisible by 4. MWULX Bit [1:0] tie to ��0�� internally.
X-axis coordination plus Horizontal display width cannot large than 8188.

[28h] Main Window Upper-Left corner Y-coordination [7:0]
[29h] Main Window Upper-Left corner Y-coordination [12:8]
Reference Main Image coordination.
Unit: Pixel
Range is between 0 and 8191.
*/
	RA8876_CmdWrite(0x26);	RA8876_DataWrite(WX);
	RA8876_CmdWrite(0x27);	RA8876_DataWrite(WX>>8);
	RA8876_CmdWrite(0x28);	RA8876_DataWrite(HY);
	RA8876_CmdWrite(0x29);	RA8876_DataWrite(HY>>8);
}
//[2Ah][2Bh][2Ch][2Dh]=========================================================================
void PIP_Display_Start_XY(uint16_t WX,uint16_t HY)	
{
/*
[2Ah] PIP Window Display Upper-Left corner X-coordination [7:0]
[2Bh] PIP Window Display Upper-Left corner X-coordination [12:8]
Reference Main Window coordination.
Unit: Pixel
It must be divisible by 4. PWDULX Bit [1:0] tie to ��0�� internally.
X-axis coordination should less than horizontal display width.
According to bit of Select Configure PIP 1 or 2 Window��s parameters. 
Function bit will be configured for relative PIP window.

[2Ch] PIP Window Display Upper-Left corner Y-coordination [7:0]
[2Dh] PIP Window Display Upper-Left corner Y-coordination [12:8]
Reference Main Window coordination.
Unit: Pixel
Y-axis coordination should less than vertical display height.
According to bit of Select Configure PIP 1 or 2 Window��s parameters.
Function bit will be configured for relative PIP window.
*/
	RA8876_CmdWrite(0x2A);	RA8876_DataWrite(WX);
	RA8876_CmdWrite(0x2B);	RA8876_DataWrite(WX>>8);
	RA8876_CmdWrite(0x2C);	RA8876_DataWrite(HY);
	RA8876_CmdWrite(0x2D);	RA8876_DataWrite(HY>>8);
}
//[2Eh][2Fh][30h][31h]=========================================================================
void PIP_Image_Start_Address(unsigned long Addr)	
{
/*
[2Eh] PIP Image Start Address[7:2]
[2Fh] PIP Image Start Address[15:8]
[30h] PIP Image Start Address [23:16]
[31h] PIP Image Start Address [31:24]
*/
	RA8876_CmdWrite(0x2E);	RA8876_DataWrite(Addr);
	RA8876_CmdWrite(0x2F);	RA8876_DataWrite(Addr>>8);
	RA8876_CmdWrite(0x30);	RA8876_DataWrite(Addr>>16);
	RA8876_CmdWrite(0x31);	RA8876_DataWrite(Addr>>24);
}
//[32h][33h]=========================================================================
void PIP_Image_Width(uint16_t WX)	
{
/*
[32h] PIP Image Width [7:0]
[33h] PIP Image Width [12:8]
Unit: Pixel.
It must be divisible by 4. PIW Bit [1:0] tie to ��0�� internally.
The value is physical pixel number.
This width should less than horizontal display width.
According to bit of Select Configure PIP 1 or 2 Window��s parameters.
Function bit will be configured for relative PIP window.
*/
	RA8876_CmdWrite(0x32);	RA8876_DataWrite(WX);
	RA8876_CmdWrite(0x33);	RA8876_DataWrite(WX>>8);
}
//[34h][35h][36h][37h]=========================================================================
void PIP_Window_Image_Start_XY(uint16_t WX,uint16_t HY)	
{

/*
[34h] PIP 1 or 2 Window Image Upper-Left corner X-coordination [7:0]
[35h] PIP Window Image Upper-Left corner X-coordination [12:8]
Reference PIP Image coordination.
Unit: Pixel
It must be divisible by 4. PWIULX Bit [1:0] tie to ��0�� internally.
X-axis coordination plus PIP image width cannot large than 8188.
According to bit of Select Configure PIP 1 or 2 Window��s parameters. 
Function bit will be configured for relative PIP window.

[36h] PIP Windows Display Upper-Left corner Y-coordination [7:0]
[37h] PIP Windows Image Upper-Left corner Y-coordination [12:8]
Reference PIP Image coordination.
Unit: Pixel
Y-axis coordination plus PIP window height should less than 8191.
According to bit of Select Configure PIP 1 or 2 Window��s parameters. 
Function bit will be configured for relative PIP window.
*/
	RA8876_CmdWrite(0x34);	RA8876_DataWrite(WX);
	RA8876_CmdWrite(0x35);	RA8876_DataWrite(WX>>8);
	RA8876_CmdWrite(0x36);	RA8876_DataWrite(HY);
	RA8876_CmdWrite(0x37);	RA8876_DataWrite(HY>>8);
}
//[38h][39h][3Ah][3Bh]=========================================================================
void PIP_Window_Width_Height(uint16_t WX,uint16_t HY)	
{
/*
[38h] PIP Window Width [7:0]
[39h] PIP Window Width [10:8]
Unit: Pixel.
It must be divisible by 4. The value is physical pixel number.
Maximum value is 8188 pixels.
According to bit of Select Configure PIP 1 or 2 Window��s parameters. 
Function bit will be configured for relative PIP window.

[3Ah] PIP Window Height [7:0]
[3Bh] PIP Window Height [10:8]
Unit: Pixel
The value is physical pixel number. Maximum value is 8191 pixels.
According to bit of Select Configure PIP 1 or 2 Window��s parameters. 
Function bit will be configured for relative PIP window.
*/
	RA8876_CmdWrite(0x38);	RA8876_DataWrite(WX);
	RA8876_CmdWrite(0x39);	RA8876_DataWrite(WX>>8);
	RA8876_CmdWrite(0x3A);	RA8876_DataWrite(HY);
	RA8876_CmdWrite(0x3B);	RA8876_DataWrite(HY>>8);
}

//[3Ch]=========================================================================
void Enable_Gamma_Correction(void)	
{
/*
Gamma correction Enable
0: Disable
1: Enable
Gamma correction is the last output stage.
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x3C);
	temp = RA8876_DataRead();
	temp |= cSetb7;
	RA8876_DataWrite(temp);
}
void Disable_Gamma_Correction(void)	
{
/*
Gamma correction Enable
0: Disable
1: Enable
Gamma correction is the last output stage.
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x3C);
	temp = RA8876_DataRead();
	temp &= cClrb7;
	RA8876_DataWrite(temp);
}
void Gamma_Table_for_Blue(void)	
{
/*
Gamma table select for MPU write gamma data
00b: Gamma table for Blue
01b: Gamma table for Green
10b: Gamma table for Red
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x3C);
	temp = RA8876_DataRead();
	temp &= cClrb6;
	temp &= cClrb5;
	RA8876_DataWrite(temp);
}
void Gamma_Table_for_Green(void)	
{
/*
Gamma table select for MPU write gamma data
00b: Gamma table for Blue
01b: Gamma table for Green
10b: Gamma table for Red
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x3C);
	temp = RA8876_DataRead();
	temp &= cClrb6;
	temp |= cSetb5;
	RA8876_DataWrite(temp);
}
void Gamma_Table_for_Red(void)	
{
/*
Gamma table select for MPU write gamma data
00b: Gamma table for Blue
01b: Gamma table for Green
10b: Gamma table for Red
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x3C);
	temp = RA8876_DataRead();
	temp |= cSetb6;
	temp &= cClrb5;
	RA8876_DataWrite(temp);
}

void Enable_Graphic_Cursor(void)	
{
/*
Graphic Cursor Enable
0 : Graphic Cursor disable.
1 : Graphic Cursor enable.
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x3C);
	temp = RA8876_DataRead();
	temp |= cSetb4;
	RA8876_DataWrite(temp);
}
void Disable_Graphic_Cursor(void)	
{
/*
Graphic Cursor Enable
0 : Graphic Cursor disable.
1 : Graphic Cursor enable.
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x3C);
	temp = RA8876_DataRead();
	temp &= cClrb4;
	RA8876_DataWrite(temp);
}
//
void Select_Graphic_Cursor_1(void)	
{
/*
Graphic Cursor Selection Bit
Select one from four graphic cursor types. (00b to 11b)
00b : Graphic Cursor Set 1.
01b : Graphic Cursor Set 2.
10b : Graphic Cursor Set 3.
11b : Graphic Cursor Set 4.
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x3C);
	temp = RA8876_DataRead();
	temp &= cClrb3;
	temp &= cClrb2;
	RA8876_DataWrite(temp);
}
void Select_Graphic_Cursor_2(void)	
{
/*
Graphic Cursor Selection Bit
Select one from four graphic cursor types. (00b to 11b)
00b : Graphic Cursor Set 1.
01b : Graphic Cursor Set 2.
10b : Graphic Cursor Set 3.
11b : Graphic Cursor Set 4.
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x3C);
	temp = RA8876_DataRead();
	temp &= cClrb3;
	temp |= cSetb2;
	RA8876_DataWrite(temp);
}
void Select_Graphic_Cursor_3(void)	
{
/*
Graphic Cursor Selection Bit
Select one from four graphic cursor types. (00b to 11b)
00b : Graphic Cursor Set 1.
01b : Graphic Cursor Set 2.
10b : Graphic Cursor Set 3.
11b : Graphic Cursor Set 4.
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x3C);
	temp = RA8876_DataRead();
	temp |= cSetb3;
	temp &= cClrb2;
	RA8876_DataWrite(temp);
}
void Select_Graphic_Cursor_4(void)	
{
/*
Graphic Cursor Selection Bit
Select one from four graphic cursor types. (00b to 11b)
00b : Graphic Cursor Set 1.
01b : Graphic Cursor Set 2.
10b : Graphic Cursor Set 3.
11b : Graphic Cursor Set 4.
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x3C);
	temp = RA8876_DataRead();
	temp |= cSetb3;
	temp |= cSetb2;
	RA8876_DataWrite(temp);
}
//
void Enable_Text_Cursor(void)	
{
/*
Text Cursor Enable
0 : Disable.
1 : Enable.
Text cursor & Graphic cursor cannot enable simultaneously.
Graphic cursor has higher priority then Text cursor if enabled simultaneously.
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x3C);
	temp = RA8876_DataRead();
	temp |= cSetb1;
	RA8876_DataWrite(temp);
}
void Disable_Text_Cursor(void)	
{
/*
Text Cursor Enable
0 : Disable.
1 : Enable.
Text cursor & Graphic cursor cannot enable simultaneously.
Graphic cursor has higher priority then Text cursor if enabled simultaneously.
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x3C);
	temp = RA8876_DataRead();
	temp &= cClrb1;
	RA8876_DataWrite(temp);
}
//
void Enable_Text_Cursor_Blinking(void)	
{
/*
Text Cursor Blinking Enable
0 : Disable.
1 : Enable.
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x3C);
	temp = RA8876_DataRead();
	temp |= cSetb0;
	RA8876_DataWrite(temp);
}
void Disable_Text_Cursor_Blinking(void)	
{
/*
Text Cursor Blinking Enable
0 : Disable.
1 : Enable.
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x3C);
	temp = RA8876_DataRead();
	temp &= cClrb0;
	RA8876_DataWrite(temp);
}
//[3Dh]=========================================================================
void Blinking_Time_Frames(uint8_t temp)	
{
/*
Text Cursor Blink Time Setting (Unit: Frame)
00h : 1 frame time.
01h : 2 frames time.
02h : 3 frames time.
:
FFh : 256 frames time.
*/
	RA8876_CmdWrite(0x3D);
	RA8876_DataWrite(temp);
}
//[3Eh]=========================================================================
void Text_Cursor_H_V(uint16_t WX,uint16_t HY)
{
/*
[3Eh]
Text Cursor Horizontal Size Setting[4:0]
Unit : Pixel
Zero-based number. Value ��0�� means 1 pixel.
Note : When font is enlarged, the cursor setting will multiply the
same times as the font enlargement.
[3Fh]
Text Cursor Vertical Size Setting[4:0]
Unit : Pixel
Zero-based number. Value ��0�� means 1 pixel.
Note : When font is enlarged, the cursor setting will multiply the
same times as the font enlargement.
*/
	RA8876_CmdWrite(0x3E);
	RA8876_DataWrite(WX);
	RA8876_CmdWrite(0x3F);
	RA8876_DataWrite(HY);
}
//[40h][41h][42h][43h]=========================================================================
void Graphic_Cursor_XY(uint16_t WX,uint16_t HY)
{
/*
[40h] Graphic Cursor Horizontal Location[7:0]
[41h] Graphic Cursor Horizontal Location[12:8]
[42h] Graphic Cursor Vertical Location[7:0]
[43h] Graphic Cursor Vertical Location[12:8]
Reference main Window coordination.
*/	
	RA8876_CmdWrite(0x40);	RA8876_DataWrite(WX);
	RA8876_CmdWrite(0x41);	RA8876_DataWrite(WX>>8);
	RA8876_CmdWrite(0x42);	RA8876_DataWrite(HY);
	RA8876_CmdWrite(0x43);	RA8876_DataWrite(HY>>8);
}
//[44h]=========================================================================
void Set_Graphic_Cursor_Color_1(uint8_t temp)
{
/*
[44h] Graphic Cursor Color 0 with 256 Colors
RGB Format [7:0] = RRRGGGBB.
*/	
	LCD_RegisterWrite(0x44,temp);
}
//[45h]=========================================================================
void Set_Graphic_Cursor_Color_2(uint8_t temp)
{
/*
[45h] Graphic Cursor Color 1 with 256 Colors
RGB Format [7:0] = RRRGGGBB.
*/	
	LCD_RegisterWrite(0x45,temp);
}

/*
*********************************************************************************************************
*	�� �� ��: Canvas_Image_Start_address
*	����˵��: ���Ǿ����׵�ַ
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void Canvas_Image_Start_address(unsigned long Addr)	
{
/*
[50h] Start address of Canvas [7:0]
[51h] Start address of Canvas [15:8]
[52h] Start address of Canvas [23:16]
[53h] Start address of Canvas [31:24]
*/
	RA8876_CmdWrite(0x50);	RA8876_DataWrite(Addr);
	RA8876_CmdWrite(0x51);	RA8876_DataWrite(Addr>>8);
	RA8876_CmdWrite(0x52);	RA8876_DataWrite(Addr>>16);
	RA8876_CmdWrite(0x53);	RA8876_DataWrite(Addr>>24);
}

/*
*********************************************************************************************************
*	�� �� ��: Canvas_image_width
*	����˵��: ����ͼ����
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void Canvas_image_width(uint16_t WX)	
{
/*
[54h] Canvas image width [7:2]
[55h] Canvas image width [12:8]
*/
	RA8876_CmdWrite(0x54);	RA8876_DataWrite(WX);
	RA8876_CmdWrite(0x55);	RA8876_DataWrite(WX>>8);	
}

/*
*********************************************************************************************************
*	�� �� ��: Active_Window_XY
*	����˵��: �����XY
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void Active_Window_XY(uint16_t WX,uint16_t HY)	
{
/*
[56h] Active Window Upper-Left corner X-coordination [7:0]
[57h] Active Window Upper-Left corner X-coordination [12:8]
[58h] Active Window Upper-Left corner Y-coordination [7:0]
[59h] Active Window Upper-Left corner Y-coordination [12:8]
*/
	RA8876_CmdWrite(0x56);	RA8876_DataWrite(WX);
	RA8876_CmdWrite(0x57);	RA8876_DataWrite(WX>>8);
	RA8876_CmdWrite(0x58);	RA8876_DataWrite(HY);
	RA8876_CmdWrite(0x59);	RA8876_DataWrite(HY>>8);

}

/*
*********************************************************************************************************
*	�� �� ��: Active_Window_XY
*	����˵��: ����ڸ߶ȺͿ��
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void Active_Window_WH(uint16_t WX,uint16_t HY)	
{
/*
[5Ah] Width of Active Window [7:0]
[5Bh] Width of Active Window [12:8]
[5Ch] Height of Active Window [7:0]
[5Dh] Height of Active Window [12:8]
*/
	RA8876_CmdWrite(0x5A);	RA8876_DataWrite(WX);
	RA8876_CmdWrite(0x5B);	RA8876_DataWrite(WX>>8);
	RA8876_CmdWrite(0x5C);	RA8876_DataWrite(HY);
	RA8876_CmdWrite(0x5D);	RA8876_DataWrite(HY>>8);
}
//[5Eh]=========================================================================
/*
*********************************************************************************************************
*	�� �� ��: Memory_XY_Mode
*	����˵��: ��ģʽ
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void Memory_XY_Mode(void)	
{
/*
Canvas addressing mode
0: Block mode (X-Y coordination addressing)
1: linear mode
*/
/*
Canvas addressing mode
0: Block ģʽ (X-Y ����Ѱַ����)��
1: Linear ģʽ��	
*/
	uint8_t temp;

	RA8876_CmdWrite(0x5E);
	temp = RA8876_DataRead();
	temp &= cClrb2;
	RA8876_DataWrite(temp);
}

/*
*********************************************************************************************************
*	�� �� ��: Memory_Linear_Mode
*	����˵��: ����ģʽ
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void Memory_Linear_Mode(void)	
{
/*
Canvas addressing mode
0: Block mode (X-Y coordination addressing)
1: linear mode
*/
	uint8_t temp;

	RA8876_CmdWrite(0x5E);
	temp = RA8876_DataRead();
	temp |= cSetb2;
	RA8876_DataWrite(temp);
}
void Memory_8bpp_Mode(void)	
{
/*
Canvas image��s color depth & memory R/W data width
In Block Mode:
00: 8bpp
01: 16bpp
1x: 24bpp
In Linear Mode:
x0: 8-bits memory data read/write.
x1: 16-bits memory data read/write
*/
	uint8_t temp;

	RA8876_CmdWrite(0x5E);
	temp = RA8876_DataRead();
	temp &= cClrb1;
	temp &= cClrb0;
	RA8876_DataWrite(temp);
}

/*
*********************************************************************************************************
*	�� �� ��: Memory_16bpp_Mode
*	����˵��: ����Ϊ16λģʽ
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void Memory_16bpp_Mode(void)	
{
/*
	Color Depth of Canvas & Active Window 
bit1-0��	
	Canvas image��s color depth & memory R/W data width
	In Block Mode:
	00: 8bpp
	01: 16bpp
	1x: 24bpp
	ע : ��ɫ���ݵ����뷽��������ʹ���κ�һ��ɫ��������ʺ�
	��ͼ���ȣ�������ȷ���롣
	In Linear Mode:
	X0: 8-bits �ڴ����ݶ�д��
	X1: 16-bits �ڴ����ݶ�д��
*/
	uint8_t temp;

	RA8876_CmdWrite(0x5E);
	temp = RA8876_DataRead();
	temp &= cClrb1;
	temp |= cSetb0;
	RA8876_DataWrite(temp);
}
void Memory_24bpp_Mode(void)	
{
/*
Canvas image��s color depth & memory R/W data width
In Block Mode:
00: 8bpp
01: 16bpp
1x: 24bpp
In Linear Mode:
x0: 8-bits memory data read/write.
x1: 16-bits memory data read/write
*/
	uint8_t temp;

	RA8876_CmdWrite(0x5E);
	temp = RA8876_DataRead();
	temp |= cSetb1;
//	temp |= cSetb0;
	RA8876_DataWrite(temp);
}

/*
*********************************************************************************************************
*	�� �� ��: Goto_Pixel_XY
*	����˵��: ����ͼ�εĶ�дλ��
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void Goto_Pixel_XY(uint16_t WX,uint16_t HY)	
{
/*
Set Graphic Read/Write position

REG[5Eh] bit3, Select to read back Graphic Read/Write position.
When DPRAM Linear mode:Graphic Read/Write Position [31:24][23:16][15:8][7:0]
When DPRAM Active window mode:Graphic Read/Write 
Horizontal Position [12:8][7:0], 
Vertical Position [12:8][7:0].
Reference Canvas image coordination. Unit: Pixel
*/
	RA8876_CmdWrite(0x5F);
	RA8876_DataWrite(WX);
	RA8876_CmdWrite(0x60);
	RA8876_DataWrite(WX>>8);
	RA8876_CmdWrite(0x61);
	RA8876_DataWrite(HY);
	RA8876_CmdWrite(0x62);
	RA8876_DataWrite(HY>>8);
}
void Goto_Linear_Addr(unsigned long Addr)
{
/*
Set Graphic Read/Write position

REG[5Eh] bit3, Select to read back Graphic Read/Write position.
When DPRAM Linear mode:Graphic Read/Write Position [31:24][23:16][15:8][7:0]
When DPRAM Active window mode:Graphic Read/Write 
Horizontal Position [12:8][7:0], 
Vertical Position [12:8][7:0].
Reference Canvas image coordination. Unit: Pixel
*/
	RA8876_CmdWrite(0x5F);	RA8876_DataWrite(Addr);
	RA8876_CmdWrite(0x60);	RA8876_DataWrite(Addr>>8);
	RA8876_CmdWrite(0x61);	RA8876_DataWrite(Addr>>16);
	RA8876_CmdWrite(0x62);	RA8876_DataWrite(Addr>>24);
}

//[63h][64h][65h][66h]=========================================================================
void Goto_Text_XY(uint16_t WX,uint16_t HY)	
{
/*
Set Text Write position
Text Write X-coordination [12:8][7:0]
Text Write Y-coordination [12:8][7:0]
Reference Canvas image coordination.
Unit: Pixel
*/
	RA8876_CmdWrite(0x63);	RA8876_DataWrite(WX);
	RA8876_CmdWrite(0x64);	RA8876_DataWrite(WX>>8);
	RA8876_CmdWrite(0x65);	RA8876_DataWrite(HY);
	RA8876_CmdWrite(0x66);	RA8876_DataWrite(HY>>8);
}

//[67h]=========================================================================
/*
[bit7]Draw Line / Triangle Start Signal
Write Function
0 : Stop the drawing function.
1 : Start the drawing function.
Read Function
0 : Drawing function complete.
1 : Drawing function is processing.
[bit5]Fill function for Triangle Signal
0 : Non fill.
1 : Fill.
[bit1]Draw Triangle or Line Select Signal
0 : Draw Line
1 : Draw Triangle
*/
void Start_Line(void)
{
	RA8876_CmdWrite(0x67);
	RA8876_DataWrite(0x80);

	Check_2D_Busy();
}
void Start_Triangle(void)
{
	RA8876_CmdWrite(0x67);
	RA8876_DataWrite(0x82);//B1000_0010

	Check_2D_Busy();
}
void Start_Triangle_Fill(void)
{
	RA8876_CmdWrite(0x67);
	RA8876_DataWrite(0xA2);//B1010_0010

	Check_2D_Busy();
}
//[68h][69h][6Ah][6Bh]=========================================================================
//�u�_�I
void Line_Start_XY(uint16_t WX,uint16_t HY)
{
/*
[68h] Draw Line/Square/Triangle Start X-coordination [7:0]
[69h] Draw Line/Square/Triangle Start X-coordination [12:8]
[6Ah] Draw Line/Square/Triangle Start Y-coordination [7:0]
[6Bh] Draw Line/Square/Triangle Start Y-coordination [12:8]
*/
	RA8876_CmdWrite(0x68);
	RA8876_DataWrite(WX);

	RA8876_CmdWrite(0x69);
	RA8876_DataWrite(WX>>8);

	RA8876_CmdWrite(0x6A);
	RA8876_DataWrite(HY);

	RA8876_CmdWrite(0x6B);
	RA8876_DataWrite(HY>>8);
}
//[6Ch][6Dh][6Eh][6Fh]=========================================================================
//�u���I
void Line_End_XY(uint16_t WX,uint16_t HY)
{
/*
[6Ch] Draw Line/Square/Triangle End X-coordination [7:0]
[6Dh] Draw Line/Square/Triangle End X-coordination [12:8]
[6Eh] Draw Line/Square/Triangle End Y-coordination [7:0]
[6Fh] Draw Line/Square/Triangle End Y-coordination [12:8]
*/
	RA8876_CmdWrite(0x6C);
	RA8876_DataWrite(WX);

	RA8876_CmdWrite(0x6D);
	RA8876_DataWrite(WX>>8);

	RA8876_CmdWrite(0x6E);
	RA8876_DataWrite(HY);

	RA8876_CmdWrite(0x6F);
	RA8876_DataWrite(HY>>8);
}
//[68h]~[73h]=========================================================================
//�T��-�I1
void Triangle_Point1_XY(uint16_t WX,uint16_t HY)
{
/*
[68h] Draw Line/Square/Triangle Start X-coordination [7:0]
[69h] Draw Line/Square/Triangle Start X-coordination [12:8]
[6Ah] Draw Line/Square/Triangle Start Y-coordination [7:0]
[6Bh] Draw Line/Square/Triangle Start Y-coordination [12:8]
*/
	RA8876_CmdWrite(0x68);
	RA8876_DataWrite(WX);

	RA8876_CmdWrite(0x69);
	RA8876_DataWrite(WX>>8);

	RA8876_CmdWrite(0x6A);
	RA8876_DataWrite(HY);

	RA8876_CmdWrite(0x6B);
	RA8876_DataWrite(HY>>8);
}
//�T��-�I2
void Triangle_Point2_XY(uint16_t WX,uint16_t HY)
{
/*
[6Ch] Draw Line/Square/Triangle End X-coordination [7:0]
[6Dh] Draw Line/Square/Triangle End X-coordination [12:8]
[6Eh] Draw Line/Square/Triangle End Y-coordination [7:0]
[6Fh] Draw Line/Square/Triangle End Y-coordination [12:8]
*/
	RA8876_CmdWrite(0x6C);
	RA8876_DataWrite(WX);

	RA8876_CmdWrite(0x6D);
	RA8876_DataWrite(WX>>8);

	RA8876_CmdWrite(0x6E);
	RA8876_DataWrite(HY);

	RA8876_CmdWrite(0x6F);
	RA8876_DataWrite(HY>>8);
}
//�T��-�I3
void Triangle_Point3_XY (uint16_t WX,uint16_t HY)
{
/*
[70h] Draw Triangle Point 3 X-coordination [7:0]
[71h] Draw Triangle Point 3 X-coordination [12:8]
[72h] Draw Triangle Point 3 Y-coordination [7:0]
[73h] Draw Triangle Point 3 Y-coordination [12:8]
*/
	RA8876_CmdWrite(0x70);
	RA8876_DataWrite(WX);

	RA8876_CmdWrite(0x71);
	RA8876_DataWrite(WX>>8);

	RA8876_CmdWrite(0x72);
	RA8876_DataWrite(HY);

	RA8876_CmdWrite(0x73);
	RA8876_DataWrite(HY>>8);
}
//��_�I
void Square_Start_XY(uint16_t WX,uint16_t HY)
{
/*
[68h] Draw Line/Square/Triangle Start X-coordination [7:0]
[69h] Draw Line/Square/Triangle Start X-coordination [12:8]
[6Ah] Draw Line/Square/Triangle Start Y-coordination [7:0]
[6Bh] Draw Line/Square/Triangle Start Y-coordination [12:8]
*/
	RA8876_CmdWrite(0x68);
	RA8876_DataWrite(WX);

	RA8876_CmdWrite(0x69);
	RA8876_DataWrite(WX>>8);

	RA8876_CmdWrite(0x6A);
	RA8876_DataWrite(HY);

	RA8876_CmdWrite(0x6B);
	RA8876_DataWrite(HY>>8);
}
//����I
void Square_End_XY(uint16_t WX,uint16_t HY)
{
/*
[6Ch] Draw Line/Square/Triangle End X-coordination [7:0]
[6Dh] Draw Line/Square/Triangle End X-coordination [12:8]
[6Eh] Draw Line/Square/Triangle End Y-coordination [7:0]
[6Fh] Draw Line/Square/Triangle End Y-coordination [12:8]
*/
	RA8876_CmdWrite(0x6C);
	RA8876_DataWrite(WX);

	RA8876_CmdWrite(0x6D);
	RA8876_DataWrite(WX>>8);

	RA8876_CmdWrite(0x6E);
	RA8876_DataWrite(HY);

	RA8876_CmdWrite(0x6F);
	RA8876_DataWrite(HY>>8);
}
//[76h]=========================================================================
/*
[bit7]
Draw Circle / Ellipse / Square /Circle Square Start Signal 
Write Function
0 : Stop the drawing function.
1 : Start the drawing function.
Read Function
0 : Drawing function complete.
1 : Drawing function is processing.
[bit6]
Fill the Circle / Ellipse / Square / Circle Square Signal
0 : Non fill.
1 : fill.
[bit5 bit4]
Draw Circle / Ellipse / Square / Ellipse Curve / Circle Square Select
00 : Draw Circle / Ellipse
01 : Draw Circle / Ellipse Curve
10 : Draw Square.
11 : Draw Circle Square.
[bit1 bit0]
Draw Circle / Ellipse Curve Part Select
00 : bottom-left Ellipse Curve
01 : upper-left Ellipse Curve
10 : upper-right Ellipse Curve
11 : bottom-right Ellipse Curve
*/
void Start_Circle_or_Ellipse(void)
{
	RA8876_CmdWrite(0x76);
	RA8876_DataWrite(0x80);//B1000_XXXX

	Check_2D_Busy();
}
void Start_Circle_or_Ellipse_Fill(void)
{
	RA8876_CmdWrite(0x76);
	RA8876_DataWrite(0xC0);//B1100_XXXX

	Check_2D_Busy();
}
//
void Start_Left_Down_Curve(void)
{
	RA8876_CmdWrite(0x76);
	RA8876_DataWrite(0x90);//B1001_XX00

	Check_2D_Busy();
}
void Start_Left_Up_Curve(void)
{
	RA8876_CmdWrite(0x76);
	RA8876_DataWrite(0x91);//B1001_XX01

	Check_2D_Busy();
}
void Start_Right_Up_Curve(void)
{
	RA8876_CmdWrite(0x76);
	RA8876_DataWrite(0x92);//B1001_XX10

	Check_2D_Busy();	
}
void Start_Right_Down_Curve(void)
{
	RA8876_CmdWrite(0x76);
	RA8876_DataWrite(0x93);//B1001_XX11

	Check_2D_Busy();	
}
//
void Start_Left_Down_Curve_Fill(void)
{
	RA8876_CmdWrite(0x76);
	RA8876_DataWrite(0xD0);//B1101_XX00

	Check_2D_Busy();
}
void Start_Left_Up_Curve_Fill(void)
{
	RA8876_CmdWrite(0x76);
	RA8876_DataWrite(0xD1);//B1101_XX01

	Check_2D_Busy();
}
void Start_Right_Up_Curve_Fill(void)
{
	RA8876_CmdWrite(0x76);
	RA8876_DataWrite(0xD2);//B1101_XX10

	Check_2D_Busy();
}
void Start_Right_Down_Curve_Fill(void)
{
	RA8876_CmdWrite(0x76);
	RA8876_DataWrite(0xD3);//B1101_XX11

	Check_2D_Busy();
}
//
void Start_Square(void)
{
	RA8876_CmdWrite(0x76);
	RA8876_DataWrite(0xA0);//B1010_XXXX

	Check_2D_Busy();
}
void Start_Square_Fill(void)
{
	RA8876_CmdWrite(0x76);
	RA8876_DataWrite(0xE0);//B1110_XXXX

	Check_2D_Busy();
}
void Start_Circle_Square(void)
{
	RA8876_CmdWrite(0x76);
	RA8876_DataWrite(0xB0);//B1011_XXXX

	Check_2D_Busy();	
}
void Start_Circle_Square_Fill(void)
{
	RA8876_CmdWrite(0x76);
	RA8876_DataWrite(0xF0);//B1111_XXXX

	Check_2D_Busy();
}
//[77h]~[7Eh]=========================================================================
//��b�|
void Circle_Radius_R (uint16_t WX)
{
/*
[77h] Draw Circle/Ellipse/Circle Square Major radius [7:0]
[78h] Draw Circle/Ellipse/Circle Square Major radius [12:8]
[79h] Draw Circle/Ellipse/Circle Square Minor radius [7:0]
[7Ah] Draw Circle/Ellipse/Circle Square Minor radius [12:8]
*/
	RA8876_CmdWrite(0x77);
	RA8876_DataWrite(WX);

	RA8876_CmdWrite(0x78);
	RA8876_DataWrite(WX>>8);

	RA8876_CmdWrite(0x79);
	RA8876_DataWrite(WX);

	RA8876_CmdWrite(0x7A);
	RA8876_DataWrite(WX>>8);
}

//���b�|
void Ellipse_Radius_RxRy (uint16_t WX,uint16_t HY)
{
/*
[77h] Draw Circle/Ellipse/Circle Square Major radius [7:0]
[78h] Draw Circle/Ellipse/Circle Square Major radius [12:8]
[79h] Draw Circle/Ellipse/Circle Square Minor radius [7:0]
[7Ah] Draw Circle/Ellipse/Circle Square Minor radius [12:8]
*/
	RA8876_CmdWrite(0x77);
	RA8876_DataWrite(WX);

	RA8876_CmdWrite(0x78);
	RA8876_DataWrite(WX>>8);

	RA8876_CmdWrite(0x79);
	RA8876_DataWrite(HY);

	RA8876_CmdWrite(0x7A);
	RA8876_DataWrite(HY>>8);
}

//����ਤ�b�|
void Circle_Square_Radius_RxRy (uint16_t WX,uint16_t HY)
{
/*
[77h] Draw Circle/Ellipse/Circle Square Major radius [7:0]
[78h] Draw Circle/Ellipse/Circle Square Major radius [12:8]
[79h] Draw Circle/Ellipse/Circle Square Minor radius [7:0]
[7Ah] Draw Circle/Ellipse/Circle Square Minor radius [12:8]
*/
	RA8876_CmdWrite(0x77);
	RA8876_DataWrite(WX);

	RA8876_CmdWrite(0x78);
	RA8876_DataWrite(WX>>8);

	RA8876_CmdWrite(0x79);
	RA8876_DataWrite(HY);

	RA8876_CmdWrite(0x7A);
	RA8876_DataWrite(HY>>8);
}

//�ꤤ��
void Circle_Center_XY(uint16_t WX,uint16_t HY)
{
/*
[7Bh] Draw Circle/Ellipse/Circle Square Center X-coordination [7:0]
[7Ch] Draw Circle/Ellipse/Circle Square Center X-coordination [12:8]
[7Dh] Draw Circle/Ellipse/Circle Square Center Y-coordination [7:0]
[7Eh] Draw Circle/Ellipse/Circle Square Center Y-coordination [12:8]
*/
	RA8876_CmdWrite(0x7B);
	RA8876_DataWrite(WX);

	RA8876_CmdWrite(0x7C);
	RA8876_DataWrite(WX>>8);

	RA8876_CmdWrite(0x7D);
	RA8876_DataWrite(HY);

	RA8876_CmdWrite(0x7E);
	RA8876_DataWrite(HY>>8);
}
//��ꤤ��
void Ellipse_Center_XY(uint16_t WX,uint16_t HY)
{
/*
[7Bh] Draw Circle/Ellipse/Circle Square Center X-coordination [7:0]
[7Ch] Draw Circle/Ellipse/Circle Square Center X-coordination [12:8]
[7Dh] Draw Circle/Ellipse/Circle Square Center Y-coordination [7:0]
[7Eh] Draw Circle/Ellipse/Circle Square Center Y-coordination [12:8]
*/
	RA8876_CmdWrite(0x7B);
	RA8876_DataWrite(WX);

	RA8876_CmdWrite(0x7C);
	RA8876_DataWrite(WX>>8);

	RA8876_CmdWrite(0x7D);
	RA8876_DataWrite(HY);

	RA8876_CmdWrite(0x7E);
	RA8876_DataWrite(HY>>8);
}





/*
*********************************************************************************************************
*	�� �� ��: Set_PWM_Prescaler_1_to_256
*	����˵��: ����PWMԤ��Ƶֵ
*	��    �Σ���
*	�� �� ֵ: �������(���账��)
*********************************************************************************************************
*/
void Set_PWM_Prescaler_1_to_256(uint16_t WX)
{
/*
PWM Prescaler Register
These 8 bits determine prescaler value for Timer 0 and 1.
Time base is ��Core_Freq / (Prescaler + 1)��
*/
	RA8876_CmdWrite(0x84);
	RA8875_Delaly1us();		/* �ӳ�1us */
	RA8876_DataWrite(WX-1);
	RA8875_Delaly1us();		/* �ӳ�1us */
}
//[85h]=========================================================================
void Select_PWM1_Clock_Divided_By_1(void)
{
/*
Select MUX input for PWM Timer 1.
00 = 1; 01 = 1/2; 10 = 1/4 ; 11 = 1/8;
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x85);
	temp = RA8876_DataRead();
	temp &= cClrb7;
	temp &= cClrb6;
	RA8876_DataWrite(temp);
}
void Select_PWM1_Clock_Divided_By_2(void)
{
/*
Select MUX input for PWM Timer 1.
00 = 1; 01 = 1/2; 10 = 1/4 ; 11 = 1/8;
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x85);
	temp = RA8876_DataRead();
	temp &= cClrb7;
	temp |= cSetb6;
	RA8876_DataWrite(temp);
}
void Select_PWM1_Clock_Divided_By_4(void)
{
/*
Select MUX input for PWM Timer 1.
00 = 1; 01 = 1/2; 10 = 1/4 ; 11 = 1/8;
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x85);
	temp = RA8876_DataRead();
	temp |= cSetb7;
	temp &= cClrb6;
	RA8876_DataWrite(temp);
}
void Select_PWM1_Clock_Divided_By_8(void)
{
/*
Select MUX input for PWM Timer 1.
00 = 1; 01 = 1/2; 10 = 1/4 ; 11 = 1/8;
*/
	uint8_t temp = 0;
	
	RA8876_CmdWrite(0x85);
	RA8875_Delaly1us();		/* �ӳ�1us */
	temp = RA8876_DataRead();
	RA8875_Delaly1us();		/* �ӳ�1us */
	temp |= cSetb7;
	temp |= cSetb6;
	RA8876_DataWrite(temp);
	RA8875_Delaly1us();		/* �ӳ�1us */
	
#if 0
	//RA8876_CmdWrite(0x85);
	RA8875_Delaly1us();		/* �ӳ�1us */	
	temp = RA8876_DataRead();
	RA8875_Delaly1us();		/* �ӳ�1us */
#endif
}
void Select_PWM0_Clock_Divided_By_1(void)
{
/*
Select MUX input for PWM Timer 0.
00 = 1; 01 = 1/2; 10 = 1/4 ; 11 = 1/8;
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x85);
	temp = RA8876_DataRead();
	temp &= cClrb5;
	temp &= cClrb4;
	RA8876_DataWrite(temp);
}
void Select_PWM0_Clock_Divided_By_2(void)
{
/*
Select MUX input for PWM Timer 0.
00 = 1; 01 = 1/2; 10 = 1/4 ; 11 = 1/8;
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x85);
	temp = RA8876_DataRead();
	temp &= cClrb5;
	temp |= cSetb4;
	RA8876_DataWrite(temp);
}
void Select_PWM0_Clock_Divided_By_4(void)
{
/*
Select MUX input for PWM Timer 0.
00 = 1; 01 = 1/2; 10 = 1/4 ; 11 = 1/8;
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x85);
	temp = RA8876_DataRead();
	temp |= cSetb5;
	temp &= cClrb4;
	RA8876_DataWrite(temp);
}
void Select_PWM0_Clock_Divided_By_8(void)
{
/*
Select MUX input for PWM Timer 0.
00 = 1; 01 = 1/2; 10 = 1/4 ; 11 = 1/8;
*/
	uint8_t temp;
	
	RA8876_CmdWrite(0x85);
	temp = RA8876_DataRead();
	temp |= cSetb5;
	temp |= cSetb4;
	RA8876_DataWrite(temp);
}
//[85h].[bit3][bit2]=========================================================================
/*
XPWM[1] pin function control
0X:	XPWM[1] output system error flag (REG[00h] bit[1:0], Scan bandwidth insufficient + Memory access out of range)
10:	XPWM[1] enabled and controlled by PWM timer 1
11:	XPWM[1] output oscillator clock
//If XTEST[0] set high, then XPWM[1] will become panel scan clock input.
*/
void Select_PWM1_is_ErrorFlag(void)
{
	uint8_t temp;
	
	RA8876_CmdWrite(0x85);
	temp = RA8876_DataRead();
	temp &= cClrb3;
	RA8876_DataWrite(temp);
}

/*
*********************************************************************************************************
*	�� �� ��: Select_PWM1
*	����˵��: ���PWM������1
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void Select_PWM1(void)
{
	uint8_t temp;
	
	RA8876_CmdWrite(0x85);
	RA8875_Delaly1us();		/* �ӳ�1us */
	temp = RA8876_DataRead();
	RA8875_Delaly1us();		/* �ӳ�1us */
	temp |= cSetb3;
	temp &= cClrb2;
	RA8876_DataWrite(temp);
	RA8875_Delaly1us();		/* �ӳ�1us */
}
void Select_PWM1_is_Osc_Clock(void)
{
	uint8_t temp;
	
	RA8876_CmdWrite(0x85);
	temp = RA8876_DataRead();
	temp |= cSetb3;
	temp |= cSetb2;
	RA8876_DataWrite(temp);
}
//[85h].[bit1][bit0]=========================================================================
/*
XPWM[0] pin function control
0X: XPWM[0] becomes GPIO-C[7]
10: XPWM[0] enabled and controlled by PWM timer 0
11: XPWM[0] output core clock
*/
void Select_PWM0_is_GPIO_C7(void)
{
	uint8_t temp;
	
	RA8876_CmdWrite(0x85);
	temp = RA8876_DataRead();
	temp &= cClrb1;
	RA8876_DataWrite(temp);
}
/*
*********************************************************************************************************
*	�� �� ��: Select_PWM0
*	����˵��: ���PWM������0
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void Select_PWM0(void)
{
	uint8_t temp;
	
	RA8876_CmdWrite(0x85);
	temp = RA8876_DataRead();
	temp |= cSetb1;
	temp &= cClrb0;
	RA8876_DataWrite(temp);
}
void Select_PWM0_is_Core_Clock(void)
{
	uint8_t temp;
	
	RA8876_CmdWrite(0x85);
	temp = RA8876_DataRead();
	temp |= cSetb1;
	temp |= cSetb0;
	RA8876_DataWrite(temp);
}
//[86h]=========================================================================
//[86h]PWM1
void Enable_PWM1_Inverter(void)
{
/*
PWM Timer 1 output inverter on/off.
Determine the output inverter on/off for Timer 1. 
0 = Inverter off 
1 = Inverter on for PWM1
*/
	uint8_t temp;
	RA8876_CmdWrite(0x86);
	temp = RA8876_DataRead();
	temp |= cSetb6;
	RA8876_DataWrite(temp);
}
void Disable_PWM1_Inverter(void)
{
/*
PWM Timer 1 output inverter on/off.
Determine the output inverter on/off for Timer 1. 
0 = Inverter off 
1 = Inverter on for PWM1
*/
	uint8_t temp;
	RA8876_CmdWrite(0x86);
	temp = RA8876_DataRead();
	temp &= cClrb6;
	RA8876_DataWrite(temp);
}
void Auto_Reload_PWM1(void)
{
/*
PWM Timer 1 auto reload on/off
Determine auto reload on/off for Timer 1. 
0 = One-shot 
1 = Interval mode(auto reload)
*/
	uint8_t temp;
	RA8876_CmdWrite(0x86);
	temp = RA8876_DataRead();
	temp |= cSetb5;
	RA8876_DataWrite(temp);
}
void One_Shot_PWM1(void)
{
/*
PWM Timer 1 auto reload on/off
Determine auto reload on/off for Timer 1. 
0 = One-shot 
1 = Interval mode(auto reload)
*/
	uint8_t temp;
	RA8876_CmdWrite(0x86);
	temp = RA8876_DataRead();
	temp &= cClrb5;
	RA8876_DataWrite(temp);
}
void Start_PWM1(void)
{
/*
PWM Timer 1 start/stop
Determine start/stop for Timer 1. 
0 = Stop 
1 = Start for Timer 1
*/
	uint8_t temp;
	RA8876_CmdWrite(0x86);
	temp = RA8876_DataRead();
	temp |= cSetb4;
	RA8876_DataWrite(temp);
}
void Stop_PWM1(void)
{
/*
PWM Timer 1 start/stop
Determine start/stop for Timer 1. 
0 = Stop 
1 = Start for Timer 1
*/
	uint8_t temp;
	RA8876_CmdWrite(0x86);
	temp = RA8876_DataRead();
	temp &= cClrb4;
	RA8876_DataWrite(temp);
}
//[86h]PWM0
void Enable_PWM0_Dead_Zone(void)
{
/*
PWM Timer 0 Dead zone enable
Determine the dead zone operation. 0 = Disable. 1 = Enable.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x86);
	temp = RA8876_DataRead();
	temp |= cSetb3;
	RA8876_DataWrite(temp);
}
void Disable_PWM0_Dead_Zone(void)
{
/*
PWM Timer 0 Dead zone enable
Determine the dead zone operation. 0 = Disable. 1 = Enable.
*/
	uint8_t temp;
	RA8876_CmdWrite(0x86);
	temp = RA8876_DataRead();
	temp &= cClrb3;
	RA8876_DataWrite(temp);
}
void Enable_PWM0_Inverter(void)
{
/*
PWM Timer 0 output inverter on/off
Determine the output inverter on/off for Timer 0. 
0 = Inverter off 
1 = Inverter on for PWM0
*/
	uint8_t temp;
	RA8876_CmdWrite(0x86);
	temp = RA8876_DataRead();
	temp |= cSetb2;
	RA8876_DataWrite(temp);
}
void Disable_PWM0_Inverter(void)
{
/*
PWM Timer 0 output inverter on/off
Determine the output inverter on/off for Timer 0. 
0 = Inverter off 
1 = Inverter on for PWM0
*/
	uint8_t temp;
	RA8876_CmdWrite(0x86);
	temp = RA8876_DataRead();
	temp &= cClrb2;
	RA8876_DataWrite(temp);
}
void Auto_Reload_PWM0(void)
{
/*
PWM Timer 0 auto reload on/off
Determine auto reload on/off for Timer 0. 
0 = One-shot 
1 = Interval mode(auto reload)
*/
	uint8_t temp;
	RA8876_CmdWrite(0x86);
	temp = RA8876_DataRead();
	temp |= cSetb1;
	RA8876_DataWrite(temp);
}
void One_Shot_PWM0(void)
{
/*
PWM Timer 1 auto reload on/off
Determine auto reload on/off for Timer 1. 
0 = One-shot 
1 = Interval mode(auto reload)
*/
	uint8_t temp;
	RA8876_CmdWrite(0x86);
	temp = RA8876_DataRead();
	temp &= cClrb1;
	RA8876_DataWrite(temp);
}
/*
*********************************************************************************************************
*	�� �� ��: Start_PWM0
*	����˵��: ����PWM0
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void Start_PWM0(void)
{
/*
PWM Timer 0 start/stop
Determine start/stop for Timer 0. 
0 = Stop 
1 = Start for Timer 0
*/
	uint8_t temp;
	RA8876_CmdWrite(0x86);
	temp = RA8876_DataRead();
	temp |= cSetb0;
	RA8876_DataWrite(temp);
}
/*
*********************************************************************************************************
*	�� �� ��: Stop_PWM0
*	����˵��: ֹͣPWM0
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void Stop_PWM0(void)
{
/*
PWM Timer 0 start/stop
Determine start/stop for Timer 0. 
0 = Stop 
1 = Start for Timer 0
*/
	uint8_t temp;
	RA8876_CmdWrite(0x86);
	temp = RA8876_DataRead();
	temp &= cClrb0;
	RA8876_DataWrite(temp);
}
//[87h]=========================================================================
void Set_Timer0_Dead_Zone_Length(uint8_t temp)
{
/*
Timer 0 Dead zone length register
These 8 bits determine the dead zone length. The 1 unit time of
the dead zone length is equal to that of timer 0.
*/
	RA8876_CmdWrite(0x87);
	RA8876_DataWrite(temp);
}
/*
*********************************************************************************************************
*	�� �� ��: Set_Timer0_Compare_Buffer
*	����˵��: ���ö�ʱ��0�Ƚϻ�����ֵ
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void Set_Timer0_Compare_Buffer(uint16_t WX)
{
/*
Timer 0 compare buffer register
Compare buffer register total has 16 bits.
When timer counter equal or less than compare buffer register will cause PWM out
high level if inv_on bit is off.
*/
	RA8876_CmdWrite(0x88);
	RA8876_DataWrite(WX);
	RA8876_CmdWrite(0x89);
	RA8876_DataWrite(WX>>8);
}
/*
*********************************************************************************************************
*	�� �� ��: Set_Timer0_Count_Buffer
*	����˵��: ���ö�ʱ��0����������
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void Set_Timer0_Count_Buffer(uint16_t WX)
{
/*
Timer 0 count buffer register
Count buffer register total has 16 bits.
When timer counter equal to 0 will cause PWM timer reload Count buffer register if reload_en bit set as enable.
It may read back timer counter��s real time value when PWM timer start.
*/
	RA8876_CmdWrite(0x8A);				/* ��8λ */
	RA8876_DataWrite(WX);
	RA8876_CmdWrite(0x8B);				/* ��8λ */
	RA8876_DataWrite(WX>>8);			
}
//[8Ch][8Dh]=========================================================================
void Set_Timer1_Compare_Buffer(uint16_t WX)
{
/*
Timer 0 compare buffer register
Compare buffer register total has 16 bits.
When timer counter equal or less than compare buffer register will cause PWM out
high level if inv_on bit is off.
*/
	RA8876_CmdWrite(0x8C);
	RA8875_Delaly1us();		/* �ӳ�1us */
	RA8876_DataWrite(WX);
	RA8875_Delaly1us();		/* �ӳ�1us */
	RA8876_CmdWrite(0x8D);
	RA8875_Delaly1us();		/* �ӳ�1us */
	RA8876_DataWrite(WX>>8);
	RA8875_Delaly1us();		/* �ӳ�1us */
}
//[8Eh][8Fh]=========================================================================
void Set_Timer1_Count_Buffer(uint16_t WX)
{
/*
Timer 0 count buffer register
Count buffer register total has 16 bits.
When timer counter equal to 0 will cause PWM timer reload Count buffer register if reload_en bit set as enable.
It may read back timer counter��s real time value when PWM timer start.
*/
	RA8876_CmdWrite(0x8E);
	RA8875_Delaly1us();		/* �ӳ�1us */
	RA8876_DataWrite(WX);
	RA8875_Delaly1us();		/* �ӳ�1us */
	RA8876_CmdWrite(0x8F);
	RA8875_Delaly1us();		/* �ӳ�1us */
	RA8876_DataWrite(WX>>8);
	RA8875_Delaly1us();		/* �ӳ�1us */	
}


//[90h]~[B5h]=========================================================================

//[90h]=========================================================================
void BTE_Enable(void)
{	
/*
BTE Function Enable
0 : BTE Function disable.
1 : BTE Function enable.
*/
    uint8_t temp;
    RA8876_CmdWrite(0x90);
    temp = RA8876_DataRead();
    temp |= cSetb4 ;
	RA8876_DataWrite(temp);  
}

//[90h]=========================================================================
void BTE_Disable(void)
{	
/*
BTE Function Enable
0 : BTE Function disable.
1 : BTE Function enable.
*/
    uint8_t temp;
    RA8876_CmdWrite(0x90);
    temp = RA8876_DataRead();
    temp &= cClrb4 ;
	RA8876_DataWrite(temp);  
}

//[90h]=========================================================================
void Check_BTE_Busy(void)
{	
/*
case1: Status Register_bit3
0 : BTE function is idle.
1 : BTE function is busy.

case2: [90h]BTE Function Control Register_bit4
Read
0 : BTE function is idle.
1 : BTE function is busy.
*/
	uint8_t temp;
	
	//case1: using status register 	
	do
	{
		temp=RA8876_StatusRead();
	}while(temp&0x08);


	//case2: using BTE Function Control Register
	do
	{
	    RA8876_CmdWrite(0x90);
	    temp = RA8876_DataRead();
	}while(temp&0x10);

}
//[90h]=========================================================================
void Pattern_Format_8X8(void)
{	
/*
Pattern Format
0 : 8X8
1 : 16X16
*/
    uint8_t temp;
    RA8876_CmdWrite(0x90);
    temp = RA8876_DataRead();
    temp &= cClrb0 ;
	RA8876_DataWrite(temp);
}	
//[90h]=========================================================================
void Pattern_Format_16X16(void)
{	
/*
Pattern Format
0 : 8X8
1 : 16X16
*/
    uint8_t temp;
    RA8876_CmdWrite(0x90);
    temp = RA8876_DataRead();
    temp |= cSetb0 ;
	  RA8876_DataWrite(temp);
}	

//[91h]=========================================================================
void BTE_ROP_Code(uint8_t setx)
{	
/*
BTE ROP Code[Bit7:4]
	
0000 : 0(Blackness)
0001 : ~S0.~S1 or ~ ( S0+S1 )
0010 : ~S0.S1
0011 : ~S0
0100 : S0.~S1
0101 : ~S1
0110 : S0^S1
0111 : ~S0+~S1 or ~ ( S0.S1 )
1000 : S0.S1
1001 : ~ ( S0^S1 )
1010 : S1
1011 : ~S0+S1
1100 : S0
1101 : S0+~S1
1110 : S0+S1
1111 : 1 ( Whiteness )
*/
    uint8_t temp;
    RA8876_CmdWrite(0x91);
    temp = RA8876_DataRead();
    temp &= 0x0f ;
		temp |= (setx<<4);
	  RA8876_DataWrite(temp);
}
	
//[91h]=========================================================================
void BTE_Operation_Code(uint8_t setx)
{	
/*
BTE Operation Code[Bit3:0]
	
0000/ 0 : MPU Write BTE with ROP.
0001    : NA
0010/ 2 : Memory copy (move) BTE in positive direction with ROP.
0011    : NA
0100/ 4 : MPU Write with chroma keying (w/o ROP)
0101/ 5 : Memory copy (move) with chroma keying (w/o ROP)
0110/ 6 : Pattern Fill with ROP
0111/ 7 : Pattern Fill with chroma keying (w/o ROP)
1000/ 8 : MPU Write with Color Expansion (w/o ROP)
1001/ 9 : MPU Write with Color Expansion and chroma keying (w/o ROP)
1010/10 : Memory write with opacity (w/o ROP)
1011/11 : MPU Write with opacity (w/o ROP)
1100/12 : Solid Fill (w/o ROP)
1101    : NA
1110/14 : Memory write with Color Expansion (w/o ROP)
1111/15 : Memory write with Color Expansion and chroma keying (w/o ROP)
*/
    uint8_t temp;
    RA8876_CmdWrite(0x91);
    temp = RA8876_DataRead();
	temp &= 0xf0 ;
	temp |= setx ;
	RA8876_DataWrite(temp);

}
//[92h]=========================================================================
void BTE_S0_Color_8bpp(void)
{	
/*
S0 Color Depth
00 : 256 Color
01 : 64k Color
1x : 16M Color
*/	
    uint8_t temp;
    RA8876_CmdWrite(0x92);
    temp = RA8876_DataRead();
    temp &= cClrb6 ;
	temp &= cClrb5 ;
	RA8876_DataWrite(temp);
}	
/*
*********************************************************************************************************
*	�� �� ��: BTE_S0_Color_16bpp
*	����˵��: 92H���
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void BTE_S0_Color_16bpp(void)
{	
/*
	Source 0/1 & Destination Color Depth[92H]
bit6-5��
	S0 Color Depth
	00: 256 ɫ (8bpp)��
	01: 64k ɫ (16bpp)��
	1x: 16M ɫ (24bpp)��
*/	
    uint8_t temp;
    RA8876_CmdWrite(0x92);
    temp = RA8876_DataRead();
    temp &= cClrb6 ;
	temp |= cSetb5 ;
	RA8876_DataWrite(temp);

}	
//[92h]=========================================================================
void BTE_S0_Color_24bpp(void)
{	
/*
S0 Color Depth
00 : 256 Color
01 : 64k Color
1x : 16M Color
*/	
    uint8_t temp;
    RA8876_CmdWrite(0x92);
    temp = RA8876_DataRead();
	temp |= cSetb6 ;
	//temp |= cSetb5 ;
	RA8876_DataWrite(temp);
}
//[92h]=========================================================================
void BTE_S1_Color_8bpp(void)
{	
/*
S1 Color Depth
000 : 256 Color
001 : 64k Color
010 : 16M Color
011 : Constant Color
100 : 8 bit pixel alpha blending
101 : 16 bit pixel alpha blending
*/	
    uint8_t temp;
    RA8876_CmdWrite(0x92);
    temp = RA8876_DataRead();
    temp &= cClrb4 ;
		temp &= cClrb3 ;
	  temp &= cClrb2 ;
	  RA8876_DataWrite(temp);
}	
/*
*********************************************************************************************************
*	�� �� ��: BTE_S1_Color_16bpp
*	����˵��: 92H���
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void BTE_S1_Color_16bpp(void)
{	
/*
	Source 0/1 & Destination Color Depth[92H]
bit4-2��
	S1 Color Depth
	000: 256 ɫ (8bpp)��
	001: 64k ɫ (16bpp)��
	010: 16M ɫ (24bpp)��
	011: Constant color (S1 memory start address�� setting definition
	change as S1 constant color definition)��
	100: 8 bit pixel alpha blending��
	101: 16 bit pixel alpha blending��
*/	
    uint8_t temp;
    RA8876_CmdWrite(0x92);
    temp = RA8876_DataRead();
    temp &= cClrb4 ;
		temp &= cClrb3 ;
	  temp |= cSetb2 ;
	  RA8876_DataWrite(temp);

}
//[92h]=========================================================================
void BTE_S1_Color_24bpp(void)
{	
/*
S1 Color Depth
000 : 256 Color
001 : 64k Color
010 : 16M Color
011 : Constant Color
100 : 8 bit pixel alpha blending
101 : 16 bit pixel alpha blending
*/	
    uint8_t temp;
    RA8876_CmdWrite(0x92);
    temp = RA8876_DataRead();
	temp &= cClrb4 ;
	temp |= cSetb3 ;
	temp &= cClrb2 ;
	RA8876_DataWrite(temp);
}

//[92h]=========================================================================
void BTE_S1_Color_Constant(void)
{	
/*
S1 Color Depth
000 : 256 Color
001 : 64k Color
010 : 16M Color
011 : Constant Color
100 : 8 bit pixel alpha blending
101 : 16 bit pixel alpha blending
*/	
    uint8_t temp;
    RA8876_CmdWrite(0x92);
    temp = RA8876_DataRead();
    temp &= cClrb4 ;
		temp |= cSetb3 ;
	  temp |= cSetb2 ;
	  RA8876_DataWrite(temp);
}



//[92h]=========================================================================
void BTE_S1_Color_8bit_Alpha(void)
{	
/*
S1 Color Depth
000 : 256 Color
001 : 64k Color
010 : 16M Color
011 : Constant Color
100 : 8 bit pixel alpha blending
101 : 16 bit pixel alpha blending
*/	
    uint8_t temp;
    RA8876_CmdWrite(0x92);
    temp = RA8876_DataRead();
    temp |= cSetb4 ;
		temp &= cClrb3 ;
	  temp &= cClrb2 ;
	  RA8876_DataWrite(temp);
}

//[92h]=========================================================================
void BTE_S1_Color_16bit_Alpha(void)
{	
/*
S1 Color Depth
000 : 256 Color
001 : 64k Color
010 : 16M Color
011 : Constant Color
100 : 8 bit pixel alpha blending
101 : 16 bit pixel alpha blending
*/	
    uint8_t temp;
    RA8876_CmdWrite(0x92);
    temp = RA8876_DataRead();
    temp |= cSetb4 ;
		temp &= cClrb3 ;
	  temp |= cSetb2 ;
	  RA8876_DataWrite(temp);
}

//[92h]=========================================================================
void BTE_Destination_Color_8bpp(void)
{	
/*
	00: 256 ɫ (8bpp)��
	01: 64k ɫ (16bpp)��
	1x: 16M ɫ (24bpp)��
*/
    uint8_t temp;
    RA8876_CmdWrite(0x92);
    temp = RA8876_DataRead();
    temp &= cClrb1 ;
		temp &= cClrb0 ;
	  RA8876_DataWrite(temp);
}	
/*
*********************************************************************************************************
*	�� �� ��: BTE_Destination_Color_16bpp
*	����˵��: 92H���
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void BTE_Destination_Color_16bpp(void)
{	
/*
	Source 0/1 & Destination Color Depth[92H]
bit1-0��
	Destination Color Depth
	00: 256 ɫ (8bpp)��
	01: 64k ɫ (16bpp)��
	1x: 16M ɫ (24bpp)��
*/	
    uint8_t temp;
    RA8876_CmdWrite(0x92);
    temp = RA8876_DataRead();
    temp &= cClrb1 ;
		temp |= cSetb0 ;
	  RA8876_DataWrite(temp);

}	
//[92h]=========================================================================
void BTE_Destination_Color_24bpp(void)
{	
/*
Destination Color Depth
00 : 256 Color
10 : 64k Color
1x : 16M Color
*/	
    uint8_t temp;
    RA8876_CmdWrite(0x92);
    temp = RA8876_DataRead();
    temp |= cSetb1 ;
		//temp |= cSetb0 ;
	  RA8876_DataWrite(temp);
}


//[93h][94h][95h][96h]=========================================================================
void BTE_S0_Memory_Start_Address(unsigned long Addr)	
{
/*
[93h] BTE S0 Memory Start Address [7:0]
[94h] BTE S0 Memory Start Address [15:8]
[95h] BTE S0 Memory Start Address [23:16]
[96h] BTE S0 Memory Start Address [31:24]
Bit [1:0] tie to ��0�� internally.
*/
	LCD_RegisterWrite(0x93,Addr);
	LCD_RegisterWrite(0x94,Addr>>8);
	LCD_RegisterWrite(0x95,Addr>>16);
	LCD_RegisterWrite(0x96,Addr>>24);
}


//[97h][98h]=========================================================================
void BTE_S0_Image_Width(uint16_t WX)	
{
/*
[97h] BTE S0 Image Width [7:0]
[98h] BTE S0 Image Width [12:8]
Unit: Pixel.
Bit [1:0] tie to ��0�� internally.
*/
	LCD_RegisterWrite(0x97,WX);
	LCD_RegisterWrite(0x98,WX>>8);
}


//[99h][9Ah][9Bh][9Ch]=========================================================================
void BTE_S0_Window_Start_XY(uint16_t WX,uint16_t HY)	
{
/*
[99h] BTE S0 Window Upper-Left corner X-coordination [7:0]
[9Ah] BTE S0 Window Upper-Left corner X-coordination [12:8]
[9Bh] BTE S0 Window Upper-Left corner Y-coordination [7:0]
[9Ch] BTE S0 Window Upper-Left corner Y-coordination [12:8]
*/

	LCD_RegisterWrite(0x99,WX);
	LCD_RegisterWrite(0x9A,WX>>8);

	LCD_RegisterWrite(0x9B,HY);
	LCD_RegisterWrite(0x9C,HY>>8);
}




//[9Dh][9Eh][9Fh][A0h]=========================================================================
void BTE_S1_Memory_Start_Address(unsigned long Addr)	
{
/*
[9Dh] BTE S1 Memory Start Address [7:0]
[9Eh] BTE S1 Memory Start Address [15:8]
[9Fh] BTE S1 Memory Start Address [23:16]
[A0h] BTE S1 Memory Start Address [31:24]
Bit [1:0] tie to ��0�� internally.
*/
	LCD_RegisterWrite(0x9D,Addr);
	LCD_RegisterWrite(0x9E,Addr>>8);
	LCD_RegisterWrite(0x9F,Addr>>16);
	LCD_RegisterWrite(0xA0,Addr>>24);
}


//Input data format:R3G3B2
void S1_Constant_color_256(uint8_t temp)
{
    RA8876_CmdWrite(0x9D);
    RA8876_DataWrite(temp);

    RA8876_CmdWrite(0x9E);
    RA8876_DataWrite(temp<<3);

    RA8876_CmdWrite(0x9F);
    RA8876_DataWrite(temp<<6);
}

//Input data format:R5G6B6
void S1_Constant_color_65k(uint16_t temp)
{
    RA8876_CmdWrite(0x9D);
    RA8876_DataWrite(temp>>8);

    RA8876_CmdWrite(0x9E);
    RA8876_DataWrite(temp>>3);

    RA8876_CmdWrite(0x9F);
    RA8876_DataWrite(temp<<3);
}

//Input data format:R8G8B8
void S1_Constant_color_16M(unsigned long temp)
{
    RA8876_CmdWrite(0x9D);
    RA8876_DataWrite(temp>>16);

    RA8876_CmdWrite(0x9E);
    RA8876_DataWrite(temp>>8);

    RA8876_CmdWrite(0x9F);
    RA8876_DataWrite(temp);
}




//[A1h][A2h]=========================================================================
void BTE_S1_Image_Width(uint16_t WX)	
{
/*
[A1h] BTE S1 Image Width [7:0]
[A2h] BTE S1 Image Width [12:8]
Unit: Pixel.
Bit [1:0] tie to ��0�� internally.
*/
	LCD_RegisterWrite(0xA1,WX);
	LCD_RegisterWrite(0xA2,WX>>8);
}


//[A3h][A4h][A5h][A6h]=========================================================================
void BTE_S1_Window_Start_XY(uint16_t WX,uint16_t HY)	
{
/*
[A3h] BTE S1 Window Upper-Left corner X-coordination [7:0]
[A4h] BTE S1 Window Upper-Left corner X-coordination [12:8]
[A5h] BTE S1 Window Upper-Left corner Y-coordination [7:0]
[A6h] BTE S1 Window Upper-Left corner Y-coordination [12:8]
*/

	LCD_RegisterWrite(0xA3,WX);
	LCD_RegisterWrite(0xA4,WX>>8);

	LCD_RegisterWrite(0xA5,HY);
	LCD_RegisterWrite(0xA6,HY>>8);
}




//[A7h][A8h][A9h][AAh]=========================================================================
void BTE_Destination_Memory_Start_Address(unsigned long Addr)	
{
/*
[A7h] BTE Destination Memory Start Address [7:0]
[A8h] BTE Destination Memory Start Address [15:8]
[A9h] BTE Destination Memory Start Address [23:16]
[AAh] BTE Destination Memory Start Address [31:24]
Bit [1:0] tie to ��0�� internally.
*/
	LCD_RegisterWrite(0xA7,Addr);
	LCD_RegisterWrite(0xA8,Addr>>8);
	LCD_RegisterWrite(0xA9,Addr>>16);
	LCD_RegisterWrite(0xAA,Addr>>24);
}


//[ABh][ACh]=========================================================================
void BTE_Destination_Image_Width(uint16_t WX)	
{
/*
[ABh] BTE Destination Image Width [7:0]
[ACh] BTE Destination Image Width [12:8]
Unit: Pixel.
Bit [1:0] tie to ��0�� internally.
*/
	LCD_RegisterWrite(0xAB,WX);
	LCD_RegisterWrite(0xAC,WX>>8);
}


//[ADh][AEh][AFh][B0h]=========================================================================
void BTE_Destination_Window_Start_XY(uint16_t WX,uint16_t HY)	
{
/*
[ADh] BTE Destination Window Upper-Left corner X-coordination [7:0]
[AEh] BTE Destination Window Upper-Left corner X-coordination [12:8]
[AFh] BTE Destination Window Upper-Left corner Y-coordination [7:0]
[B0h] BTE Destination Window Upper-Left corner Y-coordination [12:8]
*/

	LCD_RegisterWrite(0xAD,WX);
	LCD_RegisterWrite(0xAE,WX>>8);

	LCD_RegisterWrite(0xAF,HY);
	LCD_RegisterWrite(0xB0,HY>>8);
}


//[B1h][B2h][B3h][B4h]===============================================================

void BTE_Window_Size(uint16_t WX, uint16_t HY)

{
/*
[B1h] BTE Window Width [7:0]
[B2h] BTE Window Width [12:8]

[B3h] BTE Window Height [7:0]
[B4h] BTE Window Height [12:8]
*/
        LCD_RegisterWrite(0xB1,WX);
        LCD_RegisterWrite(0xB2,WX>>8);
	
	    LCD_RegisterWrite(0xB3,HY);
        LCD_RegisterWrite(0xB4,HY>>8);
}

//[B5h]=========================================================================
void BTE_Alpha_Blending_Effect(uint8_t temp)
{	
/*
Window Alpha Blending effect for S0 & S1
The value of alpha in the color code ranges from 0.0 to 1.0,
where 0.0 represents a fully transparent color, and 1.0
represents a fully opaque color.
00h: 0
01h: 1/32
02h: 2/32
:
1Eh: 30/32
1Fh: 31/32
2Xh: 1
Output Effect = (S0 image x (1 - alpha setting value)) + (S1 image x alpha setting value)
*/
    RA8876_CmdWrite(0xB5);
	RA8876_DataWrite(temp);  
}


//[B6h]=========================================================================
void Start_SFI_DMA(void)
{
	uint8_t temp;
	RA8876_CmdWrite(0xB6);
	temp = RA8876_DataRead();
    temp |= cSetb0;
	RA8876_DataWrite(temp);
}

void Check_Busy_SFI_DMA(void)
{
	RA8876_CmdWrite(0xB6);
	do
	{		
	}while((RA8876_DataRead()&0x01)==0x01);
}


/*
*********************************************************************************************************
*	�� �� ��: Select_SFI_0
*	����˵��: ��������ѡ��
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void Select_SFI_0(void)
{
/*[bit7]
Serial Flash/ROM I/F # Select
0: Serial Flash/ROM 0 I/F is selected.
1: Serial Flash/ROM 1 I/F is selected.
*/
/*
	����Flash/ROM���ƼĴ���[B7H]
bit7��
	Serial Flash/ROM I/F # Select
	0: ��������/ROM 0 ��ѡ��
	1: ��������/ROM 1 ��ѡ��
*/
	uint8_t temp;
	RA8876_CmdWrite(0xB7);
	temp = RA8876_DataRead();
    temp &= cClrb7;
	RA8876_DataWrite(temp);
}
void Select_SFI_1(void)
{
/*[bit7]
Serial Flash/ROM I/F # Select
0: Serial Flash/ROM 0 I/F is selected.
1: Serial Flash/ROM 1 I/F is selected.
*/
	uint8_t temp;
	RA8876_CmdWrite(0xB7);
	temp = RA8876_DataRead();
    temp |= cSetb7;
	RA8876_DataWrite(temp);
}

/*
*********************************************************************************************************
*	�� �� ��: Select_SFI_Font_Mode
*	����˵��: ѡ����flash / ROM ����ģʽ
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void Select_SFI_Font_Mode(void)
{
/*[bit6]
Serial Flash /ROM Access Mode
0: Font mode �V for external cgrom
1: DMA mode �V for cgram , pattern , bootstart image or osd
*/
/*
	����Flash/ROM���ƼĴ���[B7H]
bit6:
	Serial Flash /ROM Access Mode
	0: �ַ�ģʽ�C ʹ���� CGROM��
	1: DMAģʽ�C ʹ���� CGRAM��pattern��boot start image �� OSD�����ϡ�
*/
	uint8_t temp;
	RA8876_CmdWrite(0xB7);
	temp = RA8876_DataRead();
    temp &= cClrb6;
	RA8876_DataWrite(temp);
}
void Select_SFI_DMA_Mode(void)
{
/*[bit6]
Serial Flash /ROM Access Mode
0: Font mode �V for external cgrom
1: DMA mode �V for cgram , pattern , bootstart image or osd
*/
	uint8_t temp;
	RA8876_CmdWrite(0xB7);
	temp = RA8876_DataRead();
    temp |= cSetb6;
	RA8876_DataWrite(temp);
}

/*
*********************************************************************************************************
*	�� �� ��: Select_SFI_24bit_Address
*	����˵��: ѡ����flashΪ24λѰַģʽ
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void Select_SFI_24bit_Address(void)
{
/*[bit5]
Serial Flash/ROM Address Mode
0: 24 bits address mode
1: 32 bits address mode
*/
/*
	����Flash/ROM���ƼĴ���[B7H]
bit5:
	Serial Flash/ROM Address Mode
		0: 24 bits Ѱַģʽ��
		1: 32 bits Ѱַģʽ��
	���ʹ����ϣ��ʹ�� 32 bits Ѱַģʽ��ʹ���߱����������� EX4B
	����(B7h) ���������棬�����趨�� bit Ϊ 1��
	ʹ����Ҳ���Լ�����λ��֪���Ƿ��ڿ�����ʾ���Ѿ����� 32bit
	��ַģʽ��
*/
	uint8_t temp;
	RA8876_CmdWrite(0xB7);
	temp = RA8876_DataRead();
    temp &= cClrb5;
	RA8876_DataWrite(temp);
}

void Select_SFI_32bit_Address(void)
{
/*[bit5]
Serial Flash/ROM Address Mode
0: 24 bits address mode
1: 32 bits address mode
*/
	uint8_t temp;
	RA8876_CmdWrite(0xB7);
	temp = RA8876_DataRead();
    temp |= cSetb5;
	RA8876_DataWrite(temp);
}

/*
*********************************************************************************************************
*	�� �� ��: Select_standard_SPI_Mode0_or_Mode3
*	����˵��: ѡ���׼SPIģʽ0��ģʽ3
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void Select_standard_SPI_Mode0_or_Mode3(void)
{
/*[bit4]
RA8875 compatible mode
0: standard SPI mode 0 or mode 3 timing
1: Follow RA8875 mode 0 & mode 3 timing
*/
/*
	����Flash/ROM���ƼĴ���[B7H]
bit4:
	RA8876 compatible mode
		0: ��׼ SPI ģʽ 0 ��ģʽ 3 ʱ��ͼ��
		1: ���� RA8876 ģʽ 0 ��ģʽ 3 timing��
	��RA8875����ģʽ�У����ݶ�ȡ��λ������Ƶ�ʵ��½�Ե
	(high->low)����������Ҳ����Ƶ���½�Ե�仯 (high->low)��
	������ʱ������ Mode 0�� SPI Ƶ��ֹͣ�� low��
	������ʱ������ Mode 3�� SPI Ƶ��ֹͣ�� high��
*/
	uint8_t temp;
	RA8876_CmdWrite(0xB7);
	temp = RA8876_DataRead();
    temp &= cClrb4;
	RA8876_DataWrite(temp);
}

void Select_RA8875_SPI_Mode0_and_Mode3(void)
{
/*[bit4]
RA8875 compatible mode
0: standard SPI mode 0 or mode 3 timing
1: Follow RA8875 mode 0 & mode 3 timing
*/
	uint8_t temp;
	RA8876_CmdWrite(0xB7);
	temp = RA8876_DataRead();
    temp |= cSetb4;
	RA8876_DataWrite(temp);
}

void Select_SFI_Single_Mode_Dummy_0T_03h(void)
{
/*
000xb: 1x read command code = 03h. Without dummy cycle between address and data.
010xb: 1x read command code = 0Bh. 8 dummy cycles inserted between address and data.
1x0xb: 1x read command code = 1Bh. 16 dummy cycles inserted between address and data.
*/
	uint8_t temp;
	RA8876_CmdWrite(0xB7);
	temp = RA8876_DataRead();
    temp &= 0xF0;
	RA8876_DataWrite(temp);
}

/*
*********************************************************************************************************
*	�� �� ��: Select_SFI_Single_Mode_Dummy_8T_0Bh
*	����˵��: ����������
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void Select_SFI_Single_Mode_Dummy_8T_0Bh(void)
{
/*
000xb: 1x read command code = 03h. Without dummy cycle between address and data.
010xb: 1x read command code = 0Bh. 8 dummy cycles inserted between address and data.
1x0xb: 1x read command code = 1Bh. 16 dummy cycles inserted between address and data.
*/
/*
	����Flash/ROM���ƼĴ���[B7H]	
bit3-0��
	Read Command code & behavior selection
	000xb: 1x ��ȡ���� 03h����ȡ�ٶ�Ϊ Normal read �ٶȡ�������
	�� xmiso ���롣�ڵ�ַ�����ݼ䲻��Ҫ�����ڡ�
	010xb: 1x ��ȡ���� 0Bh��Ϊ faster read �ٶȡ��������� xmiso
	���룬 RA8876 �ڵ�ַ�����ݼ������ 8 �������ڡ�
	1x0xb: 1x ��ȡ���� 1Bh��Ϊ fastest read �ٶȣ��������� xmiso
	���롣 RA8876 �ڵ�ַ�����ݼ������ 16 ��������
	xx10b: 2x��ȡ���� 3Bh����xmiso��xmosi���н����������룬��
	��ַ�����ݼ������ 8 �������� (Dual mode 0����ο� �D 16-7)��
	xx11b: 2x��ȡ����BBh����ַ�������������͸��xmiso��xmosi
	���룬���ҽ�Ϊ����ʽ���롣�ڵ�ַ�����ݼ���Զ����� 4 ����
	���� (Dual mode 1����ο� �D 16-8)��
	ע:: �������е� serial flash ��֧��������������ʹ�õ� serial
	flash ��ѡ����ȷ�Ķ�ȡ����
*/	
	
	uint8_t temp;
	RA8876_CmdWrite(0xB7);
	temp = RA8876_DataRead();
	temp &= 0xF0;
   temp |= cSetb2;
	RA8876_DataWrite(temp);
}

void Select_SFI_Single_Mode_Dummy_16T_1Bh(void)
{
/*
000xb: 1x read command code = 03h. Without dummy cycle between address and data.
010xb: 1x read command code = 0Bh. 8 dummy cycles inserted between address and data.
1x0xb: 1x read command code = 1Bh. 16 dummy cycles inserted between address and data.
*/
	uint8_t temp;
	RA8876_CmdWrite(0xB7);
	temp = RA8876_DataRead();
	temp &= 0xF0;
   temp |= cSetb3;
	RA8876_DataWrite(temp);
}
void Select_SFI_Dual_Mode_Dummy_8T_3Bh(void)
{
/*
xx10b: 2x read command code = 3Bh. 8 dummy cycles inserted between address and data phase. (mode 0)
xx11b: 2x read command code = BBh. 4 dummy cycles inserted between address and data phase. (mode 1)
*/
	uint8_t temp;
	RA8876_CmdWrite(0xB7);
	temp = RA8876_DataRead();
	temp &= 0xF0;
  temp |= 0x02;
	RA8876_DataWrite(temp);
}
void Select_SFI_Dual_Mode_Dummy_4T_BBh(void)
{
/*
xx10b: 2x read command code = 3Bh. 8 dummy cycles inserted between address and data phase. (mode 0)
xx11b: 2x read command code = BBh. 4 dummy cycles inserted between address and data phase. (mode 1)
*/
	uint8_t temp;
	RA8876_CmdWrite(0xB7);
	temp = RA8876_DataRead();
	temp &= 0xF0;
  temp |= 0x03;
	RA8876_DataWrite(temp);
}







//REG[B8h] SPI master Tx /Rx FIFO Data Register (SPIDR) 
uint8_t SPI_Master_FIFO_Data_Put(uint8_t Data)
{
    uint8_t temp;
	RA8876_CmdWrite(0xB8);
	RA8876_DataWrite(Data);
	while(Tx_FIFO_Empty_Flag()==0);	//�ŤF�A����U�@��
	temp = SPI_Master_FIFO_Data_Get();

	return temp;
}

uint8_t SPI_Master_FIFO_Data_Get(void)
{
   uint8_t temp;

	while(Rx_FIFO_Empty_Flag()==1);//���O�Ū����U����
	RA8876_CmdWrite(0xB8);
	temp=RA8876_DataRead();
	//while(Rx_FIFO_full_flag()); //�s��g�J16����Ƥ~�ݭn
   return temp;
}

//REG[B9h] SPI master Control Register (SPIMCR2) 
//void Mask_SPI_Master_Interrupt_Flag(void)(B9h�ק�)
void Enable_SPI_Master_Interrupt(void)
{
/*
SPI Master Interrupt enable
0: Disable interrupt.
1: Enable interrupt.
*/
  uint8_t temp;
  RA8876_CmdWrite(0xB9);
  temp = RA8876_DataRead();
  temp |= cSetb6;
  RA8876_DataWrite(temp);
} 
void Disable_SPI_Master_Interrupt(void)
{
/*
SPI Master Interrupt enable
0: Disable interrupt.
1: Enable interrupt.
*/
  uint8_t temp;
  RA8876_CmdWrite(0xB9);
  temp = RA8876_DataRead();
  temp &= cClrb6;
  RA8876_DataWrite(temp);
}

void Select_nSS_drive_on_xnsfcs0(void)
{
  uint8_t temp;
  RA8876_CmdWrite(0xB9);
  temp = RA8876_DataRead();
  temp &= cClrb5;
  RA8876_DataWrite(temp);

}

void Select_nSS_drive_on_xnsfcs1(void)
{
  uint8_t temp;
  RA8876_CmdWrite(0xB9);
  temp = RA8876_DataRead();
  temp |= cSetb5;
  RA8876_DataWrite(temp);

}

//0: inactive (nSS port will goes high) 
void nSS_Inactive(void)
{
  uint8_t temp;
  RA8876_CmdWrite(0xB9);
  temp = RA8876_DataRead();
  temp &= cClrb4;
  RA8876_DataWrite(temp);
}
//1: active (nSS port will goes low) 
void nSS_Active(void)
{
  uint8_t temp;
  RA8876_CmdWrite(0xB9);
  temp = RA8876_DataRead();
  temp |= cSetb4;
  RA8876_DataWrite(temp);
}

//Mask interrupt for FIFO overflow error [OVFIRQEN]
//void OVFIRQEN_Enable(void)(B9h�ק�)
void Mask_FIFO_overflow_error_Interrupt(void)
{
/*
Mask interrupt for FIFO overflow error [OVFIRQEN]
0: unmask
1: mask
*/
  uint8_t temp;
  RA8876_CmdWrite(0xB9);
  temp = RA8876_DataRead();
  temp |= cSetb3;
  RA8876_DataWrite(temp);
}
void Unmask_FIFO_overflow_error_Interrupt(void)
{
/*
Mask interrupt for FIFO overflow error [OVFIRQEN]
0: unmask
1: mask
*/
  uint8_t temp;
  RA8876_CmdWrite(0xB9);
  temp = RA8876_DataRead();
  temp &= cClrb3;
  RA8876_DataWrite(temp);
}


//void EMTIRQEN_Enable(void)(B9h�ק�)
void Mask_EMTIRQEN_Interrupt(void)
{
/*
Mask interrupt for while Tx FIFO empty & SPI engine/FSM idle [EMTIRQEN]
0: unmask
1: mask
*/
  uint8_t temp;
  RA8876_CmdWrite(0xB9);
  temp = RA8876_DataRead();
  temp |= cSetb2;
  RA8876_DataWrite(temp);
}
void Unmask_EMTIRQEN_Interrupt(void)
{
/*
Mask interrupt for while Tx FIFO empty & SPI engine/FSM idle [EMTIRQEN]
0: unmask
1: mask
*/
  uint8_t temp;
  RA8876_CmdWrite(0xB9);
  temp = RA8876_DataRead();
  temp &= cClrb2;
  RA8876_DataWrite(temp);
}

/*
SPI operation mode
Only support mode 0 & mode 3, when enable serial flash��s DMA
or access Getop��s character serial ROM device.
mode / CPOL:Clock Polarity bit / CPHA:Clock Phase bit
	0	0	0
	1	0	1
	2	1	0
	3	1	1
*/
/*
*********************************************************************************************************
*	�� �� ��: Reset_CPOL
*	����˵��: ��CPOLʱ�Ӽ���λ��0
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
/*
	SPI�����ƼĴ���[B9H]
bit1-0��
	SPI operation mode
	������ DMA ���ⲿ CGROM ʱ�� SPI ֻ֧�� mode 0 �� mode 3��
	mode   CPOL:Clock Polarity bit  CPHA:Clock Phase bit
	0             0                      0
	1             0                      1
	2             1                      0
	3             1                      1
*/
void Reset_CPOL(void)
{
  uint8_t temp;
  RA8876_CmdWrite(0xB9);
  temp = RA8876_DataRead();
  temp &= cClrb1;
  RA8876_DataWrite(temp);
}

void Set_CPOL(void)
{
  uint8_t temp;
  RA8876_CmdWrite(0xB9);
  temp = RA8876_DataRead();
  temp |= cSetb1;
  RA8876_DataWrite(temp);
}

/*
*********************************************************************************************************
*	�� �� ��: Reset_CPHA
*	����˵��: ��CPHAʱ�Ӽ���λ��0
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
/*
	SPI�����ƼĴ���[B9H]
bit1-0��
	SPI operation mode
	������ DMA ���ⲿ CGROM ʱ�� SPI ֻ֧�� mode 0 �� mode 3��
	mode   CPOL:Clock Polarity bit  CPHA:Clock Phase bit
	0             0                      0
	1             0                      1
	2             1                      0
	3             1                      1
*/
void Reset_CPHA(void)
{
  uint8_t temp;
  RA8876_CmdWrite(0xB9);
  temp = RA8876_DataRead();
  temp &= cClrb0;
  RA8876_DataWrite(temp);
}

void Set_CPHA(void)
{
  uint8_t temp;
  RA8876_CmdWrite(0xB9);
  temp = RA8876_DataRead();
  temp |= cSetb0;
  RA8876_DataWrite(temp);
}







//REG[BAh] SPI master Status Register (SPIMSR) 
uint8_t Tx_FIFO_Empty_Flag(void)
{
  RA8876_CmdWrite(0xBA);
  if((RA8876_DataRead()&0x80)==0x80)
  return 1;
  else
  return 0;
}

uint8_t Tx_FIFO_Full_Flag(void)
{
  RA8876_CmdWrite(0xBA);
  if((RA8876_DataRead()&0x40)==0x40)
  return 1;
  else
  return 0;
} 

uint8_t Rx_FIFO_Empty_Flag(void)
{
  RA8876_CmdWrite(0xBA);
  if((RA8876_DataRead()&0x20)==0x20)
  return 1;
  else
  return 0;
} 

uint8_t Rx_FIFO_full_flag(void)
{
   RA8876_CmdWrite(0xBA);
   if((RA8876_DataRead()&0x10)==0x10)
   return 1;
   else
   return 0;
} 

uint8_t OVFI_Flag(void)
{
   RA8876_CmdWrite(0xBA);
   if((RA8876_DataRead()&0x08)==0x08)
   return 1;
   else
   return 0;
}

void Clear_OVFI_Flag(void)
{
   uint8_t temp;
   RA8876_CmdWrite(0xBA);
   temp = RA8876_DataRead();
   temp |= cSetb3;
   RA8876_DataWrite(temp);
}

uint8_t EMTI_Flag(void)
{
   RA8876_CmdWrite(0xBA);
   if((RA8876_DataRead()&0x04)==0x04)
   return 1;
   else
   return 0;
}

void Clear_EMTI_Flag(void)
{
   uint8_t temp;
   RA8876_CmdWrite(0xBA);
   temp = RA8876_DataRead();
   temp |= cSetb2;
   RA8876_DataWrite(temp);
}

/*
*********************************************************************************************************
*	�� �� ��: SPI_Clock_Period
*	����˵��: SPIʱ������
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
//REG[BB] SPI Clock period (SPIDIV) 
void SPI_Clock_Period(uint8_t temp)
{
/*
SPI_clock= CORE_FREQ / ((Divisor+1)x2)
*/
/*
	SPIʱ������[BBH]
bit7-0:
	SPI Clock period
	�ο�ϵͳƵ�ʼ� SPI װ����Ҫ��Ƶ�����趨��ȷ���ڡ�
	Fsck = Fcore/(divisor + 1) * 2
*/
   RA8876_CmdWrite(0xBB);
   RA8876_DataWrite(temp);
} 

/*
*********************************************************************************************************
*	�� �� ��: SFI_DMA_Source_Start_Address
*	����˵��: ����Flash DMAԴ��ʼ��ַ
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
//[BCh][BDh][BEh][BFh]=========================================================================
void SFI_DMA_Source_Start_Address(unsigned long Addr)
{
/*
DMA Source START ADDRESS
This bits index serial flash address [7:0][15:8][23:16][31:24]
*/
	RA8876_CmdWrite(0xBC);
	RA8876_DataWrite(Addr);
	RA8876_CmdWrite(0xBD);
	RA8876_DataWrite(Addr>>8);
	RA8876_CmdWrite(0xBE);
	RA8876_DataWrite(Addr>>16);
	RA8876_CmdWrite(0xBF);
	RA8876_DataWrite(Addr>>24);
}
//[C0h][C1h][C2h][C3h]=========================================================================
void SFI_DMA_Destination_Start_Address(unsigned long Addr)
{
/*
DMA Destination START ADDRESS 
[1:0]Fix at 0
This bits index SDRAM address [7:0][15:8][23:16][31:24]
*/
	RA8876_CmdWrite(0xC0);
	RA8876_DataWrite(Addr);
	RA8876_CmdWrite(0xC1);
	RA8876_DataWrite(Addr>>8);
	RA8876_CmdWrite(0xC2);
	RA8876_DataWrite(Addr>>16);
	RA8876_CmdWrite(0xC3);
	RA8876_DataWrite(Addr>>24);
}
//[C0h][C1h][C2h][C3h]=========================================================================
void SFI_DMA_Destination_Upper_Left_Corner(uint16_t WX,uint16_t HY)
{
/*
C0h
This register defines DMA Destination Window Upper-Left corner 
X-coordination [7:0] on Canvas area. 
When REG DMACR bit 1 = 1 (Block Mode) 
This register defines Destination address [7:2] in SDRAM. 
C1h
When REG DMACR bit 1 = 0 (Linear Mode) 
This register defines DMA Destination Window Upper-Left corner 
X-coordination [12:8] on Canvas area. 
When REG DMACR bit 1 = 1 (Block Mode) 
This register defines Destination address [15:8] in SDRAM.
C2h
When REG DMACR bit 1 = 0 (Linear Mode) 
This register defines DMA Destination Window Upper-Left corner
Y-coordination [7:0] on Canvas area. 
When REG DMACR bit 1 = 1 (Block Mode) 
This register defines Destination address [23:16] in SDRAM. 
C3h
When REG DMACR bit 1 = 0 (Linear Mode) 
This register defines DMA Destination Window Upper-Left corner 
Y-coordination [12:8] on Canvas area. 
When REG DMACR bit 1 = 1 (Block Mode) 
This register defines Destination address [31:24] in SDRAM. 
*/
/*
	DMAĿ�Ĵ������Ͻ�����[C0h][C1h][C2h][C3h]
*/
 RA8876_CmdWrite(0xC0);
 RA8876_DataWrite(WX);
 RA8876_CmdWrite(0xC1);
 RA8876_DataWrite(WX>>8);
 
 RA8876_CmdWrite(0xC2);
 RA8876_DataWrite(HY);
 RA8876_CmdWrite(0xC3);
 RA8876_DataWrite(HY>>8);
}



//[C6h][C7h][C8h][C9h]=========================================================================
void SFI_DMA_Transfer_Number(unsigned long Addr)
{
/*
When REG DMACR bit 1 = 0 (Linear Mode)
DMA Transfer Number [7:0][15:8][23:16][31:24]

When REG DMACR bit 1 = 1 (Block Mode)
DMA Block Width [7:0][15:8]
DMA Block HIGH[7:0][15:8]
*/
	RA8876_CmdWrite(0xC6);
	RA8876_DataWrite(Addr);
	RA8876_CmdWrite(0xC7);
	RA8876_DataWrite(Addr>>8);
	RA8876_CmdWrite(0xC8);
	RA8876_DataWrite(Addr>>16);
	RA8876_CmdWrite(0xC9);
	RA8876_DataWrite(Addr>>24);
}
void SFI_DMA_Transfer_Width_Height(uint16_t WX,uint16_t HY)
{
/*
When REG DMACR bit 1 = 0 (Linear Mode)
DMA Transfer Number [7:0][15:8][23:16][31:24]

When REG DMACR bit 1 = 1 (Block Mode)
DMA Block Width [7:0][15:8]
DMA Block HIGH[7:0][15:8]
*/
	RA8876_CmdWrite(0xC6);
	RA8876_DataWrite(WX);
	RA8876_CmdWrite(0xC7);
	RA8876_DataWrite(WX>>8);

	RA8876_CmdWrite(0xC8);
	RA8876_DataWrite(HY);
	RA8876_CmdWrite(0xC9);
	RA8876_DataWrite(HY>>8);
}
//[CAh][CBh]=========================================================================
void SFI_DMA_Source_Width(uint16_t WX)
{
/*
DMA Source Picture Width [7:0][12:8]
Unit: pixel
*/
	RA8876_CmdWrite(0xCA);
	RA8876_DataWrite(WX);
	RA8876_CmdWrite(0xCB);
	RA8876_DataWrite(WX>>8);
}

//[CCh]=========================================================================

void Font_Select_UserDefine_Mode(void)
{
/*[bit7-6]
User-defined Font /CGROM Font Selection Bit in Text Mode
00 : Internal CGROM
01 : Genitop serial flash
10 : User-defined Font
*/
	uint8_t temp;
	RA8876_CmdWrite(0xCC);
	temp = RA8876_DataRead();
    temp |= cSetb7;
	temp &= cClrb6;
	RA8876_DataWrite(temp);
}
void CGROM_Select_Internal_CGROM(void)
{
/*[bit7-6]
User-defined Font /CGROM Font Selection Bit in Text Mode
00 : Internal CGROM
01 : Genitop serial flash
10 : User-defined Font
*/
	uint8_t temp;
	RA8876_CmdWrite(0xCC);
	temp = RA8876_DataRead();
	temp &= cClrb7;
    temp &= cClrb6;
	RA8876_DataWrite(temp);
}

/*
*********************************************************************************************************
*	�� �� ��: CGROM_Select_Genitop_FontROM
*	����˵��: �ַ����ƼĴ���
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void CGROM_Select_Genitop_FontROM(void)
{
/*[bit7-6]
User-defined Font /CGROM Font Selection Bit in Text Mode
00 : Internal CGROM
01 : Genitop serial flash
10 : User-defined Font
*/
/*
	�ַ����ƼĴ���[CCH]
bit7-6��
	Character source selection
	00: �ڲ� CGROM Ϊ�ַ���Դ��
	01: �ⲿ CGROM Ϊ�ַ���Դ (��ͨ����)��
	10: ʹ���߶����ַ���
	11: NA
*/
	uint8_t temp;
	RA8876_CmdWrite(0xCC);
	temp = RA8876_DataRead();
	temp &= cClrb7;
    temp |= cSetb6;
	RA8876_DataWrite(temp);
}
void Font_Select_8x16_16x16(void)
{
/*[bit5-4]
Font Height Setting
00b : 8x16 / 16x16.
01b : 12x24 / 24x24.
10b : 16x32 / 32x32.
*** User-defined Font width is decided by font code. Genitop
serial flash��s font width is decided by font code or GT Font ROM
control register.
*/
	uint8_t temp;
	RA8876_CmdWrite(0xCC);
	temp = RA8876_DataRead();
    temp &= cClrb5;
    temp &= cClrb4;
	RA8876_DataWrite(temp);
}
void Font_Select_12x24_24x24(void)
{
/*[bit5-4]
Font Height Setting
00b : 8x16 / 16x16.
01b : 12x24 / 24x24.
10b : 16x32 / 32x32.
*** User-defined Font width is decided by font code. Genitop
serial flash��s font width is decided by font code or GT Font ROM
control register.
*/
	uint8_t temp;
	RA8876_CmdWrite(0xCC);
	temp = RA8876_DataRead();
    temp &= cClrb5;
    temp |= cSetb4;
	RA8876_DataWrite(temp);
}
void Font_Select_16x32_32x32(void)
{
/*[bit5-4]
Font Height Setting
00b : 8x16 / 16x16.
01b : 12x24 / 24x24.
10b : 16x32 / 32x32.
*** User-defined Font width is decided by font code. Genitop
serial flash��s font width is decided by font code or GT Font ROM
control register.
*/
	uint8_t temp;
	RA8876_CmdWrite(0xCC);
	temp = RA8876_DataRead();
    temp |= cSetb5;
    temp &= cClrb4;
	RA8876_DataWrite(temp);
}
void Internal_CGROM_Select_ISOIEC8859_1(void)
{
/*
Font Selection for internal CGROM
When FNCR0 B7 = 0 and B5 = 0, Internal CGROM supports the
8x16 character sets with the standard coding of ISO/IEC 8859-1,2,4,5,
which supports English and most of European country languages.
00b : ISO/IEC 8859-1.
01b : ISO/IEC 8859-2.
10b : ISO/IEC 8859-4.
11b : ISO/IEC 8859-5.
*/
	uint8_t temp;
	RA8876_CmdWrite(0xCC);
	temp = RA8876_DataRead();
    temp &= cClrb1;
    temp &= cClrb0;
	RA8876_DataWrite(temp);
}
void Internal_CGROM_Select_ISOIEC8859_2(void)
{
/*
Font Selection for internal CGROM
When FNCR0 B7 = 0 and B5 = 0, Internal CGROM supports the
8x16 character sets with the standard coding of ISO/IEC 8859-1,2,4,5,
which supports English and most of European country languages.
00b : ISO/IEC 8859-1.
01b : ISO/IEC 8859-2.
10b : ISO/IEC 8859-4.
11b : ISO/IEC 8859-5.
*/
	uint8_t temp;
	RA8876_CmdWrite(0xCC);
	temp = RA8876_DataRead();
    temp &= cClrb1;
    temp |= cSetb0;
	RA8876_DataWrite(temp);
}
void Internal_CGROM_Select_ISOIEC8859_4(void)
{
/*
Font Selection for internal CGROM
When FNCR0 B7 = 0 and B5 = 0, Internal CGROM supports the
8x16 character sets with the standard coding of ISO/IEC 8859-1,2,4,5, 
which supports English and most of European country languages.
00b : ISO/IEC 8859-1.
01b : ISO/IEC 8859-2.
10b : ISO/IEC 8859-4.
11b : ISO/IEC 8859-5.
*/
	uint8_t temp;
	RA8876_CmdWrite(0xCC);
	temp = RA8876_DataRead();
    temp |= cSetb1;
    temp &= cClrb0;
	RA8876_DataWrite(temp);
}
void Internal_CGROM_Select_ISOIEC8859_5(void)
{
/*
Font Selection for internal CGROM
When FNCR0 B7 = 0 and B5 = 0, Internal CGROM supports the
8x16 character sets with the standard coding of ISO/IEC 8859-1,2,4,5, 
which supports English and most of European country languages.
00b : ISO/IEC 8859-1.
01b : ISO/IEC 8859-2.
10b : ISO/IEC 8859-4.
11b : ISO/IEC 8859-5.
*/
	uint8_t temp;
	RA8876_CmdWrite(0xCC);
	temp = RA8876_DataRead();
    temp |= cSetb1;
    temp |= cSetb0;
	RA8876_DataWrite(temp);
}
//[CDh]=========================================================================
void Enable_Font_Alignment(void)
{
/*
Full Alignment Selection Bit
0 : Full alignment disable.
1 : Full alignment enable.
*/
	uint8_t temp;
	RA8876_CmdWrite(0xCD);
	temp = RA8876_DataRead();
    temp |= cSetb7;
	RA8876_DataWrite(temp);
}
void Disable_Font_Alignment(void)
{
/*
Full Alignment Selection Bit
0 : Full alignment disable.
1 : Full alignment enable.
*/
	uint8_t temp;
	RA8876_CmdWrite(0xCD);
	temp = RA8876_DataRead();
    temp &= cClrb7;
	RA8876_DataWrite(temp);
}
void Font_Background_select_Original_Canvas(void)
{
/*		
Font Transparency
0 : Character�s background displayed with specified color.
1 : Character�s background displayed with original canvas background.
*/
	uint8_t temp;
	RA8876_CmdWrite(0xCD);
	temp = RA8876_DataRead();
    temp |= cSetb6;
	RA8876_DataWrite(temp);
}
void Font_Background_select_Color(void)
{
/*
Font Transparency
0 : Character�s background displayed with specified color.
1 : Character�s background displayed with original canvas background.
*/
	uint8_t temp;
	RA8876_CmdWrite(0xCD);
	temp = RA8876_DataRead();
    temp &= cClrb6;
	RA8876_DataWrite(temp);
}
void Font_0_degree(void)
{
/*
Font Rotation
0 : Normal
Text direction from left to right then from top to bottom
1 : Counterclockwise 90 degree & horizontal flip
Text direction from top to bottom then from left to right
(it should accommodate with set VDIR as 1)
This attribute can be changed only when previous font write
finished (core_busy = 0)
*/
	uint8_t temp;
	RA8876_CmdWrite(0xCD);
	temp = RA8876_DataRead();
    temp &= cClrb4;
	RA8876_DataWrite(temp);
}
void Font_90_degree(void)
{
/*
Font Rotation
0 : Normal
Text direction from left to right then from top to bottom
1 : Counterclockwise 90 degree & horizontal flip
Text direction from top to bottom then from left to right
(it should accommodate with set VDIR as 1)
This attribute can be changed only when previous font write
finished (core_busy = 0)
*/
	uint8_t temp;
	RA8876_CmdWrite(0xCD);
	temp = RA8876_DataRead();
    temp |= cSetb4;
	RA8876_DataWrite(temp);
}
void Font_Width_X1(void)
{
/*
Horizontal Font Enlargement
00b : X1.
01b : X2.
10b : X3.
11b : X4.
*/
	uint8_t temp;
	RA8876_CmdWrite(0xCD);
	temp = RA8876_DataRead();
    temp &= cClrb3;
    temp &= cClrb2;
	RA8876_DataWrite(temp);
}
void Font_Width_X2(void)
{
/*
Horizontal Font Enlargement
00b : X1.
01b : X2.
10b : X3.
11b : X4.
*/
	uint8_t temp;
	RA8876_CmdWrite(0xCD);
	temp = RA8876_DataRead();
    temp &= cClrb3;
    temp |= cSetb2;
	RA8876_DataWrite(temp);
}
void Font_Width_X3(void)
{
/*
Horizontal Font Enlargement
00b : X1.
01b : X2.
10b : X3.
11b : X4.
*/
	uint8_t temp;
	RA8876_CmdWrite(0xCD);
	temp = RA8876_DataRead();
    temp |= cSetb3;
    temp &= cClrb2;
	RA8876_DataWrite(temp);
}
void Font_Width_X4(void)
{
/*
Horizontal Font Enlargement
00b : X1.
01b : X2.
10b : X3.
11b : X4.
*/
	uint8_t temp;
	RA8876_CmdWrite(0xCD);
	temp = RA8876_DataRead();
    temp |= cSetb3;
    temp |= cSetb2;
	RA8876_DataWrite(temp);
}
void Font_Height_X1(void)
{
/*
Vertical Font Enlargement
00b : X1.
01b : X2.
10b : X3.
11b : X4.
*/
	uint8_t temp;
	RA8876_CmdWrite(0xCD);
	temp = RA8876_DataRead();
    temp &= cClrb1;
    temp &= cClrb0;
	RA8876_DataWrite(temp);
}
void Font_Height_X2(void)
{
/*
Vertical Font Enlargement
00b : X1.
01b : X2.
10b : X3.
11b : X4.
*/
	uint8_t temp;
	RA8876_CmdWrite(0xCD);
	temp = RA8876_DataRead();
    temp &= cClrb1;
    temp |= cSetb0;
	RA8876_DataWrite(temp);
}
void Font_Height_X3(void)
{
/*
Vertical Font Enlargement
00b : X1.
01b : X2.
10b : X3.
11b : X4.
*/
	uint8_t temp;
	RA8876_CmdWrite(0xCD);
	temp = RA8876_DataRead();
    temp |= cSetb1;
    temp &= cClrb0;
	RA8876_DataWrite(temp);
}
void Font_Height_X4(void)
{
/*
Vertical Font Enlargement
00b : X1.
01b : X2.
10b : X3.
11b : X4.
*/
	uint8_t temp;
	RA8876_CmdWrite(0xCD);
	temp = RA8876_DataRead();
    temp |= cSetb1;
    temp |= cSetb0;
	RA8876_DataWrite(temp);
}

//[CEh]=========================================================================
void GTFont_Select_GT21L16T1W(void)
{
/*
GT Serial Font ROM Select
000b: GT21L16T1W
001b: GT30L16U2W
010b: GT30L24T3Y
011b: GT30L24M1Z
100b: GT30L32S4W
101b: GT20L24F6Y
110b: GT21L24S1W
*/
	uint8_t temp;
	RA8876_CmdWrite(0xCE);
	temp = RA8876_DataRead();
    temp &= cClrb7;
    temp &= cClrb6;
    temp &= cClrb5;
	RA8876_DataWrite(temp);
}
void GTFont_Select_GT30L16U2W(void)
{
/*
GT Serial Font ROM Select
000b: GT21L16T1W
001b: GT30L16U2W
010b: GT30L24T3Y
011b: GT30L24M1Z
100b: GT30L32S4W
101b: GT20L24F6Y
110b: GT21L24S1W
*/
	uint8_t temp;
	RA8876_CmdWrite(0xCE);
	temp = RA8876_DataRead();
    temp &= cClrb7;
    temp &= cClrb6;
    temp |= cSetb5;
	RA8876_DataWrite(temp);
}
void GTFont_Select_GT30L24T3Y(void)
{
/*
GT Serial Font ROM Select
000b: GT21L16T1W
001b: GT30L16U2W
010b: GT30L24T3Y
011b: GT30L24M1Z
100b: GT30L32S4W
101b: GT20L24F6Y
110b: GT21L24S1W
*/
	uint8_t temp;
	RA8876_CmdWrite(0xCE);
	temp = RA8876_DataRead();
    temp &= cClrb7;
    temp |= cSetb6;
    temp &= cClrb5;
	RA8876_DataWrite(temp);
}
void GTFont_Select_GT30L24M1Z(void)
{
/*
GT Serial Font ROM Select
000b: GT21L16T1W
001b: GT30L16U2W
010b: GT30L24T3Y
011b: GT30L24M1Z
100b: GT30L32S4W
101b: GT20L24F6Y
110b: GT21L24S1W
*/
	uint8_t temp;
	RA8876_CmdWrite(0xCE);
	temp = RA8876_DataRead();
    temp &= cClrb7;
    temp |= cSetb6;
    temp |= cSetb5;
	RA8876_DataWrite(temp);
}
void GTFont_Select_GT30L32S4W(void)
{
/*
GT Serial Font ROM Select
000b: GT21L16T1W
001b: GT30L16U2W
010b: GT30L24T3Y
011b: GT30L24M1Z
100b: GT30L32S4W
101b: GT20L24F6Y
110b: GT21L24S1W
*/
	uint8_t temp;
	RA8876_CmdWrite(0xCE);
	temp = RA8876_DataRead();
    temp |= cSetb7;
    temp &= cClrb6;
    temp &= cClrb5;
	RA8876_DataWrite(temp);
}
void GTFont_Select_GT20L24F6Y(void)
{
/*
GT Serial Font ROM Select
000b: GT21L16T1W
001b: GT30L16U2W
010b: GT30L24T3Y
011b: GT30L24M1Z
100b: GT30L32S4W
101b: GT20L24F6Y
110b: GT21L24S1W
*/
	uint8_t temp;
	RA8876_CmdWrite(0xCE);
	temp = RA8876_DataRead();
    temp |= cSetb7;
    temp &= cClrb6;
    temp |= cSetb5;
	RA8876_DataWrite(temp);
}
void GTFont_Select_GT21L24S1W(void)
{
/*
GT Serial Font ROM Select
000b: GT21L16T1W
001b: GT30L16U2W
010b: GT30L24T3Y
011b: GT30L24M1Z
100b: GT30L32S4W
101b: GT20L24F6Y
110b: GT21L24S1W
*/
	uint8_t temp;
	RA8876_CmdWrite(0xCE);
	temp = RA8876_DataRead();
    temp |= cSetb7;
    temp |= cSetb6;
    temp &= cClrb5;
	RA8876_DataWrite(temp);
}


/*
*********************************************************************************************************
*	�� �� ��: Set_GTFont_Decoder
*	����˵��: ��ͨ�ַ�����
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void Set_GTFont_Decoder(uint8_t temp)
{
/*
[bit7-3]
FONT ROM Coding Setting
For specific GT serial Font ROM, the coding method must be set for decoding.
00000b: GB2312
00001b: GB12345/GB18030
00010b: BIG5
00011b: UNICODE
00100b: ASCII
00101b: UNI-Japanese
00110b: JIS0208
00111b: Latin/Greek/ Cyrillic / Arabic/Thai/Hebrew
01000b: Korea
10001b: ISO-8859-1
10010b: ISO-8859-2
10011b: ISO-8859-3
10100b: ISO-8859-4
10101b: ISO-8859-5
10110b: ISO-8859-6
10111b: ISO-8859-7
11000b: ISO-8859-8
11001b: ISO-8859-9
11010b: ISO-8859-10
11011b: ISO-8859-11
11100b: ISO-8859-12
11101b: ISO-8859-13
11110b: ISO-8859-14
11111b: ISO-8859-15

[bit1-0]
ASCII / Latin/Greek/ Cyrillic / Arabic
		(ASCII)		(Latin/Greek/Cyrillic)	(Arabic)
00b		Normal			Normal 					NA
01b		Arial 		Variable Width 			Presentation Forms-A
10b		Roman 			NA 					Presentation Forms-B
11b		Bold			NA 						NA
*/
/* 
	GT�ַ�ROM���ƼĴ���
bit7-3��
	Character sets
	����ָ���ļ�ͨ CGROM�����뷽ʽ�����뷽ʽ�����Ƕ�Ӧ�ġ�
bit1-0��
	�ַ�����趨
*/
	RA8876_CmdWrite(0xCF);
	RA8876_DataWrite(temp);
}
//[D0h]=========================================================================
void Font_Line_Distance(uint8_t temp)
{
/*[bit4-0]
Font Line Distance Setting
Setting the font character line distance when setting memory font
write cursor auto move. (Unit: pixel)
*/
	RA8876_CmdWrite(0xD0);
	RA8876_DataWrite(temp);
}
//[D1h]=========================================================================
void Set_Font_to_Font_Width(uint8_t temp)
{
/*[bit5-0]
Font to Font Width Setting (Unit: pixel)
*/
	RA8876_CmdWrite(0xD1);
	RA8876_DataWrite(temp);
}
//[D2h]~[D4h]=========================================================================
void Foreground_RGB(uint8_t RED,uint8_t GREEN,uint8_t BLUE)
{
/*
[D2h] Foreground Color - Red, for draw, text or color expansion
[D3h] Foreground Color - Green, for draw, text or color expansion
[D4h] Foreground Color - Blue, for draw, text or color expansion
*/  
    RA8876_CmdWrite(0xD2);
 RA8876_DataWrite(RED);
 
    RA8876_CmdWrite(0xD3);
 RA8876_DataWrite(GREEN);
  
    RA8876_CmdWrite(0xD4);
 RA8876_DataWrite(BLUE);
}

//Input data format:R3G3B2  
void Foreground_color_256(uint8_t temp) 
{
    RA8876_CmdWrite(0xD2);
 RA8876_DataWrite(temp);
 
    RA8876_CmdWrite(0xD3);
 RA8876_DataWrite(temp<<3);
  
    RA8876_CmdWrite(0xD4);
 RA8876_DataWrite(temp<<6);
}
 
//Input data format:R5G6B5 
void Foreground_color_65k(uint16_t temp)
{
    RA8876_CmdWrite(0xD2);
 RA8876_DataWrite(temp>>8);
 
    RA8876_CmdWrite(0xD3);
 RA8876_DataWrite(temp>>3);
  
    RA8876_CmdWrite(0xD4);
 RA8876_DataWrite(temp<<3);
}
 
//Input data format:R8G8B8 
void Foreground_color_16M(unsigned long temp)
{
    RA8876_CmdWrite(0xD2);
 RA8876_DataWrite(temp>>16);
 
    RA8876_CmdWrite(0xD3);
 RA8876_DataWrite(temp>>8);
  
    RA8876_CmdWrite(0xD4);
 RA8876_DataWrite(temp);
}
 
 
 
//[D5h]~[D7h]=========================================================================
/*
[D5h] Background Color - Red, for Text or color expansion
[D6h] Background Color - Green, for Text or color expansion
[D7h] Background Color - Blue, for Text or color expansion
*/ 
void Background_RGB(uint8_t RED,uint8_t GREEN,uint8_t BLUE)
{
   
    RA8876_CmdWrite(0xD5);
 RA8876_DataWrite(RED);
  
    RA8876_CmdWrite(0xD6);
 RA8876_DataWrite(GREEN);
   
    RA8876_CmdWrite(0xD7);
 RA8876_DataWrite(BLUE);
}
 
//Input data format:R3G3B2
void Background_color_256(uint8_t temp)
{
    RA8876_CmdWrite(0xD5);
 RA8876_DataWrite(temp);
  
    RA8876_CmdWrite(0xD6);
 RA8876_DataWrite(temp<<3);
   
    RA8876_CmdWrite(0xD7);
 RA8876_DataWrite(temp<<6);
}
 
//Input data format:R5G6B6
void Background_color_65k(uint16_t temp)
{
    RA8876_CmdWrite(0xD5);
 RA8876_DataWrite(temp>>8);
  
    RA8876_CmdWrite(0xD6);
 RA8876_DataWrite(temp>>3);
   
    RA8876_CmdWrite(0xD7);
 RA8876_DataWrite(temp<<3);
}
 
//Input data format:R8G8B8
void Background_color_16M(unsigned long temp)
{
    RA8876_CmdWrite(0xD5);
 RA8876_DataWrite(temp>>16);
  
    RA8876_CmdWrite(0xD6);
 RA8876_DataWrite(temp>>8);
   
    RA8876_CmdWrite(0xD7);
 RA8876_DataWrite(temp);
}

//[DBh]~[DEh]=========================================================================
void CGRAM_Start_address(unsigned long Addr)
{
/*
CGRAM START ADDRESS [31:0]
*/	 
    RA8876_CmdWrite(0xDB);
	RA8876_DataWrite(Addr);
    RA8876_CmdWrite(0xDC);
	RA8876_DataWrite(Addr>>8);
    RA8876_CmdWrite(0xDD);
	RA8876_DataWrite(Addr>>16);
    RA8876_CmdWrite(0xDE);
	RA8876_DataWrite(Addr>>24);
}

//[DFh]=========================================================================
/*
[bit7] Enter Power saving state
0: Normal state.
1: Enter power saving state.
[bit1][bit0] Power saving Mode definition
00: NA
01: Standby Mode
10: Suspend Mode
11: Sleep Mode
*/
void Power_Normal_Mode(void)
{
	RA8876_CmdWrite(0xDF);
	RA8876_DataWrite(0x00);
	Check_IC_ready();
}
void Power_Saving_Standby_Mode(void)
{
	RA8876_CmdWrite(0xDF);
//	RA8876_DataWrite(0x01);
//	RA8876_CmdWrite(0xDF);
	RA8876_DataWrite(0x81);
}
void Power_Saving_Suspend_Mode(void)
{
	RA8876_CmdWrite(0xDF);
//	RA8876_DataWrite(0x02);
//	RA8876_CmdWrite(0xDF);
	RA8876_DataWrite(0x82);
}
void Power_Saving_Sleep_Mode(void)
{
	RA8876_CmdWrite(0xDF);
	RA8876_DataWrite(0x03);
	RA8876_CmdWrite(0xDF);
	RA8876_DataWrite(0x83);
}










//[E5h]~[E6h]=========================================================================
void RA8876_I2CM_Clock_Prescale(uint16_t WX)
{
/*
I2C Master Clock Pre-scale [7:0]
I2C Master Clock Pre-scale [15:8]
XSCL = CCLK / (5*(prescale + 2))
*/  
    RA8876_CmdWrite(0xE5);
 RA8876_DataWrite(WX);
    RA8876_CmdWrite(0xE6);
 RA8876_DataWrite(WX>>8);
}
//[E7h]=========================================================================
void RA8876_I2CM_Transmit_Data(uint8_t temp)
{
/*
I2C Master Transmit[7:0]
*/  
    RA8876_CmdWrite(0xE7);
 RA8876_DataWrite(temp);
}
//[E8h]=========================================================================
uint8_t RA8876_I2CM_Receiver_Data(void)
{
/*
I2C Master Receiver [7:0]
*/  
 uint8_t temp;
    RA8876_CmdWrite(0xE8);
 temp=RA8876_DataRead();
 return temp;
}
//[E9h]=========================================================================
/*
[bit7] START
	Generate (repeated) start condition and be cleared by hardware automatically
	Note : This bit is always read as 0.
[bit6] STOP
	Generate stop condition and be cleared by hardware automatically
	Note : This bit is always read as 0.
[bit5] READ
	READ(READ and WRITE can��t be used simultaneously)
	Read form slave and be cleared by hardware automatically
	Note : This bit is always read as 0.
[bit4] WRITE
	WRITE(READ and WRITE can��t be used simultaneously)
	Write to slave and be cleared by hardware automatically
	Note : This bit is always read as 0.
[bit3] ACKNOWLEDGE
	When as a I2C master receiver
	0 : Sent ACK.
	1 : Sent NACK.
	Note : This bit is always read as 0.
[bit0] Noise Filter
	0 : Enable.
	1 : Disable.
*/

//[bit7] START
void RA8876_I2CM_Write_With_Start(void)
{
 RA8876_CmdWrite(0xE9);
#ifdef Disable_I2CM_Noise_Filter 
 RA8876_DataWrite(0x90);
#endif
 
#ifdef Enable_I2CM_Noise_Filter 
 RA8876_DataWrite(0x91);
#endif		  
}

//[bit6] STOP 
void RA8876_I2CM_Stop(void)
{
 RA8876_CmdWrite(0xE9);
#ifdef Disable_I2CM_Noise_Filter 
 RA8876_DataWrite(0x40);
#endif
 
#ifdef Enable_I2CM_Noise_Filter 
 RA8876_DataWrite(0x41);
#endif
}

//[bit5] READ
void RA8876_I2CM_Read_With_Ack(void)
{		  
 RA8876_CmdWrite(0xE9);
#ifdef Disable_I2CM_Noise_Filter 
 RA8876_DataWrite(0x20);
#endif
#ifdef Enable_I2CM_Noise_Filter 
 RA8876_DataWrite(0x21);
#endif
}
 
void RA8876_I2CM_Read_With_Nack(void)
{		  
 RA8876_CmdWrite(0xE9);
#ifdef Disable_I2CM_Noise_Filter 
 RA8876_DataWrite(0x68);
#endif
#ifdef Enable_I2CM_Noise_Filter 
 RA8876_DataWrite(0x69);
#endif
}

//[bit4] WRITE 
void RA8876_I2CM_Write(void)
{
 RA8876_CmdWrite(0xE9);
#ifdef Disable_I2CM_Noise_Filter 
 RA8876_DataWrite(0x10);
#endif
#ifdef Enable_I2CM_Noise_Filter 
 RA8876_DataWrite(0x11);
#endif
}
 
 
 
//[EAh]=========================================================================

uint8_t RA8876_I2CM_Check_Slave_ACK(void)
{
/*[bit7]
Received acknowledge from slave
0 : Acknowledge received.
1 : No Acknowledge received. 
*/ 
uint8_t temp;
 
 RA8876_CmdWrite(0xEA);
 temp=RA8876_DataRead();
 if((temp&0x80)==0x80)
  return 1;
 else
  return 0; 
}

uint8_t RA8876_I2CM_Bus_Busy(void)
{
/*[bit6]
I2C Bus is Busy
0 : Idle.
1 : Busy.  
*/ 
 uint8_t temp; 

 RA8876_CmdWrite(0xEA);
 temp=RA8876_DataRead();
 if((temp&0x40)==0x40)
  return 1;
 else
  return 0;    
}

uint8_t RA8876_I2CM_transmit_Progress(void)
{
/*[bit6]
 0=Complete
 1=Transferring
*/ 
 uint8_t temp; 

 RA8876_CmdWrite(0xEA);
 temp=RA8876_DataRead();
 if((temp&0x02)==0x02)
 return 1;
 else
 return 0;
}
 
uint8_t RA8876_I2CM_Arbitration(void)
{
/*[bit6]
I2C Bus is Busy
0 : Idle.
1 : Busy.  
*/ 
 uint8_t temp; 

 RA8876_CmdWrite(0xEA);
 temp=RA8876_DataRead();
 temp&=0x01;
 return temp;
}


//[F0h]=========================================================================
void Set_GPIO_A_In_Out(uint8_t temp)
{
/*
GPO-A_dir[7:0] : General Purpose I/O direction control.
0: Output
1: Input
*/
	RA8876_CmdWrite(0xF0);
	RA8876_DataWrite(temp);
}
//[F1h]=========================================================================
void Write_GPIO_A_7_0(uint8_t temp)
{
/*
GPI-A[7:0] : General Purpose Input, share with DB[15:8]
GPO-A[7:0] : General Purpose Output, share with DB[15:8]
*/
	RA8876_CmdWrite(0xF1);
	RA8876_DataWrite(temp);
}
uint8_t Read_GPIO_A_7_0(void)
{
/*
GPI-A[7:0] : General Purpose Input, share with DB[15:8]
GPO-A[7:0] : General Purpose Output, share with DB[15:8]
*/
	uint8_t temp;
	RA8876_CmdWrite(0xF1);
	temp=RA8876_DataRead();
	return temp;
}
//[F2h]=========================================================================
void Write_GPIO_B_7_4(uint8_t temp)
{
/*
Bit [7:4] is port B's General Purpose Output bit [7:4]. They are share with XKOUT[3:0]
Bit [3:0] is not writable.
*/
	RA8876_CmdWrite(0xF2);
	RA8876_DataWrite(temp);
}
uint8_t Read_GPIO_B_7_0(void)
{
/*
Bit[7:0] are share with {XKIN[3:0], XA0, XnWR, XnRD, XnCS}
Bit[3:0] are only available in serial MPU I/F, otherwise fix at 0.
*/
	uint8_t temp;
	RA8876_CmdWrite(0xF2);
	temp=RA8876_DataRead();
	return temp;
}

//[F3h]=========================================================================
void Set_GPIO_C_In_Out(uint8_t temp)
{
/*
GPIO-C_dir[7:0] : General Purpose I/O direction control.
0: Output
1: Input
*/
	RA8876_CmdWrite(0xF3);
	RA8876_DataWrite(temp);
}
//[F4h]=========================================================================
void Write_GPIO_C_7_0(uint8_t temp)
{
/*
REG[F4h] GPIO-C
GPIO-C[7] : XPWM0,
GPIO_C[4:0] : XnSFCS1, XnSFCS0, XMISO, XMOSI, XSCLK.
*/
	RA8876_CmdWrite(0xF4);
	RA8876_DataWrite(temp);
}
uint8_t Read_GPIO_C_7_0(void)
{
/*
REG[F4h] GPIO-C
GPIO-C[7] : XPWM0,
GPIO_C[4:0] : XnSFCS1, XnSFCS0, XMISO, XMOSI, XSCLK.
*/
	uint8_t temp;
	RA8876_CmdWrite(0xF4);
	temp=RA8876_DataRead();
	return temp;
}
//[F5h]=========================================================================
void Set_GPIO_D_In_Out(uint8_t temp)
{
/*
GPIO-D_dir[7:0] : General Purpose I/O direction control.
0: Output
1: Input
*/
	RA8876_CmdWrite(0xF5);
	RA8876_DataWrite(temp);
}
//[F6h]=========================================================================
void Write_GPIO_D_7_0(uint8_t temp)
{
/*
GPIO-D[7:0] : General Purpose Input/Output
*/
	RA8876_CmdWrite(0xF6);
	RA8876_DataWrite(temp);
}
uint8_t Read_GPIO_D_7_0(void)
{
/*
GPIO-D[7:0] : General Purpose Input/Output
*/
	uint8_t temp;
	RA8876_CmdWrite(0xF6);
	temp=RA8876_DataRead();
	return temp;
}
//[F7h]=========================================================================
void Set_GPIO_E_In_Out(uint8_t temp)
{
/*
GPIO-E_dir[7:0] : General Purpose I/O direction control.
0: Output
1: Input
*/
	RA8876_CmdWrite(0xF7);
	RA8876_DataWrite(temp);
}
//[F8h]=========================================================================
void Write_GPIO_E_7_0(uint8_t temp)
{
/*
GPIO-E[7:0] : General Purpose Input/Output.
share with {PDAT[23:19], PDAT[15:13]}
*/
	RA8876_CmdWrite(0xF8);
	RA8876_DataWrite(temp);
}
uint8_t Read_GPIO_E_7_0(void)
{
/*
GPIO-E[7:0] : General Purpose Input/Output.
share with {PDAT[23:19], PDAT[15:13]}
*/
	uint8_t temp;
	RA8876_CmdWrite(0xF8);
	temp=RA8876_DataRead();
	return temp;
}
//[F9h]=========================================================================
void Set_GPIO_F_In_Out(uint8_t temp)
{
/*
GPIO-F_dir[7:0] : General Purpose I/O direction control.
0: Output
1: Input
*/
	RA8876_CmdWrite(0xF9);
	RA8876_DataWrite(temp);
}
//[FAh]=========================================================================
void Write_GPIO_F_7_0(uint8_t temp)
{
/*
GPIO-F[7:0] : General Purpose Input/Output.
share with {XPDAT[12:10], XPDAT[7:3]}
*/
	RA8876_CmdWrite(0xFA);
	RA8876_DataWrite(temp);
}
uint8_t Read_GPIO_F_7_0(void)
{
/*
GPIO-F[7:0] : General Purpose Input/Output.
share with {XPDAT[12:10], XPDAT[7:3]}
*/
	uint8_t temp;
	RA8876_CmdWrite(0xFA);
	temp=RA8876_DataRead();
	return temp;
}

//[FBh]=========================================================================


void Long_Key_enable(void)
{
 /*
Key-Scan Control Register 1
[bit6]		LongKey Enable Bit
1 : Enable. Long key period is set by KSCR2 bit4-2.
0 : Disable.
*/
	uint8_t temp;
	RA8876_CmdWrite(0xFB);
	temp=RA8876_DataRead();
	temp |= cSetb6;
	RA8876_DataWrite(temp);
}


void Key_Scan_Freg(uint8_t setx)
{
/*KF2-0: Key-Scan Frequency */
  uint8_t temp;
  RA8876_CmdWrite(0xFB);
  temp = RA8876_DataRead();
  temp &= 0xf0;
  temp|= (setx&0x07);
  RA8876_DataWrite(temp);
}


//[FCh]=========================================================================

void Key_Scan_Wakeup_Function_Enable(void)
{
/*
Key-Scan Controller Register 2
[bit7]	
Key-Scan Wakeup Function Enable Bit
0: Key-Scan Wakeup function is disabled.
1: Key-Scan Wakeup function is enabled.
*/
	uint8_t temp;
	RA8876_CmdWrite(0xFC);
	temp=RA8876_DataRead();
	temp |= cSetb7;
	RA8876_DataWrite(temp);
}


void Long_Key_Timing_Adjustment(uint8_t setx)
{
 /*Long Key Timing Adjustment*/ 
  uint8_t temp,temp1;
  temp = setx & 0x1c;
  RA8876_CmdWrite(0xFC);
  temp1 = RA8876_DataRead();
  temp1|=temp;
  RA8876_DataWrite(temp1);   
 
}

uint8_t Numbers_of_Key_Hit(void)
{  
   uint8_t temp;
   RA8876_CmdWrite(0xFC);
   temp = RA8876_DataRead();   //read key touch number
   temp = temp & 0x03;	 //�T�{���X�ӫ���Q���U
   return temp;
}

//[FDh][FEh][FFh]=========================================================================
uint8_t Read_Key_Strobe_Data_0(void)
{
/*
Key Strobe Data 0
The corresponding key code 0 that is pressed.
*/
	uint8_t temp;
	RA8876_CmdWrite(0xFD);
	temp=RA8876_DataRead();
	return temp;
}
uint8_t Read_Key_Strobe_Data_1(void)
{
/*
Key Strobe Data 1
The corresponding key code 1 that is pressed.
*/
	uint8_t temp;
	RA8876_CmdWrite(0xFE);
	temp=RA8876_DataRead();
	return temp;
}
uint8_t Read_Key_Strobe_Data_2(void)
{
/*
Key Strobe Data 2
The corresponding key code 2 that is pressed.
*/
	uint8_t temp;
	RA8876_CmdWrite(0xFF);
	temp=RA8876_DataRead();
	return temp;
}



/*
*********************************************************************************************************
*	�� �� ��: Set_LCD_Panel
*	����˵��: �趨LCD��
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void Set_LCD_Panel(void)
{
	if (g_LcdType == LCD_70_800X480)	/* OTA7001 */
	{
		//**[10h]**//
		Select_LCD_Sync_Mode();	// Enable XVSYNC, XHSYNC, XDE.
		//	Select_LCD_DE_Mode();	// XVSYNC & XHSYNC in idle state.

		PCLK_Falling(); 
		//PCLK_Rising();

		VSCAN_T_to_B();
		PDATA_Set_RGB();

		HSYNC_Low_Active();
		VSYNC_Low_Active();
		DE_High_Active();
		//DE_Low_Active();

	/* ���� OTA7001A�ֲ� P13 �趨 */
		LCD_HorizontalWidth_VerticalHeight(g_LcdWidth,g_LcdHeight);//INNOLUX 800x480�C

		LCD_Horizontal_Non_Display(88);	//INNOLUX800x480	88�C
		LCD_HSYNC_Start_Position(40);	//INNOLUX800x480�A40�C
		LCD_HSYNC_Pulse_Width(48);		//INNOLUX800x480�A1~48�C

		LCD_Vertical_Non_Display(32);	//INNOLUX800x480  32�C
		LCD_VSYNC_Start_Position(13);	//INNOLUX800x480�A13
		LCD_VSYNC_Pulse_Width(3);		//INNOLUX800x480�A3�C
	}
	else if (g_LcdType == LCD_101_1024X600)		/* INCH101 */
	{
		//**[10h]**//
		Select_LCD_Sync_Mode();	// Enable XVSYNC, XHSYNC, XDE.
		//	Select_LCD_DE_Mode();	// XVSYNC & XHSYNC in idle state.

		PCLK_Falling(); 
		//PCLK_Rising();

		VSCAN_T_to_B();
		PDATA_Set_RGB();

		HSYNC_Low_Active();
		VSYNC_Low_Active();
		DE_High_Active();
		//DE_Low_Active();

	/* ���� 10.1��LCD�ֲ� P11 �趨 */
		LCD_HorizontalWidth_VerticalHeight(g_LcdWidth,g_LcdHeight);//INNOLUX 800x480�C

		LCD_Horizontal_Non_Display(160);	/* HS Blanking      -  160  -  */
		LCD_HSYNC_Start_Position(160);		/* HS Front Porch   16 160 216 */
		LCD_HSYNC_Pulse_Width(48);			/* HS pulse width    1  -  140 */

		LCD_Vertical_Non_Display(23);		/* VS Blanking 		 -  23  -  */
		LCD_VSYNC_Start_Position(12);		/* VS Front Porch    1  12 127 */
		LCD_VSYNC_Pulse_Width(10);			/* VS pulse width    1  -  20  */
	}

#ifdef EJ080NA_04B	//##INNOLUX_1024x768_EJ080NA-04B

//**[03h]**//
	LVDS_Format1();//(VESA format)//only for RA8877
//	LVDS_Format2();//(JEIDA format) //only for RA8877

//**[10h]**//
	Select_LCD_Sync_Mode();	// Enable XVSYNC, XHSYNC, XDE.
//	Select_LCD_DE_Mode();	// XVSYNC & XHSYNC in idle state.

//**[12h]**//
//	PCLK_Rising();
	PCLK_Falling();

	VSCAN_T_to_B();
//	VSCAN_B_to_T();

	PDATA_Set_RGB();
//	PDATA_Set_RBG();
//	PDATA_Set_GRB();
//	PDATA_Set_GBR();
//	PDATA_Set_BRG();
//	PDATA_Set_BGR();
//	PDATA_Set_Gray();
//	PDATA_IDLE_STATE();

//**[13h]**//
	HSYNC_Low_Active();
//	HSYNC_High_Active();

	VSYNC_Low_Active();
//	VSYNC_High_Active();
	
//	DE_Low_Active();
	DE_High_Active();

	//**[14h][15h][1Ah][1Bh]**//
	LCD_HorizontalWidth_VerticalHeight(1024,768);// INNOLUX 1024x768�C

/*	[16h][17h] : Figure 19-3 [HND] 
	[18h] :		 Figure 19-3 [HST] 
	[19h] :		 Figure 19-3 [HPW]	*/
	LCD_Horizontal_Non_Display(100);	//Non Display or Back-Porch //Blank=90~376, TYP.=320�C
	LCD_HSYNC_Start_Position(100);		//Start Position or Front-Porch
	LCD_HSYNC_Pulse_Width(120);			//Pulse Width

/*	[1Ch][1Dh] : Figure 19-3 [VND]
	[1Eh] :		 Figure 19-3 [VST] 
	[1Fh] :		 Figure 19-3 [VPW]	*/
	LCD_Vertical_Non_Display(10);		//Non Display or Back-Porch  //Blank=10~77, TYP.=38�C
	LCD_VSYNC_Start_Position(10);		//Start Position or Front-Porch
	LCD_VSYNC_Pulse_Width(18);			//Pulse Width

#endif

#ifdef LQ150X1LGN2C	//##SHARP_LQ150X1LGN2C_LVDS1 & LVDS2

//**[03h]**//
	LVDS_Format1();//(VESA format)//only for RA8877
//	LVDS_Format2();//(JEIDA format) //only for RA8877

//**[10h]**//
	Select_LCD_Sync_Mode();	// Enable XVSYNC, XHSYNC, XDE.
//	Select_LCD_DE_Mode();	// XVSYNC & XHSYNC in idle state.

//**[12h]**//
//	PCLK_Rising();
	PCLK_Falling();

	VSCAN_T_to_B();
//	VSCAN_B_to_T();

	PDATA_Set_RGB();
//	PDATA_Set_RBG();
//	PDATA_Set_GRB();
//	PDATA_Set_GBR();
//	PDATA_Set_BRG();
//	PDATA_Set_BGR();
//	PDATA_Set_Gray();
//	PDATA_IDLE_STATE();

//**[13h]**//
	HSYNC_Low_Active();
//	HSYNC_High_Active();

	VSYNC_Low_Active();
//	VSYNC_High_Active();
	
//	DE_Low_Active();
	DE_High_Active();

//**[14h][15h][1Ah][1Bh]**//
	LCD_HorizontalWidth_VerticalHeight(1024,768);//SHARP1024x768�C
//**[16h][17h]**//
	LCD_Horizontal_Non_Display(30);//SHARP1024x768�ABlank=32~320~696�C
//**[18h]**//
	LCD_HSYNC_Start_Position(10);//SHARP1024x768�C
//**[19h]**//
	LCD_HSYNC_Pulse_Width(10);//SHARP1024x768�C
//**[1Ch][1Dh]**//
	LCD_Vertical_Non_Display(20);//SHARP1024x768�ABlank=5~38~222�C
//**[1Eh]**//
	LCD_VSYNC_Start_Position(10);//SHARP1024x768�C
//**[1Fh]**//
	LCD_VSYNC_Pulse_Width(10);//SHARP1024x768�C
#endif

#ifdef N070ICG_LD1	//##INNOLUX_1280x800_N070ICG-LD1

//**[03h]**//
	LVDS_Format1();//(VESA format)//only for RA8877
//	LVDS_Format2();//(JEIDA format) //only for RA8877

//**[10h]**//
	Select_LCD_Sync_Mode();	// Enable XVSYNC, XHSYNC, XDE.
	Select_LCD_DE_Mode();	// XVSYNC & XHSYNC in idle state.


//**[12h]**//
//	PCLK_Rising();
	PCLK_Falling();

	VSCAN_T_to_B();
//	VSCAN_B_to_T();

	PDATA_Set_RGB();
//	PDATA_Set_RBG();
//	PDATA_Set_GRB();
//	PDATA_Set_GBR();
//	PDATA_Set_BRG();
//	PDATA_Set_BGR();
//	PDATA_Set_Gray();
//	PDATA_IDLE_STATE();

//**[13h]**//
	HSYNC_Low_Active();
//	HSYNC_High_Active();

	VSYNC_Low_Active();
//	VSYNC_High_Active();
	
//	DE_Low_Active();
	DE_High_Active();

//**[14h][15h][1Ah][1Bh]**//
	LCD_HorizontalWidth_VerticalHeight(1280,800);//
//**[16h][17h]**//
	LCD_Horizontal_Non_Display(50);//
//**[18h]**//
	LCD_HSYNC_Start_Position(50);//
//**[19h]**//
	LCD_HSYNC_Pulse_Width(60);//
//**[1Ch][1Dh]**//
	LCD_Vertical_Non_Display(8);//
//**[1Eh]**//
	LCD_VSYNC_Start_Position(8);//
//**[1Fh]**//
	LCD_VSYNC_Pulse_Width(8);//	


#endif

#ifdef B116XW03_V0	//##AUO1366x768_B116XW03 V0

//**[03h]**//
	LVDS_Format1();//(VESA format)//only for RA8877
//	LVDS_Format2();//(JEIDA format) //only for RA8877

//**[10h]**//
	Select_LCD_Sync_Mode();	// Enable XVSYNC, XHSYNC, XDE.
	Select_LCD_DE_Mode();	// XVSYNC & XHSYNC in idle state.

//**[12h]**//
//	PCLK_Rising();
	PCLK_Falling();

	VSCAN_T_to_B();
//	VSCAN_B_to_T();

	PDATA_Set_RGB();
//	PDATA_Set_RBG();
//	PDATA_Set_GRB();
//	PDATA_Set_GBR();
//	PDATA_Set_BRG();
//	PDATA_Set_BGR();
//	PDATA_Set_Gray();
//	PDATA_IDLE_STATE();

//**[13h]**//
//	HSYNC_Low_Active();
//	HSYNC_High_Active();

//	VSYNC_Low_Active();
//	VSYNC_High_Active();
	
//	DE_Low_Active();
	DE_High_Active();

//**[14h][15h][1Ah][1Bh]**//
	LCD_HorizontalWidth_VerticalHeight(1366,768);//
//**[16h][17h]**//
	LCD_Horizontal_Non_Display(15);//
//**[18h]**//
	LCD_HSYNC_Start_Position(15);//
//**[19h]**//
	LCD_HSYNC_Pulse_Width(44);//
//**[1Ch][1Dh]**//
	LCD_Vertical_Non_Display(8);//
//**[1Eh]**//
	LCD_VSYNC_Start_Position(8);//
//**[1Fh]**//
	LCD_VSYNC_Pulse_Width(8);//
#endif

#ifdef LQ121S1LG81	//##SHARP_LQ121S1LG81

//**[03h]**//
	LVDS_Format1();//(VESA format)//only for RA8877
//	LVDS_Format2();//(JEIDA format) //only for RA8877

//**[10h]**//
	Select_LCD_Sync_Mode();	// Enable XVSYNC, XHSYNC, XDE.
//	Select_LCD_DE_Mode();	// XVSYNC & XHSYNC in idle state.

//**[12h]**//
//	PCLK_Rising();
	PCLK_Falling();

	VSCAN_T_to_B();
//	VSCAN_B_to_T();

	PDATA_Set_RGB();
//	PDATA_Set_RBG();
//	PDATA_Set_GRB();
//	PDATA_Set_GBR();
//	PDATA_Set_BRG();
//	PDATA_Set_BGR();
//	PDATA_Set_Gray();
//	PDATA_IDLE_STATE();

//**[13h]**//
	HSYNC_Low_Active();
//	HSYNC_High_Active();

	VSYNC_Low_Active();
//	VSYNC_High_Active();
	
//	DE_Low_Active();
	DE_High_Active();

//**[14h][15h][1Ah][1Bh]**//
	LCD_HorizontalWidth_VerticalHeight(800,600);//
//**[16h][17h]**//
	LCD_Horizontal_Non_Display(88);//
//**[18h]**//
	LCD_HSYNC_Start_Position(40);//
//**[19h]**//
	LCD_HSYNC_Pulse_Width(128);//
//**[1Ch][1Dh]**//
	LCD_Vertical_Non_Display(23);//
//**[1Eh]**//
	LCD_VSYNC_Start_Position(39);//
//**[1Fh]**//
	LCD_VSYNC_Pulse_Width(4);//
	
#endif


#ifdef LQ035NC111
	//**[10h]**//
		Select_LCD_Sync_Mode();	// Enable XVSYNC, XHSYNC, XDE.
	//	Select_LCD_DE_Mode();	// XVSYNC & XHSYNC in idle state.

	//PCLK_Falling();
	PCLK_Rising();

	VSCAN_T_to_B();
	PDATA_Set_RGB();


	//HSYNC_High_Active();
	//VSYNC_High_Active();

	HSYNC_Low_Active();
	VSYNC_Low_Active();
	//DE_High_Active();
	DE_Low_Active();

	LCD_HorizontalWidth_VerticalHeight(320,240);//

	LCD_Horizontal_Non_Display(44);//
	LCD_HSYNC_Start_Position(20);//
	LCD_HSYNC_Pulse_Width(24);//	non_display + pulse width = 68

	LCD_Vertical_Non_Display(15);//
	LCD_VSYNC_Start_Position(2);//
	LCD_VSYNC_Pulse_Width(1);//


	Select_Main_Window_16bpp();
	Main_Image_Start_Address(0);				
	Main_Image_Width(320);							
	Main_Window_Start_XY(0,0);	
	Canvas_Image_Start_address(0);
	Canvas_image_width(320);//
	Active_Window_XY(0,0);
	Active_Window_WH(320,240);
#endif


#ifdef AT080TN52

	//**[10h]**//
		Select_LCD_Sync_Mode();	// Enable XVSYNC, XHSYNC, XDE.
	//	Select_LCD_DE_Mode();	// XVSYNC & XHSYNC in idle state.

	PCLK_Falling();
	//PCLK_Rising();

	VSCAN_T_to_B();
	PDATA_Set_RGB();

	HSYNC_Low_Active();
	VSYNC_Low_Active();
	DE_High_Active();
	//DE_Low_Active();

	LCD_HorizontalWidth_VerticalHeight(800,600);//INNOLUX 800x600�C
	LCD_Horizontal_Non_Display(46);//INNOLUX800x600�A46�C
	LCD_HSYNC_Start_Position(16);//INNOLUX800x600�A16~354�C
	LCD_HSYNC_Pulse_Width(3);//INNOLUX800x600�A1~40�C
	LCD_Vertical_Non_Display(23);//INNOLUX800x600�A23�C
	LCD_VSYNC_Start_Position(3);//INNOLUX800x600�A1~77�C
	LCD_VSYNC_Pulse_Width(3);//INNOLUX800x600�A1~20�C
#endif

#ifdef AT070TN92

//**[10h]**//
	Select_LCD_Sync_Mode();	// Enable XVSYNC, XHSYNC, XDE.
//	Select_LCD_DE_Mode();	// XVSYNC & XHSYNC in idle state.

	PCLK_Falling();
	//PCLK_Rising();

	VSCAN_T_to_B();
	PDATA_Set_RGB();

	HSYNC_Low_Active();
	VSYNC_Low_Active();
	DE_High_Active();
	//DE_Low_Active();

	LCD_HorizontalWidth_VerticalHeight(800,480);//INNOLUX 800x480�C
	LCD_Horizontal_Non_Display(46);//INNOLUX800x600�A46�C


	LCD_HSYNC_Start_Position(210);//INNOLUX800x600�A16~354�C
	LCD_HSYNC_Pulse_Width(10);//INNOLUX800x600�A1~40�C
	LCD_Vertical_Non_Display(23);//INNOLUX800x600�A23�C
	LCD_VSYNC_Start_Position(22);//INNOLUX800x600�A1~147�C
	LCD_VSYNC_Pulse_Width(10);//INNOLUX800x600�A1~20�C
#endif

#ifdef ET101000DM6	//##EDT_1024x600_ET101000DM6

//**[03h]**//
	LVDS_Format1();//(VESA format)//only for RA8877
//	LVDS_Format2();//(JEIDA format) //only for RA8877

//**[10h]**//
	Select_LCD_Sync_Mode();	// Enable XVSYNC, XHSYNC, XDE.
//	Select_LCD_DE_Mode();	// XVSYNC & XHSYNC in idle state.

//**[12h]**//
//	PCLK_Rising();
	PCLK_Falling();

	VSCAN_T_to_B();
//	VSCAN_B_to_T();

	PDATA_Set_RGB();
//	PDATA_Set_RBG();
//	PDATA_Set_GRB();
//	PDATA_Set_GBR();
//	PDATA_Set_BRG();
//	PDATA_Set_BGR();
//	PDATA_Set_Gray();
//	PDATA_IDLE_STATE();

//**[13h]**//
	HSYNC_Low_Active();
//	HSYNC_High_Active();

	VSYNC_Low_Active();
//	VSYNC_High_Active();
	
//	DE_Low_Active();
	DE_High_Active();

//**[14h][15h][1Ah][1Bh]**//
	LCD_HorizontalWidth_VerticalHeight(1024,600);//EDT_1024x600�C
//**[16h][17h]**//
	LCD_Horizontal_Non_Display(160);//EDT_1024x600�ABlank=90~376@320�C
//**[18h]**//
	LCD_HSYNC_Start_Position(160);//EDT_1024x600�ABlank=90~376@320�C
//**[19h]**//
	LCD_HSYNC_Pulse_Width(60);//EDT_1024x600�ABlank=90~376@320�C
//**[1Ch][1Dh]**//
	LCD_Vertical_Non_Display(23);//EDT_1024x600�ABlank=10~200@35�C
//**[1Eh]**//
	LCD_VSYNC_Start_Position(12);//EDT_1024x600�ABlank=10~200@35�C
//**[1Fh]**//
	LCD_VSYNC_Pulse_Width(5);//EDT_1024x600�ABlank=10~200@35�C	
	
#endif


#ifdef G190SVT01	//##AUO_1680x342_G190SVT01

//**[10h]**//
	Select_LCD_Sync_Mode();	// Enable XVSYNC, XHSYNC, XDE.
//	Select_LCD_DE_Mode();	// XVSYNC & XHSYNC in idle state.

//**[12h]**//
//	PCLK_Rising();
	PCLK_Falling();

	VSCAN_T_to_B();
//	VSCAN_B_to_T();

	PDATA_Set_RGB();
//	PDATA_Set_RBG();
//	PDATA_Set_GRB();
//	PDATA_Set_GBR();
//	PDATA_Set_BRG();
//	PDATA_Set_BGR();
//	PDATA_Set_Gray();
//	PDATA_IDLE_STATE();

//**[13h]**//
	HSYNC_Low_Active();
//	HSYNC_High_Active();

	VSYNC_Low_Active();
//	VSYNC_High_Active();
	
//	DE_Low_Active();
	DE_High_Active();

//**[14h][15h][1Ah][1Bh]**//
	LCD_HorizontalWidth_VerticalHeight(1680,350);//1680x342
//**[16h][17h]**//
	LCD_Horizontal_Non_Display(30);//Blank=288�C
//**[18h]**//
	LCD_HSYNC_Start_Position(30);//
//**[19h]**//
	LCD_HSYNC_Pulse_Width(30);//
//**[1Ch][1Dh]**//
	LCD_Vertical_Non_Display(6);//Blank=16�C
//**[1Eh]**//
	LCD_VSYNC_Start_Position(6);//
//**[1Fh]**//
	LCD_VSYNC_Pulse_Width(6);//	    
	
#endif


#ifdef ZJ070NA_01B	//##INNOLUX_1024x600_ZJ070NA_01B

//**[03h]**//
	LVDS_Format1();//(VESA format)//only for RA8877
//	LVDS_Format2();//(JEIDA format) //only for RA8877

//**[10h]**//
	Select_LCD_Sync_Mode();	// Enable XVSYNC, XHSYNC, XDE.
//	Select_LCD_DE_Mode();	// XVSYNC & XHSYNC in idle state.

//**[12h]**//
//	PCLK_Rising();
	PCLK_Falling();

	VSCAN_T_to_B();
//	VSCAN_B_to_T();

	PDATA_Set_RGB();
//	PDATA_Set_RBG();
//	PDATA_Set_GRB();
//	PDATA_Set_GBR();
//	PDATA_Set_BRG();
//	PDATA_Set_BGR();
//	PDATA_Set_Gray();
//	PDATA_IDLE_STATE();

//**[13h]**//
	HSYNC_Low_Active();
//	HSYNC_High_Active();

	VSYNC_Low_Active();
//	VSYNC_High_Active();
	
//	DE_Low_Active();
	DE_High_Active();

//**[14h][15h][1Ah][1Bh]**//
	LCD_HorizontalWidth_VerticalHeight(1024,600);//1024x600
//**[16h][17h]**//
	LCD_Horizontal_Non_Display(50);//Blank=320�C
//**[18h]**//
	LCD_HSYNC_Start_Position(50);//
//**[19h]**//
	LCD_HSYNC_Pulse_Width(220);//
//**[1Ch][1Dh]**//
	LCD_Vertical_Non_Display(8);//Blank=35�C
//**[1Eh]**//
	LCD_VSYNC_Start_Position(7);//
//**[1Fh]**//
	LCD_VSYNC_Pulse_Width(20);//		    
	
#endif

#ifdef LQ190E1LW52	//##SHARP_1280x1024_LQ190E1LW52

//**[10h]**//
	Select_LCD_Sync_Mode();	// Enable XVSYNC, XHSYNC, XDE.
//	Select_LCD_DE_Mode();	// XVSYNC & XHSYNC in idle state.

//**[12h]**//
//	PCLK_Rising();
	PCLK_Falling();

	VSCAN_T_to_B();
//	VSCAN_B_to_T();

	PDATA_Set_RGB();
//	PDATA_Set_RBG();
//	PDATA_Set_GRB();
//	PDATA_Set_GBR();
//	PDATA_Set_BRG();
//	PDATA_Set_BGR();
//	PDATA_Set_Gray();
//	PDATA_IDLE_STATE();

//**[13h]**//
	HSYNC_Low_Active();
//	HSYNC_High_Active();

	VSYNC_Low_Active();
//	VSYNC_High_Active();
	
//	DE_Low_Active();
	DE_High_Active();

//**[14h][15h][1Ah][1Bh]**//
	LCD_HorizontalWidth_VerticalHeight(1280,1024);//1024x600
//**[16h][17h]**//
	LCD_Horizontal_Non_Display(110);//Blank=204x2�C
//**[18h]**//
	LCD_HSYNC_Start_Position(110);//
//**[19h]**//
	LCD_HSYNC_Pulse_Width(110);//
//**[1Ch][1Dh]**//
	LCD_Vertical_Non_Display(15);//Blank=42�C
//**[1Eh]**//
	LCD_VSYNC_Start_Position(15);//
//**[1Fh]**//
	LCD_VSYNC_Pulse_Width(12);//	    
	
#endif	

#ifdef HJ070IA_02F	//##INNOLUX_1280x800_HJ070IA_02F

//**[03h]**//
	LVDS_Format1();//(VESA format)//only for RA8877
//	LVDS_Format2();//(JEIDA format) //only for RA8877

//**[10h]**//
	Select_LCD_Sync_Mode();	// Enable XVSYNC, XHSYNC, XDE.
//	Select_LCD_DE_Mode();	// XVSYNC & XHSYNC in idle state.

//**[12h]**//
//	PCLK_Rising();
	PCLK_Falling();

	VSCAN_T_to_B();
//	VSCAN_B_to_T();

	PDATA_Set_RGB();
//	PDATA_Set_RBG();
//	PDATA_Set_GRB();
//	PDATA_Set_GBR();
//	PDATA_Set_BRG();
//	PDATA_Set_BGR();
//	PDATA_Set_Gray();
//	PDATA_IDLE_STATE();

//**[13h]**//
	HSYNC_Low_Active();
//	HSYNC_High_Active();

	VSYNC_Low_Active();
//	VSYNC_High_Active();
	
//	DE_Low_Active();
	DE_High_Active();

//**[14h][15h][1Ah][1Bh]**//
	LCD_HorizontalWidth_VerticalHeight(1280,800);//1280x800
//**[16h][17h]**//
	LCD_Horizontal_Non_Display(30);//Blank=160�C
//**[18h]**//
	LCD_HSYNC_Start_Position(30);//
//**[19h]**//
	LCD_HSYNC_Pulse_Width(100);//
//**[1Ch][1Dh]**//
	LCD_Vertical_Non_Display(3);//Blank=23�C
//**[1Eh]**//
	LCD_VSYNC_Start_Position(3);//
//**[1Fh]**//
	LCD_VSYNC_Pulse_Width(18);//	    
#endif


#ifdef LQ156M1LG21	//##SHARP_1920x1080_LQ156M1LG21

//**[03h]**//
	LVDS_Format1();//(VESA format)//only for RA8877
//	LVDS_Format2();//(JEIDA format) //only for RA8877

//**[10h]**//
	Select_LCD_Sync_Mode();	// Enable XVSYNC, XHSYNC, XDE.
//	Select_LCD_DE_Mode();	// XVSYNC & XHSYNC in idle state.

//**[12h]**//
//	PCLK_Rising();
	PCLK_Falling();

	VSCAN_T_to_B();
//	VSCAN_B_to_T();

	PDATA_Set_RGB();
//	PDATA_Set_RBG();
//	PDATA_Set_GRB();
//	PDATA_Set_GBR();
//	PDATA_Set_BRG();
//	PDATA_Set_BGR();
//	PDATA_Set_Gray();
//	PDATA_IDLE_STATE();

//**[13h]**//
//	HSYNC_Low_Active();
	HSYNC_High_Active();

//	VSYNC_Low_Active();
	VSYNC_High_Active();
	
//	DE_Low_Active();
	DE_High_Active();

//**[14h][15h][1Ah][1Bh]**//
	LCD_HorizontalWidth_VerticalHeight(1920,1080);//1280x800
//**[16h][17h]**//
	LCD_Horizontal_Non_Display(40);//Blank min.=140�C
//**[18h]**//
	LCD_HSYNC_Start_Position(20);//
//**[19h]**//
	LCD_HSYNC_Pulse_Width(100);//
//**[1Ch][1Dh]**//
	LCD_Vertical_Non_Display(3);//Blank min.=31�C
//**[1Eh]**//
	LCD_VSYNC_Start_Position(3);//
//**[1Fh]**//
	LCD_VSYNC_Pulse_Width(20);//	    
#endif

#ifdef LQ201U1LW32	//##SHARP_1600x1200_LQ201U1LW32

//**[03h]**//
	LVDS_Format1();//(VESA format)//only for RA8877
//	LVDS_Format2();//(JEIDA format) //only for RA8877

//**[10h]**//
	Select_LCD_Sync_Mode();	// Enable XVSYNC, XHSYNC, XDE.
//	Select_LCD_DE_Mode();	// XVSYNC & XHSYNC in idle state.

//**[12h]**//
//	PCLK_Rising();
	PCLK_Falling();

	VSCAN_T_to_B();
//	VSCAN_B_to_T();

	PDATA_Set_RGB();
//	PDATA_Set_RBG();
//	PDATA_Set_GRB();
//	PDATA_Set_GBR();
//	PDATA_Set_BRG();
//	PDATA_Set_BGR();
//	PDATA_Set_Gray();
//	PDATA_IDLE_STATE();

//**[13h]**//
//	HSYNC_Low_Active();
	HSYNC_High_Active();

//	VSYNC_Low_Active();
	VSYNC_High_Active();
	
//	DE_Low_Active();
	DE_High_Active();

//**[14h][15h][1Ah][1Bh]**//
	LCD_HorizontalWidth_VerticalHeight(1600,1200);//
//**[16h][17h]**//
	LCD_Horizontal_Non_Display(15);
//**[18h]**//
	LCD_HSYNC_Start_Position(15);
//**[19h]**//
	LCD_HSYNC_Pulse_Width(40);
//**[1Ch][1Dh]**//
	LCD_Vertical_Non_Display(5);
//**[1Eh]**//
	LCD_VSYNC_Start_Position(5);
//**[1Fh]**//
	LCD_VSYNC_Pulse_Width(5);
#endif
}



/*
*********************************************************************************************************
*	�� �� ��: Set_Serial_Flash_IF
*	����˵��: ���ô���flash
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void Set_Serial_Flash_IF(void)
{

//(*) Using Serial Flash
	#ifdef MX25L25635E
		Enable_SFlash_SPI();
	#endif

//(*) Using GT_ROM
	#ifdef GT21L16T1W
		GTFont_Select_GT21L16T1W();
		Enable_SFlash_SPI();
	#endif
	#ifdef GT30L16U2W
		GTFont_Select_GT30L16U2W();
		Enable_SFlash_SPI();
	#endif
	#ifdef GT30L24T3Y
		GTFont_Select_GT30L24T3Y();
		Enable_SFlash_SPI();
	#endif
	#ifdef GT30L24M1Z	
		GTFont_Select_GT30L24M1Z();
		Enable_SFlash_SPI();
	#endif
	#ifdef GT30L32S4W
		GTFont_Select_GT30L32S4W();
		Enable_SFlash_SPI();
	#endif
	#ifdef GT20L24F6Y
		GTFont_Select_GT20L24F6Y();
		Enable_SFlash_SPI();
	#endif
	#ifdef GT21L24S1W
		GTFont_Select_GT21L24S1W();
		Enable_SFlash_SPI();
	#endif

//(*) Set RA8876/77 Master SPI address width. 
	Select_SFI_24bit_Address();				/* ����Flash����Ϊ24bitѰַģʽ */
	//Select_SFI_32bit_Address();
	
//(*)	
	Select_standard_SPI_Mode0_or_Mode3();	/* RA8876����ģʽ����׼SPIģʽ0��ģʽ3ʱ��ͼ */
//	Select_RA8875_SPI_Mode0_and_Mode3();
	
//(*)
//	Select_SFI_Single_Mode_Dummy_0T_03h();	/* �޿����� */
//	Select_SFI_Single_Mode_Dummy_8T_0Bh();	/* �������� xmiso���룬 RA8876 �ڵ�ַ�����ݼ������ 8 �������� */
//	Select_SFI_Single_Mode_Dummy_16T_1Bh();
	Select_SFI_Dual_Mode_Dummy_8T_3Bh();	/* ˫��ģʽ0,����ˢͼ�ٶ� */
//	Select_SFI_Dual_Mode_Dummy_4T_BBh();

//(*)
/*
1. At CPOL=0�� SCK Ƶ����δ����ʱΪ 0��
	o For CPHA=0����������Ƶ�ʵ�����Ե��ȡ(low->high)���������������½�Ե(high->low)�仯��
	o For CPHA=1����������Ƶ�ʵ�����Ե��ȡ(high->low)���������������Ͻ�Ե�仯(low->high)��
2 At CPOL=1 �� SCK Ƶ����δ����ʱΪ 1(�� CPOL=0 ����)��
	o For CPHA=0, ��������Ƶ�ʵ�����Ե��ȡ(high->low)���������������Ͻ�Ե�仯(low->high)��
	o For CPHA=1, ��������Ƶ�ʵ�����Ե��ȡ(low->high)���������������½�Ե(high->low)�仯��
*/
	Reset_CPOL();				/* ��CPOLʱ�Ӽ���λ��0 */
//	Set_CPOL();
	Reset_CPHA();				/* ��CPHAʱ����λ��0 */
//	Set_CPHA();

/******************************/
}



/*
*********************************************************************************************************
*	�� �� ��: RA8876_SDRAM_initial
*	����˵��: SDRAM��ʼ������
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/ 
void RA8876_SDRAM_initial(void)
{
uint8_t	CAS_Latency;
uint16_t	Auto_Refresh;

/*
	Ex. ��� SDRAM Ƶ���� 100MHz��SDRAM ��ˢ������ Tref ��
	64ms������ row size Ϊ 8192����ô�ڲ�ˢ��ʱ��Ӧ����С��
	64e-3 / 8192 * 100e6 ~= 781 = 30Dh����˴˻�����[E2h][E3h]
	�����趨 30Dh
*/	
	
#ifdef HY57V641620F
	CAS_Latency = 2;
	Auto_Refresh = (64 * DRAM_FREQ * 1000) / (4096);
	
	// �е�ַ A0-A11; �е�ַA0-A7
	LCD_RegisterWrite(0xe0, (0 << 7) | (0 << 6) | (1 << 5) | (1 << 3) | (0 << 0));	/* 0x28 */
	
	LCD_RegisterWrite(0xe1, CAS_Latency);      //CAS:2=0x02�ACAS:3=0x03
	LCD_RegisterWrite(0xe2, Auto_Refresh);
	LCD_RegisterWrite(0xe3, Auto_Refresh >> 8);
	LCD_RegisterWrite(0xe4, 0x01);		//0x01
#endif

#ifdef IS42SM16160D

	if(DRAM_FREQ<=133)	CAS_Latency=2;
	else 				CAS_Latency=3;

	Auto_Refresh=(64*DRAM_FREQ*1000)/(8192);
	
	LCD_RegisterWrite(0xe0,0xf9);        
	LCD_RegisterWrite(0xe1,CAS_Latency);      //CAS:2=0x02�ACAS:3=0x03
	LCD_RegisterWrite(0xe2,Auto_Refresh);
	LCD_RegisterWrite(0xe3,Auto_Refresh>>8);
	LCD_RegisterWrite(0xe4,0x09);
#endif

#ifdef IS42S16320B
	
	if(DRAM_FREQ<=133)	CAS_Latency=2;
	else 				CAS_Latency=3;
	
	Auto_Refresh=(64*DRAM_FREQ*1000)/(8192);
	
	LCD_RegisterWrite(0xe0,0x32);	
	LCD_RegisterWrite(0xe1,CAS_Latency);
	LCD_RegisterWrite(0xe2,Auto_Refresh);
	LCD_RegisterWrite(0xe3,Auto_Refresh>>8);
	LCD_RegisterWrite(0xe4,0x09);

#endif

#ifdef IS42S16400F

	if(DRAM_FREQ<143)	CAS_Latency=2;
	else 				CAS_Latency=3;

	Auto_Refresh=(64*DRAM_FREQ*1000)/(4096);
	
	LCD_RegisterWrite(0xe0,0x28);      
	LCD_RegisterWrite(0xe1,CAS_Latency);      //CAS:2=0x02�ACAS:3=0x03
	LCD_RegisterWrite(0xe2,Auto_Refresh);
	LCD_RegisterWrite(0xe3,Auto_Refresh>>8);
	LCD_RegisterWrite(0xe4,0x01);
#endif

#ifdef M12L32162A
	CAS_Latency=3;
	Auto_Refresh=(64*DRAM_FREQ*1000)/(4096);
	
	LCD_RegisterWrite(0xe0,0x08);      
	LCD_RegisterWrite(0xe1,CAS_Latency);      //CAS:2=0x02�ACAS:3=0x03
	LCD_RegisterWrite(0xe2,Auto_Refresh);
	LCD_RegisterWrite(0xe3,Auto_Refresh>>8);
	LCD_RegisterWrite(0xe4,0x09);
#endif

#ifdef M12L2561616A
	CAS_Latency=3;	
	Auto_Refresh=(64*DRAM_FREQ*1000)/(8192);
	
	LCD_RegisterWrite(0xe0,0x31);      
	LCD_RegisterWrite(0xe1,CAS_Latency);      //CAS:2=0x02�ACAS:3=0x03
	LCD_RegisterWrite(0xe2,Auto_Refresh);
	LCD_RegisterWrite(0xe3,Auto_Refresh>>8);
	LCD_RegisterWrite(0xe4,0x01);
#endif

#ifdef M12L64164A
	CAS_Latency=3;
	Auto_Refresh=(64*DRAM_FREQ*1000)/(4096);
	
	LCD_RegisterWrite(0xe0,0x28);      
	LCD_RegisterWrite(0xe1,CAS_Latency);      //CAS:2=0x02�ACAS:3=0x03
	LCD_RegisterWrite(0xe2,Auto_Refresh);
	LCD_RegisterWrite(0xe3,Auto_Refresh>>8);
	LCD_RegisterWrite(0xe4,0x09);
#endif

#ifdef W9825G6JH
	CAS_Latency=3;
	Auto_Refresh=(64*DRAM_FREQ*1000)/(8192);
	
	LCD_RegisterWrite(0xe0,0x31);      
	LCD_RegisterWrite(0xe1,CAS_Latency);      //CAS:2=0x02�ACAS:3=0x03
	LCD_RegisterWrite(0xe2,Auto_Refresh);
	LCD_RegisterWrite(0xe3,Auto_Refresh>>8);
	LCD_RegisterWrite(0xe4,0x01);
#endif

#ifdef W9812G6JH
	CAS_Latency=3;
	Auto_Refresh=(64*DRAM_FREQ*1000)/(4096);
	
	LCD_RegisterWrite(0xe0,0x29);      
	LCD_RegisterWrite(0xe1,CAS_Latency);      //CAS:2=0x02�ACAS:3=0x03
	LCD_RegisterWrite(0xe2,Auto_Refresh);
	LCD_RegisterWrite(0xe3,Auto_Refresh>>8);
	LCD_RegisterWrite(0xe4,0x01);
#endif

#ifdef MT48LC4M16A
	CAS_Latency=3;
	Auto_Refresh=(64*DRAM_FREQ*1000)/(4096);
	
	LCD_RegisterWrite(0xe0,0x28);      
	LCD_RegisterWrite(0xe1,CAS_Latency);      //CAS:2=0x02�ACAS:3=0x03
	LCD_RegisterWrite(0xe2,Auto_Refresh);
	LCD_RegisterWrite(0xe3,Auto_Refresh>>8);
	LCD_RegisterWrite(0xe4,0x01);
#endif

#ifdef K4S641632N
	CAS_Latency=3;
	Auto_Refresh=(64*DRAM_FREQ*1000)/(4096);
	
	LCD_RegisterWrite(0xe0,0x28);      
	LCD_RegisterWrite(0xe1,CAS_Latency);      //CAS:2=0x02�ACAS:3=0x03
	LCD_RegisterWrite(0xe2,Auto_Refresh);
	LCD_RegisterWrite(0xe3,Auto_Refresh>>8);
	LCD_RegisterWrite(0xe4,0x01);
#endif

#ifdef K4S281632K
	CAS_Latency=3;
	Auto_Refresh=(64*DRAM_FREQ*1000)/(4096);
	
	LCD_RegisterWrite(0xe0,0x29);      
	LCD_RegisterWrite(0xe1,CAS_Latency);      //CAS:2=0x02�ACAS:3=0x03
	LCD_RegisterWrite(0xe2,Auto_Refresh);
	LCD_RegisterWrite(0xe3,Auto_Refresh>>8);
	LCD_RegisterWrite(0xe4,0x01);
#endif

#ifdef IS42S16100
	CAS_Latency=3;
	Auto_Refresh=(32*DRAM_FREQ*1000)/(2048);
	
	LCD_RegisterWrite(0xe0,0x00);      
	LCD_RegisterWrite(0xe1,CAS_Latency);      //CAS:2=0x02�ACAS:3=0x03
	LCD_RegisterWrite(0xe2,Auto_Refresh);
	LCD_RegisterWrite(0xe3,Auto_Refresh>>8);
	LCD_RegisterWrite(0xe4,0x01);
#endif

	Check_SDRAM_Ready();
}

/*
*********************************************************************************************************
*	�� �� ��: RA8876_PLL
*	����˵��: RA8876ʱ�����ú���
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
/*
1. ֻ���� PLL ����ʱ�����޸� PLL ������
2. �� REG[05h] ~ REG[0Ah] ���޸ĺ�PLL ��Ҫ�� 30us ��ʱ�����ȶ�Ƶ�������
3. ���� OSC Ƶ�� F IN �� PLLDIVM �����������������
4. �ڲ���Ƶ 
*/
void RA8876_PLL(void) 
{
/*
(1) 10MHz <= OSC_FREQ <= 15MHz
(2) 10MHz <= (OSC_FREQ/PLLDIVM) <= 40MHz
(3) 100MHz <= [OSC_FREQ/(PLLDIVM+1)]x(PLLDIVN+1) <= 600MHz

PLLDIVM:0
PLLDIVN:1~63
PLLDIVK:CPLL & MPLL = 1/2/4/8.SPLL = 1/2/4/8/16/32/64/128.

ex:
 OSC_FREQ = 10MHz
 Set X_DIVK=2
 Set X_DIVM=0
 => (X_DIVN+1)=(XPLLx4)/10
*/

	uint16_t x_Divide,PLLC1,PLLC2;

	switch(3)
	{
	//if SCAN_FREQ >= 50MHz.
	case 0:	//(All PLL Divided by 2)
		LCD_RegisterWrite(0x05,0x02);
		LCD_RegisterWrite(0x07,0x02);
		LCD_RegisterWrite(0x09,0x02);
		LCD_RegisterWrite(0x06,(SCAN_FREQ*2/10)-1);
		LCD_RegisterWrite(0x08,(DRAM_FREQ*2/10)-1);
		LCD_RegisterWrite(0x0A,(CORE_FREQ*2/10)-1);
	break;

	//if SCAN_FREQ >= 25MHz. DRAM_FREQ <=150MHz.
	case 1:	//(All PLL Divided by 4
		LCD_RegisterWrite(0x05,0x04);
		LCD_RegisterWrite(0x07,0x04);
		LCD_RegisterWrite(0x09,0x04);
		LCD_RegisterWrite(0x06,(SCAN_FREQ*4/10)-1);
		LCD_RegisterWrite(0x08,(DRAM_FREQ*4/10)-1);
		LCD_RegisterWrite(0x0A,(CORE_FREQ*4/10)-1);
	break;

	case 2:	
		// Set pixel clock
		x_Divide=(64*OSC_FREQ) / SCAN_FREQ;
		if(  1<x_Divide)	PLLC1=0x00;
		if(  2<x_Divide)	PLLC1=0x02;
		if(  4<x_Divide)	PLLC1=0x04;
		if(  8<x_Divide)	PLLC1=0x06;
		if( 16<x_Divide)	PLLC1=0x08;
		if( 32<x_Divide)	PLLC1=0x0A;
		if( 64<x_Divide)	PLLC1=0x0C;
		if(128<x_Divide)	PLLC1=0x0E;
	
		PLLC2 = (uint32_t)(SCAN_FREQ * pow(2,(PLLC1>>1)) / OSC_FREQ )-1;
	
		LCD_RegisterWrite(0x05,PLLC1);				
		LCD_RegisterWrite(0x06,PLLC2);

		// Set SDRAM clock
		x_Divide=(64*OSC_FREQ) / DRAM_FREQ;
		if(  1<x_Divide)	PLLC1=0x00;
		if(  2<x_Divide)	PLLC1=0x02;
		if(  4<x_Divide)	PLLC1=0x04;
		if(  8<x_Divide)	PLLC1=0x06;
	
		PLLC2 = (uint32_t)(DRAM_FREQ*pow(2,(PLLC1>>1))/OSC_FREQ)-1;
	
		LCD_RegisterWrite(0x07,PLLC1);				
		LCD_RegisterWrite(0x08,PLLC2);

		// Set Core clock
		x_Divide=(64*OSC_FREQ) / CORE_FREQ;
		if(  1<x_Divide)	PLLC1=0x00;
		if(  2<x_Divide)	PLLC1=0x02;
		if(  4<x_Divide)	PLLC1=0x04;
		if(  8<x_Divide)	PLLC1=0x06;
	
		PLLC2 = (uint32_t)(CORE_FREQ*pow(2,(PLLC1>>1))/OSC_FREQ)-1;
	
		LCD_RegisterWrite(0x09,PLLC1);				
		LCD_RegisterWrite(0x0A,PLLC2);

	break;



	case 3:	
/*
	�ڽ��ɳ��������· PLL ���ṩϵͳƵ�ʡ�LCD ɨ��Ƶ���� SDRAM Ƶ��ʹ��
	��һʯӢ����������: (XI/XO: 10-15MHz)
	�ڲ��������ϵͳƵ�� (���ֵ 120MHz)
	SDRAM Ƶ�� (���ֵ 166MHz)
	LCD ��Ļɨ��Ƶ�� (���ֵ 100MHz)	
*/	
/*
	SCLK PLL ʱ�ӿ��ƼĴ���1[05H]
	
bit0��
	SCLK PLLDIVM
	PCLK PLL Pre-driver parameter.
	0b: �� 1��
	1b: �� 2��
	
bit2-1��
	SCLK PLLDIVK[1:0]
	SCLK PLL �����Ƶ
	00b: �� 1��
	01b: �� 2��
	10b: �� 4��
	11b: �� 8��
	
bit5-3��
	SCLK extra divider
	xx1b: �� 16��
	000b: �� 1��
	010b: �� 2��
	100b: �� 4��
	110b: �� 8��
*/

/*
	SCLK PLL ʱ�ӿ��ƼĴ���2[06H]
bit7-6
	NA
	0  RO
bit5-0
	SCLK PLLDIVN[5:0]
	SCLK PLL �����������ֵӦ���� 1~63�� (��ֵ 0 �ǽ�ֹ��)��
*/
  		/* ��������ʱ�� */
		if( 50<SCAN_FREQ)					
		{
			LCD_RegisterWrite(0x05,0x02);				/* PLL��2 */
			LCD_RegisterWrite(0x06,(SCAN_FREQ*2/OSC_FREQ)-1);
		}
		if((25<SCAN_FREQ)&&(SCAN_FREQ<=50))	
		{								  	
			LCD_RegisterWrite(0x05,0x04);				/* PLL��4 */
			LCD_RegisterWrite(0x06,(SCAN_FREQ*4/OSC_FREQ)-1);
		}
		if((12<SCAN_FREQ)&&(SCAN_FREQ<=25))	
		{								  	
			LCD_RegisterWrite(0x05,0x06);				/* PLL��8 */
			LCD_RegisterWrite(0x06,(SCAN_FREQ*8/OSC_FREQ)-1);
		}
		if(( 7<SCAN_FREQ)&&(SCAN_FREQ<=12))	
		{
			LCD_RegisterWrite(0x05,0x08);				/* PLL��16 */
			LCD_RegisterWrite(0x06,(SCAN_FREQ*16/OSC_FREQ)-1);
		}
		if(SCAN_FREQ<=7)	
		{
			LCD_RegisterWrite(0x05,0x0A);				/* PLL��32 */
			LCD_RegisterWrite(0x06,(SCAN_FREQ*32/OSC_FREQ)-1);
		}								    

/*
	MCLK PLL ���ƼĴ���1[07H]
		
bit7-3
	NA
		
bit2-1
	MCLK PLLDIVK[1:0]
	PCLK PLL Output divider
	00b: �� 1��
	01b: �� 2��
	10b: �� 4��
	11b: �� 8��

bit0
	MCLK PLLDIVM
	MCLK PLL Pre-driver parameter.
	0b: �� 1��
	1b: �� 2
*/		
/*
	MCLK PLL ���ƼĴ���2[08H]
bit7-6
	NA

bit5-0
	MCLK PLLDIVN[5:0]
	MCLK PLL�����������ֵӦ���� 1~63�� (��ֵ 0 �ǽ�ֹ��)��
*/

		/* ����SDRAMʱ�� */
		if( 50<DRAM_FREQ)					
		{
			LCD_RegisterWrite(0x07,0x02);				//PLL Divided by 2
			LCD_RegisterWrite(0x08,(DRAM_FREQ*2/OSC_FREQ)-1);
		}
		if((25<DRAM_FREQ)&&(DRAM_FREQ<=50))	
		{
			LCD_RegisterWrite(0x07,0x04);				//PLL Divided by 4
			LCD_RegisterWrite(0x08,(DRAM_FREQ*4/OSC_FREQ)-1);
		}
		if((12<DRAM_FREQ)&&(DRAM_FREQ<=25))	
		{
			LCD_RegisterWrite(0x07,0x06);				//PLL Divided by 8
			LCD_RegisterWrite(0x08,(DRAM_FREQ*8/OSC_FREQ)-1);
		}
	
/*
	CCLK PLL���ƼĴ���1[09H] 
bit7-3
	NA

bit2-1
	CCLK PLLDIVK[1:0]
	CCLK PLL �����Ƶ
	00b: �� 1��
	01b: �� 2��
	10b: �� 4��
	11b: �� 8��

bit0
	CCLK PLLDIVM
	CCLK PLL Pre-driver parameter.
	0b: �� 1��
	1b: �� 2	
*/
/*
	CCLK PLL���ƼĴ���2[0AH] 
bit7-6
	NA

bit5-0
	CCLK PLLDIVN[5:0]
	CCLK PLL�����������ֵӦ���� 1~63�� (��ֵ 0 �ǽ�ֹ��)
*/
		/* ���ú���ʱ�� */
		if( 50<CORE_FREQ)					
		{
			LCD_RegisterWrite(0x09,0x02);				//PLL Divided by 2
			LCD_RegisterWrite(0x0A,(CORE_FREQ*2/OSC_FREQ)-1);
		}
		if((25<CORE_FREQ)&&(CORE_FREQ<=50))	
		{
			LCD_RegisterWrite(0x09,0x04);				//PLL Divided by 4
			LCD_RegisterWrite(0x0A,(CORE_FREQ*4/OSC_FREQ)-1);
		}
		if((12<CORE_FREQ)&&(CORE_FREQ<=25))	
		{
			LCD_RegisterWrite(0x09,0x06);				//PLL Divided by 8
			LCD_RegisterWrite(0x0A,(CORE_FREQ*8/OSC_FREQ)-1);
		}	 									
		break;
	}
	Enable_PLL();

	bsp_DelayMS(200);	//delay_ms(100);
}



/* RA8876�ٷ�API���� */
char tmp1[32];		 

void MPU8_8bpp_Memory_Write
(
unsigned short x //x of coordinate 
,unsigned short y // y of coordinate 
,unsigned short w //width
,unsigned short h //height
,const unsigned char *data //8bit data
)
{														  
	unsigned short i,j;
	Graphic_Mode();
    Active_Window_XY(x,y);
	Active_Window_WH(w,h); 					
	Goto_Pixel_XY(x,y);
	RA8876_CmdWrite(0x04);	
for(i=0;i< h;i++)
{	
	for(j=0;j< w;j++)
 	{	   
	 Check_Mem_WR_FIFO_not_Full();
	 RA8876_DataWrite(*data);
	 data++;
	}
}
	Check_Mem_WR_FIFO_Empty();
}	 

			
void MPU8_16bpp_Memory_Write
(
unsigned short x // x of coordinate 
,unsigned short y // y of coordinate 
,unsigned short w // width
,unsigned short h // height
,const unsigned char *data // 8bit data
)
{
	unsigned short i,j;
	Graphic_Mode();
    Active_Window_XY(x,y);
	Active_Window_WH(w,h); 					
	Goto_Pixel_XY(x,y);
	RA8876_CmdWrite(0x04);
for(i=0;i< h;i++)
{	
	for(j=0;j< w;j++)
 	{
	 Check_Mem_WR_FIFO_not_Full();
	 RA8876_DataWrite(*data);
	 data++;
	 Check_Mem_WR_FIFO_not_Full();
	 RA8876_DataWrite(*data);
	 data++;
	}
}
	Check_Mem_WR_FIFO_Empty();
}		 


void MPU8_24bpp_Memory_Write 
(
unsigned short x // x of coordinate
,unsigned short y // y of coordinate 
,unsigned short w // width
,unsigned short h // height
,const unsigned char *data // 8bit data
)

{
	unsigned short i,j;
	Graphic_Mode();
    Active_Window_XY(x,y);
	Active_Window_WH(w,h); 					
	Goto_Pixel_XY(x,y);
	RA8876_CmdWrite(0x04);
for(i=0;i< h;i++)
{	
	for(j=0;j< w;j++)
 	{
	 Check_Mem_WR_FIFO_not_Full();
	 RA8876_DataWrite(*data);
	 data++;
	 Check_Mem_WR_FIFO_not_Full();
	 RA8876_DataWrite(*data);
	 data++;
	 Check_Mem_WR_FIFO_not_Full();
	 RA8876_DataWrite(*data);
	 data++;
	}
}
	Check_Mem_WR_FIFO_Empty();
}


/*
*********************************************************************************************************
*	�� �� ��: MPU16_16bpp_Memory_Write
*	����˵��: ���ķ���������д���ڴ�
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void MPU16_16bpp_Memory_Write 
(
unsigned short x //x of coordinate
,unsigned short y //y of coordinate 
,unsigned short w //width
,unsigned short h //height
,const unsigned short *data //16bit data
)			
{
	uint32_t i;
	Graphic_Mode();
    Active_Window_XY(x,y);
	Active_Window_WH(w,h); 					
	Goto_Pixel_XY(x,y);
	RA8876_CmdWrite(0x04);

	for(i = 0; i < w * h; i++)
 	{
		//Check_Mem_WR_FIFO_not_Full();		/* ����ˢ��ʱ�� */
		RA8876_DataWrite(*data);
		data++;
	}
	Check_Mem_WR_FIFO_Empty();
}




void MPU16_24bpp_Mode1_Memory_Write 
(
unsigned short x //x of coordinate
,unsigned short y //y of coordinate
,unsigned short w //width
,unsigned short h //height
,const unsigned short *data //16bit data
)	
{
	unsigned short i,j;
	Graphic_Mode();
    Active_Window_XY(x,y);
	Active_Window_WH(w,h); 					
	Goto_Pixel_XY(x,y);
	RA8876_CmdWrite(0x04);
for(i=0;i< h;i++)
{	
	for(j=0;j< w/2;j++)
 	{
	 RA8876_DataWrite(*data);
	 Check_Mem_WR_FIFO_not_Full();
	 data++;
	 RA8876_DataWrite(*data);
	 Check_Mem_WR_FIFO_not_Full();
	 data++;
	 RA8876_DataWrite(*data);
	 Check_Mem_WR_FIFO_not_Full();
	 data++;
	}
}
	Check_Mem_WR_FIFO_Empty();
}


void MPU16_24bpp_Mode2_Memory_Write
(
unsigned short x //x of coordinate
,unsigned short y //y of coordinate
,unsigned short w //width
,unsigned short h //height
,const unsigned short *data //16bit data
)	
{
	unsigned short i,j;
	Graphic_Mode();
    Active_Window_XY(x,y);
	Active_Window_WH(w,h); 					
	Goto_Pixel_XY(x,y);
	RA8876_CmdWrite(0x04);
for(i=0;i< h;i++)
{	
	for(j=0;j< w;j++)
 	{
	 Check_Mem_WR_FIFO_not_Full();
	 RA8876_DataWrite(*data);
	 data++;
	 Check_Mem_WR_FIFO_not_Full();
	 RA8876_DataWrite(*data);
	 data++;
	}
}
	Check_Mem_WR_FIFO_Empty();
}




/*
*********************************************************************************************************
*	�� �� ��: PIP
*	����˵��: ���л��������ú���
*	��    �Σ�  On_Off ��0 : disable PIP, 1 : enable PIP
*			Select_PIP �� 1 : use PIP1 , 2 : use PIP2		
*				 PAddr �� PIP��ʼ��ַ
*					XP ��PIP���ڵ�����X
*					YP ��PIP���ڵ�����Y
*			ImageWidth ��PIPͼƬ�Ŀ��
*				 X_Dis ����ʾͼƬ������X
*				 Y_Dis ����ʾͼƬ������Y
*				   X_W ����ʾͼƬ�Ŀ��
*				   Y_H ����ʾͼƬ�ĸ߶�
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void PIP
(
unsigned char On_Off // 0 : disable PIP, 1 : enable PIP, 2 : To maintain the original state//ʹ�ܻ�ʧ��
,unsigned char Select_PIP // 1 : use PIP1 , 2 : use PIP2									//PIP1��PIP2
,unsigned long PAddr //start address of PIP													//PIP����ʼ��ַ
,unsigned short XP //coordinate X of PIP Window, It must be divided by 4.					//PIP���ڵ�����X
,unsigned short YP //coordinate Y of PIP Window, It must be divided by 4.					//PIP���ڵ�����Y
,unsigned long ImageWidth //Image Width of PIP (recommend = canvas image width)			  	//PIPͼƬ���
,unsigned short X_Dis //coordinate X of Display Window									//��ʾͼƬ������X
,unsigned short Y_Dis //coordinate Y of Display Window									//��ʾͼƬ������Y
,unsigned short X_W //width of PIP and Display Window, It must be divided by 4.			//��ʾͼƬ�Ŀ��
,unsigned short Y_H //height of PIP and Display Window , It must be divided by 4.		//��ʾͼƬ�ĸ߶�
)
{

/*	
	PIP ���ڵĲ������� : ɫ���ʼ��ַ��ͼ���ȡ���ʾ���ꡢ
�������ꡢ���ڿ�ȡ����ڸ߶� 
*/
	if(Select_PIP == 1 )  
	{
	Select_PIP1_Parameter();			/* ѡ������PIP1�Ĳ��� */
	}
	if(Select_PIP == 2 )  
	{
	Select_PIP2_Parameter();			/* ѡ������PIP2�Ĳ��� */
	}
	PIP_Display_Start_XY(X_Dis,Y_Dis);	/* PIP1��2������ʾ���Ͻǵ����� */
	PIP_Image_Start_Address(PAddr);		/* ���л�1��2ͼ����ʼ��ַ */
	PIP_Image_Width(ImageWidth);		/* PIP1��2ͼ���� */
	PIP_Window_Image_Start_XY(XP,YP);	/* PIP1��2����ͼƬ���Ͻǵ����� */
	PIP_Window_Width_Height(X_W,Y_H);	/* PIP1��2����ͼƬ��Ⱥ͸߶� */


	if(On_Off == 0)
    {
  		if(Select_PIP == 1 )  
		{ 
		Disable_PIP1();	
		}
		if(Select_PIP == 2 )  
		{
		Disable_PIP2();
		}
    }

    if(On_Off == 1)
    {
  		if(Select_PIP == 1 )  
		{ 
		Enable_PIP1();	
		}
		if(Select_PIP == 2 )  
		{
		Enable_PIP2();
		}
    }
   
}

 


/*internal font
x : Print font start coordinate of X
y : Print font start coordinate of Y
X_W : active window width
Y_H : active window height
FontColor : Set Font Color
BackGroundColor : Set Font BackGround Color 
Font Color and BackGround Color dataformat : 
ColorDepth_8bpp : R3G3B2
ColorDepth_16bpp : R5G6B5
ColorDepth_24bpp : R8G8B8
tmp2 : Hex variable you want print (range : 0 ~ 32bit)
*/
void Print_Internal_Font_Hexvariable(unsigned short x,unsigned short y,unsigned short X_W,unsigned short Y_H,unsigned long FontColor,unsigned long BackGroundColor,  unsigned int tmp2)
{	
   	Text_Mode();
	CGROM_Select_Internal_CGROM();
#ifdef MCU_8bit_ColorDepth_8bpp
    Foreground_color_256(FontColor);
	Background_color_256(BackGroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_16bpp
    Foreground_color_65k(FontColor);
	Background_color_65k(BackGroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_24bpp
    Foreground_color_16M(FontColor);
	Background_color_16M(BackGroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_16bpp	
    Foreground_color_65k(FontColor);
	Background_color_65k(BackGroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_1	
    Foreground_color_16M(FontColor);
	Background_color_16M(BackGroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_2	
    Foreground_color_16M(FontColor);
	Background_color_16M(BackGroundColor);
#endif
	Active_Window_XY(x,y);
    Active_Window_WH(X_W,Y_H);
	Goto_Text_XY(x,y);
	sprintf(tmp1 ,"%X", tmp2); 
	Show_String(tmp1);
}



/*internal font
x : Print font start coordinate of X
y : Print font start coordinate of Y
X_W : active window width
Y_H : active window height
FontColor : Set Font Color
BackGroundColor : Set Font BackGround Color 
Font Color and BackGround Color dataformat :
ColorDepth_8bpp : R3G3B2
ColorDepth_16bpp : R5G6B5
ColorDepth_24bpp : R8G8B8
tmp2 : Decimalvariable you want print (range : 0 ~ 32bit)
*/
void Print_Internal_Font_Decimalvariable(unsigned short x,unsigned short y,unsigned short X_W,unsigned short Y_H,unsigned long FontColor,unsigned long BackGroundColor,unsigned int tmp2)
{	

	Text_Mode();
	CGROM_Select_Internal_CGROM();
#ifdef MCU_8bit_ColorDepth_8bpp
    Foreground_color_256(FontColor);
	Background_color_256(BackGroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_16bpp
    Foreground_color_65k(FontColor);
	Background_color_65k(BackGroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_24bpp
    Foreground_color_16M(FontColor);
	Background_color_16M(BackGroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_16bpp	
    Foreground_color_65k(FontColor);
	Background_color_65k(BackGroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_1	
    Foreground_color_16M(FontColor);
	Background_color_16M(BackGroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_2	
    Foreground_color_16M(FontColor);
	Background_color_16M(BackGroundColor);
#endif
    Active_Window_XY(x,y);
    Active_Window_WH(X_W,Y_H);
	Goto_Text_XY(x,y);
	sprintf(tmp1 ,"%d", tmp2); 
	Show_String(tmp1);
}





void Print_Internal_Font_String
(
unsigned short x //coordinate x for print string
,unsigned short y //coordinate x for print string
,unsigned short X_W //active window width
,unsigned short Y_H //active window height
,unsigned long FontColor //FontColor : Set Font Color
,unsigned long BackGroundColor 
/*BackGroundColor : Set Font BackGround Color.Font Color and BackGround Color dataformat :
ColorDepth_8bpp : R3G3B2
ColorDepth_16bpp : R5G6B5
ColorDepth_24bpp : R8G8B8*/
,char tmp2[] //tmp2 : Font String which you want print on LCD
)
{

  	Text_Mode();
	CGROM_Select_Internal_CGROM();
	Font_Select_12x24_24x24();
	#ifdef MCU_8bit_ColorDepth_8bpp	
    Foreground_color_256(FontColor);
	Background_color_256(BackGroundColor);
	#endif
	#ifdef MCU_8bit_ColorDepth_16bpp	
    Foreground_color_65k(FontColor);
	Background_color_65k(BackGroundColor);
	#endif
	#ifdef MCU_8bit_ColorDepth_24bpp	
    Foreground_color_16M(FontColor);
	Background_color_16M(BackGroundColor);
	#endif
	#ifdef MCU_16bit_ColorDepth_16bpp	
    Foreground_color_65k(FontColor);
	Background_color_65k(BackGroundColor);
	#endif
	#ifdef MCU_16bit_ColorDepth_24bpp_Mode_1	
    Foreground_color_16M(FontColor);
	Background_color_16M(BackGroundColor);
	#endif
	#ifdef MCU_16bit_ColorDepth_24bpp_Mode_2	
    Foreground_color_16M(FontColor);
	Background_color_16M(BackGroundColor);
	#endif
	Active_Window_XY(x,y);							 
	Active_Window_WH(X_W,Y_H);
	Goto_Text_XY(x,y);
//	sprintf(tmp1,"%s",tmp2); 
//	Show_String(tmp1);
	Show_String(tmp2);

}





void Print_BIG5String
(
unsigned char Clk //SPI CLK = System Clock / 2*(Clk+1)
,unsigned char SCS //0 : use CS0 , 1 : use CS1
,unsigned short x //coordinate x for print string
,unsigned short y //coordinate y for print string
,unsigned short X_W //active window width
,unsigned short Y_H //active window height
,unsigned long FontColor //Set Font Color
,unsigned long BackGroundColor //Set Font BackGround Color 
/*Font Color and BackGround Color dataformat :
ColorDepth_8bpp : R3G3B2
ColorDepth_16bpp : R5G6B5
ColorDepth_24bpp : R8G8B8*/
,char *tmp2 //tmp2 : BIG5 Font String which you want print on LCD
)
{

  Select_SFI_Font_Mode();
  CGROM_Select_Genitop_FontROM();
  SPI_Clock_Period(Clk);
  Set_GTFont_Decoder(0x11);      
  if(SCS == 0)
  { 
  Select_SFI_0();
  }
  if(SCS == 1)
  {
  Select_SFI_1();
  }

#ifdef MCU_8bit_ColorDepth_8bpp	
  Foreground_color_256(FontColor);
  Background_color_256(BackGroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_16bpp	
  Foreground_color_65k(FontColor);
  Background_color_65k(BackGroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_24bpp	
  Foreground_color_16M(FontColor);
  Background_color_16M(BackGroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_16bpp	
  Foreground_color_65k(FontColor);					                               
  Background_color_65k(BackGroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_1	
  Foreground_color_16M(FontColor);
  Background_color_16M(BackGroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_2	
  Foreground_color_16M(FontColor);
  Background_color_16M(BackGroundColor);
#endif
  Text_Mode();
  Active_Window_XY(x,y);
  Active_Window_WH(X_W,Y_H);
  Goto_Text_XY(x,y);
//  sprintf(tmp1,"%s",tmp2); 
//  Show_String(tmp1);
  Show_String(tmp2);

}




void Print_GB2312String
(
unsigned char Clk //Clk : SPI CLK = System Clock / 2*(Clk+1)
,unsigned char SCS //SCS : 0 = CS0 , 1 = CS1
,unsigned short x //coordinate x for print string
,unsigned short y //coordinate y for print string
,unsigned short X_W //active window width
,unsigned short Y_H //active window height
,unsigned long FontColor //Set Font Color
,unsigned long BackGroundColor //Set Font BackGround Color 
/*Font Color and BackGround Color dataformat :
ColorDepth_8bpp : R3G3B2
ColorDepth_16bpp : R5G6B5
ColorDepth_24bpp : R8G8B8*/
,char tmp2[] //tmp2 : GB2312 Font String which you want print on LCD
)
{

  Select_SFI_Font_Mode();
  CGROM_Select_Genitop_FontROM();
  SPI_Clock_Period(Clk);
  Set_GTFont_Decoder(0x01);      
  if(SCS == 0)
  {
  Select_SFI_0();
  }
  if(SCS == 1)
  {
  Select_SFI_1();
  }
#ifdef MCU_8bit_ColorDepth_8bpp	
  Foreground_color_256(FontColor);
  Background_color_256(BackGroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_16bpp	
  Foreground_color_65k(FontColor);
  Background_color_65k(BackGroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_24bpp	
  Foreground_color_16M(FontColor);
  Background_color_16M(BackGroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_16bpp	
  Foreground_color_65k(FontColor);
  Background_color_65k(BackGroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_1	
  Foreground_color_16M(FontColor);
  Background_color_16M(BackGroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_2	
  Foreground_color_16M(FontColor);
  Background_color_16M(BackGroundColor);
#endif
  Text_Mode();
  Active_Window_XY(x,y);
  Active_Window_WH(X_W,Y_H);
  Goto_Text_XY(x,y);
//  sprintf(tmp1,"%s",tmp2); 
//  Show_String(tmp1);
  Show_String(tmp2);

}


void Print_GB12345String
(
unsigned char Clk //Clk : SPI CLK = System Clock / 2*(Clk+1)
,unsigned char SCS //SCS : 0 = CS0 , 1 = CS1
,unsigned short x //coordinate x for print string
,unsigned short y ////coordinate y for print string
,unsigned short X_W //active window width
,unsigned short Y_H //active window height
,unsigned long FontColor //Set Font Color
,unsigned long BackGroundColor //Set Font BackGround Color 
/*Font Color and BackGround Color dataformat :
ColorDepth_8bpp : R3G3B2
ColorDepth_16bpp : R5G6B5
ColorDepth_24bpp : R8G8B8*/
,char *tmp2 //tmp2 : GB12345 Font String which you want print on LCD
)
{  
  Select_SFI_Font_Mode();
  CGROM_Select_Genitop_FontROM();
  SPI_Clock_Period(Clk);
  Set_GTFont_Decoder(0x05);      
  if(SCS == 0)
  {
  Select_SFI_0();
  }
  
  if(SCS == 1)
  {
  Select_SFI_1();
  }
#ifdef MCU_8bit_ColorDepth_8bpp	
  Foreground_color_256(FontColor);
  Background_color_256(BackGroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_16bpp	
  Foreground_color_65k(FontColor);
  Background_color_65k(BackGroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_24bpp	
  Foreground_color_16M(FontColor);
  Background_color_16M(BackGroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_16bpp	
  Foreground_color_65k(FontColor);
  Background_color_65k(BackGroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_1	
  Foreground_color_16M(FontColor);
  Background_color_16M(BackGroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_2	
  Foreground_color_16M(FontColor);
  Background_color_16M(BackGroundColor);
#endif
  Text_Mode();
  Active_Window_XY(x,y);
  Active_Window_WH(X_W,Y_H);
  Goto_Text_XY(x,y);
//  sprintf(tmp1,"%s",tmp2); 
//  Show_String(tmp1);
  Show_String(tmp2);
}




 
void Print_UnicodeString
(
unsigned char Clk //SPI CLK = System Clock / 2*(Clk+1)
,unsigned char SCS //SCS : 0 = CS0 , 1 = CS1
,unsigned short x //Print font start coordinate of X
,unsigned short y //Print font start coordinate of Y
,unsigned short X_W //active window width
,unsigned short Y_H //active window height
,unsigned long FontColor //Set Font Color
,unsigned long BackGroundColor //Set Font BackGround Color 
/*Font Color and BackGround Color dataformat :
ColorDepth_8bpp : R3G3B2
ColorDepth_16bpp : R5G6B5
ColorDepth_24bpp : R8G8B8*/
,unsigned short *tmp2 /*tmp2 : Unicode Font String which you want print on LCD (L"string" in keil c is Unicode string)*/
)
{
  Select_SFI_Font_Mode();
  CGROM_Select_Genitop_FontROM();
  SPI_Clock_Period(Clk);
  if(SCS == 0)
  {
  Select_SFI_0();
  }
  if(SCS == 1)
  {
  Select_SFI_1();
  }
#ifdef MCU_8bit_ColorDepth_8bpp	
  Foreground_color_256(FontColor);
  Background_color_256(BackGroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_16bpp	
  Foreground_color_65k(FontColor);
  Background_color_65k(BackGroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_24bpp	
  Foreground_color_16M(FontColor);
  Background_color_16M(BackGroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_16bpp	
  Foreground_color_65k(FontColor);
  Background_color_65k(BackGroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_1	
  Foreground_color_16M(FontColor);
  Background_color_16M(BackGroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_2	
  Foreground_color_16M(FontColor);
  Background_color_16M(BackGroundColor);
#endif
  Text_Mode();
  Active_Window_XY(x,y);
  Active_Window_WH(X_W,Y_H);
  Goto_Text_XY(x,y);
  while(*tmp2 != '\0')
  {
    if((*tmp2)>0x0020 && (*tmp2)<0x0080)
		{
			/* ASCII Code*/
			Set_GTFont_Decoder(0x21); 
			RA8876_CmdWrite(0x04);
			RA8876_DataWrite(*tmp2);
			Check_2D_Busy();
		}
    else
		{
			/* Unicode */
	        Set_GTFont_Decoder(0x18);
			RA8876_CmdWrite(0x04);
			RA8876_DataWrite((*tmp2)>>8); 
            RA8876_DataWrite(*tmp2);
            Check_2D_Busy();
      
		}
 	        ++tmp2;
  }
  
  Graphic_Mode(); //back to graphic mode
}





void Select_Font_Height_WxN_HxN_ChromaKey_Alignment
(
unsigned char Font_Height 
/*Font_Height:
16 : Font = 8x16 �B16x16
24 : Font = 12x24�B24x24  
32 : Font = 16x32�B32x32 */
,unsigned char XxN //XxN :Font Width x 1~4
,unsigned char YxN //YxN :Font Height x 1~4
,unsigned char ChromaKey 
/*ChromaKey :
0 : Font Background color not transparency
1 : Set Font Background color transparency*/
,unsigned char Alignment // 0 : no alignment, 1 : Set font alignment
)
{
	if(Font_Height==16)	Font_Select_8x16_16x16();
	if(Font_Height==24)	Font_Select_12x24_24x24();
	if(Font_Height==32)	Font_Select_16x32_32x32();

	//(*)
	if(XxN==1)	Font_Width_X1();
	if(XxN==2)	Font_Width_X2();
	if(XxN==3)	Font_Width_X3();
	if(XxN==4)	Font_Width_X4();

	//(*)	
	if(YxN==1)	Font_Height_X1();
	if(YxN==2)	Font_Height_X2();
	if(YxN==3)	Font_Height_X3();
	if(YxN==4)	Font_Height_X4();

	//(*)
	if(ChromaKey==0)
	{	Font_Background_select_Color();	}

	if(ChromaKey==1)
	{	Font_Background_select_Original_Canvas();	}

	//(*)
	if(Alignment==0)
	{	Disable_Font_Alignment();	}

	if(Alignment==1)
	{	Enable_Font_Alignment();	}

} 




void Show_String(char *str)
{  
	Check_Mem_WR_FIFO_Empty();
	
	Text_Mode();
	
	RA8876_CmdWrite(0x04);
	while(*str != '\0')
	{
		Check_Mem_WR_FIFO_not_Full();  
		RA8876_DataWrite(*str);
		++str; 
	} 
	Check_2D_Busy();
	Graphic_Mode(); //back to graphic mode
}



void Draw_Line
(
unsigned long LineColor 
/*LineColor : Set Draw Line color. Line Color dataformat :
ColorDepth_8bpp : R3G3B2�BColorDepth_16bpp : R5G6B5�BColorDepth_24bpp : R8G8B8*/
,unsigned short X1 //X of point1 coordinate
,unsigned short Y1 //Y of point1 coordinate
,unsigned short X2 //X of point2 coordinate
,unsigned short Y2 // Y of point2 coordinate
)
{

#ifdef MCU_8bit_ColorDepth_8bpp	
  Foreground_color_256(LineColor);
#endif
#ifdef MCU_8bit_ColorDepth_16bpp	
  Foreground_color_65k(LineColor);
#endif
#ifdef MCU_8bit_ColorDepth_24bpp	
  Foreground_color_16M(LineColor);
#endif
#ifdef MCU_16bit_ColorDepth_16bpp	
  Foreground_color_65k(LineColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_1	
  Foreground_color_16M(LineColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_2	
  Foreground_color_16M(LineColor);
#endif
 Line_Start_XY(X1,Y1);
 Line_End_XY(X2,Y2);
 Start_Line();
 Check_2D_Busy(); 
}



void Draw_Triangle
(
unsigned long ForegroundColor 
/*ForegroundColor: Set Draw Triangle color. ForegroundColor Color dataformat :
ColorDepth_8bpp : R3G3B2�BColorDepth_16bpp : R5G6B5�BColorDepth_24bpp : R8G8B8*/
,unsigned short X1 //X of point1 coordinate
,unsigned short Y1 //Y of point1 coordinate
,unsigned short X2 //X of point2 coordinate
,unsigned short Y2 //Y of point2 coordinate
,unsigned short X3 //X of point3 coordinate
,unsigned short Y3 //Y of point3 coordinate
)
{ 
#ifdef MCU_8bit_ColorDepth_8bpp	
  Foreground_color_256(ForegroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_16bpp	
  Foreground_color_65k(ForegroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_24bpp	
  Foreground_color_16M(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_16bpp	
  Foreground_color_65k(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_1	
  Foreground_color_16M(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_2	
  Foreground_color_16M(ForegroundColor);
#endif
  Triangle_Point1_XY(X1,Y1);
  Triangle_Point2_XY(X2,Y2);
  Triangle_Point3_XY(X3,Y3);
  Start_Triangle();
  Check_2D_Busy(); 
}


void Draw_Triangle_Fill
(
unsigned long ForegroundColor 
/*ForegroundColor: Set Draw Triangle color. ForegroundColor Color dataformat :
ColorDepth_8bpp : R3G3B2�BColorDepth_16bpp : R5G6B5�BColorDepth_24bpp : R8G8B8*/
,unsigned short X1 //X of point1 coordinate
,unsigned short Y1 //Y of point1 coordinate
,unsigned short X2 //X of point2 coordinate
,unsigned short Y2 //Y of point2 coordinate
,unsigned short X3 //X of point3 coordinate
,unsigned short Y3 //Y of point3 coordinate
)
{
#ifdef MCU_8bit_ColorDepth_8bpp	
  Foreground_color_256(ForegroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_16bpp	
  Foreground_color_65k(ForegroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_24bpp	
  Foreground_color_16M(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_16bpp	
  Foreground_color_65k(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_1	
  Foreground_color_16M(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_2	
  Foreground_color_16M(ForegroundColor);
#endif
  Triangle_Point1_XY(X1,Y1);
  Triangle_Point2_XY(X2,Y2);
  Triangle_Point3_XY(X3,Y3);
  Start_Triangle_Fill();
  Check_2D_Busy(); 
}



void Draw_Circle
(
unsigned long ForegroundColor //ForegroundColor: Set Draw Circle or Circle Fill color
/*ForegroundColor Color dataformat :
ColorDepth_8bpp : R3G3B2�BColorDepth_16bpp : R5G6B5�BColorDepth_24bpp : R8G8B8*/
,unsigned short XCenter //coordinate X of Center
,unsigned short YCenter //coordinate Y of Center
,unsigned short R //Circle Radius
)
{
#ifdef MCU_8bit_ColorDepth_8bpp	
  Foreground_color_256(ForegroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_16bpp	
  Foreground_color_65k(ForegroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_24bpp	
  Foreground_color_16M(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_16bpp	
  Foreground_color_65k(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_1	
  Foreground_color_16M(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_2	
  Foreground_color_16M(ForegroundColor);
#endif
  Circle_Center_XY(XCenter,YCenter);
  Circle_Radius_R(R);
  Start_Circle_or_Ellipse();
  Check_2D_Busy(); 
}


void Draw_Circle_Fill
(
unsigned long ForegroundColor 
/*ForegroundColor: Set Draw Circle or Circle Fill color 
ForegroundColor Color dataformat :
ColorDepth_8bpp : R3G3B2�BColorDepth_16bpp : R5G6B5�BColorDepth_24bpp : R8G8B8*/
,unsigned short XCenter //coordinate X of Center
,unsigned short YCenter //coordinate Y of Center
,unsigned short R //Circle Radius
)
{
#ifdef MCU_8bit_ColorDepth_8bpp	
  Foreground_color_256(ForegroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_16bpp	
  Foreground_color_65k(ForegroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_24bpp	
  Foreground_color_16M(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_16bpp	
  Foreground_color_65k(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_1	
  Foreground_color_16M(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_2	
  Foreground_color_16M(ForegroundColor);
#endif
  Circle_Center_XY(XCenter,YCenter);
  Circle_Radius_R(R);
  Start_Circle_or_Ellipse_Fill();
  Check_2D_Busy(); 
}



void Draw_Ellipse
(
unsigned long ForegroundColor //ForegroundColor : Set Draw Ellipse or Ellipse Fill color
/*ForegroundColor Color dataformat :
ColorDepth_8bpp : R3G3B2�BColorDepth_16bpp : R5G6B5�BColorDepth_24bpp : R8G8B8*/
,unsigned short XCenter //coordinate X of Center
,unsigned short YCenter //coordinate Y of Center
,unsigned short X_R // Radius Width of Ellipse
,unsigned short Y_R // Radius Length of Ellipse
)
{
#ifdef MCU_8bit_ColorDepth_8bpp	
  Foreground_color_256(ForegroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_16bpp	
  Foreground_color_65k(ForegroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_24bpp	
  Foreground_color_16M(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_16bpp	
  Foreground_color_65k(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_1	
  Foreground_color_16M(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_2	
  Foreground_color_16M(ForegroundColor);
#endif
  Ellipse_Center_XY(XCenter,YCenter);
  Ellipse_Radius_RxRy(X_R,Y_R);
  Start_Circle_or_Ellipse();
  Check_2D_Busy(); 
}



void Draw_Ellipse_Fill
(
unsigned long ForegroundColor //ForegroundColor : Set Draw Ellipse or Ellipse Fill color
/*ForegroundColor Color dataformat :
ColorDepth_8bpp : R3G3B2�BColorDepth_16bpp : R5G6B5�BColorDepth_24bpp : R8G8B8*/
,unsigned short XCenter //coordinate X of Center
,unsigned short YCenter //coordinate Y of Center
,unsigned short X_R // Radius Width of Ellipse
,unsigned short Y_R // Radius Length of Ellipse
)				
{
#ifdef MCU_8bit_ColorDepth_8bpp	
  Foreground_color_256(ForegroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_16bpp	
  Foreground_color_65k(ForegroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_24bpp	
  Foreground_color_16M(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_16bpp	
  Foreground_color_65k(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_1	
  Foreground_color_16M(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_2	
  Foreground_color_16M(ForegroundColor);
#endif
  Ellipse_Center_XY(XCenter,YCenter);
  Ellipse_Radius_RxRy(X_R,Y_R);
  Start_Circle_or_Ellipse_Fill();
  Check_2D_Busy(); 
}




void Draw_Left_Up_Curve
(
unsigned long ForegroundColor //ForegroundColor: Set Curve or Curve Fill color 
/*ForegroundColor Color dataformat :
ColorDepth_8bpp : R3G3B2�BColorDepth_16bpp : R5G6B5�BColorDepth_24bpp : R8G8B8*/
,unsigned short XCenter //coordinate X of Center
,unsigned short YCenter //coordinate Y of Center
,unsigned short X_R // Radius Width of Curve
,unsigned short Y_R // Radius Length of Curve
)
{
#ifdef MCU_8bit_ColorDepth_8bpp	
  Foreground_color_256(ForegroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_16bpp	
  Foreground_color_65k(ForegroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_24bpp	
  Foreground_color_16M(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_16bpp	
  Foreground_color_65k(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_1	
  Foreground_color_16M(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_2	
  Foreground_color_16M(ForegroundColor);
#endif
 Ellipse_Center_XY(XCenter,YCenter);
 Ellipse_Radius_RxRy(X_R,Y_R);
 Start_Left_Up_Curve();
 Check_2D_Busy(); 
}



void Draw_Left_Up_Curve_Fill
(
unsigned long ForegroundColor //ForegroundColor: Set Curve or Curve Fill color 
/*ForegroundColor Color dataformat :
ColorDepth_8bpp : R3G3B2�BColorDepth_16bpp : R5G6B5�BColorDepth_24bpp : R8G8B8*/
,unsigned short XCenter //coordinate X of Center
,unsigned short YCenter //coordinate Y of Center
,unsigned short X_R // Radius Width of Curve
,unsigned short Y_R // Radius Length of Curve
)
{
#ifdef MCU_8bit_ColorDepth_8bpp	
  Foreground_color_256(ForegroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_16bpp	
  Foreground_color_65k(ForegroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_24bpp	
  Foreground_color_16M(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_16bpp	
  Foreground_color_65k(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_1	
  Foreground_color_16M(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_2	
  Foreground_color_16M(ForegroundColor);
#endif
  Ellipse_Center_XY(XCenter,YCenter);
  Ellipse_Radius_RxRy(X_R,Y_R);
  Start_Left_Up_Curve_Fill();
  Check_2D_Busy(); 
}




void Draw_Right_Down_Curve
(
unsigned long ForegroundColor //ForegroundColor: Set Curve or Curve Fill color 
/*ForegroundColor Color dataformat :
ColorDepth_8bpp : R3G3B2�BColorDepth_16bpp : R5G6B5�BColorDepth_24bpp : R8G8B8*/
,unsigned short XCenter //coordinate X of Center
,unsigned short YCenter //coordinate Y of Center
,unsigned short X_R // Radius Width of Curve
,unsigned short Y_R // Radius Length of Curve
) 
{
#ifdef MCU_8bit_ColorDepth_8bpp	
  Foreground_color_256(ForegroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_16bpp	
  Foreground_color_65k(ForegroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_24bpp	
  Foreground_color_16M(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_16bpp	
  Foreground_color_65k(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_1	
  Foreground_color_16M(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_2	
  Foreground_color_16M(ForegroundColor);
#endif
  Ellipse_Center_XY(XCenter,YCenter);
  Ellipse_Radius_RxRy(X_R,Y_R);
  Start_Right_Down_Curve();
  Check_2D_Busy(); 
}



void Draw_Right_Down_Curve_Fill
(
unsigned long ForegroundColor //ForegroundColor: Set Curve or Curve Fill color 
/*ForegroundColor Color dataformat :
ColorDepth_8bpp : R3G3B2�BColorDepth_16bpp : R5G6B5�BColorDepth_24bpp : R8G8B8*/
,unsigned short XCenter //coordinate X of Center
,unsigned short YCenter //coordinate Y of Center
,unsigned short X_R // Radius Width of Curve
,unsigned short Y_R // Radius Length of Curve
)
{
#ifdef MCU_8bit_ColorDepth_8bpp	
  Foreground_color_256(ForegroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_16bpp	
  Foreground_color_65k(ForegroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_24bpp	
  Foreground_color_16M(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_16bpp	
  Foreground_color_65k(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_1	
  Foreground_color_16M(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_2	
  Foreground_color_16M(ForegroundColor);
#endif
  Ellipse_Center_XY(XCenter,YCenter);
  Ellipse_Radius_RxRy(X_R,Y_R);
  Start_Right_Down_Curve_Fill();
  Check_2D_Busy(); 
}



void Draw_Right_Up_Curve
(
unsigned long ForegroundColor //ForegroundColor: Set Curve or Curve Fill color 
/*ForegroundColor Color dataformat :
ColorDepth_8bpp : R3G3B2�BColorDepth_16bpp : R5G6B5�BColorDepth_24bpp : R8G8B8*/
,unsigned short XCenter //coordinate X of Center
,unsigned short YCenter //coordinate Y of Center
,unsigned short X_R // Radius Width of Curve
,unsigned short Y_R // Radius Length of Curve
)
{
#ifdef MCU_8bit_ColorDepth_8bpp	
  Foreground_color_256(ForegroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_16bpp	
  Foreground_color_65k(ForegroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_24bpp	
  Foreground_color_16M(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_16bpp	
  Foreground_color_65k(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_1	
  Foreground_color_16M(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_2	
  Foreground_color_16M(ForegroundColor);
#endif
  Ellipse_Center_XY(XCenter,YCenter);
  Ellipse_Radius_RxRy(X_R,Y_R);
  Start_Right_Up_Curve();
  Check_2D_Busy(); 
}



void Draw_Right_Up_Curve_Fill
(
unsigned long ForegroundColor //ForegroundColor: Set Curve or Curve Fill color 
/*ForegroundColor Color dataformat :
ColorDepth_8bpp : R3G3B2�BColorDepth_16bpp : R5G6B5�BColorDepth_24bpp : R8G8B8*/
,unsigned short XCenter //coordinate X of Center
,unsigned short YCenter //coordinate Y of Center
,unsigned short X_R // Radius Width of Curve
,unsigned short Y_R // Radius Length of Curve
)
{
#ifdef MCU_8bit_ColorDepth_8bpp	
  Foreground_color_256(ForegroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_16bpp	
  Foreground_color_65k(ForegroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_24bpp	
  Foreground_color_16M(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_16bpp	
  Foreground_color_65k(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_1	
  Foreground_color_16M(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_2	
  Foreground_color_16M(ForegroundColor);
#endif
  Ellipse_Center_XY(XCenter,YCenter);
  Ellipse_Radius_RxRy(X_R,Y_R);
  Start_Right_Up_Curve_Fill();
  Check_2D_Busy();
}



void Draw_Left_Down_Curve
(
unsigned long ForegroundColor //ForegroundColor: Set Curve or Curve Fill color 
/*ForegroundColor Color dataformat :
ColorDepth_8bpp : R3G3B2�BColorDepth_16bpp : R5G6B5�BColorDepth_24bpp : R8G8B8*/
,unsigned short XCenter //coordinate X of Center
,unsigned short YCenter //coordinate Y of Center
,unsigned short X_R // Radius Width of Curve
,unsigned short Y_R // Radius Length of Curve
)
{
#ifdef MCU_8bit_ColorDepth_8bpp	
  Foreground_color_256(ForegroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_16bpp	
  Foreground_color_65k(ForegroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_24bpp	
  Foreground_color_16M(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_16bpp	
  Foreground_color_65k(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_1	
  Foreground_color_16M(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_2	
  Foreground_color_16M(ForegroundColor);
#endif
  Ellipse_Center_XY(XCenter,YCenter);
  Ellipse_Radius_RxRy(X_R,Y_R);
  Start_Left_Down_Curve();
  Check_2D_Busy(); 
}



void Draw_Left_Down_Curve_Fill
(
unsigned long ForegroundColor //ForegroundColor: Set Curve or Curve Fill color 
/*ForegroundColor Color dataformat :
ColorDepth_8bpp : R3G3B2�BColorDepth_16bpp : R5G6B5�BColorDepth_24bpp : R8G8B8*/
,unsigned short XCenter //coordinate X of Center
,unsigned short YCenter //coordinate Y of Center
,unsigned short X_R // Radius Width of Curve
,unsigned short Y_R // Radius Length of Curve
)
{
#ifdef MCU_8bit_ColorDepth_8bpp	
  Foreground_color_256(ForegroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_16bpp	
  Foreground_color_65k(ForegroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_24bpp	
  Foreground_color_16M(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_16bpp	
  Foreground_color_65k(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_1	
  Foreground_color_16M(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_2	
  Foreground_color_16M(ForegroundColor);
#endif
  Ellipse_Center_XY(XCenter,YCenter);
  Ellipse_Radius_RxRy(X_R,Y_R);
  Start_Left_Down_Curve_Fill();
  Check_2D_Busy(); 
}


void Draw_Square
(
unsigned long ForegroundColor 
/*ForegroundColor: Set Curve or Curve Fill color. ForegroundColor Color dataformat :
ColorDepth_8bpp : R3G3B2�BColorDepth_16bpp : R5G6B5�BColorDepth_24bpp : R8G8B8*/
,unsigned short X1 //X of point1 coordinate
,unsigned short Y1 //Y of point1 coordinate
,unsigned short X2 //X of point2 coordinate
,unsigned short Y2 //Y of point2 coordinate
)
{
#ifdef MCU_8bit_ColorDepth_8bpp	
  Foreground_color_256(ForegroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_16bpp	
  Foreground_color_65k(ForegroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_24bpp	
  Foreground_color_16M(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_16bpp	
  Foreground_color_65k(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_1	
  Foreground_color_16M(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_2	
  Foreground_color_16M(ForegroundColor);
#endif
  Square_Start_XY(X1,Y1);
  Square_End_XY(X2,Y2);
  Start_Square();
  Check_2D_Busy(); 
}


void Draw_Square_Fill
(
unsigned long ForegroundColor 
/*ForegroundColor: Set Curve or Curve Fill color. ForegroundColor Color dataformat :
ColorDepth_8bpp : R3G3B2�BColorDepth_16bpp : R5G6B5�BColorDepth_24bpp : R8G8B8*/
,unsigned short X1 //X of point1 coordinate
,unsigned short Y1 //Y of point1 coordinate
,unsigned short X2 //X of point2 coordinate
,unsigned short Y2 //Y of point2 coordinate
)
{
#ifdef MCU_8bit_ColorDepth_8bpp	
  Foreground_color_256(ForegroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_16bpp	
  Foreground_color_65k(ForegroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_24bpp	
  Foreground_color_16M(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_16bpp	
  Foreground_color_65k(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_1	
  Foreground_color_16M(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_2	
  Foreground_color_16M(ForegroundColor);
#endif
  Square_Start_XY(X1,Y1);
  Square_End_XY(X2,Y2);
  Start_Square_Fill();
  Check_2D_Busy(); 
}


/*
*********************************************************************************************************
*	�� �� ��: Draw_Circle_Square
*	����˵��: ����Բ�Ǿ���
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void Draw_Circle_Square
(
unsigned long ForegroundColor 
/*ForegroundColor : Set Circle Square color. ForegroundColor Color dataformat :
ColorDepth_8bpp : R3G3B2�BColorDepth_16bpp : R5G6B5�BColorDepth_24bpp : R8G8B8*/
,unsigned short X1 //X of point1 coordinate
,unsigned short Y1 //Y of point1 coordinate
,unsigned short X2 //X of point2 coordinate
,unsigned short Y2 //Y of point2 coordinate
,unsigned short X_R //Radius Width of Circle Square
,unsigned short Y_R //Radius Length of Circle Square
)
{
#ifdef MCU_8bit_ColorDepth_8bpp	
  Foreground_color_256(ForegroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_16bpp	
  Foreground_color_65k(ForegroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_24bpp	
  Foreground_color_16M(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_16bpp	
  Foreground_color_65k(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_1	
  Foreground_color_16M(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_2	
  Foreground_color_16M(ForegroundColor);
#endif 
  Square_Start_XY(X1,Y1);
  Square_End_XY(X2,Y2); 
  Circle_Square_Radius_RxRy(X_R,Y_R);
  Start_Circle_Square();
  Check_2D_Busy(); 
}



/*
*********************************************************************************************************
*	�� �� ��: Draw_Circle_Square_Fill
*	����˵��: ���Բ�Ǿ���
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void Draw_Circle_Square_Fill
(
unsigned long ForegroundColor 
/*ForegroundColor : Set Circle Square color. ForegroundColor Color dataformat :
ColorDepth_8bpp : R3G3B2�BColorDepth_16bpp : R5G6B5�BColorDepth_24bpp : R8G8B8*/
,unsigned short X1 //X of point1 coordinate
,unsigned short Y1 //Y of point1 coordinate
,unsigned short X2 //X of point2 coordinate
,unsigned short Y2 //Y of point2 coordinate
,unsigned short X_R //Radius Width of Circle Square
,unsigned short Y_R //Radius Length of Circle Square
)
{

#ifdef MCU_8bit_ColorDepth_8bpp	
  Foreground_color_256(ForegroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_16bpp	
  Foreground_color_65k(ForegroundColor);
#endif
#ifdef MCU_8bit_ColorDepth_24bpp	
  Foreground_color_16M(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_16bpp	
  Foreground_color_65k(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_1	
  Foreground_color_16M(ForegroundColor);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_2	
  Foreground_color_16M(ForegroundColor);
#endif
  Square_Start_XY(X1,Y1);
  Square_End_XY(X2,Y2); 
  Circle_Square_Radius_RxRy(X_R,Y_R);
  Start_Circle_Square_Fill();
  Check_2D_Busy(); 
}



void BTE_Pattern_Fill
(
unsigned char P_8x8_or_16x16 //0 : use 8x8 Icon , 1 : use 16x16 Icon.
,unsigned long S0_Addr //Start address of Source 0
,unsigned short S0_W //image width of Source 0 (recommend = canvas image width)
,unsigned short XS0 // coordinate X of Source 0
,unsigned short YS0 // coordinate Y of Source 0
,unsigned long S1_Addr //Start address of Source 1
,unsigned short S1_W //image width of Source 1 (recommend = canvas image width)
,unsigned short XS1 //coordinate X of Source 1
,unsigned short YS1 //coordinate Y of Source 1
,unsigned long Des_Addr // start address of Destination
,unsigned short Des_W //image width of Destination (recommend = canvas image width)
, unsigned short XDes //coordinate X of Destination
,unsigned short YDes //coordinate Y of Destination
,unsigned int ROP_Code 
/*ROP_Code :
   0000b		0(Blackness)
   0001b		~S0!E~S1 or ~(S0+S1)
   0010b		~S0!ES1
   0011b		~S0
   0100b		S0!E~S1
   0101b		~S1
   0110b		S0^S1
   0111b		~S0 + ~S1 or ~(S0 + S1)
   1000b		S0!ES1
   1001b		~(S0^S1)
   1010b		S1
   1011b		~S0+S1
   1100b		S0
   1101b		S0+~S1
   1110b		S0+S1
   1111b		1(whiteness)*/
,unsigned short X_W //Width of BTE Winodw
,unsigned short Y_H //Length of BTE Winodw
)
{
  if(P_8x8_or_16x16 == 0)
  {
  Pattern_Format_8X8();
   }
  if(P_8x8_or_16x16 == 1)
  {														    
  Pattern_Format_16X16();
  }	  
  BTE_S0_Memory_Start_Address(S0_Addr);
  BTE_S0_Image_Width(S0_W);
  BTE_S0_Window_Start_XY(XS0,YS0);

  BTE_S1_Memory_Start_Address(S1_Addr);
  BTE_S1_Image_Width(S1_W); 
  BTE_S1_Window_Start_XY(XS1,YS1);

  BTE_Destination_Memory_Start_Address(Des_Addr);
  BTE_Destination_Image_Width(Des_W);
  BTE_Destination_Window_Start_XY(XDes,YDes);	
   
  BTE_ROP_Code(ROP_Code);	
  BTE_Operation_Code(0x06); //BTE Operation: Pattern Fill with ROP.
  BTE_Window_Size(X_W,Y_H); 
  BTE_Enable();
  Check_BTE_Busy();
}

void BTE_Pattern_Fill_With_Chroma_key
(
unsigned char P_8x8_or_16x16 //0 : use 8x8 Icon , 1 : use 16x16 Icon.
,unsigned long S0_Addr //Start address of Source 0
,unsigned short S0_W //image width of Source 0 (recommend = canvas image width)
,unsigned short XS0 //coordinate X of Source 0
,unsigned short YS0 //coordinate Y of Source 0
,unsigned long S1_Addr //Start address of Source 1
,unsigned short S1_W //image width of Source 1 (recommend = canvas image width)
,unsigned short XS1 //coordinate X of Source 1
,unsigned short YS1 //coordinate Y of Source 1
,unsigned long Des_Addr //Des_Addr : start address of Destination
,unsigned short Des_W //Des_W : image width of Destination (recommend = canvas image width)
,unsigned short XDes //coordinate X of Destination
,unsigned short YDes //coordinate Y of Destination
,unsigned int ROP_Code
/*ROP_Code :
   0000b		0(Blackness)
   0001b		~S0!E~S1 or ~(S0+S1)
   0010b		~S0!ES1
   0011b		~S0
   0100b		S0!E~S1
   0101b		~S1
   0110b		S0^S1
   0111b		~S0 + ~S1 or ~(S0 + S1)
   1000b		S0!ES1
   1001b		~(S0^S1)
   1010b		S1
   1011b		~S0+S1
   1100b		S0
   1101b		S0+~S1
   1110b		S0+S1
   1111b		1(whiteness)*/
,unsigned long Background_color //Transparent color
,unsigned short X_W //Width of BTE Window
,unsigned short Y_H //Length of BTE Window
)
{
#ifdef MCU_8bit_ColorDepth_8bpp	
  Background_color_256(Background_color); 
#endif
#ifdef MCU_8bit_ColorDepth_16bpp	
  Background_color_65k(Background_color); 
#endif
#ifdef MCU_8bit_ColorDepth_24bpp	
  Background_color_16M(Background_color); 
#endif
#ifdef MCU_16bit_ColorDepth_16bpp	
  Background_color_65k(Background_color); 
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_1	
  Background_color_16M(Background_color); 
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_2	
  Background_color_16M(Background_color); 
#endif
  if(P_8x8_or_16x16 == 0)
  {
  Pattern_Format_8X8();
   }
  if(P_8x8_or_16x16 == 1)
  {														    
  Pattern_Format_16X16();
  }	  
  BTE_S0_Memory_Start_Address(S0_Addr);
  BTE_S0_Image_Width(S0_W);
  BTE_S0_Window_Start_XY(XS0,YS0);

  BTE_S1_Memory_Start_Address(S1_Addr);
  BTE_S1_Image_Width(S1_W); 
  BTE_S1_Window_Start_XY(XS1,YS1);

  BTE_Destination_Memory_Start_Address(Des_Addr);
  BTE_Destination_Image_Width(Des_W);
  BTE_Destination_Window_Start_XY(XDes,YDes);	
   
  BTE_ROP_Code(ROP_Code);	
  BTE_Operation_Code(0x07); //BTE Operation: Pattern Fill with Chroma key.
  BTE_Window_Size(X_W,Y_H); 
  BTE_Enable();
  Check_BTE_Busy();
}

void BTE_Memory_Copy
(
unsigned long S0_Addr //Start address of Source 0
,unsigned short S0_W //image width of Source 0 (recommend = canvas image width)
,unsigned short XS0 //coordinate X of Source 0
,unsigned short YS0 //coordinate Y of Source 0
,unsigned long S1_Addr //Start address of Source 1
,unsigned short S1_W //image width of Source 1 (recommend = canvas image width)
,unsigned short XS1 //coordinate X of Source 1
,unsigned short YS1 //coordinate Y of Source 1
,unsigned long Des_Addr //start address of Destination
,unsigned short Des_W //image width of Destination (recommend = canvas image width)
,unsigned short XDes //coordinate X of Destination
,unsigned short YDes //coordinate Y of Destination
,unsigned int ROP_Code
/*ROP_Code :
   0000b		0(Blackness)
   0001b		~S0!E~S1 or ~(S0+S1)
   0010b		~S0!ES1
   0011b		~S0
   0100b		S0!E~S1
   0101b		~S1
   0110b		S0^S1
   0111b		~S0 + ~S1 or ~(S0 + S1)
   1000b		S0!ES1
   1001b		~(S0^S1)
   1010b		S1
   1011b		~S0+S1
   1100b		S0
   1101b		S0+~S1
   1110b		S0+S1
   1111b		1(whiteness)*/
,unsigned short X_W //X_W : Width of BTE Window
,unsigned short Y_H //Y_H : Length of BTE Window
)
{
  BTE_S0_Memory_Start_Address(S0_Addr);
  BTE_S0_Image_Width(S0_W);
  BTE_S0_Window_Start_XY(XS0,YS0);

  BTE_S1_Memory_Start_Address(S1_Addr);
  BTE_S1_Image_Width(S1_W); 
  BTE_S1_Window_Start_XY(XS1,YS1);

  BTE_Destination_Memory_Start_Address(Des_Addr);
  BTE_Destination_Image_Width(Des_W);
  BTE_Destination_Window_Start_XY(XDes,YDes);	
   
  BTE_ROP_Code(ROP_Code);	
  BTE_Operation_Code(0x02); //BTE Operation: Memory copy (move) with ROP.
  BTE_Window_Size(X_W,Y_H); 
  BTE_Enable();
  Check_BTE_Busy();
}






void BTE_Memory_Copy_Chroma_key
(
unsigned long S0_Addr //Start address of Source 0
,unsigned short S0_W //image width of Source 0 (recommend = canvas image width) 
,unsigned short XS0 //coordinate X of Source 0
,unsigned short YS0 //coordinate Y of Source 0
,unsigned long Des_Addr //start address of Destination
,unsigned short Des_W //image width of Destination (recommend = canvas image width) 
,unsigned short XDes //coordinate X of Destination
,unsigned short YDes //coordinate Y of Destination
,unsigned long Background_color // transparent color
,unsigned short X_W //Width of BTE Window
,unsigned short Y_H //Length of BTE Window
)
{

#ifdef MCU_8bit_ColorDepth_8bpp	
  Background_color_256(Background_color); 
#endif
#ifdef MCU_8bit_ColorDepth_16bpp	
  Background_color_65k(Background_color); 
#endif
#ifdef MCU_8bit_ColorDepth_24bpp	
  Background_color_16M(Background_color); 
#endif
#ifdef MCU_16bit_ColorDepth_16bpp	
  Background_color_65k(Background_color); 
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_1	
  Background_color_16M(Background_color); 
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_2	
  Background_color_16M(Background_color); 
#endif
  BTE_S0_Memory_Start_Address(S0_Addr);
  BTE_S0_Image_Width(S0_W);
  BTE_S0_Window_Start_XY(XS0,YS0);	

  BTE_Destination_Memory_Start_Address(Des_Addr);
  BTE_Destination_Image_Width(Des_W);
  BTE_Destination_Window_Start_XY(XDes,YDes);
   
  BTE_Operation_Code(0x05);	//BTE Operation: Memory copy (move) with chroma keying (w/o ROP)
  BTE_Window_Size(X_W,Y_H); 
  BTE_Enable();
  Check_BTE_Busy();
}


void BTE_MCU_Write_MCU_8bit
(
unsigned long S1_Addr //Start address of Source 1
,unsigned short S1_W //image width of Source 1 (recommend = canvas image width)
,unsigned short XS1 //coordinate X of Source 1
,unsigned short YS1 //coordinate Y of Source 1
,unsigned long Des_Addr //start address of Destination
,unsigned short Des_W //image width of Destination (recommend = canvas image width)
,unsigned short XDes //coordinate X of Destination
,unsigned short YDes //coordinate Y of Destination
,unsigned int ROP_Code
/*ROP_Code :
   0000b		0(Blackness)
   0001b		~S0!E~S1 or ~(S0+S1)
   0010b		~S0!ES1
   0011b		~S0
   0100b		S0!E~S1
   0101b		~S1
   0110b		S0^S1
   0111b		~S0 + ~S1 or ~(S0 + S1)
   1000b		S0!ES1
   1001b		~(S0^S1)
   1010b		S1
   1011b		~S0+S1
   1100b		S0
   1101b		S0+~S1
   1110b		S0+S1
   1111b		1(whiteness)*/
,unsigned short X_W // Width of BTE Window
,unsigned short Y_H // Length of BTE Window
,const unsigned char *data // 8-bit data
)
{		
//  unsigned short i,j;

  BTE_S1_Memory_Start_Address(S1_Addr);
  BTE_S1_Image_Width(S1_W); 
  BTE_S1_Window_Start_XY(XS1,YS1);

  BTE_Destination_Memory_Start_Address(Des_Addr);
  BTE_Destination_Image_Width(Des_W);
  BTE_Destination_Window_Start_XY(XDes,YDes);
  
  BTE_Window_Size(X_W,Y_H);
  BTE_ROP_Code(ROP_Code);
  BTE_Operation_Code(0x00);		//BTE Operation: MPU Write with ROP.
  BTE_Enable();
  RA8876_CmdWrite(0x04);			//Memory Data Read/Write Port

#ifdef MCU_8bit_ColorDepth_8bpp	    			 		
	for(i=0;i< Y_H;i++)
    {	
	  for(j=0;j< (X_W);j++)
 	  {
	   Check_Mem_WR_FIFO_not_Full();
	   RA8876_DataWrite(*data);
	   data++;
	  }
    }
  Check_Mem_WR_FIFO_Empty();
#endif

#ifdef MCU_8bit_ColorDepth_16bpp	
	for(i=0;i< Y_H;i++)
    {	
	  for(j=0;j< (X_W*2);j++)
 	  {
	   Check_Mem_WR_FIFO_not_Full();
	   RA8876_DataWrite(*data);
	   data++;
	  }
    }
  Check_Mem_WR_FIFO_Empty();
#endif

#ifdef MCU_8bit_ColorDepth_24bpp	
	for(i=0;i< Y_H;i++)
    {	
	  for(j=0;j< (X_W*3);j++)
 	  {
	   Check_Mem_WR_FIFO_not_Full();
	   RA8876_DataWrite(*data);
	   data++;
	  }
    }
  Check_Mem_WR_FIFO_Empty();
#endif

}


void BTE_MCU_Write_MCU_16bit
(
unsigned long S1_Addr //Start address of Source 1
,unsigned short S1_W //image width of Source 1 (recommend = canvas image width)
,unsigned short XS1 //coordinate X of Source 1
,unsigned short YS1 //coordinate Y of Source 1
,unsigned long Des_Addr //start address of Destination
,unsigned short Des_W //image width of Destination (recommend = canvas image width)
,unsigned short XDes //coordinate X of Destination
,unsigned short YDes //coordinate Y of Destination
,unsigned int ROP_Code
/*ROP_Code :
   0000b		0(Blackness)
   0001b		~S0!E~S1 or ~(S0+S1)
   0010b		~S0!ES1
   0011b		~S0
   0100b		S0!E~S1
   0101b		~S1
   0110b		S0^S1
   0111b		~S0 + ~S1 or ~(S0 + S1)
   1000b		S0!ES1
   1001b		~(S0^S1)
   1010b		S1
   1011b		~S0+S1
   1100b		S0
   1101b		S0+~S1
   1110b		S0+S1
   1111b		1(whiteness)*/
,unsigned short X_W // Width of BTE Window
,unsigned short Y_H // Length of BTE Window
,const unsigned short *data // 16-bit data 
)
{	
  unsigned short i,j;

  BTE_S1_Memory_Start_Address(S1_Addr);
  BTE_S1_Image_Width(S1_W); 
  BTE_S1_Window_Start_XY(XS1,YS1);

  BTE_Destination_Memory_Start_Address(Des_Addr);
  BTE_Destination_Image_Width(Des_W);
  BTE_Destination_Window_Start_XY(XDes,YDes);
  
  BTE_Window_Size(X_W,Y_H);
  BTE_ROP_Code(ROP_Code);
  BTE_Operation_Code(0x00);		//BTE Operation: MPU Write with ROP.
  BTE_Enable();
  RA8876_CmdWrite(0x04);				 		//Memory Data Read/Write Port
#ifdef MCU_16bit_ColorDepth_16bpp	
	for(i=0;i< Y_H;i++)
    {	
	  for(j=0;j< (X_W);j++)
 	  {
	   Check_Mem_WR_FIFO_not_Full();
	   RA8876_DataWrite((*data));
	   data++;
	  }
    }
  Check_Mem_WR_FIFO_Empty();
#endif

#ifdef MCU_16bit_ColorDepth_24bpp_Mode_1	
	for(i=0;i< Y_H;i++)
    {	
	  for(j=0;j< (X_W/2);j++)
 	  {
	    Check_Mem_WR_FIFO_not_Full();
	    RA8876_DataWrite(*data);
	    data++;
	    Check_Mem_WR_FIFO_not_Full();
	    RA8876_DataWrite(*data);
	    data++;
	    Check_Mem_WR_FIFO_not_Full();
	    RA8876_DataWrite(*data);
	    data++;
	  }
    }
  Check_Mem_WR_FIFO_Empty();
#endif
  

#ifdef MCU_16bit_ColorDepth_24bpp_Mode_2	
	for(i=0;i< Y_H;i++)
    {	
	  for(j=0;j< (X_W);j++)
 	  {
	    Check_Mem_WR_FIFO_not_Full();
	    RA8876_DataWrite(*data);
	    data++;
	    Check_Mem_WR_FIFO_not_Full();
	    RA8876_DataWrite(*data);
	    data++;
	  }
    }
  Check_Mem_WR_FIFO_Empty();
#endif

  Check_BTE_Busy();
	
}






void BTE_MCU_Write_Chroma_key_MCU_8bit
(
unsigned long Des_Addr //start address of Destination
,unsigned short Des_W //image width of Destination (recommend = canvas image width)
,unsigned short XDes //coordinate X of Destination
,unsigned short YDes //coordinate Y of Destination
,unsigned long Background_color //transparency color
,unsigned short X_W //Width of BTE Window
,unsigned short Y_H //Length of BTE Window
,const unsigned char *data // 8-bit data
)
{
// unsigned short i,j;
	
#ifdef MCU_8bit_ColorDepth_8bpp	
  Background_color_256(Background_color);
#endif

#ifdef MCU_8bit_ColorDepth_16bpp	
  Background_color_65k(Background_color);
#endif

#ifdef MCU_8bit_ColorDepth_24bpp	
  Background_color_16M(Background_color);
#endif
  BTE_Destination_Memory_Start_Address(Des_Addr);
  BTE_Destination_Image_Width(Des_W);
  BTE_Destination_Window_Start_XY(XDes,YDes);
  
  BTE_Window_Size(X_W,Y_H);
  BTE_Operation_Code(0x04);		//BTE Operation: MPU Write w/ chroma keying (w/o ROP)
  BTE_Enable();
  
  RA8876_CmdWrite(0x04);				 		//Memory Data Read/Write Port

#ifdef MCU_8bit_ColorDepth_8bpp	
    			 		
	for(i=0;i< Y_H;i++)
    {	
	  for(j=0;j< (X_W);j++)
 	  {
	   Check_Mem_WR_FIFO_not_Full();
	   RA8876_DataWrite(*data);
	   data++;
	  }
    }
  Check_Mem_WR_FIFO_Empty();
#endif

#ifdef MCU_8bit_ColorDepth_16bpp	
	for(i=0;i< Y_H;i++)
    {	
	  for(j=0;j< (X_W*2);j++)
 	  {
	   Check_Mem_WR_FIFO_not_Full();
	   RA8876_DataWrite(*data);
	   data++;
	  }
    }
  Check_Mem_WR_FIFO_Empty();
#endif

#ifdef MCU_8bit_ColorDepth_24bpp	
	for(i=0;i< Y_H;i++)
    {	
	  for(j=0;j< (X_W*3);j++)
 	  {
	   Check_Mem_WR_FIFO_not_Full();
	   RA8876_DataWrite(*data);
	   data++;
	  }
    }
  Check_Mem_WR_FIFO_Empty();
#endif
	Check_BTE_Busy();
}



void BTE_MCU_Write_Chroma_key_MCU_16bit
(
unsigned long Des_Addr //start address of Destination
,unsigned short Des_W //image width of Destination (recommend = canvas image width)
,unsigned short XDes //coordinate X of Destination
,unsigned short YDes //coordinate Y of Destination
,unsigned long Background_color //transparency color
,unsigned short X_W //Width of BTE Window
,unsigned short Y_H //Length of BTE Window
,const unsigned short *data // 16-bit data
)
{	
  unsigned int i,j;

  
#ifdef MCU_16bit_ColorDepth_16bpp				//setting in UserDef.h
  Background_color_65k(Background_color);
#endif	
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_1		//setting in UserDef.h
  Background_color_16M(Background_color);
#endif	
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_2		//setting in UserDef.h
  Background_color_16M(Background_color);
#endif
  BTE_Destination_Memory_Start_Address(Des_Addr);
  BTE_Destination_Image_Width(Des_W);
  BTE_Destination_Window_Start_XY(XDes,YDes);
  
  BTE_Window_Size(X_W,Y_H);
  BTE_Operation_Code(0x04);		//BTE Operation: MPU Write with chroma keying (w/o ROP)
  BTE_Enable();
  RA8876_CmdWrite(0x04);			//Memory Data Read/Write Port
#ifdef MCU_16bit_ColorDepth_16bpp				//setting in UserDef.h
  				 		
	for(i=0;i< Y_H;i++)
    {	
	  for(j=0;j< X_W;j++)
 	  {
	    Check_Mem_WR_FIFO_not_Full();
		  RA8876_DataWrite(*data);  
	    data++;
	  }
    }
  Check_Mem_WR_FIFO_Empty();
#endif	

#ifdef MCU_16bit_ColorDepth_24bpp_Mode_1		//setting in UserDef.h
	for(i=0;i< Y_H;i++)
    {	
	  for(j=0;j< (X_W/2);j++)
 	  {
	    Check_Mem_WR_FIFO_not_Full();
		  RA8876_DataWrite(*data);  
	    data++;
		  Check_Mem_WR_FIFO_not_Full();
		  RA8876_DataWrite(*data);  
	    data++;
		  Check_Mem_WR_FIFO_not_Full();
		  RA8876_DataWrite(*data);  
	    data++;
	  }
    }
  Check_Mem_WR_FIFO_Empty(); 
#endif	
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_2		//setting in UserDef.h
	for(i=0;i< Y_H;i++)
    {	
	  for(j=0;j< (X_W);j++)
 	  {
	    Check_Mem_WR_FIFO_not_Full();
		  RA8876_DataWrite(*data);  
	    data++;
		  Check_Mem_WR_FIFO_not_Full();
		  RA8876_DataWrite(*data);  
	    data++;

	  }
    }
  Check_Mem_WR_FIFO_Empty();
#endif

	Check_BTE_Busy();

}



/*
No support 24bpp color depth 
S0_Addr : Source 0 Start address
S0_W : Source 0 image width(recommend = canvas image width) 
XS0 : Source 0 coordinate of X
YS0 : Source 0 coordinate of Y
Des_Addr : Destination start address
Des_W :	 Destination image width(recommend = canvas image width) 
XDes : Destination coordinate of X
YDes : Destination coordinate of Y
X_W : BTE Window Size of X
Y_H : BTE Window Size of Y
Foreground_color : The source (1bit map picture) map data 1 translate to Foreground color by color expansion
Background_color : The source (1bit map picture) map data 0 translate to background color by color expansion
*/
void BTE_Memory_Copy_ColorExpansion(unsigned long S0_Addr,unsigned short S0_W,unsigned short XS0,unsigned short YS0,unsigned long Des_Addr,unsigned short Des_W, unsigned short XDes,unsigned short YDes,unsigned short X_W,unsigned short Y_H,unsigned long Foreground_color,unsigned long Background_color)
{

#ifdef MCU_8bit_ColorDepth_8bpp
    Foreground_color_256(Foreground_color);
    Background_color_256(Background_color);
	BTE_ROP_Code(7);	//MCU data bus width: 8bit, ROP Code=7
#endif
#ifdef MCU_8bit_ColorDepth_16bpp
    Foreground_color_65k(Foreground_color);
    Background_color_65k(Background_color);
	BTE_ROP_Code(15);	//MCU data bus width: 8bit, ROP Code=7
#endif
#ifdef MCU_16bit_ColorDepth_16bpp
    Foreground_color_65k(Foreground_color);
    Background_color_65k(Background_color);
	BTE_ROP_Code(15);	//MCU data bus width: 16bit, ROP Code=7
#endif


  BTE_S0_Memory_Start_Address(S0_Addr);
  BTE_S0_Image_Width(S0_W);
  BTE_S0_Window_Start_XY(XS0,YS0);
  
  BTE_Destination_Memory_Start_Address(Des_Addr);
  BTE_Destination_Image_Width(Des_W);
  BTE_Destination_Window_Start_XY(XDes,YDes);
   
  BTE_Window_Size(X_W,Y_H);
  BTE_Operation_Code(0xE);		//BTE Operation: Memory write with Color Expansion (w/o ROP)

#ifdef MCU_8bit_ColorDepth_8bpp
  BTE_Enable();
  Check_BTE_Busy();
#endif
#ifdef MCU_8bit_ColorDepth_16bpp
  BTE_Enable();
  Check_BTE_Busy();
#endif
#ifdef MCU_16bit_ColorDepth_16bpp
  BTE_Enable();
  Check_BTE_Busy();
#endif	

}


/*
No support 24bpp color depth
S0_Addr : Source 0 Start address
S0_W : Source 0 image width(recommend = canvas image width) 
XS0 : Source 0 coordinate of X
YS0 : Source 0 coordinate of Y
Des_Addr : Destination start address
Des_W :	 Destination image width(recommend = canvas image width) 
XDes : Destination coordinate of X
YDes : Destination coordinate of Y
X_W : BTE Window Size of X
Y_H : BTE Window Size of Y
Foreground_color : The source (1bit map picture) map data 1 translate to Foreground color by color expansion
*/
void BTE_Memory_Copy_ColorExpansion_Chroma_key(unsigned long S0_Addr,unsigned short S0_W,unsigned short XS0,unsigned short YS0,unsigned long Des_Addr,unsigned short Des_W, unsigned short XDes,unsigned short YDes,unsigned short X_W,unsigned short Y_H,unsigned long Foreground_color)
{
#ifdef MCU_8bit_ColorDepth_8bpp
    Foreground_color_256(Foreground_color);
	BTE_ROP_Code(7);	//MCU data bus width: 8bit, ROP Code=7
#endif
#ifdef MCU_8bit_ColorDepth_16bpp
    Foreground_color_65k(Foreground_color);
	BTE_ROP_Code(15);	//MCU data bus width: 8bit, ROP Code=7
#endif
#ifdef MCU_16bit_ColorDepth_16bpp
    Foreground_color_65k(Foreground_color);
	BTE_ROP_Code(15);	//MCU data bus width: 16bit, ROP Code=7
#endif

  BTE_S0_Memory_Start_Address(S0_Addr);
  BTE_S0_Image_Width(S0_W);
  BTE_S0_Window_Start_XY(XS0,YS0);

  BTE_Destination_Memory_Start_Address(Des_Addr);
  BTE_Destination_Image_Width(Des_W);
  BTE_Destination_Window_Start_XY(XDes,YDes);
   
  BTE_Window_Size(X_W,Y_H);
  BTE_Operation_Code(0xF);		//BTE Operation: Memory write with Color Expansion and chroma keying (w/o ROP)
	
#ifdef MCU_8bit_ColorDepth_8bpp
  BTE_Enable();
  Check_BTE_Busy();
#endif
#ifdef MCU_8bit_ColorDepth_16bpp
  BTE_Enable();
  Check_BTE_Busy();
#endif
#ifdef MCU_16bit_ColorDepth_16bpp
  BTE_Enable();
  Check_BTE_Busy();
#endif	

}


void BTE_MCU_Write_ColorExpansion_MCU_8bit
(
unsigned long Des_Addr //start address of Destination
,unsigned short Des_W //image width of Destination (recommend = canvas image width)
,unsigned short XDes //coordinate X of Destination
,unsigned short YDes //coordinate Y of Destination
,unsigned short X_W //Width of BTE Window
,unsigned short Y_H //Length of BTE Window
,unsigned long Foreground_color 
/*Foreground_color : The source (1bit map picture) map data 1 translate to Foreground color by color expansion*/
,unsigned long Background_color 
/*Background_color : The source (1bit map picture) map data 0 translate to Foreground color by color expansion*/
,const unsigned char *data // 8-bit data
)
{    
    unsigned short i,j;
	
	/* set BTE Parameters and Run */
#ifdef MCU_8bit_ColorDepth_8bpp
    Foreground_color_256(Foreground_color);
    Background_color_256(Background_color);
#endif
#ifdef MCU_8bit_ColorDepth_16bpp
    Foreground_color_65k(Foreground_color);
    Background_color_65k(Background_color);
#endif
#ifdef MCU_8bit_ColorDepth_24bpp
    Foreground_color_16M(Foreground_color);
    Background_color_16M(Background_color);
#endif 

	BTE_Destination_Memory_Start_Address(Des_Addr);
	BTE_Destination_Image_Width(Des_W);
	BTE_Destination_Window_Start_XY(XDes,YDes);

	BTE_ROP_Code(7);	//MCU data bus width: 8bit, ROP Code=7
	BTE_Window_Size(X_W,Y_H);
	BTE_Operation_Code(0x8);		//BTE Operation: MPU Write with Color Expansion (w/o ROP)

	BTE_Enable();		
		
  RA8876_CmdWrite(0x04);				 		//Memory Data Read/Write Port
    
	for(i=0;i< Y_H;i++)
    {	
	  for(j=0;j< X_W/8;j++)
 	  {
	    Check_Mem_WR_FIFO_not_Full();
		  RA8876_DataWrite(*data);  
	    data++;
	  }
    }
	Check_Mem_WR_FIFO_Empty();
	  
	Check_BTE_Busy();


}

void BTE_MCU_Write_ColorExpansion_MCU_16bit
(
unsigned long Des_Addr //start address of Destination
,unsigned short Des_W //image width of Destination (recommend = canvas image width)
,unsigned short XDes //coordinate X of Destination
,unsigned short YDes //coordinate Y of Destination
,unsigned short X_W //Width of BTE Window
,unsigned short Y_H //Length of BTE Window
,unsigned long Foreground_color 
/*Foreground_color : The source (1bit map picture) map data 1 translate to Foreground color by color expansion*/
,unsigned long Background_color 
/*Background_color : The source (1bit map picture) map data 0 translate to Background color by color expansion*/
,const unsigned short *data //16-bit data�@
)
{ 
  unsigned short i,j;


#ifdef MCU_16bit_ColorDepth_16bpp
	Data_Format_16b_16bpp();
    Foreground_color_65k(Foreground_color);
    Background_color_65k(Background_color);
	BTE_ROP_Code(15);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_1
  Data_Format_16b_8bpp();
  Foreground_color_16M(Foreground_color);
  Background_color_16M(Background_color);
  BTE_ROP_Code(7);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_2
  Data_Format_16b_8bpp();
  Foreground_color_16M(Foreground_color);
  Background_color_16M(Background_color);
  BTE_ROP_Code(7);
#endif

  BTE_Destination_Memory_Start_Address(Des_Addr);
  BTE_Destination_Image_Width(Des_W);
  BTE_Destination_Window_Start_XY(XDes,YDes);

  BTE_Window_Size(X_W,Y_H);
  BTE_Operation_Code(0x8);		//BTE Operation: MPU Write with Color Expansion (w/o ROP)
  BTE_Enable();
	

#ifdef MCU_16bit_ColorDepth_16bpp
  RA8876_CmdWrite(0x04);				 		//Memory Data Read/Write Port  
	for(i=0;i< Y_H;i++)
    {	
	  for(j=0;j< X_W/16;j++)
 	  {
	    Check_Mem_WR_FIFO_not_Full();
		  RA8876_DataWrite(*data);  
	    data++;
	  }
    }
  Check_Mem_WR_FIFO_Empty();
#endif

#ifdef MCU_16bit_ColorDepth_24bpp_Mode_1
	RA8876_CmdWrite(0x04);				 		//Memory Data Read/Write Port 
	for(i=0;i< Y_H;i++)
    {	
	  for(j=0;j< X_W/16;j++)
 	  {
	    Check_Mem_WR_FIFO_not_Full();
			RA8876_DataWrite(*data>>8);  

	    Check_Mem_WR_FIFO_not_Full();
			RA8876_DataWrite(*data);  
	    data++;
	  }
    }
	Check_Mem_WR_FIFO_Empty();
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_2
	RA8876_CmdWrite(0x04);				 		//Memory Data Read/Write Port 
	for(i=0;i< Y_H;i++)
    {	
	  for(j=0;j< X_W/16;j++)
 	  {
	    Check_Mem_WR_FIFO_not_Full();
			RA8876_DataWrite(*data>>8);  

	    Check_Mem_WR_FIFO_not_Full();
			RA8876_DataWrite(*data);  
	    data++;
	  }
    }
	Check_Mem_WR_FIFO_Empty();
#endif	

    Check_BTE_Busy();			

#ifdef MCU_16bit_ColorDepth_16bpp	
	Data_Format_16b_16bpp();
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_1	
	Data_Format_16b_24bpp_mode1();
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_2	
	Data_Format_16b_24bpp_mode2();
#endif
}




void BTE_MCU_Write_ColorExpansion_Chroma_key_MCU_8bit
(
unsigned long Des_Addr //start address of Destination
,unsigned short Des_W //image width of Destination (recommend = canvas image width) 
,unsigned short XDes //coordinate X of Destination
,unsigned short YDes //coordinate Y of Destination
,unsigned short X_W //Width of BTE Window
,unsigned short Y_H //Length of BTE Window
,unsigned long Foreground_color 
/*Foreground_color : The source (1bit map picture) map data 1 translate to Foreground color by color expansion*/
,const unsigned char *data //8-bit data
)
{
  unsigned short i,j;

//  Data_Format_8b_8bpp();
	
	/* set BTE Parameters and Run */
#ifdef MCU_8bit_ColorDepth_8bpp
  Foreground_color_256(Foreground_color);
#endif
#ifdef MCU_8bit_ColorDepth_16bpp
  Foreground_color_65k(Foreground_color);
#endif
#ifdef MCU_8bit_ColorDepth_24bpp
  Foreground_color_16M(Foreground_color);
#endif 

	BTE_Destination_Memory_Start_Address(Des_Addr);
	BTE_Destination_Image_Width(Des_W);
	BTE_Destination_Window_Start_XY(XDes,YDes);

	BTE_ROP_Code(7);	//MCU data bus width: 8bit, ROP Code=7
	BTE_Window_Size(X_W,Y_H);
	BTE_Operation_Code(0x9);		//BTE Operation: MPU Write with Color Expansion and chroma keying (w/o ROP)

	BTE_Enable();		
		
  RA8876_CmdWrite(0x04);				 		//Memory Data Read/Write Port
    
	for(i=0;i< Y_H;i++)
  {	
	  for(j=0;j< X_W/8;j++)
 	  {
	    Check_Mem_WR_FIFO_not_Full();
		  RA8876_DataWrite(*data);  
	    data++;
	  }
  }
	Check_Mem_WR_FIFO_Empty();
	  
	Check_BTE_Busy();


}



void BTE_MCU_Write_ColorExpansion_Chroma_key_MCU_16bit
(
unsigned long Des_Addr //start address of Destination
,unsigned short Des_W //image width of Destination (recommend = canvas image width)
,unsigned short XDes //coordinate X of Destination
,unsigned short YDes //coordinate Y of Destination
,unsigned short X_W //Width of BTE Window
,unsigned short Y_H //Length of BTE Window
,unsigned long Foreground_color
/*Foreground_color : The source (1bit map picture) map data 1 translate to Foreground color by color expansion*/
,const unsigned short *data //16-bit data
)
{
  unsigned short i,j;

#ifdef MCU_16bit_ColorDepth_16bpp
	Data_Format_16b_16bpp();
    Foreground_color_65k(Foreground_color);
	BTE_ROP_Code(15);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_1
	Data_Format_16b_8bpp();
	Foreground_color_16M(Foreground_color);
	BTE_ROP_Code(7);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_2
	Data_Format_16b_8bpp();
	Foreground_color_16M(Foreground_color);
	BTE_ROP_Code(7);
#endif

	BTE_Destination_Memory_Start_Address(Des_Addr);
	BTE_Destination_Image_Width(Des_W);
	BTE_Destination_Window_Start_XY(XDes,YDes);


	BTE_Window_Size(X_W,Y_H);
	BTE_Operation_Code(0x9);		//BTE Operation: MPU Write with Color Expansion and chroma keying (w/o ROP)
	BTE_Enable();	
 	RA8876_CmdWrite(0x04);				 		//Memory Data Read/Write Port

#ifdef MCU_16bit_ColorDepth_16bpp						//setting in UserDef.h
	for(i=0;i< Y_H;i++)
  {	
	  for(j=0;j< X_W/16;j++)
 	  {
	    Check_Mem_WR_FIFO_not_Full();
		  RA8876_DataWrite(*data);  
	    data++;
	  }
  }
	Check_Mem_WR_FIFO_Empty();
#endif	

#ifdef MCU_16bit_ColorDepth_24bpp_Mode_1				//setting in UserDef.h
	for(i=0;i< Y_H;i++)
  {	
	  for(j=0;j< X_W/16;j++)
 	  {
	    Check_Mem_WR_FIFO_not_Full();
			RA8876_DataWrite(*data>>8);  

	    Check_Mem_WR_FIFO_not_Full();
			RA8876_DataWrite(*data);  
	    data++;
	  }
  }
	Check_Mem_WR_FIFO_Empty();
#endif	

#ifdef MCU_16bit_ColorDepth_24bpp_Mode_2				//setting in UserDef.h
	for(i=0;i< Y_H;i++)
  {	
	  for(j=0;j< X_W/16;j++)
 	  {
	    Check_Mem_WR_FIFO_not_Full();
			RA8876_DataWrite(*data>>8);  

	    Check_Mem_WR_FIFO_not_Full();
			RA8876_DataWrite(*data);  
	    data++;
	  }
  }
	Check_Mem_WR_FIFO_Empty();
#endif
	Check_BTE_Busy();			


}



void BTE_Solid_Fill
(
unsigned long Des_Addr //start address of destination 
,unsigned short Des_W // image width of destination (recommend = canvas image width) 
, unsigned short XDes //coordinate X of destination 
,unsigned short YDes //coordinate Y of destination 
,unsigned long Foreground_color //Solid Fill color
,unsigned short X_W //Width of BTE Window
,unsigned short Y_H //Length of BTE Window 
)
{

#ifdef MCU_8bit_ColorDepth_8bpp	
  Foreground_color_256(Foreground_color);
#endif
#ifdef MCU_8bit_ColorDepth_16bpp	
  Foreground_color_65k(Foreground_color);
#endif
#ifdef MCU_8bit_ColorDepth_24bpp	
  Foreground_color_16M(Foreground_color);
#endif

#ifdef MCU_16bit_ColorDepth_16bpp	
  Foreground_color_65k(Foreground_color);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_1	
  Foreground_color_16M(Foreground_color);
#endif
#ifdef MCU_16bit_ColorDepth_24bpp_Mode_2	
  Foreground_color_16M(Foreground_color);
#endif
  BTE_Destination_Memory_Start_Address(Des_Addr);
  BTE_Destination_Image_Width(Des_W);
  BTE_Destination_Window_Start_XY(XDes,YDes);
  BTE_Window_Size(X_W,Y_H);
  BTE_Operation_Code(0x0c);		//BTE Operation: Solid Fill (w/o ROP)
  BTE_Enable();
  Check_BTE_Busy();  
}


/*
S0_Addr : Source 0 Start address
S0_W : Source 0 image width(recommend = canvas image width) 
XS0 : Source 0 coordinate of X
YS0 : Source 0 coordinate of Y
S1_Addr : Source 1 Start address
S1_W : Source 1 image width(recommend = canvas image width) 
XS1 : Source 1 coordinate of X
YS1 : Source 1 coordinate of Y
Des_Addr : Destination start address
Des_W :	 Destination image width(recommend = canvas image width) 
XDes : Destination coordinate of X
YDes : Destination coordinate of Y
X_W : BTE Window Size of X
Y_H : BTE Window Size of Y
alpha : Alpha Blending effect 0 ~ 32, Destination data = (Source 0 * (1- alpha)) + (Source 1 * alpha)
*/
void BTE_Alpha_Blending(unsigned long S0_Addr,unsigned short S0_W,unsigned short XS0,unsigned short YS0,unsigned long S1_Addr,unsigned short S1_W,unsigned short XS1,unsigned short YS1,unsigned long Des_Addr,unsigned short Des_W, unsigned short XDes,unsigned short YDes,unsigned short X_W,unsigned short Y_H,unsigned char alpha)
{
  BTE_S0_Memory_Start_Address(S0_Addr);
  BTE_S0_Image_Width(S0_W);
  BTE_S0_Window_Start_XY(XS0,YS0);

  BTE_S1_Memory_Start_Address(S1_Addr);
  BTE_S1_Image_Width(S1_W); 
  BTE_S1_Window_Start_XY(XS1,YS1);

  BTE_Destination_Memory_Start_Address(Des_Addr);
  BTE_Destination_Image_Width(Des_W);
  BTE_Destination_Window_Start_XY(XDes,YDes);

  BTE_Window_Size(X_W,Y_H);
  BTE_Operation_Code(0x0A);		//BTE Operation: Memory write with opacity (w/o ROP)
  BTE_Alpha_Blending_Effect(alpha);
  BTE_Enable();
  Check_BTE_Busy();
}


void DMA_24bit
(
unsigned char SCS //SCS : 0 = Use SCS0, 1 = Use SCS1
,unsigned char Clk //Clk : SPI Clock = System Clock /{(Clk+1)*2}
,unsigned short X1 //X of DMA Coordinate
,unsigned short Y1 //Y of DMA Coordinate 
,unsigned short X_W //DMA Block width
,unsigned short Y_H //DMA Block height
,unsigned short P_W //DMA Picture width
,unsigned long Addr //DMA Source Start address
)
{    Enable_SFlash_SPI();									   //Enable Serial Flash
	if(SCS == 0)
	{
		Select_SFI_0();										   //Select Serial Flash 0
	}
	if(SCS == 1)
	{														    
		Select_SFI_1();										/* ѡ��Serial Flash�ӿ�0 */
	}					   
	Select_SFI_DMA_Mode();									/* ����ΪSerial Flash DMAģʽ */
//	Select_SFI_24bit_Address();								/* ����24λѰַģʽ */		
//	Select_standard_SPI_Mode0_or_Mode3();					/* ��׼ SPI ģʽ 0 ��ģʽ 3 ʱ��ͼ */
//	Select_SFI_Single_Mode_Dummy_8T_0Bh();					/* 1x ��ȡ����0Bh��Ϊfaster read�ٶȡ���������xmiso���룬RA8876�ڵ�ַ�����ݼ������ 8 �������ڡ� */

	SPI_Clock_Period(Clk);									/* Serial Flash spiʱ�ӣ�8M spi flash ���ʱ��Ƶ��Ϊ80Mhz */
	/////////////////////////////////////////****************DMA 
	Goto_Pixel_XY(X1,Y1);									//set Memory coordinate in Graphic Mode
	SFI_DMA_Destination_Upper_Left_Corner(X1,Y1);			//DMA Destination position(x,y)
	SFI_DMA_Transfer_Width_Height(X_W,Y_H);					//Set DMA Block (Height , Width)
	SFI_DMA_Source_Width(P_W);								//Set DMA Source Picture Width
	SFI_DMA_Source_Start_Address(Addr); 						  //Set Serial Flash DMA Source Starting Address
//	Canvas_image_width(800);			/* ��ͼ��� */
//	Memory_16bpp_Mode();				
//	
//	Reset_CPOL();				/* ��CPOLʱ�Ӽ���λ��0 */
////	Set_CPOL();
//	Reset_CPHA();				/* ��CPHAʱ����λ��0 */
////	Set_CPHA();
//	
//	//BTE_Operation_Code(0x0e);
//	Graphic_Mode();						/* ͼ��ģʽ */
//	Memory_XY_Mode();										/* ѡ���ģʽ */
////	Memory_Linear_Mode();
	Start_SFI_DMA();									  //Start DMA
	Check_Busy_SFI_DMA();								  //DMA Busy Check
}


void DMA_32bit
(
unsigned char SCS //SCS : 0 = Use SCS0, 1 = Use SCS1
,unsigned char Clk //Clk : SPI Clock = System Clock /{(Clk+1)*2}
,unsigned short X1 //X of DMA Coordinate
,unsigned short Y1 //Y of DMA Coordinate 
,unsigned short X_W //DMA Block width
,unsigned short Y_H //DMA Block height
,unsigned short P_W //DMA Picture width
,unsigned long Addr //DMA Source Start address
)
{  

//  Enable_SFlash_SPI();									   //Enable Serial Flash
  if(SCS == 0)
  {
   Select_SFI_0();										   //Select Serial Flash 0
  }
  if(SCS == 1)
  {														    
   Select_SFI_1();										   //Select Serial Flash 1
  }	 
  Select_SFI_DMA_Mode();								   //Set Serial Flash DMA Mode
  SPI_Clock_Period(Clk);

  Select_SFI_32bit_Address();							   //Set Serial Flash/ROM 32bits Address
  /////////////////////////////////////////****************DMA 
  Goto_Pixel_XY(X1,Y1);									  //set Memory coordinate in Graphic Mode
  SFI_DMA_Destination_Upper_Left_Corner(X1,Y1);			  //DMA Destination position(x,y)
  SFI_DMA_Transfer_Width_Height(X_W,Y_H);				  //Set DMA Block (Height , Width)
  SFI_DMA_Source_Width(P_W);							  //Set DMA Source Picture Width
  SFI_DMA_Source_Start_Address(Addr); 						  //Set Serial Flash DMA Source Starting Address

  Start_SFI_DMA();									  //Start DMA
  Check_Busy_SFI_DMA();								  //DMA Busy Check
  Select_SFI_24bit_Address();
}


void switch_24bits_to_32bits(unsigned char SCS)
{

  if(SCS == 0)
  {
   Select_nSS_drive_on_xnsfcs0();
  }
  if(SCS == 1)
  {														    
   Select_nSS_drive_on_xnsfcs1();
  }	 
//**************************//data are read on the clock's rising edge(low��high transition)
//**************************//and data are changed on a falling edge (high��low clock transition) 
  Reset_CPOL();						   
  //Set_CPOL();
  Reset_CPHA();
  //Set_CPHA();
//**************************
  //Enter 4-byte mode
  								   
  nSS_Active();					   //nSS port will goes low
  SPI_Master_FIFO_Data_Put(0xB7);  //������32��}�Ҧ�

  bsp_DelayMS(1); 
  nSS_Inactive();				   //nSS port will goes high
}



/*
*********************************************************************************************************
*	�� �� ��: PWM0
*	����˵��: PWM���ò����
*	��    �Σ�  on_off��PWMʹ��/ʧ��  1/0
*		 Clock_Divided��PWMʱ�ӷ�Ƶ 0-3(1,1/2,1/4,1/8)
*			 Prescalar��Ԥ��Ƶ 1-256
*		  Count_Buffer��PWM�������ʱ��
*		Compare_Buffer��PWM����ߵ�ƽʱ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
/*Such as the following formula :
PWM CLK = (Core CLK / Prescalar ) /2^ divided clock 
PWM output period = (Count Buffer + 1) x PWM CLK time
PWM output high level time = (Compare Buffer + 1) x PWM CLK time */
void PWM0(uint8_t on_off, uint8_t Clock_Divided, uint8_t Prescalar, uint16_t Count_Buffer, uint16_t Compare_Buffer)  
{
	Select_PWM0();								/* ʹ�ܲ�����PWM��ʱ��0 */
	Set_PWM_Prescaler_1_to_256(Prescalar);		/* ����PWMԤ��Ƶֵ */

	if(Clock_Divided ==0)							/* PWM0ʱ�ӷ��� 1/1 */
	{
		Select_PWM0_Clock_Divided_By_1();
	}
	if(Clock_Divided ==1)
	{
		Select_PWM0_Clock_Divided_By_2();			/* PWM0ʱ�ӷ��� 1/2 */
	}
	if(Clock_Divided ==2)
	{
		Select_PWM0_Clock_Divided_By_4();			/* PWM0ʱ�ӷ��� 1/4 */
	}
	if(Clock_Divided ==3)
	{
		Select_PWM0_Clock_Divided_By_8();			/* PWM0ʱ�ӷ��� 1/8 */
	}

	Set_Timer0_Count_Buffer(Count_Buffer);  		/* ���ö�ʱ��0������ֵ */
    Set_Timer0_Compare_Buffer(Compare_Buffer);		/* ���ö�ʱ��0�Ƚϻ�����ֵ */
	
	if (on_off == 1)
	{
		Start_PWM0();								/* ����PWM0 */
	}	 
	if (on_off == 0)
	{
		Stop_PWM0();								/* �ر�PWM0 */
	}
}

/*
*********************************************************************************************************
*	�� �� ��: PWM1
*	����˵��: PWM���ò����
*	��    �Σ�  on_off��PWMʹ��/ʧ��  1/0
*		 Clock_Divided��PWMʱ�ӷ�Ƶ 0-3(1,1/2,1/4,1/8)
*			 Prescalar��Ԥ��Ƶ 1-256
*		  Count_Buffer��PWM�������ʱ��
*		Compare_Buffer��PWM����ߵ�ƽʱ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
/*Such as the following formula :
PWM CLK = (Core CLK / Prescalar ) /2^ divided clock 
PWM output period = (Count Buffer + 1) x PWM CLK time
PWM output high level time = (Compare Buffer + 1) x PWM CLK time */

	//PWM1(1,3,100,4,3); 		/* ��PWM���� */
void PWM1(uint8_t on_off, uint8_t Clock_Divided, uint8_t Prescalar, uint16_t Count_Buffer, uint16_t Compare_Buffer)  
{
#if 1
	Select_PWM1();								/* ʹ�ܲ�����PWM��ʱ��1 */
	Set_PWM_Prescaler_1_to_256(Prescalar);		/* ����PWMԤ��Ƶֵ */

//	Auto_Reload_PWM1();		/* �Զ����� */
//	Enable_PWM1_Interrupt();	/* ��PWM1�ж� */
//	Enable_PWM1_Inverter();		/* ��������� */
	
	if(Clock_Divided ==0)
	{
		Select_PWM1_Clock_Divided_By_1();		/* PWM1ʱ�ӷ�Ƶ�� 1/1 */
	}
	if(Clock_Divided ==1)
	{
		Select_PWM1_Clock_Divided_By_2();		/* PWM1ʱ�ӷ�Ƶ�� 1/2 */
	}
	if(Clock_Divided ==2)
	{
		Select_PWM1_Clock_Divided_By_4();		/* PWM1ʱ�ӷ�Ƶ�� 1/4 */
	}
	if(Clock_Divided ==3)
	{
		Select_PWM1_Clock_Divided_By_8();		/* PWM1ʱ�ӷ�Ƶ�� 1/8 */
	}

	Set_Timer1_Count_Buffer(Count_Buffer);		/* ���ö�ʱ��0������ֵ���������ʱ�� */
	Set_Timer1_Compare_Buffer(Compare_Buffer); 	/* ���ö�ʱ��0�Ƚϻ�����ֵ,����ߵ�ƽʱ�� */


#endif
	if (on_off == 1)
	{
		Start_PWM1();							/* ����PWM1 */
	}	 
	if (on_off == 0)
	{
		Stop_PWM1();							/* �ر�PWM1 */
	}

}



// Note. this API does not support the case that MCU=16bit, 24bpp and mode1
void putPixel
(
unsigned short x //x of coordinate
,unsigned short y //y of coordinate
,unsigned long color 
/*color : 8bpp:R3G3B2
16bpp:R5G6B5
24bpp:R8G8B8 */
)

{

	Goto_Pixel_XY(x,y);
	RA8876_CmdWrite(0x04);	
  	Check_Mem_WR_FIFO_not_Full();

	#ifdef MCU_8bit_ColorDepth_8bpp	
	RA8876_DataWrite(color);
	#endif
	#ifdef MCU_8bit_ColorDepth_16bpp	
	RA8876_DataWrite(color);
	Check_Mem_WR_FIFO_not_Full();
	RA8876_DataWrite(color>>8);
	#endif
	#ifdef MCU_8bit_ColorDepth_24bpp	
	RA8876_DataWrite(color);
	Check_Mem_WR_FIFO_not_Full();
	RA8876_DataWrite(color>>8);
	Check_Mem_WR_FIFO_not_Full();
	RA8876_DataWrite(color>>16);
	#endif
	#ifdef MCU_16bit_ColorDepth_16bpp	
	RA8876_DataWrite(color);
	#endif
	#ifdef MCU_16bit_ColorDepth_24bpp_Mode_2	
	RA8876_DataWrite(color);
	Check_Mem_WR_FIFO_not_Full();
	RA8876_DataWrite(color>>16);
	#endif
}

#if 0
// Note. this API does not support the case that MCU=16bit, 24bpp and mode1
void lcdPutChar8x12
(
unsigned short x // x of coordinate
,unsigned short y // y of coordinate
,unsigned long fgcolor //fgcolor : foreground color(font color)
,unsigned long bgcolor //bgcolor : background color
/*8bpp:R3G3B2
16bpp:R5G6B5
24bpp:R8G8B8*/
, unsigned char bg_transparent 
/*bg_transparent = 0, background color with no transparent
bg_transparent = 1, background color with transparent*/
,unsigned char code //code : font char
)
{ 
  unsigned short i=0;
  unsigned short j=0;
  unsigned char tmp_char=0,LB;
  unsigned long temp1,temp2;

  for (i=0;i<12;i++)
  {
   tmp_char = ascii_table_8x12[((code-0x20)*12)+i];//minus 32 offset, because this table from ascii table "space" 

   for (j=0;j<8;j++)
   {

    if ( (tmp_char >>7-j) & 0x01 == 0x01)

        putPixel(x+j,y+i,fgcolor); //

    else
	{   

        if(!bg_transparent)

        putPixel(x+j,y+i,bgcolor); //

    } 


   }

  }

}


// Note. this API does not support the case that MCU=16bit, 24bpp and mode1
void lcdPutString8x12
(
unsigned short x //x of coordinate 
,unsigned short y //y of coordinate 
, unsigned long fgcolor //fgcolor : foreground color(font color)
,unsigned long bgcolor //bgcolor : background color
/*8bpp:R3G3B2
16bpp:R5G6B5
24bpp:R8G8B8*/
, unsigned char bg_transparent 
/*bg_transparent = 0, background color with no transparent
bg_transparent = 1, background color with transparent*/
,char *ptr //*ptr: font string
)
{
  unsigned short i = 0;
  //screen width = 800,  800/8 = 100 
  //if string more then 100 fonts, no show
  while ((*ptr != 0) & (i < 100))
  {
    lcdPutChar8x12(x, y, fgcolor, bgcolor,bg_transparent, *ptr);
    x += 8;
    ptr++;
    i++;  
  }

}

 
// Note. this API does not support the case that MCU=16bit, 24bpp and mode1
void lcdPutChar16x24
(
unsigned short x //x of coordinate
,unsigned short y //y of coordinate
,unsigned long fgcolor //fgcolor : foreground color(font color)
,unsigned long bgcolor //bgcolor : background color
/*8bpp:R3G3B2
16bpp:R5G6B5
24bpp:R8G8B8*/
, unsigned char bg_transparent
/*bg_transparent = 0, background color with no transparent
bg_transparent = 1, background color with transparent*/
,unsigned char code //code : font char
)
{ 
  unsigned short i=0;
  unsigned short j=0;
  unsigned long array_addr =0;
  unsigned int tmp_char=0;

  for (i=0;i<24;i++)
  {
    //minus 32 offset, because this table from ascii table "space"  
    array_addr = ((code-0x20)*2*24)+(i*2); 
    tmp_char = ascii_table_16x24[array_addr]<<8|ascii_table_16x24[array_addr+1];

   for (j=0;j<16;j++)
   {

    if ( (tmp_char >>15-j) & 0x01 == 0x01)

        putPixel(x+j,y+i,fgcolor); //

    else
	   {
	    if(!bg_transparent)

        putPixel(x+j,y+i,bgcolor); // 

       }
   }
  }
}

 
// Note. this API does not support the case that MCU=16bit, 24bpp and mode1
void lcdPutString16x24
(
unsigned short x //x of coordinate
,unsigned short y //y of coordinate
, unsigned long fgcolor //fgcolor : foreground color(font color)
, unsigned long bgcolor //bgcolor : background color
/*8bpp:R3G3B2
16bpp:R5G6B5
24bpp:R8G8B8*/
, unsigned char bg_transparent 
/*bg_transparent = 0, background color with no transparent
bg_transparent = 1, background color with transparent*/
,char *ptr //*ptr : font string
) 
{
  unsigned short i = 0;
  //screen width = 800,  800/16 = 50 
  //if string more then 50 fonts, no show
  while ((*ptr != 0) & (i < 50))
  {
    lcdPutChar16x24(x, y, fgcolor, bgcolor,bg_transparent, *ptr);
    x += 16;
    ptr++;
    i++;  
  }

}

 
// Note. this API does not support the case that MCU=16bit, 24bpp and mode1
void lcdPutChar32x48
(
unsigned short x //x of coordinate
,unsigned short y //y of coordinate
,unsigned long fgcolor //fgcolor : foreground color(font color)
,unsigned long bgcolor //bgcolor : background color
/*8bpp:R3G3B2
16bpp:R5G6B5
24bpp:R8G8B8*/
, unsigned char bg_transparent 
/*bg_transparent = 0, background color with no transparent 
bg_transparent = 1, background color with transparent*/
,unsigned char code //code : font char
)
{ 
  unsigned short i=0;
  unsigned short j=0;
  unsigned long array_addr =0;
  unsigned long tmp_char=0;

  for (i=0;i<48;i++)
  {

    //minus 32 offset, because this table from ascii table "space"  
	array_addr = ((code-0x20)*4*48)+(i*4); 
	tmp_char = ascii_table_32x48[array_addr]<<24|ascii_table_32x48[array_addr+1]<<16|ascii_table_32x48[array_addr+2]<<8|ascii_table_32x48[array_addr+3];
	     
     for (j=0;j<32;j++)
	 {

     if ( (tmp_char >> (31-j)) & 0x01 == 0x01)

       putPixel(x+j,y+i,fgcolor); //

     else

         {
		 if(!bg_transparent)
		    putPixel(x+j,y+i,bgcolor); // 
		 }
	 } 

  }

}

 
// Note. this API does not support the case that MCU=16bit, 24bpp and mode1
void lcdPutString32x48
(
unsigned short x //x of coordinate
,unsigned short y //y of coordinate
, unsigned long fgcolor //fgcolor : foreground color(font color)
, unsigned long bgcolor //bgcolor : background color
/*8bpp:R3G3B2
16bpp:R5G6B5
24bpp:R8G8B8*/
, unsigned char bg_transparent, 
/*bg_transparent = 0, background color with no transparent
bg_transparent = 1, background color with transparent*/
char *ptr //*ptr: font string
)
{
  unsigned short i = 0;
  //screen width = 800,  800/32 = 25 
  //if string more then 25 fonts, no show
  while ((*ptr != 0) & (i < 25))
  {
    lcdPutChar32x48(x, y, fgcolor, bgcolor,bg_transparent, *ptr);
    x += 32;
    ptr++;
    i++;  
  }

}
#endif

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
