/*
*********************************************************************************************************
*
*	ģ������ : MODSBUSͨ�ų�������ģʽ��ԭ����
*	�ļ����� : modbus_host.c
*	��    �� : V1.4
*	˵    �� : MODBUSЭ��
*
*	Copyright (C), 2020-2030, ���������� www.armfly.com
*
*********************************************************************************************************
*/
#include "bsp.h"
#include "modbus_host.h"


/*
*********************************************************************************************************
*	                                   ����
*********************************************************************************************************
*/
#define TIMEOUT		1000		/* �������ʱʱ��, ��λms */
#define NUM			1			/* ѭ�����ʹ��� */

/*
Baud rate	Bit rate	 Bit time	 Character time	  3.5 character times
  2400	    2400 bits/s	  417 us	      4.6 ms	      16 ms
  4800	    4800 bits/s	  208 us	      2.3 ms	      8.0 ms
  9600	    9600 bits/s	  104 us	      1.2 ms	      4.0 ms
 19200	   19200 bits/s    52 us	      573 us	      2.0 ms
 38400	   38400 bits/s	   26 us	      286 us	      1.75 ms(1.0 ms)
 115200	   115200 bit/s	  8.7 us	       95 us	      1.75 ms(0.33 ms) ����̶���Ϊ1750us
*/
typedef struct
{
	uint32_t Bps;
	uint32_t usTimeOut;
}MODBUSBPS_T;

const MODBUSBPS_T ModbusBaudRate[] =
{	
    {2400,	16000}, /* ������2400bps, 3.5�ַ��ӳ�ʱ��16000us */
	{4800,	 8000}, 
	{9600,	 4000},
	{19200,	 2000},
	{38400,	 1750},
	{115200, 1750},
	{128000, 1750},
	{230400, 1750},
};

MODH_T g_tModH = {0};
uint8_t g_modh_timeout = 0;
VAR_T g_tVar;

/*
*********************************************************************************************************
*	                                   ��������
*********************************************************************************************************
*/
static void MODH_RxTimeOut(void);
static void MODH_AnalyzeApp(void);

static void MODH_Read_01H(void);
static void MODH_Read_02H(void);
static void MODH_Read_03H(void);
static void MODH_Read_04H(void);
static void MODH_Read_05H(void);
static void MODH_Read_06H(void);
static void MODH_Read_10H(void);


/*
*********************************************************************************************************
*	�� �� ��: MODH_SendPacket
*	����˵��: �������ݰ� COM1��
*	��    ��: _buf : ���ݻ�����
*			  _len : ���ݳ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void MODH_SendPacket(uint8_t *_buf, uint16_t _len)
{
	RS485_SendBuf(_buf, _len);
}

/*
*********************************************************************************************************
*	�� �� ��: MODH_SendAckWithCRC
*	����˵��: ����Ӧ��,�Զ���CRC.  
*	��    ��: �ޡ�
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void MODH_SendAckWithCRC(void)
{
	uint16_t crc;
	
	crc = CRC16_Modbus(g_tModH.TxBuf, g_tModH.TxCount);
	g_tModH.TxBuf[g_tModH.TxCount++] = crc >> 8;
	g_tModH.TxBuf[g_tModH.TxCount++] = crc;	
	MODH_SendPacket(g_tModH.TxBuf, g_tModH.TxCount);
}

/*
*********************************************************************************************************
*	�� �� ��: MODH_AnalyzeApp
*	����˵��: ����Ӧ�ò�Э�顣����Ӧ��
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void MODH_AnalyzeApp(void)
{	
	switch (g_tModH.RxBuf[1])			/* ��2���ֽ� ������ */
	{
		case 0x01:	/* ��ȡ��Ȧ״̬ */
			MODH_Read_01H();
			break;

		case 0x02:	/* ��ȡ����״̬ */
			MODH_Read_02H();
			break;

		case 0x03:	/* ��ȡ���ּĴ��� ��һ���������ּĴ�����ȡ�õ�ǰ�Ķ�����ֵ */
			MODH_Read_03H();
			break;

		case 0x04:	/* ��ȡ����Ĵ��� */
			MODH_Read_04H();
			break;

		case 0x05:	/* ǿ�Ƶ���Ȧ */
			MODH_Read_05H();
			break;

		case 0x06:	/* д�����Ĵ��� */
			MODH_Read_06H();
			break;		

		case 0x10:	/* д����Ĵ��� */
			MODH_Read_10H();
			break;
		
		default:
			break;
	}
}

/*
*********************************************************************************************************
*	�� �� ��: MODH_Send01H
*	����˵��: ����01Hָ���ѯ1��������Ȧ�Ĵ���
*	��    ��: _addr : ��վ��ַ
*			  _reg : �Ĵ������
*			  _num : �Ĵ�������
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void MODH_Send01H(uint8_t _addr, uint16_t _reg, uint16_t _num)
{
	g_tModH.TxCount = 0;
	g_tModH.TxBuf[g_tModH.TxCount++] = _addr;		/* ��վ��ַ */
	g_tModH.TxBuf[g_tModH.TxCount++] = 0x01;		/* ������ */	
	g_tModH.TxBuf[g_tModH.TxCount++] = _reg >> 8;	/* �Ĵ������ ���ֽ� */
	g_tModH.TxBuf[g_tModH.TxCount++] = _reg;		/* �Ĵ������ ���ֽ� */
	g_tModH.TxBuf[g_tModH.TxCount++] = _num >> 8;	/* �Ĵ������� ���ֽ� */
	g_tModH.TxBuf[g_tModH.TxCount++] = _num;		/* �Ĵ������� ���ֽ� */
	
	MODH_SendAckWithCRC();		/* �������ݣ��Զ���CRC */
	g_tModH.fAck01H = 0;		/* ����ձ�־ */
	g_tModH.RegNum = _num;		/* �Ĵ������� */
	g_tModH.Reg01H = _reg;		/* ����01Hָ���еļĴ�����ַ�������Ӧ�����ݽ��з��� */	
}

/*
*********************************************************************************************************
*	�� �� ��: MODH_Send02H
*	����˵��: ����02Hָ�����ɢ����Ĵ���
*	��    ��: _addr : ��վ��ַ
*			  _reg : �Ĵ������
*			  _num : �Ĵ�������
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void MODH_Send02H(uint8_t _addr, uint16_t _reg, uint16_t _num)
{
	g_tModH.TxCount = 0;
	g_tModH.TxBuf[g_tModH.TxCount++] = _addr;		/* ��վ��ַ */
	g_tModH.TxBuf[g_tModH.TxCount++] = 0x02;		/* ������ */	
	g_tModH.TxBuf[g_tModH.TxCount++] = _reg >> 8;	/* �Ĵ������ ���ֽ� */
	g_tModH.TxBuf[g_tModH.TxCount++] = _reg;		/* �Ĵ������ ���ֽ� */
	g_tModH.TxBuf[g_tModH.TxCount++] = _num >> 8;	/* �Ĵ������� ���ֽ� */
	g_tModH.TxBuf[g_tModH.TxCount++] = _num;		/* �Ĵ������� ���ֽ� */
	
	MODH_SendAckWithCRC();		/* �������ݣ��Զ���CRC */
	g_tModH.fAck02H = 0;		/* ����ձ�־ */
	g_tModH.RegNum = _num;		/* �Ĵ������� */
	g_tModH.Reg02H = _reg;		/* ����02Hָ���еļĴ�����ַ�������Ӧ�����ݽ��з��� */	
}

/*
*********************************************************************************************************
*	�� �� ��: MODH_Send03H
*	����˵��: ����03Hָ���ѯ1���������ּĴ���
*	��    ��: _addr : ��վ��ַ
*			  _reg : �Ĵ������
*			  _num : �Ĵ�������
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void MODH_Send03H(uint8_t _addr, uint16_t _reg, uint16_t _num)
{
	g_tModH.TxCount = 0;
	g_tModH.TxBuf[g_tModH.TxCount++] = _addr;		/* ��վ��ַ */
	g_tModH.TxBuf[g_tModH.TxCount++] = 0x03;		/* ������ */	
	g_tModH.TxBuf[g_tModH.TxCount++] = _reg >> 8;	/* �Ĵ������ ���ֽ� */
	g_tModH.TxBuf[g_tModH.TxCount++] = _reg;		/* �Ĵ������ ���ֽ� */
	g_tModH.TxBuf[g_tModH.TxCount++] = _num >> 8;	/* �Ĵ������� ���ֽ� */
	g_tModH.TxBuf[g_tModH.TxCount++] = _num;		/* �Ĵ������� ���ֽ� */
	
	MODH_SendAckWithCRC();		/* �������ݣ��Զ���CRC */
	g_tModH.fAck03H = 0;		/* ����ձ�־ */
	g_tModH.RegNum = _num;		/* �Ĵ������� */
	g_tModH.Reg03H = _reg;		/* ����03Hָ���еļĴ�����ַ�������Ӧ�����ݽ��з��� */	
}

/*
*********************************************************************************************************
*	�� �� ��: MODH_Send04H
*	����˵��: ����04Hָ�������Ĵ���
*	��    ��: _addr : ��վ��ַ
*			  _reg : �Ĵ������
*			  _num : �Ĵ�������
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void MODH_Send04H(uint8_t _addr, uint16_t _reg, uint16_t _num)
{
	g_tModH.TxCount = 0;
	g_tModH.TxBuf[g_tModH.TxCount++] = _addr;		/* ��վ��ַ */
	g_tModH.TxBuf[g_tModH.TxCount++] = 0x04;		/* ������ */	
	g_tModH.TxBuf[g_tModH.TxCount++] = _reg >> 8;	/* �Ĵ������ ���ֽ� */
	g_tModH.TxBuf[g_tModH.TxCount++] = _reg;		/* �Ĵ������ ���ֽ� */
	g_tModH.TxBuf[g_tModH.TxCount++] = _num >> 8;	/* �Ĵ������� ���ֽ� */
	g_tModH.TxBuf[g_tModH.TxCount++] = _num;		/* �Ĵ������� ���ֽ� */
	
	MODH_SendAckWithCRC();		/* �������ݣ��Զ���CRC */
	g_tModH.fAck04H = 0;		/* ����ձ�־ */
	g_tModH.RegNum = _num;		/* �Ĵ������� */
	g_tModH.Reg04H = _reg;		/* ����04Hָ���еļĴ�����ַ�������Ӧ�����ݽ��з��� */	
}

/*
*********************************************************************************************************
*	�� �� ��: MODH_Send05H
*	����˵��: ����05Hָ�дǿ�õ���Ȧ
*	��    ��: _addr : ��վ��ַ
*			  _reg : �Ĵ������
*			  _value : �Ĵ���ֵ,2�ֽ�
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void MODH_Send05H(uint8_t _addr, uint16_t _reg, uint16_t _value)
{
	g_tModH.TxCount = 0;
	g_tModH.TxBuf[g_tModH.TxCount++] = _addr;			/* ��վ��ַ */
	g_tModH.TxBuf[g_tModH.TxCount++] = 0x05;			/* ������ */	
	g_tModH.TxBuf[g_tModH.TxCount++] = _reg >> 8;		/* �Ĵ������ ���ֽ� */
	g_tModH.TxBuf[g_tModH.TxCount++] = _reg;			/* �Ĵ������ ���ֽ� */
	g_tModH.TxBuf[g_tModH.TxCount++] = _value >> 8;		/* �Ĵ���ֵ ���ֽ� */
	g_tModH.TxBuf[g_tModH.TxCount++] = _value;			/* �Ĵ���ֵ ���ֽ� */
	
	MODH_SendAckWithCRC();		/* �������ݣ��Զ���CRC */

	g_tModH.fAck05H = 0;		/* ����յ��ӻ���Ӧ���������־����Ϊ1 */
}

/*
*********************************************************************************************************
*	�� �� ��: MODH_Send06H
*	����˵��: ����06Hָ�д1�����ּĴ���
*	��    ��: _addr : ��վ��ַ
*			  _reg : �Ĵ������
*			  _value : �Ĵ���ֵ,2�ֽ�
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void MODH_Send06H(uint8_t _addr, uint16_t _reg, uint16_t _value)
{
	g_tModH.TxCount = 0;
	g_tModH.TxBuf[g_tModH.TxCount++] = _addr;			/* ��վ��ַ */
	g_tModH.TxBuf[g_tModH.TxCount++] = 0x06;			/* ������ */	
	g_tModH.TxBuf[g_tModH.TxCount++] = _reg >> 8;		/* �Ĵ������ ���ֽ� */
	g_tModH.TxBuf[g_tModH.TxCount++] = _reg;			/* �Ĵ������ ���ֽ� */
	g_tModH.TxBuf[g_tModH.TxCount++] = _value >> 8;		/* �Ĵ���ֵ ���ֽ� */
	g_tModH.TxBuf[g_tModH.TxCount++] = _value;			/* �Ĵ���ֵ ���ֽ� */
	
	MODH_SendAckWithCRC();		/* �������ݣ��Զ���CRC */
	
	g_tModH.fAck06H = 0;		/* ����յ��ӻ���Ӧ���������־����Ϊ1 */
}

/*
*********************************************************************************************************
*	�� �� ��: MODH_Send10H
*	����˵��: ����10Hָ�����д������ּĴ���. ���һ��֧��23���Ĵ�����
*	��    ��: _addr : ��վ��ַ
*			  _reg : �Ĵ������
*			  _num : �Ĵ�������n (ÿ���Ĵ���2���ֽ�) ֵ��
*			  _buf : n���Ĵ��������ݡ����� = 2 * n
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void MODH_Send10H(uint8_t _addr, uint16_t _reg, uint8_t _num, uint8_t *_buf)
{
	uint16_t i;
	
	g_tModH.TxCount = 0;
	g_tModH.TxBuf[g_tModH.TxCount++] = _addr;		/* ��վ��ַ */
	g_tModH.TxBuf[g_tModH.TxCount++] = 0x10;		/* ��վ��ַ */	
	g_tModH.TxBuf[g_tModH.TxCount++] = _reg >> 8;	/* �Ĵ������ ���ֽ� */
	g_tModH.TxBuf[g_tModH.TxCount++] = _reg;		/* �Ĵ������ ���ֽ� */
	g_tModH.TxBuf[g_tModH.TxCount++] = _num >> 8;	/* �Ĵ������� ���ֽ� */
	g_tModH.TxBuf[g_tModH.TxCount++] = _num;		/* �Ĵ������� ���ֽ� */
	g_tModH.TxBuf[g_tModH.TxCount++] = 2 * _num;	/* �����ֽ��� */
	
	for (i = 0; i < 2 * _num; i++)
	{
		if (g_tModH.TxCount > H_RX_BUF_SIZE - 3)
		{
			return;		/* ���ݳ������������ȣ�ֱ�Ӷ��������� */
		}
		g_tModH.TxBuf[g_tModH.TxCount++] = _buf[i];		/* ��������ݳ��� */
	}
	
	MODH_SendAckWithCRC();	/* �������ݣ��Զ���CRC */
}

/*
*********************************************************************************************************
*	�� �� ��: MODH_ReciveNew
*	����˵��: ���ڽ����жϷ���������ñ����������յ�һ���ֽ�ʱ��ִ��һ�α�������
*	��    ��: ��������
*	�� �� ֵ: 1 ��ʾ������
*********************************************************************************************************
*/
void MODH_ReciveNew(uint8_t _data)
{
	/*
		3.5���ַ���ʱ������ֻ������RTUģʽ���棬��ΪRTUģʽû�п�ʼ���ͽ�������
		�������ݰ�֮��ֻ�ܿ�ʱ���������֣�Modbus�����ڲ�ͬ�Ĳ������£����ʱ���ǲ�һ���ģ�
		���鿴��C�ļ���ͷ
	*/
	uint8_t i;
	
	/* ���ݲ����ʣ���ȡ��Ҫ�ӳٵ�ʱ�� */
	for(i = 0; i < (sizeof(ModbusBaudRate)/sizeof(ModbusBaudRate[0])); i++)
	{
		if(HBAUD485 == ModbusBaudRate[i].Bps)
		{
			break;
		}	
	}

	/* Ӳ����ʱ�жϣ�Ӳ����ʱ��1����MODBUS�ӻ�, ��ʱ��2����MODBUS����*/
	bsp_StartHardTimer(2,  ModbusBaudRate[i].usTimeOut, (void *)MODH_RxTimeOut);

	if (g_tModH.RxCount < H_RX_BUF_SIZE)
	{
		g_tModH.RxBuf[g_tModH.RxCount++] = _data;
	}
}

/*
*********************************************************************************************************
*	�� �� ��: MODH_RxTimeOut
*	����˵��: ����3.5���ַ�ʱ���ִ�б������� ����ȫ�ֱ��� g_rtu_timeout = 1; ֪ͨ������ʼ���롣
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void MODH_RxTimeOut(void)
{
	g_modh_timeout = 1;
}

/*
*********************************************************************************************************
*	�� �� ��: MODH_Poll
*	����˵��: ���տ�����ָ��. 1ms ��Ӧʱ�䡣
*	��    ��: ��
*	�� �� ֵ: 0 ��ʾ������ 1��ʾ�յ���ȷ����
*********************************************************************************************************
*/
void MODH_Poll(void)
{	
	uint16_t crc1;
	
	if (g_modh_timeout == 0)	/* ����3.5���ַ�ʱ���ִ��MODH_RxTimeOut()������ȫ�ֱ��� g_rtu_timeout = 1 */
	{
		/* û�г�ʱ���������ա���Ҫ���� g_tModH.RxCount */
		return ;
	}

	/* �յ�����
		05 06 00 88 04 57 3B70 (8 �ֽ�)
			05    :  ��������ĺ�վ��
			06    :  ָ��
			00 88 :  �����������ʾ�Ĵ���
			04 57 :  ����,,,ת���� 10 ������ 1111.��λ��ǰ,
			3B70  :  �����ֽ� CRC ��	��05�� 57��У��
	*/
	g_modh_timeout = 0;

	/* ���յ�������С��4���ֽھ���Ϊ���󣬵�ַ��8bit��+ָ�8bit��+�����Ĵ�����16bit�� */
	if (g_tModH.RxCount < 4)
	{
		goto err_ret;
	}

	/* ����CRCУ��ͣ������ǽ����յ������ݰ���CRC16ֵһ����CRC16�������0����ʾ��ȷ���� */
	crc1 = CRC16_Modbus(g_tModH.RxBuf, g_tModH.RxCount);
	if (crc1 != 0)
	{
		goto err_ret;
	}
	
	/* ����Ӧ�ò�Э�� */
	MODH_AnalyzeApp();

err_ret:
	g_tModH.RxCount = 0;	/* ��������������������´�֡ͬ�� */
}

/*
*********************************************************************************************************
*	�� �� ��: MODH_Read_01H
*	����˵��: ����01Hָ���Ӧ�����ݣ���ȡ��Ȧ״̬��bit����
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void MODH_Read_01H(void)
{
	uint8_t bytes;
	uint8_t *p;
	
	if (g_tModH.RxCount > 0)
	{
		bytes = g_tModH.RxBuf[2];	/* ���ݳ��� �ֽ��� */				
		switch (g_tModH.Reg01H)
		{
			case REG_D01:
				if (bytes == 1)
				{
					p = &g_tModH.RxBuf[3];	
					
					g_tVar.D01 = BEBufToUint16(p); p += 2;	/* �Ĵ��� */	
					g_tVar.D02 = BEBufToUint16(p); p += 2;	/* �Ĵ��� */	
					g_tVar.D03 = BEBufToUint16(p); p += 2;	/* �Ĵ��� */	
					g_tVar.D04 = BEBufToUint16(p); p += 2;	/* �Ĵ��� */
					
					g_tModH.fAck01H = 1;
				}
				break;
		}
	}
}

/*
*********************************************************************************************************
*	�� �� ��: MODH_Read_02H
*	����˵��: ����02Hָ���Ӧ�����ݣ���ȡ����״̬��bit����
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void MODH_Read_02H(void)
{
	uint8_t bytes;
	uint8_t *p;
	
	if (g_tModH.RxCount > 0)
	{
		bytes = g_tModH.RxBuf[2];	/* ���ݳ��� �ֽ��� */				
		switch (g_tModH.Reg02H)
		{
			case REG_T01:
				if (bytes == 6)
				{
					p = &g_tModH.RxBuf[3];	
					
					g_tVar.T01 = BEBufToUint16(p); p += 2;	/* �Ĵ��� */	
					g_tVar.T02 = BEBufToUint16(p); p += 2;	/* �Ĵ��� */	
					g_tVar.T03 = BEBufToUint16(p); p += 2;	/* �Ĵ��� */	
					
					g_tModH.fAck02H = 1;
				}
				break;
		}
	}
}

/*
*********************************************************************************************************
*	�� �� ��: MODH_Read_04H
*	����˵��: ����04Hָ���Ӧ�����ݣ���ȡ����Ĵ�����16bit����
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void MODH_Read_04H(void)
{
	uint8_t bytes;
	uint8_t *p;
	
	if (g_tModH.RxCount > 0)
	{
		bytes = g_tModH.RxBuf[2];	/* ���ݳ��� �ֽ��� */				
		switch (g_tModH.Reg04H)
		{
			case REG_A01:
				if (bytes == 2)
				{
					p = &g_tModH.RxBuf[3];	
					
					g_tVar.A01 = BEBufToUint16(p); p += 2;	/* �Ĵ��� */	
					
					g_tModH.fAck04H = 1;
				}
				break;
		}
	}
}

/*
*********************************************************************************************************
*	�� �� ��: MODH_Read_05H
*	����˵��: ����05Hָ���Ӧ�����ݣ�д����Ȧ״̬��bit����
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void MODH_Read_05H(void)
{
	if (g_tModH.RxCount > 0)
	{
		if (g_tModH.RxBuf[0] == SlaveAddr)		
		{
			g_tModH.fAck05H = 1;		/* ���յ�Ӧ�� */
		}
	};
}

/*
*********************************************************************************************************
*	�� �� ��: MODH_Read_06H
*	����˵��: ����06Hָ���Ӧ�����ݣ�д��������Ĵ�����16bit����
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void MODH_Read_06H(void)
{
	if (g_tModH.RxCount > 0)
	{
		if (g_tModH.RxBuf[0] == SlaveAddr)		
		{
			g_tModH.fAck06H = 1;		/* ���յ�Ӧ�� */
		}
	}
}


/*
*********************************************************************************************************
*	�� �� ��: MODH_Read_03H
*	����˵��: ����03Hָ���Ӧ�����ݣ���ȡ���ּĴ�����16bit����
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void MODH_Read_03H(void)
{
	uint8_t bytes;
	uint8_t *p;
	
	if (g_tModH.RxCount > 0)
	{
		bytes = g_tModH.RxBuf[2];	/* ���ݳ��� �ֽ��� */				
		switch (g_tModH.Reg03H)
		{
			case REG_P01:
				if (bytes == 4)
				{
					p = &g_tModH.RxBuf[3];	
					
					g_tVar.P01 = BEBufToUint16(p); p += 2;	/* �Ĵ��� */	
					g_tVar.P02 = BEBufToUint16(p); p += 2;	/* �Ĵ��� */	
		
					g_tModH.fAck03H = 1;
				}
				break;
		}
	}
}

/*
*********************************************************************************************************
*	�� �� ��: MODH_Read_10H
*	����˵��: ����10Hָ���Ӧ�����ݣ�д�������Ĵ�����16bit����
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void MODH_Read_10H(void)
{
	/*
		10Hָ���Ӧ��:
			�ӻ���ַ                11
			������                  10
			�Ĵ�����ʼ��ַ���ֽ�	00
			�Ĵ�����ʼ��ַ���ֽ�    01
			�Ĵ����������ֽ�        00
			�Ĵ����������ֽ�        02
			CRCУ����ֽ�           12
			CRCУ����ֽ�           98
	*/
	if (g_tModH.RxCount > 0)
	{
		if (g_tModH.RxBuf[0] == SlaveAddr)		
		{
			g_tModH.fAck10H = 1;		/* ���յ�Ӧ�� */
		}
	}
}

/*
*********************************************************************************************************
*	�� �� ��: MODH_ReadParam_01H
*	����˵��: ��������. ͨ������01Hָ��ʵ�֣�����֮�󣬵ȴ��ӻ�Ӧ��
*	��    ��: ��
*	�� �� ֵ: 1 ��ʾ�ɹ���0 ��ʾʧ�ܣ�ͨ�ų�ʱ�򱻾ܾ���
*********************************************************************************************************
*/
uint8_t MODH_ReadParam_01H(uint16_t _reg, uint16_t _num)
{
	int32_t time1;
	uint8_t i;
	
	for (i = 0; i < NUM; i++)
	{
		MODH_Send01H (SlaveAddr, _reg, _num);		  /* �������� */
		time1 = bsp_GetRunTime();	/* ��¼����͵�ʱ�� */
		
		while (1)				/* �ȴ�Ӧ��,��ʱ����յ�Ӧ����break  */
		{
			bsp_Idle();

			if (bsp_CheckRunTime(time1) > TIMEOUT)		
			{
				break;		/* ͨ�ų�ʱ�� */
			}
			
			if (g_tModH.fAck01H > 0)
			{
				break;		/* ���յ�Ӧ�� */
			}
		}
		
		if (g_tModH.fAck01H > 0)
		{
			break;			/* ѭ��NUM�Σ�������յ�������breakѭ�� */
		}
	}
	
	if (g_tModH.fAck01H == 0)
	{
		return 0;
	}
	else 
	{
		return 1;	/* 01H ���ɹ� */
	}
}

/*
*********************************************************************************************************
*	�� �� ��: MODH_ReadParam_02H
*	����˵��: ��������. ͨ������02Hָ��ʵ�֣�����֮�󣬵ȴ��ӻ�Ӧ��
*	��    ��: ��
*	�� �� ֵ: 1 ��ʾ�ɹ���0 ��ʾʧ�ܣ�ͨ�ų�ʱ�򱻾ܾ���
*********************************************************************************************************
*/
uint8_t MODH_ReadParam_02H(uint16_t _reg, uint16_t _num)
{
	int32_t time1;
	uint8_t i;
	
	for (i = 0; i < NUM; i++)
	{
		MODH_Send02H (SlaveAddr, _reg, _num);
		time1 = bsp_GetRunTime();	/* ��¼����͵�ʱ�� */
		
		while (1)
		{
			bsp_Idle();

			if (bsp_CheckRunTime(time1) > TIMEOUT)		
			{
				break;		/* ͨ�ų�ʱ�� */
			}
			
			if (g_tModH.fAck02H > 0)
			{
				break;
			}
		}
		
		if (g_tModH.fAck02H > 0)
		{
			break;
		}
	}
	
	if (g_tModH.fAck02H == 0)
	{
		return 0;
	}
	else 
	{
		return 1;	/* 02H ���ɹ� */
	}
}
/*
*********************************************************************************************************
*	�� �� ��: MODH_ReadParam_03H
*	����˵��: ��������. ͨ������03Hָ��ʵ�֣�����֮�󣬵ȴ��ӻ�Ӧ��
*	��    ��: ��
*	�� �� ֵ: 1 ��ʾ�ɹ���0 ��ʾʧ�ܣ�ͨ�ų�ʱ�򱻾ܾ���
*********************************************************************************************************
*/
uint8_t MODH_ReadParam_03H(uint16_t _reg, uint16_t _num)
{
	int32_t time1;
	uint8_t i;
	
	for (i = 0; i < NUM; i++)
	{
		MODH_Send03H (SlaveAddr, _reg, _num);
		time1 = bsp_GetRunTime();	/* ��¼����͵�ʱ�� */
		
		while (1)
		{
			bsp_Idle();

			if (bsp_CheckRunTime(time1) > TIMEOUT)		
			{
				break;		/* ͨ�ų�ʱ�� */
			}
			
			if (g_tModH.fAck03H > 0)
			{
				break;
			}
		}
		
		if (g_tModH.fAck03H > 0)
		{
			break;
		}
	}
	
	if (g_tModH.fAck03H == 0)
	{
		return 0;	/* ͨ�ų�ʱ�� */
	}
	else 
	{
		return 1;	/* д��03H�����ɹ� */
	}
}


/*
*********************************************************************************************************
*	�� �� ��: MODH_ReadParam_04H
*	����˵��: ��������. ͨ������04Hָ��ʵ�֣�����֮�󣬵ȴ��ӻ�Ӧ��
*	��    ��: ��
*	�� �� ֵ: 1 ��ʾ�ɹ���0 ��ʾʧ�ܣ�ͨ�ų�ʱ�򱻾ܾ���
*********************************************************************************************************
*/
uint8_t MODH_ReadParam_04H(uint16_t _reg, uint16_t _num)
{
	int32_t time1;
	uint8_t i;
	
	for (i = 0; i < NUM; i++)
	{
		MODH_Send04H (SlaveAddr, _reg, _num);
		time1 = bsp_GetRunTime();	/* ��¼����͵�ʱ�� */
		
		while (1)
		{
			bsp_Idle();

			if (bsp_CheckRunTime(time1) > TIMEOUT)		
			{
				break;		/* ͨ�ų�ʱ�� */
			}
			
			if (g_tModH.fAck04H > 0)
			{
				break;
			}
		}
		
		if (g_tModH.fAck04H > 0)
		{
			break;
		}
	}
	
	if (g_tModH.fAck04H == 0)
	{
		return 0;	/* ͨ�ų�ʱ�� */
	}
	else 
	{
		return 1;	/* 04H ���ɹ� */
	}
}
/*
*********************************************************************************************************
*	�� �� ��: MODH_WriteParam_05H
*	����˵��: ��������. ͨ������05Hָ��ʵ�֣�����֮�󣬵ȴ��ӻ�Ӧ��
*	��    ��: ��
*	�� �� ֵ: 1 ��ʾ�ɹ���0 ��ʾʧ�ܣ�ͨ�ų�ʱ�򱻾ܾ���
*********************************************************************************************************
*/
uint8_t MODH_WriteParam_05H(uint16_t _reg, uint16_t _value)
{
	int32_t time1;
	uint8_t i;

	for (i = 0; i < NUM; i++)
	{
		MODH_Send05H (SlaveAddr, _reg, _value);
		time1 = bsp_GetRunTime();	/* ��¼����͵�ʱ�� */
		
		while (1)
		{
			bsp_Idle();
			
			/* ��ʱ���� TIMEOUT������Ϊ�쳣 */
			if (bsp_CheckRunTime(time1) > TIMEOUT)		
			{
				break;	/* ͨ�ų�ʱ�� */
			}
			
			if (g_tModH.fAck05H > 0)
			{
				break;
			}
		}
		
		if (g_tModH.fAck05H > 0)
		{
			break;
		}
	}
	
	if (g_tModH.fAck05H == 0)
	{
		return 0;	/* ͨ�ų�ʱ�� */
	}
	else
	{
		return 1;	/* 05H д�ɹ� */
	}
}

/*
*********************************************************************************************************
*	�� �� ��: MODH_WriteParam_06H
*	����˵��: ��������. ͨ������06Hָ��ʵ�֣�����֮�󣬵ȴ��ӻ�Ӧ��ѭ��NUM��д����
*	��    ��: ��
*	�� �� ֵ: 1 ��ʾ�ɹ���0 ��ʾʧ�ܣ�ͨ�ų�ʱ�򱻾ܾ���
*********************************************************************************************************
*/
uint8_t MODH_WriteParam_06H(uint16_t _reg, uint16_t _value)
{
	int32_t time1;
	uint8_t i;
	
	for (i = 0; i < NUM; i++)
	{	
		MODH_Send06H (SlaveAddr, _reg, _value);
		time1 = bsp_GetRunTime();	/* ��¼����͵�ʱ�� */
				
		while (1)
		{
			bsp_Idle();
		
			if (bsp_CheckRunTime(time1) > TIMEOUT)		
			{
				break;
			}
			
			if (g_tModH.fAck06H > 0)
			{
				break;
			}
		}
		
		if (g_tModH.fAck06H > 0)
		{
			break;
		}
	}
	
	if (g_tModH.fAck06H == 0)
	{
		return 0;	/* ͨ�ų�ʱ�� */
	}
	else
	{
		return 1;	/* д��06H�����ɹ� */
	}
}

/*
*********************************************************************************************************
*	�� �� ��: MODH_WriteParam_10H
*	����˵��: ��������. ͨ������10Hָ��ʵ�֣�����֮�󣬵ȴ��ӻ�Ӧ��ѭ��NUM��д����
*	��    ��: ��
*	�� �� ֵ: 1 ��ʾ�ɹ���0 ��ʾʧ�ܣ�ͨ�ų�ʱ�򱻾ܾ���
*********************************************************************************************************
*/
uint8_t MODH_WriteParam_10H(uint16_t _reg, uint8_t _num, uint8_t *_buf)
{
	int32_t time1;
	uint8_t i;
	
	for (i = 0; i < NUM; i++)
	{	
		MODH_Send10H(SlaveAddr, _reg, _num, _buf);
		time1 = bsp_GetRunTime();	/* ��¼����͵�ʱ�� */
				
		while (1)
		{
			bsp_Idle();
		
			if (bsp_CheckRunTime(time1) > TIMEOUT)		
			{
				break;
			}
			
			if (g_tModH.fAck10H > 0)
			{
				break;
			}
		}
		
		if (g_tModH.fAck10H > 0)
		{
			break;
		}
	}
	
	if (g_tModH.fAck10H == 0)
	{
		return 0;	/* ͨ�ų�ʱ�� */
	}
	else
	{
		return 1;	/* д��10H�����ɹ� */
	}
}

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/

