#ifndef __LORA_CORE_H
#define __LORA_CORE_H

#include "stdint.h"
#include "string.h"
#include "APP.h"
#include "lora_flash.h"
#include "lora_user.h"
#include "lora_at_cmd.h"
#include "ymodem.h"
#include "SendPacket.h"

#define Lora_Register_Enable 0	//�Ƿ���Loraע���б��ܣ������������ע��ʱ��Ҫ����һ��ע��ʹ��
#define Lora_Always_Master	 0  //�Ƿ�һֱ��Ϊ����
#define Lora_Always_Slaver   1  //�Ƿ�һֱ��Ϊ�ӻ�
#define Power_Saving_Enable  0  //�Ƿ���ʡ��ģʽ
#define Lora_Is_APP			 1 // �Ƿ�ΪAPP,���Ϊboot�������вü���Ч���

#define TRUE            1
#define FALSE           0

#define Device_Num_Max 			32
#define Register_Device_Num_Max 10	

/* DeviceType */
#define Type_Master				0x77
#define Type_RoomCtrl			0x7B
#define Device_Type 			Type_RoomCtrl

#define	BoardCast				0xFFFE	//Panid:ȫ�ŵ��㲥  SAddr:ȫ���㲥

/* Public Cmd 0x01~0x9F*/	
#define BeaconRequest			0x01 	//���豸��������
#define Beacon					0x02 	//�����ظ����豸��������
#define	SlaverInNet				0x03	//�豸������(�ӻ��ظ�)
#define DeviceAnnonce			0x04	//�豸������(�����㲥)

#define Master_Request_Leave	0x11	//����Ҫ�����豸����(��ʱֱ��ɾ������)
#define	Slaver_Leave_ACK		0x12	//���豸�ظ�������(���豸ɾ����������)
#define Slaver_Request_Leave    0x13    //���豸Ҫ������(��ʱֱ��ɾ����������)
#define Master_Leave_ACK        0x14    //�����ظ����豸����Ҫ��(ɾ�����豸����)
#define Lora_Change_Para		0x15	//����Ҫ�����豸�ı�ͨѶ����
#define Lora_Query_RSSI 		0X16	//������ӻ������ź�����
#define Lora_Query_RSSI_ACK		0X17	//������ӻ������ź�����ACK
#define HeartBeat				0x20	//�豸����������

#define Query_Version			0x40	//������ӻ��汾��
#define Query_Version_ACK		0x41	//�ӻ��ظ��汾��
#define Query_SubVersion		0x42	//���������豸�汾��
#define Query_SubVersion_ACK	0x43	//�ӻ��ظ�����
#define Query_Rssi				0x44	//������ӻ������ź�����
#define OTA_Device				0x51	//��������OTA����
#define OTA_Device_ACK			0x52	//�ӻ�ACK
#define OTA_SubDeVice			0x53	//����Ҫ��ӻ������豸OTA����
#define OTA_SubDevice_ACK		0x54	//�ӻ�ACK

#define MBS_Cmd_Request			0x60	//��������͸����MBSָ��
#define MBS_Cmd_ACK				0x61	//�ӻ�����͸����MBSָ��

#define Lora_Para_Set			0x70	//Lora��������
#define Lora_Para_Set_ACK		0x71	//Lora��������ACK

#define Lora_SendData			0x80	//͸��
#define Lora_SendData_ACK		0x81	//͸��

#define Lora_SendPacket_Init	0x82	//�������ݰ�
#define Lora_SendPacket_Init_ACK 0x83	//�������ݰ�ACK
#define Lora_SendPacket_Start	0x84	//�������ݰ�
#define Lora_SendPacket_ing		0x85	//���ź�����
#define Lora_SendPacket_End1	0x86	//�������ݰ����
#define Lora_NeedPacket			0x87	//��������ĳ������
#define Lora_NeedPacket_ACK		0x88	//��������ĳ������ACK
#define Lora_SendPacket_End2 	0x89	//�ӻ����ͽ��ս�����

/* Private Cmd ��׺0xA0~0xEF*/
#define MBS_Switch_CMD			0xA0	//�������״̬�ϱ�
#define MBS_Switch_CMD_ACK		0xA1	//�������״̬�ϱ�ACK
#define MBS_Switch_Query		0XA2	//�������״̬��ѯ

#define MBS_IR_Detect_CMD		0xA4	//����1���״̬�ϱ�
#define MBS_IR_Detect2_CMD		0xA5	//����2���״̬�ϱ�

#define MBS_BLE_CMD				0xA6	//����״̬�ϱ�	

#define MBS_RoomIR_CMD			0xA8	//�������״̬�ϱ�

extern uint8_t RoomCtrlTestMode;
#define RoomCtrl_Test_CMD		0XB0	//�пز���ָ��

#define DeviceTimeout			7200	//�豸ͨѶ��ʱ  ��λ��
#define HeatBeat_Period			60		//���豸���������
#define Register_Timeout		30		//ע�ᳬʱʱ��
#define Leave_Timeout			5		//��������ʱ

/* ����״̬ net_state */
#define Net_NotJoin				0
#define Net_Joining				1
#define Net_JoinGateWay			2
#define Net_Timeout				3

/* ���ݸ�ʽƫ���� */
#define DevType_Addr			0
#define PanIDH_Addr				1
#define PanIDL_Addr				2
#define SAddrH_Addr				3
#define SAddrL_Addr				4
#define DAddrH_Addr				5
#define DAddrL_Addr				6
#define PackID_Addr				7
#define Cmd_Addr				8
#define Len_Addr				9
#define Data_Addr				10

typedef struct
{
	uint8_t isMaster;			//�Ƿ�Ϊ����
	//��������
	uint16_t PanID;
	uint16_t SAddr;			//����������Ķ̵�ַ
	uint16_t chip_ID;			//chipID�ܺ�
	uint8_t Channel;			//ͨѶ�ŵ� 0~25 472~497
	uint8_t DeviceType;
	uint8_t Net_State;			//����״̬ 0:δ���� 1:�����յ��������󲢷��� 2:�������
	uint8_t Mac[8];
	//ͨѶ���ݰ�
	int16_t Rx_RSSI;					//�ź�����
	int16_t Device_RSSI;				//�����ź�����
	uint8_t Rx_Data[255];
	uint8_t Rx_DevType;
	uint16_t Rx_PanID;
	uint16_t Rx_SAddr;
	uint16_t Rx_DAddr;
	uint8_t Rx_CMD;
	uint8_t Rx_PID;
	uint16_t Rx_Len;
	uint8_t Tx_Data[255];
	uint8_t Tx_Len;

	//�ӻ�ר��
	uint8_t Sleep_Timeout;
	uint8_t Wakeup_Event;
	uint8_t Wait_ACK;
	uint8_t ACK_Timeout;		//ACK��ʱ�����ط�
	uint8_t ErrTimes;

	//����ר��
	uint8_t NetOpenTime;		//ע��ʱ��
}Lora_state_t;
extern Lora_state_t Lora_State;

typedef struct
{	
	uint16_t SAddr;         //16λ����̵�ַ
	uint16_t chip_ID;			//chipID�ܺ�
	uint8_t DeviceType;         //�豸����
	uint8_t Net_State;			//����״̬ 0:δ���� 1:�����յ��������󲢷��� 2:�������
	uint8_t Mac[8];
    uint8_t Wait_ACK;           //�������ڵȴ���ACK
	uint8_t ACK_Timeout;		//ACK��ʱ�����ط�
	uint8_t ErrTimes;			//�������	
	int16_t RSSI;				//�ź�����	
	uint8_t IAP_Step;			//IAP״̬ 0:δ��ʼ 1:�������� 2:�յ�����ظ� 3:������������ 4:������� 0x8?:����ʧ��+ԭ��
	uint32_t Timeout;			//�豸TimeOut
}associated_devices_t;
extern associated_devices_t Associated_devices[Device_Num_Max];

typedef struct 
{
	uint16_t SAddr;         //16λ����̵�ַ
	uint16_t chip_ID;
	uint8_t DeviceType;
	int16_t RSSI;
	uint8_t Mac[8];
	uint8_t timeout;
	uint8_t Register_enable;
}register_device_t;
extern register_device_t Register_Device[Register_Device_Num_Max];

extern uint8_t Reset_FLAG;

// ͨ��
void ChipID_Syn();// chipIDͬ��
void Random_Delay(void);// ����ӳ�
uint8_t XOR_Calculate(uint8_t *data,uint8_t len);//Lora���ݰ�XORУ��
void Lora_ReceiveData2State();// Lora��������ͬ��
void CusProfile_Send(uint16_t DAddr, uint8_t cmd, uint8_t *data, uint8_t len, uint8_t isAck);//lora���ݰ�����
void CusProfile_Receive();//���ݰ����մ���

uint8_t Get_IDLE_ID(void);//������ȡ����ID
uint8_t Get_Register_IDLE_ID(void);//������ȡע���б����ID
uint8_t Get_Register_Num(void);//������ȡ��ע���豸����
uint8_t Compare_ShortAddr(uint16_t Short_Addr);//�����Աȶ̵�ַ�Ƿ����б���
uint8_t Compare_Register_SAddr(uint16_t Short_Addr);//�����Աȶ̵�ַ�Ƿ���ע���б���
uint8_t Compare_MAC(uint8_t *Mac);//�����Ա�MAC�Ƿ����б���
uint8_t Compare_Register_MAC(uint8_t *Mac);//�����Ա�MAC�Ƿ���ע���б���
void Lora_DataRetrans_Enable(uint8_t ID,uint8_t Cmd);// Lora���ݰ���ʱ�ط�ʹ��
void Lora_DataRetrans_Disable(uint8_t ID);// Lora���ݰ���ʱ�ط���ֹ

void Set_Sleeptime(uint8_t time);// �ӻ���������ʱ��
void IAP_Data_Re_Request();// �ӻ�����IAP���ݰ�

//����
void Cmd_BeaconRequest(void);
void PCmd_Beacon(void);
void Cmd_SlaverInNet(void);
void PCmd_DeviceAnnonce(void);
void Cmd_HeartBeat(void);

void Cmd_Slaver_Request_Leave(void);
void PCmd_Master_Request_Leave(uint8_t ID);

void PCmd_MBS_Cmd_Request(uint8_t *data,uint8_t len);
void Cmd_MBS_Cmd_ACK();

void Cmd_OTA_Device();
void Cmd_OTA_SubDevice();
void Cmd_Lora_SendData();
// �ӻ�
void Cmd_Beacon(void);
void PCmd_BeaconRequest(void);
void Cmd_DeviceAnnonce(void);
void Cmd_Lora_Change_Para(void);
void PCmd_HeartBeat(void);

void Cmd_Master_Request_Leave(void);
void PCmd_Slaver_Request_Leave(void);

void PCmd_MBS_Cmd_ACK(uint8_t *data,uint8_t len);
void Cmd_MBS_Cmd_Request();

void Slaver_Period_1s(void);
void Master_Period_1s(void);

void PCmd_OTA_SubDeviceAck(uint8_t *data,uint8_t len);

void Cmd_Query_Version();
void Cmd_Query_SubVersion();
void PCmd_Query_SubVersion_ACK();
void Cmd_Query_Rssi();

void Lora_Sleep(void);
#endif

