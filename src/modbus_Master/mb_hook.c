#include "mb_include.h"

void mbh_hook_rec01(uint8_t id,uint8_t *data,uint8_t datalen)
{
    uint8_t i;
    if(data[0] != (((mb_host.txbuf[4] << 8) + mb_host.txbuf[5]) + 7) / 8)
    {
        Debug_B("MBH 01H ACK:len error\r\n");
        return;
    }
    for (i = 0; i < MBS_Slaver_Num; i++)
    {
        // Debug_B("id1:%d id2:%d\r\n",MBS_ALL[i].map->mbs_id,id);
        if(MBS_ALL[i].map->mbs_id == id)
        {
            mbs_coildata_syn(MBS_ALL[i].map, (mb_host.txbuf[2] << 8) + mb_host.txbuf[3], &data[1], (mb_host.txbuf[4] << 8) + mb_host.txbuf[5]);
            break;
        }
    }
    if(i >= MBS_Slaver_Num)
        Debug_B("MBH 01H ACK:no such addr\r\n");
}

void mbh_hook_rec03(uint8_t id,uint8_t *data,uint8_t datalen)
{
    uint8_t i;
    if(data[0] != ((mb_host.txbuf[4] << 8) + mb_host.txbuf[5]) * 2)
    {
        Debug_B("MBH 03H ACK:len error\r\n");
        return;
    }
    for (i = 0; i < MBS_Slaver_Num; i++)
    {
        // Debug_B("id1:%d id2:%d\r\n",MBS_ALL[i].map->mbs_id,id);
        if(MBS_ALL[i].map->mbs_id == id)
        {
            // Debug_B("id:%d \r\n",i);
            // Debug_B("reg data syn\r\n");
            mbs_regdata_syn(MBS_ALL[i].map, (mb_host.txbuf[2] << 8) + mb_host.txbuf[3], &data[1], (mb_host.txbuf[4] << 8) + mb_host.txbuf[5]);
            break;
        }
    }
    if(i >= MBS_Slaver_Num)
        Debug_B("MBH 03H ACK:no such addr\r\n");
}


void mbh_hook_rec05(uint8_t id,uint8_t *data,uint8_t datalen)
{
    uint8_t i,temp_data;
    for(i = 0; i < datalen;i++)
    {
        if(data[i] != mb_host.txbuf[i + 2])
            break;
    }
    if(i < datalen)
        Debug_B("MBH 05H ACK:write error\r\n");
    for (i = 0; i < MBS_Slaver_Num; i++)
    {
        // Debug_B("id1:%d id2:%d\r\n",MBS_ALL[i].map->mbs_id,id);
        if(MBS_ALL[i].map->mbs_id == id)
        {
            // Debug_B("reg data syn\r\n");
            if(mb_host.txbuf[4] == 0xFF)
                temp_data = 0xFF;
            else
                temp_data = 0;
            mbs_coildata_syn(MBS_ALL[i].map, (mb_host.txbuf[2] << 8) + mb_host.txbuf[3], &temp_data, 1);
            break;
        }
    }
    if(i >= MBS_Slaver_Num)
        Debug_B("MBH 03H ACK:no such addr\r\n");
}

void mbh_hook_rec06(uint8_t id,uint8_t *data,uint8_t datalen)
{
    uint8_t i;
    for(i = 0; i < datalen;i++)
    {
        if(data[i] != mb_host.txbuf[i + 2])
            break;
    }
    if(i < datalen)
        Debug_B("MBH 06H ACK:write error\r\n");
    for (i = 0; i < MBS_Slaver_Num; i++)
    {
        // Debug_B("id1:%d id2:%d\r\n",MBS_ALL[i].map->mbs_id,id);
        if(MBS_ALL[i].map->mbs_id == id)
        {
            // Debug_B("reg data syn\r\n");
            mbs_regdata_syn(MBS_ALL[i].map, (mb_host.txbuf[2] << 8) + mb_host.txbuf[3], &mb_host.txbuf[4], 1);
            break;
        }
    }
    if(i >= MBS_Slaver_Num)
        Debug_B("MBH 03H ACK:no such addr\r\n");
}

void mbh_hook_rec15(uint8_t id,uint8_t *data,uint8_t datalen)
{
    uint8_t i;
    if(data[3] != mb_host.txbuf[5])
    {
        Debug_B("MBH 0FH ACK:num error\r\n");
        return;
    }
    if(data[0] != mb_host.txbuf[2] || data[1] != mb_host.txbuf[3])
    {
        Debug_B("MBH 0FH ACK:addr error\r\n");
        return;
    }
    for (i = 0; i < MBS_Slaver_Num; i++)
    {
        // Debug_B("id1:%d id2:%d\r\n",MBS_ALL[i].map->mbs_id,id);
        if(MBS_ALL[i].map->mbs_id == id)
        {
            // Debug_B("reg data syn\r\n");
            mbs_coildata_syn(MBS_ALL[i].map, (mb_host.txbuf[2] << 8) + mb_host.txbuf[3], &mb_host.txbuf[7], (mb_host.txbuf[4] << 8) + mb_host.txbuf[5]);
            break;
        }
    }
    if(i >= MBS_Slaver_Num)
        Debug_B("MBH 0FH ACK:no such addr\r\n");
}

void mbh_hook_rec16(uint8_t id,uint8_t *data,uint8_t datalen)
{
    uint8_t i;
    if(data[3] != mb_host.txbuf[5])
    {
        Debug_B("MBH 10H ACK:num error\r\n");
        return;
    }
    if(data[0] != mb_host.txbuf[2] || data[1] != mb_host.txbuf[3])
    {
        Debug_B("MBH 10H ACK:addr error\r\n");
        return;
    }
    for (i = 0; i < MBS_Slaver_Num; i++)
    {
        // Debug_B("id1:%d id2:%d\r\n",MBS_ALL[i].map->mbs_id,id);
        if(MBS_ALL[i].map->mbs_id == id)
        {
            // Debug_B("reg data syn\r\n");
            mbs_regdata_syn(MBS_ALL[i].map, (mb_host.txbuf[2] << 8) + mb_host.txbuf[3], &mb_host.txbuf[7], mb_host.txbuf[5]);
            break;
        }
    }
    if(i >= MBS_Slaver_Num)
        Debug_B("MBH 10H ACK:no such addr\r\n");
}


