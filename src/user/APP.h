	#ifndef __APP_H
#define __APP_H

#include <stdio.h>

#include "lora_core.h"
#include "delay.h"
#include "timer.h"
#include "radio.h"
#include "tremo_uart.h"
#include "tremo_gpio.h"
#include "tremo_rcc.h"
#include "tremo_pwr.h"
#include "tremo_delay.h"
#include "tremo_bstimer.h"
#include "tremo_system.h"
#include "tremo_flash.h"
#include "tremo_adc.h"
#include "tremo_timer.h"
#include "tremo_lptimer.h"
#include "tremo_regs.h"
#include "tremo_wdg.h"
#include "tremo_dac.h"
#include "pt.h"

#include "Modbus_CORE.h"
#include "mb_include.h"
#include "ringbuff.h"
#include "MBS_Cmd_Ctrl.h"

// 大中控 4按键 + 4可控硅 + 10继电器 + 8 595继电器 + 3 595LED + 3PWM
// 小中控 10继电器 + 3 PWM
#define ROOM_PLUS	1
#define ROOM_MINI	0

#if ROOM_TYPE
	#define Relay_1A1_PORT GPIOA
	#define Relay_1A1_PIN GPIO_PIN_5
	#define Relay_1A2_PORT GPIOA
	#define Relay_1A2_PIN GPIO_PIN_13
	#define Relay_1A3_PORT GPIOD
	#define Relay_1A3_PIN GPIO_PIN_10
	#define Relay_1A4_PORT GPIOD
	#define Relay_1A4_PIN GPIO_PIN_12

	#define Relay_2A1_PORT GPIOD
	#define Relay_2A1_PIN GPIO_PIN_14
	#define Relay_2A2_PORT GPIOB
	#define Relay_2A2_PIN GPIO_PIN_15
	#define Relay_2A3_PORT GPIOB
	#define Relay_2A3_PIN GPIO_PIN_14
	#define Relay_2A4_PORT GPIOB
	#define Relay_2A4_PIN GPIO_PIN_11
	#define Relay_2A5_PORT GPIOB
	#define Relay_2A5_PIN GPIO_PIN_8

	// 3/4路继电器通过595控制
	#define Relay_595_SCLK_PORT GPIOA
	#define Relay_595_SCLK_PIN GPIO_PIN_14
	#define Relay_595_HCLK_PORT GPIOA
	#define Relay_595_HCLK_PIN GPIO_PIN_15
	#define Relay_595_DATA_PORT GPIOB
	#define Relay_595_DATA_PIN GPIO_PIN_7

	#define Relay_5A1_PORT GPIOB
	#define Relay_5A1_PIN GPIO_PIN_1
	#define Relay_5A2_PORT GPIOB
	#define Relay_5A2_PIN GPIO_PIN_0
	#define Relay_5A3_PORT GPIOA
	#define Relay_5A3_PIN GPIO_PIN_2
	#define Relay_5A4_PORT GPIOA
	#define Relay_5A4_PIN GPIO_PIN_3
	#define Relay_5A5_PORT GPIOA
	#define Relay_5A5_PIN GPIO_PIN_0

	#define System_Led_PORT GPIOA
	#define System_Led_PIN GPIO_PIN_1

	#define Zero_Crossing_PORT GPIOB
	#define Zero_Crossing_PIN GPIO_PIN_9

	#define PWM_R_PORT GPIOA
	#define PWM_R_PIN GPIO_PIN_11
	#define PWM_G_PORT GPIOA
	#define PWM_G_PIN GPIO_PIN_8
	#define PWM_B_PORT GPIOA
	#define PWM_B_PIN GPIO_PIN_12

	#define SCR_1_PORT GPIOC
	#define SCR_1_PIN GPIO_PIN_10
	#define SCR_2_PORT GPIOC
	#define SCR_2_PIN GPIO_PIN_12
	#define SCR_3_PORT GPIOC
	#define SCR_3_PIN GPIO_PIN_13
	#define SCR_4_PORT GPIOA
	#define SCR_4_PIN GPIO_PIN_4

	#define DAC_PORT GPIOA
	#define DAC_PIN GPIO_PIN_9

	#define LED_595_SCLK_PORT GPIOC
	#define LED_595_SCLK_PIN GPIO_PIN_8
	#define LED_595_HCLK_PORT GPIOC
	#define LED_595_HCLK_PIN GPIO_PIN_5
	#define LED_595_DATA_PORT GPIOC
	#define LED_595_DATA_PIN GPIO_PIN_9

	#define GPIO_RST_PORT GPIOC
	#define GPIO_RST_PIN GPIO_PIN_2

	#define KEY_1_PORT GPIOC
	#define KEY_1_PIN GPIO_PIN_1
	#define KEY_2_PORT GPIOC
	#define KEY_2_PIN GPIO_PIN_0
	#define KEY_3_PORT GPIOC
	#define KEY_3_PIN GPIO_PIN_4
	#define KEY_4_PORT GPIOC
	#define KEY_4_PIN GPIO_PIN_3

	#define UART_RX_PORT GPIOB
	#define UART_RX_PIN GPIO_PIN_12
	#define UART_TX_PORT GPIOB
	#define UART_TX_PIN GPIO_PIN_13
	#define RS485_DIR_PORT GPIOB
	#define RS485_DIR_PIN GPIO_PIN_10
#else
	#define Relay_2A1_PORT		GPIOA
	#define Relay_2A1_PIN		GPIO_PIN_14
	#define Relay_2A2_PORT		GPIOA
	#define Relay_2A2_PIN		GPIO_PIN_15
	#define Relay_2A3_PORT		GPIOD
	#define Relay_2A3_PIN		GPIO_PIN_14
	#define Relay_2A4_PORT		GPIOD
	#define Relay_2A4_PIN		GPIO_PIN_12
	#define Relay_2A5_PORT		GPIOD
	#define Relay_2A5_PIN		GPIO_PIN_10

	#define Relay_5A1_PORT		GPIOA
	#define Relay_5A1_PIN		GPIO_PIN_4
	#define Relay_5A2_PORT		GPIOA
	#define Relay_5A2_PIN		GPIO_PIN_9
	#define Relay_5A3_PORT		GPIOC
	#define Relay_5A3_PIN		GPIO_PIN_15
	#define Relay_5A4_PORT		GPIOC
	#define Relay_5A4_PIN		GPIO_PIN_13
	#define Relay_5A5_PORT		GPIOC
	#define Relay_5A5_PIN		GPIO_PIN_12

	#define System_Led_PORT		GPIOC
	#define System_Led_PIN		GPIO_PIN_5

	#define GPIO_RST_PORT  		GPIOC
	#define GPIO_RST_PIN		GPIO_PIN_1	
	
	#define PWM_R_PORT			GPIOA
	#define PWM_R_PIN			GPIO_PIN_8
	#define PWM_G_PORT			GPIOA
	#define PWM_G_PIN			GPIO_PIN_11
	#define PWM_B_PORT			GPIOA
	#define PWM_B_PIN			GPIO_PIN_5

	#define UART_RX_PORT 		GPIOB	
	#define UART_RX_PIN 		GPIO_PIN_0
	#define UART_TX_PORT		GPIOB
	#define UART_TX_PIN 		GPIO_PIN_1
	#define RS485_DIR_PORT 		GPIOA
	#define RS485_DIR_PIN 		GPIO_PIN_3
#endif

//#define __DEBUG
#ifdef __DEBUG
#define Debug_B(format, ...)  printf(format, ##__VA_ARGS__);
#else
#define Debug_A(format, ...)
#define Debug_B(format, ...)
#define Debug_C(format, ...)                            
#endif

#ifdef __DEBUG
#define Uart_BAUD 1000000
#else
#define Uart_BAUD 9600
#endif

/* uart状态 */
#define UART_IDLE_Timeout 5
typedef struct
{
	uint8_t busy;	  // 串口忙
	uint8_t IDLE;	  // 串口空闲-0:空闲
	uint8_t has_data; // 串口一帧数据接收完成

	uint8_t rx_buff[256];
	uint8_t rx_len;
} uart_state_t;
extern uart_state_t uart_state;

void System_Run(void);
void lora_init(void);
void Lora_IRQ_Rrocess(void);

void Flash_Data_Syn();
void LoraState_Save();
void UART_SendData(const uint8_t *data, uint16_t len);
extern uint32_t Sys_ms;

#endif
