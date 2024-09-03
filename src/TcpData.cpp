/**************************************************
     >Author: zarkdrd
     >Date: 2024-08-14 11:37:29
     >LastEditTime: 2024-08-29 11:14:48
     >LastEditors: zarkdrd
     >Description:
     >FilePath: /VehType_NMG/src/TcpData.cpp
**************************************************/
#include "VehType.h"
// 客户端解析数据头与crc校验
bool VehType::ClientReceiveCmd()
{
    int i;
    int ret, Index1 = -1, Index2 = 0, pos = 0;
    int Command_Len, Frame_Data_Len;
    unsigned char Recv_Data_Min[2048];
    unsigned char Recv_Data[2048];
    unsigned char Recv_Data_bak[2048];

    while (1)
    {
        ret = myTcpClient->Read(Recv_Data_Min, sizeof(Recv_Data_Min));
        if (ret <= 0)
        {
            // log_message(ERROR, "recv 返回值 [%d] 错误描述 [%s]", ret, strerror(errno));
            // log_message(ERROR, "客户端断开连接，等待重连...");
            myTcpClient->Close();
            return false;
        }

        /**将获取到的数据进行保存**/
        memcpy(Recv_Data + pos, Recv_Data_Min, ret);
        pos += ret;
        if (pos < 2)
        {
            continue;
        }
    Find_Frame:
        for (i = Index2; i < pos - 1; i++)
        {
            /**找到数据头**/
            if ((Recv_Data[i] == 0xFF) && (Recv_Data[i + 1] == 0xFF) && (Recv_Data[i + 2] == 0x00))
            {
                Index1 = i;
                break;
            }
        }
        if (Index1 < 0)
        {
            /**未找到数据头，清空数据**/
            Index2 = 0;
            pos = 0;
            continue;
        }

        if (pos - Index1 < 8)
        {
            continue;
        }
        else
        {
            /**获取指令长度**/
            Command_Len = Recv_Data[Index1 + 6] * 256 + Recv_Data[Index1 + 7];
        }

        if ((Command_Len + 10) > (pos - Index1))
        {
            log_message(INFO, "数据帧不完整");
            /**数据帧不完整**/
            continue;
        }
        else if ((Command_Len + 10) < (pos - Index1))
        {
            log_message(INFO, "数据帧大于一帧");
            /**大于一帧数据**/
            Frame_Data_Len = Command_Len + 10;
            Index2 = Index1 + Frame_Data_Len;
            printf("framelen:%d\n", Frame_Data_Len);
            printf("command:%d\n", Command_Len);
            printf("pos - Index1:%d\n", pos - Index1);
        }
        else
        {
            log_message(INFO, "数据帧刚好一帧");
            /**刚好一帧数据**/
            Index2 = 0;
            Frame_Data_Len = Command_Len + 10;
            printf("framelen:%d\n", Frame_Data_Len);
            printf("command:%d\n", Command_Len);
        }
        unsigned int crc = CRC16(&Recv_Data[Index1 + 2], Frame_Data_Len - 4);
        char buf[2] = {0};
        buf[0] = (crc >> 8) & 0xFF;
        buf[1] = crc & 0xFF;
        printf("buf0:%2X\n", buf[0]);
        printf("buf1:%2X\n", buf[1]);
        if ((((crc >> 8) & 0xFF) == Recv_Data[Index1 + Frame_Data_Len - 2]) || ((crc & 0xFF) == Recv_Data[Index1 + Frame_Data_Len - 1]))
        {
            CommunicationAnalysis(&Recv_Data[Index1], Frame_Data_Len); // 传入数据帧与数据帧长度
        }
        else
        {
            /**说明这个数据帧是错的，找第二个头**/
            hex_message(ERROR, "Recv Tcp data err:", &Recv_Data[Index1], Frame_Data_Len);
            for (i = Index1 + 1; i < pos - 1; i++)
            {
                /**找到数据头**/
                if (Recv_Data[i] == 0xFF && Recv_Data[i + 1] == 0xFF)
                {
                    Index1 = -1;
                    Index2 = i;
                    goto Find_Frame;
                }
            }
            Index2 = 0;
            pos = 0;
            continue;
        }

        if (Index2 == 0)
        {
            /**无拼帧情况**/
            pos = 0;
            Index1 = -1;
            Index2 = 0;
            memset(Recv_Data, 0, sizeof(Recv_Data));
            continue;
        }

        if (Index2 > 0)
        {
            /**有拼帧情况**/
            pos = pos - Index2;
            memcpy(Recv_Data_bak, Recv_Data + Index2, pos);
            memset(Recv_Data, 0, sizeof(Recv_Data));
            memcpy(Recv_Data, Recv_Data_bak, pos);
        }

        if (pos < 4)
        {
            /**不够完整的帧**/
            Index1 = -1;
            Index2 = 0;
            continue;
        }
        if ((Recv_Data[2] + 4) > pos)
        {
            /**帧长度不够**/
            Index1 = -1;
            Index2 = 0;
            continue;
        }
        /**帧长度充足，再次解析帧**/
        Index1 = -1;
        Index2 = 0;
        goto Find_Frame;
    }
}

// 解析数据头与crc校验
bool VehType::ServerReceiveCmd()
{
    int i;
    int ret, Index1 = -1, Index2 = 0, pos = 0;
    int Command_Len, Frame_Data_Len;
    unsigned char Recv_Data_Min[2048];
    unsigned char Recv_Data[2048];
    unsigned char Recv_Data_bak[2048];

    while (1)
    {
        ret = myTcpServer->RecvByte(Recv_Data_Min, sizeof(Recv_Data_Min));
        if (ret <= 0)
        {
            // log_message(ERROR, "recv 返回值 [%d] 错误描述 [%s]", ret, strerror(errno));
            // log_message(ERROR, "客户端断开连接，等待重连...");
            myTcpServer->Close();
            return false;
        }

        /**将获取到的数据进行保存**/
        memcpy(Recv_Data + pos, Recv_Data_Min, ret);
        pos += ret;
        if (pos < 2)
        {
            continue;
        }
    Find_Frame:
        for (i = Index2; i < pos - 1; i++)
        {
            /**找到数据头**/
            if ((Recv_Data[i] == 0xFF) && (Recv_Data[i + 1] == 0xFF) && (Recv_Data[i + 2] == 0x00))
            {
                Index1 = i;
                break;
            }
        }
        if (Index1 < 0)
        {
            /**未找到数据头，清空数据**/
            Index2 = 0;
            pos = 0;
            continue;
        }

        if (pos - Index1 < 8)
        {
            continue;
        }
        else
        {
            /**获取指令长度**/
            Command_Len = Recv_Data[Index1 + 6] * 256 + Recv_Data[Index1 + 7];
        }

        if ((Command_Len + 10) > (pos - Index1))
        {
            log_message(INFO, "数据帧不完整");
            /**数据帧不完整**/
            continue;
        }
        else if ((Command_Len + 10) < (pos - Index1))
        {
            log_message(INFO, "数据帧大于一帧");
            /**大于一帧数据**/
            Frame_Data_Len = Command_Len + 10;
            Index2 = Index1 + Frame_Data_Len;
            printf("framelen:%d\n", Frame_Data_Len);
            printf("command:%d\n", Command_Len);
            printf("pos - Index1:%d\n", pos - Index1);
        }
        else
        {
            log_message(INFO, "数据帧刚好一帧");
            /**刚好一帧数据**/
            Index2 = 0;
            Frame_Data_Len = Command_Len + 10;
            printf("framelen:%d\n", Frame_Data_Len);
            printf("command:%d\n", Command_Len);
        }
        unsigned int crc = CRC16(&Recv_Data[Index1 + 2], Frame_Data_Len - 4);
        char buf[2] = {0};
        buf[0] = (crc >> 8) & 0xFF;
        buf[1] = crc & 0xFF;
        printf("buf0:%2X\n", buf[0]);
        printf("buf1:%2X\n", buf[1]);
        if ((((crc >> 8) & 0xFF) == Recv_Data[Index1 + Frame_Data_Len - 2]) || ((crc & 0xFF) == Recv_Data[Index1 + Frame_Data_Len - 1]))
        {
            CommunicationAnalysis(&Recv_Data[Index1], Frame_Data_Len); // 传入数据帧与数据帧长度
        }
        else
        {
            /**说明这个数据帧是错的，找第二个头**/
            hex_message(ERROR, "Recv Tcp data err:", &Recv_Data[Index1], Frame_Data_Len);
            for (i = Index1 + 1; i < pos - 1; i++)
            {
                /**找到数据头**/
                if (Recv_Data[i] == 0xFF && Recv_Data[i + 1] == 0xFF)
                {
                    Index1 = -1;
                    Index2 = i;
                    goto Find_Frame;
                }
            }
            Index2 = 0;
            pos = 0;
            continue;
        }

        if (Index2 == 0)
        {
            /**无拼帧情况**/
            pos = 0;
            Index1 = -1;
            Index2 = 0;
            memset(Recv_Data, 0, sizeof(Recv_Data));
            continue;
        }

        if (Index2 > 0)
        {
            /**有拼帧情况**/
            pos = pos - Index2;
            memcpy(Recv_Data_bak, Recv_Data + Index2, pos);
            memset(Recv_Data, 0, sizeof(Recv_Data));
            memcpy(Recv_Data, Recv_Data_bak, pos);
        }

        if (pos < 4)
        {
            /**不够完整的帧**/
            Index1 = -1;
            Index2 = 0;
            continue;
        }
        if ((Recv_Data[2] + 4) > pos)
        {
            /**帧长度不够**/
            Index1 = -1;
            Index2 = 0;
            continue;
        }
        /**帧长度充足，再次解析帧**/
        Index1 = -1;
        Index2 = 0;
        goto Find_Frame;
    }
}
