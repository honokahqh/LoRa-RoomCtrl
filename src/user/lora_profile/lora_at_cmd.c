#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "lora_core.h"
// AT Process用于外部控制Lora模块，通过串口发送AT指令，Lora模块接收到指令后，会调用对应的处理函数
// 如果将协议集成到设备，可以直接调parseATCommand(const char *input)，也可以自己处理
// 处理函数原型
typedef void (*AT_Handler)(int parameter);


Lora_Para_AT_t Lora_Para_AT,Lora_Para_AT_Last;

// AT+MASTER:设置为主机，重置网络信息，并重启
// AT+SLVAER:设置为从机，重置网络信息，并重启
// AT+RST:设备复位，将Channel、BW、SF、Power、	PANID、SAddr写入FLash
// AT+CHANNEL:主机有效，设置通讯信道
// AT+BW:设置带宽
// AT+SF:设置扩频因子
// AT+TXPOWER:设置发送功率
// AT+PANID:主机有效，设置网络ID，默认为chipID
// AT+SADDR:从机有效，默认为chipID+PANID
// AT+NetOpen指令：主机有效，重启进入注册模式
// AT+NETCLOSE指令：主机有效，重启进入工作信道
// AT+LEAVE：从机-退出网络
// AT+DELETE：主机删除某个设备
// AT+PRINT:打印当前网络信息
// AT+CONNECT:从机根据PanID发起连接，不填则随机连接开放网络

// 从机flash
// flash1:
// 	channel+bw
// 	SF
// 	power+netstate
// 	saddr
// 	saddr
// 	panid
// 	panid
// 	netopentime	

// 主机flash
// flash1:
// 	channel+bw
// 	sf
// 	power
// 	panid
// 	panid
// 	netopentime
// 	device_num 用于验证
// flash2:
// 	ID为奇数连接的设备信息	Saddr+mac 10位 + ID + isEnable
// 		数据在AS内

const char *AT_CommandList[] = {
    "AT+MASTER",     // 设置为协调器          重启生效，会清除当前网络信息
    "AT+SLAVER", // 设置设备类型          重启生效，会清除当前网络信息
    "AT+RST",      // 重启
    "AT+CHANNEL",  // 设置信道              重启生效
    "AT+BW",       // 设置带宽              重启生效
    "AT+SF",       // 设置发送速率          重启生效
    "AT+TXPOWER",  // 设置发送功率          重启生效
    "AT+PANID",    // 设置PANID             重启生效
    "AT+SADDR",    // 设置短地址            重启生效
    "AT+NETOPEN",  // 开启网络              重启生效    
    "AT+NETCLOSE", // 关闭网络              重启生效
    "AT+LEAVE",    // Endpoint离开网络      直接生效
    "AT+DELETE",    // 删除设备X             直接生效
    "AT+PRINT",    // 打印当前连接的设备短地址       直接生效 ID:0 SADDR:5577 MAC:1234567812345678
    "AT+CONNECT",  // 根据当前的Panid发起连接       直接生效
    "AT+SLEEP",    // 进入睡眠模式                  直接生效
    "AT+MINRSSI",  // 设置最小RSSI值                主机有效，重启生效
    "AT+CR",  // 设置最小RSSI值                主机有效，重启生效
};  

typedef struct
{
    uint8_t index;
    int parameter;
} AT_Command;

AT_Command parseATCommand(char *input)
{
    AT_Command result = {0xFF, -1};

    for (int i = 0; i < sizeof(AT_CommandList) / sizeof(AT_CommandList[0]); i++)
    {
        if (strncmp(input, AT_CommandList[i], strlen(AT_CommandList[i])) == 0)
        {
            result.index = i;
			// Debug_B("AT Index:%d\r\n",result.index);
            const char *param_str = input + strlen(AT_CommandList[i]);
            if (*param_str != '\0')
            {
                result.parameter = atoi(param_str);
            }
            break;
        }
    }
    return result;
}
extern uint16_t page1_offset;

void handleSend(uint8_t *data, uint8_t len)
{
    uint8_t cmd;
    uint16_t DAddr;
    cmd = data[0];
    switch (cmd)
    {
    case Lora_SendData:
        DAddr = data[1] << 8 | data[2];
        CusProfile_Send(DAddr, cmd, &data[3], len - 3, 1);
	    printf("AT+SEND\r\n");
        break;
    case 0x05:
        DAddr = data[1];
        if(DAddr < USER_COIL_NUM)
        {
            mbsCoilValue[DAddr].pData = data[2];
            Debug_B("DAddr:%d pData:%d\r\n",DAddr,mbsCoilValue[DAddr].pData);
        }
        else
        {
            printf("ERROR:No such ADDR.\r\n");
            return;
        }
        break;
    case 0x06:
        DAddr = data[1];
        if(DAddr < USER_HOLDREG_NUM)
        {   
            mbsHoldRegValue[DAddr].pData = data[2] << 8 | data[3];
            Debug_B("DAddr:%d pData:%d\r\n",DAddr,mbsHoldRegValue[DAddr].pData);
        }
        else
        {
            printf("ERROR:No such ADDR.\r\n");
            return; 
        }
        break;
    default:
        printf("ERROR:No such device.\r\n");
        return;
    }
}
void handleMaster(int parameter)
{
    printf("AT+MASTER\r\n");
    Lora_Para_AT.isMaster = true;
    // 清除所有连接的设备信息和flash后重启
    flash_erase_page(FlashData1_ADDR);
    flash_erase_page(FlashData2_ADDR);
    // 将lora硬件参数写入flash1
    Lora_Para_AT.SAddr = 0xFFFF;
    Lora_Para_AT.PanID = 0xFFFF;
    Lora_Para_AT.Net_State = 0;
	Lora_Para_AT.NetOpenTime = 0;
    Lora_Para_AT.MinRSSI = -40;
    page1_offset = 0;
	Lora_State_Save();
    delay_ms(100);
    NVIC_SystemReset();
}

void handleSlaver(int parameter)
{
    printf("Handling AT+ENDPOINT command.\r\n");
    Lora_Para_AT.isMaster = false;
    // 清除所有连接的设备信息和flash后重启
    flash_erase_page(FlashData1_ADDR);
    flash_erase_page(FlashData2_ADDR);
    // 将lora硬件参数写入flash1
    Lora_Para_AT.SAddr = 0xFFFF;
    Lora_Para_AT.PanID = 0xFFFF;
    Lora_Para_AT.Net_State = 0;
	Lora_Para_AT.NetOpenTime = 0;
    page1_offset = 0;
	Lora_State_Save();
	printf("AT+SLAVER\r\n");
    delay_ms(100);
    NVIC_SystemReset();

}

void handleSleep(int parameter)
{
    printf("AT+SLEEP\r\n");
}

void handleRst(int parameter)
{
    uint8_t temp_data[8];
    memset(temp_data, 0xFF, 8);
    Lora_State_Save();
	printf("AT+RST\r\n");
    delay_ms(100);
    NVIC_SystemReset();
}

void handleChannel(int parameter)
{
    if(Lora_Para_AT.isMaster == false)
    {
        printf("ERROR:Only master can change channel.\r\n");
        return;
    }
    if(parameter < 0 || parameter > 100)
    {
        printf("ERROR:Invalid channel number.\r\n");
        return;
    }
    printf("AT+CHANNEL:%d\r\n",parameter);
    Lora_Para_AT.channel = parameter;
}

void handleBw(int parameter)
{
    if(parameter < 0 || parameter > 2)
    {
        printf("ERROR:Invalid bandwidth.\r\n");
        return;
    }
    printf("AT+BW:%d\r\n",parameter);
    Lora_Para_AT.BandWidth = parameter;
}

void handleSf(int parameter)
{
    if(parameter < 7 || parameter > 12)
    {
        printf("ERROR:Invalid spreading factor.\r\n");
        return;
    }
    printf("AT+SF:%d\r\n",parameter);
    Lora_Para_AT.SpreadingFactor = parameter;
}

void handleCR(int parameter)
{
    if(parameter < 0 || parameter > 3)
    {
        printf("ERROR:Invalid spreading factor.\r\n");
        return;
    }
    printf("AT+SF:%d\r\n",parameter);
    Lora_Para_AT.CodingRate = parameter;
}

void handleTxpower(int parameter)
{
    if(parameter < 0 || parameter > 21)
    {
        printf("ERROR:Invalid tx power.\r\n");
        return;
    }
    printf("AT+TXPOWER:%d\r\n",parameter);
    // Lora_Para_AT.Power = parameter;
}

void handlePanid(int parameter)
{
    if(Lora_Para_AT.isMaster == true)
    {
        //0xFFFE为组播ID、0xFFFF为广播ID
        if(parameter < 1 || parameter > 0xFFFD)
        {
            printf("ERROR:Invalid panid number.\r\n");
            return;
        }
    }
    printf("AT+PANID:%04X\r\n",parameter);
    Lora_Para_AT.PanID = parameter;
}

void handleSaddr(int parameter)
{
    if(Lora_Para_AT.isMaster == true)
    {
        printf("ERROR:Invalid device type\r\n");
        return;
    }
    if(parameter < 1 || parameter > 0xFFFD)
    {
        printf("ERROR:Invalid SADDR number.\r\n");
        return;
    }
    printf("AT+SADDR:%04X\r\n",parameter);
    Lora_Para_AT.SAddr = parameter;
}

void handleNetopen(int parameter)
{
    printf("AT+NETOPEN:%d\r\n",parameter%256);
    Lora_Para_AT.NetOpenTime = parameter%256;
}

void handleNetclose(int parameter)
{
    printf("AT+NETCLOSE\r\n");
    Lora_Para_AT.NetOpenTime = 0;
    Lora_State.NetOpenTime = 0;
}

void handleLeave(int parameter)
{
    if(Lora_Para_AT.isMaster == true)
    {
        printf("This command is only for slaver.\r\n");
        return;
    }
    printf("AT+LEAVE\r\n");
    // 处理逻辑...
    PCmd_Slaver_Request_Leave();
}

void handleDelet(int parameter)
{
    if(Lora_Para_AT.isMaster == false)
    {
        printf("This command is only for Master.\r\n");
        return;
    }
    if(parameter < 0 || parameter > Device_Num_Max)
    {
        printf("Invalid delet number.\r\n");
        return;
    }
    printf("AT+DELET:%d\r\n",parameter);
    PCmd_Master_Request_Leave(parameter);
    Lora_AsData_Del(parameter);
    memset(&Associated_devices[parameter], 0, sizeof(associated_devices_t));
}

void handleMinrssi(int parameter)
{
    if(Lora_Para_AT.isMaster == false)
    {
        printf("This command is only for Master.\r\n");
        return;
    }
    printf("AT+MINRSSI:%d\r\n",parameter);
    Lora_Para_AT.MinRSSI = parameter;
}

void handlePrint(int parameter)
{
	if(Lora_Para_AT.isMaster == false)
	{
        if(Lora_State.Net_State == 0)
            printf("AT+PRINT:Net_JoinGateWay\r\n");
        else
            printf("AT+PRINT:net%d PanID:%04x SAddr:%04x channel:%d BW:%d SF:%d CR:%d\r\n",
            Lora_State.Net_State, Lora_State.PanID, Lora_State.SAddr, Lora_State.Channel, 
            Lora_Para_AT.BandWidth, Lora_Para_AT.SpreadingFactor, Lora_Para_AT.CodingRate);
	}
	else
	{
    for(uint8_t i = 0;i < Device_Num_Max;i++)
    {
        if(Associated_devices[i].SAddr != 0 && Associated_devices[i].SAddr != 0xFFFF )
        {
            Debug_B("ID:%d SAddr:%04X Mac:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X\r\n", i, Associated_devices[i].SAddr,
                Associated_devices[i].Mac[0], Associated_devices[i].Mac[1], Associated_devices[i].Mac[2], Associated_devices[i].Mac[3],
                Associated_devices[i].Mac[4], Associated_devices[i].Mac[5], Associated_devices[i].Mac[6], Associated_devices[i].Mac[7]);
        }
    }
}
}



void handleConnect(int parameter)
{
    printf("Connect cmd is not support.\r\n");  
}
// 定义处理函数数组
AT_Handler AT_Handlers[] = {
    handleMaster,
    handleSlaver,
    handleRst,
    handleChannel,
    handleBw,
    handleSf,
    handleTxpower,
    handlePanid,
    handleSaddr,
    handleNetopen,
    handleNetclose,
    handleLeave,
    handleDelet,
    handlePrint,
    handleConnect,
    handleSleep,
    handleMinrssi,
    handleCR,
};

void executeCommand(AT_Command parsed_command)
{
    if (parsed_command.index != 0xFF)
    {
        Debug_B("index:%d \r\n",parsed_command.index);
        AT_Handlers[parsed_command.index](parsed_command.parameter);
    }
    else
    {
        printf("Invalid AT command.\r\n");
    }
}

uint8_t processATCommand(char *input)
{
    AT_Command command = parseATCommand(input);
    if (command.index == 0xFF)
    {
		printf("Invalid AT command.\r\n");
        return 0;
    }
    executeCommand(command);
    return 1;
}