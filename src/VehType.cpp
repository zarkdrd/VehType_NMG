/**************************************************
     >Author: zarkdrd
     >Date: 2024-08-13 09:30:52
     >LastEditTime: 2024-08-29 15:20:12
     >LastEditors: zarkdrd
     >Description:此文件具体实现车型识别系统的功能
     >FilePath: /VehType_NMG/src/VehType.cpp
**************************************************/
#include "VehType.h"
#include "GetConfigParam.h"

/********************************【配置与初始化】****************************************/
VehType::VehType() {
};

VehType::~VehType()
{
    delete myTcpServer;
    delete myTcpClient;
    StartFlag = false;
}

bool VehType::ReadConfig()
{
    log_message(INFO, "获取配置文件信息...");
    myTcpServer = new TcpServer(IniFile::getInstance()->GetIntValue("Tcp", "DevicePort", 5000));
    log_message(INFO, "获取配置文件已完成！");
    return true;
}

bool VehType::Init()
{
    ReadConfig();
    log_message(INFO, "正在初始化车型识别设备...");

    // 此处启动各个线程
    if (tcpserverThread.joinable() == false)
    {
        log_message(INFO, "此时服务器线程已经启动,正在等待上位机分配IP等重要参数...");
        myTcpServer->Open();
        tcpserverThread = std::thread(TcpServerThread, this); // 程序对车道软件通讯的服务器线程
    }

    while (1)
    {
        startFlagMutex.lock(); // 手动加锁
        if (StartFlag == true)
        {
            if (xiaoshentongThread.joinable() == false)
            {
                xiaoshentongThread = std::thread(http_server, this); // 对接小神瞳系统http服务器的线程
                log_message(INFO, "对接小神瞳系统的服务器线程已经启动！");
            }
            if (tcpclientThread.joinable() == false)
            {
                myTcpClient = new TcpClient(convertIP(TcpServerIP), convertPort(TcpServerPort));
                tcpclientThread = std::thread(TcpClientThread, this); // 程序对车道软件通讯的客户端线程
                log_message(INFO, "对接车道软件的客户端线程已经启动！");
            }
            if (monitorThread.joinable() == false)
            {
                monitorThread = std::thread(MonitorThread, this); // 程序对车道软件的监控（心跳）线程
                log_message(INFO, "对接车道软件的心跳线程已经启动！");
            }
            log_message(INFO, "初始化车型识别设备已完成,所有线程已经启动！");

            break;
        }
        else
        {
            sleep(1);
            continue;
        }
    }
    return true;
}

bool VehType::Start()
{
    myTcpClient->Open();
    while (1)
    {
        if (myTcpServer->isOpen == false)
        {
            // log_message(INFO, "Tcp服务端现在为关闭状态!");
            sleep(1);
            continue;
        }
        if (myTcpClient->isOpen == false)
        {
            // log_message(INFO, "Tcp客户端现在为关闭状态!");
            sleep(1);
            continue;
        }
        sleep(10);
    }
    return true;
}
/***************************************【OVER】**********************************************/
/**************************************【线程区】*********************************************/
// 服务器线程
void TcpServerThread(class VehType *myVehType)
{
    while (1)
    {
        if (myVehType->myTcpServer->isOpen == false)
        {
            usleep(100 * 1000);
            continue;
        }
        myVehType->ServerReceiveCmd();
    }
}
// 客户端线程
void TcpClientThread(class VehType *myVehType)
{
    while (1)
    {
        if (myVehType->myTcpClient->isOpen == false)
        {
            usleep(100 * 1000);
            continue;
        }
        myVehType->ClientReceiveCmd();
    }
}
// 监控（心跳）线程
void MonitorThread(class VehType *myVehType)
{
    myVehType->DeviceVerification(); // 先进行设备验证
    while (1)
    {
        int pos = 0;
        unsigned char timeBuffer[4] = {0};
        unsigned char monitor_data[128] = {0};

        monitor_data[pos++] = 0x0C; // 心跳指令代码

        GetUnixTime(timeBuffer); // 获取时间戳
        memcpy(&monitor_data[pos], &timeBuffer[0], 4);
        pos += 4;

        memcpy(&monitor_data[pos], &myVehType->LaneHex[0], 5); // 车道编号
        pos += 5;

        memcpy(&monitor_data[pos], &myVehType->CertificationInfo[0], 8); // 设备验证信息
        pos += 8;

        monitor_data[pos++] = 0x00; // 保留字节
        monitor_data[pos++] = 0x00;

        auto now = std::chrono::steady_clock::now(); // 获取软件运行到现在的时间
        auto uptime = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
        memcpy(&monitor_data[pos], &uptime, 4); // 设备从开机到现在的运行时间 单位秒
        pos += 4;

        monitor_data[pos++] = 0x00; // 设备整体运行状态，正常-00H，异常-01H默认正常运行

        if (myVehType->myTcpServer->isOpen == true)
        {
            monitor_data[pos++] = 0x00; // 服务器连接正常
        }
        else
        {
            monitor_data[pos++] = 0x01; // 服务器连接异常
        }

        monitor_data[pos++] = 0x01; // 设备模块数量

        monitor_data[pos++] = myVehType->OverstockImageJourCount[0]; // 积压车型识别流水数
        monitor_data[pos++] = myVehType->OverstockImageJourCount[1];

        monitor_data[pos++] = myVehType->OverstockImageCount[0]; // 积压图片数
        monitor_data[pos++] = myVehType->OverstockImageCount[1];

        monitor_data[pos++] = myVehType->OverstockVideoCount[0]; // 积压视频数
        monitor_data[pos++] = myVehType->OverstockVideoCount[1];

        monitor_data[pos++] = myVehType->LightWorkStatus; // 补光灯的工作状态，正常-00H，异常-01H

        myVehType->CommunicationSend(monitor_data, pos, 0xFF, Client);
        // hex_message(INFO, "上传了监视（心跳）数据帧：", monitor_data, pos);
        sleep(5); // 5秒上传一次
    }
}
/***************************************【OVER】**********************************************/
/**************************************【指令帧区】********************************************/
// 设备验证
int VehType::DeviceVerification()
{
    int pos = 0;
    unsigned char timeBuffer[4] = {0};
    unsigned char verification_data[128] = {0};

    verification_data[0] = 0x0E;

    GetUnixTime(timeBuffer); // 获取时间戳
    memcpy(&verification_data[pos], &timeBuffer[0], 4);
    pos += 4;

    memcpy(&verification_data[pos], &LaneHex[0], 5); // 车道编号
    pos += 5;

    memcpy(&verification_data[pos], &CertificationInfo[0], 8); // 设备验证信息
    pos += 8;

    verification_data[pos++] = 0x00; // 保留字节
    verification_data[pos++] = 0x00;

    verification_data[pos++] = 0x01; // 设备功能模块数量N

    verification_data[pos++] = 0xE3; // 功能模块类型编码

    verification_data[pos++] = 0x01; // 功能模块序号

    CommunicationSend(verification_data, pos, 0xFF, Client);
    hex_message(INFO, "进行了设备验证响应：", verification_data, pos);
    return pos;
}

// 设备升级包
int VehType::DeviceUpgrade()
{
    int pos = 0;
    unsigned char timeBuffer[4] = {0};
    unsigned char upgrade_data[128] = {0};

    upgrade_data[pos++] = 0x0F; // 指令代码，此处取值 0FH

    GetUnixTime(timeBuffer); // 获取时间戳
    memcpy(&upgrade_data[pos], &timeBuffer[0], 4);
    pos += 4;

    memcpy(&upgrade_data[pos], &LaneHex[0], 5); // 车道编号
    pos += 5;

    memcpy(&upgrade_data[pos], &CertificationInfo[0], 8); // 设备验证信息
    pos += 8;

    upgrade_data[pos++] = 0x00; // 保留字节
    upgrade_data[pos++] = 0x00;

    upgrade_data[pos++] = 0xE3; // 功能模块类型编码,取值为 E3H

    upgrade_data[pos++] = 0x01; // 功能模块序号

    upgrade_data[pos++] = 0xE3; // 下载升级包版本号注释：10 字节软件版本号组成字段
    upgrade_data[pos++] = 0x00; // （1 字节功能模块类型、2字节厂家代码、2 字节型号代码、
    upgrade_data[pos++] = 0x01; // 4 字节日期 BCD 码（如：20200419）、1 字节版本序列号）
    upgrade_data[pos++] = 0x00;
    upgrade_data[pos++] = 0x01;
    time_t t = time(NULL); // 获取当前系统时间
    struct tm *currentTime = localtime(&t);
    unsigned char bcdTime[4];                                                                                    // 年、月、日转换为BCD
    bcdTime[0] = (((currentTime->tm_year + 1900) / 100 / 10) << 4) | ((currentTime->tm_year + 1900) / 100 % 10); // 年的前两位（世纪）
    bcdTime[1] = (((currentTime->tm_year + 1900) % 100 / 10) << 4) | ((currentTime->tm_year + 1900) % 100 % 10); // 年的后两位
    bcdTime[2] = ((currentTime->tm_mon + 1 / 10) << 4) | (currentTime->tm_mon + 1 % 10);                         // 月
    bcdTime[3] = ((currentTime->tm_mday / 10) << 4) | (currentTime->tm_mday % 10);                               // 日
    memcpy(&upgrade_data[pos], &bcdTime[0], 4);
    pos += 4;
    upgrade_data[pos++] = 0x00;

    CommunicationSend(upgrade_data, pos, 0xFF, Client);
    hex_message(INFO, "进行了设备升级响应：", upgrade_data, pos);
    return pos;
}

// 初始化响应
int VehType::InitializeResponse()
{
    int pos = 0;
    unsigned char timeBuffer[4] = {0};
    unsigned char response_data[128] = {0};

    response_data[pos++] = 0x0F; // 指令代码，此处取值 0FH

    GetUnixTime(timeBuffer); // 获取时间戳
    memcpy(&response_data[pos], &timeBuffer[0], 4);
    pos += 4;

    memcpy(&response_data[pos], &LaneHex[0], 5); // 车道编号
    pos += 5;

    memcpy(&response_data[pos], &CertificationInfo[0], 8); // 设备验证信息
    pos += 8;

    response_data[pos++] = 0x00; // 保留字节
    response_data[pos++] = 0x00;

    response_data[pos++] = 0x01; // 功能模块数量N

    response_data[pos++] = 0xE3; // 1 字节功能模块类型编码
    response_data[pos++] = 0x01; // 1 字节功能模块序号
    response_data[pos++] = 0x00; // 1 字节功能模块状态参数，正常-00H，异常-01H
    response_data[pos++] = 0x00; // 2 字节软件版本号
    response_data[pos++] = 0x01;
    response_data[pos++] = 0x00; // 2 字节固件版本号
    response_data[pos++] = 0x01;
    response_data[pos++] = 0x00; // 2 字节接口协议版本号，默认填写 00H
    response_data[pos++] = 0x00;
    response_data[pos++] = 0x00; // 1 字节功能模块个性化初始化内容字节长度

    CommunicationSend(response_data, pos, 0xFF, Client);
    hex_message(INFO, "进行了初始化响应：", response_data, pos);
    return pos;
}

// 设备IP信息配置
int VehType::DeviceIPConfig(unsigned char *Cmd, int CmdLen, unsigned char *Oput)
{
    int pos = 0;
    unsigned char timeBuffer[4] = {0};
    int TxSEQ = Cmd[3];

    memcpy(&LaneHex[0], &Cmd[13], 5);
    log_message(INFO, "已经记录车道编号:%02X%02X%02X%02X%02X", LaneHex[0], LaneHex[1], LaneHex[2], LaneHex[3], LaneHex[4]);

    memcpy(&CertificationInfo[0], &Cmd[18], 8);
    log_message(INFO, "已经记录设备验证信息:%02X%02X%02X%02X%02X%02X%02X%02X",
                CertificationInfo[0], CertificationInfo[1], CertificationInfo[2], CertificationInfo[3],
                CertificationInfo[4], CertificationInfo[5], CertificationInfo[6], CertificationInfo[7]);

    memcpy(&TcpServerIP[0], &Cmd[28], 4);
    log_message(INFO, "已经记录工作模式车道软件TCP服务器IP:%d.%d.%d.%d", TcpServerIP[0], TcpServerIP[1], TcpServerIP[2], TcpServerIP[3]);

    memcpy(&TcpServerPort[0], &Cmd[32], 1);
    memcpy(&TcpServerPort[1], &Cmd[33], 1);
    unsigned int port = (static_cast<unsigned int>(TcpServerPort[0]) << 8) | static_cast<unsigned int>(TcpServerPort[1]);
    log_message(INFO, "已经记录工作模式车道软件TCP端口号:%u", port);

    memcpy(&NtpIp[0], &Cmd[73], 4);
    log_message(INFO, "已经记录NTP服务器IP:%d.%d.%d.%d", NtpIp[0], NtpIp[1], NtpIp[2], NtpIp[3]);

    Oput[pos++] = 0xA0; // 指令代码，此处取值 A0H

    GetUnixTime(timeBuffer); // 获取时间戳
    memcpy(&Oput[pos], &timeBuffer[0], 4);
    pos += 4;

    memcpy(&Oput[pos], &LaneHex[0], 5); // 车道编号
    pos += 5;

    memcpy(&Oput[pos], &CertificationInfo[0], 8); // 设备验证信息
    pos += 8;

    Oput[pos++] = 0x00; // 保留字节

    Oput[pos++] = 0x00; // 状态码，00H-设置成功，其他-设置失败

    StartFlag = true;
    startFlagMutex.unlock(); // 手动解锁
    sleep(1);
    CommunicationSend(Oput, pos, TxSEQ, Server);
    hex_message(INFO, "回复设备IP设置帧:", Oput, pos);
    return pos;
}

// 上传车型识别结果
int VehType::VehTypeResultReport(VehType_Info info, unsigned int AxleFlag)
{
    int pos = 0;
    unsigned char timeBuffer[4] = {0};
    unsigned char response_data[1024] = {0};

    response_data[pos++] = 0xE3; // 指令代码，此处取值 E3H

    response_data[pos++] = 0x01; // 功能模块序号

    response_data[pos++] = 0x01; // 功能模块功能编号 此处取值 01H

    response_data[pos++] = AxleFlag; // 车型识别结果 00H-未开启 01H-识别成功 02H-识别失败，若不为 01H，则下文车型识别信息默认无效

    if (AxleFlag == 1)
    {
        // 设置车型
        int vehTypeNum = atoi(info.vehType.c_str());
        if (vehTypeNum == 0)
        {
            log_message(ERROR, "车型转换整数失败");
            return -1;
        }
        response_data[pos] = vehTypeNum;
        pos += 1;

        // 设置轴型
        unsigned char bcd[3]; // 存储 BCD 结果的数组
        convertAxleTypeToBCD(info.axleType, bcd);
        memcpy(&response_data[pos], &bcd[0], 3);
        pos += 3;

        // 设置轴数
        memcpy(&response_data[pos], info.axleCount.c_str(), 1); // 轴数
        pos += 1;

        // 设置轴距
        memcpy(&response_data[pos], info.axleDist.c_str(), 2); // 轴距
        pos += 2;

        // 设置车长宽高
        memcpy(&response_data[pos], info.vehicleLengthMeter.c_str(), 2); // 车长
        pos += 2;
        memcpy(&response_data[pos], info.vehicleWidthMeter.c_str(), 2); // 车宽
        pos += 2;
        memcpy(&response_data[pos], info.vehicleHeightMeter.c_str(), 2); // 车高
        pos += 2;

        // 设置是否为危化品
        response_data[pos++] = info.vehicleDG;

        // 设置是否为集装箱车辆
        memcpy(&response_data[pos], &info.vehicleTypeBySide, 3);

        // 设置车牌号
        char vehPlate_utf8[12] = {0};
        char vehPlate_GBK[12] = {0};
        memcpy(vehPlate_utf8, info.vehPlate.c_str(), info.vehPlate.length());
        charset_convert_UTF8_TO_GBK(vehPlate_utf8, sizeof(vehPlate_utf8), vehPlate_GBK, sizeof(vehPlate_GBK));
        memcpy(&response_data[pos], vehPlate_GBK, strlen(vehPlate_GBK));

        // 设置车牌颜色
        int vehColorNum = -1;
        if (info.vehPlateColor == "蓝")
        {
            vehColorNum = 0;
        }
        else if (info.vehPlateColor == "黄")
        {
            vehColorNum = 1;
        }
        else if (info.vehPlateColor == "黑")
        {
            vehColorNum = 2;
        }
        else if (info.vehPlateColor == "白")
        {
            vehColorNum = 3;
        }
        else if (info.vehPlateColor == "黄绿")
        {
            vehColorNum = 5;
        }
        else if (info.vehPlateColor == "绿")
        {
            vehColorNum = 11;
        }
        else
        {
            log_message(ERROR, "颜色未知,发送失败。info.vehColor:%s", info.vehPlateColor.c_str());
            return -1;
        }
        response_data[pos++] = vehColorNum;

        // 设置车辆速度
        int speed = calculateSpeed(info.faceSnapTime, info.tailSnapTime, info.vehicleLengthMeter);
        response_data[pos++] = speed;

        // 设置车辆品牌型号
        memcpy(&response_data[pos], &info.vehicleBand, 2);
        pos += 2;

        // 设置车身颜色
        int bodyColor = -1;
        if (info.vehColor == "2")
        {
            bodyColor = 2; // 白色
        }
        else if (info.vehColor == "13")
        {
            bodyColor = 13; // 银色
        }
        else if (info.vehColor == "3")
        {
            bodyColor = 3; // 灰色深灰色
        }
        else if (info.vehColor == "1")
        {
            bodyColor = 1; // 黑色
        }
        else if (info.vehColor == "6")
        {
            bodyColor = 6; // 黄色金色
        }
        else if (info.vehColor == "7")
        {
            bodyColor = 7; // 橙色
        }
        else if (info.vehColor == "5")
        {
            bodyColor = 5; // 蓝色
        }
        else if (info.vehColor == "11")
        {
            bodyColor = 11; // 青色
        }
        else if (info.vehColor == "9")
        {
            bodyColor = 9; // 绿色
        }
        else if (info.vehColor == "8")
        {
            bodyColor = 8; // 棕色
        }
        else if (info.vehColor == "12")
        {
            bodyColor = 12; // 粉色
        }
        else if (info.vehColor == "4")
        {
            bodyColor = 4; // 红色
        }
        else if (info.vehColor == "10")
        {
            bodyColor = 10; // 紫色
        }
        else
        {
            bodyColor = 255; // 颜色未知
        }
        response_data[pos++] = bodyColor;

        // 设置置信度
        memcpy(&response_data[pos], &info.confidence, 1);
        pos += 1;

        // 设置车脸识别特征码和算法版本号，厂商自定义
        response_data[pos++] = 0x00;
        response_data[pos++] = 0x00;

        // 设置人脸特征码和算法版本号，厂商自定义
        response_data[pos++] = 0x00;
        response_data[pos++] = 0x00;

        // 设置10位批次码
        memcpy(&response_data[pos], &info.axleNum, 7);
        pos += 7;

        // 设置抓拍时间
        memcpy(&response_data[pos], &info.faceSnapTime, 4);
        pos += 4;

        // 设置图像数量
        response_data[pos++] = 0x05;

        // 设置图像大小
        response_data[pos++] = info.platePic.size() / 1024; // 车牌图片大小
        response_data[pos++] = info.headPic.size() / 1024;  // 车头图片大小
        response_data[pos++] = info.tailPic.size() / 1024;  // 车尾图片大小
        response_data[pos++] = info.wholePic.size() / 1024; // 车身图片大小
        response_data[pos++] = info.binPic.size() / 1024;   // 二值化图片大小
        response_data[pos++] = info.vehVideo.size() / 1024; // 5秒过车视频大小

        CommunicationSend(response_data, pos, 0xFF, Client);
        hex_message(INFO, "上传车型识别结果帧:", response_data, pos);
        return pos;
    }
    else
    {
        log_message(WARN, "识别失败,即将返回！");
        return -1;
    }
}
int VehType::OsdPicProcess(unsigned char *Cmd, int CmdLen, unsigned char *Oput)
{
    int TxSEQ = Cmd[3];
    std::string inputStr(reinterpret_cast<const char *>(&Cmd[17]));
    std::istringstream ss(inputStr);
    std::string task;

    while (std::getline(ss, task, '_'))
    {
        std::string tempTask = task;
        std::istringstream taskStream(task);
        std::string part;
        std::vector<std::string> parts;

        // 将每个部分分离出来
        while (std::getline(taskStream, part, '#'))
        {
            parts.push_back(part);
        }

        // 判断最后一个XX是否为0
        if (parts.size() == 3)
        {
            if (parts[2] == "0")
            {
                OsdClearPicText += tempTask + "_";
            }
            else
            {
                OsdPicTxt += tempTask + "_";
            }
        }
    }

    // 移除末尾多余的下划线
    if (!OsdPicTxt.empty())
    {
        OsdPicTxt.pop_back();
    }
    if (!OsdClearPicText.empty())
    {
        OsdClearPicText.pop_back();
    }
    Oput[0] = 0xE3;
    Oput[1] = 0x01;
    Oput[2] = 0x02;
    Oput[3] = 0x00; // 表示控制成功
    CommunicationSend(Oput, 4, TxSEQ, Client);
    hex_message(INFO, "上传图片OSD控制结果帧", Oput, 4);
    return 4;
}
int VehType::OsdVideoProcess(unsigned char *Cmd, int CmdLen, unsigned char *Oput)
{
    int TxSEQ = Cmd[3];
    std::string inputStr(reinterpret_cast<const char *>(&Cmd[17]));
    std::istringstream ss(inputStr);
    std::string task;

    while (std::getline(ss, task, '_'))
    {
        std::string tempTask = task;
        std::istringstream taskStream(task);
        std::string part;
        std::vector<std::string> parts;

        // 将每个部分分离出来
        while (std::getline(taskStream, part, '#'))
        {
            parts.push_back(part);
        }

        // 判断最后一个XX是否为0
        if (parts.size() == 3)
        {
            if (parts[2] == "0")
            {
                OsdClearPicText += tempTask + "_";
            }
            else
            {
                OsdPicTxt += tempTask + "_";
            }
        }
    }

    // 移除末尾多余的下划线
    if (!OsdPicTxt.empty())
    {
        OsdPicTxt.pop_back();
    }
    if (!OsdClearPicText.empty())
    {
        OsdClearPicText.pop_back();
    }
    Oput[0] = 0xE3;
    Oput[1] = 0x01;
    Oput[2] = 0x03;
    Oput[3] = 0x00; // 表示控制成功
    CommunicationSend(Oput, 4, TxSEQ, Client);
    hex_message(INFO, "上传视频OSD控制结果帧", Oput, 4);
    return 4;
}
int VehType::CommunicationSend(unsigned char *Cmd, int CmdLen, int SEQ, int Type)
{
    int DataLen = 0;
    unsigned char SendData[1024];
    unsigned int CRC = 0;

    SendData[DataLen++] = 0xFF; // 帧开始的标志
    SendData[DataLen++] = 0xFF;

    SendData[DataLen++] = 0x00; // 协议版本号

    if (SEQ == 0xFF) // 主动上传
    {
        while (1)
            if (TxSEQCount < 10)
            {
                SendData[DataLen++] = TxSEQCount * 10;
                break;
            }
            else
            {
                TxSEQCount = 1;
                continue;
            }
    }
    else
    {
        SendData[DataLen++] = SEQ; // 帧序号
    }

    SendData[DataLen++] = 0x00; // 帧长度，前两位保留
    SendData[DataLen++] = 0x00;
    SendData[DataLen++] = (CmdLen) / 256;
    SendData[DataLen++] = (CmdLen) % 256;

    memcpy(&SendData[DataLen], &Cmd[0], CmdLen); // DATA域具体命令
    DataLen += CmdLen;

    CRC = CRC16(SendData, DataLen - 2); // CRC校验
    SendData[DataLen++] = CRC / 256;
    SendData[DataLen++] = CRC % 256;

    if (Type == Server)
    {
        myTcpServer->WriteByte(SendData, DataLen);
    }
    else if (Type == Client)
    {
        myTcpClient->Write(SendData, DataLen);
    }

    return 0;
}
/***************************************【OVER】**********************************************/
/**************************************【数据解析区】******************************************/
// 解析功能
int VehType::Analysis(unsigned char *Cmd, int CmdLen, unsigned char &Type, unsigned char *Oput)
{
    if (CmdLen == 0)
    {
        log_message(WARN, "指令帧里无内容！");
        return -1;
    }
    switch (Cmd[8])
    {
    case 0x0A:
        log_message(INFO, "开始设备IP信息配置...");
        DeviceIPConfig(Cmd, CmdLen, Oput);
        break;
    case 0x0B:
        log_message(INFO, "开始设备参数配置...");
        break;
    case 0x0D:
        log_message(INFO, "准备重启中...");
        break;
    case 0x1D:
        log_message(INFO, "准备进行设备IP信息采集...");
        break;
    case 0xE1:
        log_message(INFO, "准备进行设备参数采集...");
        break;
    case 0xC0:
        log_message(INFO, "了解到监控上报结果...");
        break;
    case 0xE0:
        if (Cmd[28] == 0x00)
        {
            log_message(INFO, "了解到设备验证结果:验证成功！");
        }
        else
        {
            log_message(INFO, "了解到设备验证结果:验证失败！重新验证！");
            DeviceVerification();
        }
        break;
    case 0xF0:
        log_message(INFO, "准备进行升级...");
        break;
    case 0x1B:
        log_message(INFO, "准备进行控制设备...");
        break;
    case 0xC1:
        if (Cmd[13] == 0x00)
        {
            log_message(INFO, "了解到了上报的结果:上报成功！");
        }
        else
        {
            log_message(INFO, "了解到了上报的结果:上报失败！");
        }

        break;
    case 0xE3:
        log_message(INFO, "进行车型识别控制...");
        {
            if (Cmd[10] == 0x02)
            {
                log_message(INFO, "进行图片字符叠加的控制...");
                OsdPicProcess(Cmd, CmdLen, Oput);
            }
            if (Cmd[10] == 0x03)
            {
                log_message(INFO, "进行视频字符叠加的控制...");
                OsdVideoProcess(Cmd, CmdLen, Oput);
            }
        }
        break;
    default:
        log_message(WARN, "未能识别的数据帧...");
        break;
    }
    return 0;
}

// 解析数据帧
int VehType::CommunicationAnalysis(unsigned char *Cmd, int CmdLen)
{
    unsigned char Type;
    unsigned char Result[512] = {0};
    unsigned char TxSEQ = Cmd[3];
    switch (Cmd[8])
    {
    case 0x0A:
        hex_message(INFO, "收到设备IP信息设置:", Cmd, CmdLen);
        Type = 0xA0;
        Analysis(&Cmd[0], CmdLen, Type, Result);
        break;
    case 0x0B:
        hex_message(INFO, "收到设备参数设置:", Cmd, CmdLen);
        Type = 0xB0;
        Analysis(&Cmd[0], CmdLen, Type, Result);
        break;
    case 0x0D:
        hex_message(INFO, "收到重启指令帧:", Cmd, CmdLen);
        Type = 0xD0;
        Analysis(&Cmd[0], CmdLen, Type, Result);
        break;
    case 0x1D:
        hex_message(INFO, "收到设备IP信息采集指令:", Cmd, CmdLen);
        Type = 0xD1;
        Analysis(&Cmd[0], CmdLen, Type, Result);
        break;
    case 0xE1:
        hex_message(INFO, "收到设备参数采集:", Cmd, CmdLen);
        Type = 0x1E;
        Analysis(&Cmd[0], CmdLen, Type, Result);
        break;
    case 0xC0:
        hex_message(INFO, "收到监控上报返回帧:", Cmd, CmdLen);
        Type = 0x0C;
        Analysis(&Cmd[0], CmdLen, Type, Result);
        break;
    case 0xE0:
        hex_message(INFO, "收到设备验证结果帧:", Cmd, CmdLen);
        Type = 0x0E;
        Analysis(&Cmd[0], CmdLen, Type, Result);
        break;
    case 0xF0:
        hex_message(INFO, "收到设备升级包下载响应帧:", Cmd, CmdLen);
        Type = 0xF0;
        Analysis(&Cmd[0], CmdLen, Type, Result);
        break;
    case 0x1B:
        hex_message(INFO, "收到控制查询帧:", Cmd, CmdLen);
        Type = 0xB1;
        Analysis(&Cmd[0], CmdLen, Type, Result);
        break;
    case 0xC1:
        hex_message(INFO, "收到上报确认帧:", Cmd, CmdLen);
        Type = 0x1C;
        Analysis(&Cmd[0], CmdLen, Type, Result);
        break;
    case 0xE3:
        hex_message(INFO, "收到车型识别的控制指令：", Cmd, CmdLen);
        Type = 0xE3;
        Analysis(&Cmd[0], CmdLen, Type, Result);
        break;
    default:
        hex_message(ERROR, "不支持的指令帧:", Cmd, CmdLen);
        break;
    }
}

/***************************************【OVER】**********************************************/
/***************************************【功能函数】******************************************/
// 连接NTP服务器对时
int VehType::SyncTimeWithNTP(const char *ntp_ip)
{
    int sockfd;
    struct sockaddr_in server_addr;
    unsigned char ntp_packet[48] = {0};
    struct timeval timeout;
    timeout.tv_sec = 10; // 设置10秒超时
    timeout.tv_usec = 0;

    // 创建套接字
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        std::cerr << "Socket creation failed" << std::endl;
        return -1;
    }

    // 设置超时
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof(timeout));

    // 设置NTP服务器地址
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(123); // NTP端口号123
    server_addr.sin_addr.s_addr = inet_addr(ntp_ip);

    // 设置NTP请求头（模式为3，客户端模式）
    ntp_packet[0] = 0x1B; // LI, Version, Mode

    // 发送请求到NTP服务器
    if (write(sockfd, ntp_packet, sizeof(ntp_packet)) < 0)
    {
        std::cerr << "Failed to send NTP request" << std::endl;
        close(sockfd);
        return -1;
    }

    // 接收NTP服务器的响应
    if (recv(sockfd, ntp_packet, sizeof(ntp_packet), 0) < 0)
    {
        std::cerr << "Failed to receive NTP response" << std::endl;
        close(sockfd);
        return -1;
    }

    // 关闭套接字
    close(sockfd);

    // 提取NTP时间（自1900年1月1日的秒数）
    uint32_t ntp_seconds = (ntp_packet[43] << 24) | (ntp_packet[42] << 16) | (ntp_packet[41] << 8) | ntp_packet[40];

    // 将NTP时间转换为UNIX时间（自1970年1月1日的秒数）
    time_t unix_time = ntp_seconds - 2208988800UL;

    // 校正系统时间
    struct timeval tv;
    tv.tv_sec = unix_time;
    tv.tv_usec = 0;
    if (settimeofday(&tv, NULL) < 0)
    {
        std::cerr << "Failed to set system time" << std::endl;
        return -1;
    }

    std::cout << "System time synchronized successfully." << std::endl;
    return 0;
}
// 将轴型转化为BCD码
void VehType::convertAxleTypeToBCD(const std::string &axleType, unsigned char *bcd)
{
    // 初始化 BCD 结果
    bcd[0] = 0x00;
    bcd[1] = 0x00;
    bcd[2] = 0x00;

    // 遍历轴型字符串，将每个数字转换为 BCD
    int bcdIndex = 0;
    for (char ch : axleType)
    {
        if (isdigit(ch))
        {
            // 将字符转换为对应的整数值
            unsigned char num = ch - '0';
            if (bcdIndex % 2 == 0)
            {
                // 如果是高位，存储在当前字节的高4位
                bcd[bcdIndex / 2] |= (num << 4);
            }
            else
            {
                // 如果是低位，存储在当前字节的低4位
                bcd[bcdIndex / 2] |= num;
            }
            bcdIndex++;
        }
    }
}
// 粗略计算车辆速度（不严谨）
float VehType::calculateSpeed(const std::string &vehicleEnterTime, const std::string &vehicleExitTime, const std::string &distance)
{
    // 解析时间并计算时间差（秒）
    struct tm enterTime = {};
    struct tm exitTime = {};

    strptime(vehicleEnterTime.c_str(), "%Y-%m-%d %H:%M:%S", &enterTime);
    strptime(vehicleExitTime.c_str(), "%Y-%m-%d %H:%M:%S", &exitTime);

    time_t enterTimeT = mktime(&enterTime);
    time_t exitTimeT = mktime(&exitTime);

    double timeDifference = difftime(exitTimeT, enterTimeT); // 时间差，单位为秒

    // 将距离字符串转换为浮点数
    float distanceInMeters = std::stof(distance);

    // 计算速度（千米/小时）
    float speed = (distanceInMeters / timeDifference) * 3.6;

    return speed;
}
/***************************************【OVER】**********************************************/
