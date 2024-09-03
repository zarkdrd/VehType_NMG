/**************************************************
     >Author: zarkdrd
     >Date: 2024-08-13 09:31:00
     >LastEditTime: 2024-08-29 14:07:46
     >LastEditors: zarkdrd
     >Description:
     >FilePath: /VehType_NMG/include/VehType.h
**************************************************/
#ifndef _INC_VEHTYPE_H_
#define _INC_VEHTYPE_H_

#include <thread>
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <string>
#include <cmath>
#include <mutex>
#include <unistd.h> // For sleep()
#include <iostream>
#include "TcpClient.h"
#include "TcpServer.h"
#include "convert.h"
#include "AIVehTypeMng.h"
#include "Http_Upload.h"
#include "AIVehType_Interface.h"
#include "Get_Inifiles.h"
#include "Log_Message.h"
#include "DataCheck.h"
#include "GetTime.h"

#define Server 0x00
#define Client 0x01

class VehType
{
public:
    VehType();
    ~VehType();
    bool ReadConfig(); // 读配置文件
    bool Init();       // 初始化
    bool Start();      // 启动
    int DeviceIPCollection(unsigned char *Cmd, int CmdLen, unsigned char *Oput);
    int DeviceVerification();
    int DeviceUpgrade();
    int InitializeResponse();
    int DeviceIPConfig(unsigned char *Cmd, int CmdLen, unsigned char *Oput);
    int VehTypeResultReport(VehType_Info info, unsigned int AxleFlag);
    int OsdPicProcess(unsigned char *Cmd, int CmdLen, unsigned char *Oput);
    int OsdVideoProcess(unsigned char *Cmd, int CmdLen, unsigned char *Oput);
    int CommunicationSend(unsigned char *Cmd, int CmdLen, int SEQ, int Type);
    int Analysis(unsigned char *Cmd, int CmdLen, unsigned char &Type, unsigned char *Oput);
    int CommunicationAnalysis(unsigned char *Cmd, int CmdLen);
    bool ClientReceiveCmd();
    bool ServerReceiveCmd();
    int SyncTimeWithNTP(const char *ntp_ip);
    void convertAxleTypeToBCD(const std::string &axleType, unsigned char *bcd);
    float calculateSpeed(const std::string &vehicleEnterTime, const std::string &vehicleExitTime, const std::string &distance);

public:
    class TcpServer *myTcpServer;
    class TcpClient *myTcpClient;

private:
    std::thread tcpserverThread;
    std::thread tcpclientThread;
    std::thread monitorThread;
    std::thread xiaoshentongThread;

private:
    std::mutex startFlagMutex; // Mutex for protecting StartFlag
    bool StartFlag;            // 各个函数启动的标志

public:
    unsigned char TcpServerIP[4];            // 车道软件服务器IP
    unsigned char TcpServerPort[2];          // 车道软件服务器端口号
    unsigned char NtpIp[4];                  // 对时服务器IP
    unsigned char LaneHex[5];                // 车道编号
    unsigned char CertificationInfo[8];      // 设备验证信息
    unsigned char DeviceIP[4];               // 设备IP
    unsigned char DevicePort[2];             // 设备服务端端口号
    unsigned char Gateway[4];                // 设备网关
    unsigned char Netmask[4];                // 设备子网掩码
    unsigned int OverstockImageJourCount[2]; // 积压车型识别流水数
    unsigned int OverstockImageCount[2];     // 积压图片数
    unsigned int OverstockVideoCount[2];     // 积压视频数
    unsigned int LightWorkStatus = 0x00;     // 补光灯的工作状态，正常-00H，异常-01H
    string OsdPicTxt;                        // 图片字符叠加指令存储
    string OsdClearPicText;                  // 图片字符叠加清除指令存储
    string OsdVideoTxt;                      // 视频字符叠加指令存储
    string OsdClearVideoTxt;                 // 视频字符叠加清除指令存储
    int TxSEQCount = 1;                      // 发送序号计算
};

void TcpServerThread(class VehType *myVehType);
void TcpClientThread(class VehType *myVehType);
void MonitorThread(class VehType *myVehType);
// void UploadVehTypeInfo(class);
#endif // !_VEHTYPE_