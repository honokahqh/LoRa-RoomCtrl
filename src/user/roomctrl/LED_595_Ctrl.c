#include "MBS_Cmd_Ctrl.h"

#if ROOM_TYPE
uint8_t led_displayer[]={0x03,0x9F,0x25,0x0D,0x99,0x49,0x41,0x1F,0x01,0x09};
uint8_t hc595_data[5] = {0xff,0Xff,0xff,0xff,0xff};	//74CH595缓存数组 1为灭 0为亮

void HC595_send_data(uint8_t *data)
{
    for(uint8_t i = 0;i < 5;i++)
    {
        for(uint8_t bit = 0;bit < 8;bit++)
        {
            if((data[i] << bit) & 0x80)
                gpio_write(LED_595_DATA_PORT,LED_595_DATA_PIN,GPIO_LEVEL_HIGH);
            else
                gpio_write(LED_595_DATA_PORT,LED_595_DATA_PIN,GPIO_LEVEL_LOW);
            gpio_write(LED_595_SCLK_PORT,LED_595_SCLK_PIN,GPIO_LEVEL_LOW);
            gpio_write(LED_595_SCLK_PORT,LED_595_SCLK_PIN,GPIO_LEVEL_HIGH);
        }
        gpio_write(LED_595_HCLK_PORT,LED_595_HCLK_PIN,GPIO_LEVEL_LOW);
        gpio_write(LED_595_HCLK_PORT,LED_595_HCLK_PIN,GPIO_LEVEL_HIGH);
    }
}   


void led_ctrl(uint8_t wei, uint8_t duan)
{
    if(mbsCoilValue[Coil_MainSwitch].pData)
    {
        for(uint8_t i = 0;i < 30;i++)
        {
            if(*DeviceControlState[i + 1])
                hc595_data[4 - (i/8)] &=~ (1 << (7 - (i%8)));
            else
                hc595_data[4 - (i/8)] |= (1 << (7 - (i%8)));
        }
    }
    else
    {
        for(uint8_t i = 0;i < 30;i++)
        {
            hc595_data[4 - (i/8)] |= (1 << (7 - (i%8)));
        }
    }


    switch (wei)    
	{
	case 0:
		hc595_data[1] &= 0xFC;
		break;
	case 1:
		hc595_data[1] &= 0xFC;
		hc595_data[1] |= 0x02;
		break;

	case 2:
		hc595_data[1] &= 0xFC;
		hc595_data[1] |= 0x01;
		break;
	}
	hc595_data[0] = duan; // 段码
	HC595_send_data(hc595_data);

}

void Led_display_service()
{
    static uint8_t led_place;
	if(*DeviceControlState[Key_state.select_id] > 99)
		*DeviceControlState[Key_state.select_id] = 99;
    if(Key_state.mode == IDLE)
        led_ctrl(0,led_displayer[0]);
    led_place = (led_place + 1) % 2;
    if(Key_state.mode == Choose && !led_place)
        led_ctrl(1,led_displayer[Key_state.select_id / 10]);
    if(Key_state.mode == Choose && led_place)
        led_ctrl(2,led_displayer[Key_state.select_id % 10]);
    if(Key_state.mode == Ctrl && !led_place)
        led_ctrl(1,led_displayer[*DeviceControlState[Key_state.select_id] / 10]);
    if(Key_state.mode == Ctrl && led_place)
        led_ctrl(2,led_displayer[*DeviceControlState[Key_state.select_id] % 10]);
}

#endif