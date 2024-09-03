/**************************************************
     >Author: zarkdrd
     >Date: 2024-08-20 10:29:11
     >LastEditTime: 2024-08-20 10:29:12
     >LastEditors: zarkdrd
     >Description: 
     >FilePath: /VehType_NMG/PhotoHander.h
**************************************************/
#pragma once
#ifndef _INCLUDE_PHOTOHANDER_H_
#define _INCLUDE_PHOTOHANDER_H_
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <string>
#include <unistd.h>

std::string Encode(const char *Data, int DataByte);
std::string Decode(const char *Data, int DataByte, int &OutByte);
bool ReadPhotoFile(const std::string &strFileName, std::string &strData);
bool Encoded_WritePhotoFile(const std::string &strFileName, const std::string &strData);
/*解码后的数据保存成图片*/
bool Decoded_WritePhotoFile(const std::string &strFileName, const std::string &strData,int strLen);
// 二值图转换 source destination
void genBinaryImg(void* srcImg,unsigned long srcImgSize,void* destImg,unsigned long* destImgSize);

#endif //_INCLUDE_BASE64TOPHOTO_H_
