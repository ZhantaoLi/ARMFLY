/*
*********************************************************************************************************
*
*	模块名称 : SD卡Fat文件系统和SD卡模拟演示模块。
*	文件名称 : demo_sd_fatfs.c
*	版    本 : V1.0
*	说    明 : SD卡的固件加载支持串口命令:
*              1 - 显示SD卡根目录下的文件名
*              2 - 启动固件加载, 校验和跳转
*
*	修改记录 :
*		版本号   日期         作者        说明
*		V1.0    2022-06-26   Eric2013    正式发布
*
*	Copyright (C), 2022-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/
#include "bsp.h"
#include "ff.h"		
#include "ff_gen_drv.h"
#include "sd_diskio_dma.h"

#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/aes.h"


/*
*********************************************************************************************************
*	                                        函数
*********************************************************************************************************
*/
/* 仅允许本文件内调用的函数声明 */
static void LoadFirmware(void);
static void DispMenu(void);
static void ViewRootDir(void);
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
/* 变量 */
mbedtls_aes_context aes;

unsigned char key[] =
{
  0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c
};

unsigned char iv[] =
{
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f
};

FATFS fs;
FIL file;
DIR DirInf;
FILINFO FileInf;
__IO uint32_t uwCRCValue;
__IO uint32_t uwExpectedCRCValue;
__IO uint32_t uwAppSize;

ALIGN_32BYTES(uint8_t output[4096]);  /* 读取数据的缓冲 */
ALIGN_32BYTES(uint8_t tempbuf[4096]);  /* 读取数据的缓冲 */
char DiskPath[4];                      /* SD卡逻辑驱动路径，比盘符0，就是"0:/" */

/* FatFs API的返回值 */
static const char * FR_Table[]= 
{
	"FR_OK：成功",				                             /* (0) Succeeded */
	"FR_DISK_ERR：底层硬件错误",			                 /* (1) A hard error occurred in the low level disk I/O layer */
	"FR_INT_ERR：断言失败",				                     /* (2) Assertion failed */
	"FR_NOT_READY：物理驱动没有工作",			             /* (3) The physical drive cannot work */
	"FR_NO_FILE：文件不存在",				                 /* (4) Could not find the file */
	"FR_NO_PATH：路径不存在",				                 /* (5) Could not find the path */
	"FR_INVALID_NAME：无效文件名",		                     /* (6) The path name format is invalid */
	"FR_DENIED：由于禁止访问或者目录已满访问被拒绝",         /* (7) Access denied due to prohibited access or directory full */
	"FR_EXIST：文件已经存在",			                     /* (8) Access denied due to prohibited access */
	"FR_INVALID_OBJECT：文件或者目录对象无效",		         /* (9) The file/directory object is invalid */
	"FR_WRITE_PROTECTED：物理驱动被写保护",		             /* (10) The physical drive is write protected */
	"FR_INVALID_DRIVE：逻辑驱动号无效",		                 /* (11) The logical drive number is invalid */
	"FR_NOT_ENABLED：卷中无工作区",			                 /* (12) The volume has no work area */
	"FR_NO_FILESYSTEM：没有有效的FAT卷",		             /* (13) There is no valid FAT volume */
	"FR_MKFS_ABORTED：由于参数错误f_mkfs()被终止",	         /* (14) The f_mkfs() aborted due to any parameter error */
	"FR_TIMEOUT：在规定的时间内无法获得访问卷的许可",		 /* (15) Could not get a grant to access the volume within defined period */
	"FR_LOCKED：由于文件共享策略操作被拒绝",				 /* (16) The operation is rejected according to the file sharing policy */
	"FR_NOT_ENOUGH_CORE：无法分配长文件名工作区",		     /* (17) LFN working buffer could not be allocated */
	"FR_TOO_MANY_OPEN_FILES：当前打开的文件数大于_FS_SHARE", /* (18) Number of open files > _FS_SHARE */
	"FR_INVALID_PARAMETER：参数无效"	                     /* (19) Given parameter is invalid */
};

/*
*********************************************************************************************************
*	函 数 名: DemoFatFS
*	功能说明: FatFS文件系统演示主程序
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void DemoFatFS(void)
{
	uint8_t cmd;

	/* 打印命令列表，用户可以通过串口操作指令 */
	DispMenu();
	
	/* 注册SD卡驱动 */
	FATFS_LinkDriver(&SD_Driver, DiskPath);
	
	bsp_StartAutoTimer(0, 500);	/* 启动1个500ms的自动重装的定时器 */
	
	while (1)
	{
		/* 判断定时器超时时间 */
		if (bsp_CheckTimer(0))	
		{            
			/* 每隔500ms 进来一次 */  
			bsp_LedToggle(2);
		}

		if (comGetChar(COM1, &cmd))	/* 从串口读入一个字符(非阻塞方式) */
		{
			printf("\r\n");
			switch (cmd)
			{
				case '1':
					printf("【1 - 显示SD卡根目录下的文件名】\r\n");
					ViewRootDir();
					break;
				
				case '2':
					printf("【2 - 启动固件解密编程, 校验和跳转】\r\n");
					LoadFirmware();		
					BootHexCrcVeriy();
					JumpToApp();
					break;
	
				default:
					DispMenu();
					break;
			}
		}
	}
}

/*
*********************************************************************************************************
*	函 数 名: DispMenu
*	功能说明: 显示操作提示菜单
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DispMenu(void)
{
	printf("请选择操作命令:\r\n");
	printf("1 - 显示SD卡根目录下的文件名\r\n");
	printf("2 - 启动固件加载, 校验和跳转\r\n");
}

/*
*********************************************************************************************************
*	函 数 名: JumpToApp
*	功能说明: 跳转到应用JumpToApp
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
extern uint8_t BSP_SD_DeInit(void);
static void JumpToApp(void)
{
	uint32_t i=0;
	void (*AppJump)(void);         /* 声明一个函数指针 */
    
	BSP_SD_DeInit();
	
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
	FRESULT result;
	float FinishPecent;
	uint32_t bw;
	char path[64];
	uint32_t SectorCount = 0;
	uint32_t SectorRemain = 0;
	FILINFO fno;
	uint32_t i = 0;
	uint8_t ucState;
    uint32_t Count = 0;
    uint32_t TotalSize = 0;
	
	
	mbedtls_aes_init(&aes);
	mbedtls_aes_setkey_dec(&aes, key, 128);
	
	/* 第1步：挂载文件系统 ***************************************************************/
	result = f_mount(&fs, DiskPath, 0);		
	if (result != FR_OK)
	{
		printf("挂载文件系统失败 (%s)\r\n", FR_Table[result]);
	}

	/* 第2步：打开文件 ***************************************************************/
	sprintf(path, "%saes.bin", DiskPath);
	result = f_open(&file, path, FA_OPEN_EXISTING | FA_READ);
	if (result !=  FR_OK)
	{
		printf("Don't Find File : 128.bin\r\n");
		return;
	}
	
	/* 第2步：获取文件大小和APP固件版本 ***********************************************/
	f_stat(path, &fno);

	/* 打印文件大小, 最大4G */
	printf("APP固件大小：%d\r\n", (int)fno.fsize);
	
	uwAppSize = fno.fsize;

	/* 第4步：扇区擦除 **************************************************************/
	SectorCount = fno.fsize/(128*1024);
	SectorRemain = fno.fsize%(128*1024);	
	
	for(i = 0; i < SectorCount; i++)
	{
		printf("开始擦除扇区 = %08x\r\n", AppAddr + i*128*1024);
		bsp_EraseCpuFlash((uint32_t)(AppAddr + i*128*1024));
	}
	
	if(SectorRemain)
	{
		printf("开始擦除剩余扇区 = %08x\r\n", AppAddr + i*128*1024);
		bsp_EraseCpuFlash((uint32_t)(AppAddr + i*128*1024));
	}
	
	/* 第5步：将APP固件写入到内部Flash ********************************************/
	for(;;)
	{
		/* 读取一个扇区的数据到buf */
		result = f_read(&file, &tempbuf, sizeof(tempbuf), &bw);
		mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_DECRYPT, sizeof(tempbuf), iv, tempbuf, output);
		
		
		/* 读取出错或者读取完毕，退出 */
		if ((result != FR_OK)||bw == 0)
		{
			printf("APP固件加载完毕\r\n");
			printf("=================================================\r\n");
			break;
		}
		
		/* 编程内部Flash */
		TotalSize += bw;
		ucState = bsp_WriteCpuFlash((uint32_t)(AppAddr + Count*sizeof(tempbuf)),  (uint8_t *)output, bw);
		
		/* 如果返回非0，表示编程失败 */
		if(ucState != 0)
		{
			printf("擦除失败\r\n");
			break;
		}
		
		/* 显示复制进度 */
		Count = Count + 1;
		FinishPecent = (float)(TotalSize) / fno.fsize;
		printf("当前AES解析和编程:%02d%%\r\n", (uint8_t)(FinishPecent*100));
	}
	
	/* 关闭文件*/
	f_close(&file);

	/* 卸载文件系统 */
	f_mount(NULL, DiskPath, 0);
}

/*
*********************************************************************************************************
*	函 数 名: ViewRootDir
*	功能说明: 显示SD卡根目录下的文件名
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
extern SD_HandleTypeDef uSdHandle;
static void ViewRootDir(void)
{
	FRESULT result;
	uint32_t cnt = 0;
	FILINFO fno;
	
 	/* 挂载文件系统 */
	result = f_mount(&fs, DiskPath, 0);	/* Mount a logical drive */
	if (result != FR_OK)
	{
		printf("挂载文件系统失败 (%s)\r\n", FR_Table[result]);
	}

	/* 打开根文件夹 */
	result = f_opendir(&DirInf, DiskPath); /* 如果不带参数，则从当前目录开始 */
	if (result != FR_OK)
	{
		printf("打开根目录失败  (%s)\r\n", FR_Table[result]);
		return;
	}

	printf("属性        |  文件大小 | 短文件名 | 长文件名\r\n");
	for (cnt = 0; ;cnt++)
	{
		result = f_readdir(&DirInf, &FileInf); 		/* 读取目录项，索引会自动下移 */
		if (result != FR_OK || FileInf.fname[0] == 0)
		{
			break;
		}

		if (FileInf.fname[0] == '.')
		{
			continue;
		}

		/* 判断是文件还是子目录 */
		if (FileInf.fattrib & AM_DIR)
		{
			printf("(0x%02d)目录  ", FileInf.fattrib);
		}
		else
		{
			printf("(0x%02d)文件  ", FileInf.fattrib);
		}

		f_stat(FileInf.fname, &fno);
		
		/* 打印文件大小, 最大4G */
		printf(" %10d", (int)fno.fsize);


		printf("  %s\r\n", (char *)FileInf.fname);	/* 长文件名 */
	}
 
    /* 打印卡速度信息 */
    if(uSdHandle.SdCard.CardSpeed == CARD_NORMAL_SPEED)
    {
        printf("Normal Speed Card <12.5MB/S, MAX Clock < 25MHz, Spec Version 1.01\r\n");           
    }
    else if (uSdHandle.SdCard.CardSpeed == CARD_HIGH_SPEED)
    {
        printf("High Speed Card <25MB/s, MAX Clock < 50MHz, Spec Version 2.00\r\n");            
    }
    else if (uSdHandle.SdCard.CardSpeed == CARD_ULTRA_HIGH_SPEED)
    {
        printf("UHS-I SD Card <50MB/S for SDR50, DDR50 Cards, MAX Clock < 50MHz OR 100MHz\r\n");
        printf("UHS-I SD Card <104MB/S for SDR104, MAX Clock < 108MHz, Spec version 3.01\r\n");   
    }    

    
	/* 卸载文件系统 */
	 f_mount(NULL, DiskPath, 0);
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
