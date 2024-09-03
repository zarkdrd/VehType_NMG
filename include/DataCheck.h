/*************************************************************************
    > File Name: include/DataCheck.h
    > Author: ARTC
    > Descripttion:
    > Created Time: 2023-11-02
 ************************************************************************/

#ifndef _INCLUDE_DATACHECK_H_
#define _INCLUDE_DATACHECK_H_

#ifdef __cplusplus
extern "C"
{
#endif

    unsigned int CRC16(unsigned char *crcdata, unsigned int length);
    unsigned short checkCRC16Value(char *data, int dataLength);

#ifdef __cplusplus
}
#endif

#endif //_INCLUDE_DATACHECK_H_
