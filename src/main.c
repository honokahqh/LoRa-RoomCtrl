#include <stdio.h>
#include <string.h>
#include "APP.h"

static void IAP_Detect(void);       // 检测是否需要升级
static void State_Updata(void);     // 上报版本号
static void BSTimer_Init(void);     // 定时器初始化
static void uart_log_init(void);    // 串口初始化
static void watch_dog_init(void);   // 看门狗初始化
static void GPTimer0_3_PWM(void);   // 调光初始化
static void Relay_GPIO_Init(void);  // 继电器初始化
static void DAC_Init();             // 0~10v调光初始化
static void board_init(void);       // 硬件初始化

uint32_t Sys_ms;                    // 系统时间
uart_state_t uart_state;            // 串口状态

int main(void)
{
    board_init(); // 硬件初始化
	Relay_GPIO_Init();                           // 继电器初始化
#if Lora_Is_APP
    delay_ms(2000); // 等待Z9启动完成
#endif
    IAP_Check();                                 // Boot:检查是否有APP-是否需要升级 APP:检测标志位是否正常
    Lora_StateInit();                            // 状态初始化
    delay_ms(((Lora_State.chip_ID) % 10) * 100); // 随机延迟
    GPTimer0_3_PWM();                            //	调光
    DAC_Init();                                  // 0~10v调光初始化
    MBS_MappingInit();                           // map初始化、寄存器状态初始化
    MBS_Data_Init();                             // 从机继电器初始化
    mbs_data_syn();                              // 从Flash读取从机mbs数据-如果没有数据用上面的初始化数据
    mbh_map_init();                              // 主机
    lora_init();                                 // lora初始化
    wdg_reload();                                // 看门狗喂狗
    IAP_Detect();                                // 检测是否需要升级
    State_Updata();                              // 上报版本号
    System_Run();
}

static void IAP_Detect()
{
#if !Lora_Is_APP
    if (ymodem_session.state != YMODEM_STATE_IDLE)
    {
        uint8_t temp_data;
        Debug_B("OTA Start, page:%d\r\n", ((FLASH_MAX_SIZE - (APP_ADDR - Boot_ADDR)) / 4096) - 3);
        temp_data = 0x01;
        /* 需要保证APP和BOOT 4kb对齐 */
        for (uint8_t i = 0; i < ((FLASH_MAX_SIZE - (APP_ADDR - Boot_ADDR)) / 4096) - 3; i++)
        {
            flash_erase_page(APP_ADDR + 0x1000 * i);
        }
        flash_program_bytes(OTA_ADDR + 16, &temp_data, 1); // 正在进行OTA标志写1
        ymodem_session.state = YMODEM_STATE_START;
        // 485模式start帧响应在Boot而非APP
        if (Lora_Para_AT.Enable == 0)
        {
            uint8_t temp_data;
            temp_data = YMODEM_CRC;
            UART_SendData(&temp_data, 1);
        }
    }
#endif
}

/**
 * BSTimer_Init
 * @brief 定时器初始化,1ms定时
 * @author Honokahqh
 * @date 2023-08-05
 */
static void BSTimer_Init()
{
    bstimer_init_t bstimer_init_config;

    bstimer_init_config.bstimer_mms = BSTIMER_MMS_ENABLE;
    bstimer_init_config.period = 23;     // time period is ((1 / 2.4k) * (2399 + 1))
    bstimer_init_config.prescaler = 999; // sysclock defaults to 24M, is divided by (prescaler + 1) to 2.4k
    bstimer_init_config.autoreload_preload = true;
    bstimer_init(BSTIMER0, &bstimer_init_config);
    bstimer_config_overflow_update(BSTIMER0, ENABLE);
    bstimer_config_interrupt(BSTIMER0, ENABLE);
    bstimer_cmd(BSTIMER0, true);
    NVIC_EnableIRQ(BSTIMER0_IRQn);
    NVIC_SetPriority(BSTIMER0_IRQn, 2);

#if ROOM_TYPE
    bstimer_init_config.prescaler = 99; // sysclock defaults to 24M, is divided by (prescaler + 1) to 2.4k
    bstimer_init(BSTIMER1, &bstimer_init_config);
    bstimer_config_overflow_update(BSTIMER1, ENABLE);
    bstimer_config_interrupt(BSTIMER1, ENABLE);
    bstimer_cmd(BSTIMER1, true);
    NVIC_EnableIRQ(BSTIMER1_IRQn);
    NVIC_SetPriority(BSTIMER1_IRQn, 2);
#endif
}

/**
 * BSTIMER0_IRQHandler
 * @brief 定时器中断服务函数,1ms定时,用于计时和485接收超时判断,485接收超时后,将数据发送给处理函数
 * @author Honokahqh
 * @date 2023-08-05
 */
void BSTIMER0_IRQHandler(void)
{
    if (bstimer_get_status(BSTIMER0, BSTIMER_SR_UIF))
    {
        // UIF flag is active
        Sys_ms++;
        if (uart_state.busy)
        {
            uart_state.IDLE++;
            if (uart_state.IDLE >= UART_IDLE_Timeout)
            {
                uart_state.busy = 0;
                uart_state.IDLE = 0;
                uart_state.has_data = true;
            }
        }
    }
}

/**
 * BSTIMER1_IRQHandler
 * @brief 100us定时用于可控硅
 * @author Honokahqh
 * @date 2023-08-05
 */
#if ROOM_TYPE
void BSTIMER1_IRQHandler(void)
{
    if (bstimer_get_status(BSTIMER1, BSTIMER_SR_UIF))
    {
        SCR_Ctrl();
    }
}
#endif
/**
 * millis
 * @brief ptos会调用,用于OS系统
 * @return 系统时间
 * @author Honokahqh
 * @date 2023-08-05
 */
unsigned int millis(void)
{
    return Sys_ms;
}

/**
 * UART_SendData
 * @brief 串口0发送数据
 * @param data 数据指针
 * @param len 数据长度
 * @author Honokahqh
 * @date 2023-08-05
 */
void UART_SendData(const uint8_t *data, uint16_t len)
{
    uint16_t i;
    gpio_write(RS485_DIR_PORT, RS485_DIR_PIN, GPIO_LEVEL_HIGH); // 485 TX Enable
#if ROOM_TYPE
    for (i = 0; i < len; i++)
    {
        while (uart_get_flag_status(UART3, UART_FLAG_TX_FIFO_FULL) == SET)
            ;
        UART3->DR = *data++;
    }
    while (uart_get_flag_status(UART3, UART_FLAG_TX_FIFO_EMPTY) == RESET)
        ;
#else
    for (i = 0; i < len; i++)
    {
        while (uart_get_flag_status(UART0, UART_FLAG_TX_FIFO_FULL) == SET)
            ;
        UART0->DR = *data++;
    }
    while (uart_get_flag_status(UART0, UART_FLAG_TX_FIFO_EMPTY) == RESET)
        ;
#endif
    delay_us(1050);                                            // 波特率:9600->1041us
    gpio_write(RS485_DIR_PORT, RS485_DIR_PIN, GPIO_LEVEL_LOW); // 485 TX Disable
}

/**
 * UART3_IRQHandler
 * @brief 接收数据传入mbs缓冲区,并设置接收标志位,由timer0进行IDLE计时
 * @author Honokahqh
 * @date 2023-08-05
 */
void UART3_IRQHandler(void)
{
    if (uart_get_interrupt_status(UART3, UART_INTERRUPT_RX_DONE))
    {
        uart_clear_interrupt(UART3, UART_INTERRUPT_RX_DONE);
        uart_state.rx_buff[uart_state.rx_len++] = UART3->DR & 0xFF;
        uart_state.busy = true;
        uart_state.IDLE = 0;
    }
    if (uart_get_interrupt_status(UART3, UART_INTERRUPT_RX_TIMEOUT))
    {
        uart_clear_interrupt(UART3, UART_INTERRUPT_RX_TIMEOUT);
    }
}

/**
 * UART0_IRQHandler
 * @brief 接收数据传入mbs缓冲区,并设置接收标志位,由timer0进行IDLE计时
 * @author Honokahqh
 * @date 2023-08-05
 */
void UART0_IRQHandler(void)
{
    if (uart_get_interrupt_status(UART0, UART_INTERRUPT_RX_DONE))
    {
        uart_clear_interrupt(UART0, UART_INTERRUPT_RX_DONE);
        uart_state.rx_buff[uart_state.rx_len++] = UART0->DR & 0xFF;
        uart_state.busy = true;
        uart_state.IDLE = 0;
    }
    if (uart_get_interrupt_status(UART0, UART_INTERRUPT_RX_TIMEOUT))
    {
        uart_clear_interrupt(UART0, UART_INTERRUPT_RX_TIMEOUT);
    }
}

/**
 * uart_log_init
 * @brief IO16\IO17初始化为UART0,工作模式:9600、Debug:1M
 * @author Honokahqh
 * @date 2023-08-05
 */
static void uart_log_init(void)
{
#if ROOM_TYPE
    // UART3
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_UART3, true);
    gpio_set_iomux(UART_TX_PORT, UART_TX_PIN, 2); // TX
    gpio_set_iomux(UART_RX_PORT, UART_RX_PIN, 2); // RX
#else
	rcc_enable_peripheral_clk(RCC_PERIPHERAL_UART0, true);
	gpio_set_iomux(UART_TX_PORT, UART_TX_PIN, 1); // TX
    gpio_set_iomux(UART_RX_PORT, UART_RX_PIN, 1); // RX
#endif    
    
    /* uart config struct init */
    uart_config_t uart_config;
    uart_config_init(&uart_config);

    uart_config.baudrate = Uart_BAUD;

#if ROOM_TYPE
    uart_init(UART3, &uart_config);

    uart_config_interrupt(UART3, UART_INTERRUPT_RX_DONE, ENABLE);
    uart_config_interrupt(UART3, UART_INTERRUPT_RX_TIMEOUT, ENABLE);

    uart_cmd(UART3, ENABLE);
    NVIC_EnableIRQ(UART3_IRQn);
    NVIC_SetPriority(UART3_IRQn, 2);
#else
    uart_init(UART0, &uart_config);

    uart_config_interrupt(UART0, UART_INTERRUPT_RX_DONE, ENABLE);
    uart_config_interrupt(UART0, UART_INTERRUPT_RX_TIMEOUT, ENABLE);

    uart_cmd(UART0, ENABLE);
    NVIC_EnableIRQ(UART0_IRQn);
    NVIC_SetPriority(UART0_IRQn, 2);     
#endif
}

/**
 * watch_dog_init
 * @brief 看门口初始化:10s
 * @author Honokahqh
 * @date 2023-08-05
 */
static void watch_dog_init()
{
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_WDG, true);
    uint32_t timeout = 10000;
    uint32_t wdgclk_freq = rcc_get_clk_freq(RCC_PCLK0);
    uint32_t reload_value = timeout * (wdgclk_freq / 1000 / 2);

    // start wdg
    wdg_start(reload_value);
    NVIC_EnableIRQ(WDG_IRQn);
}

/**
 * GPTimer0_3_PWM
 * @brief 三路调光PWM使能
 * @author Honokahqh
 * @date 2023-08-05
 */
timer_init_t timerx_init;
timer_oc_init_t oc_init;
static void GPTimer0_3_PWM()
{
    gpio_set_iomux(PWM_R_PORT, PWM_R_PIN, 6); // GTIM1 CH2
    gpio_set_iomux(PWM_G_PORT, PWM_G_PIN, 6); // GTIM1 CH0
    gpio_set_iomux(PWM_B_PORT, PWM_B_PIN, 6); // GTIM1 CH3

    memset(&oc_init, 0, sizeof(oc_init));
    oc_init.oc_mode.oc0m_mode = TIMER_OC0M_PWM1;
    
    oc_init.pulse = 64;
    oc_init.high_level = true;
    oc_init.oc_fast = false;

    timerx_init.prescaler = 0;
    timerx_init.counter_mode = TIMER_COUNTERMODE_UP;
    timerx_init.period = 1025;
    timerx_init.clock_division = TIMER_CKD_FPCLK_DIV1;
    timerx_init.autoreload_preload = false;
#if ROOM_TYPE
    timer_config_pwm(TIMER1, &oc_init, &timerx_init, TIMER_CHANNEL_0);
    timer_config_pwm(TIMER1, &oc_init, &timerx_init, TIMER_CHANNEL_2);
	oc_init.oc_mode.oc1m_mode = TIMER_OC1M_PWM1;
    timer_config_pwm(TIMER1, &oc_init, &timerx_init, TIMER_CHANNEL_3);
    timer_cmd(TIMER1, true);
#else
    timer_config_pwm(TIMER1, &oc_init, &timerx_init, 0);
    timer_config_pwm(TIMER1, &oc_init, &timerx_init, 2);
    timer_config_pwm(TIMER0, &oc_init, &timerx_init, 0);
    timer_cmd(TIMER1, true);
    timer_cmd(TIMER0, true);
#endif

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


/**
 * Relay_GPIO_Init
 * @brief 10路继电器IO配置
 * @author Honokahqh
 * @date 2023-08-05
 */
static void Relay_GPIO_Init()
{
    // Relay IO init
    gpio_init(Relay_2A1_PORT, Relay_2A1_PIN, GPIO_MODE_OUTPUT_PP_LOW);
    gpio_init(Relay_2A2_PORT, Relay_2A2_PIN, GPIO_MODE_OUTPUT_PP_LOW);
    gpio_init(Relay_2A3_PORT, Relay_2A3_PIN, GPIO_MODE_OUTPUT_PP_LOW);
    gpio_init(Relay_2A4_PORT, Relay_2A4_PIN, GPIO_MODE_OUTPUT_PP_LOW);
    gpio_init(Relay_2A5_PORT, Relay_2A5_PIN, GPIO_MODE_OUTPUT_PP_LOW);

    gpio_init(Relay_5A1_PORT, Relay_5A1_PIN, GPIO_MODE_OUTPUT_PP_LOW);
    gpio_init(Relay_5A2_PORT, Relay_5A2_PIN, GPIO_MODE_OUTPUT_PP_LOW);
    gpio_init(Relay_5A3_PORT, Relay_5A3_PIN, GPIO_MODE_OUTPUT_PP_LOW);
    gpio_init(Relay_5A4_PORT, Relay_5A4_PIN, GPIO_MODE_OUTPUT_PP_LOW);
    gpio_init(Relay_5A5_PORT, Relay_5A5_PIN, GPIO_MODE_OUTPUT_PP_LOW);

    // RS485 IO init
    gpio_init(RS485_DIR_PORT, RS485_DIR_PIN, GPIO_MODE_OUTPUT_PP_LOW);

    // RESET IO init
    gpio_init(GPIO_RST_PORT, GPIO_RST_PIN, GPIO_MODE_INPUT_PULL_UP);
    gpio_config_interrupt(GPIO_RST_PORT, GPIO_RST_PIN, GPIO_INTR_FALLING_EDGE);

    // System Led
    gpio_init(System_Led_PORT, System_Led_PIN, GPIO_MODE_OUTPUT_PP_LOW);
    gpio_write(System_Led_PORT, System_Led_PIN, GPIO_LEVEL_LOW);

#if ROOM_TYPE
    gpio_init(Relay_1A1_PORT, Relay_1A1_PIN, GPIO_MODE_OUTPUT_PP_LOW);
    gpio_init(Relay_1A2_PORT, Relay_1A2_PIN, GPIO_MODE_OUTPUT_PP_LOW);
    gpio_init(Relay_1A3_PORT, Relay_1A3_PIN, GPIO_MODE_OUTPUT_PP_LOW);
    gpio_init(Relay_1A4_PORT, Relay_1A4_PIN, GPIO_MODE_OUTPUT_PP_LOW);
    // 595 IO init
    gpio_init(Relay_595_SCLK_PORT, Relay_595_SCLK_PIN, GPIO_MODE_OUTPUT_PP_LOW);
    gpio_init(Relay_595_HCLK_PORT, Relay_595_HCLK_PIN, GPIO_MODE_OUTPUT_PP_LOW);
    gpio_init(Relay_595_DATA_PORT, Relay_595_DATA_PIN, GPIO_MODE_OUTPUT_PP_LOW);

    gpio_init(LED_595_SCLK_PORT, LED_595_SCLK_PIN, GPIO_MODE_OUTPUT_PP_LOW);
    gpio_init(LED_595_HCLK_PORT, LED_595_HCLK_PIN, GPIO_MODE_OUTPUT_PP_LOW);
    gpio_init(LED_595_DATA_PORT, LED_595_DATA_PIN, GPIO_MODE_OUTPUT_PP_LOW);

    // zero crossing
    gpio_init(Zero_Crossing_PORT, Zero_Crossing_PIN, GPIO_MODE_INPUT_PULL_UP);
    gpio_config_interrupt(Zero_Crossing_PORT, Zero_Crossing_PIN, GPIO_INTR_FALLING_EDGE);

    // SCR IO init
    gpio_init(SCR_1_PORT, SCR_1_PIN, GPIO_MODE_OUTPUT_PP_LOW);
    gpio_init(SCR_2_PORT, SCR_2_PIN, GPIO_MODE_OUTPUT_PP_LOW);
    gpio_init(SCR_3_PORT, SCR_3_PIN, GPIO_MODE_OUTPUT_PP_LOW);
    gpio_init(SCR_4_PORT, SCR_4_PIN, GPIO_MODE_OUTPUT_PP_LOW);

    // key IO init
    gpio_init(KEY_1_PORT, KEY_1_PIN, GPIO_MODE_INPUT_PULL_UP);
    gpio_init(KEY_2_PORT, KEY_2_PIN, GPIO_MODE_INPUT_PULL_UP);
    gpio_init(KEY_3_PORT, KEY_3_PIN, GPIO_MODE_INPUT_PULL_UP);
    gpio_init(KEY_4_PORT, KEY_4_PIN, GPIO_MODE_INPUT_PULL_UP);
	
	gpio_write(SCR_1_PORT, SCR_1_PIN, GPIO_LEVEL_HIGH);
	gpio_write(SCR_2_PORT, SCR_2_PIN, GPIO_LEVEL_HIGH);
	gpio_write(SCR_3_PORT, SCR_3_PIN, GPIO_LEVEL_HIGH);
	gpio_write(SCR_4_PORT, SCR_4_PIN, GPIO_LEVEL_HIGH);

    gpio_write(LED_595_SCLK_PORT,LED_595_SCLK_PIN,GPIO_LEVEL_HIGH);
    gpio_write(LED_595_HCLK_PORT,LED_595_HCLK_PIN,GPIO_LEVEL_HIGH);
#endif
    NVIC_EnableIRQ(GPIO_IRQn);
    NVIC_SetPriority(GPIO_IRQn, 2);
}

/**
 * GPIO_IRQHandler
 * @brief Rst pin中断处理
 * @author Honokahqh
 * @date 2023-08-05
 */
uint16_t GPIO_RST_Flag;
void GPIO_IRQHandler(void)
{
#if ROOM_TYPE
    // zero crossing
    if (gpio_get_interrupt_status(Zero_Crossing_PORT, Zero_Crossing_PIN) == SET)
    {
        gpio_clear_interrupt(Zero_Crossing_PORT, Zero_Crossing_PIN);
        SCR_Reset();
    }
#endif
    if (gpio_get_interrupt_status(GPIO_RST_PORT, GPIO_RST_PIN) == SET)
    {
        gpio_clear_interrupt(GPIO_RST_PORT, GPIO_RST_PIN);
        GPIO_RST_Flag = 1;
        Debug_B("Device Parameter Init");
    }
}

static void DAC_Init()
{
#if ROOM_TYPE
    dac_config_t config;

    rcc_enable_peripheral_clk(RCC_PERIPHERAL_DAC, true);
	rcc_enable_peripheral_clk(RCC_PERIPHERAL_GPIOA, true);
    // gpio_init(DAC_PORT, DAC_PIN, GPIO_MODE_ANALOG);
    memset(&config, 0, sizeof(dac_config_t));
    config.trigger_src = DAC_TRIGGER_SRC_SOFTWARE;

    dac_init(&config);
    dac_cmd(true);
    
    dac_write_data(2000);
    dac_software_trigger_cmd(true);
#endif 
}
/**
 * board_init
 * @brief 硬件初始化
 * @author Honokahqh
 * @date 2023-08-05
 */
static void board_init()
{
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_AFEC, true);

    // enable the clk
    rcc_enable_oscillator(RCC_OSC_XO32K, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_UART0, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_BSTIMER0, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_BSTIMER1, true);
	rcc_enable_peripheral_clk(RCC_PERIPHERAL_TIMER0, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_TIMER1, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_GPIOA, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_GPIOB, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_GPIOC, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_GPIOD, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_PWR, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_SAC, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_LORA, true);

    delay_ms(100);

    watch_dog_init();
    pwr_xo32k_lpm_cmd(true);
    uart_log_init();
    BSTimer_Init();
    /* 复位Lora模块 */
    (*(volatile uint32_t *)(0x40000000 + 0x18)) &= ~RCC_RST0_LORA_RST_N_MASK;
    delay_ms(100);
    (*(volatile uint32_t *)(0x40000000 + 0x18)) |= RCC_RST0_LORA_RST_N_MASK;
}

// 上报版本号
static void State_Updata()
{
    if (Lora_Para_AT.Enable == 1)
    {
        if (Lora_State.Net_State == Net_JoinGateWay && ymodem_session.state == YMODEM_STATE_IDLE)
        {
            Lora_State.Device_RSSI = 255;
            PCmd_HeartBeat();
            Cmd_Query_SubVersion();
        }
    }
    else
    {
        if (ymodem_session.state == YMODEM_STATE_IDLE)
        {
            uint8_t temp_buf[] = {0xFF, MBS_RoomCtrlAddr, 0xFE};
            UART_SendData(temp_buf, 3);
        }
    }
}
#ifdef USE_FULL_ASSERT
void assert_failed(void *file, uint32_t line)
{
    (void)file;
    (void)line;

    while (1)
    {
    }
}
#endif
