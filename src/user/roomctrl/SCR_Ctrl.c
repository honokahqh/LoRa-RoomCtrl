#include "MBS_Cmd_Ctrl.h"

#if ROOM_TYPE
#define SCR_MIN_DELAY 5
#define SCR_MAX_DELAY 95

volatile SCR_state_t SCR_state = {
    .delay = {0},
    .port = {SCR_1_PORT, SCR_2_PORT, SCR_3_PORT, SCR_4_PORT},
    .pin = {SCR_1_PIN, SCR_2_PIN, SCR_3_PIN, SCR_4_PIN},
    .mbs_index = {Reg_SCR_1, Reg_SCR_2, Reg_SCR_3, Reg_SCR_4}};
static uint32_t closeDelayCounter;

static uint16_t scr_data_last[4] = {0};
// 过零检测时调用 10ms左右
void SCR_Reset()
{
    uint8_t i = 0;
    static uint8_t count_delay = 0;
    
    count_delay++;
    if (count_delay > 10)
    {
        count_delay = 0;
        for (i = 0; i < SCR_Num; i++)
        {
            if (scr_data_last[i] < mbsHoldRegValue[SCR_state.mbs_index[i]].pData)
                scr_data_last[i]++;
            else if (scr_data_last[i] > mbsHoldRegValue[SCR_state.mbs_index[i]].pData)
                scr_data_last[i]--;
        }
    }
    if (mbsCoilValue[Coil_MainSwitch].pData)
    {
        for (i = 0; i < SCR_Num; i++)
        {
            if (scr_data_last[i] >= SCR_MAX_DELAY)
                SCR_state.delay[i] = 1;
            else if (scr_data_last[i] < SCR_MIN_DELAY)
                SCR_state.delay[i] = 0;
            else
                SCR_state.delay[i] = SCR_MAX_DELAY - scr_data_last[i];
        }
        closeDelayCounter = SCR_MAX_DELAY;
    }
	else
		memset(scr_data_last, 0,sizeof(scr_data_last));
}

// 100us定时器调用
void SCR_Ctrl()
{
    uint8_t i = 0;
    for (i = 0; i < SCR_Num; i++)
    {
        if (SCR_state.delay[i] > 0)
        {
            SCR_state.delay[i]--;
            if (SCR_state.delay[i] == 0)
            {
                gpio_write(SCR_state.port[i], SCR_state.pin[i], GPIO_LEVEL_LOW);
            }
        }
    }
    if (closeDelayCounter)
    {
        closeDelayCounter--;
        if (closeDelayCounter == 0)
        {
            for (i = 0; i < SCR_Num; i++)
            {
                gpio_write(SCR_state.port[i], SCR_state.pin[i], GPIO_LEVEL_HIGH);
            }
        }
    }
}

#endif