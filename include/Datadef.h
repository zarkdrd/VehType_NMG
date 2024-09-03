/**************************************************
     >Author: zarkdrd
     >Date: 2024-08-16 15:11:03
     >LastEditTime: 2024-08-16 15:11:05
     >LastEditors: zarkdrd
     >Description:
     >FilePath: /VehType_NMG/inc/Datadef.h
**************************************************/
#ifndef _DATADEF_H_
#define _DATADEF_H_

typedef struct
{
    char Plate[16];     // 车牌号码    例如：粤AF3574
    int Color;          // 车牌颜色	  例如：0、1、2
    char FileName[128]; // 抓拍图文件名
    int VehType;        // 识别车型	 1~5（客车） 11~16（货车）  21~26（专项作业车）6：未知车型
} SVehicleInfo;

typedef void (*pAIVTCb)(int nCroNO, SVehicleInfo *stVehInfo);

#endif