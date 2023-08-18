/*
*********************************************************************************************************
*
*	ģ������ : FlashFS�ļ�ϵͳ��ʾģ�顣
*	�ļ����� : demo_nand_flashfs.c
*	��    �� : V1.0
*	˵    �� : ����NAND��BootLoader��
*
*	�޸ļ�¼ :
*		�汾��    ����         ����            ˵��
*       V1.0    2022-06-15   Eric2013    1. RL-FlashFS���ļ����汾V6.x
*
*	Copyright (C), 2022-2030, ���������� www.armfly.com
*
*********************************************************************************************************
*/
#include "includes.h"



/*
*********************************************************************************************************
*	                                        ����
*********************************************************************************************************
*/
/* �������ļ��ڵ��õĺ������� */
static void ViewNANDCapacity(void);
static void DispMenu(void);
static void DotFormat(uint64_t _ullVal, char *_sp);
static void LoadFirmware(void);
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
__IO uint32_t uwCRCValue;
__IO uint32_t uwExpectedCRCValue;
__IO uint32_t uwAppSize;

ALIGN_32BYTES(uint8_t tempbuf[4096]);           /* ��ȡ���ݵĻ��� */

/* FlashFS API�ķ���ֵ */
static const char * ReVal_Table[]= 
{
	"fsOK���ɹ�",				                        
	"fsError��δָ���Ĵ���",
	"fsUnsupported��������֧��",
	"fsAccessDenied����Դ���ʱ��ܾ�",
	
	"fsInvalidParameter��������Ч",
	"fsInvalidDrive��������Ч������������",
	"fsInvalidPath��·����Ч",
	"fsUninitializedDrive������δ��ʼ�� ",

	"fsDriverError����д����",
	"fsMediaError��ý�����",
	"fsNoMedia��ý�鲻���ڣ�����δ��ʼ��",
	"fsNoFileSystem���ļ�ϵͳδ��ʽ��",

	"fsNoFreeSpace��û�п��ÿռ�",
	"fsFileNotFound���ļ�δ�ҵ�",
	"fsDirNotEmpty���ļ��зǿ�",
	"fsTooManyOpenFiles�����ļ�̫��",
};

/*
*********************************************************************************************************
*	�� �� ��: DemoFlashFS
*	����˵��: FlashFS��ϵͳ��ʾ������
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void DemoFlashFS(void)
{
	uint8_t cmd;
	uint8_t result;

	/* ��ӡ�����б��û�����ͨ�����ڲ���ָ�� */
	DispMenu();
	
	result = finit("N0:");
	if(result != NULL)
	{
		/* �������ʧ�ܣ���ز�Ҫ�ٵ���FlashFS������API��������ֹ����Ӳ���쳣 */
		printf("�����ļ�ϵͳʧ�� (%s)\r\n", ReVal_Table[result]);
	}
	else
	{
		printf("�����ļ�ϵͳ�ɹ� (%s)\r\n", ReVal_Table[result]);
	}
	
	while(1)
	{
		if (comGetChar(COM1, &cmd))	/* �Ӵ��ڶ���һ���ַ�(��������ʽ) */
		{
			printf("\r\n");
			switch (cmd)
			{
				case '1':
					printf("��1 - ��ʾNAND������ʣ��������\r\n");
					ViewNANDCapacity();		
					break;
				
				case '2':
					printf("��2 - �����̼�����, У�����ת��\r\n");
					LoadFirmware();
					BootHexCrcVeriy();
					JumpToApp();
					break;

				case 'a':
					printf("��a - OpenUSB Device��\r\n");
					printf("ע�⣬NAND�����ڼ䣬���ɲ����������Ҫж�غ�ſ��Բ�������\r\n");
					USBD_Initialize(0U);                 
					USBD_Connect   (0U);      
					break;	

				case 'b':
					printf("��b - CloseUSB Device��\r\n");
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
*	�� �� ��: DispMenu
*	����˵��: ��ʾ������ʾ�˵�
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void DispMenu(void)
{
	printf("\r\n------------------------------------------------\r\n");
	printf("��ѡ�񴮿ڲ���������Լ��̴�ӡ���ּ���:\r\n");
	printf("�״�ʹ�ã���ص��ô�������1������NAND Flash�ĵͼ���ʽ�����ļ�ϵͳ��ʽ��\r\n");	
	printf("1 - ��ʾNAND������ʣ������\r\n");
	printf("2 - �����̼�����, У�����ת\r\n");
	printf("a - ��NANDģ��U�̣�ע����Сд��ĸa\r\n");
	printf("b - �ر�NANDģ��U�̣�ע����Сд��ĸb\r\n");
}

/*
*********************************************************************************************************
*	�� �� ��: JumpToApp
*	����˵��: ��ת��Ӧ��JumpToApp
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void JumpToApp(void)
{
	uint32_t i=0;
	void (*AppJump)(void);         /* ����һ������ָ�� */
	
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
	
	/* ��1���������ļ�ϵͳ ***************************************************************/
	result = fmount("N0:");
	
	if(result != NULL)
	{
		/* �������ʧ�ܣ���ز�Ҫ�ٵ���FlashFS������API��������ֹ����Ӳ���쳣 */
		printf("�����ļ�ϵͳʧ�� (%s)\r\n", ReVal_Table[result]);
		goto access_fail;
	}
	else
	{
		printf("�����ļ�ϵͳ�ɹ� (%s)\r\n", ReVal_Table[result]);
	}

	/* ��2�������ļ� ***************************************************************/
	fin = fopen ("N0:\\app.bin", "r"); 
	if (fin != NULL) 
	{
		/* �ļ��򿪳ɹ� */
	}
	else
	{
		printf("���ļ�N0:\\app.binʧ��, �����ļ�������\r\n");
		goto access_fail;
	}
	
	/* ��3������ȡ�ļ���С��APP�̼��汾 ***********************************************/
	if (ffind ("N0:\\app.bin", &info) == fsOK) 
	{
		/* �ļ���Ϣ��ȡʶ�� */		
	}
	else
	{
		printf("��ȡ�ļ���Ϣʧ��\r\n");
		goto access_fail;
	}

	/* ��ӡ�ļ���С, ���4G */
	printf("APP�̼���С��%d\r\n", (int)info.size);
	
	uwAppSize = info.size;
	
	fseek(fin, 28L, SEEK_SET);
	fread(&ver, sizeof(uint32_t), 1, fin);
	fseek(fin, 0L, SEEK_SET);	
	printf("APP�̼��汾��V%X.%02X\r\n", ver >> 8, ver & 0xFF);
	
	/* ��4������������ **************************************************************/
	SectorCount = info.size/(128*1024);
	SectorRemain = info.size%(128*1024);	
	
	for(i = 0; i < SectorCount; i++)
	{
		bsp_EraseCpuFlash((uint32_t)(AppAddr + i*128*1024));
		printf("�������� = %08x\r\n", AppAddr + i*128*1024);
	}
	
	if(SectorRemain)
	{
		bsp_EraseCpuFlash((uint32_t)(AppAddr + i*128*1024));
		printf("����ʣ������ = %08x\r\n", AppAddr + i*128*1024);
	}
	
	/* ��5������APP�̼�д�뵽�ڲ�Flash ********************************************/
	for(;;)
	{
		/* ��ȡһ�����������ݵ�buf */
		bw = fread(tempbuf, sizeof(uint8_t), sizeof(tempbuf)/sizeof(uint8_t), fin);
		
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
		FinishPecent = (float)(TotalSize) / info.size;
		printf("��ǰ��ɱ��:%02d%%\r\n", (uint8_t)(FinishPecent*100));
		
		/* ��ȡ������߶�ȡ��ϣ��˳� */
		if (bw < sizeof(tempbuf)/sizeof(uint8_t))
		{
			printf("APP�̼��������\r\n");
			fclose(fin);
			break;
		}
	}
	
access_fail:
	/* ж��SD�� */
	result = funmount("N0:");
	if(result != NULL)
	{
		printf("ж���ļ�ϵͳʧ��\r\n");
	}
	else
	{
		printf("ж���ļ�ϵͳ�ɹ�\r\n");
	}
	
	printf("------------------------------------------------------------------\r\n");
}

/*
*********************************************************************************************************
*	�� �� ��: DotFormatjavascript:;
*	����˵��: �����ݹ淶����ʾ�������û��鿴
*             ����
*             2345678   ---->  2.345.678
*             334426783 ---->  334.426.783
*             ��������Ϊ��λ������ʾ
*	��    ��: _ullVal   ��Ҫ�淶��ʾ����ֵ
*             _sp       �淶��ʾ�����ݴ洢��buf��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void DotFormat(uint64_t _ullVal, char *_sp) 
{
	/* ��ֵ���ڵ���10^9 */
	if (_ullVal >= (uint64_t)1e9) 
	{
		_sp += sprintf (_sp, "%d.", (uint32_t)(_ullVal / (uint64_t)1e9));
		_ullVal %= (uint64_t)1e9;
		_sp += sprintf (_sp, "%03d.", (uint32_t)(_ullVal / (uint64_t)1e6));
		_ullVal %= (uint64_t)1e6;
		sprintf (_sp, "%03d.%03d", (uint32_t)(_ullVal / 1000), (uint32_t)(_ullVal % 1000));
		return;
	}
	
	/* ��ֵ���ڵ���10^6 */
	if (_ullVal >= (uint64_t)1e6) 
	{
		_sp += sprintf (_sp,"%d.", (uint32_t)(_ullVal / (uint64_t)1e6));
		_ullVal %= (uint64_t)1e6;
		sprintf (_sp,"%03d.%03d", (uint32_t)(_ullVal / 1000), (uint32_t)(_ullVal % 1000));
		return;
	}
	
	/* ��ֵ���ڵ���10^3 */
	if (_ullVal >= 1000) 
	{
		sprintf (_sp, "%d.%03d", (uint32_t)(_ullVal / 1000), (uint32_t)(_ullVal % 1000));
		return;
	}
	
	/* ������ֵ */
	sprintf (_sp,"%d",(uint32_t)(_ullVal));
}

/*
*********************************************************************************************************
*	�� �� ��: ViewNANDCapacity
*	����˵��: NAND�Ĺ��أ�ж�ؼ���������ʾ
*	��    ��: ��
*	�� �� ֵ: ��
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
	
	/* ����SD�� */
	result = fmount("N0:");
	if(result != NULL)
	{
		/* �������ʧ�ܣ���ز�Ҫ�ٵ���FlashFS������API��������ֹ����Ӳ���쳣 */
		printf("�����ļ�ϵͳʧ�� (%s)\r\n", ReVal_Table[result]);
	}
	else
	{
		printf("�����ļ�ϵͳ�ɹ� (%s)\r\n", ReVal_Table[result]);
	}
	
	/* ��ʽ�� */
	opt = "/LL /L nand /FAT16";
	printf("�ļ�ϵͳ��ʽ��......\r\n");
	result = fformat ("N0:", opt);
	printf("�ļ�ϵͳ��ʽ�� (%s)\r\n", ReVal_Table[result]);
	
	printf("------------------------------------------------------------------\r\n");
	
	/* ��ȡvolume label */
	if (fvol ("N0:", (char *)buf, &ser_num) == 0) 
	{
		if (buf[0]) 
		{
			printf ("NAND��volume label�� %s\r\n", buf);
		}
		else 
		{
			printf ("NANDû��volume label\r\n");
		}
		
		printf ("NAND��volume serial number�� %d\r\n", ser_num);
	}
	else 
	{
		printf ("Volume���ʴ���\r\n");
	}

	/* ��ȡSD��ʣ������ */
	ullSdCapacity = ffree("N0:");
	DotFormat(ullSdCapacity, (char *)buf);
	printf("NANDʣ������ = %10s�ֽ�\r\n", buf);
	
	/* ��ȡ��Ӧ�洢�豸�ľ����ע����غ���������IO���Ʋ�����Ч�� */
	id = fs_ioc_get_id("N0");          
   
	/* ���ʵ�ʱ��Ҫ������ */
	fs_ioc_lock (id);
	
	/* ��ʼ��FAT�ļ�ϵͳ��ʽ�Ĵ洢�豸 */

	/* ��ȡ�洢�豸��������Ϣ */
	restatus = fs_ioc_read_info (id, &info);
	if(restatus == fsOK)
	{
		/* �ܵ������� * ������С��SD����������С��512�ֽ� */
		ullSdCapacity = (uint64_t)info.block_cnt << 9;
		DotFormat(ullSdCapacity, (char *)buf);
		printf("NAND������ = %10s�ֽ�\r\nSD������������ = %d \r\n", buf, info.block_cnt);
	}
	else
	{
		printf("��ȡ������Ϣʧ�� %s\r\n", ReVal_Table[restatus]);
	}
	
	/* ���ʽ���Ҫ���� */
	fs_ioc_unlock (id);

	printf("NAND��������С = %d�ֽ�\r\n", info.read_blen);
	printf("NANDд������С = %d�ֽ�\r\n", info.write_blen);
	
	/* ж��SD�� */
	result = funmount("N0:");
	if(result != NULL)
	{
		printf("ж���ļ�ϵͳʧ��\r\n");
	}
	else
	{
		printf("ж���ļ�ϵͳ�ɹ�\r\n");
	}

	printf("------------------------------------------------------------------\r\n");
}

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
