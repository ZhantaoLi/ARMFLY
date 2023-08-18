/*
*********************************************************************************************************
*
*	模块名称 : FlashFS文件系统演示模块。
*	文件名称 : demo_nand_flashfs.c
*	版    本 : V1.0
*	说    明 : 基于NAND的BootLoader。
*
*	修改记录 :
*		版本号    日期         作者            说明
*       V1.0    2022-06-15   Eric2013    1. RL-FlashFS短文件名版本V6.x
*
*	Copyright (C), 2022-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/
#include "includes.h"



/*
*********************************************************************************************************
*	                                        函数
*********************************************************************************************************
*/
/* 仅允许本文件内调用的函数声明 */
static void ViewNANDCapacity(void);
static void DispMenu(void);
static void DotFormat(uint64_t _ullVal, char *_sp);
static void LoadFirmware(void);
static void JumpToApp(void);
static void BootHexCrcVeriy(void);

/*
*********************************************************************************************************
*	                                       宏定义
*********************************************************************************************************
*/
#define AppAddr  0x08100000    /* APP地址 */

/*
*********************************************************************************************************
*	                                       变量
*********************************************************************************************************
*/
__IO uint32_t uwCRCValue;
__IO uint32_t uwExpectedCRCValue;
__IO uint32_t uwAppSize;

ALIGN_32BYTES(uint8_t tempbuf[4096]);           /* 读取数据的缓冲 */

/* FlashFS API的返回值 */
static const char * ReVal_Table[]= 
{
	"fsOK：成功",				                        
	"fsError：未指定的错误",
	"fsUnsupported：操作不支持",
	"fsAccessDenied：资源访问被拒绝",
	
	"fsInvalidParameter：参数无效",
	"fsInvalidDrive：驱动无效或驱动不存在",
	"fsInvalidPath：路径无效",
	"fsUninitializedDrive：驱动未初始化 ",

	"fsDriverError：读写操作",
	"fsMediaError：媒介错误",
	"fsNoMedia：媒介不存在，或者未初始化",
	"fsNoFileSystem：文件系统未格式化",

	"fsNoFreeSpace：没有可用空间",
	"fsFileNotFound：文件未找到",
	"fsDirNotEmpty：文件夹非空",
	"fsTooManyOpenFiles：打开文件太多",
};

/*
*********************************************************************************************************
*	函 数 名: DemoFlashFS
*	功能说明: FlashFS件系统演示主程序
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void DemoFlashFS(void)
{
	uint8_t cmd;
	uint8_t result;

	/* 打印命令列表，用户可以通过串口操作指令 */
	DispMenu();
	
	result = finit("N0:");
	if(result != NULL)
	{
		/* 如果挂载失败，务必不要再调用FlashFS的其它API函数，防止进入硬件异常 */
		printf("挂载文件系统失败 (%s)\r\n", ReVal_Table[result]);
	}
	else
	{
		printf("挂载文件系统成功 (%s)\r\n", ReVal_Table[result]);
	}
	
	while(1)
	{
		if (comGetChar(COM1, &cmd))	/* 从串口读入一个字符(非阻塞方式) */
		{
			printf("\r\n");
			switch (cmd)
			{
				case '1':
					printf("【1 - 显示NAND容量和剩余容量】\r\n");
					ViewNANDCapacity();		
					break;
				
				case '2':
					printf("【2 - 启动固件加载, 校验和跳转】\r\n");
					LoadFirmware();
					BootHexCrcVeriy();
					JumpToApp();
					break;

				case 'a':
					printf("【a - OpenUSB Device】\r\n");
					printf("注意，NAND挂载期间，不可操作其它命令，要卸载后才可以操作！！\r\n");
					USBD_Initialize(0U);                 
					USBD_Connect   (0U);      
					break;	

				case 'b':
					printf("【b - CloseUSB Device】\r\n");
					USBD_Disconnect(0);
					USBD_Uninitialize(0);
					break;	
				
				default:
					DispMenu();
					break;
			}
		}
		osDelay(10);
	}
}

/*
*********************************************************************************************************
*	函 数 名: DispMenu
*	功能说明: 显示操作提示菜单
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DispMenu(void)
{
	printf("\r\n------------------------------------------------\r\n");
	printf("请选择串口操作命令，电脑键盘打印数字即可:\r\n");
	printf("首次使用，务必调用串口命令1，进行NAND Flash的低级格式化和文件系统格式化\r\n");	
	printf("1 - 显示NAND容量和剩余容量\r\n");
	printf("2 - 启动固件加载, 校验和跳转\r\n");
	printf("a - 打开NAND模拟U盘，注意是小写字母a\r\n");
	printf("b - 关闭NAND模拟U盘，注意是小写字母b\r\n");
}

/*
*********************************************************************************************************
*	函 数 名: JumpToApp
*	功能说明: 跳转到应用JumpToApp
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void JumpToApp(void)
{
	uint32_t i=0;
	void (*AppJump)(void);         /* 声明一个函数指针 */
	
    /* 关闭全局中断 */
	DISABLE_INT(); 
    
    /* 设置所有时钟到默认状态，使用HSI时钟 */
	HAL_RCC_DeInit();
    
	/* 关闭滴答定时器，复位到默认值 */
	SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL = 0;

	/* 关闭所有中断，清除所有中断挂起标志 */
	for (i = 0; i < 8; i++)
	{
		NVIC->ICER[i]=0xFFFFFFFF;
		NVIC->ICPR[i]=0xFFFFFFFF;
	}	

	/* 使能全局中断 */
	ENABLE_INT();

	/* 跳转到应用程序，首地址是MSP，地址+4是复位中断服务程序地址 */
	AppJump = (void (*)(void)) (*((uint32_t *) (AppAddr + 4)));

	/* 设置主堆栈指针 */
	__set_MSP(*(uint32_t *)AppAddr);
	
	/* 在RTOS工程，这条语句很重要，设置为特权级模式，使用MSP指针 */
	__set_CONTROL(0);

	/* 跳转到系统BootLoader */
	AppJump(); 

	/* 跳转成功的话，不会执行到这里，用户可以在这里添加代码 */
	while (1)
	{

	}
}

/*
*********************************************************************************************************
*	函 数 名: BootHexCrcVeriy
*	功能说明: 固件CRC校验
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void BootHexCrcVeriy(void)
{
	CRC_HandleTypeDef   CrcHandle = {0};
	
	/* 读取bin文件的CRC */
	uwExpectedCRCValue  = *(__IO uint32_t *)(AppAddr + uwAppSize - 4);
	
	/* 初始化硬件CRC */
	__HAL_RCC_CRC_CLK_ENABLE();      
	
	CrcHandle.Instance = CRC;
	CrcHandle.Init.DefaultPolynomialUse    = DEFAULT_POLYNOMIAL_ENABLE;
	CrcHandle.Init.DefaultInitValueUse     = DEFAULT_INIT_VALUE_ENABLE;
	CrcHandle.Init.InputDataInversionMode  = CRC_INPUTDATA_INVERSION_NONE;
	CrcHandle.Init.OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_DISABLE;
	CrcHandle.InputDataFormat              = CRC_INPUTDATA_FORMAT_WORDS;

	if (HAL_CRC_Init(&CrcHandle) != HAL_OK)
	{
        Error_Handler(__FILE__, __LINE__);
	}

	/* 计算是否与硬件CRC一致 */
	uwCRCValue = HAL_CRC_Calculate(&CrcHandle, (uint32_t *)(AppAddr), uwAppSize/4 - 1);
	
	printf("实际APP固件CRC校验值 = 0x%x\r\n", uwExpectedCRCValue);
	printf("计算APP固件CRC校验值 = 0x%x\r\n", uwCRCValue);	
	
	if (uwCRCValue != uwExpectedCRCValue)
	{
		printf("校验失败\r\n");
        Error_Handler(__FILE__, __LINE__);		
	}
	else
	{
		printf("校验通过，开始跳转到APP\r\n");
	}
	
	printf("=================================================\r\n");
}

/*
*********************************************************************************************************
*	函 数 名: LoadFirmware
*	功能说明: 固件加载
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void LoadFirmware(void)
{
	uint8_t result;
	float FinishPecent;
	uint32_t bw;
	char path[64];
	uint32_t SectorCount = 0;
	uint32_t SectorRemain = 0;
	fsFileInfo info;
	uint32_t ver;
	uint32_t i = 0;
	uint8_t ucState;
    uint32_t Count = 0;
    uint32_t TotalSize = 0;
	FILE *fin;
	
	info.fileID = 0;                             // info.fileID must be set to 0
	
	/* 第1步：挂载文件系统 ***************************************************************/
	result = fmount("N0:");
	
	if(result != NULL)
	{
		/* 如果挂载失败，务必不要再调用FlashFS的其它API函数，防止进入硬件异常 */
		printf("挂载文件系统失败 (%s)\r\n", ReVal_Table[result]);
		goto access_fail;
	}
	else
	{
		printf("挂载文件系统成功 (%s)\r\n", ReVal_Table[result]);
	}

	/* 第2步：打开文件 ***************************************************************/
	fin = fopen ("N0:\\app.bin", "r"); 
	if (fin != NULL) 
	{
		/* 文件打开成功 */
	}
	else
	{
		printf("打开文件N0:\\app.bin失败, 可能文件不存在\r\n");
		goto access_fail;
	}
	
	/* 第3步：获取文件大小和APP固件版本 ***********************************************/
	if (ffind ("N0:\\app.bin", &info) == fsOK) 
	{
		/* 文件信息获取识别 */		
	}
	else
	{
		printf("获取文件信息失败\r\n");
		goto access_fail;
	}

	/* 打印文件大小, 最大4G */
	printf("APP固件大小：%d\r\n", (int)info.size);
	
	uwAppSize = info.size;
	
	fseek(fin, 28L, SEEK_SET);
	fread(&ver, sizeof(uint32_t), 1, fin);
	fseek(fin, 0L, SEEK_SET);	
	printf("APP固件版本：V%X.%02X\r\n", ver >> 8, ver & 0xFF);
	
	/* 第4步：扇区擦除 **************************************************************/
	SectorCount = info.size/(128*1024);
	SectorRemain = info.size%(128*1024);	
	
	for(i = 0; i < SectorCount; i++)
	{
		bsp_EraseCpuFlash((uint32_t)(AppAddr + i*128*1024));
		printf("擦除扇区 = %08x\r\n", AppAddr + i*128*1024);
	}
	
	if(SectorRemain)
	{
		bsp_EraseCpuFlash((uint32_t)(AppAddr + i*128*1024));
		printf("擦除剩余扇区 = %08x\r\n", AppAddr + i*128*1024);
	}
	
	/* 第5步：将APP固件写入到内部Flash ********************************************/
	for(;;)
	{
		/* 读取一个扇区的数据到buf */
		bw = fread(tempbuf, sizeof(uint8_t), sizeof(tempbuf)/sizeof(uint8_t), fin);
		
		/* 编程内部Flash */
		TotalSize += bw;
		ucState = bsp_WriteCpuFlash((uint32_t)(AppAddr + Count*sizeof(tempbuf)),  (uint8_t *)tempbuf, bw);
		
		/* 如果返回非0，表示编程失败 */
		if(ucState != 0)
		{
			printf("擦除失败\r\n");
			break;
		}
		
		/* 显示复制进度 */
		Count = Count + 1;
		FinishPecent = (float)(TotalSize) / info.size;
		printf("当前完成编程:%02d%%\r\n", (uint8_t)(FinishPecent*100));
		
		/* 读取出错或者读取完毕，退出 */
		if (bw < sizeof(tempbuf)/sizeof(uint8_t))
		{
			printf("APP固件加载完毕\r\n");
			fclose(fin);
			break;
		}
	}
	
access_fail:
	/* 卸载SD卡 */
	result = funmount("N0:");
	if(result != NULL)
	{
		printf("卸载文件系统失败\r\n");
	}
	else
	{
		printf("卸载文件系统成功\r\n");
	}
	
	printf("------------------------------------------------------------------\r\n");
}

/*
*********************************************************************************************************
*	函 数 名: DotFormatjavascript:;
*	功能说明: 将数据规范化显示，方便用户查看
*             比如
*             2345678   ---->  2.345.678
*             334426783 ---->  334.426.783
*             以三个数为单位进行显示
*	形    参: _ullVal   需要规范显示的数值
*             _sp       规范显示后数据存储的buf。
*	返 回 值: 无
*********************************************************************************************************
*/
static void DotFormat(uint64_t _ullVal, char *_sp) 
{
	/* 数值大于等于10^9 */
	if (_ullVal >= (uint64_t)1e9) 
	{
		_sp += sprintf (_sp, "%d.", (uint32_t)(_ullVal / (uint64_t)1e9));
		_ullVal %= (uint64_t)1e9;
		_sp += sprintf (_sp, "%03d.", (uint32_t)(_ullVal / (uint64_t)1e6));
		_ullVal %= (uint64_t)1e6;
		sprintf (_sp, "%03d.%03d", (uint32_t)(_ullVal / 1000), (uint32_t)(_ullVal % 1000));
		return;
	}
	
	/* 数值大于等于10^6 */
	if (_ullVal >= (uint64_t)1e6) 
	{
		_sp += sprintf (_sp,"%d.", (uint32_t)(_ullVal / (uint64_t)1e6));
		_ullVal %= (uint64_t)1e6;
		sprintf (_sp,"%03d.%03d", (uint32_t)(_ullVal / 1000), (uint32_t)(_ullVal % 1000));
		return;
	}
	
	/* 数值大于等于10^3 */
	if (_ullVal >= 1000) 
	{
		sprintf (_sp, "%d.%03d", (uint32_t)(_ullVal / 1000), (uint32_t)(_ullVal % 1000));
		return;
	}
	
	/* 其它数值 */
	sprintf (_sp,"%d",(uint32_t)(_ullVal));
}

/*
*********************************************************************************************************
*	函 数 名: ViewNANDCapacity
*	功能说明: NAND的挂载，卸载及其容量显示
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void ViewNANDCapacity(void)
{
	uint8_t result;
	fsMediaInfo info;
	uint64_t ullSdCapacity;
	int32_t id;  
	uint8_t buf[15];
	uint32_t ser_num;
	fsStatus restatus;
	char *opt;
	
	/* 加载SD卡 */
	result = fmount("N0:");
	if(result != NULL)
	{
		/* 如果挂载失败，务必不要再调用FlashFS的其它API函数，防止进入硬件异常 */
		printf("挂载文件系统失败 (%s)\r\n", ReVal_Table[result]);
	}
	else
	{
		printf("挂载文件系统成功 (%s)\r\n", ReVal_Table[result]);
	}
	
	/* 格式化 */
	opt = "/LL /L nand /FAT16";
	printf("文件系统格式中......\r\n");
	result = fformat ("N0:", opt);
	printf("文件系统格式化 (%s)\r\n", ReVal_Table[result]);
	
	printf("------------------------------------------------------------------\r\n");
	
	/* 获取volume label */
	if (fvol ("N0:", (char *)buf, &ser_num) == 0) 
	{
		if (buf[0]) 
		{
			printf ("NAND的volume label是 %s\r\n", buf);
		}
		else 
		{
			printf ("NAND没有volume label\r\n");
		}
		
		printf ("NAND的volume serial number是 %d\r\n", ser_num);
	}
	else 
	{
		printf ("Volume访问错误\r\n");
	}

	/* 获取SD卡剩余容量 */
	ullSdCapacity = ffree("N0:");
	DotFormat(ullSdCapacity, (char *)buf);
	printf("NAND剩余容量 = %10s字节\r\n", buf);
	
	/* 获取相应存储设备的句柄，注意挂载后操作下面的IO控制才是有效的 */
	id = fs_ioc_get_id("N0");          
   
	/* 访问的时候要加上锁 */
	fs_ioc_lock (id);
	
	/* 初始化FAT文件系统格式的存储设备 */

	/* 获取存储设备的扇区信息 */
	restatus = fs_ioc_read_info (id, &info);
	if(restatus == fsOK)
	{
		/* 总的扇区数 * 扇区大小，SD卡的扇区大小是512字节 */
		ullSdCapacity = (uint64_t)info.block_cnt << 9;
		DotFormat(ullSdCapacity, (char *)buf);
		printf("NAND总容量 = %10s字节\r\nSD卡的总扇区数 = %d \r\n", buf, info.block_cnt);
	}
	else
	{
		printf("获取配置信息失败 %s\r\n", ReVal_Table[restatus]);
	}
	
	/* 访问结束要解锁 */
	fs_ioc_unlock (id);

	printf("NAND读扇区大小 = %d字节\r\n", info.read_blen);
	printf("NAND写扇区大小 = %d字节\r\n", info.write_blen);
	
	/* 卸载SD卡 */
	result = funmount("N0:");
	if(result != NULL)
	{
		printf("卸载文件系统失败\r\n");
	}
	else
	{
		printf("卸载文件系统成功\r\n");
	}

	printf("------------------------------------------------------------------\r\n");
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
