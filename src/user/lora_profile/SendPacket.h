#ifndef  _SENDPACKET_H_
#define  _SENDPACKET_H_

#include "APP.h"

/*  cmd datalen data
    1.���ͷ�����START��,����ͨѶ����(���ط�)    0x82 13 SF BW CR delay type size[3:0] len num crc
    2.���շ��ظ��Ƿ�֧�ֲ���,��֧���������Լ�֧�ֵĲ���(���ط�) 0x83 13 SF BW CR delay type size[3:0] len num crc
    3.���ͷ����Ͳ���ͬ��ȷ��    0x84 13 SF BW CR delay type size[3:0] len num crc       -�����շ���ĳһ��������ͬ�ⷢ�͵��ֽ�CANCEL 0x18
    4.���շ��յ�3ֱ���в���,���ͷ��ȴ�?��
    5.���ͷ��������ݰ�,�������ź����� 0x85 132 pid_h pid_l data crc
    6.���ͷ�����End��   0x86 NULL
    7.���շ��յ�End��������ʧ/��Ч�İ� 0x87 2 pid_h pid_l
    8.���ͷ����Ͷ�ʧ/��Ч�İ� 0x88 132 pid_h pid_l data crc
    9.���շ�ȫ��������� 0x89 NULL
*/
#define PACKET_TYPE_WAV     1       // WAV��ʽ��Ƶ�ļ�

#define SEND_PACKET_IDLE    0       // ����״̬��û�д������ڽ���
#define SEND_PACKET_Init    1       // ��ʼ��״̬,������ʼ��,�ȴ�ACK
#define SEND_PACKET_Start   2       // �ȴ�?�뿪ʼ����
#define SEND_PACKET_PID     3       // ����packet��
#define SEND_PACKET_END     4       // ����END��,�ȴ�ACK

#define SEND_PACKET_TIMEOUT 30       // ��ʱʱ�䣬��λS

#define Parameter_Len       11      // ��������
typedef struct {

    // ��ǰ״̬�����ܵ�ֵ�����ȴ���ʼ�����ڴ��䡢�ȴ�Ӧ�𡢴�����ɵ�
    int state;
    
    // ��ǰ���ڴ����ȫ������
    uint8_t parameter[Parameter_Len];

    // ��ǰ���ڴ�����ļ�����
    uint8_t packet_type;
    
    // ��ǰ���ڴ�����ļ���С
    unsigned long filesize;
    
    // �ļ�����ָ��
    uint8_t *filepointer;
    // ��ǰ���
    unsigned int packet_number;
    
    // �ܹ���Ҫ����/���յĿ���
    unsigned int packet_total;

    // ÿ��֮�䷢���ӳ�
    unsigned int delay;
    
    // ���ͻ���յ����ݻ�����
    char buffer[20];
    
    // ��������������ڼ�¼�ط��Ĵ���
    int error_count;

    // ��ʱʱ��
    unsigned int timeout;
} send_packet_state_t;
extern send_packet_state_t send_packet_state;

int Profile_SP_init(uint8_t *data, uint8_t len, uint8_t *file);
int Profile_SP_Period(uint8_t rx_cmd, uint8_t *rx_data, uint8_t rx_len, uint8_t *tx_data, uint8_t *tx_len);

#endif