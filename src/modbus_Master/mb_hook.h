/**
  ******************************************************************************
  * @file    mb_hook.c
  * @author  Derrick Wang
  * @brief   modebus�ص�����ͷ�ļ�
  ******************************************************************************
  * @note
  * 
  ******************************************************************************
  */

#ifndef __MB_HOOK_H
#define __MB_HOOK_H
#include "mb_include.h"
/**
 * 	@brief  MODBUS����ģʽ�½��յ��ӻ��ظ���ͬ������Ļص�����
 * 	@param	id:�ӻ��ĵ�ַ
 * 	@param 	data:���յ��Ĵӻ�����������ָ��
 *  @param 	datalen:���յ��Ĵӻ����������ݳ���
 * 	@return	NONE
 * 	@note	rec01\02\03���������ִ�������
 */

//��ַ�� + ������ + ��ַ*2 + ��Ȧ����*2 + CRC
//��ַ�� + ������ + �ֽ��� + ��Ȧ״̬*(num/16) + CRC
//ͨѶ����:01 01 13 88 00 0A 38 A3 --- 01 01 02 41 02 08 6D                     ��5000��ʼ10����Ȧ,5000��50006��5009Ϊ1
//ͨѶ����:02 05 13 88 FF 00 08 A7 --- 02 05 13 88 FF 00 08 A7                  д5000��ȦΪ1
//ͨѶ����:02 0F 13 88 00 14 03 01 04 08 BB AD --- 02 0F 13 88 00 14 D1 59      д5000��5010��5019��ȦΪ1
//ͨѶ����:02 03 27 10 00 04 4F 4B --- 02 03 08 03 E8 00 00 4E 20 00 00 A4 6A   ��10000��ʼ4�����ּĴ���
//ͨѶ����:02 06 27 10 22 B8 9A 5A --- 02 06 27 10 22 B8 9A 5A                  д10000�Ĵ���Ϊ8888
//ͨѶ����:02 10 27 10 00 04 08 00 01 00 02 00 03 00 04 D8 60 --- 02 10 27 10 00 04 CA 88  д10000��ʼ4���Ĵ���Ϊ1��2��3��4

void mbh_hook_rec01(uint8_t id,uint8_t *data,uint8_t datalen);//01H:����Ȧ״̬
void mbh_hook_rec02(uint8_t id,uint8_t *data,uint8_t datalen);//02H:����ɢ����״̬
void mbh_hook_rec03(uint8_t id,uint8_t *data,uint8_t datalen);//03H:�����ּĴ���
void mbh_hook_rec04(uint8_t id,uint8_t *data,uint8_t datalen);//04H:������Ĵ���

//��ַ�� + ������ + ��ַ*2 + ��Ȧ����*2 + CRC
void mbh_hook_rec05(uint8_t id,uint8_t *data,uint8_t datalen);//05H:д������Ȧ
void mbh_hook_rec06(uint8_t id,uint8_t *data,uint8_t datalen);//06H:д�������ּĴ���
void mbh_hook_rec15(uint8_t id,uint8_t *data,uint8_t datalen);//0FH:д�����Ȧ
void mbh_hook_rec16(uint8_t id,uint8_t *data,uint8_t datalen);//10H:д������ּĴ���

#endif

