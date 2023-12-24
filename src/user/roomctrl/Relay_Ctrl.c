#include "MBS_Cmd_Ctrl.h"

uint16_t *DeviceControlState[Relay_Num]; // DAC SCR Relay
uint16_t *Relay_Index[Relay_Num];		 // index指向mbsHoldRegValue[35 + i].pData

/**
 * AirCon_Relay_Auto_Close
 * @brief 空调阀关自动关闭
 * @author Honokahqh
 * @date 2023-08-05
 */
#define Aircon_Auto_Close 0
#define AirCon_Close_Time 30 // 定义等待时间为30秒
uint8_t AirCon_Close_Flag = 0, close_count = 0;
void AirCon_Relay_Auto_Close()
{
#if Aircon_Auto_Close
	if (AirCon_Close_Flag == 1) // close继电器被置1
	{
		if (close_count == 0)
			close_count = AirCon_Close_Time; // 开始计时
		if (close_count)
		{
			close_count--;
			if (close_count == 0) // 计时结束
			{
				AirCon_Close_Flag = 0;
				*DeviceControlState[27] = 0; // 阀关关闭
			}
		}
	}
#endif
}

/**
 * MBS_DATA_Print
 * @brief 寄存器状态打印
 * @author Honokahqh
 * @date 2023-08-05
 */
void MBS_DATA_Print()
{
}

/**
 * MBS_Data_Init
 * @brief 主要用于继电器重新排序 继电器编号和地址一一对应
 * @author Honokahqh
 * @date 2023-08-05
 */
void MBS_Data_Init()
{
	/* 源数据重新排序 和灯板灯号对应 */
	DeviceControlState[0] = &mbsHoldRegValue[Reg_DAC].pData;	   // DAC
	DeviceControlState[1] = &mbsHoldRegValue[Reg_SCR_1].pData;	   // SCR 1
	DeviceControlState[2] = &mbsHoldRegValue[Reg_SCR_2].pData;	   // SCR 2
	DeviceControlState[3] = &mbsHoldRegValue[Reg_SCR_3].pData;	   // SCR 3
	DeviceControlState[4] = &mbsHoldRegValue[Reg_SCR_4].pData;	   // SCR 4
	DeviceControlState[5] = &mbsCoilValue[30].pData;			   // 自定义:1	1A1
	DeviceControlState[6] = &mbsCoilValue[31].pData;			   // 自定义:2	1A2
	DeviceControlState[7] = &mbsCoilValue[10].pData;			   // 沙发		1A3
	DeviceControlState[8] = &mbsCoilValue[11].pData;			   // 壁插		1A4
	DeviceControlState[9] = &mbsCoilValue[6].pData;				   // 吊灯		2A1
	DeviceControlState[10] = &mbsCoilValue[8].pData;			   // 射灯		2A2
	DeviceControlState[11] = &mbsCoilValue[9].pData;			   // 台灯		2A3
	DeviceControlState[12] = &mbsCoilValue[12].pData;			   // 卫生间	2A4
	DeviceControlState[13] = &mbsCoilValue[13].pData;			   // 排风扇	2A5
	DeviceControlState[14] = &mbsCoilValue[27].pData;			   // 自定义:3	3A1
	DeviceControlState[15] = &mbsCoilValue[22].pData;			   //
	DeviceControlState[16] = &mbsCoilValue[23].pData;			   //
	DeviceControlState[17] = &mbsCoilValue[24].pData;			   // 自定义:4	3A4
	DeviceControlState[18] = &mbsCoilValue[28].pData;			   // 自定义:5	3A5
	DeviceControlState[19] = &mbsCoilValue[32].pData;			   // 布帘开	4A1
	DeviceControlState[20] = &mbsCoilValue[33].pData;			   // 布帘关	4A2
	DeviceControlState[21] = &mbsCoilValue[25].pData;			   // 纱帘开	4A3
	DeviceControlState[22] = &mbsCoilValue[26].pData;			   // 纱帘关	4A4
	DeviceControlState[23] = &mbsCoilValue[29].pData;			   // 自定义:8	4A5
	DeviceControlState[24] = &mbsCoilValue[19].pData;			   // 低		5A1
	DeviceControlState[25] = &mbsCoilValue[18].pData;			   // 中		5A2
	DeviceControlState[26] = &mbsCoilValue[17].pData;			   // 高		5A3
	DeviceControlState[27] = &mbsCoilValue[20].pData;			   // 阀关		5A4
	DeviceControlState[28] = &mbsCoilValue[21].pData;			   // 阀开		5A5
	DeviceControlState[29] = &mbsCoilValue[Coil_MainSwitch].pData; // 总开关
	DeviceControlState[30] = &mbsHoldRegValue[Reg_DAC].pData;	   // DAC

	mbsHoldRegValue[Reg_DAC].pData = 50;   // DAC
	mbsHoldRegValue[Reg_SCR_1].pData = 50; // SCR 1
	mbsHoldRegValue[Reg_SCR_2].pData = 50; // SCR 2
	mbsHoldRegValue[Reg_SCR_3].pData = 50; // SCR 3
	mbsHoldRegValue[Reg_SCR_4].pData = 50; // SCR 4
	mbsCoilValue[Coil_MainSwitch].pData = 1;
	mbsCoilValue[Coil_PWM_Switch].pData = 0;
	for (uint8_t i = 0; i < 30; i++) // 继电器映射表
	{
		mbsHoldRegValue[35 + i].pData = i;
		Relay_Index[i] = &mbsHoldRegValue[35 + i].pData;
	}
	for (uint8_t i = 5; i < 30; i++)
		*DeviceControlState[i] = 1;
}

/**
 * RelayGPIO_Write
 * @brief 继电器控制函数 3A 4A两组不在这里控制
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
 * @brief 关闭全部继电器
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
 * @brief 控制流程:1.根据index查询并记录互斥的几组继电器地址,而后控制 2.处理剩余的继电器
 * @author Honokahqh
 * @date 2023-08-05
 */
static uint8_t Wind_level = 0;
void Relay_CTRL()
{
	uint8_t i, Temp_index[3], used[30];

	memset(used, 0, 30);
	if (RoomCtrlTestMode == false) // 测试模式下不启用互斥
	{
		/* 假定 *Relay_Index[5] = 19 *Relay_Index[6] = 20,则继电器5、6互斥,19、20不互斥.且修改寄存器5时实际控制19*/
		memset(Temp_index, 0xFF, 3);
		for (i = 5; i < 30; i++)
		{
			if (*Relay_Index[i] == 19) // 布帘开		开为正转、关为反转、均为0停止
				Temp_index[0] = i;
			if (*Relay_Index[i] == 20) // 布帘关
				Temp_index[1] = i;
		}
		if (Temp_index[0] != 0xFF && Temp_index[1] != 0xFF)
		{ // 两个互斥地址有效
			if (*DeviceControlState[19])
			{									   // 有差值时，先将需要断开的断开，防止出现两个互斥均为1的情况
				RelayGPIO_Write(Temp_index[1], 0); // 布帘关 断开
				RelayGPIO_Write(Temp_index[0], 1); // 布帘开 连接
				*DeviceControlState[20] = 0;
			}
			else if (*DeviceControlState[20])
			{
				RelayGPIO_Write(Temp_index[0], 0); // 布帘开 断开
				RelayGPIO_Write(Temp_index[1], 1); // 布帘关 断开
				*DeviceControlState[19] = 0;
			}
			else
			{
				RelayGPIO_Write(Temp_index[0], 0); // 布帘开 断开
				RelayGPIO_Write(Temp_index[1], 0); // 布帘关 断开
				*DeviceControlState[19] = 0;
				*DeviceControlState[20] = 0;
			}
			// Debug_B("index:%d-%d 布帘:%d-%d\r\n", Temp_index[0],Temp_index[1],*DeviceControlState[19], *DeviceControlState[20]);
			// }
		}
		used[Temp_index[0]] = 1;
		used[Temp_index[1]] = 1;

		memset(Temp_index, 0xFF, 3);
		for (i = 5; i < 30; i++)
		{
			if (*Relay_Index[i] == 21) // 纱帘开		开为正转、关为反转、均为0停止
				Temp_index[0] = i;
			if (*Relay_Index[i] == 22) // 纱帘关
				Temp_index[1] = i;
		}
		if (Temp_index[0] != 0xFF && Temp_index[1] != 0xFF)
		{ // 两个互斥地址有效
			if (*DeviceControlState[21])
			{									   // 有差值时，先将需要断开的断开，防止出现两个互斥均为1的情况
				RelayGPIO_Write(Temp_index[1], 0); // 纱帘关 断开
				RelayGPIO_Write(Temp_index[0], 1); // 纱帘开 连接
				*DeviceControlState[22] = 0;
			}
			else if (*DeviceControlState[22])
			{
				RelayGPIO_Write(Temp_index[0], 0); // 纱帘开 断开
				RelayGPIO_Write(Temp_index[1], 1); // 纱帘关 断开
				*DeviceControlState[21] = 0;
			}
			else
			{
				RelayGPIO_Write(Temp_index[0], 0); // 纱帘开 断开
				RelayGPIO_Write(Temp_index[1], 0); // 纱帘关 断开
				*DeviceControlState[21] = 0;
				*DeviceControlState[22] = 0;
			}
		}
		used[Temp_index[0]] = 1;
		used[Temp_index[1]] = 1;

		memset(Temp_index, 0xFF, 3);
		for (i = 5; i < 30; i++)
		{
			if (*Relay_Index[i] == 28) // 空调阀开
				Temp_index[0] = i;
			if (*Relay_Index[i] == 27) // 空调阀关
				Temp_index[1] = i;
		}
		if (Temp_index[0] != 0xFF && Temp_index[1] != 0xFF)
		{ // 两个互斥地址有效
			if (*DeviceControlState[28])
			{									   // 有差值时，先将需要断开的断开，防止出现两个互斥均为1的情况
				RelayGPIO_Write(Temp_index[1], 0); // 阀关 断开
				RelayGPIO_Write(Temp_index[0], 1); // 阀开 连接
				*DeviceControlState[27] = 0;
				AirCon_Close_Flag = 0;
				close_count = 0;
			}
			else if (*DeviceControlState[27])
			{
				RelayGPIO_Write(Temp_index[0], 0); // 阀开 断开
				RelayGPIO_Write(Temp_index[1], 1); // 阀关 连接
				*DeviceControlState[28] = 0;
				AirCon_Close_Flag = 1; // 阀关打开后5秒自动关闭
			}
			else
			{
				RelayGPIO_Write(Temp_index[0], 0); // 阀开 断开
				RelayGPIO_Write(Temp_index[1], 0); // 阀关 连接
				*DeviceControlState[27] = 0;
				*DeviceControlState[28] = 0;
				AirCon_Close_Flag = 0;
				close_count = 0;
			}
		}
		// Debug_B("index:%d-%d 空调阀:%d-%d aircon:%d close:%d\r\n", Temp_index[0], Temp_index[1], *DeviceControlState[28], *DeviceControlState[27],
		// 		AirCon_Close_Flag, close_count);

		used[Temp_index[0]] = 1;
		used[Temp_index[1]] = 1;

		memset(Temp_index, 0xFF, 3);
		for (i = 5; i < 30; i++)
		{
			if (*Relay_Index[i] == 24) // 风速-低
				Temp_index[0] = i;
			if (*Relay_Index[i] == 25) // 风速-中
				Temp_index[1] = i;
			if (*Relay_Index[i] == 26) // 风速-高
				Temp_index[2] = i;
		}
		if (Temp_index[0] != 0xFF && Temp_index[1] != 0xFF && Temp_index[2] != 0xFF)
		{ // 三个互斥地址有效
			if (*DeviceControlState[24] && Wind_level != 1)
			{									   // 有差值时，先将需要断开的断开，防止出现两个互斥均为1的情况
				RelayGPIO_Write(Temp_index[1], 0); // 风速-中 断开
				RelayGPIO_Write(Temp_index[2], 0); // 风速-高 断开
				RelayGPIO_Write(Temp_index[0], 1); // 风速-低 连接
				*DeviceControlState[24] = 1;
				*DeviceControlState[25] = 0;
				*DeviceControlState[26] = 0;
				Wind_level = 1;
			}
			else if (*DeviceControlState[25] && Wind_level != 2)
			{
				RelayGPIO_Write(Temp_index[0], 0); // 风速-低 断开
				RelayGPIO_Write(Temp_index[2], 0); // 风速-高 断开
				RelayGPIO_Write(Temp_index[1], 1); // 风速-中 连接
				*DeviceControlState[24] = 0;
				*DeviceControlState[25] = 1;
				*DeviceControlState[26] = 0;
				Wind_level = 2;
			}
			else if (*DeviceControlState[26] && Wind_level != 3)
			{
				RelayGPIO_Write(Temp_index[0], 0); // 风速-低 断开
				RelayGPIO_Write(Temp_index[1], 0); // 风速-中 断开
				RelayGPIO_Write(Temp_index[2], 1); // 风速-高 连接
				*DeviceControlState[24] = 0;
				*DeviceControlState[25] = 0;
				*DeviceControlState[26] = 1;
				Wind_level = 3;
			}
			else if (*DeviceControlState[24] == 0 && *DeviceControlState[25] == 0 && *DeviceControlState[26] == 0 && Wind_level != 0)
			{
				RelayGPIO_Write(Temp_index[0], 0); // 风速-低 断开
				RelayGPIO_Write(Temp_index[1], 0); // 风速-中 断开
				RelayGPIO_Write(Temp_index[2], 0); // 风速-高 断开
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
 * @brief 继电器控制-主循环内调用
 * @author Honokahqh
 * @date 2023-08-05
 */
void Relay_Service()
{
	static uint8_t i, AirConEnable = 0;
	if (mbsCoilValue[Coil_MainSwitch].pData)
	{ // 总开关
		AirConEnable = 1;
		Relay_CTRL(); // 继电器
	}
	else
	{ // 总开关
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