#include "MBS_Cmd_Ctrl.h"
/**
 * RGB_PWM_CTRL
 * @brief pwm控制函数1
 * @author Honokahqh
 * @date 2023-08-05
 */
void RGB_PWM_CTRL(uint16_t color, uint16_t light)
{
	uint16_t res;
#if ROOM_TYPE
	res = (32 - (color >> 11)) * 32;
	TIMER1->CCR2 = res; // r

	res = (64 - ((color >> 5) & 0x3F)) * 16;
	TIMER1->CCR0 = res; // g

	res = (32 - (color & 0x1F)) * 32;
	TIMER1->CCR3 = res; // b
#else
	res = (32 - (color >> 11)) * 32;
	TIMER1->CCR0 = res;

	res = (64 - ((color >> 5) & 0x3F)) * 16;
	TIMER1->CCR2 = res;

	res = (32 - (color & 0x1F)) * 32;
	TIMER0->CCR0 = res;
#endif
}

/**
 * PWM_CTRL_DataFrom_showcow20
 * @brief pwm控制函数2
 * @author Honokahqh
 * @date 2023-08-05
 */
static uint16_t r_last = 1000, g_last = 1000, b_last = 1000;
void PWM_CTRL_DataFrom_showcow20()
{
	int r, b, g;
	/* 30亮度以下灯带无法点亮，重新映射一下 */
	if (mbsHoldRegValue[Reg_SCR_1].pData <= 5)
		r = 1000;
	else
		r = 700 - mbsHoldRegValue[Reg_SCR_1].pData * 7;

	if (mbsHoldRegValue[Reg_SCR_2].pData <= 5)
		b = 1000;
	else
		b = 700 - mbsHoldRegValue[Reg_SCR_2].pData * 7;

	if (mbsHoldRegValue[Reg_SCR_3].pData <= 5)
		g = 1000;
	else
		g = 700 - mbsHoldRegValue[Reg_SCR_3].pData * 7;

	// 渐变计算 10ms周期
	if (r_last < r && r_last < 1023)
		r_last += 2;
	else if (r_last > r && r_last > 1)
		r_last -= 2;
	if (b_last < b && b_last < 1023)
		b_last += 2;
	else if (b_last > b && b_last > 1)
		b_last -= 2;
	if (g_last < g && g_last < 1023)
		g_last += 2;
	else if (g_last > g && g_last > 1)
		g_last -= 2;

#if ROOM_TYPE
	TIMER1->CCR2 = r_last; // r
	TIMER1->CCR0 = g_last; // g
	TIMER1->CCR3 = b_last; // b
#else
	TIMER1->CCR0 = r_last; // r
	TIMER0->CCR0 = b_last; // b
	TIMER1->CCR2 = g_last; // g
#endif
}

/**
 * RGB_Service
 * @brief 三路PWM调光控制-主循环调用10ms
 * @author Honokahqh
 * @date 2023-08-05
 */
void RGB_Service()
{
	if (mbsCoilValue[Coil_MainSwitch].pData)
	{ // 中控总开关已打开
		if (mbsCoilValue[Coil_PWM_Switch].pData == 0)
		{ // 数据来源采用可控硅界面
			PWM_CTRL_DataFrom_showcow20();
		}
		else
		{
			RGB_PWM_CTRL(mbsHoldRegValue[Reg_RGB_Color].pData, mbsHoldRegValue[Reg_RGB_Light].pData);
		}
	}
	else
	{
		r_last = 1024;
		g_last = 1024; 
		b_last = 1024;
#if ROOM_TYPE
		TIMER1->CCR0 = 1024;
		TIMER1->CCR2 = 1024;
		TIMER1->CCR3 = 1024;
#else
		TIMER1->CCR0 = 1024;
		TIMER0->CCR0 = 1024;
		TIMER1->CCR2 = 1024;
#endif
	}
}
