#include "SendPacket.h"
#include "lora_core.h"

send_packet_state_t send_packet_state;

/**
 * UpdateCRC16
 * @brief ymodem CRC16校验算法
 * @author Honokahqh
 * @date 2023-08-05
 */
static uint16_t UpdateCRC16(uint16_t crc_in, uint8_t byte)
{
    uint32_t crc = crc_in;
    uint32_t in = byte | 0x100;
    do
    {
        crc <<= 1;
        in <<= 1;
        if (in & 0x100)
            ++crc;
        if (crc & 0x10000)
            crc ^= 0x1021;
    } while (!(in & 0x10000));

    return crc & 0xffffu;
}

/**
 * YmodemCRC
 * @brief ymodem CRC16校验算法
 * @author Honokahqh
 * @date 2023-08-05
 */
static uint16_t YmodemCRC(uint8_t *data, uint32_t len)
{
    uint32_t crc = 0;
    uint8_t *dataEnd;
    dataEnd = data + len;

    while (data < dataEnd)
        crc = UpdateCRC16(crc, *data++);

    crc = UpdateCRC16(crc, 0);
    crc = UpdateCRC16(crc, 0);

    return crc & 0xffff;
}

/**
 * ymodem_init
 * @brief 初始化ymodem
 * @author Honokahqh
 * @date 2023-08-05
 */
int Profile_SP_init(uint8_t *data, uint8_t len, uint8_t *file)
{
    if (len != Parameter_Len)
    {
        Debug_B("SP_init len error\r\n");
        return -1;
    }
    if (data[0] < 5 || data[0] > 12)
    {
        Debug_B("SP_init SF error\r\n");
        return -1;
    }
    if (data[1] < 0 || data[1] > 2)
    {
        Debug_B("SP_init BW error\r\n");
        return -1;
    }
    if (data[2] < 1 || data[2] > 2)
    {
        Debug_B("SP_init CR error\r\n");
        return -1;
    }
    if (data[4] < 1 || data[4] > 1)
    {
        Debug_B("SP_init type error\r\n");
        return -1;
    }
    send_packet_state.filesize = (data[5] << 24) + (data[6] << 16) + (data[7] << 8) + data[8];
    if (send_packet_state.filesize > 64 * 1024)
    {
        Debug_B("SP_init size error\r\n");
        return -1;
    }
    // 初始化
    memset(&send_packet_state, 0, sizeof(send_packet_state_t));
    send_packet_state.state = SEND_PACKET_Init;
    memcpy(send_packet_state.parameter, data, len);
    send_packet_state.packet_type = data[4];
    send_packet_state.packet_total = (send_packet_state.filesize + 128 - 1) / 128;
    send_packet_state.packet_number = 0;
    send_packet_state.delay = data[3];
    send_packet_state.filepointer = file;
    send_packet_state.timeout = SEND_PACKET_TIMEOUT;
	return 1;
}

#define SP_Start_delay 2     // 开始延迟 s
#define SP_Function_Period 5 // 调用周期 ms

int Profile_SP_Period(uint8_t rx_cmd, uint8_t *rx_data, uint8_t rx_len, uint8_t *tx_data, uint8_t *tx_len)
{
    static uint32_t time_count = 0, Start_delay = 0;
    switch (send_packet_state.state)
    {
    case SEND_PACKET_Init: // 1s超时后发送data
        if (time_count * SP_Function_Period > 1000 && rx_len == 0)
        {
            time_count = 0;
            memcpy(tx_data, send_packet_state.parameter, Parameter_Len);
            *tx_len = Parameter_Len + 2;
            uint16_t crc = YmodemCRC(tx_data, 11);
            tx_data[Parameter_Len] = crc >> 8;
            tx_data[Parameter_Len + 1] = crc & 0xff;
            send_packet_state.error_count++;
            return Lora_SendPacket_Init;
        }
        else // 有ACK数据
        {
            if (rx_cmd == Lora_SendPacket_Init_ACK)
            {
                for(uint8_t i = 0;i < Parameter_Len;i++)
                {
                    if(rx_data[i] != send_packet_state.parameter[i])
                    {
                        memset(&send_packet_state, 0, sizeof(send_packet_state_t));
                        return -1;
                    }
                }
                time_count = 0;
                memcpy(tx_data, send_packet_state.parameter, Parameter_Len);
                *tx_len = Parameter_Len;
                send_packet_state.error_count++;
                Start_delay = 0;
                send_packet_state.state = SEND_PACKET_Start;
                Debug_B("SP_Init ACK\r\n");
                return Lora_SendPacket_Start;
            }
            else
            {
                Debug_B("SP_Init ACK error\r\n");
            }
        }
        break;
    case SEND_PACKET_Start:
        if(Start_delay > SP_Start_delay)
        {
            send_packet_state.state = SEND_PACKET_PID;
            Debug_B("SP_Start change lora parameter\r\n");
            return 0x55;
        }
        if (time_count * SP_Function_Period > 1000 && rx_len == 0)
        {
            time_count = 0;
            memcpy(tx_data, send_packet_state.parameter, Parameter_Len);
            *tx_len = Parameter_Len;
            Start_delay++;
            return Lora_SendPacket_Start;
        }
    case SEND_PACKET_PID:
        if(time_count * SP_Function_Period > send_packet_state.delay)
        {
            time_count = 0;
            send_packet_state.packet_number++;
            if(send_packet_state.packet_number > send_packet_state.packet_total)
            {
                send_packet_state.state = SEND_PACKET_END;
                Debug_B("SP_PID change lora parameter\r\n");
                return Lora_SendPacket_End1;
            }
            else
            {
                tx_data[0] = send_packet_state.packet_number >> 8;
                tx_data[1] = send_packet_state.packet_number & 0xff;
                memcpy(tx_data + 2, send_packet_state.filepointer + (send_packet_state.packet_number - 1) * 128, 128);
                uint16_t crc = YmodemCRC(tx_data + 2, 128);
                tx_data[130] = crc >> 8;
                tx_data[131] = crc & 0xff;
                *tx_len = 132;
                send_packet_state.state = SEND_PACKET_PID;
                return Lora_SendPacket_ing;
            }
        }
    case SEND_PACKET_END:
        if (rx_cmd == Lora_NeedPacket)
        {
			uint16_t packet_num = (rx_data[0] << 8) + rx_data[1];
            if( packet_num <= send_packet_state.packet_total)
            {
                send_packet_state.packet_number = packet_num;
                tx_data[0] = send_packet_state.packet_number >> 8;
                tx_data[1] = send_packet_state.packet_number & 0xff;
                memcpy(tx_data + 2, send_packet_state.filepointer + (packet_num - 1) * 128, 128);
                uint16_t crc = YmodemCRC(tx_data + 2, 128);
                tx_data[130] = crc >> 8;
                tx_data[131] = crc & 0xff;
                *tx_len = 132;
                return Lora_NeedPacket_ACK;
            }
            else
            {
                Debug_B("SP_NeedPacket ID error\r\n");
                memset(&send_packet_state, 0, sizeof(send_packet_state_t));
                return -1;
            }
        }
        if (rx_cmd == Lora_SendPacket_End2)
        {
            memset(&send_packet_state, 0, sizeof(send_packet_state_t));
            Debug_B("SP_End\r\n");
            return 0;
        }
        else
        {
            Debug_B("SP_End ACK error\r\n");
        }
        break;
    default:
        break;
    }

    if (send_packet_state.error_count > 5)
    {
        memset(&send_packet_state, 0, sizeof(send_packet_state_t));
        return -1;
    }
    time_count++;
	return 0;
}