
#ifndef _LCD_RA8876_H_
#define _LCD_RA8876_H_

/* PLL 时钟定义 */
#define OSC_FREQ	12				/* OSC晶振时钟频率,单位: MHz */
#define DRAM_FREQ	133				/* SDRAM时钟频率,单位: MHz */
#define CORE_FREQ	120				/* 系统内核时钟频率,单位: MHz */
#define SCAN_FREQ	51				/* 面板扫描时钟频率,单位: MHz */

/* SDRAM 型号定义 */
#define HY57V641620F
//	#define IS42SM16160D
//	#define IS42S16320B
//	#define IS42S16400F
//	#define M12L32162A
//	#define M12L2561616A
//	#define M12L64164A
//	#define W9825G6JH
//	#define W9812G6JH
//	#define MT48LC4M16A
//	#define K4S641632N
//	#define K4S281632K
//	#define IS42S16100

/* 面板设置 */
#define INCH101			/* 10.1寸屏 */
//	#define OTA7001			/* 7寸屏 *///##OTD9960 & OTA7001 800x480
//	#define EJ080NA_04B		//##INNOLUX_1024x768_EJ080NA-04B
//	#define LQ150X1LGN2C	//##SHARP_LQ150X1LGN2C_LVDS1 & LVDS2
//	#define N070ICG_LD1		//##INNOLUX_1280x800_N070ICG-LD1
//	#define B116XW03_V0		//##AUO1366x768_B116XW03 V0
//	#define LQ121S1LG81		//##SHARP_LQ121S1LG81
//	#define LQ035NC111
//	#define AT080TN52		//##INNOLUX_800x600
//	#define AT070TN92		//##INNOLUX_800x480
//	#define ET101000DM6		//##EDT_1024x600_ET101000DM6
//	#define G190SVT01		//##AUO_1680x342_G190SVT01
//	#define ZJ070NA_01B		//##INNOLUX_1024x600_ZJ070NA_01B
//	#define	LQ190E1LW52		//##SHARP_1280x1024_LQ190E1LW52
//	#define HJ070IA_02F		//##INNOLUX_1280x800_HJ070IA_02F
//	#define LQ156M1LG21		//##SHARP_1920x1080_LQ156M1LG21
//	#define LQ201U1LW32		//##SHARP_1600x1200_LQ201U1LW32

/*==== [HW_(5)] Serial Flash Memory */ 
//	#define MX25L25635E
	#define W25Q256FV

/* 字库ROM GT_Font ROM*/ 
//	#define GT21L16T1W
//	#define GT30L16U2W
//	#define GT30L24T3Y
//	#define GT30L24M1Z
	#define GT30L32S4W
//	#define GT20L24F6Y
//	#define GT21L24S1W


/* 选择MCU和颜色深度 */
//	#define MCU_8bit_ColorDepth_8bpp			  
//	#define MCU_8bit_ColorDepth_16bpp		
//	#define MCU_8bit_ColorDepth_24bpp
#define MCU_16bit_ColorDepth_16bpp			/* 16位颜色深度 */
//	#define MCU_16bit_ColorDepth_24bpp_Mode_1
//	#define MCU_16bit_ColorDepth_24bpp_Mode_2


/* 按位置1 */
#define	cSetb0		0x01
#define	cSetb1		0x02
#define	cSetb2		0x04
#define	cSetb3		0x08
#define	cSetb4		0x10
#define	cSetb5		0x20
#define	cSetb6		0x40
#define	cSetb7		0x80

/* 按位清0 */
#define	cClrb0		0xfe
#define	cClrb1		0xfd
#define	cClrb2		0xfb
#define	cClrb3		0xf7
#define	cClrb4		0xef
#define	cClrb5		0xdf
#define	cClrb6		0xbf
#define	cClrb7		0x7f

/* 底层接口函数 */
void RA8876_PLL(void); 
void RA8876_SDRAM_initial(void);
void RA8876_HW_Reset(void);
void Enable_PLL(void);
void Check_SDRAM_Ready(void);
void TFT_24bit(void);
void TFT_18bit(void);
void Host_Bus_8bit(void);
void Data_Format_16b_24bpp_mode1(void);
void Data_Format_16b_24bpp_mode2(void);
void Select_Main_Window_24bpp(void);
void Set_LCD_Panel(void);
void Memory_Linear_Mode(void);	
void Memory_XY_Mode(void);
void Set_Serial_Flash_IF(void);
void Color_Bar_ON(void);
void Color_Bar_OFF(void);
void Select_PIP1_Window_24bpp(void);
void Memory_16bpp_Mode(void);
void RA8876_SW_Reset(void);
void Enable_PIP1(void);
void Memory_Select_Gamma_Table(void);
void Write_GPIO_D_7_0(uint8_t temp);
void Set_GPIO_D_In_Out(uint8_t temp);
void Data_Format_8b_8bpp(void);
void Select_Main_Window_8bpp(void);
void Select_PIP1_Window_8bpp(void);
void Select_PIP2_Window_8bpp(void);
void Memory_Linear_Mode(void);
void BTE_S0_Color_8bpp(void);
void BTE_S1_Color_8bpp(void);
void BTE_Destination_Color_8bpp(void);
void Memory_8bpp_Mode(void);
void Select_PIP2_Window_24bpp(void);
void BTE_S0_Color_24bpp(void);
void BTE_S1_Color_24bpp(void);
void Memory_24bpp_Mode(void);
void BTE_Destination_Color_24bpp(void);
void Data_Format_8b_16bpp(void);
void BTE_S1_Color_16bpp(void);
void Check_IC_ready(void);
void BTE_Destination_Color_16bpp(void);
void System_Check_Temp(void);
void Check_Mem_WR_FIFO_not_Full(void);
void Check_2D_Busy(void);
void TFT_16bit(void);
void Host_Bus_16bit(void);
void Data_Format_16b_16bpp(void);
void MemWrite_Left_Right_Top_Down(void);
void MemWrite_Right_Left_Top_Down(void);
void MemWrite_Down_Top_Left_Right(void);
void MemWrite_Top_Down_Left_Right(void);
void Graphic_Mode(void);
void Text_Mode(void);
void Memory_Select_SDRAM(void);
void Select_Main_Window_16bpp(void);
void Select_PIP1_Window_16bpp(void);
void Select_PIP2_Window_16bpp(void);
void Display_ON(void);
void Display_OFF(void);
void VSCAN_T_to_B(void);
void VSCAN_B_to_T(void);
void Main_Image_Start_Address(unsigned long Addr);
void Main_Image_Width(uint16_t WX);
void Main_Window_Start_XY(uint16_t WX,uint16_t HY);
void Canvas_Image_Start_address(unsigned long Addr);
void Canvas_image_width(uint16_t WX);
void Active_Window_WH(uint16_t WX,uint16_t HY);	
void Active_Window_XY(uint16_t WX,uint16_t HY);
void Goto_Pixel_XY(uint16_t WX,uint16_t HY);
void BTE_S0_Color_16bpp(void);
uint8_t SPI_Master_FIFO_Data_Put(uint8_t Data);
uint8_t SPI_Master_FIFO_Data_Get(void);
uint8_t Tx_FIFO_Empty_Flag(void);
uint8_t Tx_FIFO_Full_Flag(void);
uint8_t Rx_FIFO_Empty_Flag(void);
void CGRAM_Start_address(unsigned long Addr);
void PDATA_Set_RGB(void);
void Enable_SFlash_SPI(void);
void Disable_SFlash_SPI(void);
void Font_Background_select_Original_Canvas(void);
void Font_Background_select_Color(void);
void Font_0_degree(void);
void Font_90_degree(void);
void Select_SFI_Font_Mode(void);
void CGROM_Select_Genitop_FontROM(void);
void Set_GTFont_Decoder(uint8_t temp);
void Goto_Text_XY(uint16_t WX,uint16_t HY);
void Select_SFI_0(void);
void Select_SFI_1(void);
void Select_SFI_Font_Mode(void);
void SPI_Clock_Period(uint8_t temp);
void Foreground_color_65k(uint16_t temp);
void Background_color_65k(uint16_t temp);
void Font_Select_8x16_16x16(void);
void Font_Select_12x24_24x24(void);
void Font_Select_16x32_32x32(void);
void Font_Line_Distance(uint8_t temp);
void Set_Font_to_Font_Width(uint8_t temp);
void Font_Width_X1(void);
void Font_Width_X2(void);
void Font_Width_X3(void);
void Font_Width_X4(void);
void Font_Height_X1(void);
void Font_Height_X2(void);
void Font_Height_X3(void);
void Font_Height_X4(void);


/* API函数 */
void Check_Mem_RD_FIFO_not_Full(void);
void MPU8_8bpp_Memory_Write (unsigned short x,unsigned short y,unsigned short w ,unsigned short h ,const unsigned char *data);
void MPU8_16bpp_Memory_Write (unsigned short x,unsigned short y,unsigned short w ,unsigned short h ,const unsigned char *data);
void MPU8_24bpp_Memory_Write (unsigned short x,unsigned short y,unsigned short w ,unsigned short h ,const unsigned char *data);
void MPU16_16bpp_Memory_Write (unsigned short x,unsigned short y,unsigned short w ,unsigned short h ,const unsigned short *data);
void MPU16_24bpp_Mode1_Memory_Write (unsigned short x,unsigned short y,unsigned short w ,unsigned short h ,const unsigned short *data);
void MPU16_24bpp_Mode2_Memory_Write (unsigned short x,unsigned short y,unsigned short w ,unsigned short h ,const unsigned short *data);
void PIP(unsigned char On_Off, unsigned char Select_PIP,unsigned long PAddr,unsigned short XP,unsigned short YP,unsigned long ImageWidth,unsigned short X_Dis,unsigned short Y_Dis,unsigned short X_W,unsigned short Y_H);
void Print_Internal_Font_Hexvariable(unsigned short x,unsigned short y,unsigned short X_W,unsigned short Y_H,unsigned long FontColor,unsigned long BackGroundColor,  unsigned int tmp2);
void Print_Internal_Font_Decimalvariable(unsigned short x,unsigned short y,unsigned short X_W,unsigned short Y_H,unsigned long FontColor,unsigned long BackGroundColor,unsigned int tmp2);
void Print_Internal_Font_String(unsigned short x,unsigned short y,unsigned short X_W,unsigned short Y_H,unsigned long FontColor,unsigned long BackGroundColor, char tmp2[]);
void Print_BIG5String(unsigned char Clk,unsigned char SCS,unsigned short x,unsigned short y,unsigned short X_W,unsigned short Y_H,unsigned long FontColor,unsigned long BackGroundColor,char *tmp2);
void Print_GB2312String(unsigned char Clk,unsigned char SCS,unsigned short x,unsigned short y,unsigned short X_W,unsigned short Y_H,unsigned long FontColor,unsigned long BackGroundColor,char *tmp2);
void Print_GB12345String(unsigned char Clk,unsigned char SCS,unsigned short x,unsigned short y,unsigned short X_W,unsigned short Y_H,unsigned long FontColor,unsigned long BackGroundColor,char tmp2[]);
void Print_UnicodeString(unsigned char Clk,unsigned char SCS,unsigned short x,unsigned short y,unsigned short X_W,unsigned short Y_H,unsigned long FontColor,unsigned long BackGroundColor,unsigned short *tmp2);
void Select_Font_Height_WxN_HxN_ChromaKey_Alignment(unsigned char Font_Height,unsigned char XxN,unsigned char YxN,unsigned char ChromaKey,unsigned char Alignment);
void Show_String(char *str);
void Draw_Line(unsigned long LineColor,unsigned short X1,unsigned short Y1,unsigned short X2,unsigned short Y2);
void Draw_Triangle(unsigned long ForegroundColor,unsigned short X1,unsigned short Y1,unsigned short X2,unsigned short Y2,unsigned short X3,unsigned short Y3);
void Draw_Triangle_Fill(unsigned long ForegroundColor,unsigned short X1,unsigned short Y1,unsigned short X2,unsigned short Y2,unsigned short X3,unsigned short Y3);
void Draw_Circle(unsigned long ForegroundColor,unsigned short XCenter,unsigned short YCenter,unsigned short R);
void Draw_Circle_Fill(unsigned long ForegroundColor,unsigned short XCenter,unsigned short YCenter,unsigned short R);
void Draw_Ellipse(unsigned long ForegroundColor,unsigned short XCenter,unsigned short YCenter,unsigned short X_R,unsigned short Y_R);
void Draw_Ellipse_Fill(unsigned long ForegroundColor,unsigned short XCenter,unsigned short YCenter,unsigned short X_R,unsigned short Y_R);
void Draw_Left_Up_Curve(unsigned long ForegroundColor,unsigned short XCenter,unsigned short YCenter,unsigned short X_R,unsigned short Y_R);
void Draw_Left_Up_Curve_Fill(unsigned long ForegroundColor,unsigned short XCenter,unsigned short YCenter,unsigned short X_R,unsigned short Y_R);
void Draw_Right_Down_Curve(unsigned long ForegroundColor,unsigned short XCenter,unsigned short YCenter,unsigned short X_R,unsigned short Y_R);
void Draw_Right_Down_Curve_Fill(unsigned long ForegroundColor,unsigned short XCenter,unsigned short YCenter,unsigned short X_R,unsigned short Y_R);
void Draw_Right_Up_Curve(unsigned long ForegroundColor,unsigned short XCenter,unsigned short YCenter,unsigned short X_R,unsigned short Y_R);
void Draw_Right_Up_Curve_Fill(unsigned long ForegroundColor,unsigned short XCenter,unsigned short YCenter,unsigned short X_R,unsigned short Y_R);
void Draw_Left_Down_Curve(unsigned long ForegroundColor,unsigned short XCenter,unsigned short YCenter,unsigned short X_R,unsigned short Y_R);
void Draw_Left_Down_Curve_Fill(unsigned long ForegroundColor,unsigned short XCenter,unsigned short YCenter,unsigned short X_R,unsigned short Y_R);
void Draw_Square(unsigned long ForegroundColor,unsigned short X1,unsigned short Y1,unsigned short X2,unsigned short Y2);
void Draw_Square_Fill(unsigned long ForegroundColor,unsigned short X1,unsigned short Y1,unsigned short X2,unsigned short Y2);
void Draw_Circle_Square(unsigned long ForegroundColor,unsigned short X1,unsigned short Y1,unsigned short X2,unsigned short Y2,unsigned short R,unsigned short Y_R);
void Draw_Circle_Square_Fill(unsigned long ForegroundColor,unsigned short X1,unsigned short Y1,unsigned short X2,unsigned short Y2,unsigned short R,unsigned short Y_R);
void BTE_Memory_Copy(unsigned long S0_Addr,unsigned short S0_W,unsigned short XS0,unsigned short YS0,unsigned long S1_Addr,unsigned short S1_W,unsigned short XS1,unsigned short YS1,unsigned long Des_Addr,unsigned short Des_W, unsigned short XDes,unsigned short YDes,unsigned int ROP_Code,unsigned short X_W,unsigned short Y_H);
void BTE_Memory_Copy_Chroma_key(unsigned long S0_Addr,unsigned short S0_W,unsigned short XS0,unsigned short YS0,unsigned long Des_Addr,unsigned short Des_W, unsigned short XDes,unsigned short YDes,unsigned long Background_color,unsigned short X_W,unsigned short Y_H);
void BTE_MCU_Write_MCU_8bit(unsigned long S1_Addr,unsigned short S1_W,unsigned short XS1,unsigned short YS1,unsigned long Des_Addr,unsigned short Des_W, unsigned short XDes,unsigned short YDes,unsigned int ROP_Code,unsigned short X_W,unsigned short Y_H,const unsigned char *data);
void BTE_MCU_Write_MCU_16bit(unsigned long S1_Addr,unsigned short S1_W,unsigned short XS1,unsigned short YS1,unsigned long Des_Addr,unsigned short Des_W, unsigned short XDes,unsigned short YDes,unsigned int ROP_Code,unsigned short X_W,unsigned short Y_H,const unsigned short *data);
void BTE_MCU_Write_Chroma_key_MCU_8bit(unsigned long Des_Addr,unsigned short Des_W, unsigned short XDes,unsigned short YDes,unsigned long Background_color,unsigned short X_W,unsigned short Y_H,const unsigned char *data);
void BTE_MCU_Write_Chroma_key_MCU_16bit(unsigned long Des_Addr,unsigned short Des_W, unsigned short XDes,unsigned short YDes,unsigned long Background_color,unsigned short X_W,unsigned short Y_H,const unsigned short *data);
void BTE_Memory_Copy_ColorExpansion(unsigned long S0_Addr,unsigned short S0_W,unsigned short XS0,unsigned short YS0,unsigned long Des_Addr,unsigned short Des_W, unsigned short XDes,unsigned short YDes,unsigned short X_W,unsigned short Y_H,unsigned long Foreground_color,unsigned long Background_color);
void BTE_Memory_Copy_ColorExpansion_Chroma_key(unsigned long S0_Addr,unsigned short S0_W,unsigned short XS0,unsigned short YS0,unsigned long Des_Addr,unsigned short Des_W, unsigned short XDes,unsigned short YDes,unsigned short X_W,unsigned short Y_H,unsigned long Foreground_color);
void BTE_MCU_Write_ColorExpansion_MCU_8bit(unsigned long Des_Addr,unsigned short Des_W, unsigned short XDes,unsigned short YDes,unsigned short X_W,unsigned short Y_H,unsigned long Foreground_color,unsigned long Background_color,const unsigned char *data);
void BTE_MCU_Write_ColorExpansion_MCU_16bit(unsigned long Des_Addr,unsigned short Des_W, unsigned short XDes,unsigned short YDes,unsigned short X_W,unsigned short Y_H,unsigned long Foreground_color,unsigned long Background_color,const unsigned short *data);
void BTE_MCU_Write_ColorExpansion_Chroma_key_MCU_8bit(unsigned long Des_Addr,unsigned short Des_W, unsigned short XDes,unsigned short YDes,unsigned short X_W,unsigned short Y_H,unsigned long Foreground_color,const unsigned char *data);
void BTE_MCU_Write_ColorExpansion_Chroma_key_MCU_16bit(unsigned long Des_Addr,unsigned short Des_W, unsigned short XDes,unsigned short YDes,unsigned short X_W,unsigned short Y_H,unsigned long Foreground_color,const unsigned short *data);
void BTE_Pattern_Fill(unsigned char P_8x8_or_16x16, unsigned long S0_Addr,unsigned short S0_W,unsigned short XS0,unsigned short YS0,unsigned long S1_Addr,unsigned short S1_W,unsigned short XS1,unsigned short YS1,unsigned long Des_Addr,unsigned short Des_W, unsigned short XDes,unsigned short YDes,unsigned int ROP_Code,unsigned short X_W,unsigned short Y_H);
void BTE_Pattern_Fill_With_Chroma_key(unsigned char P_8x8_or_16x16 ,unsigned long S0_Addr,unsigned short S0_W,unsigned short XS0,unsigned short YS0,unsigned long S1_Addr,unsigned short S1_W,unsigned short XS1,unsigned short YS1,unsigned long Des_Addr,unsigned short Des_W, unsigned short XDes,unsigned short YDes,unsigned int ROP_Code,unsigned long Background_color,unsigned short X_W,unsigned short Y_H);
void BTE_Solid_Fill(unsigned long Des_Addr,unsigned short Des_W, unsigned short XDes,unsigned short YDes,unsigned long Foreground_color,unsigned short X_W,unsigned short Y_H);
void BTE_Alpha_Blending(unsigned long S0_Addr,unsigned short S0_W,unsigned short XS0,unsigned short YS0,unsigned long S1_Addr,unsigned short S1_W,unsigned short XS1,unsigned short YS1,unsigned long Des_Addr,unsigned short Des_W, unsigned short XDes,unsigned short YDes,unsigned short X_W,unsigned short Y_H,unsigned char alpha);
void DMA_24bit(unsigned char SCS,unsigned char Clk,unsigned short X1,unsigned short Y1,unsigned short X_W,unsigned short Y_H,unsigned short P_W,unsigned long Addr);
void DMA_32bit(unsigned char SCS,unsigned char Clk,unsigned short X1,unsigned short Y1,unsigned short X_W,unsigned short Y_H,unsigned short P_W,unsigned long Addr);
void switch_24bits_to_32bits(unsigned char SCS);
void PWM0(unsigned char on_off, unsigned char Clock_Divided, unsigned char Prescalar, unsigned short Count_Buffer,unsigned short Compare_Buffer);
void PWM1(unsigned char on_off, unsigned char Clock_Divided, unsigned char Prescalar, unsigned short Count_Buffer,unsigned short Compare_Buffer);
void putPixel(unsigned short x,unsigned short y,unsigned long color);
void lcdPutChar8x12(unsigned short x,unsigned short y,unsigned long fgcolor,unsigned long bgcolor, unsigned char bg_transparent,unsigned char code);
void lcdPutString8x12(unsigned short x,unsigned short y, unsigned long fgcolor, unsigned long bgcolor,unsigned char bg_transparent,char *ptr);
void lcdPutChar16x24(unsigned short x,unsigned short y,unsigned long fgcolor,unsigned long bgcolor,unsigned char bg_transparent,unsigned char code);
void lcdPutString16x24(unsigned short x,unsigned short y, unsigned long fgcolor, unsigned long bgcolor,unsigned char bg_transparent,char *ptr);
void lcdPutChar32x48(unsigned short x,unsigned short y,unsigned long fgcolor,unsigned long bgcolor,unsigned char bg_transparent,unsigned char code);
void lcdPutString32x48(unsigned short x,unsigned short y, unsigned long fgcolor, unsigned long bgcolor,unsigned char bg_transparent,char *ptr);


#define RA8876_BASE0        ((uint32_t)0x6C000000)//ra8876
#define RA8876_BASE1        ((uint32_t)(RA8876_BASE0 + (1 << 19)))//((u32)0x60020000)//ra8876

#define RA8876_CmdWrite(cmd)	  *(__IO uint16_t *) (RA8876_BASE0)= (cmd);
#define RA8876_DataWrite(data)   *(__IO uint16_t *) (RA8876_BASE1)= (data);
#define	RA8876_StatusRead()		 *(__IO uint16_t *) (RA8876_BASE0) //if use read  Mcu interface DB0~DB15 needs increase pull high 
#define	RA8876_DataRead()   	 *(__IO uint16_t *) (RA8876_BASE1) //if use read  Mcu interface DB0~DB15 needs increase pull high 

/* RA8876用户层函数*/
void RA8876_InitHard(void);		/* RA8876初始化 */
void RA8876_DispOn(void);
void RA8876_DispOff(void);
void RA8876_ClrScr(uint16_t _usColor);
void RA8876_PutPixel(uint16_t _usX, uint16_t _usY, uint16_t _usColor);
uint16_t RA8876_GetPixel(uint16_t _usX, uint16_t _usY);
void RA8876_DrawLine(uint16_t _usX1 , uint16_t _usY1 , uint16_t _usX2 , uint16_t _usY2 , uint16_t _usColor);
void RA8876_DrawRect(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth, uint16_t _usColor);
void RA8876_DrawCircle(uint16_t _usX, uint16_t _usY, uint16_t _usRadius, uint16_t _usColor);
void RA8876_FillRect(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth, uint16_t _usColor);
void RA8876_FillCircle(uint16_t _usX, uint16_t _usY, uint16_t _usRadius, uint16_t _usColor);
void RA8876_DrawBMP(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth, uint16_t *_ptr);
void RA8876_SetFont(uint8_t _ucFontType, uint8_t _ucLineSpace, uint8_t _ucCharSpace);
void RA8876_DispStr(uint8_t SCS, uint16_t _usX, uint16_t _usY, uint32_t FontColor, uint32_t BackGroundColor, char *_ptr);
void RA8876_DispBmpInFlash(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth, uint32_t _uiFlashAddr);
void RA8876_SetDirection(uint8_t _ucDir);
void RA8876_SetBackLight(uint8_t _bright);
void RA8876_DrawRoundRect(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth, uint16_t _usArc, uint16_t _usColor);
void RA8876_FillRoundRect(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth, uint16_t _usArc, uint16_t _usColor);
void RA8876_SetTextTransp(uint8_t _Enable);
void RA8876_SetTextZoom(uint8_t _ucHSize, uint8_t _ucVSize);
void RA8876_Check_OK(void);
uint8_t Check_Mem_RD_FIFO_Empty(void);	

extern uint8_t g_Interface;			/* 当前界面。1：主界面 2：从界面 */
extern uint8_t g_Drawing;			/* 是否在用特效画整个界面 */
extern uint16_t CanvasWidth;		/* 内存底图宽度 */

#endif

