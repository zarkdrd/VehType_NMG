/*************************************************************************
    > File Name: src/DataCheck.cpp
    > Author: ARTC
    > Descripttion:
    > Created Time: 2023-11-02
 ************************************************************************/

#include "DataCheck.h"
#include "Log_Message.h"

// #define CRC_POLYNOM 0x8408
#define CRC_POLYNOM 0x8408
#define CRC_PRESET 0xffff
unsigned int CRC16(unsigned char *crcdata, unsigned int length)
{
    unsigned int crc = CRC_PRESET;
    unsigned int i, j;

    for (i = 0; i < length; i++)
    {
        crc ^= *crcdata;
        for (j = 0; j < 8; j++)
        {
            if (crc & 0x0001)
            {
                crc = (crc >> 1) ^ CRC_POLYNOM;
            }
            else
            {
                crc = (crc >> 1);
            }
        }
        crcdata++;
    }

    crc = (~crc) & CRC_PRESET;

    return (crc);
}

unsigned short checkCRC16Value(char *data, int dataLength)
{
    unsigned short crc16 = 0;

    int polynomial = 0x1021;
    // 此处index从0开始
    for (int index = 0; index < dataLength; index++)
    {
        unsigned char b = data[index];
        for (int i = 0; i < 8; i++)
        {
            bool bit = ((b >> (7 - i) & 1) == 1);
            bool c15 = ((crc16 >> 15 & 1) == 1);

            crc16 <<= 1;
            if (c15 ^ bit)
            {
                crc16 ^= polynomial;
            }
        }
    }
    return crc16;
}

unsigned char chr2hex(unsigned char chr)
{
    if (chr >= '0' && chr <= '9')
    {
        return chr - '0';
    }
    else if (chr >= 'A' && chr <= 'F')
    {
        return (chr - 'A' + 10);
    }
    else if (chr >= 'a' && chr <= 'f')
    {
        return (chr - 'a' + 10);
    }
    return 0;
}

unsigned char hex2chr(unsigned char hex)
{
    if (hex <= 9)
    {
        return hex + '0';
    }
    else if (hex >= 10 && hex <= 15)
    {
        return (hex - 10 + 'A');
    }
    return '0';
}

void HexToStr(const unsigned char *from, unsigned int fromSize, char *to, unsigned int *toSize)
{
    int size = 0;
    while (fromSize != 0)
    {
        *to++ = hex2chr((*from >> 4) & 0X0F);
        *to++ = hex2chr((*from) & 0X0F);
        size += 2;
        from++;
        fromSize--;
    }
    *to = 0; // 添加结束符
    *toSize = size;
}

void StrToHex(const char *from, unsigned int fromSize, unsigned char *to, unsigned int *toSize)
{
    unsigned int size = 0;
    while (fromSize > 1)
    {
        *to++ = (chr2hex(from[0]) << 4) | chr2hex(from[1]);
        size++;
        from += 2;
        fromSize -= 2;
    }
    *toSize = size;
}
