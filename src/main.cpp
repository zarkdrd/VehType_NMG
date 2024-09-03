/**************************************************
     >Author: zarkdrd
     >Date: 2024-08-13 09:23:32
     >LastEditTime: 2024-08-23 16:45:32
     >LastEditors: zarkdrd
     >Description:
     >FilePath: /VehType_NMG/src/main.cpp
**************************************************/
#include <iostream>
#include <signal.h>
#include "VehType.h"
#include "Log_Message.h"
#include "Auto_version.h"
#include "Get_Inifiles.h"
#include "GetConfigParam.h"
int g_upload_mode = 0; // 初始化上传模式

static void handler(int sig)
{
    if (sig == 2)
    {
        log_message(INFO, "退出当前进程");
    }
    else
    {
        log_message(ERROR, "进程崩溃");
    }

    sleep(2);
    exit(-1);
}
static void Exit(void)
{
    sleep(2);
    exit(0);
}
int main(void)
{
    log_message(INFO, "==== Soft version:%s  Compile Time: %s ====", VERSION, COMPILEDATE);
    IniFile::getInstance()->Load("Config.ini");
    signal(SIGINT, handler);
    signal(SIGSEGV, handler);
    g_upload_mode = GetUploadMode(); // 获取上传模式和心跳开关

    class VehType *myVehType;
    myVehType = new VehType();

    if (myVehType->Init() == false)
    {
        delete myVehType;
        Exit();
    }
    if (myVehType->Start() == true)
    {
        log_message(INFO, "客户端与服务器已全部启动！");
    }

    while (1)
    {
        sleep(10); // 防止程序退出
    }
    return 0;
}
