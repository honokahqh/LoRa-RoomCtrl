#ifndef __LORA_FLASH_H__
#define __LORA_FLASH_H__

#include "APP.h"
      
#define CM4_IRQ_VECT_BASE           0xE000ED08
#define CM4_IRQ_EN                  0xE000E100
#define CM4_IRQ_CLR                 0xE000E180

#define FLASH_START_ADDR            0x08000000
#define FLASH_MAX_SIZE              0x20000 //128KB

#define FlashData1_ADDR              0x0800D000 // �豸ͨѶ����
#define FlashData2_ADDR              0x0800E000 // �����豸:16�ֽ�һ�飬ÿ���12���ֽ�Ϊ0xFF��Ч��Ϊ0��Ч
#define OTA_ADDR                    0x0800F000

#define Boot_ADDR                   0x08000000 // 13ҳ 52KB //3ҳ������
#define APP_ADDR                    0x08010000 // 16ҳ 64KB 

// ���Ᵽ���п�����
#define FlashMBSDATA_ADDR           0x0800E000  

void boot_to_app(uint32_t addr);
void Lora_AsData_Save(uint8_t ID);
void Lora_AsData_Syn();
void Lora_AsData_Add(uint8_t ID);
void Lora_AsData_Del(uint8_t ID);
void Lora_State_Save();
void Lora_State_Data_Syn();
void IAP_Check();
void mbs_data_syn();
void mbs_data_save();

#endif
