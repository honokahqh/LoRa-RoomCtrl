#include "APP.h"

static struct pt Lora_Rx;
static struct pt Period_1s;
static struct pt MBS_CMD_CTRL;
static struct pt Slave_485_Device;

static int Task1_Lora_Rx(struct pt *pt);
static int Task2_1s_Period(struct pt *pt);
static int Task3_MBS_CMD_CTRL(struct pt *pt);
static int Task4_Slave_485_Device(struct pt *pt);
static void ProcessRS485_Data(void);
static void Led_Ctrl(uint32_t count_10ms);
static void Ymodem_timeout_process(void);
static void Reset_Key_Process(void); //�������ô���
/**
 * System_Run
 * @brief PTOS����
 * @author Honokahqh
 * @date 2023-08-05
 */
void System_Run()
{
	PT_INIT(&Lora_Rx);
	PT_INIT(&Period_1s);
	PT_INIT(&MBS_CMD_CTRL);
	PT_INIT(&Slave_485_Device);
	while (1)
	{
		Task1_Lora_Rx(&Lora_Rx);
		Task2_1s_Period(&Period_1s);
		Task3_MBS_CMD_CTRL(&MBS_CMD_CTRL);
		Task4_Slave_485_Device(&Slave_485_Device);
	}
}

/**
 * Task1_Lora_Rx
 * @brief Lora�жϴ���Rst��������
 * @author Honokahqh
 * @date 2023-08-05
 */
extern uint16_t GPIO_RST_Flag;
static int Task1_Lora_Rx(struct pt *pt)
{
	PT_BEGIN(pt);
	while (1)
	{
		if (Lora_Para_AT.Enable == 1)
			Lora_IRQ_Rrocess();
		if (GPIO_RST_Flag) // RST��������
		{
			if (gpio_read(GPIO_RST_PORT, GPIO_RST_PIN) == GPIO_LEVEL_LOW)
				GPIO_RST_Flag++;
			else
				GPIO_RST_Flag = 0;
			if (GPIO_RST_Flag > 600) // 600*5/1000 = 3s
			{
				Lora_Para_AT.Enable = 1;
				Lora_Para_AT.SAddr = 0xFFFF;
				Lora_Para_AT.PanID = 0xFFFF;
				Lora_Para_AT.Net_State = 0;
				Lora_Para_AT.NetOpenTime = 255;
				Lora_State_Save();
				delay_ms(100);
				system_reset();
			}
		}
		PT_TIMER_DELAY(pt, 5);
	}
	PT_END(pt);
}

/**
 * Task2_1s_Period
 * @brief 1����������-lora slaver״̬����-��λ���-led��˸-ι��
 * @author Honokahqh
 * @date 2023-08-05
 */
static int Task2_1s_Period(struct pt *pt)
{
	PT_BEGIN(pt);
	while (1)
	{
		if (Lora_Para_AT.Enable == 1)
			Slaver_Period_1s();
		if (mb_host.state == MBS_STATE_IDLE && mb_host.Read_Verison == 0)
		{
			for (uint8_t i = 0; i < MBS_Slaver_Num; i++)
			{
				if (MBS_ALL[i].map->isAlive != MBS_ALL[i].map->isAlive_Last)
				{
					PCmd_Query_SubVersion_ACK();
					break;
				}
			}
		}
		/* ASR6601�ڶ��Ƕ�׵ĺ�����reset�ᱨ��	--- �ѽ��,�ϵ�ʱ��lora�Ĵ������и�λ */
		if (Reset_FLAG == 1)
		{
			system_reset();
		}
		Ymodem_timeout_process(); // IAPͨѶ��ʱ����
		wdg_reload();			  // ι��
		PT_TIMER_DELAY(pt, 1000);
	}
	PT_END(pt);
}

/**
 * Task3_MBS_CMD_CTRL
 * @brief mbs������ѯ-�п��������-���ݱ���
 * @author Honokahqh
 * @date 2023-08-05
 */
extern uint8_t STM8_IAP_Flag;
extern const uint8_t stm8ArmCode_PA2[];
uint16_t IAP_Delay = 0;
static int Task3_MBS_CMD_CTRL(struct pt *pt)
{
	static uint32_t count_10ms = 0;
	PT_BEGIN(pt);
	while (1)
	{
		count_10ms++;
		mbh_poll();				// MBS����
		if (STM8_IAP_Flag == 2) // LoRa����STM8�ӻ�ʱ,�е�һ���ָ�������
		{
			IAP_Delay++;
			if (IAP_Delay > 200)
			{
				UART_SendData(stm8ArmCode_PA2, 714);
				IAP_Delay = 0;
				STM8_IAP_Flag = 0;
			}
		}
		if (count_10ms % 10 == 0 && Lora_Para_AT.Enable) // ������ѯ���豸
			MBS_Master_Period_100ms();
		if (ymodem_session.state == YMODEM_STATE_IDLE)
			mbs_ctrl();
		if (count_10ms % 1000 == 2) // 10s���һ��
		{
			mbs_data_save(); // ����mbs���ݱ���
		}
		Led_Ctrl(count_10ms); // led��˸
		PT_TIMER_DELAY(pt, 10);
	}
	PT_END(pt);
}

/**
 * Task3_MBS_CMD_CTRL
 * @brief mbs������ѯ-�п��������-���ݱ���
 * @author Honokahqh
 * @date 2023-08-05
 */
static int Task4_Slave_485_Device(struct pt *pt)
{
	PT_BEGIN(pt);
	while (1)
	{
		/* ���ȼ� 1.55AA����ָ�� 2.ATCmd 3.mbs master 4.mbs slaver 5.OTA  */
#ifdef __DEBUG
		if (uart_state.rx_buff[0] == 0x55 && uart_state.rx_buff[1] == 0xAA && uart_state.rx_len > 5)
		{
			handleSend(&uart_state.rx_buff[2], uart_state.rx_len - 2);
		}
		uint8_t res = processATCommand((char *)uart_state.rx_buff);
		if (res == false)
		{
			ProcessRS485_Data();
		}
#else
		ProcessRS485_Data();
#endif
		memset(uart_state.rx_buff, 0, sizeof(uart_state.rx_buff));
		uart_state.rx_len = 0;
		uart_state.has_data = 0;
		PT_WAIT_UNTIL(pt, uart_state.has_data); // �ȴ���������
	}
	PT_END(pt);
}

/**
 * Ymodem_data_process
 * @brief LoRa����-485����ʱOTA�������Դ���
 * @author Honokahqh
 * @date 2023-10-07
 */
void Ymodem_data_process()
{
	static uint8_t IAP_Key[] = {0xff, MBS_RoomCtrlAddr, 0x50, 0xA5, 0x5A, 0x38, 0x26, 0xFE};
	if (ymodem_session.state == YMODEM_STATE_IDLE && Lora_Para_AT.Enable == 0)
	{
		for (uint8_t i = 0; i < 8; i++)
		{
			if (uart_state.rx_buff[i] != IAP_Key[i])
				return;
		}
		/* д��־λ,��boot */
		uint8_t temp_data[8];
		temp_data[0] = 0x01;
		flash_erase_page(OTA_ADDR);
		flash_program_bytes(OTA_ADDR + 0, temp_data, 8); // �����һ���ϵ�FLAG
		flash_program_bytes(OTA_ADDR + 8, temp_data, 8); // ��ҪIAP
		Reset_FLAG = 1;
		return;
	}
#if !Lora_Is_APP
	if (ymodem_session.state != YMODEM_STATE_IDLE && Lora_Para_AT.Enable == 0)
	{
		uint8_t data[2];
		uint16_t len;
		int res = ymodem_packet_analysis(uart_state.rx_buff, uart_state.rx_len, data, &len);
		delay_ms(5);
		if (len > 0)
			UART_SendData(data, len);
		if (res == 2) // �������
		{
			boot_to_app(APP_ADDR);
		}
	}
#endif
}
/**
 * Ymodem_timeout_process
 * @brief IAPͨѶ��ʱ����
 * @author Honokahqh
 * @date 2023-10-07
 */
static void Ymodem_timeout_process()
{
#if !Lora_Is_APP
	if (ymodem_session.state != YMODEM_STATE_IDLE)
	{ // ��loraģʽ��,OTA��ʱ����ڴ˴�����Slaver_Period_1s
		uint8_t temp[2];
		ymodem_session.timeout++;
		if (ymodem_session.timeout > 3)
		{
			ymodem_session.error_count++;
			temp[0] = NAK;
			if (Lora_Para_AT.Enable)
				CusProfile_Send(0, OTA_Device_ACK, temp, 1, true);
			else
				UART_SendData(temp, 1);
		}
		if (ymodem_session.error_count > 5)
		{
			memset(&ymodem_session, 0, sizeof(ymodem_session_t));
			temp[0] = CAN;
			temp[1] = CAN;
			if (Lora_Para_AT.Enable)
				CusProfile_Send(0x0000, OTA_Device_ACK, temp, 2, true);
			else
				UART_SendData(temp, 2);
			Reset_FLAG = 1;
		}
	}
#endif
}
/**
 * ProcessRS485_Data
 * @brief ������յ��Ĵ���0����
 * @author Honokahqh
 * @date 2023-08-05
 */
void ProcessRS485_Data()
{
	static uint8_t init_flag = 1;
	// v1.01���� ����Ƿ�Ϊmbs����(z8 z9 485ͨ��)ָ��,Enable = 0ʱ�������mbs����ָ��
	Ymodem_data_process();
	if (uart_state.rx_len >= 4 && mb_crc16(uart_state.rx_buff, uart_state.rx_len) == 0 && MBS_RoomCtrlAddr == uart_state.rx_buff[0])
	{
		if (Lora_Para_AT.Enable != 0)
		{
			Lora_Para_AT.Enable = 0;
			Lora_Para_AT.SAddr = 0xFFFF;
			Lora_Para_AT.PanID = 0xFFFF;
			Lora_Para_AT.Net_State = 0;
			Lora_Para_AT.NetOpenTime = 0;
			mbsHoldRegValue[MBS_Addr].pData = MBS_RoomCtrlAddr; // �л��豸��ַ
			mbsHoldRegValue[MBS_Ver].pData = MBS_Version;		// �л��汾��
			Lora_State_Save();
		}
		MBS_Buf._rxLen = uart_state.rx_len;
		memcpy(MBS_Buf._rxBuff, uart_state.rx_buff, MBS_Buf._rxLen);
		MBS_CorePoll();
		memset(MBS_Buf._rxBuff, 0, sizeof(MBS_Buf._rxBuff));
		Debug_B("485 mbs process Ending\r\n");
		MBS_Buf._rxLen = 0;
	}
	if (Lora_Para_AT.Enable == 0)
		return;

	// ���¾�ΪLoraʹ�ܺ�Ĵ���취
	if (ymodem_session.state != YMODEM_STATE_IDLE || uart_state.rx_len == 0)
		return;
	/* ���豸�ϵ緢��mbs��ַ */
	if (uart_state.rx_len % 3 == 0 && uart_state.rx_buff[0] == 0xFF && uart_state.rx_buff[2] == 0xFE)
	{
		if (init_flag)
		{
			init_flag = 0;
			delay_ms(1000);
		}
		uint8_t device_num = 0;
		while (device_num < uart_state.rx_len / 3)
		{
			if (uart_state.rx_buff[device_num * 3] == 0xFF && uart_state.rx_buff[device_num * 3 + 2] == 0xFE)
			{
				for (uint8_t i = 0; i < MBS_Slaver_Num; i++)
				{
					if (MBS_ALL[i].map->mbs_id == uart_state.rx_buff[device_num * 3 + 1])
					{
						// MBS_ALL[i].map->isAlive = 1;
						Debug_B("Device:%d is online\r\n", uart_state.rx_buff[device_num * 3 + 1]);
						MBS_03H(MBS_ALL[i].map->mbs_id, MBS_ALL[i].map->reg[0].addr, 2); // ����ַ�Ͱ汾��
						if (MBS_ALL[i].map->mbs_id == 0x28)								 // ��̬�� ��һ�����ݴ����ͨѶʧ��,��дһ��������,��ʹ���ӳ�
							MBS_03H(MBS_ALL[i].map->mbs_id, MBS_ALL[i].map->reg[0].addr, 2);
					}
				}
			}
			else
				break;
			device_num++;
		}
	}
	/* MBSָ�� */
	else if (uart_state.rx_len >= 4 && mb_crc16(uart_state.rx_buff, uart_state.rx_len) == 0 &&
			 mb_host.addr == uart_state.rx_buff[0] && mb_host.cmd == uart_state.rx_buff[1])
	{
		if (mbh_exec(uart_state.rx_buff, uart_state.rx_len) == false) // ��������ָ���(�п�ָ�ֻ������ѯָ��)
		{
			PCmd_MBS_Cmd_ACK(uart_state.rx_buff, uart_state.rx_len);
		}
	}
	// ѡ���Ƿ������Ч���� 1.������
	else
	{
		if (uart_state.rx_len == 1 && uart_state.rx_buff[0] == 0)
		{
			return;
		}
		if (STM8_IAP_Flag && uart_state.rx_buff[0] == 0xA9)
		{
			STM8_IAP_Flag = 2; // delay 1�뷢��armcode
		}
		PCmd_OTA_SubDeviceAck(uart_state.rx_buff, uart_state.rx_len);
	}
}

void Led_Ctrl(uint32_t count_10ms)
{
	static uint32_t last_count = 0;
#if Lora_Is_APP
	if (Lora_State.Net_State == 2) // ����һ��
	{
		if (count_10ms - last_count >= 200)
		{
			gpio_write(System_Led_PORT, System_Led_PIN, 0);
			last_count = count_10ms;
		}
		else if (count_10ms - last_count >= 10)
		{
			gpio_write(System_Led_PORT, System_Led_PIN, 1);
		}
	}
	else // ����״̬ 0.6��һ��
	{
		if (count_10ms - last_count >= 60)
		{
			gpio_write(System_Led_PORT, System_Led_PIN, 0);
			last_count = count_10ms;
		}
		else if (count_10ms - last_count >= 30)
		{
			gpio_write(System_Led_PORT, System_Led_PIN, 1);
		}
	}
#else
	if (Lora_State.Net_State == 2) // һ��һ��
	{
		if (count_10ms - last_count >= 100)
		{
			gpio_write(System_Led_PORT, System_Led_PIN, 0);
			last_count = count_10ms;
		}
		else if (count_10ms - last_count >= 10)
		{
			gpio_write(System_Led_PORT, System_Led_PIN, 1);
		}
	}
	else // 0.3sһ��
	{
		if (count_10ms - last_count >= 30)
		{
			gpio_write(System_Led_PORT, System_Led_PIN, 1);
			last_count = count_10ms;
		}
		else if (count_10ms - last_count >= 15)
		{
			gpio_write(System_Led_PORT, System_Led_PIN, 0);
		}
	}
#endif
}

#define Special_Rst_Mode 0
#define RST_TIMEOUT 5000
static uint8_t Key_Mode[] = {100, 100, 100, 100, 100}; // ʱ�������ϰ���1s �ɿ�1s ����1s �ɿ�1s ����1s
static void Reset_Key_Process()
{
#if Special_Rst_Mode
	static uint32_t timeout = 5000, index;
	if ((Key_Mode[index] && index % 2 == 0 && gpio_read(GPIO_RST_PORT, GPIO_RST_PIN) == GPIO_LEVEL_LOW) || Key_Mode[index] && index % 2 == 1 && gpio_read(GPIO_RST_PORT, GPIO_RST_PIN) == GPIO_LEVEL_HIGH)
	{
		Key_Mode[index]--;
		timeout = 5000;
	}
	else
	{
		if (Key_Mode[index] == 0)
			index++;
		if (index == sizeof(Key_Mode))
		{
			// ����
			Lora_Para_AT.Enable = 1;
			Lora_Para_AT.SAddr = 0xFFFF;
			Lora_Para_AT.PanID = 0xFFFF;
			Lora_Para_AT.Net_State = 0;
			Lora_Para_AT.NetOpenTime = 255;
			Lora_State_Save();
			delay_ms(100);
			system_reset();
		}
		if (timeout > 0)
			timeout--;
		if (timeout == 0)
		{
			index = 0;
			memset(Key_Mode, 100, sizeof(Key_Mode));
		}
	}
#else
	if (gpio_read(GPIO_RST_PORT, GPIO_RST_PIN) == GPIO_LEVEL_LOW)
		GPIO_RST_Flag++;
	else
		GPIO_RST_Flag = 0;
	if (GPIO_RST_Flag > 600) // 600*5/1000 = 3s
	{
		Lora_Para_AT.Enable = 1;
		Lora_Para_AT.SAddr = 0xFFFF;
		Lora_Para_AT.PanID = 0xFFFF;
		Lora_Para_AT.Net_State = 0;
		Lora_Para_AT.NetOpenTime = 255;
		Lora_State_Save();
		delay_ms(100);
		system_reset();
	}
#endif
}