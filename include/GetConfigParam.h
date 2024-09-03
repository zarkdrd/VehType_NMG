/**************************************************
     >Author: zarkdrd
     >Date: 2024-08-20 16:22:58
     >LastEditTime: 2024-08-27 14:23:50
     >LastEditors: zarkdrd
     >Description: 
     >FilePath: /VehType_NMG/include/GetConfigParam.h
**************************************************/
/*************************************************************************
    > File Name: DEVICE_INIT.h
    > Author: Routine
    > Description:
    > Created Time: 2023-09-22
 ************************************************************************/
#pragma once
#ifndef _GET_CONFIG_PARAM_
#define _GET_CONFIG_PARAM_

#include <string>

using std::string;

int GetServerPort(void);

int GetUploadMode(void);

int GetHeatSwitch(void);   

string GetSerialPath(void);   

int GetLocalPort();

string GetRemoteIp(void);

int GetRemotePort();

std::string convertIP(unsigned char ip[]);

unsigned int convertPort(unsigned char port[]);
#endif //_GET_CONFIG_PARAM_
