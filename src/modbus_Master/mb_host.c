/**
 ******************************************************************************
 * @file    mb_host.c
 * @author  Derrick Wang
 * @brief   modebus主机实现代码
 ******************************************************************************
 * @note
 * 该文件无需修改
 ******************************************************************************
 */
#include "mb_include.h"
#include "string.h"

mb_host_t mb_host;

/* 区分主机和中控MBS指令ack:主机的会直接下发,selfcmd为0,中控下发时调用的MBS_01H等会置1,收到ack/timeout后会清零 */

/**
 * MBS_Master_Period_100ms
 * @brief 100ms执行一次 周期检测子设备状态
 * @author Honokahqh
 * @date 2023-08-05
 */
extern uint32_t OTA_Last_time;
void MBS_Master_Period_100ms(void)
{
	if (mb_host.Read_Verison)
		return;
	static uint32_t count_100ms = 0;
	count_100ms++;
	if (OTA_Last_time)
	{
		OTA_Last_time--;
		return;
	}
	/* 检查存活策略:每10秒轮询一次在线设备,每100秒轮询一次未在线设备 */
	for (uint8_t i = 0; i < MBS_Slaver_Num; i++)
	{
		if ((MBS_ALL[i].map->reg[1].data || MBS_ALL[i].map->isAlive) && count_100ms % 100 == i)
		{
			MBS_03H(MBS_ALL[i].map->mbs_id, 59000, 2);
		}
		else if (count_100ms % 1000 == i * 10)
		{
			MBS_03H(MBS_ALL[i].map->mbs_id, 59000, 2);
		}
	}
	/* 存活设备特殊轮询 400ms 人体红外检测 */
	if (MBS_ALL[MBS_IR_Detect_Index].map->isAlive && count_100ms % 12 == 0)
	{
		MBS_01H(IR_Detect_ADDR, 10000, 1);
	}
	if (MBS_ALL[MBS_IR_Detect2_Index].map->isAlive && count_100ms % 12 == 1)
	{
		MBS_01H(IR_Detect2_ADDR, 10000, 1);
	}
	/* 开关面板按键检测 */
	if (MBS_ALL[MBS_Switch_Index].map->isAlive && count_100ms % 4 == 2)
	{
		MBS_01H(MBS_ALL[MBS_Switch_Index].map->mbs_id, 12000, 4);
	}
	/* 中控红外检测 */
	if (MBS_ALL[MBS_RoomIR_Index].map->isAlive && count_100ms % 4 == 3)
	{
		MBS_01H(MBS_ALL[MBS_RoomIR_Index].map->mbs_id, 10000, 31);
	}
	if (MBS_ALL[MBS_RoomIR_Index].map->isAlive && count_100ms % 100 == 1)
	{
		MBS_03H(MBS_ALL[MBS_RoomIR_Index].map->mbs_id, 50000, 1);
	}
	/* 蓝牙模块检测 */
	if (MBS_ALL[MBS_BLE_Index].map->isAlive && count_100ms % 12 == 3)
	{
		MBS_01H(MBS_ALL[MBS_BLE_Index].map->mbs_id, 12009, 1);
	}
	if (MBS_ALL[MBS_BLE_Index].map->isAlive && count_100ms % 12 == 4)
	{
		MBS_03H(MBS_ALL[MBS_BLE_Index].map->mbs_id, 51008, 1);
	}
}

/**
 * mbh_data_process
 * @brief 处理开关面板按键数据(12000,120001)\人体红外数据(10000)
 * @brief 若数据发生变化则上报至主机
 * @author Honokahqh
 * @date 2023-08-05
 */
static void mbh_data_process()
{
	uint8_t temp_data[4];
	if (MBS_ALL[MBS_Switch_Index].map->coil[0].data || MBS_ALL[MBS_Switch_Index].map->coil[1].data)
	{
		temp_data[0] = MBS_ALL[MBS_Switch_Index].map->coil[0].data;
		temp_data[1] = MBS_ALL[MBS_Switch_Index].map->coil[1].data;
		temp_data[2] = MBS_ALL[MBS_Switch_Index].map->coil[2].data;
		temp_data[3] = MBS_ALL[MBS_Switch_Index].map->coil[3].data;
		CusProfile_Send(0x0000, MBS_Switch_CMD, temp_data, 4, false);
		MBS_ALL[MBS_Switch_Index].map->coil[0].data = 0;
		MBS_ALL[MBS_Switch_Index].map->coil[1].data = 0;
		MBS_0FH(MBS_ALL[MBS_Switch_Index].map->mbs_id, 12000, 2, 0);
	}

	static uint8_t ADDR_0X90_State, ADDR_0X91_State;
	if (MBS_ALL[MBS_IR_Detect_Index].map->coil[0].data != ADDR_0X90_State)
	{
		ADDR_0X90_State = MBS_ALL[MBS_IR_Detect_Index].map->coil[0].data;
		temp_data[0] = IR_Detect_ADDR;
		temp_data[1] = MBS_ALL[MBS_IR_Detect_Index].map->coil[0].data;
		CusProfile_Send(0x0000, MBS_IR_Detect_CMD, temp_data, 2, false);
	}
	if (MBS_ALL[MBS_IR_Detect2_Index].map->coil[1].data != ADDR_0X91_State)
	{
		ADDR_0X91_State = MBS_ALL[MBS_IR_Detect2_Index].map->coil[1].data;
		temp_data[0] = IR_Detect2_ADDR;
		temp_data[1] = MBS_ALL[MBS_IR_Detect2_Index].map->coil[1].data;
		CusProfile_Send(0x0000, MBS_IR_Detect2_CMD, temp_data, 2, false);
	}

	static uint8_t ble_connected, ble_song_index;
	if (MBS_ALL[MBS_BLE_Index].map->coil[0].data != ble_connected || MBS_ALL[MBS_BLE_Index].map->reg[2].data != ble_song_index)
	{
		ble_connected = MBS_ALL[MBS_BLE_Index].map->coil[0].data;
		ble_song_index = MBS_ALL[MBS_BLE_Index].map->reg[2].data;
		temp_data[0] = ble_song_index;
		temp_data[1] = ble_connected;
		CusProfile_Send(0x0000, MBS_BLE_CMD, temp_data, 2, false);
		Debug_B("MBS_BLE_CMD:%d %d\r\n", temp_data[0], temp_data[1]);
	}

	static uint8_t RoomIR_all_open;
	if (MBS_ALL[MBS_RoomIR_Index].map->coil[0].data)
	{
		MBS_ALL[MBS_RoomIR_Index].map->coil[0].data = 0;
		*DeviceControlState[29] = 1;
		for (uint8_t i = 1; i < 5; i++) // SCR
		{
			if (MBS_ALL[MBS_RoomIR_Index].map->coil[i].data)
			{
				if (*DeviceControlState[i])
				{
					*DeviceControlState[i] = 0;
				}
				else
				{
					*DeviceControlState[i] = 100;
				}
			}
		}
		for (uint8_t i = 5; i < 24; i++) // 继电器
		{
			if (MBS_ALL[MBS_RoomIR_Index].map->coil[i].data)
			{
				*DeviceControlState[i] = !(*DeviceControlState[i]);
			}
		}
		for (uint8_t i = 24; i < 27; i++) // 空调低中高
		{
			if (MBS_ALL[MBS_RoomIR_Index].map->coil[i].data)
			{
				*DeviceControlState[i] = !(*DeviceControlState[i]);
				*DeviceControlState[28] = *DeviceControlState[i]; //阀开
				*DeviceControlState[27] = 0;					  //阀关
			}
		}
		if (MBS_ALL[MBS_RoomIR_Index].map->coil[29].data)
		{
			RoomIR_all_open = !RoomIR_all_open;
			if (RoomIR_all_open)
			{
				for (uint8_t i = 1; i < 5; i++)
				{
					*DeviceControlState[i] = 100;
				}
				for (uint8_t i = 5; i < 29; i++) // 继电器
				{
					*DeviceControlState[i] = 1;
				}
			}
			else
			{
				for (uint8_t i = 1; i < 5; i++)
				{
					*DeviceControlState[i] = 0;
				}
				for (uint8_t i = 5; i < 29; i++) // 继电器
				{
					*DeviceControlState[i] = 0;
				}
			}
		}
		for (uint8_t i = 0; i < 30; i++)
		{
			MBS_ALL[MBS_RoomIR_Index].map->coil[i].data = 0;
		}
	}
}

// 接收正确,已过CRC校验和首字节校验
uint8_t mbh_exec(uint8_t *pframe, uint8_t len)
{
	/* 解析处理,如果当前mb指令源于中控 */
	for (uint8_t i = 0; i < MBS_Slaver_Num; i++)
	{
		if (MBS_ALL[i].map->mbs_id == pframe[0])
		{
			MBS_ALL[i].map->isAlive = 1; // 数据有效-子设备存活
			MBS_ALL[i].map->err_count = 0;
		}
	}
	if (mb_host.isSelfcmd == false)
	{
		mb_host.state = MBS_STATE_IDLE; // 不是本机指令不做处理,重置mbs状态
		return false;
	}

	switch (pframe[1]) // cmd
	{
	case 1:
		mbh_hook_rec01(pframe[0], (pframe + 2), len - 2);
		break;
	case 3:
		mbh_hook_rec03(pframe[0], (pframe + 2), len - 2);
		break;
	case 5:
		mbh_hook_rec05(pframe[0], (pframe + 2), len - 2);
		break;
	case 6:
		mbh_hook_rec06(pframe[0], (pframe + 2), len - 2);
		break;
	case 15:
		mbh_hook_rec15(pframe[0], (pframe + 2), len - 2);
		break;
	case 16:
		mbh_hook_rec16(pframe[0], (pframe + 2), len - 2);
		break;
	default:
		Debug_B("cmd error:%d\r\n", pframe[1]);
		break;
	}
	mb_host.isSelfcmd = false;
	mb_host.state = MBS_STATE_IDLE;
	mbh_data_process();
	return true;
}

/* MBS轮询 */
void mbh_poll()
{
	uint8_t temp_data[256], data_len;
	switch (mb_host.state)
	{
	case MBS_STATE_IDLE:
		if (ringbuff_read_Hex(&Z9_ringbuf, temp_data, &data_len) == TRUE)
		{
			mb_host.ErrTimes = 0;
			mb_host.isSelfcmd = false; // 来自主机的指令,不处理MBS ack
			mb_port_send_data(temp_data, data_len);
			return; // 此时状态变为wait ack
		}
		else if (ringbuff_read_Hex(&MBS_ringbuf, temp_data, &data_len) == TRUE)
		{
			mb_host.ErrTimes = 0;
			mb_host.isSelfcmd = true;
			mb_port_send_data(temp_data, data_len);
			return; // 此时状态变为wait ack
		}
		else if (mb_host.Read_Verison)
		{
			mb_host.Read_Verison = false;
			PCmd_Query_SubVersion_ACK();
		}
		break;
	case MBS_STATE_WAIT_ACK:
		if ((uint32_t)(mb_port_get_tick() - mb_host.timeout) > MBS_TIMEOUT)
		{
			if (mb_host.ErrTimes < MBS_ERRTIME_MAX && mb_host.isSelfcmd) // 本机指令超时重发
			{
				mb_host.ErrTimes++;
				mb_port_send_data(mb_host.txbuf, mb_host.txlen);
			}
			else
			{
				if (mb_host.isSelfcmd)
				{
					/* 查询超时,将对应设备的ALIVE设置为0 */
					for (uint8_t i = 0; i < MBS_Slaver_Num; i++)
					{
						if (MBS_ALL[i].map->mbs_id == mb_host.txbuf[0])
						{
							// CusProfile_Send(0x0000,0x80, mb_host.txbuf, 5, false);
							MBS_ALL[i].map->err_count++;
							if (MBS_ALL[i].map->err_count > 3)
							{
								MBS_ALL[i].map->isAlive = 0;
								MBS_ALL[i].map->err_count = 0;
							}
						}
					}
				}
				mb_host.ErrTimes = 0;
				mb_host.isSelfcmd = false;
				mb_host.state = MBS_STATE_IDLE;
			}
		}
		break;
	default:
		break;
	}
}

/* MBS指令集 详细格式参考mb_hook.h */
void MBS_01H(uint8_t ID, uint16_t addr, uint16_t num)
{
	uint16_t crc;
	uint8_t data[8];

	data[0] = ID;
	data[1] = 0x01;
	data[2] = addr >> 8;
	data[3] = addr;
	data[4] = num >> 8;
	data[5] = num;
	crc = mb_crc16(data, 6);
	data[6] = crc;
	data[7] = crc >> 8;
	if (mb_host.state == MBS_STATE_IDLE)
	{
		Debug_B("MBS_01H CRC:%x\n", crc);
		mb_host.ErrTimes = 0;
		mb_host.isSelfcmd = 1;
		mb_port_send_data(data, 8);
	}
	else
	{
		if (ringbuff_write_Hex(&MBS_ringbuf, data, 8) == FALSE)
		{
			Debug_B("MBS_ringbuf is full\n");
		}
	}
}

void MBS_03H(uint8_t ID, uint16_t addr, uint16_t num)
{
	uint16_t crc;
	uint8_t data[8];

	data[0] = ID;
	data[1] = 0x03;
	data[2] = addr >> 8;
	data[3] = addr;
	data[4] = num >> 8;
	data[5] = num;
	crc = mb_crc16(data, 6);
	data[6] = crc;
	data[7] = crc >> 8;
	if (mb_host.state == MBS_STATE_IDLE)
	{
		// Debug_B("MBS_03H CRC:%x\n", crc);
		mb_host.ErrTimes = 0;
		mb_host.isSelfcmd = 1;
		mb_port_send_data(data, 8);
	}
	else
	{
		if (ringbuff_write_Hex(&MBS_ringbuf, data, 8) == FALSE)
		{
			Debug_B("MBS_ringbuf is full\n");
		}
	}
}

void MBS_05H(uint8_t ID, uint16_t addr, uint16_t value)
{
	uint16_t crc;
	uint8_t data[8];

	data[0] = ID;
	data[1] = 0x05;
	data[2] = addr >> 8;
	data[3] = addr;
	if (value)
	{
		data[4] = 0xFF;
		data[5] = 0x00;
	}
	else
	{
		data[4] = 0x00;
		data[5] = 0x00;
	}
	crc = mb_crc16(data, 6);
	data[6] = crc;
	data[7] = crc >> 8;
	if (mb_host.state == MBS_STATE_IDLE)
	{
		Debug_B("MBS_05H CRC:%x\n", crc);
		mb_host.ErrTimes = 0;
		mb_host.isSelfcmd = 1;
		mb_port_send_data(data, 8);
	}
	else
	{
		if (ringbuff_write_Hex(&MBS_ringbuf, data, 8) == FALSE)
		{
			Debug_B("MBS_ringbuf is full\n");
		}
	}
}

void MBS_06H(uint8_t ID, uint16_t addr, uint16_t value)
{
	uint16_t crc;
	uint8_t data[8];

	data[0] = ID;
	data[1] = 0x06;
	data[2] = addr >> 8;
	data[3] = addr;
	data[4] = value >> 8;
	data[5] = value;
	crc = mb_crc16(data, 6);
	data[6] = crc;
	data[7] = crc >> 8;
	if (mb_host.state == MBS_STATE_IDLE)
	{
		Debug_B("MBS_06H CRC:%x\n", crc);
		mb_host.ErrTimes = 0;
		mb_host.isSelfcmd = 1;
		mb_port_send_data(data, 8);
	}
	else
	{
		if (ringbuff_write_Hex(&MBS_ringbuf, data, 8) == FALSE)
		{
			Debug_B("MBS_ringbuf is full\n");
		}
	}
}

void MBS_0FH(uint8_t ID, uint16_t addr, uint16_t num, uint8_t *data)
{
	uint16_t crc;
	uint8_t temp_data[256];
	if (num > 1968)
	{
		Debug_B("MBS_0FH num > 1968\n");
		return;
	}
	temp_data[0] = ID;
	temp_data[1] = 0x0F;
	temp_data[2] = addr >> 8;
	temp_data[3] = addr;
	temp_data[4] = num >> 8;
	temp_data[5] = num;
	// len = num/8向上取整
	temp_data[6] = (num + 7) / 8;
	for (uint8_t i = 0; i < temp_data[6]; i++)
	{
		temp_data[7 + i] = data[i];
	}
	crc = mb_crc16(temp_data, 7 + temp_data[6]);
	temp_data[7 + temp_data[6]] = crc;
	temp_data[8 + temp_data[6]] = crc >> 8;

	if (mb_host.state == MBS_STATE_IDLE)
	{
		// Debug_B("MBS_0FH CRC:%x\n", crc);
		mb_host.ErrTimes = 0;
		mb_host.isSelfcmd = 1;
		mb_port_send_data(temp_data, 9 + temp_data[6]);
	}
	else
	{
		if (ringbuff_write_Hex(&MBS_ringbuf, temp_data, 9 + temp_data[6]) == FALSE)
		{
			Debug_B("MBS_ringbuf is full\n");
		}
	}
	// Debug_B("len:%d\n", 9 + temp_data[6]);
}

void MBS_10H(uint8_t ID, uint16_t addr, uint16_t num, uint16_t *data)
{
	uint16_t crc;
	uint8_t temp_data[256];
	if (num > 123)
	{
		Debug_B("MBS_10H num > 123\n");
		return;
	}
	temp_data[0] = ID;
	temp_data[1] = 0x10;
	temp_data[2] = addr >> 8;
	temp_data[3] = addr;
	temp_data[4] = num >> 8;
	temp_data[5] = num;
	// len = num*2
	temp_data[6] = num * 2;
	for (uint8_t i = 0; i < num; i++)
	{
		temp_data[7 + i * 2] = data[i] >> 8;
		temp_data[8 + i * 2] = data[i];
	}
	crc = mb_crc16(temp_data, 7 + num * 2);
	temp_data[7 + num * 2] = crc;
	temp_data[8 + num * 2] = crc >> 8;

	if (mb_host.state == MBS_STATE_IDLE)
	{
		Debug_B("MBS_10H CRC:%x\n", crc);
		mb_host.ErrTimes = 0;
		mb_host.isSelfcmd = 1;
		mb_port_send_data(temp_data, 9 + num * 2);
	}
	else
	{
		if (ringbuff_write_Hex(&MBS_ringbuf, temp_data, 9 + num * 2) == FALSE)
		{
			Debug_B("MBS_ringbuf is full\n");
		}
	}
}

void mbs_coildata_syn(MBS_Map_t *MBS_Map, uint16_t addr, uint8_t *data, uint8_t len)
{
	// 根据首地址将数据同步至mbs_coil
	uint16_t i;
	Debug_B("addr:%d len:%d\n", addr, len);
	for (i = 0; i < MBS_Map->coil_num; i++)
	{
		// 首地址、连续性验证
		if (MBS_Map->coil[i].addr == addr && MBS_Map->coil[i + len - 1].addr == addr + len - 1)
		{
			// Debug_B("i:%d\r\n",i);

			// 将modbus byte数据转为bit并保存
			for (uint8_t j = 0; j < len; j++)
			{
				MBS_Map->coil[i + j].data = (data[j / 8] >> (j % 8)) & 0x01;
			}
			break;
		}
	}
	if (i == MBS_Map->coil_num)
	{
		Debug_B("mbs_coildata_syn error\n");
	}
}

void mbs_regdata_syn(MBS_Map_t *MBS_Map, uint16_t addr, uint8_t *data, uint8_t len)
{
	// 根据首地址将数据同步至mbs_reg
	uint16_t i;
	// Debug_B("mbs addr:%d data:\r\n",addr);
	// for(i = 0;i < len; i++)
	// {
	// 	Debug_B("%02x ",data[i]);
	// }
	// Debug_B("MBS_Room_Map:%02x\r\n",MBS_RoomCtrl.reg[65].addr);
	// Debug_B("MBS_Map->reg_num:%d, MBS_Map->reg[65].addr:%d \r\n",MBS_Map->reg_num,MBS_Map->reg[65].addr);

	for (i = 0; i < MBS_Map->reg_num; i++)
	{
		// 首地址、连续性验证
		if (MBS_Map->reg[i].addr == addr && MBS_Map->reg[i + len - 1].addr == addr + len - 1)
		{
			// Debug_B("mbs_regdata_syn addr:%d\n", addr);
			for (uint8_t j = 0; j < len; j++)
			{
				MBS_Map->reg[i + j].data = (data[j * 2] << 8) + data[(j * 2) + 1];
			}
			break;
		}
	}

	if (i >= MBS_Map->reg_num)
	{
		Debug_B("mbs_regdata_syn error\n");
	}
	// MBS_Map->reg[0].data = 0x11;
}