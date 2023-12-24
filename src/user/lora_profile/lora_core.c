#include "lora_core.h"

associated_devices_t Associated_devices[Device_Num_Max];
register_device_t Register_Device[Register_Device_Num_Max];
Lora_state_t Lora_State;

/**
 * CusProfile_Send
 * @brief Lora数据发送
 * @author Honokahqh
 * @date 2023-08-05
 */
uint8_t PackageID; // 主动发送的Package和ACK的Package
void CusProfile_Send(uint16_t DAddr, uint8_t cmd, uint8_t *data, uint8_t len, uint8_t isAck)
{
    uint8_t i;
    Lora_State.Tx_Data[DevType_Addr] = Device_Type;          // 主机
    Lora_State.Tx_Data[PanIDH_Addr] = Lora_State.PanID >> 8; // 所属网络
    Lora_State.Tx_Data[PanIDL_Addr] = Lora_State.PanID;      // 所属网络
    if (Lora_State.isMaster == true)
    {
        Lora_State.Tx_Data[SAddrH_Addr] = 0; // 本机网络地址
        Lora_State.Tx_Data[SAddrL_Addr] = 0; // 本机网络地址
    }
    else if (Lora_State.Net_State == Net_NotJoin)
    {
        Lora_State.Tx_Data[SAddrH_Addr] = Lora_State.chip_ID >> 8; // 本机网络地址
        Lora_State.Tx_Data[SAddrL_Addr] = Lora_State.chip_ID;      // 本机网络地址
    }
    else
    {
        Lora_State.Tx_Data[SAddrH_Addr] = Lora_State.SAddr >> 8; // 本机网络地址
        Lora_State.Tx_Data[SAddrL_Addr] = Lora_State.SAddr;      // 本机网络地址
    }
    Lora_State.Tx_Data[DAddrH_Addr] = DAddr >> 8; // 目的网络地址
    Lora_State.Tx_Data[DAddrL_Addr] = DAddr;      // 目的网络地址
    if (isAck)
    {
        Lora_State.Tx_Data[PackID_Addr] = Lora_State.Rx_PID;
    }
    else
    {
        Lora_State.Tx_Data[PackID_Addr] = PackageID;
        PackageID++;
    }
    Lora_State.Tx_Data[Cmd_Addr] = cmd;
    Lora_State.Tx_Data[Len_Addr] = len;
    for (i = 0; i < len; i++)
        Lora_State.Tx_Data[i + Data_Addr] = *data++;
    Lora_State.Tx_Data[len + Data_Addr] = XOR_Calculate(Lora_State.Tx_Data, len + Data_Addr);
    Lora_State.Tx_Len = len + Data_Addr + 1;

    Debug_B("Send:");
    for (i = 0; i < Lora_State.Tx_Len; i++)
        Debug_B("%02X ", Lora_State.Tx_Data[i]);
    Debug_B("\r\n");
    Lora_Send_Data(Lora_State.Tx_Data, Lora_State.Tx_Len);
}

/**
 * CusProfile_Receive
 * @brief Lora数据接收处理-根据协议解析数据并调用相应函数处理数据
 * @author Honokahqh
 * @date 2023-08-05
 */
void CusProfile_Receive()
{
    Lora_ReceiveData2State();
    Debug_B("RSSI:%d Receive:", Lora_State.Rx_RSSI);
    for (uint16_t i = 0; i < Lora_State.Rx_Len; i++)
        Debug_B("%02X ", Lora_State.Rx_Data[i]);
    Debug_B("\r\n");
    /* 异或校验是否一致 */
    if (XOR_Calculate(Lora_State.Rx_Data, Lora_State.Rx_Len - 1) != Lora_State.Rx_Data[Lora_State.Rx_Len - 1])
        return;
    /* 验证短地址 step:1 */
    if (Lora_State.Rx_SAddr != BoardCast && Lora_State.Rx_DAddr != Lora_State.SAddr)
        return;
    /* 验证PanID step:1 */
    if (Lora_State.Rx_PanID != Lora_State.PanID && Lora_State.Rx_PanID != BoardCast && Lora_State.PanID != BoardCast)
        return;
    /* 主机收到BeaconRequest且在注册模式时，跳过短地址验证 step2 */
    if (Lora_State.isMaster && Lora_State.Rx_CMD == BeaconRequest && Lora_State.NetOpenTime)
    {
        goto DataProcess;
    }
#if Lora_Register_Enable
    /* 注册flag使能时,待注册设备信息保存在Register_Device内而非AS内 */
    if (Compare_Register_SAddr(Lora_State.Rx_SAddr) != 0xFF && Lora_State.Rx_CMD == SlaverInNet) // 在注册列表内
        goto DataProcess;
#endif
    /* 短地址验证 step2 */
    if (Lora_State.isMaster && Compare_ShortAddr(Lora_State.Rx_SAddr) == 0xFF)
    {
        CusProfile_Send(Lora_State.Rx_SAddr, Master_Request_Leave, NULL, 0, TRUE);
        return;
    }
    
DataProcess:
    switch (Lora_State.Rx_CMD)
    {
    case BeaconRequest:
        Cmd_BeaconRequest();
        break;
    case Beacon:
        Cmd_Beacon();
        break;
    case SlaverInNet:
        Cmd_SlaverInNet();
        break;
    case DeviceAnnonce:
        Cmd_DeviceAnnonce();
        break;
    case Lora_Change_Para:
        Cmd_Lora_Change_Para();
        break;
    case Lora_Query_RSSI:
        Cmd_Query_Rssi();
        break;
    case HeartBeat:
        Cmd_HeartBeat();
        break;
    case Master_Request_Leave:
        Cmd_Master_Request_Leave();
        break;
    case Slaver_Request_Leave:
        Cmd_Slaver_Request_Leave();
        break;
    case MBS_Cmd_Request:
        Cmd_MBS_Cmd_Request();
        break;
    case MBS_Cmd_ACK:
        Cmd_MBS_Cmd_ACK();
        break;
    case Lora_SendData:
        Cmd_Lora_SendData();
    case OTA_Device:
        Cmd_OTA_Device();
        break;
    case OTA_SubDeVice:
        Cmd_OTA_SubDevice();
        break;
    case Query_Version:
        Cmd_Query_Version();
        break;
    case Query_SubVersion:
        Cmd_Query_SubVersion();
        break;
    default:
        User_Slaver_Cmd();
        break;
    }
}

/**
 * Slaver_Period_1s
 * @brief 从机每秒状态更新处理
 * @author Honokahqh
 * @date 2023-08-05
 */
void Slaver_Period_1s()
{
    static uint8_t HeartBeat_Period = 20, Temperature_Period = 0, Slaver_Period = 0;
    if (Lora_State.isMaster == true)
    {
        return;
    }
    /* 未入网状态持续入网 */
    if (Lora_State.Wait_ACK == 0 && Lora_State.Net_State == Net_NotJoin && Lora_State.NetOpenTime)
    {
        // Debug_B("BeaconRequest\r\n");
        PCmd_BeaconRequest();
    }
    /* ACK超时 */
    if (Lora_State.ACK_Timeout)
    {
        Lora_State.ACK_Timeout--;
        if (Lora_State.Wait_ACK && Lora_State.ACK_Timeout == 0)
        {

            Lora_State.ErrTimes++;
            if (Lora_State.ErrTimes > 3)
            {
                switch (Lora_State.Wait_ACK)
                {
                case BeaconRequest:          // 入网失败
                    Lora_State.ErrTimes = 0; // 重置次数 继续请求
                    break;
                case SlaverInNet: // 入网失败
                    Lora_State.Net_State = Net_NotJoin;
                    Lora_State.SAddr = Lora_State.chip_ID;
                    Lora_State.Wait_ACK = 0;
                    Lora_State.ErrTimes = 0;
                    Lora_State.PanID = BoardCast;
                    break;
                default:
                    Lora_State.Wait_ACK = 0;
                    Lora_State.ErrTimes = 0;
                    break;
                }
            }
            switch (Lora_State.Wait_ACK)
            {
            case BeaconRequest:
                CusProfile_Send(0x0000, BeaconRequest, Lora_State.Mac, 8, FALSE);
                Lora_State.ACK_Timeout = 3;
                break;
            case SlaverInNet:
                CusProfile_Send(0x0000, SlaverInNet, Lora_State.Mac, 8, FALSE);
                Lora_State.ACK_Timeout = 3;
                break;
            }
        }
    }
    if (Lora_State.NetOpenTime && Lora_State.NetOpenTime != 0xFF)
    { // 注册模式倒计时
        Lora_State.NetOpenTime--;
        if (Lora_State.NetOpenTime == 0)
        {
            Lora_Para_AT.NetOpenTime = 0;
            Lora_State_Save();
            delay_ms(100);
            NVIC_SystemReset();
        }
    }
    /* 温度 */
    Temperature_Period++;
    if (MBS_ALL[MBS_RoomIR_Index].map->isAlive && Lora_State.Net_State == Net_JoinGateWay && ymodem_session.state == YMODEM_STATE_IDLE && Temperature_Period >= (30 + (Lora_State.SAddr % 30)))
    {
        Temperature_Period = 0;
        PCmd_RoomIR_Update();
        return; // 保证周期数据包1s只发一包
    }
    /* 心跳 */
    HeartBeat_Period++;
    if (HeartBeat_Period > (100 + (Lora_State.SAddr % 100)) && Lora_State.Net_State == Net_JoinGateWay && ymodem_session.state == YMODEM_STATE_IDLE)
    {
        HeartBeat_Period = 0;
        PCmd_HeartBeat();
        return; // 保证周期数据包1s只发一包
    }
    /* 子设备心跳 */
    Slaver_Period++;
    if (Slaver_Period > (100 + (Lora_State.SAddr % 100)) && Lora_State.Net_State == Net_JoinGateWay && ymodem_session.state == YMODEM_STATE_IDLE)
    {
        Slaver_Period = 0;
        PCmd_Query_SubVersion_ACK();
        return; // 保证周期数据包1s只发一包
    }
}

/**
 * Master_Period_1s
 * @brief 主机每秒状态更新处理
 * @author Honokahqh
 * @date 2023-08-05
 */
void Master_Period_1s()
{
    if (Lora_State.isMaster == false)
    {
        return;
    }
#if Lora_Register_Enable
    for (i = 0; i < Register_Device_Num_Max; i++)
    { // 注册设备超时检测
        if (Register_Device[i].timeout)
            Register_Device[i].timeout--;
        if (Register_Device[i].timeout == 0 && Register_Device[i].chip_ID)
            memset(&Register_Device[i], 0, sizeof(register_device_t));
    }
#endif

    if (Lora_State.NetOpenTime && Lora_State.NetOpenTime != 0xFF)
    {
        Lora_State.NetOpenTime--;
        if (Lora_State.NetOpenTime == 0)
        {
            Lora_Para_AT.NetOpenTime = 0;
            Lora_State_Save();
            delay_ms(100);
            NVIC_SystemReset();
        }
    }
}
