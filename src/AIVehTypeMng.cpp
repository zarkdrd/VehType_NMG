/**************************************************
     >Author: zarkdrd
     >Date: 2024-08-16 15:08:08
     >LastEditTime: 2024-08-23 17:35:44
     >LastEditors: zarkdrd
     >Description:
     >FilePath: /VehType_NMG/src/AIVehTypeMng.cpp
**************************************************/
#include "AIVehTypeMng.h"
#include "Log_Message.h"
#include "VehType.h"
#include "AIVehType_HttpServer.h"
#include <Poco/Net/HTTPServerParams.h>
#include <string>
#include <thread>
#include <unistd.h>
using Poco::Net::HTTPServerParams;

extern int g_upload_mode;
pthread_t ClearList_id, UploadResult_id;

AIVehTypeMng::AIVehTypeMng()
{
    m_nServerPort = 0;
    m_pHTTPServer = nullptr;
    m_nDeviceNo = 0;
    m_nStatus = 0;
    m_nMatchCount = 6;
    m_queRegResult.clear();
    m_queRegResultDel.clear();
    m_bExit = false;
}

AIVehTypeMng::~AIVehTypeMng()
{
    m_bExit = true;
    StopHttpServer();
    if (m_pHTTPServer != nullptr)
    {
        delete m_pHTTPServer;
        m_pHTTPServer = nullptr;
    }
}

void *ClearListThread(void *pUser)
{
    log_message(INFO, "检测超时清理车型信息线程启动！");
    AIVehTypeMng *pMng = (AIVehTypeMng *)pUser;
    while (!pMng->m_bExit)
    {
        pMng->ClearTimeoutList();
        sleep(60); //  60s
    }
    log_message(INFO, "检测超时清理车型信息线程结束...");
    return NULL;
}

int AIVehTypeMng::Init()
{
    readIniCfg();
    m_pHTTPServer = new HTTPServer(new RequestHandlerFactory, ServerSocket(m_nServerPort), new HTTPServerParams);
    m_bExit = false;
    if (g_upload_mode == 2)
        pthread_create(&ClearList_id, NULL, ClearListThread, this);
    return 0;
}

void AIVehTypeMng::ClearTimeoutList()
{
    time_t curTime = time(NULL);
    log_message(INFO, "车型链表中有%d个元素", m_queRegResult.size());
    for (list<VehType_Info>::iterator iter = m_queRegResult.begin(); iter != m_queRegResult.end();)
    {
        // log_message(INFO,"curTime - iter->nDelTime = %ld",(long)(curTime - iter->nDelTime));
        if (curTime - iter->nDelTime > 3600)
        { // 3600秒 一小时清一次
            log_message(INFO, "清除超时3600秒车牌%s的车型信息", iter->vehPlate.c_str());
            iter = m_queRegResult.erase(iter);
        }
        else
        {
            iter++;
        }
    }
}

void AIVehTypeMng::readIniCfg()
{
    try
    {
        VehType *myVehType = new VehType;
        m_nServerPort = IniFile::getInstance()->GetIntValue("HTTP", "ServerPort", 5001);
        printf("HTTP.ServerPort=%d\n", m_nServerPort);
        log_message(INFO, "读取到配置文件信息，[HTTP.ServerPort]:%d", m_nServerPort);
    }
    catch (Poco::Exception &exc)
    {
        log_message(ERROR, "Connect Server code=%d,%s", exc.code(), exc.displayText().c_str());
    }
}

int AIVehTypeMng::StartHttpServer()
{
    try
    {
        // StopHttpServer();
        m_pHTTPServer->start(); // 启动http服务器
        printf("http server start\n");
        log_message(INFO, "receive http server start");
    }
    catch (Poco::Exception &exc)
    {
        log_message(ERROR, "Connect Server Exception code=%d,%s", exc.code(), exc.displayText().c_str());
    }
    catch (std::exception &exc)
    {
        log_message(ERROR, "Connect Server exception:%s", exc.what());
    }
    catch (...)
    {
        log_message(ERROR, "unknow error");
    }
    return m_nDeviceNo;
}

void AIVehTypeMng::StopHttpServer()
{
    try
    {
        m_pHTTPServer->stop();
    }
    catch (Poco::Exception &exc)
    {
        log_message(ERROR, "Connect Server Exception code=%d,%s", exc.code(), exc.displayText().c_str());
    }
    catch (std::exception &exc)
    {
        log_message(ERROR, "Connect Server exception:%s", exc.what());
    }
    catch (...)
    {
        log_message(ERROR, "unknow error");
    }
}

void AIVehTypeMng::DealRecvTcpDataThread()
{
    AIVehTypeMng &AIVTMng = AIVehTypeMng::Instance();
}

void AIVehTypeMng::SetCameraIP(std::string sCameraIP)
{
    m_sCameraIP = sCameraIP;
}

int AIVehTypeMng::GetDeviceNo()
{
    return m_nDeviceNo;
}

void AIVehTypeMng::SetCallBack(pAIVTCb AIVTCb)
{
    m_pAIVTCb = AIVTCb;
}

VehType_Info AIVehTypeMng::UploadResult(string param, int method)
{
    log_message(INFO, "通过%s%s 查询对应车型信息", (method == PLATE ? "车牌号:" : "GUID:"), param.c_str());
    // 通过车牌或GUID在队列中查找
    VehType_Info stVehicleInfo = getVehicleInfoFromPlate(param, method);

    return stVehicleInfo;
}

// 获取队列头部的车辆信息
VehType_Info AIVehTypeMng::getHeadVehicleInfo()
{
    VehType_Info stVehicleInfo;
    if (m_queRegResult.empty())
    {
        log_message(INFO, "查询队列为空 返回空");
        return stVehicleInfo;
    }

    return *m_queRegResult.begin();
}

VehType_Info AIVehTypeMng::getVehicleInfoFromPlate(string strPlateNo, int method)
{
    VehType_Info stVehicleInfo;

    // 队列为空返回空
    if (!m_queRegResult.empty())
    {
        // 通过车牌查找
        if (method == PLATE)
        {

            int retry_count = 5;
            while (retry_count)
            {

                for (list<VehType_Info>::reverse_iterator iter = m_queRegResult.rbegin(); iter != m_queRegResult.rend(); iter++)
                {
                    if (strPlateNo == (*iter).vehPlate)
                    {
                        log_message(INFO, "精准查询到结果 车牌:%s GUID:%s 车型:%s", (*iter).vehPlate.c_str(), (*iter).guid.c_str(), (*iter).vehType.c_str());
                        return *iter;
                    }
                }

                // 未找到完全匹配的 按后5位车牌进行匹配 先判断车牌大小是否大于等于5
                if (strPlateNo.size() >= 5)
                {
                    // 取出目标车牌号的后五位
                    std::string targetLastFive = strPlateNo.substr(strPlateNo.size() - 5);

                    for (list<VehType_Info>::reverse_iterator iter = m_queRegResult.rbegin(); iter != m_queRegResult.rend(); iter++)
                    {
                        std::string plate = (*iter).vehPlate;
                        std::string plateLastFive = plate.substr(plate.size() - 5); // 取出列表中车牌号的后五位
                        if (plateLastFive == targetLastFive)
                        {
                            log_message(INFO, "五位匹配查询到结果 车牌:%s GUID:%s 车型:%s", (*iter).vehPlate.c_str(), (*iter).guid.c_str(), (*iter).vehType.c_str());
                            return *iter;
                        }
                    }
                }

                retry_count--;
                sleep(1);
            }
            // 如果仍然无结果则返回队列首辆车型信息
            // Info_Log(string("无结果返回首辆结果 车牌:") + (*m_queRegResult.begin()).vehPlate + string(" GUID:") + (*m_queRegResult.begin()).guid + string(" 车型:") + (*m_queRegResult.begin()).vehType);
            // return *m_queRegResult.begin();
            // 无车牌结果 返回空
            log_message(INFO, "车牌查询无结果 返回空");
            return stVehicleInfo;
        }
        // 通过GUID查找
        else if (method == GUID)
        {
            for (list<VehType_Info>::reverse_iterator iter = m_queRegResult.rbegin(); iter != m_queRegResult.rend(); iter++)
            {
                if (strPlateNo == (*iter).guid)
                {
                    log_message(INFO, "GUID查询到结果 车牌:%s GUID:%s 车型:%s", (*iter).vehPlate.c_str(), (*iter).guid.c_str(), (*iter).vehType.c_str());
                    return *iter;
                }
            }
            // GUID未找到完全匹配的 返回空
            log_message(INFO, "GUID查询无结果 返回空");
            return stVehicleInfo;
        }
    }

    log_message(INFO, "查询队列为空 返回空");
    return stVehicleInfo;
}

VehType_Info AIVehTypeMng::getVehicleInfoFromDelList(string strPlateNo, int method)
{
    if (method == PLATE)
    {
        for (list<SVehDel>::iterator iter = m_queRegResultDel.begin(); iter != m_queRegResultDel.end(); iter++)
        {
            if (strPlateNo == iter->stVehInfo.vehPlate)
            {
                VehType_Info stVehicleInfo = iter->stVehInfo;
                VehType_Info stVehicleInfo1(stVehicleInfo);
                return stVehicleInfo1;
            }
        }
        // 未找到完全匹配的
        VehType_Info stVehicleInfo;
        int nPlateLen = strPlateNo.length();
        if (m_nMatchCount > nPlateLen)
        {
            return stVehicleInfo;
        }
        for (int i = nPlateLen; i >= m_nMatchCount; i--)
        {
            for (list<SVehDel>::iterator iter = m_queRegResultDel.begin(); iter != m_queRegResultDel.end(); iter++)
            {
                if (findMaxCommonStrInBit(strPlateNo, iter->stVehInfo.vehPlate) >= i || findMaxCommonStr(strPlateNo, iter->stVehInfo.vehPlate) >= i)
                {
                    stVehicleInfo = iter->stVehInfo;
                    return stVehicleInfo;
                }
            }
        }
        return stVehicleInfo;
    }
    else
    {
        for (list<SVehDel>::iterator iter = m_queRegResultDel.begin(); iter != m_queRegResultDel.end(); iter++)
        {
            if (strPlateNo == iter->stVehInfo.guid)
            {
                VehType_Info stVehicleInfo = iter->stVehInfo;
                VehType_Info stVehicleInfo1(stVehicleInfo);
                return stVehicleInfo1;
            }
        }
        // 未找到完全匹配的
        VehType_Info stVehicleInfo;

        return stVehicleInfo;
    }
}

int AIVehTypeMng::GetState()
{
    return m_nStatus;
}

void AIVehTypeMng::PushRegResult(VehType_Info &stVehInfo)
{
    // std::cout<<"nDelTime="<<stVehInfo.nDelTime<<std::endl;
    m_queRegResult.push_back(stVehInfo);
    // if (m_queRegResult.size() > 10) {
    //     m_queRegResult.erase(m_queRegResult.begin(), std::next(m_queRegResult.begin(), m_queRegResult.size() - 10));
    // }
    // for (list<VehType_Info>::iterator iter = m_queRegResult.begin(); iter != m_queRegResult.end(); iter++) {
    //     Info_Log(string("已接收数据 车牌:") + (*iter).vehPlate + string(" GUID:") + (*iter).guid + string(" 车型:") + (*iter).vehType);
    // }
}

Poco::JSON::Array AIVehTypeMng::getAllVehicleInfo()
{
    Poco::JSON::Array dataArray;
    for (list<VehType_Info>::iterator iter = m_queRegResult.begin(); iter != m_queRegResult.end(); iter++)
    {
        Poco::JSON::Object dataObject;
        dataObject.set("deviceId", (*iter).deviceId);
        dataObject.set("checkTime", (*iter).checkTime);
        dataObject.set("vehPlate", (*iter).vehPlate);
        dataObject.set("vehPlateColor", (*iter).vehPlateColor);
        dataObject.set("vehType", (*iter).vehType);
        dataObject.set("vehSize", (*iter).vehSize);
        dataObject.set("guid", (*iter).guid);
        dataArray.add(dataObject);
    }

    // 字符串形式
    // std::ostringstream data;
    // dataArray.stringify(data);

    // return data.str();
    return dataArray;
}

int AIVehTypeMng::findMaxCommonStr(string s1, string s2)
{
    if (s1.length() > s2.length())
        swap(s1, s2); // s1用于保存较短的子串
    int len1 = s1.length(), len2 = s2.length();
    int maxLen = 0, start = 0;
    vector<vector<int>> dp(len1 + 1, vector<int>(len2 + 1, 0));
    for (int i = 1; i <= len1; ++i)
    {
        for (int j = 1; j <= len2; ++j)
        {
            if (s1[i - 1] == s2[j - 1])
            {
                dp[i][j] = dp[i - 1][j - 1] + 1;
                if (dp[i][j] > maxLen)
                {
                    maxLen = dp[i][j];
                    start = i - maxLen; // 记录最长公共子串的起始位置
                }
            }
        }
    }
    return maxLen;
}

int AIVehTypeMng::findMaxCommonStrInBit(string s1, string s2)
{
    int nMatchCount = 0;
    for (int i = 0; i < s1.length() && i < s2.length(); i++)
    {
        if (s1.at(i) == s2.at(i))
        {
            nMatchCount++;
        }
    }
    return nMatchCount;
}