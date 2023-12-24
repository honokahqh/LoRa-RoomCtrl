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

  mbsCoil._startAddr = 10000; /* 起始地址 */
  mbsCoil._endAddr = 10127;   /* 结束地址 */

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
  for (i = 0; i < 19; i++) // 设备基本参数
  {
    MBS_MappingHoldRegInit(&mbsHoldRegValue[i], 50000 + i, 0);
  }
  for (i = 0; i < 16; i++) // RGB默认十六种颜色
  {
    MBS_MappingHoldRegInit(&mbsHoldRegValue[i + 19], 50600 + i, 0);
  }
  for (i = 0; i < 30; i++) // 继电器映射
  {
    MBS_MappingHoldRegInit(&mbsHoldRegValue[i + 35], 50700 + i, i);
  }
  for (i = 0; i < 5; i++) // 本机MBS地址+版本号+通讯检测
  {
    MBS_MappingHoldRegInit(&mbsHoldRegValue[i + 65], 59000 + i, 0);
  }
  mbsHoldRegValue[Reg_RGB_Color_DEFAULT].pData = 0xF800;      // 红色
  mbsHoldRegValue[Reg_RGB_Color_DEFAULT + 1].pData = 0x001f;  // 蓝色
  mbsHoldRegValue[Reg_RGB_Color_DEFAULT + 2].pData = 0x07e0;  // 绿色
  mbsHoldRegValue[Reg_RGB_Color_DEFAULT + 3].pData = 0x8010;  // 紫色
  mbsHoldRegValue[Reg_RGB_Color_DEFAULT + 4].pData = 0xFFE0;  // 黄色
  mbsHoldRegValue[Reg_RGB_Color_DEFAULT + 5].pData = 0xFFE0;  // 黄色
  mbsHoldRegValue[Reg_RGB_Color_DEFAULT + 6].pData = 0x0010;  // 海军蓝
  mbsHoldRegValue[Reg_RGB_Color_DEFAULT + 7].pData = 0xFC00;  // 橘黄色
  mbsHoldRegValue[Reg_RGB_Color_DEFAULT + 8].pData = 0xBABA;  // 水鸭色
  mbsHoldRegValue[Reg_RGB_Color_DEFAULT + 9].pData = 0xFC18;  // 粉色
  mbsHoldRegValue[Reg_RGB_Color_DEFAULT + 10].pData = 0xF810; // 紫红色
  mbsHoldRegValue[Reg_RGB_Color_DEFAULT + 11].pData = 0x2104; // 灰色
  mbsHoldRegValue[Reg_RGB_Color_DEFAULT + 12].pData = 0x0400; // 绿色
  mbsHoldRegValue[Reg_RGB_Color_DEFAULT + 13].pData = 0x8000; // 暗红色
  mbsHoldRegValue[Reg_RGB_Color_DEFAULT + 14].pData = 0x4410; // 橄榄色
  mbsHoldRegValue[Reg_RGB_Color_DEFAULT + 15].pData = 0xFFFF; // 白色

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

  for (i = 0; i < USER_COIL_NUM; i++) // 查找符合地址
  {
    if (mbsCoilValue[i].coilAddr == CoilAddr)
      break;
  }
  if (i >= USER_COIL_NUM)
    return i; /*读取失败，地址错误*/

  for (j = 0; j < Len; j++)
    mbsCoilValue[i + j].pData = (data[j / 8] >> (j % 8)) & 0x01;

  return 1; /* 读取成功 */
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
    if (mbsHoldRegValue[i].regAddr == regAddr) /* 保持寄存器首地址正确 */
    {
      if (mbsHoldRegValue[i + num - 1].regAddr == (regAddr + num - 1)) /* 保持寄存器尾地址正确 */
      {
        for (j = 0; j < num; j++)
        {
          *Value++ = (mbsHoldRegValue[i + j].pData) >> 8;
          *Value++ = (mbsHoldRegValue[i + j].pData);
        }
        return 1; /* 读取成功 */
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
    if (mbsHoldRegValue[i].regAddr == regAddr) /* 保持寄存器首地址正确 */
    {
      if (mbsHoldRegValue[i + num - 1].regAddr == (regAddr + num - 1)) /* 保持寄存器尾地址正确 */
      {
        for (j = 0; j < num; j++)
        {
          mbsHoldRegValue[i + j].pData = (*Value++) << 8;
          mbsHoldRegValue[i + j].pData |= *Value++;
        }
        return 1; /* 写入成功 */
      }
    }
  }
  return 0;
}

void MBS_Data_Process()
{
}

/* 将数据同步至MBS数组 */
void MBS_DataSyn()
{
}