/**************************************************
     >Author: zarkdrd
     >Date: 2024-08-20 10:28:54
     >LastEditTime: 2024-08-20 10:28:55
     >LastEditors: zarkdrd
     >Description: 
     >FilePath: /VehType_NMG/PhotoHander.cpp
**************************************************/
#include "PhotoHander.h"
#include "jpeg/jpeglib.h"
#include "string.h"

using namespace std;

std::string Encode(const char *Data, int DataByte)
{
    // 编码表
    const char EncodeTable[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    // 返回值
    string strEncode;
    unsigned char Tmp[4] = {0};
    int LineLength = 0;
    for (int i = 0; i < (int)(DataByte / 3); i++) {
        Tmp[1] = *Data++;
        Tmp[2] = *Data++;
        Tmp[3] = *Data++;
        strEncode += EncodeTable[Tmp[1] >> 2];
        strEncode += EncodeTable[((Tmp[1] << 4) | (Tmp[2] >> 4)) & 0x3F];
        strEncode += EncodeTable[((Tmp[2] << 2) | (Tmp[3] >> 6)) & 0x3F];
        strEncode += EncodeTable[Tmp[3] & 0x3F];
        if (LineLength += 4, LineLength == 76) {
            strEncode += "\r\n";
            LineLength = 0;
        }
    }
    // 对剩余数据进行编码
    int Mod = DataByte % 3;
    if (Mod == 1) {
        Tmp[1] = *Data++;
        strEncode += EncodeTable[(Tmp[1] & 0xFC) >> 2];
        strEncode += EncodeTable[((Tmp[1] & 0x03) << 4)];
        strEncode += "==";
    } else if (Mod == 2) {
        Tmp[1] = *Data++;
        Tmp[2] = *Data++;
        strEncode += EncodeTable[(Tmp[1] & 0xFC) >> 2];
        strEncode += EncodeTable[((Tmp[1] & 0x03) << 4) | ((Tmp[2] & 0xF0) >> 4)];
        strEncode += EncodeTable[((Tmp[2] & 0x0F) << 2)];
        strEncode += "=";
    }

    return strEncode;
}

std::string Decode(const char *Data, int DataByte, int &OutByte)
{
    // 解码表
    const char DecodeTable[] =
        {
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            62, // '+'
            0, 0, 0,
            63,                                     // '/'
            52, 53, 54, 55, 56, 57, 58, 59, 60, 61, // '0'-'9'
            0, 0, 0, 0, 0, 0, 0,
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
            13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, // 'A'-'Z'
            0, 0, 0, 0, 0, 0,
            26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38,
            39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, // 'a'-'z'
        };
    // 返回值
    string strDecode;
    int nValue;
    int i = 0;
    while (i < DataByte) {
        if (*Data != '\r' && *Data != '\n') {
            nValue = DecodeTable[*Data++] << 18;
            nValue += DecodeTable[*Data++] << 12;
            strDecode += (nValue & 0x00FF0000) >> 16;
            OutByte++;
            if (*Data != '=') {
                nValue += DecodeTable[*Data++] << 6;
                strDecode += (nValue & 0x0000FF00) >> 8;
                OutByte++;
                if (*Data != '=') {
                    nValue += DecodeTable[*Data++];
                    strDecode += nValue & 0x000000FF;
                    OutByte++;
                }
            }
            i += 4;
        } else // 回车换行,跳过
        {
            Data++;
            i++;
        }
    }
    return strDecode;
}

bool ReadPhotoFile(const std::string &strFileName, std::string &strData)
{
    int fd = open(strFileName.c_str(), O_RDONLY);
    if (fd == -1) {
        return false;
    }

    off_t fileSize = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);

    char *pBuffer = new char[fileSize];
    if (read(fd, pBuffer, fileSize) != fileSize) {
        delete[] pBuffer;
        close(fd);
        return false;
    }

    strData = Encode(pBuffer, fileSize);

    delete[] pBuffer;
    close(fd);
    return true;
}

/*加密后的数据保存为图片*/
bool Encoded_WritePhotoFile(const std::string &strFileName, const std::string &strData)
{
    int fd = open(strFileName.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (fd == -1) {
        return false;
    }

    int dataLen = 0;
    std::string strdcode = Decode(strData.data(), strData.size(), dataLen);
    if (write(fd, strdcode.data(), dataLen) != dataLen) {
        close(fd);
        return false;
    }
    close(fd);
    return true;
}

/*解码后的数据保存成图片*/
bool Decoded_WritePhotoFile(const std::string &strFileName, const std::string &strData,int strLen)
{
    int fd = open(strFileName.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (fd == -1) {
        return false;
    }

    if (write(fd, strData.data(), strLen) != strLen) {
        close(fd);
        return false;
    }
    close(fd);
    return true;
}

// 二值图转换 source destination
void genBinaryImg(void* srcImg,unsigned long srcImgSize,void* destImg,unsigned long* destImgSize)
{
	//加载图片
	struct jpeg_decompress_struct decinfo;
    struct jpeg_error_mgr jerr;
    decinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&decinfo);
    jpeg_mem_src(&decinfo, (unsigned char *)srcImg,srcImgSize);
    jpeg_read_header(&decinfo, TRUE);
    jpeg_start_decompress(&decinfo);
    int width = decinfo.output_width;
    int height = decinfo.output_height;
    unsigned char *data = (unsigned char *) malloc(decinfo.output_height * decinfo.output_width * decinfo.output_components);
    unsigned char *line_pointer;
    int i = 0;
    while (decinfo.output_scanline < decinfo.image_height) {
        line_pointer = data + i * decinfo.output_width * decinfo.output_components;
        jpeg_read_scanlines(&decinfo, &line_pointer, 1);
        i ++;
    }
    jpeg_finish_decompress(&decinfo);
    jpeg_destroy_decompress(&decinfo);

	//转换二值图
	unsigned long long nGrayTotal=0;
	unsigned long nPixelCount=0;
    unsigned char *pos;
	for(int x=0;x<width;x++)
	{
		for(int y=0;y<height;y++)
		{
			pos = data + (y * width + x) * 3;
			nGrayTotal+=pos[0]+pos[1]+pos[2];
			nPixelCount++;
		}
	}
    // 求全图平均灰度值
	int nGrayscal=nGrayTotal/nPixelCount/3;
	std::cout<<"nGrayscal:"<<nGrayscal<<std::endl;
	if(nGrayscal<20)
		nGrayscal=20;
	else if(nGrayscal>235)
		nGrayscal=235;

	int nGray=0;
	for(int x=0;x<width;x++)
	{
		for(int y=0;y<height;y++)
		{
			pos = data + (y * width + x) * 3;
			nGray=(pos[0]+pos[1]+pos[2])/3;
			if(nGray>nGrayscal)
				nGray=255;
			else
				nGray=0;
			pos[0] = nGray;
    		pos[1] = nGray;
    		pos[2] = nGray;

		}
	}

	//输出图片
	struct jpeg_compress_struct cinfo;
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);

	memset(destImg,0,*destImgSize);
    jpeg_mem_dest(&cinfo, (unsigned char **)&destImg,destImgSize);

    cinfo.image_width = width;
    cinfo.image_height = height;
    cinfo.in_color_space = JCS_RGB;
    cinfo.input_components = 3;
    jpeg_set_defaults(&cinfo);
    jpeg_start_compress(&cinfo, TRUE);
    i = 0;
    while (cinfo.next_scanline < cinfo.image_height) {
        line_pointer = data + i * cinfo.image_width * cinfo.input_components;
        jpeg_write_scanlines(&cinfo, &line_pointer, 1);
        i ++;
    }
    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);
	free(data);
}

// int main()
// {
//     // 图片转base64
//     string path = "boqi.jpg"; // 图片路径
//     string data;                                       // 转换后的字符串
//     ReadPhotoFile(path, data);

//     // base64转图片
//     string strfilename = "123.jpg"; // 转换后的的图片的路径和格式（可改为其他格式）
//     WritePhotoFile(strfilename, data);

//     return 0;
// }