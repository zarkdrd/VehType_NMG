/*************************************************************************
    > File Name: src/Fee_Monitor.c
    > Author: ARTC
    > Descripttion:
    > Created Time: 2023-06-05
 ************************************************************************/

#include "Get_Inifiles.h"
#include "GetConfigParam.h"
#include "Log_Message.h"

// 读取服务器端口号
int GetServerPort(void)
{
    int ServerPort;
    try
    {
        ServerPort = IniFile::getInstance()->GetIntValue("HTTP", "ServerPort", 0);
    }
    catch (string Msg)
    {
        log_message(ERROR, "读取配置文件失败，原因是：%s", Msg.c_str());
        return -1;
    }
    catch (...)
    {
        log_message(ERROR, "读取配置文件失败，原因未知");
        return -1;
    }
    log_message(INFO, "读取到配置文件信息，[HTTP.ServerPort]:%d", ServerPort);

    return ServerPort;
}

// 读取串口文件路径
string GetSerialPath(void)
{
    string SerialPath;
    try
    {
        SerialPath = IniFile::getInstance()->GetStringValue("SERIAL", "SerialPath", "/dev/ttyS3");
    }
    catch (string Msg)
    {
        log_message(ERROR, "读取配置文件失败，原因是：%s", Msg.c_str());
        return NULL;
    }
    catch (...)
    {
        log_message(ERROR, "读取配置文件失败，原因未知");
        return NULL;
    }
    log_message(INFO, "读取到配置文件信息，[SERIAL.SerialPath]:%s", SerialPath.c_str());

    return SerialPath.c_str();
}

// 读取车型设备上传模式
int GetUploadMode()
{
    int UploadMode;
    try
    {
        UploadMode = IniFile::getInstance()->GetIntValue("MODE", "UploadMode", 0);
    }
    catch (string Msg)
    {
        log_message(ERROR, "读取配置文件失败，原因是：%s", Msg.c_str());
        return -1;
    }
    catch (...)
    {
        log_message(ERROR, "读取配置文件失败，原因未知");
        return -1;
    }
    log_message(INFO, "读取到配置文件信息，[MODE.UploadMode]:%d", UploadMode);

    return UploadMode;
}

// 读取心跳开关
int GetHeatSwitch(void)
{
    int HeatSwitch;
    try
    {
        HeatSwitch = IniFile::getInstance()->GetIntValue("MODE", "HeatSwitch", 0);
    }
    catch (string Msg)
    {
        log_message(ERROR, "读取配置文件失败，原因是：%s", Msg.c_str());
        return -1;
    }
    catch (...)
    {
        log_message(ERROR, "读取配置文件失败，原因未知");
        return -1;
    }
    log_message(INFO, "读取到配置文件信息，[MODE.HeatSwitch]:%d", HeatSwitch);

    return HeatSwitch;
}

// 读取本地Udp端口号
int GetLocalPort()
{
    int LocalPort;
    try
    {
        LocalPort = IniFile::getInstance()->GetIntValue("UDP", "LocalPort", 0);
    }
    catch (string Msg)
    {
        log_message(ERROR, "读取配置文件失败，原因是：%s", Msg.c_str());
        return -1;
    }
    catch (...)
    {
        log_message(ERROR, "读取配置文件失败，原因未知");
        return -1;
    }
    log_message(INFO, "读取到配置文件信息，[UDP.LocalPort]:%d", LocalPort);

    return LocalPort;
}

// 读取对方Udp的IP
string GetRemoteIp(void)
{
    string RemoteIp;
    try
    {
        RemoteIp = IniFile::getInstance()->GetStringValue("UDP", "RemoteIp", "getRemoteIp error");
    }
    catch (string Msg)
    {
        log_message(ERROR, "读取配置文件失败，原因是：%s", Msg.c_str());
        return NULL;
    }
    catch (...)
    {
        log_message(ERROR, "读取配置文件失败，原因未知");
        return NULL;
    }
    log_message(INFO, "读取到配置文件信息，[UDP.RemoteIp]:%s", RemoteIp.c_str());

    return RemoteIp.c_str();
}

// 读取对方Udp端口号
int GetRemotePort()
{
    int RemotePort;
    try
    {
        RemotePort = IniFile::getInstance()->GetIntValue("UDP", "RemotePort", 0);
    }
    catch (string Msg)
    {
        log_message(ERROR, "读取配置文件失败，原因是：%s", Msg.c_str());
        return -1;
    }
    catch (...)
    {
        log_message(ERROR, "读取配置文件失败，原因未知");
        return -1;
    }
    log_message(INFO, "读取到配置文件信息，[UDP.RemotePort]:%d", RemotePort);

    return RemotePort;
}

// IP地址的转换：将4个字节的数组转换为标准的IP地址字符串格式。
std::string convertIP(unsigned char ip[])
{
    char ipStr[16]; // 4个字节的IP地址可以用15个字符表示（包括点号和结束符）
    snprintf(ipStr, sizeof(ipStr), "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
    return std::string(ipStr);
}
// 端口号的转换
unsigned int convertPort(unsigned char port[])
{
   // 将高字节和低字节合并为一个 int 型的端口号
    int combinedPort = (static_cast<int>(port[0]) << 8) | static_cast<int>(port[1]);

    return combinedPort;
}