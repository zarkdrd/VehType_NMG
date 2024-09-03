/**************************************************
     >Author: zarkdrd
     >Date: 2024-08-16 15:07:56
     >LastEditTime: 2024-08-26 13:40:40
     >LastEditors: zarkdrd
     >Description:
     >FilePath: /VehType_NMG/src/AIVehType_Interface.cpp
**************************************************/
#include "AIVehType_Interface.h"
#include "AIVehTypeMng.h"
#include "Log_Message.h"
#include <string>

using namespace std;
using namespace Poco::Net;

std::string g_IniFileName = "./Config.ini";

bool D_CALLTYPE AIVT_Init()
{
    // LOG_INIT("AIVTLog");
    AIVehTypeMng::Instance().Init();

    return true;
}

int D_CALLTYPE AIVT_Connect()
{
    // AIVehTypeMng::Instance().SetCameraIP(lpszIP);
    AIVehTypeMng::Instance().StartHttpServer();
    return AIVehTypeMng::Instance().GetDeviceNo();
}

int D_CALLTYPE AIVT_DisConnect(int nCroNO)
{
    AIVehTypeMng::Instance().StopHttpServer();
    return 0;
}

int D_CALLTYPE AIVT_GetState(int nCroNO)
{
    return AIVehTypeMng::Instance().GetState();
}

class MyServerApp
{
public:
    MyServerApp()
    {
        AIVT_Init();               // HTTP初始化
        DeviceNo = AIVT_Connect(); // 连接摄像头
    }

    ~MyServerApp()
    {
        AIVT_DisConnect(DeviceNo);                      // 断开连接
        log_message(INFO, "RECEIVE HTTP SERVER CLOSE"); // 服务器关闭时日志
    }

private:
    int DeviceNo;
};

// void *http_server(void *arg)
// {
//     bool *pIsRunning = (bool *)arg;
//     log_message(INFO, "对接车型平台的http服务器线程启动!");
//     MyServerApp app;
//     while (*pIsRunning)
//     {
//         sleep(1);
//     }
//     log_message(INFO, "对接车型平台的http服务器线程结束...");
//     return NULL;
// }
void http_server(class VehType *myVehType)
{
    log_message(INFO, "对接车型平台的http服务器线程启动!");
    MyServerApp app;
    while (1/*myVehType->StartFlag*/) // 使用对象中的标志变量控制线程的运行
    {
        sleep(1);
    }
    log_message(INFO, "对接车型平台的http服务器线程结束...");
}