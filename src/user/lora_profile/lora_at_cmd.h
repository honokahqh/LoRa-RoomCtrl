#ifndef AT_PROCESS_H
#define AT_PROCESS_H
#include "lora_core.h"
#include "lora_user.h"

//��Ҫ�����ȫ������
//LoraӲ������:Ƶ��,����,����,��Ƶ����
//Э�����(ͨ��):�������,�豸����,�豸��ַ,�豸PANID
//Э�����(����):����ʱ��,�����豸�б�(MAC+SADDR)
//Э�����(�ӻ�):�Ƿ�����

typedef struct
{
    uint8_t Enable;
    uint8_t channel;
    uint8_t Power;
    uint8_t BandWidth;
    uint8_t SpreadingFactor;
    uint8_t CodingRate;
    // uint8_t PreambleLength;

    uint8_t isMaster;
    uint16_t PanID;
    uint16_t SAddr;

    short MinRSSI;
    uint8_t NetOpenTime;//��������ʱ��
    uint8_t Net_State;
}Lora_Para_AT_t;
extern Lora_Para_AT_t Lora_Para_AT,Lora_Para_AT_Last;
uint8_t processATCommand(char *input);
void handleSend(uint8_t *data, uint8_t len);
#endif //
