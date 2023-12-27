#ifndef MODBUS_USER_H
#define MODBUS_USER_H
#include "APP.h"
/*********************************************************************
 * @author      Honokahqh
 *
 * @brief       Modbus slaver
 *
 * @Init:       ���ñ�����ַ                    MBS_SelfAddr
                ʹ��ʵ�ֵ�Modbus������          MBS_FUNCTION_xx_ENABLE
                ���ô����������                MBS_PhysicalSendBuff()
 *              ���üĴ���Map                   MBS_MappingInit()
                �����Ҫ������ֱ�Ӵ����Hook��Ӧ�������뺯��
 * @Loop:       ���ս�������ú���MBS_CorePoll   
 *
 * @time:       2022.3.26
 */
/*Modbus slaver By Honokahqh*/
/***************************************************************/

/***************************************************************/
#define	MBS_FUNCTION_01_ENABLE			        (1)			
#define	MBS_FUNCTION_02_ENABLE			        (0)
#define	MBS_FUNCTION_03_ENABLE			        (1)
#define	MBS_FUNCTION_04_ENABLE			        (0)
#define	MBS_FUNCTION_05_ENABLE			        (1)
#define	MBS_FUNCTION_06_ENABLE			        (1)
#define	MBS_FUNCTION_0F_ENABLE			        (1)
#define	MBS_FUNCTION_10_ENABLE			        (1)

#define USER_COIL_NUM					        (34)		
#define USER_HOLDREG_NUM				        (70)		
#define USER_DISINPUT_NUM				        (0)
#define USER_INPUTREG_NUM				        (0)

#define MBS_PORT_RXBUFF_SIZE			        150
#define MBS_PORT_TXBUFF_SIZE			        150

//������ŵļ̵�����Ӧ����Ȧ��MBS_Cmd_CTRL.c
#define Coil_MainSwitch                         0   
#define Coil_SCR_Switch                         1
#define Coil_PWM_Switch                         2
#define Coil_UFH_Switch                         5

#define Reg_SCR_1                               0
#define Reg_SCR_2                               10
#define Reg_SCR_3                               11
#define Reg_SCR_4                               12

#define Reg_RGB_Color                           1
#define Reg_RGB_Switch_Time                     2
#define Reg_RGB_Index_Start                     3
#define Reg_RGB_Index_End                       4
#define Reg_RGB_Light                           14

#define Reg_DAC                                 13

#define Reg_RGB_Color_DEFAULT                   19//Ĭ��ʮ������ɫ����λindex
#define REG_RELAY_INDEX_MAP                     35//�̵���map

#define MBS_Addr                                65
#define MBS_Ver                                 66

void MBS_MappingInit(void);
void MBS_Data_Init(void);
uint8 MBS_MemReadCoilState(uint16 coilAddr);
uint8 MBS_MemWriteCoilState(uint16 coilAddr, uint16 data);
uint16 MBS_MemWriteCoilsState(uint16 CoilAddr, uint16 Len, uint8 *data);
uint8 MBS_MemReadHoldRegValue(uint16 regAddr, uint8 *Value, uint8 num);
uint8 MBS_MemWriteHoldRegValue(uint16 regAddr, uint8 *Value, uint8 num);
void MBS_Data_Process();
#endif /* MODBUS_USER_H */