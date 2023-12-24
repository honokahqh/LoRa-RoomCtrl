#include "Modbus_Core.h"
#include "lora_core.h"

MBS_CoilValueTypes mbsCoilValue[USER_COIL_NUM];
MBS_CoilTypes mbsCoil = {mbsCoilValue, 0, 0, USER_COIL_NUM};

MBS_HoldRegValueTypes mbsHoldRegValue[USER_HOLDREG_NUM];
MBS_HoldRegTypes mbsHoldReg = {mbsHoldRegValue, 0, 0, USER_HOLDREG_NUM};

void MBS_MappingCoilInit(MBS_CoilValueTypes *coil, uint16_t addr, uint8_t Value)
{
  coil->coilAddr = addr;
  coil->pData = Value;
}

void MBS_MappingHoldRegInit(MBS_HoldRegValueTypes *reg, uint16 addr, uint16 Value)
{
  reg->regAddr = addr;
  reg->pData = Value;
}
/*
 */
void MBS_MappingInit()
{
  uint8 i;

  mbsCoil._startAddr = 10000; /* ��ʼ��ַ */
  mbsCoil._endAddr = 10127;   /* ������ַ */

  for (i = 0; i < 6; i++)
  {
    MBS_MappingCoilInit(&mbsCoilValue[i], i + 10000, 0);
  }
  for (i = 0; i < 28; i++)
  {
    MBS_MappingCoilInit(&mbsCoilValue[i + 6], i + 10100, 0);
  }

  mbsHoldReg._startAddr = 50000;
  mbsHoldReg._endAddr = 59004;
  for (i = 0; i < 19; i++) // �豸��������
  {
    MBS_MappingHoldRegInit(&mbsHoldRegValue[i], 50000 + i, 0);
  }
  for (i = 0; i < 16; i++) // RGBĬ��ʮ������ɫ
  {
    MBS_MappingHoldRegInit(&mbsHoldRegValue[i + 19], 50600 + i, 0);
  }
  for (i = 0; i < 30; i++) // �̵���ӳ��
  {
    MBS_MappingHoldRegInit(&mbsHoldRegValue[i + 35], 50700 + i, i);
  }
  for (i = 0; i < 5; i++) // ����MBS��ַ+�汾��+ͨѶ���
  {
    MBS_MappingHoldRegInit(&mbsHoldRegValue[i + 65], 59000 + i, 0);
  }
  mbsHoldRegValue[Reg_RGB_Color_DEFAULT].pData = 0xF800;      // ��ɫ
  mbsHoldRegValue[Reg_RGB_Color_DEFAULT + 1].pData = 0x001f;  // ��ɫ
  mbsHoldRegValue[Reg_RGB_Color_DEFAULT + 2].pData = 0x07e0;  // ��ɫ
  mbsHoldRegValue[Reg_RGB_Color_DEFAULT + 3].pData = 0x8010;  // ��ɫ
  mbsHoldRegValue[Reg_RGB_Color_DEFAULT + 4].pData = 0xFFE0;  // ��ɫ
  mbsHoldRegValue[Reg_RGB_Color_DEFAULT + 5].pData = 0xFFE0;  // ��ɫ
  mbsHoldRegValue[Reg_RGB_Color_DEFAULT + 6].pData = 0x0010;  // ������
  mbsHoldRegValue[Reg_RGB_Color_DEFAULT + 7].pData = 0xFC00;  // �ٻ�ɫ
  mbsHoldRegValue[Reg_RGB_Color_DEFAULT + 8].pData = 0xBABA;  // ˮѼɫ
  mbsHoldRegValue[Reg_RGB_Color_DEFAULT + 9].pData = 0xFC18;  // ��ɫ
  mbsHoldRegValue[Reg_RGB_Color_DEFAULT + 10].pData = 0xF810; // �Ϻ�ɫ
  mbsHoldRegValue[Reg_RGB_Color_DEFAULT + 11].pData = 0x2104; // ��ɫ
  mbsHoldRegValue[Reg_RGB_Color_DEFAULT + 12].pData = 0x0400; // ��ɫ
  mbsHoldRegValue[Reg_RGB_Color_DEFAULT + 13].pData = 0x8000; // ����ɫ
  mbsHoldRegValue[Reg_RGB_Color_DEFAULT + 14].pData = 0x4410; // ���ɫ
  mbsHoldRegValue[Reg_RGB_Color_DEFAULT + 15].pData = 0xFFFF; // ��ɫ

  if (Lora_Para_AT.Enable == 1)
  {
    mbsHoldRegValue[MBS_Addr].pData = MBS_SelfAddr;
    mbsHoldRegValue[MBS_Ver].pData = Dev_Version;
  }
  else
  {
    mbsHoldRegValue[MBS_Addr].pData = MBS_RoomCtrlAddr;
    mbsHoldRegValue[MBS_Ver].pData = MBS_Version;
  }
}

uint8 MBS_MemReadCoilState(uint16 coilAddr)
{
  uint16 i;
  for (i = 0; i < USER_COIL_NUM; i++)
  {
    if (mbsCoilValue[i].coilAddr == coilAddr)
      return (mbsCoilValue[i].pData) % 2;
  }
  return i;
}
uint16 MBS_MemWriteCoilsState(uint16 CoilAddr, uint16 Len, uint8 *data)
{
  uint16 i, j;

  for (i = 0; i < USER_COIL_NUM; i++) // ���ҷ��ϵ�ַ
  {
    if (mbsCoilValue[i].coilAddr == CoilAddr)
      break;
  }
  if (i >= USER_COIL_NUM)
    return i; /*��ȡʧ�ܣ���ַ����*/

  for (j = 0; j < Len; j++)
    mbsCoilValue[i + j].pData = (data[j / 8] >> (j % 8)) & 0x01;

  return 1; /* ��ȡ�ɹ� */
}
uint8 MBS_MemWriteCoilState(uint16 coilAddr, uint16 data)
{
  uint16 i;
  for (i = 0; i < USER_COIL_NUM; i++)
  {
    if (mbsCoilValue[i].coilAddr == coilAddr)
    {
      if (data)
        mbsCoilValue[i].pData = 1;
      else
        mbsCoilValue[i].pData = 0;
      return 1;
    }
  }
  return i;
}

uint8 MBS_MemReadHoldRegValue(uint16 regAddr, uint8 *Value, uint8 num)
{
  uint8 i, j;
  for (i = 0; i < USER_HOLDREG_NUM; i++)
  {
    if (mbsHoldRegValue[i].regAddr == regAddr) /* ���ּĴ����׵�ַ��ȷ */
    {
      if (mbsHoldRegValue[i + num - 1].regAddr == (regAddr + num - 1)) /* ���ּĴ���β��ַ��ȷ */
      {
        for (j = 0; j < num; j++)
        {
          *Value++ = (mbsHoldRegValue[i + j].pData) >> 8;
          *Value++ = (mbsHoldRegValue[i + j].pData);
        }
        return 1; /* ��ȡ�ɹ� */
      }
    }
  }
  return 0;
}

uint8 MBS_MemWriteHoldRegValue(uint16 regAddr, uint8 *Value, uint8 num)
{
  uint8 i, j;
  for (i = 0; i < USER_HOLDREG_NUM; i++)
  {
    if (mbsHoldRegValue[i].regAddr == regAddr) /* ���ּĴ����׵�ַ��ȷ */
    {
      if (mbsHoldRegValue[i + num - 1].regAddr == (regAddr + num - 1)) /* ���ּĴ���β��ַ��ȷ */
      {
        for (j = 0; j < num; j++)
        {
          mbsHoldRegValue[i + j].pData = (*Value++) << 8;
          mbsHoldRegValue[i + j].pData |= *Value++;
        }
        return 1; /* д��ɹ� */
      }
    }
  }
  return 0;
}

void MBS_Data_Process()
{
}

/* ������ͬ����MBS���� */
void MBS_DataSyn()
{
}