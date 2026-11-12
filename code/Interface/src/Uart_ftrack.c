#include "headfile.h"
#include "UartFunction.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#define GNSS_PROTOCOL_HEAD_COMMON	('$')
static u8 gnss_gga_protlHead[GNSS_PROTOCOL_HEAD_LEN] = {GNSS_PROTOCOL_HEAD_COMMON, 'x', 'x', 'G', 'G', 'A'};
static u8 gnss_gsa_protlHead[GNSS_PROTOCOL_HEAD_LEN] = {GNSS_PROTOCOL_HEAD_COMMON, 'x', 'x', 'G', 'S', 'A'};
static u8 gnss_rmc_protlHead[GNSS_PROTOCOL_HEAD_LEN] = {GNSS_PROTOCOL_HEAD_COMMON, 'x', 'x', 'R', 'M', 'C'};
static u8 gnss_gsv_protlHead[GNSS_PROTOCOL_HEAD_LEN] = {GNSS_PROTOCOL_HEAD_COMMON, 'x', 'x', 'G', 'S', 'V'};
static u8 gnss_gll_protlHead[GNSS_PROTOCOL_HEAD_LEN] = {GNSS_PROTOCOL_HEAD_COMMON, 'x', 'x', 'G', 'L', 'L'};
static u8 gnss_vtg_protlHead[GNSS_PROTOCOL_HEAD_LEN] = {GNSS_PROTOCOL_HEAD_COMMON, 'x', 'x', 'V', 'T', 'G'};
static u8 gnss_protlEnd[GNSS_PROTOCOL_END_LEN] = {'*', 'x', 'x', 0x0D, 0x0A};

//GNSS Ð­Òé
static tGnssProtocol inst_Gnss_GGA_Protocol;
static tGnssProtocol inst_Gnss_GSA_Protocol;
static tGnssProtocol inst_Gnss_RMC_Protocol;
static tGnssProtocol inst_Gnss_GSV_Protocol;
static tGnssProtocol inst_Gnss_GLL_Protocol;
static tGnssProtocol inst_Gnss_VTG_Protocol;

static u8 Comma_id_sep[32]; //
static void init(void);
static void SendMessage(Support_ProtocolList proto, u8 addr, u16 u16msg, u8 *buf, u32 buflen);
static void ProtocolProcess(void);

static void RecvCmdProcess(void);
static void SendCmdProcess(void);

static void GnssProtoInit(void);
static u8 Check_GPS_data(const u8* Buffer, u16 len);
static void Creat_Comma_Index(u8 *buffer, u16 len);
static char *Real_Process_Comma(char *buffer, u8 num);

static inline void ProcessCmd_private(u8 *buf, u32 len);
static inline void ProcessCmd_Pelco_D_P(Support_ProtocolList proto, u8 *buf, u32 len);
static inline void processRecvCmd(u8 *buf, u16 len);

static tProtocol inst_PrivateProtocol =
{
    .u8FrameStart[0] = 0x55,
    .u8FrameStart[1] = 0xAA,
    .u8Address       = 0x01,
    .u8Reserve       = 0x00,
    .u8FrameEnd      = 0xF0
};

static tProtocol inst_PelcoD_Protocol =
{
    .u8FrameStart[0]        = 0xFF,
    .u8Address              = 0x01,
    .u8Reserve              = 0x00,
    .u8FrameEnd             = 0x00
};

static tProtocol inst_PelcoP_Protocol =
{
    .u8FrameStart[0]        = 0xA0,
    .u8Address              = 0x01,
    .u8Reserve              = 0xAF,
    .u8FrameEnd             = 0x00
};
/************************************************************************/

static _tUartFunc inst_UartFunc =
{
    .init                               =               init,
    .SendMessage                        =               SendMessage,
    .ProtocolProcess               =               ProtocolProcess
};

_tUartFunc *ptr_ftrackFunc = &inst_UartFunc;


static void GnssProtoInit(void)
{
    memcpy(inst_Gnss_GGA_Protocol.u8FrameStart, gnss_gga_protlHead, 6);
    memcpy(inst_Gnss_GSA_Protocol.u8FrameStart, gnss_gsa_protlHead, 6);
    memcpy(inst_Gnss_RMC_Protocol.u8FrameStart, gnss_rmc_protlHead, 6);
    memcpy(inst_Gnss_GSV_Protocol.u8FrameStart, gnss_gsv_protlHead, 6);
    memcpy(inst_Gnss_GLL_Protocol.u8FrameStart, gnss_gll_protlHead, 6);
    memcpy(inst_Gnss_VTG_Protocol.u8FrameStart, gnss_vtg_protlHead, 6);

    memcpy(inst_Gnss_GGA_Protocol.u8FrameEnd, gnss_protlEnd, 5);
    memcpy(inst_Gnss_GSA_Protocol.u8FrameEnd, gnss_protlEnd, 5);
    memcpy(inst_Gnss_RMC_Protocol.u8FrameEnd, gnss_protlEnd, 5);
    memcpy(inst_Gnss_GSV_Protocol.u8FrameEnd, gnss_protlEnd, 5);
    memcpy(inst_Gnss_GLL_Protocol.u8FrameEnd, gnss_protlEnd, 5);
    memcpy(inst_Gnss_VTG_Protocol.u8FrameEnd, gnss_protlEnd, 5);
}
static void init(void)
{
    GnssProtoInit();
    ptr_FtrackQueue->InitCmdQueue(ptr_FtrackQueue);
    inst_UartFunc.u8pBuf = (u8 *)malloc(sizeof(u8) * RCV_QUEUE_SIZE);
}


static void processSendInternalCmd(u8 *buf, u32 buflen)
{
    u32 i = 0;

    for (i = 0; i < buflen; i++)
    {
        if (NULL != ptr_FtrackQueue->pCqSendTop)
        {
            *(ptr_FtrackQueue->pCqSendTop) =  *(u8 *)(buf + i);
            ptr_FtrackQueue->pCqSendTop         = ptr_FtrackQueue->NextCell(ptr_FtrackQueue, SND_QUEEN, QUEEN_TOP);
        }
    }
}

static void processSendPelcoDPCmd(Support_ProtocolList proto, u8 address, u8 *buf, u32 buflen)
{
    u8 u8DataCheck = 0x0;
    u8 i = 0;
    u8 j = 0;

    u8 frame_headbuf[2] = {0x0};

    switch (proto)
    {
        case protoPELCO_D:
        {
            u8DataCheck = 0x0;
            u8DataCheck += address;

            frame_headbuf[0] = inst_PelcoD_Protocol.u8FrameStart[0];
            frame_headbuf[1] = address;

            for (i = 0; i < buflen; i++)
            {
                u8DataCheck += *(u8 *)(buf + i);
            }

            for (j = 0; j < buflen + 3; j++)
            {
                if (NULL != ptr_FtrackQueue->pCqSendTop)
                {
                    if (2 > j)
                    {
                        *(ptr_FtrackQueue->pCqSendTop) = frame_headbuf[j];
                    }
                    else if (6 > j)
                    {
                        *(ptr_FtrackQueue->pCqSendTop) =  *(u8 *)(buf + j - 2);
                    }
                    else
                    {
                        *(ptr_FtrackQueue->pCqSendTop) =  u8DataCheck;
                    }

                    ptr_FtrackQueue->pCqSendTop         = ptr_FtrackQueue->NextCell(ptr_FtrackQueue, SND_QUEEN, QUEEN_TOP);
                }
            }

            break;
        }

        case protoPELCO_P:
        {
            u8DataCheck = 0x0;
            u8DataCheck ^= address;

            frame_headbuf[0] = inst_PelcoP_Protocol.u8FrameStart[0];
            frame_headbuf[1] = address;

            for (i = 0; i < buflen; i++)
            {
                u8DataCheck ^= *(u8 *)(buf + i);
            }

            for (j = 0; j < buflen + 3; j++)
            {
                if (NULL != ptr_FtrackQueue->pCqSendTop)
                {
                    if (2 > j)
                    {
                        *(ptr_FtrackQueue->pCqSendTop) =  frame_headbuf[j];
                    }
                    else if (6 > j)
                    {
                        *(ptr_FtrackQueue->pCqSendTop) =  *(u8 *)(buf + j - 2);
                    }
                    else if (6 == j)
                    {
                        *(ptr_FtrackQueue->pCqSendTop) =  0xAF;
                    }
                    else if (7 == j)
                    {
                        *(ptr_FtrackQueue->pCqSendTop) =  u8DataCheck;
                    }

                    ptr_FtrackQueue->pCqSendTop         = ptr_FtrackQueue->NextCell(ptr_FtrackQueue, SND_QUEEN, QUEEN_TOP);
                }
            }

            break;
        }

        default:
            break;
    }
}


static void processSendPrivateCmd(u16 u16msg, u8 *buf, u32 buflen)
{
    u16 u16cmdMsg = msgUpdateNull;
    u32 j = 0;

    u8 frame_headbuf[10] = {0x00};

    u16cmdMsg = u16msg;

    frame_headbuf[0] = inst_PrivateProtocol.u8FrameStart[0];
    frame_headbuf[1] = inst_PrivateProtocol.u8FrameStart[1];
    frame_headbuf[2] = ptr_3588Func->u8Address - 4;
    frame_headbuf[3] = 0;
    frame_headbuf[4] = u16cmdMsg & 0xFF;
    frame_headbuf[5] = (buflen >> 24) & 0xFF;
    frame_headbuf[6] = (buflen >> 16) & 0xFF;
    frame_headbuf[7] = (buflen >> 8) & 0xFF;
    frame_headbuf[8] = (buflen >> 0) & 0xFF;
    frame_headbuf[9] = (frame_headbuf[3] + frame_headbuf[4] + frame_headbuf[5] + frame_headbuf[6] + frame_headbuf[7] + frame_headbuf[8]) & 0xFF;

    for (j = 0; j < buflen + 12; j++)
    {
        if (NULL != ptr_FtrackQueue->pCqSendTop)
        {
            if (10 > j)
            {
                *(ptr_FtrackQueue->pCqSendTop) = frame_headbuf[j];
            }
            else if ((10 + buflen) > j)
            {
                *(ptr_FtrackQueue->pCqSendTop) = *(u8 *)(buf + j - 10);
            }
            else if ((buflen + 12 - 2) == j)
            {
                *(ptr_FtrackQueue->pCqSendTop) = inst_PrivateProtocol.u8Reserve;
            }
            else if ((buflen + 12 - 1) == j)
            {
                *(ptr_FtrackQueue->pCqSendTop) = inst_PrivateProtocol.u8FrameEnd;
            }

            ptr_FtrackQueue->pCqSendTop         = ptr_FtrackQueue->NextCell(ptr_FtrackQueue, SND_QUEEN, QUEEN_TOP);
        }
    }
}

static void SendMessage(Support_ProtocolList proto, u8 addr, u16 u16msg, u8 *buf, u32 buflen)
{
    switch (proto)
    {
        case proto_internal:
            processSendInternalCmd(buf, buflen);
            break;

        case protoPELCO_D:
        case protoPELCO_P:
            processSendPelcoDPCmd(proto, inst_PelcoD_Protocol.u8Address, buf, buflen);
            break;

        case proto_private:
            processSendPrivateCmd(u16msg, buf, buflen);
            break;

        default:
            break;
    }
}


static void ProtocolProcess(void)
{
    RecvCmdProcess();
    SendCmdProcess();
}

#define SEARCHFRAMEHEAD_LEN 20
static inline u8 SearchFrameHead(void)
{
    Cmd_T *cqBtm;
    u16 i = 0;
    u32 u32Top = 0;
    u32 u32Btm = 0;

    static u8 u8frameheadbuf[SEARCHFRAMEHEAD_LEN];
    static u8 u8frameheadbuf_cnt = 0;

    u32Top = (u32)ptr_FtrackQueue->pCqRecvTop;
    u32Btm = (u32)ptr_FtrackQueue->pCqRecvBtm;

    if (u32Top != u32Btm)
    {
        cqBtm = ptr_FtrackQueue->pCqRecvBtm;
        u8frameheadbuf[u8frameheadbuf_cnt] = *(cqBtm);
        u8frameheadbuf_cnt ++;
        cqBtm = ptr_FtrackQueue->NextCell(ptr_FtrackQueue, RCV_QUEEN, QUEEN_BTM);
        ptr_FtrackQueue->pCqRecvBtm = cqBtm;

        if (u8frameheadbuf_cnt == GNSS_PROTOCOL_HEAD_LEN)
        {
            if (u8frameheadbuf[0] == GNSS_PROTOCOL_HEAD_COMMON)
            {
                inst_UartFunc.protoType = proto_GNSS;

                //GGA
                if((u8frameheadbuf[3]  == inst_Gnss_GGA_Protocol.u8FrameStart[3]) &&
                        (u8frameheadbuf[4]  == inst_Gnss_GGA_Protocol.u8FrameStart[4]) &&
                        (u8frameheadbuf[5]  == inst_Gnss_GGA_Protocol.u8FrameStart[5]))
                {
                    for(i = 0; i < u8frameheadbuf_cnt; i++)
                    {
                        *(u8 *)(inst_UartFunc.u8pBuf + i) = u8frameheadbuf[i];
                        u8frameheadbuf[i] = 0x0;
                    }

                    u8frameheadbuf_cnt = 0;

                    inst_UartFunc.u16cmdType = msgGGA;
                    return 1;
                }
                //GSA
                else if((u8frameheadbuf[3]  == inst_Gnss_GSA_Protocol.u8FrameStart[3]) &&
                        (u8frameheadbuf[4]  == inst_Gnss_GSA_Protocol.u8FrameStart[4]) &&
                        (u8frameheadbuf[5]  == inst_Gnss_GSA_Protocol.u8FrameStart[5]))
                {
                    for(i = 0; i < u8frameheadbuf_cnt; i++)
                    {
                        *(u8 *)(inst_UartFunc.u8pBuf + i) = u8frameheadbuf[i];
                        u8frameheadbuf[i] = 0x0;
                    }

                    u8frameheadbuf_cnt = 0;

                    inst_UartFunc.u16cmdType = msgGSA;
                    return 1;
                }
                //RMC
                else if((u8frameheadbuf[3]  == inst_Gnss_RMC_Protocol.u8FrameStart[3]) &&
                        (u8frameheadbuf[4]  == inst_Gnss_RMC_Protocol.u8FrameStart[4]) &&
                        (u8frameheadbuf[5]  == inst_Gnss_RMC_Protocol.u8FrameStart[5]))
                {
                    for(i = 0; i < u8frameheadbuf_cnt; i++)
                    {
                        *(u8 *)(inst_UartFunc.u8pBuf + i) = u8frameheadbuf[i];
                        u8frameheadbuf[i] = 0x0;
                    }

                    u8frameheadbuf_cnt = 0;

                    inst_UartFunc.u16cmdType = msgRMC;
                    return 1;
                }
                //GSV
                else if((u8frameheadbuf[3]  == inst_Gnss_GSV_Protocol.u8FrameStart[3]) &&
                        (u8frameheadbuf[4]  == inst_Gnss_GSV_Protocol.u8FrameStart[4]) &&
                        (u8frameheadbuf[5]  == inst_Gnss_GSV_Protocol.u8FrameStart[5]))
                {
                    for(i = 0; i < u8frameheadbuf_cnt; i++)
                    {
                        *(u8 *)(inst_UartFunc.u8pBuf + i) = u8frameheadbuf[i];
                        u8frameheadbuf[i] = 0x0;
                    }

                    u8frameheadbuf_cnt = 0;

                    inst_UartFunc.u16cmdType = msgGSV;
                    return 1;
                }
                //GLL
                else if((u8frameheadbuf[3]  == inst_Gnss_GLL_Protocol.u8FrameStart[3]) &&
                        (u8frameheadbuf[4]  == inst_Gnss_GLL_Protocol.u8FrameStart[4]) &&
                        (u8frameheadbuf[5]  == inst_Gnss_GLL_Protocol.u8FrameStart[5]))
                {
                    for(i = 0; i < u8frameheadbuf_cnt; i++)
                    {
                        *(u8 *)(inst_UartFunc.u8pBuf + i) = u8frameheadbuf[i];
                        u8frameheadbuf[i] = 0x0;
                    }

                    u8frameheadbuf_cnt = 0;

                    inst_UartFunc.u16cmdType = msgGLL;
                    return 1;
                }
                //VTG
                else if((u8frameheadbuf[3]  == inst_Gnss_VTG_Protocol.u8FrameStart[3]) &&
                        (u8frameheadbuf[4]  == inst_Gnss_VTG_Protocol.u8FrameStart[4]) &&
                        (u8frameheadbuf[5]  == inst_Gnss_VTG_Protocol.u8FrameStart[5]))
                {
                    for(i = 0; i < u8frameheadbuf_cnt; i++)
                    {
                        *(u8 *)(inst_UartFunc.u8pBuf + i) = u8frameheadbuf[i];
                        u8frameheadbuf[i] = 0x0;
                    }

                    u8frameheadbuf_cnt = 0;

                    inst_UartFunc.u16cmdType = msgVTG;
                    return 1;
                }
                else
                {
                    u8frameheadbuf[0] 	= u8frameheadbuf[1];
                    u8frameheadbuf[1] 	= u8frameheadbuf[2];
                    u8frameheadbuf[2] 	= u8frameheadbuf[3];
                    u8frameheadbuf[3]	  = u8frameheadbuf[4];
                    u8frameheadbuf[4] 	= u8frameheadbuf[5];
                    u8frameheadbuf_cnt -= 1;
                }
            }
            else
            {
                u8frameheadbuf[0] 	= u8frameheadbuf[1];
                u8frameheadbuf[1] 	= u8frameheadbuf[2];
                u8frameheadbuf[2] 	= u8frameheadbuf[3];
                u8frameheadbuf[3]	= u8frameheadbuf[4];
                u8frameheadbuf[4] 	= u8frameheadbuf[5];
                u8frameheadbuf_cnt -= 1;
            }
        }
    }

    return 0;
}

static inline u8 SearchFrameEnd(void)
{
    static u16 u16BufLen = 0;
    u8 *u8pDataBuf = (u8 *)inst_UartFunc.u8pBuf + GNSS_PROTOCOL_HEAD_LEN;

    if (ptr_FtrackQueue->pCqRecvBtm != ptr_FtrackQueue->pCqRecvTop)
    {
        *(u8 *)(u8pDataBuf + u16BufLen) = *(ptr_FtrackQueue->pCqRecvBtm);
        ptr_FtrackQueue->pCqRecvBtm = ptr_FtrackQueue->NextCell(ptr_FtrackQueue, RCV_QUEEN, QUEEN_BTM);
        u16BufLen++;
    }

    if (*(u8pDataBuf + u16BufLen - 1) == gnss_protlEnd[4])
    {

        if ((*(u8 *)(u8pDataBuf + u16BufLen - 2) == gnss_protlEnd[3]) && (*(u8 *)(u8pDataBuf + u16BufLen - 5) == gnss_protlEnd[0]))
        {
            inst_UartFunc.u16BufLen = u16BufLen + GNSS_PROTOCOL_HEAD_LEN;
            u16BufLen = 0;

            if (Check_GPS_data(inst_UartFunc.u8pBuf, inst_UartFunc.u16BufLen))
            {
                Creat_Comma_Index(inst_UartFunc.u8pBuf, inst_UartFunc.u16BufLen);
                return 1;
            }
            else
            {
                return 2;
            }
        }
    }

    return 0;
}

static void RecvCmdProcess(void)
{
    static u8 u8Status = 0;
    u8 u8Rtn    = 0;

    if (0 == u8Status)
    {
        u8Rtn = SearchFrameHead();

        if (1 == u8Rtn)
        {
            u8Status = 1;
        }
        else if (0 == u8Rtn)
        {
            u8Status = 0;
        }
    }

    if (1 == u8Status)
    {
        u8Rtn = SearchFrameEnd();

        if (1 == u8Rtn)
        {
            u8Status = 2;
        }
        else if (2 == u8Rtn)
        {
            u8Status = 0;
        }
    }

    if (2 == u8Status)
    {
        processRecvCmd(inst_UartFunc.u8pBuf, inst_UartFunc.u16BufLen);
        u8Status = 3;
    }
    else if (3 == u8Status)
    {
        inst_UartFunc.protoType = protoNull;
        u8Status = 0;
    }
}

static void SendCmdProcess(void)
{
//    if (ptr_FtrackQueue->pCqSendBtm != ptr_FtrackQueue->pCqSendTop)
//    {
//        //inst_UartFunc.uart->ChangeDir(UART_DIR_SEND);

//        while (ptr_FtrackQueue->pCqSendBtm != ptr_FtrackQueue->pCqSendTop)
//        {
//            //ps_Laser->UartSendData(ptr_FtrackQueue->pCqSendBtm,1);
//            ptr_FtrackQueue->pCqSendBtm = ptr_FtrackQueue->NextCell(ptr_FtrackQueue, SND_QUEEN, QUEEN_BTM);
//        }

//        //inst_UartFunc.uart->ChangeDir(UART_DIR_RECIVE);
//    }
}

static inline void processRecvCmd(u8 *buf, u16 len)
{
    char* temp;
    char index = 0;
    char dotFind = 0;
    unsigned char i, j, frame_num;

    switch(inst_UartFunc.u16cmdType)
    {
        case msgGGA:  //$xxGGA,161229.487,3723.24751,N,12158.34160,W,1,07,1.0,9.0,M.0000*18
        {
            ptr_Gnss->info.Use_EPH_Sum  = atoi(Real_Process_Comma((char *)buf, 7));
            ptr_Gnss->info.MSL_Altitude = atof(Real_Process_Comma((char *)buf, 9));
            break;
        }

        case msgGSA:  //$xxGSA,A,3,07,02,26,27,09,04,15, , , , , , 1.8,1.0,1.5*33
        {
            temp = Real_Process_Comma((char *)buf, 2);

            if ((*temp == '2') || (*temp == '3'))
            {
                ptr_Gnss->info.Locate_Mode = *temp;
            }
            else
            {
                ptr_Gnss->info.Locate_Mode = 0xFF;
            }

            for ( i = 0; i < 12; i++ )
            {
                ptr_Gnss->info.User_EPH[i] = atoi(Real_Process_Comma((char *)buf, i + 3));
            }

            ptr_Gnss->info.PDOP = atof(Real_Process_Comma((char *)buf, 15));
            ptr_Gnss->info.HDOP = atof(Real_Process_Comma((char *)buf, 16));
            ptr_Gnss->info.VDOP = atof(Real_Process_Comma((char *)buf, 17));

            break;
        }

        case msgRMC:  //$xxRMC,161229.487,A,3723.24751,N,12158.34161,W,0.13,309.62,120598,,*10
        {
            temp = Real_Process_Comma((char *)buf, 1);

            if (*temp != 0)
            {
                memcpy(ptr_Gnss->info.UTC_Time, temp, 6);
            }

            if (*(Real_Process_Comma((char *)buf, 2)) == 'A')
            {
                ptr_Gnss->info.Real_Locate = 1;
                ptr_Gnss->info.Located = 1;
            }
            else
            {
                ptr_Gnss->info.Real_Locate = 0;
            }

            temp = Real_Process_Comma((char *)buf, 3);

            if ((*temp >= 0x31) && (*temp <= 0x39))
            {
                for (index = 0; index < 9; index++)
                {
                    if (temp[index] == '.')
                    {
                        ptr_Gnss->info.Latitude[index - 2] = '`';
                        ptr_Gnss->info.Latitude[index - 1] = temp[index - 2];
                        ptr_Gnss->info.Latitude[index] = temp[index - 1];
                        ptr_Gnss->info.Latitude[index + 1] = temp[index];
                        dotFind = 1;
                    }
                    else
                    {
                        if (dotFind == 0)
                        {
                            ptr_Gnss->info.Latitude[index] = temp[index];
                        }
                        else
                        {
                            ptr_Gnss->info.Latitude[index + 1] = temp[index];
                        }
                    }
                }

                ptr_Gnss->info.Latitude[10] = '\'';
                ptr_Gnss->info.Latitude[11] = ' ';

                //memcpy(ptr_Gnss->info.Latitude, temp, 9);
                //ptr_Gnss->info.Latitude[9] = 0;
            }
            else
            {
                ptr_Gnss->info.Latitude[0] = 0;
                ptr_Gnss->info.Latitude[1] = 0;
            }

            dotFind = 0;
            ptr_Gnss->info.NS_Indicator = *(Real_Process_Comma((char *)buf, 4));

            temp = Real_Process_Comma((char *)buf, 5 );

            if ((*temp >= 0x31) && (*temp <= 0x39))
            {
                for (index = 0; index < 10; index++)
                {
                    if (temp[index] == '.')
                    {
                        ptr_Gnss->info.Longitude[index - 2] = '`';
                        ptr_Gnss->info.Longitude[index - 1] = temp[index - 2];
                        ptr_Gnss->info.Longitude[index] = temp[index - 1];
                        ptr_Gnss->info.Longitude[index + 1] = temp[index];
                        dotFind = 1;
                    }
                    else
                    {
                        if (dotFind == 0)
                        {
                            ptr_Gnss->info.Longitude[index] = temp[index];
                        }
                        else
                        {
                            ptr_Gnss->info.Longitude[index + 1] = temp[index];
                        }
                    }
                }

                ptr_Gnss->info.Longitude[11] = '\'';
                ptr_Gnss->info.Longitude[12] = ' ';

                //memcpy(ptr_Gnss->info.Longitude, temp, 10);
                //ptr_Gnss->info.Longitude[10] = 0;
            }
            else
            {
                ptr_Gnss->info.Longitude[0] = 0;
                ptr_Gnss->info.Longitude[1] = 0;
            }

            ptr_Gnss->info.EW_Indicator = *(Real_Process_Comma((char *)buf, 6));

            ptr_Gnss->info.Speed = atof(Real_Process_Comma((char *)buf, 7));

            ptr_Gnss->info.Course = atof(Real_Process_Comma((char *)buf, 8));

            temp = Real_Process_Comma((char *)buf, 9);

            if ( *temp != 0 )
            {
                memcpy(ptr_Gnss->info.UTC_Date, temp, 6);
            }

            break;
        }

        case msgGSV:  //$xxGSV,2,1,07,07,79,048,42,02,51,062,43,26,36,256,42,27,27,138,42*71
        {
            frame_num = atof( Real_Process_Comma((char *)buf, 2));

            if((frame_num <= 3) && (frame_num != 0))
            {
                for(i = (frame_num - 1) * 4, j = 4; i < frame_num * 4; i++)
                {
                    ptr_Gnss->info.EPH_State[i][0] = atoi(Real_Process_Comma((char *)buf, j++ ));
                    ptr_Gnss->info.EPH_State[i][1] = atoi(Real_Process_Comma((char *)buf, j++ ));
                    ptr_Gnss->info.EPH_State[i][2] = atoi(Real_Process_Comma((char *)buf, j++ ));
                    ptr_Gnss->info.EPH_State[i][3] = atoi(Real_Process_Comma((char *)buf, j++ ));
                }
            }

            break;
        }

        case msgGLL:  //$xxGLL,3723.24755,N,12158.34161,W,161229.487,A*2C.
        {
            break;
        }

        case msgVTG:  //$xxVTG,309.62,T,M,0.13,N,0.2,K*6E
        {
            break;
        }

        default:
            break;
    }
}
static inline void ProcessCmd_private(u8 *buf, u32 len)
{
    static uint8_t rtn_data[19] = {0x55, 0xaa, 0x01, 0xb1, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0};

    switch(inst_UartFunc.u8Address)
    {
        case 0x01://ת̨
        {

            break;
        }

        default:
            break;
    }
}



static u8 Check_GPS_data(const u8* Buffer, u16 len)
{
    u16 i = 0;
    u8 j, k, sum;
    sum = 0;

    for (i = 1; i < len - 5; i++)
    {
        sum ^= Buffer[i];
    }

    j = Buffer[i + 1];
    k = Buffer[i + 2];

    if (isalpha(j))
    {
        if (isupper(j))
        {
            j -= 0x37;
        }
        else
        {
            j -= 0x57;
        }
    }
    else
    {
        if ((j >= 0x30) && (j <= 0x39))
        {
            j -= 0x30;
        }
    }

    if (isalpha(k))
    {
        if (isupper(k))
        {
            k -= 0x37;
        }
        else
        {
            k -= 0x57;
        }
    }
    else
    {
        if ((k >= 0x30) && (k <= 0x39))
        {
            k -= 0x30;
        }
    }

    j = (j << 4) + k;

    if (sum == j)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

static void Creat_Comma_Index(u8 *buffer, u16 len)
{
    unsigned short i;
    unsigned char idj;

    memset(Comma_id_sep, 0, sizeof(Comma_id_sep));

    for (i = 0, idj = 0; i < len; i++)
    {
        if (buffer[i] == ',')
        {
            Comma_id_sep[idj] = i;
            idj++;
            buffer[i] = 0x00;
        }
    }
}

static char *Real_Process_Comma(char *buffer, u8 num)
{
    if (num < 1)
        return  &buffer[0];

    return  &buffer[Comma_id_sep[num - 1] + 1];
}
