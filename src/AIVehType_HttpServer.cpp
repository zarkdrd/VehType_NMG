/**************************************************
     >Author: zarkdrd
     >Date: 2024-08-16 15:07:40
     >LastEditTime: 2024-08-29 11:31:42
     >LastEditors: zarkdrd
     >Description:
     >FilePath: /VehType_NMG/src/AIVehType_HttpServer.cpp
**************************************************/
#include "AIVehType_HttpServer.h"
#include "AIVehTypeMng.h"
#include "Log_Message.h"
#include "PhotoHander.h"
#include "VehType.h"
#include "AIVehTypeMng.h"
#include "Http_Upload.h"
#include "GetConfigParam.h"
#include "GetTime.h"
#include "convert.h"
#include "VehType.h"

#include <Poco/Base64Decoder.h>
#include <Poco/Base64Encoder.h>
#include <Poco/Exception.h>
#include <Poco/Timestamp.h>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>

using std::string;

extern int g_upload_mode;

AIVT_RequestHandLer::AIVT_RequestHandLer()
{
    m_pregBuff = new char[BUFLEN];
    // 插入键值对，在容器中用键查找值，实现转换
    qmColorNo.insert(std::pair<int, string>(0, "其它"));
    qmColorNo.insert(std::pair<int, string>(1, "黑"));
    qmColorNo.insert(std::pair<int, string>(2, "白"));
    qmColorNo.insert(std::pair<int, string>(3, "蓝"));
    qmColorNo.insert(std::pair<int, string>(4, "黄"));
    qmColorNo.insert(std::pair<int, string>(5, "绿"));
    qmColorNo.insert(std::pair<int, string>(6, "黄绿"));

    qmVehColorNo.insert(std::pair<string, int>("白色", 2));
    qmVehColorNo.insert(std::pair<string, int>("银色", 13));
    qmVehColorNo.insert(std::pair<string, int>("灰色", 3));
    qmVehColorNo.insert(std::pair<string, int>("深灰色", 3));
    qmVehColorNo.insert(std::pair<string, int>("黑色", 1));
    qmVehColorNo.insert(std::pair<string, int>("金色", 6));
    qmVehColorNo.insert(std::pair<string, int>("黄色", 6));
    qmVehColorNo.insert(std::pair<string, int>("橙色", 7));
    qmVehColorNo.insert(std::pair<string, int>("蓝色", 5));
    qmVehColorNo.insert(std::pair<string, int>("青色", 11));
    qmVehColorNo.insert(std::pair<string, int>("绿色", 9));
    qmVehColorNo.insert(std::pair<string, int>("棕色", 8));
    qmVehColorNo.insert(std::pair<string, int>("粉色", 12));
    qmVehColorNo.insert(std::pair<string, int>("红色", 4));
    qmVehColorNo.insert(std::pair<string, int>("紫色", 10));
    qmVehColorNo.insert(std::pair<string, int>("未知", 255));

    qmVehTypeNo.insert(std::pair<string, string>("客一", "1"));
    qmVehTypeNo.insert(std::pair<string, string>("客二", "2"));
    qmVehTypeNo.insert(std::pair<string, string>("客三", "3"));
    qmVehTypeNo.insert(std::pair<string, string>("客四", "4"));
    qmVehTypeNo.insert(std::pair<string, string>("货一", "11"));
    qmVehTypeNo.insert(std::pair<string, string>("货二", "12"));
    qmVehTypeNo.insert(std::pair<string, string>("货三", "13"));
    qmVehTypeNo.insert(std::pair<string, string>("货四", "14"));
    qmVehTypeNo.insert(std::pair<string, string>("货五", "15"));
    qmVehTypeNo.insert(std::pair<string, string>("货六", "16"));
    qmVehTypeNo.insert(std::pair<string, string>("专一", "21"));
    qmVehTypeNo.insert(std::pair<string, string>("专二", "22"));
    qmVehTypeNo.insert(std::pair<string, string>("专三", "23"));
    qmVehTypeNo.insert(std::pair<string, string>("专四", "24"));
    qmVehTypeNo.insert(std::pair<string, string>("专五", "25"));
    qmVehTypeNo.insert(std::pair<string, string>("专六", "26"));
}

void AIVT_RequestHandLer::handleRequest(HTTPServerRequest &request, HTTPServerResponse &response)
{
    // string data(std::istreambuf_iterator<char>(request.stream()));
    std::istreambuf_iterator<char> eos;
    std::string data(std::istreambuf_iterator<char>(request.stream()), eos);
    // std::cout<<"request data:"<<data<<std::endl;
    //  std::cout << "URL:" << request.getURI() << std::endl;
    // log_message(INFO,"VehType URL:%s",request.getURI().c_str());
    //  memset(m_pregBuff, 0, BUFLEN);
    //  std::cout << "m_pregBuff:" << m_pregBuff << std::endl;
    if (request.getURI() == "/RegResult" || request.getURI() == "/VehVideo")
    {
        handleRegResult(data);
    }
    else if (request.getURI() == "/HeartBeat")
    {
        handleHeartBeat(data);
    }
    // else if (request.getURI() == "/VehVideo")
    // {
    //     handleVideoResult(data);
    // }

    response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
    response.setChunkedTransferEncoding(true);                 // 设置分组传输编码
    response.setContentType("application/json;charset=UTF-8"); // 设置内容类型
    response.send();
}

void AIVT_RequestHandLer::handleRegResult(std::string &data)
{
    // 响应参数结构体
    VehType_Info info;
    Poco::JSON::Parser jsonParser;
    log_message(INFO, "handleRegResult:");
    Poco::JSON::Object::Ptr object = jsonParser.parse(data).extract<Poco::JSON::Object::Ptr>();
    // Poco::Dynamic::Var datapParams = object->get("params");
    // Poco::JSON::Object::Ptr objParams = datapParams.extract<Poco::JSON::Object::Ptr>();

    uint8_t dataRelayFlag = 0;
    // 车头车牌号码
    if (object->get("plateNo").isString())
    {
        info.vehPlate = object->get("plateNo").toString();
        dataRelayFlag |= 0x01;
        std::cout << "info.vehPlate:" << info.vehPlate << std::endl;
        // log_message(INFO,"info.vehPlate:%s",info.vehPlate.c_str());
    }

    // 车头车牌颜色
    if (object->get("plateColor").isString())
    {
        string sColorNo = object->get("plateColor").toString();
        int nColorNo = atoi(sColorNo.c_str());
        if (qmColorNo.find(nColorNo) != qmColorNo.end())
        {
            info.vehPlateColor = qmColorNo[nColorNo];
        }
        else
        {
            info.vehPlateColor = "未知";
        }
        dataRelayFlag |= 0x02;
        std::cout << "info.vehPlateColor:" << info.vehPlateColor << std::endl;
        // log_message(INFO,"info.vehPlateColor:%s",info.vehPlateColor.c_str());
    }

    // 收费车型
    if (object->get("vehicleType").isString())
    {
        string sVehicleType = object->get("vehicleType").toString();
        // char buf[8] = {0};
        // charset_convert_UTF8_TO_GBK((char *)sVehicleType.c_str(), strlen(sVehicleType.c_str()), buf, 8);
        // std::cout << "sVehicleType:" << sVehicleType << std::endl;
        if (qmVehTypeNo.find(sVehicleType) != qmVehTypeNo.end())
        {
            info.vehType = qmVehTypeNo[sVehicleType];
        }
        else
        {
            info.vehType = "0"; // 未知车型
        }
        dataRelayFlag |= 0x04;
        std::cout << "info.vehType:" << info.vehType << std::endl;
        // log_message(INFO,"info.vehType:%s",info.vehType.c_str());
    }

    // 是否有车头图像
    if (object->get("headImgBase64").isString())
    {
        info.headPic = object->get("headImgBase64").toString();
        // std::cout<<"info.headPic length:"<< info.headPic.size()<<std::endl;
        log_message(INFO, "info.headPic length:%d", info.headPic.size());
    }

    // 是否有车牌彩图
    if (object->get("plateImgBase64").isString())
    {
        info.platePic = object->get("plateImgBase64").toString();
        // std::cout<<"info.platePic length:"<< info.platePic.size()<<std::endl;
        log_message(INFO, "info.platePic length:%d", info.platePic.size());
    }

    // 是否有车尾图像
    if (object->get("tailImgBase64").isString())
    {
        info.tailPic = object->get("tailImgBase64").toString();
        // std::cout<<"info.tailPic length:"<< info.tailPic.size()<<std::endl;
        log_message(INFO, "info.tailPic length:%d", info.tailPic.size());
    }
    // 是否有车身图像
    if (object->get("sideImgBase64").isString())
    {
        info.wholePic = object->get("sideImgBase64").toString();
        // std::cout<<"info.wholePic length:"<< info.wholePic.size()<<std::endl;
        log_message(INFO, "info.wholePic length:%d", info.wholePic.size());
    }

    // 车身颜色
    if (object->get("vehicleColor").isString())
    {
        string svehicleColor = object->get("vehicleColor").toString();
        // std::cout << "vehicleColor:" << svehicleColor << std::endl;
        if (qmVehColorNo.find(svehicleColor) != qmVehColorNo.end())
        {
            info.vehColor = qmVehColorNo[svehicleColor];
        }
        else
        {
            info.vehColor = "未知";
        }
        // std::cout << "info.vehColor:" << info.vehColor << std::endl;
        log_message(INFO, "info.vehColor:%s", info.vehColor.c_str());
    }

    // 轴数
    if (object->get("axleCount").isNumeric())
    {
        info.axleCount = to_string(object->getValue<int>("axleCount"));
        dataRelayFlag |= 0x08;
        std::cout << "info.axleCount:" << atoi(info.axleCount.c_str()) << std::endl;
        // log_message(INFO,"info.axleCount:%d",atoi(info.axleCount.c_str()));
    }

    // 轴距
    double axleDist = 0;
    if (object->get("axleDist").isNumeric())
    {
        axleDist = object->getValue<double>("axleDist");
        // log_message(INFO,"info.axleDist:%d",atoi(info.axleDist.c_str()));
    }
    info.axleDist = to_string((short)(axleDist * 100));

    // 轴型
    if (object->get("axleType").isString())
    {
        info.axleType = object->get("axleType").toString();
        // log_message(INFO,"info.axleType:%d",atoi(info.axleType.c_str()));
    }

    // 置信度
    if (object->get("confidence").isString())
    {
        info.confidence = object->get("confidence").toString();
        // std::cout << "info.confidence:" << info.confidence << std::endl;
        log_message(INFO, "info.confidence:%s", info.confidence.c_str());
    }

    double vehicleLengthMeter = 0;
    // 是否有车辆尺寸 长
    if (object->get("vehicleLengthMeter").isNumeric())
    {
        vehicleLengthMeter = object->getValue<double>("vehicleLengthMeter");
    }
    double vehicleWidthMeter = 0;
    // 是否有车辆尺寸 宽
    if (object->get("vehicleWidthMeter").isNumeric())
    {
        vehicleWidthMeter = object->getValue<double>("vehicleWidthMeter");
    }
    double vehicleHeightMeter = 0;
    // 是否有车辆尺寸 高
    if (object->get("vehicleHeightMeter").isNumeric())
    {
        vehicleHeightMeter = object->getValue<double>("vehicleHeightMeter");
    }
    info.vehicleLengthMeter = to_string((short)(vehicleLengthMeter * 100));
    info.vehicleWidthMeter = to_string((short)(vehicleWidthMeter * 100));
    info.vehicleHeightMeter = to_string((short)(vehicleHeightMeter * 100));
    info.vehSize = to_string((short)(vehicleLengthMeter * 100) * (short)(vehicleWidthMeter * 100) * (short)(vehicleHeightMeter * 100));
    // std::cout << "info.vehSize(cm):" << info.vehSize << " 车长(cm):" << info.vehicleLengthMeter << " 车宽(cm):" << info.vehicleWidthMeter << " 车高(cm):" << info.vehicleHeightMeter << std::endl;
    log_message(INFO, "info.vehSize(cm):%s 车长(cm):%s 车宽(cm):%s 车高(cm):%s", info.vehSize.c_str(), info.vehicleLengthMeter.c_str(), info.vehicleWidthMeter.c_str(), info.vehicleHeightMeter.c_str());

    // 是否为危化车辆
    string isDG;
    if (object->get("vehicleDG").isString())
    {
        isDG = object->get("vehicleDG").toString();
        // log_message(INFO,"是否为危化品车辆:%s",atoi(info.vehicleDG.c_str()));
    }
    if (isDG == "非危化品")
    {
        info.vehicleDG = 0x00;
    }
    else
    {
        info.vehicleDG = 0x01;
    }

    // 特殊车辆标识
    int isSP;
    if (object->get("vehicleTypeBySide").isNumeric())
    {
        isDG = object->get("vehicleTypeBySide").toString();
        // log_message(INFO,"特殊车辆类型:%d",atoi(info.vehicleTypeBySide.c_str()));
    }
    if (isSP == 8)
    {
        info.vehicleTypeBySide = 0x000001;
    }
    else
    {
        info.vehicleTypeBySide = 0x000000;
    }

    // 车辆速度
    double vehicleSpeed;
    if (object->get("faceSnapTime").isString())
    {
        if (object->get("tailSnapTime").isString())
        {
            if (object->get("vehicleLengthMeter").isString())
            {
                info.faceSnapTime = object->get("faceSnapTime").toString();
                info.tailSnapTime = object->get("tailSnapTime").toString();
            }
        }
    }

    // 车辆品牌型号
    if (object->get("vehicleBand").isString())
    {
        info.vehicleBand = object->get("vehicleBand").toString();
        // log_message(INFO,"车辆品牌:%s",atoi(info.vehicleDG.c_str()));
    }

    // 10位批次码以及视频文件名
    if (object->get("id").isString())
    {
        // 获取当前时间
        std::time_t now = std::time(nullptr);
        std::tm *localTime = std::localtime(&now);

        // 生成前10位的时间字符串
        std::ostringstream axleNumStream;
        axleNumStream << std::put_time(localTime, "%Y%m%d%H");

        // 确保序号是4位，前面补零
        axleNumStream << std::setw(4) << std::setfill('0') << object->get("id").toString();
        info.axleNum = axleNumStream.str();

        // 拼接完整的视频文件名
        std::string videoFileName = "Video_Vehicle_[" + info.axleNum + "].mp4";
        info.axlevideoName = videoFileName;
    }

    // 抓拍时间
    if (object->get("faceSnapTime").isString())
    {
        std::string faceSnapTimeStr = object->get("faceSnapTime");
        std::tm tm = {};                                                            // 使用大括号初始化
        if (strptime(faceSnapTimeStr.c_str(), "%Y-%m-%d %H:%M:%S", &tm) != nullptr) // 解析字符串到tm结构体
        {
            std::time_t timeStamp = mktime(&tm);
            if (timeStamp != -1) // 确保时间戳有效
            {
                unsigned long long unixTimestamp = static_cast<unsigned long long>(timeStamp);
                info.faceSnapTime = std::to_string(unixTimestamp);
            }
            else
            {
                // 处理mktime失败的情况
                info.faceSnapTime = "Invalid timestamp";
            }
        }
        else
        {
            // 处理strptime失败的情况
            info.faceSnapTime = "Parse error";
        }
    }

    // 是否有车牌图片以及二值化图片
    if (object->get("plateImgBase64").isString())
    {
        unsigned long binImageSize = 500 * 1024;
        char *binImage = new char[500 * 1024];
        char *binImageBase64 = new char[500 * 1024 * 2];
        info.platePic = object->get("plateImgBase64").toString();
        genBinaryImg(&info.platePic, info.platePic.size(), binImage, &binImageSize);
        base64_encode(binImage, binImageBase64, binImageSize);
        info.platePic = binImageBase64;
        log_message(INFO, "info.platePic length:%ld", info.platePic.size());
        log_message(INFO, "info.binPic length:%ld", info.binPic.size());
    }

    // 是否有车头图片
    if (object->get("headImgBase64").isString())
    {
        info.headPic = object->get("headImgBase64").toString();
        log_message(INFO, "info.headPic length:%ld", info.headPic.size());
    }

    // 是否有车身图片
    if (object->get("sideImgBase64").isString())
    {
        info.wholePic = object->get("sideImgBase64").toString();
        log_message(INFO, "info.wholePic length:%ld", info.wholePic.size());
    }

    // 是否有车尾图片
    if (object->get("tailImgBase64").isString())
    {
        info.tailPic = object->get("tailImgBase64").toString();
        log_message(INFO, "info.tailPic length:%ld", info.tailPic.size());
    }

    // 是否有五秒短视频
    if (object->get("videoBase64").isString())
    {
        info.vehVideo = object->get("videoBase64").toString();
        // printf("info.vehVideo:%ld\n", info.vehVideo.size());
        log_message(INFO, "info.vehVideo length:%ld", info.vehVideo.size());
    }

    // 厂商编号
    // info.deviceId = "0";

    time_t RealTime = time(NULL);
    // 上传数据时间，用于后续监测删除
    info.nDelTime = RealTime;
    std::cout << "info.nDelTime:" << info.nDelTime << std::endl;
    // log_message(INFO,"info.nDelTime=%ld",info.nDelTime);

    log_message(INFO, "接收到车型数据 车牌:%s 车牌颜色：%s 车型:%s 轴数：%s", info.vehPlate.c_str(), info.vehPlateColor.c_str(), info.vehType.c_str(), info.axleCount.c_str());
    VehType *myVehType = new VehType;
    UploadHttpClient *uploadClient = new UploadHttpClient;
    if (dataRelayFlag == 0x0F)
    {
        // 判断是否为主动模式/等待模式
        if (g_upload_mode == 1) // 主动上传
        {
            string upperIP = convertIP(myVehType->TcpServerIP);        // 获取上位机服务器IP
            int upperPort = convertPort(myVehType->TcpServerPort);     // 获取上位机服务器端口号
            myVehType->VehTypeResultReport(info, 0x01);                // 向上位机发送上报帧
            uploadClient->PostReqImgInfo(upperIP, upperPort, &info);   // Post图片信息到上位机(目前是未处理过的)
            uploadClient->PostReqVideoInfo(upperIP, upperPort, &info); // Post视频信息到上位机(目前是未处理过的)
        }
        else if (g_upload_mode == 2) // 等待指令后再上传
        {
            // 上传数据到链表
            AIVehTypeMng::Instance().PushRegResult(info);
        }
    }
    else
    {
        myVehType->VehTypeResultReport(info, 0x02);
        log_message(ERROR, "车型设备接收数据不全,dataRelayFlag=%02X", dataRelayFlag);
    }

    info.clearData();
}

std::string AIVT_RequestHandLer::currentDateToString()
{
    // static int i = 100000;
    // i++;
    // return to_string(i);
    time_t rawtime;
    struct tm *timeinfo;
    char buffer[80] = {0};

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(buffer, sizeof(buffer), "%Y-%m-%d_%H-%M-%S", timeinfo);
    std::string str(buffer);
    return str;
}

void AIVT_RequestHandLer::handleHeartBeat(std::string &data)
{
    // AIVT_RequestHandLer::GetInstance().UpdateHeartTime();
}

void AIVT_RequestHandLer::handleVideoResult(std::string &data)
{

    Poco::JSON::Parser jsonParser;
    // std::cout << "handleVideoResult" << std::endl;
    log_message(INFO, "handleVideoResult");
    Poco::JSON::Object::Ptr object = jsonParser.parse(data).extract<Poco::JSON::Object::Ptr>();

    // //是否有五秒短视频
    // if (object->get("videoBase64").isString())
    // {
    //     info.vehVideo = object->get("videoBase64").toString();
    //     // printf("info.vehVideo:%ld\n", info.vehVideo.size());
    //     log_message(INFO, "info.vehVideo length:%ld", info.vehVideo.size());
    // }
}

void AIVT_RequestHandLer::saveFileFromBase64(const std::string &source, std::string fileName)
{
    std::istringstream in(source);
    std::ofstream out;
    out.open(fileName, std::ios_base::binary);
    Poco::Base64Decoder b64_d(in);

    std::copy(std::istreambuf_iterator<char>(b64_d),
              std::istreambuf_iterator<char>(),
              std::ostreambuf_iterator<char>(out));

    out.close();
}

// void AIVT_RequestHandLer::genBinaryImg(void *srcImg, unsigned long srcImgSize, void *destImg, unsigned long *destImgSize)
// {
//     // // 加载图片

//     // struct jpeg_decompress_struct decinfo;
//     // struct jpeg_error_mgr jerr;
//     // decinfo.err = jpeg_std_error(&jerr);
//     // jpeg_create_decompress(&decinfo);
//     // jpeg_mem_src(&decinfo, (unsigned char *)srcImg, srcImgSize);
//     // jpeg_read_header(&decinfo, TRUE);
//     // jpeg_start_decompress(&decinfo);
//     // int width = decinfo.output_width;
//     // int height = decinfo.output_height;
//     // unsigned char *data = (unsigned char *)malloc(decinfo.output_height * decinfo.output_width * decinfo.output_components);
//     // unsigned char *line_pointer;
//     // int i = 0;
//     // while (decinfo.output_scanline < decinfo.image_height)
//     // {
//     //     line_pointer = data + i * decinfo.output_width * decinfo.output_components;
//     //     jpeg_read_scanlines(&decinfo, &line_pointer, 1);
//     //     i++;
//     // }
//     // jpeg_finish_decompress(&decinfo);
//     // jpeg_destroy_decompress(&decinfo);

//     // // 转换二值图
//     // unsigned long long nGrayTotal = 0;
//     // unsigned long nPixelCount = 0;
//     // unsigned char *pos;
//     // for (int x = 0; x < width; x++)
//     // {
//     //     for (int y = 0; y < height; y++)
//     //     {
//     //         pos = data + (y * width + x) * 3;
//     //         nGrayTotal += pos[0] + pos[1] + pos[2];
//     //         nPixelCount++;
//     //     }
//     // }

//     // int nGrayscal = nGrayTotal / nPixelCount / 3;
//     // std::cout << "nGrayscal:" << nGrayscal << std::endl;
//     // if (nGrayscal < 20)
//     //     nGrayscal = 20;
//     // else if (nGrayscal > 235)
//     //     nGrayscal = 235;

//     // int nGray = 0;
//     // for (int x = 0; x < width; x++)
//     // {
//     //     for (int y = 0; y < height; y++)
//     //     {
//     //         pos = data + (y * width + x) * 3;
//     //         nGray = (pos[0] + pos[1] + pos[2]) / 3;
//     //         if (nGray > nGrayscal)
//     //             nGray = 255;
//     //         else
//     //             nGray = 0;
//     //         pos[0] = nGray;
//     //         pos[1] = nGray;
//     //         pos[2] = nGray;
//     //     }
//     // }

//     // // 输出图片
//     // struct jpeg_compress_struct cinfo;
//     // cinfo.err = jpeg_std_error(&jerr);
//     // jpeg_create_compress(&cinfo);

//     // memset(destImg, 0, *destImgSize);
//     // jpeg_mem_dest(&cinfo, (unsigned char **)&destImg, destImgSize);

//     // cinfo.image_width = width;
//     // cinfo.image_height = height;
//     // cinfo.in_color_space = JCS_RGB;
//     // cinfo.input_components = 3;
//     // jpeg_set_defaults(&cinfo);
//     // jpeg_start_compress(&cinfo, TRUE);
//     // i = 0;
//     // while (cinfo.next_scanline < cinfo.image_height)
//     // {
//     //     line_pointer = data + i * cinfo.image_width * cinfo.input_components;
//     //     jpeg_write_scanlines(&cinfo, &line_pointer, 1);
//     //     i++;
//     // }
//     // jpeg_finish_compress(&cinfo);
//     // jpeg_destroy_compress(&cinfo);
//     // free(data);
// }

HTTPRequestHandler *RequestHandlerFactory::createRequestHandler(const HTTPServerRequest &request)
{
    return new AIVT_RequestHandLer();
}
