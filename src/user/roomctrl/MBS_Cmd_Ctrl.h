#ifndef __MBS_CMD_CTRL_H
#define __MBS_CMD_CTRL_H
#include "APP.h"

#define SCR_Num 4
typedef struct
{
	/* data */
	uint8_t delay[SCR_Num];
	gpio_t *port[SCR_Num];
	uint8_t pin[SCR_Num];
	uint8_t mbs_index[SCR_Num];
} SCR_state_t;
extern volatile SCR_state_t SCR_state;

#define Relay_Num 31 // 其中20个无效

typedef enum
{
    IDLE = 0, // 0
    Choose,   // 1
    Ctrl      // 2
} Mode_t;

typedef struct
{
    uint16_t key1;      // enter
    uint16_t key2;      // up
    uint16_t key3;      // down
    uint16_t key4;      // back
    uint16_t select_id; // 选择的ID
    Mode_t mode;
    uint32_t timeout;
} Key_state_t;
extern Key_state_t Key_state;

extern uint16_t *DeviceControlState[Relay_Num];  // DAC SCR Relay
extern uint16_t *Relay_Index[Relay_Num]; // index指向mbsHoldRegValue[35 + i].pData

void set_dac_output();

void key_scan();

void AirCon_Relay_Auto_Close();
void Relay_Service();

void RGB_Service();

void SCR_Reset();
void SCR_Ctrl();

void Led_display_service();

void mbs_ctrl();

#endif
