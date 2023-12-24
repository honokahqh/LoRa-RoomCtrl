#include "MBS_Cmd_Ctrl.h"

// value:0~100
void set_dac_output()
{
    if(mbsCoilValue[Coil_MainSwitch].pData == 0)
    {
        dac_write_data(0);
        dac_software_trigger_cmd(true);
        return;
    }
    if(mbsHoldRegValue[Reg_DAC].pData > 100)
        dac_write_data(4000);
    else
        dac_write_data(mbsHoldRegValue[Reg_DAC].pData * 40);
    dac_software_trigger_cmd(true);
}