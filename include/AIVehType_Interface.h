/**************************************************
     >Author: zarkdrd
     >Date: 2024-08-16 15:08:36
     >LastEditTime: 2024-08-26 10:28:06
     >LastEditors: zarkdrd
     >Description:
     >FilePath: /VehType_NMG/include/AIVehType_Interface.h
**************************************************/
#pragma once

#if defined(__cplusplus)
#define D_EXTERN_C extern "C"
#else
#define D_EXTERN_C
#endif

#ifdef WIN32 //
#define D_CALLTYPE __stdcall
#ifdef AIVEHTYPEARTC_EXPORTS
#define D_AIVEHTYPEARTC_EXPORTS __declspec(dllexport)
#else
#define D_AIVEHTYPEARTC_EXPORTS __declspec(dllimport)
#endif
#else //
#define D_CALLTYPE
#define D_AIVEHTYPEARTC_EXPORTS __attribute__((visibility("default")))

#endif

#include <pthread.h>
#include <stdbool.h>
#include "VehType.h"

D_EXTERN_C D_AIVEHTYPEARTC_EXPORTS bool D_CALLTYPE AIVT_Init();

D_EXTERN_C D_AIVEHTYPEARTC_EXPORTS void D_CALLTYPE AIVT_Quit();

D_EXTERN_C D_AIVEHTYPEARTC_EXPORTS int D_CALLTYPE AIVT_Connect(/*char *lpszIP*/);

D_EXTERN_C D_AIVEHTYPEARTC_EXPORTS int D_CALLTYPE AIVT_DisConnect(int nCroNO);

D_EXTERN_C D_AIVEHTYPEARTC_EXPORTS int D_CALLTYPE AIVT_GetState(int nCroNO);

// 车型识别(只识别车型)
D_EXTERN_C D_AIVEHTYPEARTC_EXPORTS int D_CALLTYPE Type_Recognize(char *Plate, int Color);

// http服务器
// D_EXTERN_C D_AIVEHTYPEARTC_EXPORTS void *D_CALLTYPE http_server(void *arg);
D_EXTERN_C D_AIVEHTYPEARTC_EXPORTS void D_CALLTYPE http_server(class VehType *myVehType);
