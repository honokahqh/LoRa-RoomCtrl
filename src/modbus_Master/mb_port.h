#ifndef __MB_PORT_H
#define __MB_PORT_H

#include "mb_include.h"

#define MBS_Slaver_Num      7

typedef struct
{
    uint16_t addr; // ��Ȧ��ַ
    uint8_t data;  // ��Ȧ����
} MBS_Coil_t;

typedef struct
{
    uint16_t addr; // �Ĵ�����ַ
    uint16_t data; // �Ĵ�������
} MBS_Reg_t;


typedef struct
{
    MBS_Coil_t *coil;
    MBS_Reg_t *reg;
    uint16_t reg_num;
    uint16_t coil_num;
    uint8_t mbs_id;

    /* �п�����mbs��־ */
    uint8_t isAlive;    // ��ǰ״̬    
    uint8_t isAlive_Last; // ��һ�ε�״̬
    uint8_t err_count;
} MBS_Map_t;

typedef struct 
{
    MBS_Map_t *map;
} MBS_ALL_Map_t;
extern MBS_ALL_Map_t MBS_ALL[MBS_Slaver_Num];

/* ���� */
#define IR_Aircon_ADDR       0x6F
#define MBS_IR_Aircon_RegNum  2 
#define MBS_IR_Aircon_Ver    0
#define MBS_IR_Aircon_Index  0
extern MBS_Reg_t mbs_reg_ir[MBS_IR_Aircon_RegNum];
extern MBS_Map_t MBS_IR;

/* ����ģ�� */
// coil:12009 0:δ���� 1:������ 
#define BLE_ADDR  120
#define MBS_BLE_RegNum  3
#define MBS_BLE_CoilNum  1
#define MBS_BLE_Ver    0
#define MBS_BLE_Index  1
extern MBS_Reg_t mbs_reg_ble[MBS_BLE_RegNum];
extern MBS_Coil_t mbs_coil_ble[MBS_BLE_CoilNum];
extern MBS_Map_t MBS_ble;

/* ��̬�� */
#define RoomLight_ADDR 0x28
#define MBS_RoomLight_RegNum  2
#define MBS_RoomLight_Ver    0
#define MBS_RoomLight_Index  2
extern MBS_Reg_t mbs_reg_roomlight[MBS_RoomLight_RegNum];
extern MBS_Map_t MBS_RoomLight;

/* ������� */
// coil:10000 0:���� 1:���� 
// reg: 45000 �ӳ�ʱ��
#define IR_Detect_ADDR  0x5A
#define MBS_IR_Detect_RegNum  3
#define MBS_IR_Detect_CoilNum  1
#define MBS_IR_Detect_Ver    0
#define MBS_IR_Detect_Index  3
extern MBS_Reg_t mbs_reg_irdetect[MBS_IR_Detect_RegNum];
extern MBS_Coil_t mbs_coil_irdetect[MBS_IR_Detect_CoilNum];

/* ������� */
// coil:10000 0:���� 1:���� 
// reg: 45000 �ӳ�ʱ��
#define IR_Detect2_ADDR  0x5B
#define MBS_IR_Detect2_Index  4
extern MBS_Reg_t mbs_reg_irdetect[MBS_IR_Detect_RegNum];
extern MBS_Coil_t mbs_coil_irdetect[MBS_IR_Detect_CoilNum];

/* �ⲿ���� */
// coil:12000 ����1:0:�� 1:��   12001:����2:0:�� 1:��
#define Switch_ADDR  45
#define MBS_Switch_RegNum  2
#define MBS_Switch_CoilNum  4
#define MBS_Switch_Ver    0
#define MBS_Switch_Index  5
extern MBS_Reg_t mbs_reg_switch[MBS_Switch_RegNum];
extern MBS_Coil_t mbs_coil_switch[MBS_Switch_CoilNum];
extern MBS_Map_t MBS_Switch;

/* �пغ��� */
// coil:10000~10030
// reg:50000
#define RoomIR_ADDR  0x7D
#define MBS_RoomIR_RegNum  3
#define MBS_RoomIR_CoilNum  31
#define MBS_RoomIR_Ver    0
#define MBS_RoomIR_Index  6
extern MBS_Reg_t mbs_reg_roomir[MBS_RoomIR_RegNum];
extern MBS_Coil_t mbs_coil_roomir[MBS_RoomIR_CoilNum];
extern MBS_Map_t MBS_RoomIR;


void mbh_map_init();
void mb_port_send_data(uint8_t *data, uint16_t datalen);
uint8_t mb_port_rec_data(uint8_t *data, uint8_t datalen);
uint32_t mb_port_get_tick(void);

void MBS_01H(uint8_t ID, uint16_t addr, uint16_t num);
void MBS_03H(uint8_t ID, uint16_t addr, uint16_t num);
void MBS_05H(uint8_t ID, uint16_t addr, uint16_t value);
void MBS_06H(uint8_t ID, uint16_t addr, uint16_t value);
void MBS_0FH(uint8_t ID, uint16_t addr, uint16_t num, uint8_t *data);
void MBS_10H(uint8_t ID, uint16_t addr, uint16_t num, uint16_t *data);

#endif
