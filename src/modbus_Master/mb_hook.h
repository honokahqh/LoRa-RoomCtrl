/**
  ******************************************************************************
  * @file    mb_hook.c
  * @author  Derrick Wang
  * @brief   modebus回调函数头文件
  ******************************************************************************
  * @note
  * 
  ******************************************************************************
  */

#ifndef __MB_HOOK_H
#define __MB_HOOK_H
#include "mb_include.h"
/**
 * 	@brief  MODBUS主机模式下接收到从机回复不同功能码的回调处理
 * 	@param	id:从机的地址
 * 	@param 	data:接收到的从机发来的数据指针
 *  @param 	datalen:接收到的从机发来的数据长度
 * 	@return	NONE
 * 	@note	rec01\02\03……等数字代表功能码
 */

//地址码 + 功能码 + 地址*2 + 线圈数量*2 + CRC
//地址码 + 功能码 + 字节数 + 线圈状态*(num/16) + CRC
//通讯样例:01 01 13 88 00 0A 38 A3 --- 01 01 02 41 02 08 6D                     第5000开始10个线圈,5000、50006、5009为1
//通讯样例:02 05 13 88 FF 00 08 A7 --- 02 05 13 88 FF 00 08 A7                  写5000线圈为1
//通讯样例:02 0F 13 88 00 14 03 01 04 08 BB AD --- 02 0F 13 88 00 14 D1 59      写5000、5010、5019线圈为1
//通讯样例:02 03 27 10 00 04 4F 4B --- 02 03 08 03 E8 00 00 4E 20 00 00 A4 6A   读10000开始4个保持寄存器
//通讯样例:02 06 27 10 22 B8 9A 5A --- 02 06 27 10 22 B8 9A 5A                  写10000寄存器为8888
//通讯样例:02 10 27 10 00 04 08 00 01 00 02 00 03 00 04 D8 60 --- 02 10 27 10 00 04 CA 88  写10000开始4个寄存器为1、2、3、4

void mbh_hook_rec01(uint8_t id,uint8_t *data,uint8_t datalen);//01H:读线圈状态
void mbh_hook_rec02(uint8_t id,uint8_t *data,uint8_t datalen);//02H:读离散输入状态
void mbh_hook_rec03(uint8_t id,uint8_t *data,uint8_t datalen);//03H:读保持寄存器
void mbh_hook_rec04(uint8_t id,uint8_t *data,uint8_t datalen);//04H:读输入寄存器

//地址码 + 功能码 + 地址*2 + 线圈数量*2 + CRC
void mbh_hook_rec05(uint8_t id,uint8_t *data,uint8_t datalen);//05H:写单个线圈
void mbh_hook_rec06(uint8_t id,uint8_t *data,uint8_t datalen);//06H:写单个保持寄存器
void mbh_hook_rec15(uint8_t id,uint8_t *data,uint8_t datalen);//0FH:写多个线圈
void mbh_hook_rec16(uint8_t id,uint8_t *data,uint8_t datalen);//10H:写多个保持寄存器

#endif

