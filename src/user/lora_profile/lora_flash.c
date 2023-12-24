/* ��Ҫ���� flash_program_bytes(uint32_t addr, uint8_t* data, uint32_t size)
            flash_erase_page(uint32_t addr) */
/* FlashData1_ADDR �������:�̵�ַ��PANID��������־,��ҳ4KB����С8�ֽ�д�� 512�� */
/* OTA_ADDR IAP����:APP1(�°汾�̼�vX.x) APP2(�°汾�̼�vX.x) */

#include "Lora_core.h"

uint16_t page1_offset, page2_offset, page3_offset;

/**
 * Lora_State_Data_Syn
 * @brief Lora�������ݶ�ȡ
 * @author Honokahqh
 * @date 2023-08-05
 */
void Lora_State_Data_Syn()
{
    uint32_t data;
    ChipID_Syn();
    /*get page1_offset*/
    for (page1_offset = 0; page1_offset < 500; page1_offset++)
    {
        data = *(uint32_t *)(FlashData1_ADDR + page1_offset * 8);
        if (data == 0xFFFFFFFF)
            break;
    }
    if (page1_offset != 0)
    {
        page1_offset--;
        Lora_Para_AT.SpreadingFactor = (*(uint8_t *)(FlashData1_ADDR + page1_offset * 8) & 0x0F);
        Lora_Para_AT.BandWidth = (*(uint8_t *)(FlashData1_ADDR + page1_offset * 8 + 0) >> 4) & 0x03;
        Lora_Para_AT.CodingRate = (*(uint8_t *)(FlashData1_ADDR + page1_offset * 8 + 0) >> 6) & 0x03;
        Lora_Para_AT.channel = *(uint8_t *)(FlashData1_ADDR + page1_offset * 8 + 1) & 0x7F;
        Lora_Para_AT.isMaster = (*(uint8_t *)(FlashData1_ADDR + page1_offset * 8 + 1) >> 7) & 0x01;
#if Lora_Always_Master
        Lora_Para_AT.isMaster = true;
#elif Lora_Always_Slaver
        Lora_Para_AT.isMaster = false;
#else
        Lora_Para_AT.isMaster = (*(uint8_t *)(FlashData1_ADDR + page1_offset * 8 + 1) >> 7) & 0x01;
#endif
        // Lora_Para_AT.Power = *(uint8_t *)(FlashData1_ADDR + page1_offset * 8 + 2) & 0x1F;
        (*(uint8_t *)(FlashData1_ADDR + page1_offset * 8 + 2) & 0x0F) ? (Lora_Para_AT.Enable = 1) : (Lora_Para_AT.Enable = 0);
        if (Lora_Para_AT.isMaster == true)
        {
            Lora_Para_AT.MinRSSI = (*(uint8_t *)(FlashData1_ADDR + page1_offset * 8 + 3) << 8) | (*(uint8_t *)(FlashData1_ADDR + page1_offset * 8 + 4));
            Lora_Para_AT.SAddr = 0;
        }   
        else
        {
            Lora_Para_AT.SAddr = (*(uint8_t *)(FlashData1_ADDR + page1_offset * 8 + 3) << 8) | (*(uint8_t *)(FlashData1_ADDR + page1_offset * 8 + 4));
        }

        Lora_Para_AT.PanID = (*(uint8_t *)(FlashData1_ADDR + page1_offset * 8 + 5) << 8) | (*(uint8_t *)(FlashData1_ADDR + page1_offset * 8 + 6));
        Lora_Para_AT.Net_State = (*(uint8_t *)(FlashData1_ADDR + page1_offset * 8 + 2) >> 5) & 0x03;
        Lora_Para_AT.NetOpenTime = *(uint8_t *)(FlashData1_ADDR + page1_offset * 8 + 7);

        page1_offset++;
    }
    else
    { // ȫ������ΪĬ��ֵ
        Lora_Para_AT.Enable = 1;
        Lora_Para_AT.channel = 0;
        Lora_Para_AT.BandWidth = 2;
        Lora_Para_AT.SpreadingFactor = 7;
        // Lora_Para_AT.Power = 14;
        Lora_Para_AT.SAddr = Lora_State.chip_ID;
        Lora_Para_AT.PanID = 0;
        Lora_Para_AT.Net_State = 0;
        Lora_Para_AT.MinRSSI = -40;
    }
    // ��ֵ�Ƿ���Ч��֤
    if (Lora_Para_AT.channel > 100)
        Lora_Para_AT.channel = 0;
    if (Lora_Para_AT.BandWidth > 2)
        Lora_Para_AT.BandWidth = 2;
    if (Lora_Para_AT.SpreadingFactor > 12 || Lora_Para_AT.SpreadingFactor < 7)
        Lora_Para_AT.SpreadingFactor = 7;
    if (Lora_Para_AT.CodingRate > 2 || Lora_Para_AT.CodingRate < 1)
        Lora_Para_AT.CodingRate = 1;
    // if (Lora_Para_AT.Power > 22)
    //     Lora_Para_AT.Power = 14;
    if (Lora_Para_AT.Net_State > 2)
        Lora_Para_AT.Net_State = 0;
    if (Lora_Para_AT.isMaster == true)
    {
        Lora_Para_AT.SAddr = 0;
        if (Lora_Para_AT.PanID == 0xFFFF || Lora_Para_AT.PanID == 0x0000)
            Lora_Para_AT.PanID = Lora_State.chip_ID;
    }
    else
    {
        if (Lora_Para_AT.SAddr == 0xFFFF || Lora_Para_AT.SAddr == 0x0000)
            Lora_Para_AT.SAddr = Lora_State.chip_ID;
        if (Lora_Para_AT.Net_State == Net_NotJoin)
            Lora_Para_AT.PanID = BoardCast;
    }
    if (Lora_Para_AT.Net_State != 2 && Lora_Para_AT.Enable == 1)
    {
        Lora_Para_AT.Net_State = 0;
        Lora_Para_AT.NetOpenTime = 255;
    }
    // ����ͬ��
    Lora_State.Channel = Lora_Para_AT.channel;
    Lora_State.SAddr = Lora_Para_AT.SAddr;
    Lora_State.PanID = Lora_Para_AT.PanID;
    Lora_State.Net_State = Lora_Para_AT.Net_State;
    Lora_State.NetOpenTime = Lora_Para_AT.NetOpenTime;
    Lora_State.isMaster = Lora_Para_AT.isMaster;
    memcpy(&Lora_Para_AT_Last, &Lora_Para_AT, sizeof(Lora_Para_AT));

    // ��ӡȫ������
    Debug_B("Lora_Para_AT.Enable = %d\r\n", Lora_Para_AT.Enable);
    Debug_B("Lora_Para_AT.channel = %d\r\n", Lora_Para_AT.channel);
    Debug_B("Lora_Para_AT.BandWidth = %d\r\n", Lora_Para_AT.BandWidth);
    Debug_B("Lora_Para_AT.SpreadingFactor = %d\r\n", Lora_Para_AT.SpreadingFactor);
    Debug_B("Lora_Para_AT.CodingRate = %d\r\n", Lora_Para_AT.CodingRate);
    // Debug_B("Lora_Para_AT.Power = %d\r\n", Lora_Para_AT.Power);
    Debug_B("Lora_Para_AT.isMaster = %d\r\n", Lora_Para_AT.isMaster);
    Debug_B("Lora_Para_AT.SAddr = %04X\r\n", Lora_Para_AT.SAddr);
    Debug_B("Lora_Para_AT.PanID = %04X\r\n", Lora_Para_AT.PanID);
    Debug_B("Lora_Para_AT.Net_State = %d\r\n", Lora_Para_AT.Net_State);
    Debug_B("Lora_Para_AT.NetOpenTime = %d\r\n", Lora_Para_AT.NetOpenTime);
    if (Lora_Para_AT.isMaster == true)
        Debug_B("Lora_Para_AT.MinRSSI = %d\r\n", Lora_Para_AT.MinRSSI);
}

/**
 * Lora_State_Save
 * @brief Lora�������ݱ���
 * @author Honokahqh
 * @date 2023-08-05X
 */
void Lora_State_Save()
{
    uint8_t temp_data[8];
    memset(temp_data, 0xFF, 8);
    // v1.00->v1.01 ����Lora Enable�����л�lora��485
    temp_data[0] = (Lora_Para_AT.SpreadingFactor & 0x0F) + ((Lora_Para_AT.BandWidth & 0x03) << 4) + 
                    ((Lora_Para_AT.CodingRate & 0X03) << 6); // SF 7~12 BW 0~2
    temp_data[1] = (Lora_Para_AT.channel & 0x7F) + ((Lora_Para_AT.isMaster & 0X01) << 7);                                                    // Channel:0~127
    temp_data[2] = (Lora_Para_AT.Enable & 0x0F) + ((Lora_Para_AT.Net_State & 0x03) << 5);
    if (Lora_Para_AT.isMaster == true)
    {
        temp_data[3] = Lora_Para_AT.MinRSSI >> 8;
        temp_data[4] = Lora_Para_AT.MinRSSI;
    }
    else
    {
        temp_data[3] = Lora_Para_AT.SAddr >> 8;   // ������Ч
        temp_data[4] = Lora_Para_AT.SAddr & 0xFF; // ������Ч
    }
    temp_data[5] = Lora_Para_AT.PanID >> 8;
    temp_data[6] = Lora_Para_AT.PanID & 0xFF;
    temp_data[7] = Lora_Para_AT.NetOpenTime;

    if (page1_offset >= 500)
    {
        page1_offset = 0;
        flash_erase_page(FlashData1_ADDR);
    }
    flash_program_bytes(FlashData1_ADDR + page1_offset * 8, temp_data, 8);
    page1_offset++;
}

/**
 * Lora_AsData_Add
 * @brief ������Ч-��AS�����ڵ����ݱ�����Flash��
 * @param ID:AS����index
 * @author Honokahqh
 * @date 2023-08-05
 */
void Lora_AsData_Add(uint8_t ID)
{
    // 8�ֽ�Mac + 2�ֽ�Saddr + 1�ֽ�ID + 1�ֽ�״̬ + 4�ֽ�ʱ��
    uint8_t temp_data[16];
    if (page2_offset >= 500)
    {
        page2_offset = 0;
        flash_erase_page(FlashData2_ADDR);
    }
    memset(temp_data, 0xFF, 16);
    memcpy(temp_data, Associated_devices[ID].Mac, 8);
    temp_data[8] = Associated_devices[ID].SAddr >> 8;
    temp_data[9] = Associated_devices[ID].SAddr;
    temp_data[10] = ID;
    temp_data[11] = 0xFF; // 0xFF��Ч 0x00Ϊ��Ч
    temp_data[12] = Associated_devices[ID].Timeout >> 24;
    temp_data[13] = Associated_devices[ID].Timeout >> 16;
    temp_data[14] = Associated_devices[ID].Timeout >> 8;
    temp_data[15] = Associated_devices[ID].Timeout;
    flash_program_bytes(FlashData2_ADDR + page2_offset * 8, temp_data, 16);
    page2_offset += 2;
}

/**
 * Lora_AsData_Del
 * @brief ������Ч-��ĳ��ID����ɾ��
 * @param ID:AS����index
 * @author Honokahqh
 * @date 2023-08-05
 */
void Lora_AsData_Del(uint8_t ID)
{
    /* ������������ȫ��дΪ0��ASR6601CBʵ�ʲ���д��һ�κ��ٴ�д��ʧ�� */
    // // ����flash2 �ҵ�ID��Ӧ�����ݣ�������Ϊ��Ч
    // uint8_t temp_data[16];
    // for (uint8_t i = 0; i < 500; i += 2)
    // {
    //     if (*(uint32_t *)(FlashData2_ADDR + i * 8) == 0xFFFFFFFF)
    //         break;
    //     if (*(uint8_t *)(FlashData2_ADDR + i * 8 + 10) == ID)
    //     {
    //         memset(temp_data, 0, 16);
    //         flash_program_bytes(FlashData2_ADDR + i * 8, temp_data, 16);
    //     }
    // }

    /* �������:дһ���µ�����,���ڲ����ݾ�Ϊ0 */
    uint8_t temp_data[16];
    memset(temp_data, 0, 16);
    temp_data[10] = ID;
    temp_data[11] = 0xFF;
    flash_program_bytes(FlashData2_ADDR + page2_offset * 8, temp_data, 16);
    page2_offset += 2;
}

/**
 * Lora_AsData_Syn
 * @brief ������Ч-��flash�ڶ�ȡAS����
 * @author Honokahqh
 * @date 2023-08-05
 */
void Lora_AsData_Syn()
{
    uint8_t ID;
    // �������ӵ��豸����ͬ�� ÿ16���ֽ�һ�����ݣ����ݵ�12���ֽ��Ƿ�Ϊ0�ж������Ƿ���Ч
    for (page2_offset = 0; page2_offset < 500; page2_offset += 2)
    {
        // ����̵�ַΪ0����0xFFFF����break
        if (*(uint32_t *)(FlashData2_ADDR + page2_offset * 8) == 0xFFFFFFFF)
            break;
        if (*(uint8_t *)(FlashData2_ADDR + page2_offset * 8 + 11) == 0xFF &&
            *(uint8_t *)(FlashData2_ADDR + page2_offset * 8 + 10) < Device_Num_Max)
        {
            ID = *(uint8_t *)(FlashData2_ADDR + page2_offset * 8 + 10);
            Associated_devices[ID].Mac[0] = *(uint8_t *)(FlashData2_ADDR + page2_offset * 8);
            Associated_devices[ID].Mac[1] = *(uint8_t *)(FlashData2_ADDR + page2_offset * 8 + 1);
            Associated_devices[ID].Mac[2] = *(uint8_t *)(FlashData2_ADDR + page2_offset * 8 + 2);
            Associated_devices[ID].Mac[3] = *(uint8_t *)(FlashData2_ADDR + page2_offset * 8 + 3);
            Associated_devices[ID].Mac[4] = *(uint8_t *)(FlashData2_ADDR + page2_offset * 8 + 4);
            Associated_devices[ID].Mac[5] = *(uint8_t *)(FlashData2_ADDR + page2_offset * 8 + 5);
            Associated_devices[ID].Mac[6] = *(uint8_t *)(FlashData2_ADDR + page2_offset * 8 + 6);
            Associated_devices[ID].Mac[7] = *(uint8_t *)(FlashData2_ADDR + page2_offset * 8 + 7);
            Associated_devices[ID].SAddr = (*(uint8_t *)(FlashData2_ADDR + page2_offset * 8 + 8) << 8) + (*(uint8_t *)(FlashData2_ADDR + page2_offset * 8 + 9));
            Associated_devices[ID].Timeout = *(uint32_t *)(FlashData2_ADDR + page2_offset * 8 + 12);
        }
    }
    for (uint8_t i = 0; i < Device_Num_Max; i++)
    {
        if (Associated_devices[i].SAddr)
        {
            Debug_B("ID:%d SAddr:%04x Mac:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X\r\n", i, Associated_devices[i].SAddr,
                    Associated_devices[i].Mac[0], Associated_devices[i].Mac[1], Associated_devices[i].Mac[2], Associated_devices[i].Mac[3],
                    Associated_devices[i].Mac[4], Associated_devices[i].Mac[5], Associated_devices[i].Mac[6], Associated_devices[i].Mac[7]);
        }
    }
}

/**
 * mbs_data_syn
 * @brief ��flash�ڶ�ȡMBS���ݣ�ͬ����mbsCoilValue��mbsHoldRegValue������
 * @author Honokahqh
 * @date 2023-08-05
 */
static uint16_t mbs_coil_last[34];
static uint16_t mbs_reg1_last[19];
static uint16_t mbs_reg2_last[30];
void mbs_data_syn()
{
    page2_offset = 0;
    uint16_t i = 0;
    for (page2_offset = 0; page2_offset < 500; page2_offset++)
    {
        if (*(uint32_t *)(FlashMBSDATA_ADDR + page2_offset * 8) == 0xFFFFFFFF)
            break;
        // 0x01:coil �ϼ�34bit 5�ֽ� 0x02:reg(����) 19�ֽ� 0x03:reg2(ӳ���) 30�ֽ�
        else if (*(uint8_t *)(FlashMBSDATA_ADDR + page2_offset * 8 + 0) == 0x01)
        {
            for (i = 0; i < 6; i++)
            {
                mbsCoilValue[i].pData = ((*(uint8_t *)(FlashMBSDATA_ADDR + page2_offset * 8 + 1)) & (0x01 << i))>>i;
            }
            for (i = 0; i < 28; i++)
            {
                mbsCoilValue[i + 6].pData = ((*(uint8_t *)(FlashMBSDATA_ADDR + page2_offset * 8 + 2 + (i / 8))) & (0x01 << (i%8))) >> (i%8);
            }
            // (34 + 1)/(8*8) = 0
            Debug_B("detect coil:%d\r\n", page2_offset);
        }
        else if (*(uint8_t *)(FlashMBSDATA_ADDR + page2_offset * 8 + 0) == 0x02)
        {
            for (i = 0; i < 19; i++)
            {
                mbsHoldRegValue[i].pData = *(uint16_t *)(FlashMBSDATA_ADDR + page2_offset * 8 + 2 + 2*i);
            }
            //(19 + 1)/8 = 2
            Debug_B("detect reg1:%d\r\n", page2_offset);
            page2_offset += 4;
        }
        else if (*(uint8_t *)(FlashMBSDATA_ADDR + page2_offset * 8 + 0) == 0x03)
        {
            for (i = 0; i < 30; i++)
            {
                mbsHoldRegValue[i + 35].pData = *(uint16_t *)(FlashMBSDATA_ADDR + page2_offset * 8 + 2 + 2*i);
            }
            Debug_B("detect reg2:%d\r\n", page2_offset);
            page2_offset += 7;
        }
    }
    //ͬ����last
    Debug_B("Coil Data:");
    for(i = 0;i<34;i++)
    {
        mbs_coil_last[i] = mbsCoilValue[i].pData;
        Debug_B("%d:%d ",i, mbs_coil_last[i]);
    }
    Debug_B("\r\nReg1 Data:");
    for(i = 0;i<19;i++)
    {
        mbs_reg1_last[i] = mbsHoldRegValue[i].pData;
        Debug_B("%d:%d ",i, mbs_reg1_last[i]);
    }
    Debug_B("\r\nReg2 Data:");
    for(i = 0;i<30;i++)
    {
        if(mbsHoldRegValue[i + 35].pData > 30 ) 
        {
            mbsHoldRegValue[i + 35].pData = i;// ����Ĭ��ֵ
            Debug_B("illegal:%d \r\n",i);
        }
            
        mbs_reg2_last[i] = mbsHoldRegValue[i + 35].pData;
        Debug_B("%d:%d ",i, mbs_reg2_last[i]);
    }
}

/**
 * mbs_data_save
 * @brief ��mbsCoilValue��mbsHoldRegValue�����е����ݱ�����flash��
 *       0x01:coil �ϼ�34bit 5�ֽ� 0x02:reg(����) 19�ֽ� 0x03:reg2(ӳ���) 30�ֽ�
 * @author Honokahqh
 * @date 2023-08-05
 */
void mbs_data_save()
{
    uint8_t i,type = 0;
    uint8_t temp_data[64];
    if (page2_offset >= 496)
    {
        page2_offset = 0;
        Debug_B("Erase Page2\r\n");
        memset(mbs_coil_last, 0, sizeof(mbs_coil_last));
        memset(mbs_reg1_last, 0, sizeof(mbs_reg1_last));
        memset(mbs_reg2_last, 0, sizeof(mbs_reg2_last));
        flash_erase_page(FlashData2_ADDR);
    }
    // ���MBS coil��reg1��reg2�����Ƿ�ı�
    for(i = 0;i<34;i++)
    {
        if(mbs_coil_last[i] != mbsCoilValue[i].pData)
        {
            type = 0x01;
            break;
        }
    }
    if(type == 0x01)
    {
        memset(temp_data, 0, 8);
        temp_data[0] = type;
        for (i = 0; i < 6; i++)
        {
            temp_data[1] |= mbsCoilValue[i].pData << i;
        }
        for (i = 0; i < 28; i++)
        {
            temp_data[2 + (i / 8)] |= mbsCoilValue[i + 6].pData << (i%8);
        }
        flash_program_bytes(FlashMBSDATA_ADDR + page2_offset * 8, temp_data, 8);
        Debug_B("mbs_data_save coil:%d\r\n", page2_offset);
        page2_offset++;
    }
    for(i = 0;i<19;i++)
    {
        if(mbs_reg1_last[i] != mbsHoldRegValue[i].pData)
        {
            type = 0x02;
            break;
        }
    }
    if(type == 0x02)
    {
        memset(temp_data, 0, 40);
        temp_data[0] = type;
        for (i = 0; i < 19; i++)
        {
            temp_data[2 + 2*i] = mbsHoldRegValue[i].pData; 
            temp_data[3 + 2*i] = mbsHoldRegValue[i].pData >> 8; 
        }
        flash_program_bytes(FlashMBSDATA_ADDR + page2_offset * 8, temp_data, 40);
        Debug_B("mbs_data_save reg1:%d\r\n", page2_offset);
        page2_offset += 5;
    }
    for(i = 0;i<30;i++)
    {
        if(mbs_reg2_last[i] != mbsHoldRegValue[i + 35].pData)
        {
            type = 0x03;
            break;
        }
    }
    if(type == 0x03)
    {
        memset(temp_data, 0, 64);
        temp_data[0] = type;
        for (i = 0; i < 30; i++)
        {
            temp_data[2 + 2*i] = mbsHoldRegValue[i + 35].pData; 
            temp_data[3 + 2*i] = mbsHoldRegValue[i + 35].pData >> 8; 
        }
        flash_program_bytes(FlashMBSDATA_ADDR + page2_offset * 8, temp_data, 64);
        Debug_B("mbs_data_save reg2:%d\r\n", page2_offset);
        page2_offset += 8;
    }
    // last����ͬ��
    for(i = 0;i<34;i++)
    {
        mbs_coil_last[i] = mbsCoilValue[i].pData;
    }
    for(i = 0;i<19;i++)
    {
        mbs_reg1_last[i] = mbsHoldRegValue[i].pData;
    }   
    for(i = 0;i<30;i++)
    {
        mbs_reg2_last[i] = mbsHoldRegValue[i + 35].pData;
    }   
}

/**
 * IAP_Check
 * @brief ������ת���-��־λ���
 * @author Honokahqh
 * @date 2023-08-05
 */
void IAP_Check()
{
    uint8_t temp_data[8];
#if Lora_Is_APP
    if (*(uint8_t *)(OTA_ADDR + 24) != 0x01)
    {
        flash_erase_page(OTA_ADDR);
        temp_data[0] = 1;
        flash_program_bytes(OTA_ADDR + 24, temp_data, 1);
        Debug_B("APP:OTA Success\r\n");
    }
    if (*(uint8_t *)(OTA_ADDR + 0) != 0x01 || *(uint8_t *)(OTA_ADDR + 8) != 0x01 || *(uint8_t *)(OTA_ADDR + 16) != 0x01 || *(uint8_t *)(OTA_ADDR + 24) != 0x01)
    {
        flash_erase_page(OTA_ADDR);
        temp_data[0] = 1;
        flash_program_bytes(OTA_ADDR + 0, temp_data, 1);
        flash_program_bytes(OTA_ADDR + 8, temp_data, 1);
        flash_program_bytes(OTA_ADDR + 16, temp_data, 1);
        flash_program_bytes(OTA_ADDR + 24, temp_data, 1);
    }
#else

    /* �豸��һ���ϵ� OTA page�����ݾ�Ϊ0xFF */
    if (*(uint8_t *)(OTA_ADDR + 0) == 0xFF)
    {
        Debug_B("First Power On\r\n");
        temp_data[0] = 0x01;
        flash_program_bytes(OTA_ADDR + 0, temp_data, 8);
        boot_to_app(APP_ADDR);
    }
    /* �豸�������� */
    else if (*(uint8_t *)(OTA_ADDR + 24) == 0x01)
    {
        Debug_B("Normal Reboot\r\n");
        boot_to_app(APP_ADDR);
    }
    /* �豸��ҪOTA */
    else if (*(uint8_t *)(OTA_ADDR + 8) == 0x01 && *(uint8_t *)(OTA_ADDR + 16) == 0xFF)
    {
        Debug_B("Need OTA\r\n");
        ymodem_init();
    }
    /* �豸OTAʧ�� */
    else if (*(uint8_t *)(OTA_ADDR + 8) == 0x01 && *(uint8_t *)(OTA_ADDR + 16) != 0xFF)
    {
        Debug_B("OTA Error\r\n");
    }
    else
    {
        Debug_B("Unknow Error\r\n");
    }
#endif
}

/**
 * jumpToApp
 * @brief ��ת��APP
 * @author Honokahqh
 * @date 2023-08-05
 */
void jumpToApp(int addr)
{
    __asm("LDR SP, [R0]");
    __asm("LDR PC, [R0, #4]");
}

/**
 * boot_to_app
 * @brief �޸��ж�ƫ��������ת
 * @author Honokahqh
 * @date 2023-08-05
 */
void boot_to_app(uint32_t addr)
{
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;

    TREMO_REG_WR(CM4_IRQ_CLR, 0xFFFFFFFF); // close interrupts
    TREMO_REG_WR(CM4_IRQ_VECT_BASE, addr); // set VTOR
    jumpToApp(addr);
}