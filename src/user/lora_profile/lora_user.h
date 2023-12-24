#ifndef __LORA_USER_H
#define __LORA_USER_H

#include "lora_core.h"


void Lora_StateInit();
void User_Slaver_Cmd();

void Lora_Send_Data(uint8_t *data,uint8_t len);

void PCmd_Call_Serivce();
void PCmd_RoomIR_Update();

void Wait2TXEnd();
void Wait2RXEnd();


#endif
