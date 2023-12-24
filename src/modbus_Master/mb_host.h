#ifndef __MB_HOST_H
#define __MB_HOST_H

#include "mb_include.h"

#define MBS_STATE_IDLE      0X00    // ����
#define MBS_STATE_WAIT_ACK  0X01    // �ȴ��ӻ��ظ�

#define MBS_TIMEOUT         300     // ��ʱʱ��
#define MBS_ERRTIME_MAX     0       // �ط����� 

typedef struct 
{
    /* data */
    uint8_t state;
    uint8_t cmd;
    uint8_t addr;
    uint8_t ErrTimes;
    uint32_t timeout;

    uint8_t txbuf[256];
    uint16_t txlen;

    /* ����ָ�� */
    uint8_t Read_Verison; // ���������ȫ�����豸�汾��
    uint8_t isSelfcmd;
    uint16_t Rx_SAddr;
}mb_host_t;
extern mb_host_t mb_host;


void mbh_init(uint32_t baud);
void mbh_poll(void);
void MBS_Master_Period_100ms(void);
uint8_t mbh_exec(uint8_t *pframe,uint8_t len);

void mbs_coildata_syn(MBS_Map_t *MBS_Map, uint16_t addr, uint8_t *data, uint8_t len);
void mbs_regdata_syn(MBS_Map_t *MBS_Map, uint16_t addr, uint8_t *data, uint8_t len);

#endif

