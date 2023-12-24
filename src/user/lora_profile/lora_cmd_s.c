#include "lora_core.h"

/**
 * Cmd_Beacon
 * @brief Sub-device received network access permission information
 * @author Honokahqh
 * @date 2023-08-05
 */
void Cmd_Beacon()
{
    if (Lora_State.isMaster || Lora_State.Rx_SAddr != 0)
        return;
    if (Lora_State.Rx_Data[Len_Addr] == 3)
    {
        Lora_State.SAddr = (Lora_State.Rx_Data[Data_Addr] << 8) + Lora_State.Rx_Data[Data_Addr + 1];
        Lora_State.PanID = Lora_State.Rx_PanID;
        Lora_State.Channel = Lora_State.Rx_Data[Data_Addr + 2];
        Lora_State.Net_State = Net_Joining; // 未入网 已建立连接
        Lora_State.Wait_ACK = SlaverInNet;
        CusProfile_Send(0x0000, SlaverInNet, Lora_State.Mac, 8, TRUE);
    }
    else if (Lora_State.Rx_Data[Len_Addr] == 6)
    {
        Lora_State.SAddr = (Lora_State.Rx_Data[Data_Addr] << 8) + Lora_State.Rx_Data[Data_Addr + 1];
        Lora_State.PanID = Lora_State.Rx_PanID;
        Lora_State.Channel = Lora_State.Rx_Data[Data_Addr + 2];
        Lora_Para_AT.BandWidth = Lora_State.Rx_Data[Data_Addr + 3];
        Lora_Para_AT.SpreadingFactor = Lora_State.Rx_Data[Data_Addr + 4];
        Lora_Para_AT.CodingRate = Lora_State.Rx_Data[Data_Addr + 5];
        Lora_State.Net_State = Net_Joining; // 未入网 已建立连接
        Lora_State.Wait_ACK = SlaverInNet;
        CusProfile_Send(0x0000, SlaverInNet, Lora_State.Mac, 8, TRUE);
    }
}

/**
 * Cmd_DeviceAnnonce
 * @brief Sub-device declaration after receiving the host's network reply.
 * @author Honokahqh
 * @date 2023-08-05
 */
void Cmd_DeviceAnnonce()
{
    if (Lora_State.isMaster)
        return;
    Lora_State.Net_State = Net_JoinGateWay;
    Lora_State.Wait_ACK = 0;
    Lora_Para_AT.channel = Lora_State.Channel;
    Lora_Para_AT.SAddr = Lora_State.SAddr;
    Lora_Para_AT.PanID = Lora_State.PanID;
    Lora_Para_AT.Net_State = Net_JoinGateWay;
    Lora_Para_AT.NetOpenTime = 0;
    Lora_State_Save();
    delay_ms(100);
    NVIC_SystemReset();
}

/**
 * Cmd_Master_Request_Leave
 * @brief Sub-device receives a request to leave the network.
 * @author Honokahqh
 * @date 2023-08-05
 */
void Cmd_Master_Request_Leave()
{
    if (Lora_State.isMaster)
        return;
    memset(&Lora_State, 0, sizeof(Lora_state_t));
    Lora_Para_AT.SAddr = 0xFFFF;
    Lora_Para_AT.PanID = 0xFFFF;
    Lora_Para_AT.Net_State = 0;
    Lora_Para_AT.NetOpenTime = 0xFF;
    Lora_State_Save();
    Reset_FLAG = 1;
}

/**
 * Cmd_Lora_Change_Para
 * @brief change lora parameter, such as channel, spreading factor, bandwidth
 * @author Honokahqh
 * @date 2023-10-07
 */
void Cmd_Lora_Change_Para()
{
    if (Lora_State.isMaster)
        return;
    Lora_Para_AT.SAddr = (Lora_State.Rx_Data[Data_Addr] << 8) + Lora_State.Rx_Data[Data_Addr + 1];
    Lora_Para_AT.PanID = (Lora_State.Rx_Data[Data_Addr + 2] << 8) + Lora_State.Rx_Data[Data_Addr + 3];
    Lora_Para_AT.channel = Lora_State.Rx_Data[Data_Addr + 4];
    Lora_Para_AT.BandWidth = Lora_State.Rx_Data[Data_Addr + 5];
    Lora_Para_AT.SpreadingFactor = Lora_State.Rx_Data[Data_Addr + 6];
    Lora_Para_AT.CodingRate = Lora_State.Rx_Data[Data_Addr + 7];
    Lora_State_Save();
    Reset_FLAG = 1;
}

void Cmd_Query_Rssi()
{
    uint8_t temp[2];
    temp[0] = Lora_State.Rx_RSSI >> 8;
    temp[1] = Lora_State.Rx_RSSI;
    CusProfile_Send(0x0000, Lora_Query_RSSI_ACK, temp, 2, FALSE);
}
/**
 * PCmd_BeaconRequest
 * @brief Sub-device actively requests to join the network.
 * @author Honokahqh
 * @date 2023-08-05
 */
void PCmd_BeaconRequest()
{
    if (Lora_State.isMaster)
        return;

    Lora_State.Wait_ACK = BeaconRequest;
    Lora_State.ErrTimes = 0;
    Lora_State.ACK_Timeout = 3;
    Lora_State.Net_State = Net_NotJoin; // 未入网 未建立连接
    CusProfile_Send(0x0000, BeaconRequest, Lora_State.Mac, 8, FALSE);
}

/**
 * PCmd_Slaver_Request_Leave
 * @brief Sub-device actively requests to leave the network.
 * @author Honokahqh
 * @date 2023-08-05
 */
void PCmd_Slaver_Request_Leave()
{
    if (Lora_State.isMaster)
        return;
    CusProfile_Send(0x0000, Slaver_Request_Leave, NULL, 0, FALSE);
    memset(&Lora_State, 0, sizeof(Lora_state_t));
    Lora_Para_AT.SAddr = 0xFFFF;
    Lora_Para_AT.PanID = 0xFFFF;
    Lora_Para_AT.Net_State = 0;
    Lora_Para_AT.NetOpenTime = 0xFF;
    Lora_State_Save();
    Reset_FLAG = 1;
}

/**
 * PCmd_HeartBeat
 * @brief Sub-device sends a heartbeat.
 * @author Honokahqh
 * @date 2023-08-05
 */
void PCmd_HeartBeat()
{
    uint8_t temp[6];
    if (Lora_State.isMaster)
    {
        return;
    }
    if (Lora_State.Net_State != Net_JoinGateWay)
    {
        return;
    }
    temp[0] = Device_Type;
#if Lora_Is_APP
    temp[1] = Dev_Version;
#else
    temp[1] = Dev_Version - 100;
#endif
    temp[2] = MBS_ALL[MBS_RoomIR_Index].map->reg[2].data >> 8;
    temp[3] = MBS_ALL[MBS_RoomIR_Index].map->reg[2].data;
    temp[4] = Lora_State.Device_RSSI >> 8;
    temp[5] = Lora_State.Device_RSSI;
    CusProfile_Send(0x0000, HeartBeat, temp, 6, FALSE);
}

/**
 * Cmd_MBS_Cmd_Request
 * @brief Sub-device has received MBS type data, and passes it to the MBS protocol stack for processing after copying.
 * @author Honokahqh
 * @date 2023-08-05
 */
void Cmd_MBS_Cmd_Request()
{
    Debug_B("Receive MBS CMD, devicetype:%d\r\n", Lora_State.Rx_Data[Data_Addr]);

    MBS_Buf._rxLen = Lora_State.Rx_Data[Len_Addr];
    memcpy(MBS_Buf._rxBuff, &Lora_State.Rx_Data[Data_Addr], MBS_Buf._rxLen);
    MBS_CorePoll();
    mb_host.Rx_SAddr = Lora_State.Rx_SAddr;
    memset(MBS_Buf._rxBuff, 0, sizeof(MBS_Buf._rxBuff));
    Debug_B("mbs process Ending\r\n");
    MBS_Buf._rxLen = 0;
}

/**
 * Cmd_MBS_Cmd_Request
 * @brief Sub-device has received MBS type data, and passes it to the MBS protocol stack for processing after copying.
 * @author Honokahqh
 * @date 2023-08-05
 */
void PCmd_MBS_Cmd_ACK(uint8_t *data, uint8_t len)
{
    CusProfile_Send(mb_host.Rx_SAddr, MBS_Cmd_ACK, data, len, FALSE);
}