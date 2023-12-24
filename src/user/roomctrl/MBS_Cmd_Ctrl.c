#include "MBS_Cmd_Ctrl.h"

// 10ms调用一次,中控外设控制
void mbs_ctrl()
{
    static uint32_t mbs_ctrl_counter = 0;   
    mbs_ctrl_counter++;
    if(mbs_ctrl_counter%10 == 0)
    {
        #if ROOM_TYPE
        set_dac_output();
        #endif
    }
    if(mbs_ctrl_counter%30 == 0)
    {
        Relay_Service();
    }
    if(mbs_ctrl_counter%100 == 0)
    {
        AirCon_Relay_Auto_Close();
    }
    RGB_Service();
#if ROOM_TYPE
    Led_display_service();
    key_scan();
#endif
}