#ifndef __MB_HOST_H
#define __MB_HOST_H

#include "mb_include.h"

#define MBS_STATE_IDLE      0X00    // 空闲
#define MBS_STATE_WAIT_ACK  0X01    // 等待从机回复

#define MBS_TIMEOUT         300     // 超时时间
#define MBS_ERRTIME_MAX     0       // 重发次数 

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

    /* 特殊指令 */
    uint8_t Read_Verison; // 主机请求读全部子设备版本号
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

