/**************************************************
     >Author: zarkdrd
     >Date: 2024-08-16 15:08:48
     >LastEditTime: 2024-08-21 11:21:49
     >LastEditors: zarkdrd
     >Description:
     >FilePath: /VehType_NMG/include/AIVehTypeMng.h
**************************************************/
#ifndef _INCLUDE_AIVEHTYPEMNG_H_
#define _INCLUDE_AIVEHTYPEMNG_H_

#include "Datadef.h"
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPServer.h>
#include <Poco/JSON/Array.h>
#include <list>
#include <string>

using namespace std;
using Poco::Net::HTTPServer;

#define PLATE 1
#define GUID 2

// 上传信息
typedef struct RESPONSE_INFO_FORJSON
{
    string deviceId;           // deviceId   String  厂家编号
    string checkTime;          // checkTime  String  检测时间
    string vehPlate;           // vehPlate	String	车牌号
    string vehPlateColor;      // vehPlateColor String 车牌颜色
    string vehType;            // vehType	String	车型
    string vehicleLengthMeter; // vehicleLengthMeter    String 车长cm
    string vehicleWidthMeter;  // vehicleWidthMeter     String 车宽cm
    string vehicleHeightMeter; // vehicleHeightMeter    String 车高cm
    string vehSize;            // vehSize    String  车辆尺寸
    string vehColor;           // vehColor   String  车辆颜色
    string axleCount;          // axleCount  String  轴数
    string axleDist;           // axleDist	Double	轴距
    string axleType;           // axleType	String	轴型（1-2-3）
    string axleTypeNew;        // axleTypeNew	String	轴型（1-22-222）
    int vehicleDG;             // vehicleDG	String	是否为危化品车辆：危化品，非危化品
    int vehicleTypeBySide;     // 特殊车辆标识，000000H-普通车辆，000001H-集装箱车
    string faceSnapTime;       // 车头抓拍时间
    string tailSnapTime;       // 车尾抓拍时间
    double vehicleSpeed;       // 车辆速度
    string vehicleBand;        // 车辆品牌型号，自定义，由设备上送。例如：宝马 A4L，0000H- 无识别
    string confidence;         //  车牌置信度
    string axleNum;            // 10位批次码
    string axlevideoName;      // 视频文件名
    string snapTime;           // 抓拍时间
    string guid;               // guid       String  车型识别设备车辆信息标识
    string platePic;           // platePic	String	车牌图片信息
    string headPic;            // headPic	String	车头图片信息
    string tailPic;            // tailPic	String	车尾图片信息
    string wholePic;           // wholePic	String	车身图像信息
    string binPic;             // 二值化图片信息
    string vehVideo;           // vehVideo	String	五秒短视频信息

    time_t nDelTime;

    RESPONSE_INFO_FORJSON() = default; // 默认构造函数

    RESPONSE_INFO_FORJSON &operator=(const RESPONSE_INFO_FORJSON &other)
    {
        if (this != &other)
        { // 检查是否为自我赋值
            deviceId = other.deviceId;
            checkTime = other.checkTime;
            vehPlate = other.vehPlate;
            vehPlateColor = other.vehPlateColor;
            vehType = other.vehType;
            vehicleLengthMeter = other.vehicleLengthMeter;
            vehicleWidthMeter = other.vehicleWidthMeter;
            vehicleHeightMeter = other.vehicleHeightMeter;
            vehSize = other.vehSize;
            vehColor = other.vehColor;
            axleCount = other.axleCount;
            axleDist = other.axleDist;
            axleType = other.axleType;
            axleTypeNew = other.axleTypeNew;
            vehicleDG = other.vehicleDG;
            vehicleTypeBySide = other.vehicleTypeBySide;
            faceSnapTime = other.faceSnapTime;
            tailSnapTime = other.tailSnapTime;
            vehicleSpeed = other.vehicleSpeed;
            vehicleBand = other.vehicleBand;
            confidence = other.confidence;
            axleNum = other.axleNum;
            snapTime = other.snapTime;
            guid = other.guid;
            platePic = other.platePic;
            headPic = other.headPic;
            tailPic = other.tailPic;
            wholePic = other.wholePic;
            binPic = other.binPic;
            vehVideo = other.vehVideo;

            nDelTime = other.nDelTime;
            return *this;
        }

        return *this;
    }

    void clearData()
    {
        *this = RESPONSE_INFO_FORJSON(); // 创建一个新实例并赋值给当前实例
    }

} VehType_Info;

typedef struct
{
    time_t nDelTime;         // 删除时间
    VehType_Info stVehInfo; // 车型信息
} SVehDel;

// AIVehTypeMng类，用于管理AIVehTypeMng实例
class AIVehTypeMng
{
public:
    // AIVehTypeMng(const AIVehTypeMng&) = delete;
    // AIVehTypeMng& operator=(const AIVehTypeMng&) = delete;
    static AIVehTypeMng &Instance()
    {
        static AIVehTypeMng instance;
        return instance;
    }

    // 初始化AIVehTypeMng实例
    int Init();

    // 处理接收的tcp数据
    static void DealRecvTcpDataThread();

    // 启动http服务器
    int StartHttpServer();

    // 停止http服务器
    void StopHttpServer();

    // 设置摄像头IP地址
    void SetCameraIP(std::string sCameraIP);

    // 获取设备号
    int GetDeviceNo();

    // 设置回调函数
    void SetCallBack(pAIVTCb AIVTCb);

    // 上传结果(从队列获取车辆信息)
    VehType_Info UploadResult(string param, int method);

    // 获取队列头部的车辆信息
    VehType_Info getHeadVehicleInfo();

    // 获取所有车型信息
    Poco::JSON::Array getAllVehicleInfo();

    // 获取状态
    int GetState();

    // 将注册结果放入队列
    void PushRegResult(VehType_Info &stVehInfo);

    // // 调用回调函数
    // void InvokeCallBack(VehType_Info *pVehicleInfo);

    // 清除超时列表
    void ClearTimeoutList();

private:
    AIVehTypeMng();
    ~AIVehTypeMng();

    // 读取ini配置文件
    void readIniCfg();
    // 根据车牌号获取车型信息
    VehType_Info getVehicleInfoFromPlate(string strPlateNo, int method);
    // 根据车牌号获取车型信息
    VehType_Info getVehicleInfoFromDelList(string strPlateNo, int method);
    // 求两个字符串的最大公共子串
    int findMaxCommonStr(string s1, string s2);
    // 求两个字符串的最大公共子串，考虑字符串长度
    int findMaxCommonStrInBit(string s1, string s2);

private:
    int m_nServerPort;
    int m_nDeviceNo;
    HTTPServer *m_pHTTPServer;
    std::string m_sCameraIP;

    pAIVTCb m_pAIVTCb;
    int m_nStatus;

    list<VehType_Info> m_queRegResult;
    list<SVehDel> m_queRegResultDel;

    int m_nMatchCount;

public:
    bool m_bExit;
};

// // ResponseVehType类，用于管理ResponseVehType实例
// class ResponseVehType
// {
// public:
//     // ResponseVehType(const ResponseVehType&) = delete;
//     // ResponseVehType& operator=(const ResponseVehType&) = delete;
//     static ResponseVehType &Response()
//     {
//         static ResponseVehType response;
//         return response;
//     }

//     // 初始化ResponseVehType实例
//     int Init();

//     // 处理接收的tcp数据
//     static void DealRecvTcpDataThread();

//     // 启动http服务器
//     void StartHttpServer();

//     // 停止http服务器
//     void StopHttpServer();

// private:
//     ResponseVehType();
//     ~ResponseVehType();

//     // 读取ini配置文件
//     void readIniCfg();

// private:
//     int m_nResponsePort;
//     HTTPServer *m_pHTTPServer;

// public:
//     bool m_bExit;
// };
#endif