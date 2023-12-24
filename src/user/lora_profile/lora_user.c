#include "lora_core.h"
#include "lora_user.h"

#include "APP.h"
#include "ymodem.h"

/**
 * ChipID_Syn
 * @brief 芯片数据同步至MAC和chipID
 * @author Honokahqh
 * @date 2023-08-05
 */
void ChipID_Syn()
{
    uint32_t id[2];
    id[0] = EFC->SN_L;
    id[1] = EFC->SN_H;
    Lora_State.Mac[0] = id[0] & 0xFF;
    Lora_State.Mac[1] = (id[0] >> 8) & 0xFF;
    Lora_State.Mac[2] = (id[0] >> 16) & 0xFF;
    Lora_State.Mac[3] = (id[0] >> 24) & 0xFF;

    Lora_State.Mac[4] = id[1] & 0xFF;
    Lora_State.Mac[5] = (id[1] >> 8) & 0xFF;
    Lora_State.Mac[6] = (id[1] >> 16) & 0xFF;
    Lora_State.Mac[7] = (id[1] >> 24) & 0xFF;
    Lora_State.chip_ID = Lora_State.Mac[0] + Lora_State.Mac[1] + Lora_State.Mac[2] + Lora_State.Mac[3] + Lora_State.Mac[4] + Lora_State.Mac[5] + Lora_State.Mac[6] + Lora_State.Mac[7];
}

/**
 * Lora_StateInit
 * @brief 设备初始化
 * @author Honokahqh
 * @date 2023-08-05
 */
void Lora_StateInit()
{
    Lora_State.DeviceType = Device_Type;
    ChipID_Syn();
    Lora_State_Data_Syn();
    // Lora_AsData_Syn();
}

/**
 * User_Slaver_Cmd
 * @brief 自定义的指令
 * @author Honokahqh
 * @date 2023-08-05
 */
uint8_t RoomCtrlTestMode = 0;
void User_Slaver_Cmd()
{
    uint8_t temp_data[4], i;
    switch (Lora_State.Rx_CMD)
    {
    case MBS_Switch_Query:
        temp_data[0] = MBS_ALL[MBS_Switch_Index].map->coil[0].data;
        temp_data[1] = MBS_ALL[MBS_Switch_Index].map->coil[1].data;
        temp_data[2] = MBS_ALL[MBS_Switch_Index].map->coil[2].data;
        temp_data[3] = MBS_ALL[MBS_Switch_Index].map->coil[3].data;
        CusProfile_Send(Lora_State.Rx_DAddr, MBS_Switch_CMD, temp_data, 4, false);
        break;
    case RoomCtrl_Test_CMD:
        for (i = 0; i < 30; i++) // 继电器映射表
        {
            if (mbsHoldRegValue[35 + i].pData != i)
                break;
        }
        if (i == 30)
            RoomCtrlTestMode = 1;
    default:
        break;
    }
}

/**
 * Random_Delay
 * @brief 0~10ms的随机延迟
 * @author Honokahqh
 * @date 2023-08-05
 */
void Random_Delay()
{
    static uint8_t i;
    uint16_t ms;
    ms = (Lora_State.Mac[3] << 8) + Lora_State.Mac[4];
    ms = (ms / (1 + i * 2)) % 10;
    i++;
    delay_ms(ms);
}

/**
 * Lora_Send_Data
 * @brief 数据发送
 * @param data 要发送的数据
 * @param len 数据的长度
 * @author Honokahqh
 * @date 2023-08-05
 */
void Lora_Send_Data(uint8_t *data, uint8_t len)
{
    // Random_Delay();
    delay_ms(10);
    Radio.Send(data, len);
    Wait2TXEnd();
}

/**
 * Lora_Sleep
 * @brief Lora进入睡眠模式
 * @author Honokahqh
 * @date 2023-08-05
 */
void Lora_Sleep()
{
    Radio.Rx(0);
    Wait2RXEnd();
    Radio.Sleep();
    if (Radio.GetStatus() != RF_IDLE)
        return;
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_RTC, false);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_SAC, false);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_LORA, false);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_ADC, false);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_UART0, false);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_BSTIMER0, false);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_ADC, false);
    delay_ms(5);
    pwr_deepsleep_wfi(PWR_LP_MODE_STOP3);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_RTC, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_SAC, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_LORA, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_ADC, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_UART0, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_BSTIMER0, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_ADC, true);
    delay_ms(5);
}

/**
 * Cmd_OTA_Device
 * @brief 从机处理OTA数据
 * @author Honokahqh
 * @date 2023-08-05
 */
uint8_t IAP_Key[] = {0xff, Device_Type, 0x50, 0xA5, 0x5A, 0x38, 0x26, 0xFE};
void Cmd_OTA_Device()
{
    if (Lora_State.isMaster || Lora_State.Rx_SAddr != 0)
        return;
    if (ymodem_session.state == YMODEM_STATE_IDLE)
    {
        for (uint8_t i = 0; i < 8; i++)
        {
            if (Lora_State.Rx_Data[Data_Addr + i] != IAP_Key[i])
                return;
        }
        /* 写标志位,进boot */
        uint8_t temp_data[8];
        temp_data[0] = 0x01;
        flash_erase_page(OTA_ADDR);
        flash_program_bytes(OTA_ADDR + 0, temp_data, 8); // 清除第一次上电FLAG
        flash_program_bytes(OTA_ADDR + 8, temp_data, 8); // 需要IAP

        temp_data[0] = Lora_State.Rx_Data[Data_Addr + 8];
        Debug_B("OTA BW:%d\r\n", temp_data[0]);
        if (temp_data[0] > 2)
            temp_data[0] = 0;
        flash_program_bytes(OTA_ADDR + 40, temp_data, 8); // OTA模式BW带宽设置
        temp_data[0] = YMODEM_CRC;
        CusProfile_Send(0x0000, OTA_Device_ACK, temp_data, 1, true);
        Reset_FLAG = 1;
        return;
    }
#if !Lora_Is_APP
    uint8_t data[133];
    uint16_t len;
    int res = ymodem_packet_analysis(&Lora_State.Rx_Data[Data_Addr], Lora_State.Rx_Data[Len_Addr], data, &len);
    delay_ms(5);
    if (len > 0)
        CusProfile_Send(0x0000, OTA_Device_ACK, data, len, true);

    if (res == 2) // 更新完成
    {
        boot_to_app(APP_ADDR);
    }
#endif
}

/**
 * Cmd_OTA_SubDevice
 * @brief 主机OTA从机
 * @author Honokahqh
 * @date 32023-08-05
 */
#define OTA_TIME_OUT 100 // 10s
uint32_t OTA_Last_time = 0;
uint8_t STM8_IAP_Flag = 0;
void Cmd_OTA_SubDevice()
{
    OTA_Last_time = OTA_TIME_OUT;
    if (Lora_State.Rx_Data[Len_Addr] == 8 && Lora_State.Rx_Data[Data_Addr] == 0xFF)
    {
        for (uint8_t i = 0; i < MBS_Slaver_Num; i++)
        {
            if (MBS_ALL[i].map->mbs_id == Lora_State.Rx_Data[Data_Addr + 1])
            {
                MBS_ALL[i].map->isAlive = 0;
                MBS_ALL[i].map->isAlive_Last = 0;
            }
        }
    }
    if (Lora_State.Rx_Data[Data_Addr] == 0xCC && Lora_State.Rx_Data[Data_Addr + 1] == 0xDD)
    {
        STM8_IAP_Flag = 1;
    }
    MBS_Physical2SendBuff(&Lora_State.Rx_Data[Data_Addr], Lora_State.Rx_Data[Len_Addr]);
}

/**
 * PCmd_OTA_SubDeviceAck
 * @brief 主机OTA从机应答
 * @param data 应答数据
 * @param len 应答数据长度
 * @author Honokahqh
 * @date 2023-08-05
 */
void PCmd_OTA_SubDeviceAck(uint8_t *data, uint8_t len)
{
    CusProfile_Send(0x0000, OTA_SubDevice_ACK, data, len, true);
}

/**
 * Cmd_Query_Version
 * @brief 主机查询从机版本号
 * @author Honokahqh
 * @date 2023-08-05
 */
void Cmd_Query_Version()
{
    uint8_t tx_buf[4];
    if (Lora_Para_AT.isMaster == true)
        return;
    tx_buf[0] = 0;
    tx_buf[1] = Device_Type;
    tx_buf[2] = 0;
#if Lora_Is_APP
    tx_buf[3] = Dev_Version;
#else
    tx_buf[3] = Dev_Version - 100;
#endif
    CusProfile_Send(Lora_State.Rx_SAddr, Query_Version_ACK, tx_buf, 4, true);
}

/**
 * Cmd_Query_SubVersion
 * @brief 主机查询子设备版本号
 * @author Honokahqh
 * @date 2023-08-05
 */
void Cmd_Query_SubVersion()
{
    if (Lora_Para_AT.isMaster == true)
        return;
    if (Lora_State.Net_State != Net_JoinGateWay)
        return;
    mb_host.Read_Verison = 1;
    mb_host.Rx_SAddr = Lora_State.Rx_SAddr;
    for (uint8_t i = 0; i < MBS_Slaver_Num; i++)
    {
        // MBS_ALL[i].map->reg[1].data = 0;                                 // 版本号清空
        MBS_03H(MBS_ALL[i].map->mbs_id, MBS_ALL[i].map->reg[0].addr, 2); // 读地址和版本号
    }
}

/**
 * PCmd_Query_SubVersion_ACK
 * @brief 主机查询子设备版本号回复
 * @author Honokahqh
 * @date 2023-08-05
 */
void PCmd_Query_SubVersion_ACK()
{
    if (Lora_Para_AT.isMaster == true)
        return;
    if (Lora_State.Net_State != Net_JoinGateWay)
        return;
    uint8_t Tx_buf[MBS_Slaver_Num * 4], num = 0;
    for (uint8_t i = 0; i < MBS_Slaver_Num; i++)
    {
        if (MBS_ALL[i].map->isAlive) // 版本号有效
        {
            Tx_buf[num * 4 + 0] = (MBS_ALL[i].map->reg[0].data) >> 8;   // addr
            Tx_buf[num * 4 + 1] = (MBS_ALL[i].map->reg[0].data) & 0xFF; // addr
            Tx_buf[num * 4 + 2] = (MBS_ALL[i].map->reg[1].data) >> 8;   // ver
            Tx_buf[num * 4 + 3] = (MBS_ALL[i].map->reg[1].data) & 0xFF; // ver
            num++;
        }
        else if (MBS_ALL[i].map->reg[1].data)
        {
            Tx_buf[num * 4 + 0] = (MBS_ALL[i].map->reg[0].data) >> 8;   // addr
            Tx_buf[num * 4 + 1] = (MBS_ALL[i].map->reg[0].data) & 0xFF; // addr
            Tx_buf[num * 4 + 2] = 0;                                    // ver
            Tx_buf[num * 4 + 3] = 0;                                    // ver
            num++;
        }
        MBS_ALL[i].map->isAlive_Last = MBS_ALL[i].map->isAlive;
    }
    CusProfile_Send(mb_host.Rx_SAddr, Query_SubVersion_ACK, Tx_buf, num * 4, true);
}

void PCmd_RoomIR_Update()
{
    uint8_t data[2];
    if (MBS_ALL[MBS_RoomIR_Index].map->reg[2].data == 0xFFFF)
        return;
    data[0] = MBS_ALL[MBS_RoomIR_Index].map->reg[2].data >> 8;
    data[1] = MBS_ALL[MBS_RoomIR_Index].map->reg[2].data;
    CusProfile_Send(0x0000, MBS_RoomIR_CMD, data, 2, true);
}
