/**************************************************
     >Author: zarkdrd
     >Date: 2024-08-13 09:37:39
     >LastEditTime: 2024-08-22 11:09:38
     >LastEditors: zarkdrd
     >Description: 用于对上位机软件进行数据上传
     >FilePath: /VehType_NMG/include/Http_Upload.h
**************************************************/

#ifndef _LPR_UPLOAD_H_
#define _LPR_UPLOAD_H_

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <iomanip>
#include <sstream>
#include <errno.h>

#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTMLForm.h>
#include <Poco/Net/PartSource.h>
#include <Poco/Net/FilePartSource.h>
#include <Poco/Net/StringPartSource.h>
#include <Poco/StreamCopier.h>
#include <Poco/URI.h>
#include <Poco/JSON/Object.h>
#include <Poco/Timestamp.h>
#include <Poco/DateTimeFormatter.h>
#include <Poco/JSON/Parser.h>
#include <Poco/JSON/Stringifier.h>
#include <opencv2/opencv.hpp>
#include <opencv2/freetype.hpp>
#include <opencv2/imgproc.hpp>
#include "AIVehTypeMng.h"
using std::condition_variable;
using std::map;
using std::mutex;
using std::string;
using std::thread;
using std::vector;

#define TYPEPIC 1
#define TYPEVIDEO 2

class UploadHttpClient
{
public:
     enum ReqType
     {
          INIT,
          BASE_INFO,
          FLOWING_INFO,
          IMG_INFO,
          VIDEO_INFO,
          STATE
     };
     bool PostRequest(string remoteHost, int remotePort, string &bodyStr, string &resultStr, int Type);
     int PostReqImgInfo(string remoteHost, int remotePort, const VehType_Info *response_info);
     int PostReqVideoInfo(string remoteHost, int remotePort, const VehType_Info *response_info);
     bool PushLPRData(VehType_Info &response_info);
     UploadHttpClient();
     ~UploadHttpClient();
     std::string EncodeReqImgInfo(const VehType_Info *response_info);
     std::string EncodeReqVideoInfo(const VehType_Info *response_info);
     bool DecodeResponse(const string &respStr, enum ReqType type);
     string GetFilename(enum ReqType);
     void parseOverlayInstructions(const std::string &instructions, std::vector<std::tuple<int, int, int, std::string>> &overlayTasks);
     void parseClearInstructions(const std::string &instructions, std::vector<std::tuple<int, int, int>> &clearTasks);
     void overlayPicText(cv::Mat &image, const std::vector<std::tuple<int, int, int, std::string>> &overlayTasks);
     void overlayVideoText(cv::Mat &image, const std::vector<std::tuple<int, int, int, std::string>> &overlayTasks);
     void clearPicText(cv::Mat &image, const std::vector<std::tuple<int, int, int>> &clearTasks);
     void clearVideoText(cv::Mat &image, const std::vector<std::tuple<int, int, int>> &clearTasks);
     void processImage(const std::string &imagePath, const std::string &overlayInstructions, const std::string &clearInstructions);
     void processVideo(const std::string &videoPath, const std::string &overlayInstructions, const std::string &clearInstructions);

     // 删除拷贝构造函数和赋值运算符，防止复制实例
     UploadHttpClient(const UploadHttpClient &) = delete;
     UploadHttpClient &operator=(const UploadHttpClient &) = delete;

private:
     string m_RemoteHost;         // 对方上位机服务器的ip
     unsigned short m_RemotePort; // 对方上位机服务器端口号
     int m_driveDir;              // 行驶方向
     string m_cameraNum;          // 相机编号
     string m_ipAddress;          // 猜是相机IP地址
     int m_port;                  // 猜是相机端口号

     thread *LPRDataThread; // 用于上传车牌数据的线程
     thread *deleteThread;  // 用于定时删除过时车牌数据线程
     bool threadFlag;       // 上传线程标志位，用于控制线程结束

     vector<VehType_Info> LPR_Vector; // 用于存放车牌数据的容器
     mutex vectorMutex;               // 用于保护存放车牌数据容器的互斥锁
     condition_variable LPR_CV;       // 通知上传数据给服务器的条件变量
};

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

     /*URL百分号解码，将%E4%BA%ACA99999转成京A99999*/
     // int percent_to_UTF_8(const char input[], char output[],int outLen);

#ifdef __cplusplus
}
#endif // __cplusplus
#endif
