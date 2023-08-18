/*
*********************************************************************************************************
*
*	ģ������ : ������ģ��
*	�ļ����� : main.c
*	��    �� : V1.0
*	˵    �� : �͹��Ĵ���LPUART��ͣ�����ѡ�
*              ʵ��Ŀ�ģ�
*                1��ѧϰ�͹��Ĵ��ڵ�ͣ�����ѡ�
*              ʵ�����ݣ�
*                1����ǰ����ʹ�õĴ��ڴ�ӡ�����õĵ͹��Ĵ��ڣ���USART1��LPUART1������ʹ��PA9��PA10��
*                2���ϵ�������һ�������ʱ����ÿ100ms��תһ��LED2��
*                3��USART1��LPUART������ʹ��PA9��PA10���������ڴ�ӡ���ܣ����������õ�LPUART�������崮�ڴ�ӡ��
*                4��LPUART����ѡ��HSIʱ�ӣ�LSEʱ�Ӻ�D3PCLK1ʱ�ӣ���bsp_lpuart_fifo.c�ļ���ͷ�������á�
*                   �����Ҫ�͹���ģʽ���ѣ�����ʹ��LSE����HSIʱ�ӣ���������bsp_lpuart_fifo.h���壬���������õ�HSIʱ�ӡ�
*                   LPUARTʱ��ѡ��LSE(32768Hz)������ٶ���10922bps�����8bps��
*                   LPUARTʱ��ѡ��HSI(64MHz)�����ֵ��21MHz����Сֵ15625bps��
*                   LPUARTʱ��ѡ��D3PCLK1(100MHz)�����ֵ33Mbps����Сֵ24414bps��
*              ʵ�������
*                1��K1�����£�����ͣ��ģʽ���͹��Ĵ��ڽ��������ֽ����ݿ��Ի��ѡ�
*                2��K2�����£�����ͣ��ģʽ���͹��Ĵ��ڼ�⵽��ʼλ���Ի��ѡ�
*                3��K3�����£�����ͣ��ģʽ���͹��Ĵ��ڼ�⵽��ַ0x99���Ի��ѡ�
*              ע�����
*                1. ��ʵ���Ƽ�ʹ�ô������SecureCRT�鿴��ӡ��Ϣ��������115200������λ8����żУ��λ�ޣ�ֹͣλ1��
*                2. ��ؽ��༭��������������TAB����Ϊ4���Ķ����ļ���Ҫ��������ʾ�����롣
*
*	�޸ļ�¼ :
*		�汾��   ����         ����        ˵��
*		V1.0    2020-02-11   Eric2013     1. CMSIS����汾 V5.6.0
*                                         2. HAL��汾 V1.7.0
*
*	Copyright (C), 2020-2030, ���������� www.armfly.com
*
*********************************************************************************************************
*/	
#include "bsp.h"			 /* �ײ�Ӳ������ */



/* ���������������̷������� */
#define EXAMPLE_NAME	"V7-�͹��Ĵ��ڵ�ͣ�����ѣ�����FIFO��ʽ��"
#define EXAMPLE_DATE	"2020-02-11"
#define DEMO_VER		"1.0"

static void PrintfLogo(void);
static void PrintfHelp(void);
UART_WakeUpTypeDef WakeUpSelection = {0};

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
	uint8_t ucKeyCode;	/* �������� */
	uint8_t ucReceive;
	

	bsp_Init();		/* Ӳ����ʼ�� */
	PrintfLogo();	/* ��ӡ�������ƺͰ汾����Ϣ */
	PrintfHelp();	/* ��ӡ������ʾ */
	
	HAL_EnableDBGStopMode(); /* ʹ��ͣ��ģʽ�£�LPUART���̿��Լ������� */
	__HAL_RCC_WAKEUPSTOP_CLK_CONFIG(RCC_STOP_WAKEUPCLOCK_HSI); /* ��ͣ��ģʽ���Ѻ�ʹ��HSIʱ�� */
	__HAL_RCC_LPUART1_CLKAM_ENABLE();     /* ����LPUART������ģʽ����ͣ��״̬�¿��Լ���������Ϣ */
	__HAL_UART_ENABLE_IT(&UartHandle, UART_IT_WUF);/* ʹ�ܻ����ж� */
	
	
	bsp_StartAutoTimer(0, 100);	/* ����1��100ms���Զ���װ�Ķ�ʱ�� */
	
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
				case KEY_DOWN_K1:			/* K1�����£�����ͣ��ģʽ���͹��Ĵ��ڽ��������ֽ����ݿ��Ի��� */
					/* ʹ��LPUART��ͣ������ */
					HAL_UARTEx_EnableStopMode(&UartHandle); 

					/* ȷ��LPUARTû����ͨ���� */
					while(__HAL_UART_GET_FLAG(&UartHandle, USART_ISR_BUSY) == SET){}
					while(__HAL_UART_GET_FLAG(&UartHandle, USART_ISR_REACK) == RESET){}

					/* ���յ����ݻ��ѣ���RXNE��־��λ */
					WakeUpSelection.WakeUpEvent = UART_WAKEUP_ON_READDATA_NONEMPTY;
					if (HAL_UARTEx_StopModeWakeUpSourceConfig(&UartHandle, WakeUpSelection)!= HAL_OK)
					{
						Error_Handler(__FILE__, __LINE__);						
					}

					/* ����ͣ��ģʽ */
					HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);

					/* �˳�ͣ��ģʽҪ��������HSE��PLL*/
					SystemClock_Config();

					/* �ر�LPUART��ͣ������ */
					HAL_UARTEx_DisableStopMode(&UartHandle);
					
					lpcomGetChar(LPCOM1, &ucReceive);
					
					printf("�͹��Ĵ��ڽ��յ����� %x ����\r\n", ucReceive);
					break;
					
				case KEY_DOWN_K2:			/* K2�����£�����ͣ��ģʽ���͹��Ĵ��ڼ�⵽��ʼλ���Ի��� */
					/* ʹ��LPUART��ͣ������ */
					HAL_UARTEx_EnableStopMode(&UartHandle); 

					/* ȷ��LPUARTû����ͨ���� */
					while(__HAL_UART_GET_FLAG(&UartHandle, USART_ISR_BUSY) == SET){}
					while(__HAL_UART_GET_FLAG(&UartHandle, USART_ISR_REACK) == RESET){}

					/* ������ʼλ���� */
					WakeUpSelection.WakeUpEvent = UART_WAKEUP_ON_STARTBIT;
					if (HAL_UARTEx_StopModeWakeUpSourceConfig(&UartHandle, WakeUpSelection)!= HAL_OK)
					{
						Error_Handler(__FILE__, __LINE__);						
					}

					/* ����ͣ��ģʽ */
					HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);

					/* �˳�ͣ��ģʽҪ��������HSE��PLL*/
					SystemClock_Config();

					/* �ر�LPUART��ͣ������ */
					HAL_UARTEx_DisableStopMode(&UartHandle);
					
					lpcomGetChar(LPCOM1, &ucReceive);
					
					printf("�͹��Ĵ��ڼ�⵽��ʼλ�����ݣ� %x ����\r\n", ucReceive);
					break;
					
				case KEY_DOWN_K3:			/* K3�����£�����ͣ��ģʽ���͹��Ĵ��ڼ�⵽��ַ0x99���Ի��� */
					/* ʹ��LPUART��ͣ������ */
					HAL_UARTEx_EnableStopMode(&UartHandle); 

					/* ȷ��LPUARTû����ͨ���� */
					while(__HAL_UART_GET_FLAG(&UartHandle, USART_ISR_BUSY) == SET){}
					while(__HAL_UART_GET_FLAG(&UartHandle, USART_ISR_REACK) == RESET){}

					/* ���յ�ַ0x99�����͵�����MSBλҪΪ1�������Ի��� */
					WakeUpSelection.WakeUpEvent   = UART_WAKEUP_ON_ADDRESS;
					WakeUpSelection.AddressLength = UART_ADDRESS_DETECT_7B;
					WakeUpSelection.Address       = 0x19;
					if (HAL_UARTEx_StopModeWakeUpSourceConfig(&UartHandle, WakeUpSelection)!= HAL_OK)
					{
						Error_Handler(__FILE__, __LINE__);						
					}
					
					CLEAR_BIT(LPUART1->CR1, USART_CR1_RXNEIE); /* �رմ��ڽ����ж� */
					
					/* ����ͣ��ģʽ */
					HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);

					/* �˳�ͣ��ģʽҪ��������HSE��PLL*/
					SystemClock_Config();

					SET_BIT(LPUART1->CR1, USART_CR1_RXNEIE);  /* ʹ�ܴ��ڽ����ж� */
					
					/* �ر�LPUART��ͣ������ */
					HAL_UARTEx_DisableStopMode(&UartHandle);
					
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
	printf("1. �ϵ�������һ�������ʱ����ÿ100ms��תһ��LED2\r\n");
	printf("2. K1�����£�����ͣ��ģʽ���͹��Ĵ��ڽ��������ֽ����ݿ��Ի���\r\n");
	printf("3. K2�����£�����ͣ��ģʽ���͹��Ĵ��ڼ�⵽��ʼλ���Ի���\r\n");
	printf("4. K3�����£�����ͣ��ģʽ���͹��Ĵ��ڼ�⵽��ַ0x99���Ի���\r\n");
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
	printf("* HAL��汾  : V1.7.0 (STM32H7xx HAL Driver)\r\n");
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
