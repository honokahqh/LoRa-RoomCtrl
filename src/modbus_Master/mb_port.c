#include "mb_include.h"
#include "lora_core.h"

/**
 *   调mbs_map_init()初始化map
 *   调mbs_poll()接收超时/待发送数据发送
 *   调mb_port_rec_data()处理接收到的数据
 */
extern uint32_t Sys_ms;
MBS_ALL_Map_t MBS_ALL[MBS_Slaver_Num];

/**
 * mb_port_send_data
 * @brief 本机485向从机发送数据
 * @author Honokahqh
 * @date 2023-08-05
 */
void mb_port_send_data(uint8_t *data, uint16_t datalen)
{
    if (datalen > 256)
    {
        Debug_B("mb_port_send_data datalen > 256\r\n");
        return;
    }
    memcpy(mb_host.txbuf, data, datalen);
    mb_host.txlen = datalen;
    mb_host.state = MBS_STATE_WAIT_ACK;
    mb_host.addr = data[0];
    mb_host.cmd = data[1];
    // 发送数据
    MBS_Physical2SendBuff(data, datalen);
    // 发送完成
    mb_host.timeout = mb_port_get_tick();
}

uint32_t mb_port_get_tick(void)
{
    return Sys_ms;
}

/* 红外 */
MBS_Map_t MBS_IR;
MBS_Reg_t mbs_reg_ir[MBS_IR_Aircon_RegNum];
void MBS_IR_Map_Init()
{
    uint16_t i;
    MBS_IR.reg = mbs_reg_ir;

    for (i = 0; i < 2; i++) // MBS地址和版本号
    {
        MBS_IR.reg[i].addr = i + 59000;
        MBS_IR.reg[i].data = 0;
    }
    MBS_IR.reg_num = MBS_IR_Aircon_RegNum;
    MBS_IR.mbs_id = IR_Aircon_ADDR;
    MBS_IR.reg[MBS_IR_Aircon_Ver].data = IR_Aircon_ADDR;
    MBS_ALL[MBS_IR_Aircon_Index].map = &MBS_IR;
}

/* 房态灯 */
MBS_Reg_t mbs_reg_roomlight[MBS_RoomLight_RegNum];
MBS_Map_t MBS_RoomLight;
void MBS_RoomLight_Map_Init()
{
    uint16_t i;
    MBS_RoomLight.reg = mbs_reg_roomlight;
    for (i = 0; i < 2; i++) // MBS地址和版本号
    {
        MBS_RoomLight.reg[i].addr = i + 59000;
        MBS_RoomLight.reg[i].data = 0;
    }
    MBS_RoomLight.reg_num = MBS_RoomLight_RegNum;
    MBS_RoomLight.mbs_id = RoomLight_ADDR;
    MBS_RoomLight.reg[MBS_RoomLight_Ver].data = RoomLight_ADDR;
    MBS_ALL[MBS_RoomLight_Index].map = &MBS_RoomLight;
}

/* 人体红外 */
MBS_Reg_t mbs_reg_irdetect[MBS_IR_Detect_RegNum];
MBS_Coil_t mbs_coil_irdetect[MBS_IR_Detect_CoilNum];
MBS_Map_t MBS_IR_Detect;
void MBS_IR_Detect_Map_Init()
{
    uint16_t i;
    MBS_IR_Detect.reg = mbs_reg_irdetect;
    MBS_IR_Detect.coil = mbs_coil_irdetect;
    for (i = 0; i < 2; i++) // MBS地址和版本号
    {
        MBS_IR_Detect.reg[i].addr = i + 59000;
        MBS_IR_Detect.reg[i].data = 0;
    }
    for (i = 2; i < 3; i++) // MBS地址和版本号
    {
        MBS_IR_Detect.reg[i].addr = i + 45000;
        MBS_IR_Detect.reg[i].data = 0;
    }
    for (i = 0; i < 1; i++)
    {
        MBS_IR_Detect.coil[i].addr = i + 10000;
        MBS_IR_Detect.coil[i].data = 0;
    }
    MBS_IR_Detect.reg_num = MBS_IR_Detect_RegNum;
    MBS_IR_Detect.coil_num = MBS_IR_Detect_CoilNum;
    MBS_IR_Detect.mbs_id = IR_Detect_ADDR;
    MBS_IR_Detect.reg[MBS_IR_Detect_Ver].data = IR_Detect_ADDR;
    MBS_ALL[MBS_IR_Detect_Index].map = &MBS_IR_Detect;
}

/* 人体红外2 */
MBS_Reg_t mbs_reg_irdetect2[MBS_IR_Detect_RegNum];
MBS_Coil_t mbs_coil_irdetect2[MBS_IR_Detect_CoilNum];
MBS_Map_t MBS_IR_Detect2;
void MBS_IR_Detect2_Map_Init()
{
    uint16_t i;
    MBS_IR_Detect2.reg = mbs_reg_irdetect2;
    MBS_IR_Detect2.coil = mbs_coil_irdetect2;
    for (i = 0; i < 2; i++) // MBS地址和版本号
    {
        MBS_IR_Detect2.reg[i].addr = i + 59000;
        MBS_IR_Detect2.reg[i].data = 0;
    }
    for (i = 2; i < 3; i++) // MBS地址和版本号
    {
        MBS_IR_Detect2.reg[i].addr = i + 45000;
        MBS_IR_Detect2.reg[i].data = 0;
    }
    for (i = 0; i < 1; i++)
    {
        MBS_IR_Detect2.coil[i].addr = i + 10000;
        MBS_IR_Detect2.coil[i].data = 0;
    }
    MBS_IR_Detect2.reg_num = MBS_IR_Detect_RegNum;
    MBS_IR_Detect2.coil_num = MBS_IR_Detect_CoilNum;
    MBS_IR_Detect2.mbs_id = IR_Detect2_ADDR;
    MBS_IR_Detect2.reg[MBS_IR_Detect_Ver].data = IR_Detect2_ADDR;
    MBS_ALL[MBS_IR_Detect2_Index].map = &MBS_IR_Detect2;
}

/* 外部开关 */
MBS_Reg_t mbs_reg_switch[MBS_Switch_RegNum];
MBS_Coil_t mbs_coil_switch[MBS_Switch_CoilNum];
MBS_Map_t MBS_Switch;
void MBS_Switch_Map_Init()
{
    uint16_t i;
    MBS_Switch.reg = mbs_reg_switch;
    MBS_Switch.coil = mbs_coil_switch;
    for (i = 0; i < 2; i++) // MBS地址和版本号
    {
        MBS_Switch.reg[i].addr = i + 59000;
        MBS_Switch.reg[i].data = 0;
    }
    for (i = 0; i < 4; i++)
    {
        MBS_Switch.coil[i].addr = i + 12000;
        MBS_Switch.coil[i].data = 0;
    }
    MBS_Switch.reg_num = MBS_Switch_RegNum;
    MBS_Switch.coil_num = MBS_Switch_CoilNum;
    MBS_Switch.mbs_id = Switch_ADDR;
    MBS_Switch.reg[MBS_Switch_Ver].data = Switch_ADDR;
    MBS_ALL[MBS_Switch_Index].map = &MBS_Switch;
}

/* 蓝牙 */
MBS_Reg_t mbs_reg_ble[MBS_BLE_RegNum];
MBS_Coil_t mbs_coil_ble[MBS_BLE_CoilNum];
MBS_Map_t MBS_ble;
void MBS_BLE_Map_Init()
{
    uint16_t i;
    MBS_ble.reg = mbs_reg_ble;
    MBS_ble.coil = mbs_coil_ble;
    for (i = 0; i < 2; i++) // MBS地址和版本号
    {
        MBS_ble.reg[i].addr = i + 59000;
        MBS_ble.reg[i].data = 0;
    }
    for (i = 2; i < 3; i++) // MBS地址和版本号
    {
        MBS_ble.reg[i].addr = 51008;
        MBS_ble.reg[i].data = 0;
    }
    for (i = 0; i < 1; i++)
    {
        MBS_ble.coil[i].addr = i + 12009;
        MBS_ble.coil[i].data = 0;
    }
    MBS_ble.reg_num = MBS_BLE_RegNum;
    MBS_ble.coil_num = MBS_BLE_CoilNum;
    MBS_ble.mbs_id = BLE_ADDR;
    MBS_ble.reg[MBS_BLE_Ver].data = BLE_ADDR;
    MBS_ALL[MBS_BLE_Index].map = &MBS_ble;
}

/* 中控红外 */
MBS_Reg_t mbs_reg_roomir[MBS_RoomIR_RegNum];
MBS_Coil_t mbs_coil_roomir[MBS_RoomIR_CoilNum];
MBS_Map_t MBS_RoomIR;
void MBS_RoomIR_Map_Init()
{
    uint16 i;
    MBS_RoomIR.reg = mbs_reg_roomir;
    MBS_RoomIR.coil = mbs_coil_roomir;

    MBS_RoomIR.reg[2].addr = 50000; // 温度
    MBS_RoomIR.reg[2].data = 0xFFFF;

    for (i = 0; i < 2; i++) // MBS地址和版本号
    {
        MBS_RoomIR.reg[i].addr = i + 59000;
        MBS_RoomIR.reg[i].data = 0;
    }
    for (i = 0; i < 31; i++) // MBS地址和版本号
    {
        MBS_RoomIR.coil[i].addr = i + 10000;
        MBS_RoomIR.coil[i].data = 0;
    }
    MBS_RoomIR.reg_num = MBS_RoomIR_RegNum;
    MBS_RoomIR.coil_num = MBS_RoomIR_CoilNum;
    MBS_RoomIR.mbs_id = RoomIR_ADDR;
    MBS_RoomIR.reg[MBS_RoomIR_Ver].data = RoomIR_ADDR;
    MBS_ALL[MBS_RoomIR_Index].map = &MBS_RoomIR;
}
/* mbs主机初始化 */
void mbh_map_init()
{
    mb_host.state = MBS_STATE_IDLE;
    MBS_IR_Map_Init();
    MBS_BLE_Map_Init();
    MBS_RoomLight_Map_Init();
    MBS_IR_Detect_Map_Init();
    MBS_IR_Detect2_Map_Init();
    MBS_Switch_Map_Init();
    MBS_RoomIR_Map_Init();
}