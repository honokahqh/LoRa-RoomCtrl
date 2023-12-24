#include "lora_core.h"

associated_devices_t Associated_devices[Device_Num_Max];
register_device_t Register_Device[Register_Device_Num_Max];
Lora_state_t Lora_State;

/**
 * CusProfile_Send
 * @brief Lora���ݷ���
 * @author Honokahqh
 * @date 2023-08-05
 */
uint8_t PackageID; // �������͵�Package��ACK��Package
void CusProfile_Send(uint16_t DAddr, uint8_t cmd, uint8_t *data, uint8_t len, uint8_t isAck)
{
    uint8_t i;
    Lora_State.Tx_Data[DevType_Addr] = Device_Type;          // ����
    Lora_State.Tx_Data[PanIDH_Addr] = Lora_State.PanID >> 8; // ��������
    Lora_State.Tx_Data[PanIDL_Addr] = Lora_State.PanID;      // ��������
    if (Lora_State.isMaster == true)
    {
        Lora_State.Tx_Data[SAddrH_Addr] = 0; // ���������ַ
        Lora_State.Tx_Data[SAddrL_Addr] = 0; // ���������ַ
    }
    else if (Lora_State.Net_State == Net_NotJoin)
    {
        Lora_State.Tx_Data[SAddrH_Addr] = Lora_State.chip_ID >> 8; // ���������ַ
        Lora_State.Tx_Data[SAddrL_Addr] = Lora_State.chip_ID;      // ���������ַ
    }
    else
    {
        Lora_State.Tx_Data[SAddrH_Addr] = Lora_State.SAddr >> 8; // ���������ַ
        Lora_State.Tx_Data[SAddrL_Addr] = Lora_State.SAddr;      // ���������ַ
    }
    Lora_State.Tx_Data[DAddrH_Addr] = DAddr >> 8; // Ŀ�������ַ
    Lora_State.Tx_Data[DAddrL_Addr] = DAddr;      // Ŀ�������ַ
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
 * @brief Lora���ݽ��մ���-����Э��������ݲ�������Ӧ������������
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
    /* ���У���Ƿ�һ�� */
    if (XOR_Calculate(Lora_State.Rx_Data, Lora_State.Rx_Len - 1) != Lora_State.Rx_Data[Lora_State.Rx_Len - 1])
        return;
    /* ��֤�̵�ַ step:1 */
    if (Lora_State.Rx_SAddr != BoardCast && Lora_State.Rx_DAddr != Lora_State.SAddr)
        return;
    /* ��֤PanID step:1 */
    if (Lora_State.Rx_PanID != Lora_State.PanID && Lora_State.Rx_PanID != BoardCast && Lora_State.PanID != BoardCast)
        return;
    /* �����յ�BeaconRequest����ע��ģʽʱ�������̵�ַ��֤ step2 */
    if (Lora_State.isMaster && Lora_State.Rx_CMD == BeaconRequest && Lora_State.NetOpenTime)
    {
        goto DataProcess;
    }
#if Lora_Register_Enable
    /* ע��flagʹ��ʱ,��ע���豸��Ϣ������Register_Device�ڶ���AS�� */
    if (Compare_Register_SAddr(Lora_State.Rx_SAddr) != 0xFF && Lora_State.Rx_CMD == SlaverInNet) // ��ע���б���
        goto DataProcess;
#endif
    /* �̵�ַ��֤ step2 */
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
 * @brief �ӻ�ÿ��״̬���´���
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
    /* δ����״̬�������� */
    if (Lora_State.Wait_ACK == 0 && Lora_State.Net_State == Net_NotJoin && Lora_State.NetOpenTime)
    {
        // Debug_B("BeaconRequest\r\n");
        PCmd_BeaconRequest();
    }
    /* ACK��ʱ */
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
                case BeaconRequest:          // ����ʧ��
                    Lora_State.ErrTimes = 0; // ���ô��� ��������
                    break;
                case SlaverInNet: // ����ʧ��
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
    { // ע��ģʽ����ʱ
        Lora_State.NetOpenTime--;
        if (Lora_State.NetOpenTime == 0)
        {
            Lora_Para_AT.NetOpenTime = 0;
            Lora_State_Save();
            delay_ms(100);
            NVIC_SystemReset();
        }
    }
    /* �¶� */
    Temperature_Period++;
    if (MBS_ALL[MBS_RoomIR_Index].map->isAlive && Lora_State.Net_State == Net_JoinGateWay && ymodem_session.state == YMODEM_STATE_IDLE && Temperature_Period >= (30 + (Lora_State.SAddr % 30)))
    {
        Temperature_Period = 0;
        PCmd_RoomIR_Update();
        return; // ��֤�������ݰ�1sֻ��һ��
    }
    /* ���� */
    HeartBeat_Period++;
    if (HeartBeat_Period > (100 + (Lora_State.SAddr % 100)) && Lora_State.Net_State == Net_JoinGateWay && ymodem_session.state == YMODEM_STATE_IDLE)
    {
        HeartBeat_Period = 0;
        PCmd_HeartBeat();
        return; // ��֤�������ݰ�1sֻ��һ��
    }
    /* ���豸���� */
    Slaver_Period++;
    if (Slaver_Period > (100 + (Lora_State.SAddr % 100)) && Lora_State.Net_State == Net_JoinGateWay && ymodem_session.state == YMODEM_STATE_IDLE)
    {
        Slaver_Period = 0;
        PCmd_Query_SubVersion_ACK();
        return; // ��֤�������ݰ�1sֻ��һ��
    }
}

/**
 * Master_Period_1s
 * @brief ����ÿ��״̬���´���
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
    { // ע���豸��ʱ���
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
