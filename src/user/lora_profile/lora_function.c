#include "lora_core.h"

uint8_t Reset_FLAG = 0;

/**
 * Get_IDLE_ID
 * @brief ��ȡ��С����ID
 * @author Honokahqh
 * @date 2023-08-05
 */
uint8_t Get_IDLE_ID()
{
    uint8_t i;
    for (i = 0; i < Device_Num_Max; i++)
    {
        if (Associated_devices[i].SAddr == 0)
            break;
    }
    if (i < Device_Num_Max) // �п���ID
        return i;
    return 0xFF;
}

/**
 * Get_Register_Num
 * @brief ��ȡ��������ע����豸����
 * @author Honokahqh
 * @date 2023-08-05
 */
uint8_t Get_Register_Num()
{
    uint8_t i, num;
    num = 0;
    for (i = 0; i < Register_Device_Num_Max; i++)
    {
        if (Register_Device[i].DeviceType != 0)
            num++;
    }
    return num;
}

/**
 * Get_Register_IDLE_ID
 * @brief ��ȡ��С����ID
 * @author Honokahqh
 * @date 2023-08-05
 */
uint8_t Get_Register_IDLE_ID()
{
    uint8_t i;
    for (i = 0; i < Register_Device_Num_Max; i++)
    {
        if (Register_Device[i].DeviceType == 0)
            break;
    }
    if (i < Register_Device_Num_Max) // �п���ID
        return i;
    return 0xFF;
}

/**
 * Compare_ShortAddr
 * @brief �̵�ַƥ��
 * @author Honokahqh
 * @date 2023-08-05
 */
uint8_t Compare_ShortAddr(uint16_t Short_Addr)
{
    uint8_t i;
    for (i = 0; i < Device_Num_Max; i++)
        if (Associated_devices[i].SAddr == Short_Addr && Associated_devices[i].SAddr != 0)
            break;
    if (i < Device_Num_Max)
        return i;
    return 0xFF;
}

/**
 * Compare_MAC
 * @brief MAC��ַƥ��
 * @author Honokahqh
 * @date 2023-08-05
 */
uint8_t Compare_MAC(uint8_t *Mac)
{
    uint8_t i, j, count;
    for (i = 0; i < Device_Num_Max; i++)
    {
        count = 0;
        for (j = 0; j < 8; j++)
        {
            if (Associated_devices[i].Mac[j] == Mac[j])
                count++;
        }
        if (count == 8)
            return i;
    }
    return 0xFF;
}

/**
 * Compare_Register_SAddr
 * @brief ��ַƥ��
 * @author Honokahqh
 * @date 2023-08-05
 */
uint8_t Compare_Register_SAddr(uint16_t Short_Addr)
{
    uint8_t i;
    for (i = 0; i < Register_Device_Num_Max; i++)
        if (Register_Device[i].SAddr == Short_Addr && Register_Device[i].SAddr != 0)
            break;
    if (i < Register_Device_Num_Max)
        return i;
    return 0xFF;
}

/**
 * Compare_Register_MAC
 * @brief MAC��ַƥ��
 * @author Honokahqh
 * @date 2023-08-05
 */
uint8_t Compare_Register_MAC(uint8_t *Mac)
{
    uint8_t i, j, count;
    for (i = 0; i < Register_Device_Num_Max; i++)
    {
        count = 0;
        for (j = 0; j < 8; j++)
        {
            if (Register_Device[i].Mac[j] == Mac[j])
                count++;
        }
        if (count == 8)
            return i;
    }
    return 0xFF;
}

/**
 * XOR_Calculate
 * @brief ���У�����
 * @author Honokahqh
 * @date 2023-08-05
 */
uint8_t XOR_Calculate(uint8_t *data, uint8_t len)
{
    uint8_t x_or = 0, i;
    for (i = 0; i < len; i++)
        x_or = x_or ^ *data++;
    return x_or;
}

/**
 * Set_Sleeptime
 * @brief ����˯��ʱ��
 * @author Honokahqh
 * @date 2023-08-05
 */
void Set_Sleeptime(uint8_t time)
{
    if(Lora_State.Sleep_Timeout < time)
        Lora_State.Sleep_Timeout = time;
}

/**
 * Lora_ReceiveData2State
 * @brief �������ݴ���������
 * @author Honokahqh
 * @date 2023-08-05
 */
void Lora_ReceiveData2State()
{
    Lora_State.Rx_DevType = Lora_State.Rx_Data[DevType_Addr];
    Lora_State.Rx_PanID = (Lora_State.Rx_Data[PanIDH_Addr]<<8) + Lora_State.Rx_Data[PanIDL_Addr];
    Lora_State.Rx_DAddr = (Lora_State.Rx_Data[DAddrH_Addr]<<8) + Lora_State.Rx_Data[DAddrL_Addr];
    Lora_State.Rx_SAddr = (Lora_State.Rx_Data[SAddrH_Addr]<<8) + Lora_State.Rx_Data[SAddrL_Addr];
    Lora_State.Rx_CMD = Lora_State.Rx_Data[Cmd_Addr];
    Lora_State.Rx_PID = Lora_State.Rx_Data[PackID_Addr];
}
