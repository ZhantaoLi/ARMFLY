/*
*********************************************************************************************************
*
*	ģ������ : SD��Fat�ļ�ϵͳ��SD��ģ����ʾģ�顣
*	�ļ����� : demo_sd_fatfs.c
*	��    �� : V1.0
*	˵    �� : SD���Ĺ̼�����֧�ִ�������:
*              1 - ��ʾSD����Ŀ¼�µ��ļ���
*              2 - �����̼�����, У�����ת
*
*	�޸ļ�¼ :
*		�汾��   ����         ����        ˵��
*		V1.0    2022-06-15   Eric2013    ��ʽ����
*
*	Copyright (C), 2022-2030, ���������� www.armfly.com
*
*********************************************************************************************************
*/
#include "bsp.h"
#include "ff.h"		
#include "ff_gen_drv.h"
#include "sd_diskio_dma.h"


/*
*********************************************************************************************************
*	                                        ����
*********************************************************************************************************
*/
/* �������ļ��ڵ��õĺ������� */
static void LoadFirmware(void);
static void DispMenu(void);
static void ViewRootDir(void);
static void JumpToApp(void);
static void BootHexCrcVeriy(void);

/*
*********************************************************************************************************
*	                                       �궨��
*********************************************************************************************************
*/
#define AppAddr  0x08100000    /* APP��ַ */

/*
*********************************************************************************************************
*	                                       ����
*********************************************************************************************************
*/
FATFS fs;
FIL file;
DIR DirInf;
FILINFO FileInf;
__IO uint32_t uwCRCValue;
__IO uint32_t uwExpectedCRCValue;
__IO uint32_t uwAppSize;

ALIGN_32BYTES(uint8_t tempbuf[4096]);  /* ��ȡ���ݵĻ��� */
char DiskPath[4];                      /* SD���߼�����·�������̷�0������"0:/" */

/* FatFs API�ķ���ֵ */
static const char * FR_Table[]= 
{
	"FR_OK���ɹ�",				                             /* (0) Succeeded */
	"FR_DISK_ERR���ײ�Ӳ������",			                 /* (1) A hard error occurred in the low level disk I/O layer */
	"FR_INT_ERR������ʧ��",				                     /* (2) Assertion failed */
	"FR_NOT_READY����������û�й���",			             /* (3) The physical drive cannot work */
	"FR_NO_FILE���ļ�������",				                 /* (4) Could not find the file */
	"FR_NO_PATH��·��������",				                 /* (5) Could not find the path */
	"FR_INVALID_NAME����Ч�ļ���",		                     /* (6) The path name format is invalid */
	"FR_DENIED�����ڽ�ֹ���ʻ���Ŀ¼�������ʱ��ܾ�",         /* (7) Access denied due to prohibited access or directory full */
	"FR_EXIST���ļ��Ѿ�����",			                     /* (8) Access denied due to prohibited access */
	"FR_INVALID_OBJECT���ļ�����Ŀ¼������Ч",		         /* (9) The file/directory object is invalid */
	"FR_WRITE_PROTECTED������������д����",		             /* (10) The physical drive is write protected */
	"FR_INVALID_DRIVE���߼���������Ч",		                 /* (11) The logical drive number is invalid */
	"FR_NOT_ENABLED�������޹�����",			                 /* (12) The volume has no work area */
	"FR_NO_FILESYSTEM��û����Ч��FAT��",		             /* (13) There is no valid FAT volume */
	"FR_MKFS_ABORTED�����ڲ�������f_mkfs()����ֹ",	         /* (14) The f_mkfs() aborted due to any parameter error */
	"FR_TIMEOUT���ڹ涨��ʱ�����޷���÷��ʾ�����",		 /* (15) Could not get a grant to access the volume within defined period */
	"FR_LOCKED�������ļ�������Բ������ܾ�",				 /* (16) The operation is rejected according to the file sharing policy */
	"FR_NOT_ENOUGH_CORE���޷����䳤�ļ���������",		     /* (17) LFN working buffer could not be allocated */
	"FR_TOO_MANY_OPEN_FILES����ǰ�򿪵��ļ�������_FS_SHARE", /* (18) Number of open files > _FS_SHARE */
	"FR_INVALID_PARAMETER��������Ч"	                     /* (19) Given parameter is invalid */
};

/*
*********************************************************************************************************
*	�� �� ��: DemoFatFS
*	����˵��: FatFS�ļ�ϵͳ��ʾ������
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void DemoFatFS(void)
{
	uint8_t cmd;

	/* ��ӡ�����б��û�����ͨ�����ڲ���ָ�� */
	DispMenu();
	
	/* ע��SD������ */
	FATFS_LinkDriver(&SD_Driver, DiskPath);
	
	bsp_StartAutoTimer(0, 500);	/* ����1��500ms���Զ���װ�Ķ�ʱ�� */
	
	while (1)
	{
		/* �ж϶�ʱ����ʱʱ�� */
		if (bsp_CheckTimer(0))	
		{            
			/* ÿ��500ms ����һ�� */  
			bsp_LedToggle(2);
		}

		if (comGetChar(COM1, &cmd))	/* �Ӵ��ڶ���һ���ַ�(��������ʽ) */
		{
			printf("\r\n");
			switch (cmd)
			{
				case '1':
					printf("��1 - ��ʾSD����Ŀ¼�µ��ļ�����\r\n");
					ViewRootDir();
					break;
				
				case '2':
					printf("��2 - �����̼�����, У�����ת��\r\n");
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
*	�� �� ��: DispMenu
*	����˵��: ��ʾ������ʾ�˵�
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void DispMenu(void)
{
	printf("��ѡ���������:\r\n");
	printf("1 - ��ʾSD����Ŀ¼�µ��ļ���\r\n");
	printf("2 - �����̼�����, У�����ת\r\n");
}

/*
*********************************************************************************************************
*	�� �� ��: JumpToApp
*	����˵��: ��ת��Ӧ��JumpToApp
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
extern uint8_t BSP_SD_DeInit(void);
static void JumpToApp(void)
{
	uint32_t i=0;
	void (*AppJump)(void);         /* ����һ������ָ�� */
    
	BSP_SD_DeInit();
	
    /* �ر�ȫ���ж� */
	DISABLE_INT(); 
    
    /* ��������ʱ�ӵ�Ĭ��״̬��ʹ��HSIʱ�� */
	HAL_RCC_DeInit();
    
	/* �رյδ�ʱ������λ��Ĭ��ֵ */
	SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL = 0;

	/* �ر������жϣ���������жϹ����־ */
	for (i = 0; i < 8; i++)
	{
		NVIC->ICER[i]=0xFFFFFFFF;
		NVIC->ICPR[i]=0xFFFFFFFF;
	}	

	/* ʹ��ȫ���ж� */
	ENABLE_INT();

	/* ��ת��Ӧ�ó����׵�ַ��MSP����ַ+4�Ǹ�λ�жϷ�������ַ */
	AppJump = (void (*)(void)) (*((uint32_t *) (AppAddr + 4)));

	/* ��������ջָ�� */
	__set_MSP(*(uint32_t *)AppAddr);
	
	/* ��RTOS���̣�����������Ҫ������Ϊ��Ȩ��ģʽ��ʹ��MSPָ�� */
	__set_CONTROL(0);

	/* ��ת��ϵͳBootLoader */
	AppJump(); 

	/* ��ת�ɹ��Ļ�������ִ�е�����û�������������Ӵ��� */
	while (1)
	{

	}
}

/*
*********************************************************************************************************
*	�� �� ��: BootHexCrcVeriy
*	����˵��: �̼�CRCУ��
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void BootHexCrcVeriy(void)
{
	CRC_HandleTypeDef   CrcHandle = {0};
	
	/* ��ȡbin�ļ���CRC */
	uwExpectedCRCValue  = *(__IO uint32_t *)(AppAddr + uwAppSize - 4);
	
	/* ��ʼ��Ӳ��CRC */
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

	/* �����Ƿ���Ӳ��CRCһ�� */
	uwCRCValue = HAL_CRC_Calculate(&CrcHandle, (uint32_t *)(AppAddr), uwAppSize/4 - 1);
	
	printf("ʵ��APP�̼�CRCУ��ֵ = 0x%x\r\n", uwExpectedCRCValue);
	printf("����APP�̼�CRCУ��ֵ = 0x%x\r\n", uwCRCValue);	
	
	if (uwCRCValue != uwExpectedCRCValue)
	{
		printf("У��ʧ��\r\n");
        Error_Handler(__FILE__, __LINE__);		
	}
	else
	{
		printf("У��ͨ������ʼ��ת��APP\r\n");
	}
	
	printf("=================================================\r\n");
}

/*
*********************************************************************************************************
*	�� �� ��: LoadFirmware
*	����˵��: �̼�����
*	��    ��: ��
*	�� �� ֵ: ��
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
	uint32_t ver;
	
	
	/* ��1���������ļ�ϵͳ ***************************************************************/
	result = f_mount(&fs, DiskPath, 0);		
	if (result != FR_OK)
	{
		printf("�����ļ�ϵͳʧ�� (%s)\r\n", FR_Table[result]);
	}

	/* ��2�������ļ� ***************************************************************/
	sprintf(path, "%sapp.bin", DiskPath);
	result = f_open(&file, path, FA_OPEN_EXISTING | FA_READ);
	if (result !=  FR_OK)
	{
		printf("Don't Find File : app.bin\r\n");
		return;
	}
	
	/* ��2������ȡ�ļ���С��APP�̼��汾 ***********************************************/
	f_stat(path, &fno);

	/* ��ӡ�ļ���С, ���4G */
	printf("APP�̼���С��%d\r\n", (int)fno.fsize);
	
	uwAppSize = fno.fsize;
	
	/* ��ȡ�̼���С */
	f_lseek(&file, 28);
	f_read(&file, &ver, sizeof(ver), &bw);
	f_lseek(&file, 0);
	printf("APP�̼��汾��V%X.%02X\r\n", ver >> 8, ver & 0xFF);
	
	/* ��4������������ **************************************************************/
	SectorCount = fno.fsize/(128*1024);
	SectorRemain = fno.fsize%(128*1024);	
	
	for(i = 0; i < SectorCount; i++)
	{
		printf("��ʼ�������� = %08x\r\n", AppAddr + i*128*1024);
		bsp_EraseCpuFlash((uint32_t)(AppAddr + i*128*1024));
	}
	
	if(SectorRemain)
	{
		printf("��ʼ����ʣ������ = %08x\r\n", AppAddr + i*128*1024);
		bsp_EraseCpuFlash((uint32_t)(AppAddr + i*128*1024));
	}
	
	/* ��5������APP�̼�д�뵽�ڲ�Flash ********************************************/
	for(;;)
	{
		/* ��ȡһ�����������ݵ�buf */
		result = f_read(&file, &tempbuf, sizeof(tempbuf), &bw);
		
		/* ��ȡ������߶�ȡ��ϣ��˳� */
		if ((result != FR_OK)||bw == 0)
		{
			printf("APP�̼��������\r\n");
			printf("=================================================\r\n");
			break;
		}
		
		/* ����ڲ�Flash */
		TotalSize += bw;
		ucState = bsp_WriteCpuFlash((uint32_t)(AppAddr + Count*sizeof(tempbuf)),  (uint8_t *)tempbuf, bw);
		
		/* ������ط�0����ʾ���ʧ�� */
		if(ucState != 0)
		{
			printf("����ʧ��\r\n");
			break;
		}
		
		/* ��ʾ���ƽ��� */
		Count = Count + 1;
		FinishPecent = (float)(TotalSize) / fno.fsize;
		printf("��ǰ��ɱ��:%02d%%\r\n", (uint8_t)(FinishPecent*100));
	}
	
	/* �ر��ļ�*/
	f_close(&file);

	/* ж���ļ�ϵͳ */
	f_mount(NULL, DiskPath, 0);
}

/*
*********************************************************************************************************
*	�� �� ��: ViewRootDir
*	����˵��: ��ʾSD����Ŀ¼�µ��ļ���
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
extern SD_HandleTypeDef uSdHandle;
static void ViewRootDir(void)
{
	FRESULT result;
	uint32_t cnt = 0;
	FILINFO fno;
	
 	/* �����ļ�ϵͳ */
	result = f_mount(&fs, DiskPath, 0);	/* Mount a logical drive */
	if (result != FR_OK)
	{
		printf("�����ļ�ϵͳʧ�� (%s)\r\n", FR_Table[result]);
	}

	/* �򿪸��ļ��� */
	result = f_opendir(&DirInf, DiskPath); /* ���������������ӵ�ǰĿ¼��ʼ */
	if (result != FR_OK)
	{
		printf("�򿪸�Ŀ¼ʧ��  (%s)\r\n", FR_Table[result]);
		return;
	}

	printf("����        |  �ļ���С | ���ļ��� | ���ļ���\r\n");
	for (cnt = 0; ;cnt++)
	{
		result = f_readdir(&DirInf, &FileInf); 		/* ��ȡĿ¼��������Զ����� */
		if (result != FR_OK || FileInf.fname[0] == 0)
		{
			break;
		}

		if (FileInf.fname[0] == '.')
		{
			continue;
		}

		/* �ж����ļ�������Ŀ¼ */
		if (FileInf.fattrib & AM_DIR)
		{
			printf("(0x%02d)Ŀ¼  ", FileInf.fattrib);
		}
		else
		{
			printf("(0x%02d)�ļ�  ", FileInf.fattrib);
		}

		f_stat(FileInf.fname, &fno);
		
		/* ��ӡ�ļ���С, ���4G */
		printf(" %10d", (int)fno.fsize);


		printf("  %s\r\n", (char *)FileInf.fname);	/* ���ļ��� */
	}
 
    /* ��ӡ���ٶ���Ϣ */
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

    
	/* ж���ļ�ϵͳ */
	 f_mount(NULL, DiskPath, 0);
}

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
