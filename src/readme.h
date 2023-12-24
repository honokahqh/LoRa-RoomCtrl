/* lora中控需要完成事情

1. 检查设备存活,若状态翻转需要上报      房态灯-红外-人体感应-开关面板
2. 对存活设备进行mbs喂狗               房态灯-红外-人体感应-开关面板
3. 对存活设备进行mbs读取数据            人体感应-开关面板
4. 其他数据透传 

1. 检查设备存活,若状态翻转需要上报      房态灯-红外-人体感应-开关面板
    对设备进行轮询,以成功轮询的设备周期为10s,未标记在线的设备时间为255s

程序用JLINK烧录需要64KB对齐,用OTA程序烧录不需要(麻烦)
APP update后需要同步更新Boot cpy一份改lora_core.h中的Lora_Is_APP和ld链接文件中的flash地址
文件list    
    src
        modbus        从机-直接对接lora主机
        modbus_master 主机-轮询子设备用-查版本号和喂狗  
        user    
        Lora_Demo     芯片官方的Lora驱动代码-小修改
        main.c        初始化程序
        readme.h      说明文件
        user
            APP.c     主程序
            lora_profile 自定义通讯协议
*/