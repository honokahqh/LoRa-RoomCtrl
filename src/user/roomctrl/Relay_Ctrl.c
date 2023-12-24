#include "MBS_Cmd_Ctrl.h"

uint16_t *DeviceControlState[Relay_Num]; // DAC SCR Relay
uint16_t *Relay_Index[Relay_Num];		 // indexָ��mbsHoldRegValue[35 + i].pData

/**
 * AirCon_Relay_Auto_Close
 * @brief �յ������Զ��ر�
 * @author Honokahqh
 * @date 2023-08-05
 */
#define Aircon_Auto_Close 0
#define AirCon_Close_Time 30 // ����ȴ�ʱ��Ϊ30��
uint8_t AirCon_Close_Flag = 0, close_count = 0;
void AirCon_Relay_Auto_Close()
{
#if Aircon_Auto_Close
	if (AirCon_Close_Flag == 1) // close�̵�������1
	{
		if (close_count == 0)
			close_count = AirCon_Close_Time; // ��ʼ��ʱ
		if (close_count)
		{
			close_count--;
			if (close_count == 0) // ��ʱ����
			{
				AirCon_Close_Flag = 0;
				*DeviceControlState[27] = 0; // ���عر�
			}
		}
	}
#endif
}

/**
 * MBS_DATA_Print
 * @brief �Ĵ���״̬��ӡ
 * @author Honokahqh
 * @date 2023-08-05
 */
void MBS_DATA_Print()
{
}

/**
 * MBS_Data_Init
 * @brief ��Ҫ���ڼ̵����������� �̵�����ź͵�ַһһ��Ӧ
 * @author Honokahqh
 * @date 2023-08-05
 */
void MBS_Data_Init()
{
	/* Դ������������ �͵ư�ƺŶ�Ӧ */
	DeviceControlState[0] = &mbsHoldRegValue[Reg_DAC].pData;	   // DAC
	DeviceControlState[1] = &mbsHoldRegValue[Reg_SCR_1].pData;	   // SCR 1
	DeviceControlState[2] = &mbsHoldRegValue[Reg_SCR_2].pData;	   // SCR 2
	DeviceControlState[3] = &mbsHoldRegValue[Reg_SCR_3].pData;	   // SCR 3
	DeviceControlState[4] = &mbsHoldRegValue[Reg_SCR_4].pData;	   // SCR 4
	DeviceControlState[5] = &mbsCoilValue[30].pData;			   // �Զ���:1	1A1
	DeviceControlState[6] = &mbsCoilValue[31].pData;			   // �Զ���:2	1A2
	DeviceControlState[7] = &mbsCoilValue[10].pData;			   // ɳ��		1A3
	DeviceControlState[8] = &mbsCoilValue[11].pData;			   // �ڲ�		1A4
	DeviceControlState[9] = &mbsCoilValue[6].pData;				   // ����		2A1
	DeviceControlState[10] = &mbsCoilValue[8].pData;			   // ���		2A2
	DeviceControlState[11] = &mbsCoilValue[9].pData;			   // ̨��		2A3
	DeviceControlState[12] = &mbsCoilValue[12].pData;			   // ������	2A4
	DeviceControlState[13] = &mbsCoilValue[13].pData;			   // �ŷ���	2A5
	DeviceControlState[14] = &mbsCoilValue[27].pData;			   // �Զ���:3	3A1
	DeviceControlState[15] = &mbsCoilValue[22].pData;			   //
	DeviceControlState[16] = &mbsCoilValue[23].pData;			   //
	DeviceControlState[17] = &mbsCoilValue[24].pData;			   // �Զ���:4	3A4
	DeviceControlState[18] = &mbsCoilValue[28].pData;			   // �Զ���:5	3A5
	DeviceControlState[19] = &mbsCoilValue[32].pData;			   // ������	4A1
	DeviceControlState[20] = &mbsCoilValue[33].pData;			   // ������	4A2
	DeviceControlState[21] = &mbsCoilValue[25].pData;			   // ɴ����	4A3
	DeviceControlState[22] = &mbsCoilValue[26].pData;			   // ɴ����	4A4
	DeviceControlState[23] = &mbsCoilValue[29].pData;			   // �Զ���:8	4A5
	DeviceControlState[24] = &mbsCoilValue[19].pData;			   // ��		5A1
	DeviceControlState[25] = &mbsCoilValue[18].pData;			   // ��		5A2
	DeviceControlState[26] = &mbsCoilValue[17].pData;			   // ��		5A3
	DeviceControlState[27] = &mbsCoilValue[20].pData;			   // ����		5A4
	DeviceControlState[28] = &mbsCoilValue[21].pData;			   // ����		5A5
	DeviceControlState[29] = &mbsCoilValue[Coil_MainSwitch].pData; // �ܿ���
	DeviceControlState[30] = &mbsHoldRegValue[Reg_DAC].pData;	   // DAC

	mbsHoldRegValue[Reg_DAC].pData = 50;   // DAC
	mbsHoldRegValue[Reg_SCR_1].pData = 50; // SCR 1
	mbsHoldRegValue[Reg_SCR_2].pData = 50; // SCR 2
	mbsHoldRegValue[Reg_SCR_3].pData = 50; // SCR 3
	mbsHoldRegValue[Reg_SCR_4].pData = 50; // SCR 4
	mbsCoilValue[Coil_MainSwitch].pData = 1;
	mbsCoilValue[Coil_PWM_Switch].pData = 0;
	for (uint8_t i = 0; i < 30; i++) // �̵���ӳ���
	{
		mbsHoldRegValue[35 + i].pData = i;
		Relay_Index[i] = &mbsHoldRegValue[35 + i].pData;
	}
	for (uint8_t i = 5; i < 30; i++)
		*DeviceControlState[i] = 1;
}

/**
 * RelayGPIO_Write
 * @brief �̵������ƺ��� 3A 4A���鲻���������
 * @author Honokahqh
 * @date 2023-08-05
 */
void RelayGPIO_Write(uint8_t Index, uint8_t newState)
{
	if (newState > 1)
		return;
#if ROOM_TYPE
	if (Index == 5)
		gpio_write(Relay_1A1_PORT, Relay_1A1_PIN, newState);
	if (Index == 6)
		gpio_write(Relay_1A2_PORT, Relay_1A2_PIN, newState);
	if (Index == 7)
		gpio_write(Relay_1A3_PORT, Relay_1A3_PIN, newState);
	if (Index == 8)
		gpio_write(Relay_1A4_PORT, Relay_1A4_PIN, newState);
#endif
	if (Index == 9)
		gpio_write(Relay_2A1_PORT, Relay_2A1_PIN, newState);
	if (Index == 10)
		gpio_write(Relay_2A2_PORT, Relay_2A2_PIN, newState);
	if (Index == 11)
		gpio_write(Relay_2A3_PORT, Relay_2A3_PIN, newState);
	if (Index == 12)
		gpio_write(Relay_2A4_PORT, Relay_2A4_PIN, newState);
	if (Index == 13)
		gpio_write(Relay_2A5_PORT, Relay_2A5_PIN, newState);
	if (Index == 24)
		gpio_write(Relay_5A1_PORT, Relay_5A1_PIN, newState);
	if (Index == 25)
		gpio_write(Relay_5A2_PORT, Relay_5A2_PIN, newState);
	if (Index == 26)
		gpio_write(Relay_5A3_PORT, Relay_5A3_PIN, newState);
	if (Index == 27)
		gpio_write(Relay_5A4_PORT, Relay_5A4_PIN, newState);
	if (Index == 28)
		gpio_write(Relay_5A5_PORT, Relay_5A5_PIN, newState);
}
static void HC595_send_data(uint8_t data)
{
#if ROOM_TYPE
	for (uint8_t bit = 0; bit < 8; bit++)
	{
		if ((data << bit) & 0x80)
			gpio_write(Relay_595_DATA_PORT, Relay_595_DATA_PIN, GPIO_LEVEL_HIGH);
		else
			gpio_write(Relay_595_DATA_PORT, Relay_595_DATA_PIN, GPIO_LEVEL_LOW);
		gpio_write(Relay_595_SCLK_PORT, Relay_595_SCLK_PIN, GPIO_LEVEL_LOW);
		gpio_write(Relay_595_SCLK_PORT, Relay_595_SCLK_PIN, GPIO_LEVEL_HIGH);
	}
	gpio_write(Relay_595_HCLK_PORT, Relay_595_HCLK_PIN, GPIO_LEVEL_LOW);
	gpio_write(Relay_595_HCLK_PORT, Relay_595_HCLK_PIN, GPIO_LEVEL_HIGH);
#endif
}
/**
 * Relay_All_Closed
 * @brief �ر�ȫ���̵���
 * @author Honokahqh
 * @date 2023-08-05
 */
void Relay_All_Closed(void)
{
	uint8_t i;
	for (i = 0; i < 29; i++)
		RelayGPIO_Write(i, 0);
	HC595_send_data(0);
}

/**
 * Relay_CTRL
 * @brief ��������:1.����index��ѯ����¼����ļ���̵�����ַ,������� 2.����ʣ��ļ̵���
 * @author Honokahqh
 * @date 2023-08-05
 */
static uint8_t Wind_level = 0;
void Relay_CTRL()
{
	uint8_t i, Temp_index[3], used[30];

	memset(used, 0, 30);
	if (RoomCtrlTestMode == false) // ����ģʽ�²����û���
	{
		/* �ٶ� *Relay_Index[5] = 19 *Relay_Index[6] = 20,��̵���5��6����,19��20������.���޸ļĴ���5ʱʵ�ʿ���19*/
		memset(Temp_index, 0xFF, 3);
		for (i = 5; i < 30; i++)
		{
			if (*Relay_Index[i] == 19) // ������		��Ϊ��ת����Ϊ��ת����Ϊ0ֹͣ
				Temp_index[0] = i;
			if (*Relay_Index[i] == 20) // ������
				Temp_index[1] = i;
		}
		if (Temp_index[0] != 0xFF && Temp_index[1] != 0xFF)
		{ // ���������ַ��Ч
			if (*DeviceControlState[19])
			{									   // �в�ֵʱ���Ƚ���Ҫ�Ͽ��ĶϿ�����ֹ�������������Ϊ1�����
				RelayGPIO_Write(Temp_index[1], 0); // ������ �Ͽ�
				RelayGPIO_Write(Temp_index[0], 1); // ������ ����
				*DeviceControlState[20] = 0;
			}
			else if (*DeviceControlState[20])
			{
				RelayGPIO_Write(Temp_index[0], 0); // ������ �Ͽ�
				RelayGPIO_Write(Temp_index[1], 1); // ������ �Ͽ�
				*DeviceControlState[19] = 0;
			}
			else
			{
				RelayGPIO_Write(Temp_index[0], 0); // ������ �Ͽ�
				RelayGPIO_Write(Temp_index[1], 0); // ������ �Ͽ�
				*DeviceControlState[19] = 0;
				*DeviceControlState[20] = 0;
			}
			// Debug_B("index:%d-%d ����:%d-%d\r\n", Temp_index[0],Temp_index[1],*DeviceControlState[19], *DeviceControlState[20]);
			// }
		}
		used[Temp_index[0]] = 1;
		used[Temp_index[1]] = 1;

		memset(Temp_index, 0xFF, 3);
		for (i = 5; i < 30; i++)
		{
			if (*Relay_Index[i] == 21) // ɴ����		��Ϊ��ת����Ϊ��ת����Ϊ0ֹͣ
				Temp_index[0] = i;
			if (*Relay_Index[i] == 22) // ɴ����
				Temp_index[1] = i;
		}
		if (Temp_index[0] != 0xFF && Temp_index[1] != 0xFF)
		{ // ���������ַ��Ч
			if (*DeviceControlState[21])
			{									   // �в�ֵʱ���Ƚ���Ҫ�Ͽ��ĶϿ�����ֹ�������������Ϊ1�����
				RelayGPIO_Write(Temp_index[1], 0); // ɴ���� �Ͽ�
				RelayGPIO_Write(Temp_index[0], 1); // ɴ���� ����
				*DeviceControlState[22] = 0;
			}
			else if (*DeviceControlState[22])
			{
				RelayGPIO_Write(Temp_index[0], 0); // ɴ���� �Ͽ�
				RelayGPIO_Write(Temp_index[1], 1); // ɴ���� �Ͽ�
				*DeviceControlState[21] = 0;
			}
			else
			{
				RelayGPIO_Write(Temp_index[0], 0); // ɴ���� �Ͽ�
				RelayGPIO_Write(Temp_index[1], 0); // ɴ���� �Ͽ�
				*DeviceControlState[21] = 0;
				*DeviceControlState[22] = 0;
			}
		}
		used[Temp_index[0]] = 1;
		used[Temp_index[1]] = 1;

		memset(Temp_index, 0xFF, 3);
		for (i = 5; i < 30; i++)
		{
			if (*Relay_Index[i] == 28) // �յ�����
				Temp_index[0] = i;
			if (*Relay_Index[i] == 27) // �յ�����
				Temp_index[1] = i;
		}
		if (Temp_index[0] != 0xFF && Temp_index[1] != 0xFF)
		{ // ���������ַ��Ч
			if (*DeviceControlState[28])
			{									   // �в�ֵʱ���Ƚ���Ҫ�Ͽ��ĶϿ�����ֹ�������������Ϊ1�����
				RelayGPIO_Write(Temp_index[1], 0); // ���� �Ͽ�
				RelayGPIO_Write(Temp_index[0], 1); // ���� ����
				*DeviceControlState[27] = 0;
				AirCon_Close_Flag = 0;
				close_count = 0;
			}
			else if (*DeviceControlState[27])
			{
				RelayGPIO_Write(Temp_index[0], 0); // ���� �Ͽ�
				RelayGPIO_Write(Temp_index[1], 1); // ���� ����
				*DeviceControlState[28] = 0;
				AirCon_Close_Flag = 1; // ���ش򿪺�5���Զ��ر�
			}
			else
			{
				RelayGPIO_Write(Temp_index[0], 0); // ���� �Ͽ�
				RelayGPIO_Write(Temp_index[1], 0); // ���� ����
				*DeviceControlState[27] = 0;
				*DeviceControlState[28] = 0;
				AirCon_Close_Flag = 0;
				close_count = 0;
			}
		}
		// Debug_B("index:%d-%d �յ���:%d-%d aircon:%d close:%d\r\n", Temp_index[0], Temp_index[1], *DeviceControlState[28], *DeviceControlState[27],
		// 		AirCon_Close_Flag, close_count);

		used[Temp_index[0]] = 1;
		used[Temp_index[1]] = 1;

		memset(Temp_index, 0xFF, 3);
		for (i = 5; i < 30; i++)
		{
			if (*Relay_Index[i] == 24) // ����-��
				Temp_index[0] = i;
			if (*Relay_Index[i] == 25) // ����-��
				Temp_index[1] = i;
			if (*Relay_Index[i] == 26) // ����-��
				Temp_index[2] = i;
		}
		if (Temp_index[0] != 0xFF && Temp_index[1] != 0xFF && Temp_index[2] != 0xFF)
		{ // ���������ַ��Ч
			if (*DeviceControlState[24] && Wind_level != 1)
			{									   // �в�ֵʱ���Ƚ���Ҫ�Ͽ��ĶϿ�����ֹ�������������Ϊ1�����
				RelayGPIO_Write(Temp_index[1], 0); // ����-�� �Ͽ�
				RelayGPIO_Write(Temp_index[2], 0); // ����-�� �Ͽ�
				RelayGPIO_Write(Temp_index[0], 1); // ����-�� ����
				*DeviceControlState[24] = 1;
				*DeviceControlState[25] = 0;
				*DeviceControlState[26] = 0;
				Wind_level = 1;
			}
			else if (*DeviceControlState[25] && Wind_level != 2)
			{
				RelayGPIO_Write(Temp_index[0], 0); // ����-�� �Ͽ�
				RelayGPIO_Write(Temp_index[2], 0); // ����-�� �Ͽ�
				RelayGPIO_Write(Temp_index[1], 1); // ����-�� ����
				*DeviceControlState[24] = 0;
				*DeviceControlState[25] = 1;
				*DeviceControlState[26] = 0;
				Wind_level = 2;
			}
			else if (*DeviceControlState[26] && Wind_level != 3)
			{
				RelayGPIO_Write(Temp_index[0], 0); // ����-�� �Ͽ�
				RelayGPIO_Write(Temp_index[1], 0); // ����-�� �Ͽ�
				RelayGPIO_Write(Temp_index[2], 1); // ����-�� ����
				*DeviceControlState[24] = 0;
				*DeviceControlState[25] = 0;
				*DeviceControlState[26] = 1;
				Wind_level = 3;
			}
			else if (*DeviceControlState[24] == 0 && *DeviceControlState[25] == 0 && *DeviceControlState[26] == 0 && Wind_level != 0)
			{
				RelayGPIO_Write(Temp_index[0], 0); // ����-�� �Ͽ�
				RelayGPIO_Write(Temp_index[1], 0); // ����-�� �Ͽ�
				RelayGPIO_Write(Temp_index[2], 0); // ����-�� �Ͽ�
				*DeviceControlState[24] = 0;
				*DeviceControlState[25] = 0;
				*DeviceControlState[26] = 0;
				Wind_level = 0;
			}
		}
		used[Temp_index[0]] = 1;
		used[Temp_index[1]] = 1;
		used[Temp_index[2]] = 1;
	}

	for (i = 5; i < 30; i++)
	{
		if (used[i] == 0)
		{
			RelayGPIO_Write(i, *DeviceControlState[*Relay_Index[i]]);
		}
	}
	uint8_t data = 0;
	data |= *DeviceControlState[14] << 7;
	data |= *DeviceControlState[17] << 6;
	data |= *DeviceControlState[18] << 5;
	data |= *DeviceControlState[19] << 4;
	data |= *DeviceControlState[20] << 3;
	data |= *DeviceControlState[21] << 2;
	data |= *DeviceControlState[22] << 1;
	data |= *DeviceControlState[23] << 0;
	HC595_send_data(data);
}

/**
 * Relay_Service
 * @brief �̵�������-��ѭ���ڵ���
 * @author Honokahqh
 * @date 2023-08-05
 */
void Relay_Service()
{
	static uint8_t i, AirConEnable = 0;
	if (mbsCoilValue[Coil_MainSwitch].pData)
	{ // �ܿ���
		AirConEnable = 1;
		Relay_CTRL(); // �̵���
	}
	else
	{ // �ܿ���
		if (AirConEnable)
		{	
			for (i = 0; i < 30; i++)
			{
				if (*Relay_Index[i] == 28)
					RelayGPIO_Write(i, 0);
			}
			delay_ms(100);
			for (i = 0; i < 30; i++)
			{
				if (*Relay_Index[i] == 27)
					RelayGPIO_Write(i, 1);
			}
			delay_ms(300);
			Wind_level = 0;
			AirConEnable = 0;
		}
		Relay_All_Closed();
	}
}