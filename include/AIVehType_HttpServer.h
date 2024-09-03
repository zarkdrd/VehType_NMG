/**************************************************
     >Author: zarkdrd
     >Date: 2024-08-16 15:08:26
     >LastEditTime: 2024-08-29 11:09:16
     >LastEditors: zarkdrd
     >Description:
     >FilePath: /VehType_NMG/include/AIVehType_HttpServer.h
**************************************************/
#ifndef VLPR_HTTPSERVER_INCLUDED
#define VLPR_HTTPSERVER_INCLUDED

#include "Poco/JSON/Parser.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Net/ServerSocket.h"

#include <string>
#include <unordered_map>

using Poco::Net::HTTPRequestHandler;
using Poco::Net::HTTPRequestHandlerFactory;
using Poco::Net::ServerSocket;

using Poco::Net::HTTPServerRequest;
using Poco::Net::HTTPServerResponse;

#define BUFLEN 204800

class AIVT_RequestHandLer : public HTTPRequestHandler
{
public:
    AIVT_RequestHandLer();
    void handleRequest(HTTPServerRequest &request, HTTPServerResponse &response);

private:
    void handleRegResult(std::string &data);
    void handleHeartBeat(std::string &data);
    void handleVideoResult(std::string &data);
    std::string from_base64(const std::string &source);
    // void genBinaryImg(void *srcImg, unsigned long srcImgSize, void *destImg, unsigned long *destImgSize);
    void saveFileFromBase64(const std::string &source, std::string fileName);
    std::string currentDateToString();
    char *m_pregBuff;
    std::unordered_map<int, std::string> qmColorNo;           // 车牌颜色序号转换
    std::unordered_map<std::string, int> qmVehColorNo;        // 车辆颜色序号转换
    std::unordered_map<std::string, std::string> qmVehTypeNo; // 收费车型字符转换
};

class RequestHandlerFactory : public HTTPRequestHandlerFactory
{
public:
    HTTPRequestHandler *createRequestHandler(const HTTPServerRequest &request);
};

// class ResponseMessageHandLer : public HTTPRequestHandler
// {
// public:
//     ResponseMessageHandLer() {}
//     void handleRequest(HTTPServerRequest &request, HTTPServerResponse &response);

// private:
//     // 获取车型基本信息
//     void getVehData(std::string &vehPlate, std::string &vehPlateColor, HTTPServerResponse &response);
//     // 获取车型图片信息
//     void getVehPicVideo(std::string &guid, std::string &type, HTTPServerResponse &response);
//     // 获取车型识别数据
//     void getVehRecord(HTTPServerResponse &response);
// };

// class ResponseMessageFactory : public HTTPRequestHandlerFactory
// {
// public:
//     HTTPRequestHandler *createRequestHandler(const HTTPServerRequest &request);
// };

#endif // VLPR_HTTPSERVER_INCLUDED
