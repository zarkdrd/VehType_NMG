/**************************************************
     >Author: zarkdrd
     >Date: 2024-08-13 09:36:51
     >LastEditTime: 2024-08-22 16:39:22
     >LastEditors: zarkdrd
     >Description: 此文件用于与车道软件上传图片，视频
     >FilePath: /VehType_NMG/src/Http_Upload.cpp
**************************************************/
#include "Http_Upload.h"
#include "Log_Message.h"
#include "GetTime.h"
#include "Auto_version.h"
#include "convert.h"

UploadHttpClient::UploadHttpClient()
{
}
UploadHttpClient::~UploadHttpClient()
{
}
/********************************************封装上传的数据*****************************************************/

// 封装图片上传的JSON数据
std::string UploadHttpClient::EncodeReqImgInfo(const VehType_Info *response_info)
{
     Poco::JSON::Object root;
     Poco::JSON::Array picInfoList;
     Poco::JSON::Object picInfo;

     picInfo.set("deviceType", 0xE3);
     picInfo.set("deviceIndex", 0x01);
     picInfo.set("axleNum", response_info->axleNum);
     picInfo.set("axleTime", response_info->faceSnapTime);
     picInfo.set("licenseCode", response_info->vehPlate);
     picInfo.set("licenseColor", response_info->vehPlateColor);
     picInfo.set("plateImage", response_info->platePic);
     picInfo.set("binPlateImage", response_info->binPic);
     picInfo.set("vehPlateImage", response_info->headPic);
     picInfo.set("vebPlateImage", response_info->wholePic);
     picInfo.set("vetPlateImage", response_info->tailPic);

     picInfoList.add(picInfo);
     root.set("msgid", NULL);
     root.set("picInfoList", picInfoList);

     std::stringstream ss;
     Poco::JSON::Stringifier::stringify(root, ss);
     return ss.str();
}

// 封装视频上传的JSON数据
std::string UploadHttpClient::EncodeReqVideoInfo(const VehType_Info *response_info)
{
     Poco::JSON::Object videoInfo;

     videoInfo.set("msgid", NULL);
     videoInfo.set("deviceType", 0xE3);
     videoInfo.set("deviceIndex", 0x01);
     videoInfo.set("axleNum", response_info->axleNum);
     videoInfo.set("axlevideoName", response_info->axlevideoName);

     std::stringstream ss;
     Poco::JSON::Stringifier::stringify(videoInfo, ss);
     return ss.str();
}

// 解析响应数据
bool UploadHttpClient::DecodeResponse(const string &respStr, enum ReqType type)
{
     // std::cout<<"response:"<<respStr<<std::endl;
     if (respStr.empty())
     {
          log_message(ERROR, "响应为空");
          return false;
     }
     Poco::JSON::Parser jsonParser;
     Poco::JSON::Object::Ptr objParser = jsonParser.parse(respStr).extract<Poco::JSON::Object::Ptr>();

     string info;        // 信息
     string receiveTime; // 接收时间
     if (objParser->get("info").isString())
     {
          info = objParser->getValue<string>("info");
     }
     else
     {
          log_message(ERROR, "info is not String");
          return false;
     }
     if (objParser->get("receiveTime").isString())
     {
          receiveTime = objParser->getValue<string>("receiveTime");
     }
     else
     {
          log_message(ERROR, "receiveTime is not String");
          return false;
     }
     return true;
}

/*********************************************发送数据*************************************************************/
// Post发送数据, Content-Type: multipart/form-data
bool UploadHttpClient::PostRequest(string remoteHost, int remotePort, string &bodyStr, string &resultStr, int Type)
{
     static int count = 0;
     try
     {
          std::string path;
          Poco::URI uri;

          // 指定服务器地址和访问路径
          if (Type == TYPEPIC)
          {
               path = "/axle/axleImage";
          }
          else if (Type == TYPEVIDEO)
          {
               path = "/axle/vehvideo";
          }
          uri.setScheme("http");
          uri.setHost(remoteHost);
          uri.setPort(remotePort);
          uri.setPath(path);

          // 定义客户端
          Poco::Net::HTTPClientSession *http_session = new Poco::Net::HTTPClientSession();
          http_session->setHost(uri.getHost());
          http_session->setPort(uri.getPort());
          http_session->setKeepAlive(true);

          // 定义POST请求
          Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_POST, uri.getPathAndQuery(), Poco::Net::HTTPMessage::HTTP_1_1);
          request.add("Accept", "application/json");
          request.setExpectContinue(true);
          request.add("Connection", "Keep-Alive");

          // 准备multipart form data
          Poco::Net::HTMLForm form(Poco::Net::HTMLForm::ENCODING_MULTIPART);
          form.addPart("json", new Poco::Net::StringPartSource(bodyStr, "application/json"));

          // 将表单数据写入到请求流中，发送数据
          form.prepareSubmit(request);
          std::ostream &os = http_session->sendRequest(request);
          form.write(os);

          // 接收响应
          Poco::Net::HTTPResponse response;
          std::istream &streamIn = http_session->receiveResponse(response);

          if (response.getStatus() != Poco::Net::HTTPResponse::HTTP_OK)
          {
               delete http_session;
               http_session = nullptr;
               log_message(ERROR, "Http 校验返回结果错误: %d", response.getStatus());
               return false;
          }

          std::ostringstream responseStream;
          Poco::StreamCopier::copyStream(streamIn, responseStream);
          resultStr = responseStream.str();

          delete http_session;
          http_session = nullptr;

          if (remoteHost == m_RemoteHost)
          {
               count = 0;
          }
          return true;
     }
     catch (Poco::Exception &exc)
     {
          if (count < 1 && remoteHost == m_RemoteHost)
          {
               log_message(ERROR, "Http Post to %s 出错: code = %d, %s", remoteHost.c_str(), exc.code(), exc.displayText().c_str());
               count = 1;
          }
     }
     catch (std::exception &exc)
     {
          log_message(ERROR, "Http Post出错: %s", exc.what());
     }
     catch (...)
     {
          log_message(ERROR, "Http Post出错: 未知错误类型");
     }

     return false;
}

// 图片上传Post
int UploadHttpClient::PostReqImgInfo(string remoteHost, int remotePort, const VehType_Info *response_info)
{
     string bodyStr, respStr;
     // 封装JSON数据
     EncodeReqImgInfo(response_info);

     // log_message(INFO,"PostReqImgInfo:%s",response_info->LicenseNoU8.c_str());
     // post发送数据
     if (PostRequest(remoteHost, remotePort, bodyStr, respStr, TYPEPIC) == false)
     {
          // printf("body:%s",bodyStr.c_str());
          log_message(ERROR, "PostRequest failed. 请检查网络是否通畅，或对方服务器(%s:%u)是否开启", remoteHost.c_str(), remotePort);
          return -1;
     }
     // 解析响应数据
     if (DecodeResponse(respStr, ReqType::IMG_INFO) == false)
     {
          log_message(ERROR, "respStr:%s", respStr.c_str());
          return -2;
     }
     return 0;
}

// 视频上传Post
int UploadHttpClient::PostReqVideoInfo(string remoteHost, int remotePort, const VehType_Info *response_info)
{
     string bodyStr, respStr;
     // 封装JSON数据
     EncodeReqVideoInfo(response_info);

     // log_message(INFO,"PostReqVideoInfo:%s",response_info->LicenseNoU8.c_str());
     // post发送数据
     if (PostRequest(remoteHost, remotePort, bodyStr, respStr, TYPEVIDEO) == false)
     {
          // printf("body:%s",bodyStr.c_str());
          log_message(ERROR, "PostRequest failed. 请检查网络是否通畅，或对方服务器(%s:%u)是否开启", remoteHost.c_str(), remotePort);
          return -1;
     }
     // 解析响应数据
     if (DecodeResponse(respStr, ReqType::VIDEO_INFO) == false)
     {
          log_message(ERROR, "respStr:%s", respStr.c_str());
          return -2;
     }
     return 0;
}
/*******************************************【处理字符叠加】******************************************************/
// 解析字符叠加指令
void parseOverlayInstructions(const std::string &instructions, std::vector<std::tuple<int, int, int, std::string>> &overlayTasks)
{
     std::istringstream ss(instructions);
     std::string task;
     while (std::getline(ss, task, '_'))
     {
          std::istringstream taskStream(task);
          std::string fontSizeStr, rowStr, colStr, contentStr;
          std::getline(taskStream, fontSizeStr, '#');
          std::getline(taskStream, rowStr, '#');
          std::getline(taskStream, colStr, '#');
          std::getline(taskStream, contentStr, '#');

          int fontSize = std::stoi(fontSizeStr);
          int row = std::stoi(rowStr) - 1; // 行位置从 1 开始
          int col = std::stoi(colStr) - 1; // 列位置从 1 开始

          overlayTasks.push_back(std::make_tuple(fontSize, row, col, contentStr));
     }
}

// 解析字符清除指令
void parseClearInstructions(const std::string &instructions, std::vector<std::tuple<int, int, int>> &clearTasks)
{
     std::istringstream ss(instructions);
     std::string task;
     while (std::getline(ss, task, '_'))
     {
          std::istringstream taskStream(task);
          std::string rowStr, colStr, clearTypeStr;
          std::getline(taskStream, rowStr, '#');
          std::getline(taskStream, colStr, '#');
          std::getline(taskStream, clearTypeStr, '#');

          int row = std::stoi(rowStr) - 1; // 行位置从 1 开始
          int col = std::stoi(colStr) - 1; // 列位置从 1 开始
          int clearType = std::stoi(clearTypeStr);

          clearTasks.push_back(std::make_tuple(row, col, clearType));
     }
}

// 在图片上叠加中文文字
void overlayImageText(cv::Mat &image, const std::vector<std::tuple<int, int, int, std::string>> &overlayTasks, cv::Ptr<cv::freetype::FreeType2> &ft2)
{
     for (const auto &task : overlayTasks)
     {
          int fontSize = (std::get<0>(task)) * 3;
          int row = std::get<1>(task);
          int col = std::get<2>(task);
          std::string content = std::get<3>(task);

          cv::Point position(col * 200, row * 50 + 20);
          ft2->putText(image, content, position, fontSize, cv::Scalar(255, 255, 255), -1, cv::LINE_AA, true);
     }
}

// 清除图片上的文字
void clearImageText(cv::Mat &image, const std::vector<std::tuple<int, int, int>> &clearTasks)
{
     for (const auto &task : clearTasks)
     {
          int row = std::get<0>(task);
          int col = std::get<1>(task);
          int clearType = std::get<2>(task);
          if (clearType == 0)
          {
               cv::Point position(col * 200, row * 50);
               cv::rectangle(image, cv::Rect(position.x, position.y - 20, 200, 50), cv::Scalar(0, 0, 0), cv::FILLED);
          }
     }
}

// 在视频帧上叠加中文文字
void overlayVideoText(cv::Mat &frame, const std::vector<std::tuple<int, int, int, std::string>> &overlayTasks, cv::Ptr<cv::freetype::FreeType2> &ft2)
{
     for (const auto &task : overlayTasks)
     {
          int fontSize = (std::get<0>(task)) * 3;
          int row = std::get<1>(task);
          int col = std::get<2>(task);
          std::string content = std::get<3>(task);

          cv::Point position(col * 200, row * 50 + 20);
          ft2->putText(frame, content, position, fontSize, cv::Scalar(255, 255, 255), -1, cv::LINE_AA, true);
     }
}

// 清除视频帧上的文字
void clearVideoText(cv::Mat &frame, const std::vector<std::tuple<int, int, int>> &clearTasks)
{
     for (const auto &task : clearTasks)
     {
          int row = std::get<0>(task);
          int col = std::get<1>(task);
          int clearType = std::get<2>(task);
          if (clearType == 0)
          {
               cv::Point position(col * 100, row * 30);
               cv::rectangle(frame, cv::Rect(position.x, position.y - 20, 100, 30), cv::Scalar(0, 0, 0), cv::FILLED);
          }
     }
}

// 调用图片字符叠加的解析和处理函数，第一个参数是图片保存的路径，第二个参数是字符覆盖的指令，第三个参数是清理字符的指令
// 处理图片，将字符叠加到图片上
void processImage(const std::string &imagePath, const std::string &overlayInstructions, const std::string &clearInstructions, cv::Ptr<cv::freetype::FreeType2> &ft2)
{
     cv::Mat image = cv::imread(imagePath);
     if (image.empty())
     {
          throw std::runtime_error("无法打开图片文件");
     }

     std::vector<std::tuple<int, int, int, std::string>> overlayTasks;
     std::vector<std::tuple<int, int, int>> clearTasks;
     parseOverlayInstructions(overlayInstructions, overlayTasks);
     parseClearInstructions(clearInstructions, clearTasks);

     overlayImageText(image, overlayTasks, ft2);
     clearImageText(image, clearTasks);

     // 保存处理后的图片
     std::string outputPath = "processed_" + imagePath;
     cv::imwrite(outputPath, image);

     std::cout << "图片处理完成，保存至 " << outputPath << std::endl;
}

// 将读取视频文件，逐帧处理，然后将处理后的帧写回新的视频文件
void processVideo(const std::string &videoPath, const std::string &overlayInstructions, const std::string &clearInstructions, cv::Ptr<cv::freetype::FreeType2> &ft2)
{
     cv::VideoCapture cap(videoPath);
     if (!cap.isOpened())
     {
          throw std::runtime_error("无法打开视频文件");
     }

     int frame_width = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH));
     int frame_height = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT));
     int fps = static_cast<int>(cap.get(cv::CAP_PROP_FPS));

     // 使用合适的编解码器
     cv::VideoWriter writer("processed_video.mp4", cv::VideoWriter::fourcc('m', 'p', '4', 'v'), fps, cv::Size(frame_width, frame_height));

     std::vector<std::tuple<int, int, int, std::string>> overlayTasks;
     std::vector<std::tuple<int, int, int>> clearTasks;
     parseOverlayInstructions(overlayInstructions, overlayTasks);
     parseClearInstructions(clearInstructions, clearTasks);

     cv::Mat frame;
     while (cap.read(frame))
     {
          overlayVideoText(frame, overlayTasks, ft2);
          clearVideoText(frame, clearTasks);
          writer.write(frame);
     }

     cap.release();
     writer.release();
}
