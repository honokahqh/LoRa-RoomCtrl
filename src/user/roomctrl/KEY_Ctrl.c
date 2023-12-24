#include "MBS_Cmd_Ctrl.h"

#if ROOM_TYPE
Key_state_t Key_state;

// 10ms调用周期
void key_scan()
{
    static uint8_t key1_release, key2_release, key3_release, key4_release; // 按键释放标志
    (gpio_read(KEY_1_PORT, KEY_1_PIN) == GPIO_LEVEL_LOW) ? Key_state.key1++ : (Key_state.key1 = 0, key1_release = 1);
    (gpio_read(KEY_2_PORT, KEY_2_PIN) == GPIO_LEVEL_LOW) ? Key_state.key2++ : (Key_state.key2 = 0, key2_release = 1);
    (gpio_read(KEY_3_PORT, KEY_3_PIN) == GPIO_LEVEL_LOW) ? Key_state.key3++ : (Key_state.key3 = 0, key3_release = 1);
    (gpio_read(KEY_4_PORT, KEY_4_PIN) == GPIO_LEVEL_LOW) ? Key_state.key4++ : (Key_state.key4 = 0, key4_release = 1);

    // 模式切换
    if (Key_state.key2 > 300 && Key_state.key3 > 300)
        Key_state.mode = Choose;
    if (Key_state.mode == Choose && Key_state.key1)
        Key_state.mode = Ctrl;
    if (Key_state.mode == Ctrl && Key_state.key4)
        Key_state.mode = Choose;
        
    // mode 1 select id 
    if (Key_state.mode == Choose)
    {
        if ((Key_state.key2 && key2_release) || (Key_state.key2 > 100 && Key_state.key2 % 20 == 0))
        {
            Key_state.select_id++;
            if (Key_state.select_id > 30)
                Key_state.select_id = 30;
            key2_release = 0;
        }
        if ((Key_state.key3 && key3_release) || (Key_state.key3 > 100 && Key_state.key3 % 20 == 0))
        {
            if (Key_state.select_id)
                Key_state.select_id--;
            key3_release = 0;
        }
    }

    // mode 2 ctrl
    if (Key_state.mode == Ctrl)
    {
        if (Key_state.select_id < 5)
        {
            // 0 DAC 1~4 SCR 
            if (Key_state.key2 && *DeviceControlState[Key_state.select_id] < 100)
                *DeviceControlState[Key_state.select_id] += 1;
            else if (Key_state.key3 && *DeviceControlState[Key_state.select_id] > 0)
                *DeviceControlState[Key_state.select_id] -= 1;
        }
        else if (Key_state.select_id < 30)
        {
            // 5-29 控制对应的继电器
            if (Key_state.key2)
                *DeviceControlState[Key_state.select_id] = 1;
            else if (Key_state.key3)
                *DeviceControlState[Key_state.select_id] = 0;
        }
        else if (Key_state.select_id == 30)
        {
            // 30 总开关
            if(Key_state.key2)
                mbsCoilValue[Coil_MainSwitch].pData = 1;
            else if(Key_state.key3)
                mbsCoilValue[Coil_MainSwitch].pData = 0;
        }
    }

    // 超时关闭
    if (Key_state.key1 || Key_state.key2 || Key_state.key3 || Key_state.key4)
        Key_state.timeout = 6000;
    if (!Key_state.key1 && !Key_state.key2 && !Key_state.key3 && !Key_state.key4 && Key_state.timeout)
    {
        Key_state.timeout--;
        if (Key_state.timeout == 0)
            Key_state.mode = IDLE;
    }
}

#endif