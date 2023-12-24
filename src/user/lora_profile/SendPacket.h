#ifndef  _SENDPACKET_H_
#define  _SENDPACKET_H_

#include "APP.h"

/*  cmd datalen data
    1.发送方发送START包,包含通讯参数(有重发)    0x82 13 SF BW CR delay type size[3:0] len num crc
    2.接收方回复是否支持参数,不支持则填入自己支持的参数(有重发) 0x83 13 SF BW CR delay type size[3:0] len num crc
    3.发送方发送参数同意确认    0x84 13 SF BW CR delay type size[3:0] len num crc       -若接收方对某一个参数不同意发送单字节CANCEL 0x18
    4.接收方收到3直接切参数,发送方等待?秒
    5.发送方发送数据包,包含包号和数据 0x85 132 pid_h pid_l data crc
    6.发送方发送End包   0x86 NULL
    7.接收方收到End包后请求丢失/无效的包 0x87 2 pid_h pid_l
    8.发送方发送丢失/无效的包 0x88 132 pid_h pid_l data crc
    9.接收方全部接收完毕 0x89 NULL
*/
#define PACKET_TYPE_WAV     1       // WAV格式音频文件

#define SEND_PACKET_IDLE    0       // 空闲状态，没有传输正在进行
#define SEND_PACKET_Init    1       // 初始化状态,发送起始包,等待ACK
#define SEND_PACKET_Start   2       // 等待?秒开始发送
#define SEND_PACKET_PID     3       // 发送packet中
#define SEND_PACKET_END     4       // 发送END包,等待ACK

#define SEND_PACKET_TIMEOUT 30       // 超时时间，单位S

#define Parameter_Len       11      // 参数长度
typedef struct {

    // 当前状态，可能的值包括等待开始、正在传输、等待应答、传输完成等
    int state;
    
    // 当前正在处理的全部参数
    uint8_t parameter[Parameter_Len];

    // 当前正在处理的文件类型
    uint8_t packet_type;
    
    // 当前正在处理的文件大小
    unsigned long filesize;
    
    // 文件数据指针
    uint8_t *filepointer;
    // 当前块号
    unsigned int packet_number;
    
    // 总共需要发送/接收的块数
    unsigned int packet_total;

    // 每包之间发送延迟
    unsigned int delay;
    
    // 发送或接收的数据缓冲区
    char buffer[20];
    
    // 错误计数，可用于记录重发的次数
    int error_count;

    // 超时时间
    unsigned int timeout;
} send_packet_state_t;
extern send_packet_state_t send_packet_state;

int Profile_SP_init(uint8_t *data, uint8_t len, uint8_t *file);
int Profile_SP_Period(uint8_t rx_cmd, uint8_t *rx_data, uint8_t rx_len, uint8_t *tx_data, uint8_t *tx_len);

#endif