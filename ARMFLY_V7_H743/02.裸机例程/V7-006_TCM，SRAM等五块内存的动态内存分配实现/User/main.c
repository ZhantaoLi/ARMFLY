/*
*********************************************************************************************************
*
*	ģ������ : ������ģ��
*	�ļ����� : main.c
*	��    �� : V1.0
*	˵    �� : TCM��SRAM������ڴ�Ķ�̬�ڴ����ʵ�֡�
*              ʵ��Ŀ�ģ�
*                1. ѧϰTCM��SRAM������ڴ�Ķ�̬�ڴ����ʵ�֡�
*              ʵ�����ݣ�
*                1. �����Զ���װ�����ʱ��0��ÿ100ms��תһ��LED2��
*              ʵ�������
*                1. K1�����£���DTCM��������280�ֽڣ�64�ֽں�6111�ֽڡ�
*                2. K1���ɿ����ͷŴ�DTCM����Ŀռ䡣
*                3. K2�����£���AXI SRAM��������160�ֽڣ�32�ֽں�2333�ֽڡ�
*                4. K2���ɿ����ͷŴ�AXI SRAM����Ŀռ䡣
*                5. K3�����£���D2��SRAM��������200�ֽڣ�96�ֽں�4111�ֽڡ�
*                6. K3���ɿ����ͷŴ�D2��SRAM����Ŀռ䡣
*                7. ҡ��OK�����£���D3��SRAM��������300�ֽڣ�128�ֽں�5111�ֽڡ�
*                8. ҡ��OK���ɿ����ͷŴ�D3��SRAM����Ŀռ䡣
*              ע�����
*                1. ��ʵ���Ƽ�ʹ�ô������SecureCRT�鿴��ӡ��Ϣ��������115200������λ8����żУ��λ�ޣ�ֹͣλ1��
*                2. ��ؽ��༭��������������TAB����Ϊ4���Ķ����ļ���Ҫ��������ʾ�����롣
*
*	�޸ļ�¼ :
*		�汾��   ����         ����        ˵��
*		V1.0    2018-12-12   Eric2013     1. CMSIS����汾 V5.4.0
*                                         2. HAL��汾 V1.3.0
*
*	Copyright (C), 2018-2030, ���������� www.armfly.com
*
*********************************************************************************************************
*/	
#include "bsp.h"			/* �ײ�Ӳ������ */



/* ���������������̷������� */
#define EXAMPLE_NAME	"V7-TCM��SRAM������ڴ�Ķ�̬�ڴ����ʵ��"
#define EXAMPLE_DATE	"2018-12-12"
#define DEMO_VER		"1.0"

static void PrintfLogo(void);
static void PrintfHelp(void);


/* DTCM, 64KB */
mem_head_t *DTCMUsed;
uint64_t AppMallocDTCM[64*1024/8];


#if defined ( __ICCARM__ )    /* ʹ�õ�IAR */

/* D1��, AXI SRAM, 512KB */
mem_head_t *AXISRAMUsed;
#pragma location = 0x24000000
uint64_t AppMallocAXISRAM[512*1024/8];

/* D2��, 128KB SRAM1(0x30000000) + 128KB SRAM2(0x30020000) + 32KB SRAM3(0x30040000)  */
mem_head_t *SRAM1Used; 
#pragma location = 0x30000000
uint64_t AppMallocSRAM1[288*1024/8];

/* D3��, SRAM4, 64KB */
mem_head_t *SRAM4Used;
#pragma location = 0x38000000
uint64_t AppMallocSRAM4[64*1024/8];

#elif defined ( __CC_ARM )  /* ʹ�õ�MDK */
/* D1��, AXI SRAM, 512KB */
mem_head_t *AXISRAMUsed;
uint64_t AppMallocAXISRAM[512*1024/8]__attribute__((at(0x24000000)));

/* D2��, 128KB SRAM1(0x30000000) + 128KB SRAM2(0x30020000) + 32KB SRAM3(0x30040000)  */
mem_head_t *SRAM1Used; 
uint64_t AppMallocSRAM1[288*1024/8]__attribute__((at(0x30000000)));

/* D3��, SRAM4, 64KB */
mem_head_t *SRAM4Used;
uint64_t AppMallocSRAM4[64*1024/8]__attribute__((at(0x38000000)));
#endif

/*
*********************************************************************************************************
*	�� �� ��: main
*	����˵��: c�������
*	��    ��: ��
*	�� �� ֵ: �������(���账��)
*********************************************************************************************************
*/
int main(void)
{
	uint8_t ucKeyCode;		/* �������� */
	uint32_t *DTCM_Addres0, *AXISRAM_Addres0, *SRAM1_Addres0, *SRAM4_Addres0;
	uint16_t *DTCM_Addres1, *AXISRAM_Addres1, *SRAM1_Addres1, *SRAM4_Addres1;
	uint8_t  *DTCM_Addres2, *AXISRAM_Addres2, *SRAM1_Addres2, *SRAM4_Addres2;
    
    
	bsp_Init();		/* Ӳ����ʼ�� */
	
	/* ��ʼ����̬�ڴ�ռ� */
	osRtxMemoryInit(AppMallocDTCM,    sizeof(AppMallocDTCM));
	osRtxMemoryInit(AppMallocAXISRAM, sizeof(AppMallocAXISRAM));
	osRtxMemoryInit(AppMallocSRAM1,   sizeof(AppMallocSRAM1));
	osRtxMemoryInit(AppMallocSRAM4,   sizeof(AppMallocSRAM4));
	
	PrintfLogo();	/* ��ӡ�������ƺͰ汾����Ϣ */
	PrintfHelp();	/* ��ӡ������ʾ */

	bsp_StartAutoTimer(0, 100);	/* ����1��100ms���Զ���װ�Ķ�ʱ�� */
	

	/* ����������ѭ���� */
	while (1)
	{
		bsp_Idle();		/* ���������bsp.c�ļ����û������޸��������ʵ��CPU���ߺ�ι�� */

		/* �ж϶�ʱ����ʱʱ�� */
		if (bsp_CheckTimer(0))	
		{
			/* ÿ��100ms ����һ�� */  
			bsp_LedToggle(2);
		}

		/* �����˲��ͼ���ɺ�̨systick�жϷ������ʵ�֣�����ֻ��Ҫ����bsp_GetKey��ȡ��ֵ���ɡ� */
		ucKeyCode = bsp_GetKey();	/* ��ȡ��ֵ, �޼�����ʱ���� KEY_NONE = 0 */
		if (ucKeyCode != KEY_NONE)
		{
			switch (ucKeyCode)
			{
                /* ��DTCM��������280�ֽڣ�64�ֽں�6111�ֽ� */
				case KEY_DOWN_K1:	
                    /* ��DTCM����280�ֽڿռ䣬ʹ��ָ�����DTCM_Addres0������Щ�ռ�ʱ��Ҫ����280�ֽڴ�С */	
					printf("=========================================================\r\n");
					DTCM_Addres0 = osRtxMemoryAlloc(AppMallocDTCM, 280, 0);
					DTCMUsed = MemHeadPtr(AppMallocDTCM);
					printf("DTCM�ܴ�С = %d�ֽڣ������С = 0280�ֽڣ���ǰ��ʹ�ô�С = %d�ֽ�\r\n", 
				                                                DTCMUsed->size, DTCMUsed->used);
				
					/* ��DTCM����64�ֽڿռ䣬ʹ��ָ�����DTCM_Addres1������Щ�ռ�ʱ��Ҫ����64�ֽڴ�С */	
					DTCM_Addres1 = osRtxMemoryAlloc(AppMallocDTCM, 64, 0);
					DTCMUsed = MemHeadPtr(AppMallocDTCM);
					printf("DTCM�ܴ�С = %d�ֽڣ������С = 0064�ֽڣ���ǰ��ʹ�ô�С = %d�ֽ�\r\n", 
											                   DTCMUsed->size, DTCMUsed->used);
				
					/* ��DTCM����6111�ֽڿռ䣬ʹ��ָ�����DTCM_Addres2������Щ�ռ�ʱ��Ҫ����6111�ֽڴ�С */	
					DTCM_Addres2 = osRtxMemoryAlloc(AppMallocDTCM, 6111, 0);
					DTCMUsed = MemHeadPtr(AppMallocDTCM);
					printf("DTCM�ܴ�С = %d�ֽڣ������С = 6111�ֽڣ���ǰ��ʹ�ô�С = %d�ֽ�\r\n", 
				                                                DTCMUsed->size, DTCMUsed->used);
					break;
				
				/* �ͷŴ�DTCM����Ŀռ� */
				case KEY_UP_K1:	
					/* �ͷŴ�DTCM�����280�ֽڿռ� */
					osRtxMemoryFree(AppMallocDTCM, DTCM_Addres0);
					DTCMUsed = MemHeadPtr(AppMallocDTCM);
					printf("�ͷ�DTCM��̬�ڴ��������0280�ֽڣ���ǰ��ʹ�ô�С = %d�ֽ�\r\n", DTCMUsed->used);
				
					/* �ͷŴ�DTCM�����64�ֽڿռ� */
					osRtxMemoryFree(AppMallocDTCM, DTCM_Addres1);
					DTCMUsed = MemHeadPtr(AppMallocDTCM);
					printf("�ͷ�DTCM��̬�ڴ��������0064�ֽڣ���ǰ��ʹ�ô�С = %d�ֽ�\r\n", DTCMUsed->used);
				
					/* �ͷŴ�DTCM�����6111�ֽڿռ� */
					osRtxMemoryFree(AppMallocDTCM, DTCM_Addres2);
					DTCMUsed = MemHeadPtr(AppMallocDTCM);
					printf("�ͷ�DTCM��̬�ڴ��������6111�ֽڣ���ǰ��ʹ�ô�С = %d�ֽ�\r\n", DTCMUsed->used);
					break;
				
				/* ��AXI SRAM��������160�ֽڣ�32�ֽں�2333�ֽ� */
				case KEY_DOWN_K2:	
                    /* ��AXI SRAM ����160�ֽڿռ䣬ʹ��ָ�����AXISRAM_Addres0������Щ�ռ�ʱ��Ҫ����160�ֽڴ�С */	
					printf("=========================================================\r\n");				
					AXISRAM_Addres0 = osRtxMemoryAlloc(AppMallocAXISRAM, 160, 0);
					AXISRAMUsed = MemHeadPtr(AppMallocAXISRAM);
					printf("AXI SRAM�ܴ�С = %d�ֽڣ������С = 0162�ֽڣ���ǰ��ʹ�ô�С = %d�ֽ�\r\n", 
				                                                AXISRAMUsed->size, AXISRAMUsed->used);
				
					/* ��AXI SRAM ����32�ֽڿռ䣬ʹ��ָ�����AXISRAM_Addres1������Щ�ռ�ʱ��Ҫ����32�ֽڴ�С */	
					AXISRAM_Addres1 = osRtxMemoryAlloc(AppMallocAXISRAM, 32, 0);
					AXISRAMUsed = MemHeadPtr(AppMallocAXISRAM);
					printf("AXI SRAM�ܴ�С = %d�ֽڣ������С = 0032�ֽڣ���ǰ��ʹ�ô�С = %d�ֽ�\r\n", 
											                   AXISRAMUsed->size, AXISRAMUsed->used);
				
					/* ��AXI SRAM ����2333�ֽڿռ䣬ʹ��ָ�����AXISRAM_Addres2������Щ�ռ�ʱ��Ҫ����2333�ֽڴ�С */	
					AXISRAM_Addres2 = osRtxMemoryAlloc(AppMallocAXISRAM, 2333, 0);
					AXISRAMUsed = MemHeadPtr(AppMallocAXISRAM);
					printf("AXI SRAM�ܴ�С = %d�ֽڣ������С = 2333�ֽڣ���ǰ��ʹ�ô�С = %d�ֽ�\r\n", 
				                                                AXISRAMUsed->size, AXISRAMUsed->used);
					break;
				
				/* �ͷŴ�AXI SRAM����Ŀռ� */
				case KEY_UP_K2:	
					/* �ͷŴ�AXI SRAM�����160�ֽڿռ� */
					osRtxMemoryFree(AppMallocAXISRAM, AXISRAM_Addres0);
					AXISRAMUsed = MemHeadPtr(AppMallocAXISRAM);
					printf("�ͷ�AXI SRAM��̬�ڴ��������0160�ֽڣ���ǰ��ʹ�ô�С = %d�ֽ�\r\n", AXISRAMUsed->used);
				
					/* �ͷŴ�AXI SRAM�����32�ֽڿռ� */
					osRtxMemoryFree(AppMallocAXISRAM, AXISRAM_Addres1);
					AXISRAMUsed = MemHeadPtr(AppMallocAXISRAM);
					printf("�ͷ�AXI SRAM��̬�ڴ��������0032�ֽڣ���ǰ��ʹ�ô�С = %d�ֽ�\r\n", AXISRAMUsed->used);
				
					/* �ͷŴ�AXI SRAM�����2333�ֽڿռ� */
					osRtxMemoryFree(AppMallocAXISRAM, AXISRAM_Addres2);
					AXISRAMUsed = MemHeadPtr(AppMallocAXISRAM);
					printf("�ͷ�AXI SRAM��̬�ڴ��������2333�ֽڣ���ǰ��ʹ�ô�С = %d�ֽ�\r\n", AXISRAMUsed->used);
					break;
				
				/* ��D2��SRAM��������200�ֽڣ�96�ֽں�4111�ֽ� */
				case KEY_DOWN_K3:	
                    /* ��D2���SRAM����200�ֽڿռ䣬ʹ��ָ�����SRAM1_Addres0������Щ�ռ�ʱ��Ҫ����200�ֽڴ�С */	
					printf("=========================================================\r\n");				
					SRAM1_Addres0 = osRtxMemoryAlloc(AppMallocSRAM1, 200, 0);
					SRAM1Used = MemHeadPtr(AppMallocSRAM1);
					printf("D2��SRAM�ܴ�С = %d�ֽڣ������С = 0200�ֽڣ���ǰ��ʹ�ô�С = %d�ֽ�\r\n", 
				                                                SRAM1Used->size, SRAM1Used->used);
				
					/* ��D2���SRAM����96�ֽڿռ䣬ʹ��ָ�����SRAM1_Addres1������Щ�ռ�ʱ��Ҫ����96�ֽڴ�С */	
					SRAM1_Addres1 = osRtxMemoryAlloc(AppMallocSRAM1, 96, 0);
					SRAM1Used = MemHeadPtr(AppMallocSRAM1);
					printf("D2��SRAM�ܴ�С = %d�ֽڣ������С = 0096�ֽڣ���ǰ��ʹ�ô�С = %d�ֽ�\r\n", 
											                   SRAM1Used->size, SRAM1Used->used);
				
					/* ��D2���SRAM����4111�ֽڿռ䣬ʹ��ָ�����SRAM1_Addres2������Щ�ռ�ʱ��Ҫ����4111�ֽڴ�С */	
					SRAM1_Addres2 = osRtxMemoryAlloc(AppMallocSRAM1, 4111, 0);
					SRAM1Used = MemHeadPtr(AppMallocSRAM1);
					printf("D2��SRAM�ܴ�С = %d�ֽڣ������С = 4111�ֽڣ���ǰ��ʹ�ô�С = %d�ֽ�\r\n", 
				                                                SRAM1Used->size, SRAM1Used->used);
					break;
				
				/* �ͷŴ�D2��SRAM����Ŀռ� */
				case KEY_UP_K3:	
					/* �ͷŴ�D2���SRAM�����200�ֽڿռ� */
					osRtxMemoryFree(AppMallocSRAM1, SRAM1_Addres0);
					SRAM1Used = MemHeadPtr(AppMallocSRAM1);
					printf("�ͷ�D2��SRAM��̬�ڴ��������0200�ֽڣ���ǰ��ʹ�ô�С = %d�ֽ�\r\n", SRAM1Used->used);
				
					/* �ͷŴ�D2���SRAM�����96�ֽڿռ� */
					osRtxMemoryFree(AppMallocSRAM1, SRAM1_Addres1);
					SRAM1Used = MemHeadPtr(AppMallocSRAM1);
					printf("�ͷ�D2��SRAM��̬�ڴ��������0096�ֽڣ���ǰ��ʹ�ô�С = %d�ֽ�\r\n", SRAM1Used->used);
				
					/* �ͷŴ�D2���SRAM�����4111�ֽڿռ� */
					osRtxMemoryFree(AppMallocSRAM1, SRAM1_Addres2);
					SRAM1Used = MemHeadPtr(AppMallocSRAM1);
					printf("�ͷ�D2��SRAM��̬�ڴ��������4111�ֽڣ���ǰ��ʹ�ô�С = %d�ֽ�\r\n", SRAM1Used->used);
					break;
				
				/* ��D3��SRAM��������300�ֽڣ�128�ֽں�5111�ֽ� */
				case JOY_DOWN_OK:	
                    /* ��D3���SRAM����300�ֽڿռ䣬ʹ��ָ�����SRAM4_Addres0������Щ�ռ�ʱ��Ҫ����300�ֽڴ�С */	
					printf("=========================================================\r\n");				
					SRAM4_Addres0 = osRtxMemoryAlloc(AppMallocSRAM4, 300, 0);
					SRAM4Used = MemHeadPtr(AppMallocSRAM4);
					printf("D3��SRAM�ܴ�С = %d�ֽڣ������С = 0300�ֽڣ���ǰ��ʹ�ô�С = %d�ֽ�\r\n", 
				                                                SRAM4Used->size, SRAM4Used->used);
				
					/* ��D3���SRAM����96�ֽڿռ䣬ʹ��ָ�����SRAM4_Addres1������Щ�ռ�ʱ��Ҫ����96�ֽڴ�С */	
					SRAM4_Addres1 = osRtxMemoryAlloc(AppMallocSRAM4, 128, 0);
					SRAM4Used = MemHeadPtr(AppMallocSRAM4);
					printf("D3��SRAM�ܴ�С = %d�ֽڣ������С = 0128�ֽڣ���ǰ��ʹ�ô�С = %d�ֽ�\r\n", 
											                   SRAM4Used->size, SRAM4Used->used);
				
					/* ��D3���SRAM����5111�ֽڿռ䣬ʹ��ָ�����SRAM4_Addres2������Щ�ռ�ʱ��Ҫ����5111�ֽڴ�С */	
					SRAM4_Addres2 = osRtxMemoryAlloc(AppMallocSRAM4, 5111, 0);
					SRAM4Used = MemHeadPtr(AppMallocSRAM4);
					printf("D3��SRAM�ܴ�С = %d�ֽڣ������С = 5111�ֽڣ���ǰ��ʹ�ô�С = %d�ֽ�\r\n", 
				                                                SRAM4Used->size, SRAM4Used->used);
					break;
				
				/* �ͷŴ�D3��SRAM����Ŀռ� */
				case JOY_UP_OK:	
					/* �ͷŴ�D3���SRAM�����300�ֽڿռ� */
					osRtxMemoryFree(AppMallocSRAM4, SRAM4_Addres0);
					SRAM4Used = MemHeadPtr(AppMallocSRAM4);
					printf("�ͷ�D3��SRAM��̬�ڴ��������0300�ֽڣ���ǰ��ʹ�ô�С = %d�ֽ�\r\n", SRAM4Used->used);
				
					/* �ͷŴ�D3���SRAM�����128�ֽڿռ� */
					osRtxMemoryFree(AppMallocSRAM4, SRAM4_Addres1);
					SRAM4Used = MemHeadPtr(AppMallocSRAM4);
					printf("�ͷ�D3��SRAM��̬�ڴ��������0128�ֽڣ���ǰ��ʹ�ô�С = %d�ֽ�\r\n", SRAM4Used->used);
				
					/* �ͷŴ�D3���SRAM�����5111�ֽڿռ� */
					osRtxMemoryFree(AppMallocSRAM4, SRAM4_Addres2);
					SRAM4Used = MemHeadPtr(AppMallocSRAM4);
					printf("�ͷ�D3��SRAM��̬�ڴ��������5111�ֽڣ���ǰ��ʹ�ô�С = %d�ֽ�\r\n", SRAM4Used->used);
					break;
			
				default:
				  /* �����ļ�ֵ������ */
				  break;
			}
		}
	}
}

/*
*********************************************************************************************************
*	�� �� ��: PrintfHelp
*	����˵��: ��ӡ������ʾ
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void PrintfHelp(void)
{
	printf("������ʾ:\r\n");
	printf("1. K1�����£���DTCM��������280�ֽڣ�64�ֽں�6111�ֽ�\r\n");
	printf("2. K1���ɿ����ͷŴ�DTCM����Ŀռ�\r\n");
	printf("3. K2�����£���AXI SRAM��������160�ֽڣ�32�ֽں�2333�ֽ�\r\n");
	printf("4. K2���ɿ����ͷŴ�AXI SRAM����Ŀռ�\r\n");
	printf("5. K3�����£���D2��SRAM��������200�ֽڣ�96�ֽں�4111�ֽ�\r\n");
	printf("6. K3���ɿ����ͷŴ�D2��SRAM����Ŀռ�\r\n");
	printf("7. ҡ��OK�����£���D3��SRAM��������300�ֽڣ�128�ֽں�5111�ֽ�\r\n");
	printf("8. ҡ��OK���ɿ����ͷŴ�D3��SRAM����Ŀռ�\r\n");
}

/*
*********************************************************************************************************
*	�� �� ��: PrintfLogo
*	����˵��: ��ӡ�������ƺ����̷�������, ���ϴ����ߺ󣬴�PC���ĳ����ն�������Թ۲���
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void PrintfLogo(void)
{
	printf("*************************************************************\n\r");
	
	/* ���CPU ID */
	{
		uint32_t CPU_Sn0, CPU_Sn1, CPU_Sn2;
		
		CPU_Sn0 = *(__IO uint32_t*)(0x1FF1E800);
		CPU_Sn1 = *(__IO uint32_t*)(0x1FF1E800 + 4);
		CPU_Sn2 = *(__IO uint32_t*)(0x1FF1E800 + 8);

		printf("\r\nCPU : STM32H743XIH6, BGA240, ��Ƶ: %dMHz\r\n", SystemCoreClock / 1000000);
		printf("UID = %08X %08X %08X\n\r", CPU_Sn2, CPU_Sn1, CPU_Sn0);
	}

	printf("\n\r");
	printf("*************************************************************\n\r");
	printf("* ��������   : %s\r\n", EXAMPLE_NAME);	/* ��ӡ�������� */
	printf("* ���̰汾   : %s\r\n", DEMO_VER);		/* ��ӡ���̰汾 */
	printf("* ��������   : %s\r\n", EXAMPLE_DATE);	/* ��ӡ�������� */

	/* ��ӡST��HAL��汾 */
	printf("* HAL��汾  : V1.3.0 (STM32H7xx HAL Driver)\r\n");
	printf("* \r\n");	/* ��ӡһ�пո� */
	printf("* QQ    : 1295744630 \r\n");
	printf("* ����  : armfly\r\n");
	printf("* Email : armfly@qq.com \r\n");
	printf("* ΢�Ź��ں�: armfly_com \r\n");
	printf("* �Ա���: armfly.taobao.com\r\n");
	printf("* Copyright www.armfly.com ����������\r\n");
	printf("*************************************************************\n\r");
}

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
