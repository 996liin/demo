#include "UartFunction.h"
#include "Queue.h"
#include <stdlib.h>
#include "message.h"
#include "delay.h"
#include <string.h>
#include "globalConfig.h"
#include "SysParam.h"
#include "LaserCtrl.h"
#include "RecvCtrl.h"
#include "TempCtrl.h"
#include "lensctrl.h"
#include "ADC124S.h"
static void init(u32 buad);
static void SendMessage(Support_ProtocolList proto, u16 u16msg, u8 *buf, u32 buflen);
static void ProtocolProcess(void);

static void RecvCmdProcess(void);
static void SendCmdProcess(void);

static inline void ProcessCmd_private(u8 *buf, u32 len);
static inline void ProcessCmd_Pelco_D_P(Support_ProtocolList proto, u8 *buf, u32 len);

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
    .ProtocolParseProcess               =               ProtocolProcess
};

_tUartFunc *ptr_Uart4Func = &inst_UartFunc;

static u8 pelco_stop_signal = 1;

static void init(u32 buad)
{
    inst_UartFunc.uart = ptr_Uart4;
    inst_UartFunc.uart->Init(buad);
    inst_UartFunc.u8pBuf = (u8 *)malloc(sizeof(u8) * RCV_QUEUE_SIZE);
}


static void processSendInternalCmd(u8 *buf, u32 buflen)
{
    u32 i = 0;

    for (i = 0; i < buflen; i++)
    {
        if (NULL != ptr_Uart4Queue->pCqSendTop)
        {
            *(ptr_Uart4Queue->pCqSendTop) =  *(u8 *)(buf + i);
            ptr_Uart4Queue->pCqSendTop         = ptr_Uart4Queue->NextCell(ptr_Uart4Queue, SND_QUEEN, QUEEN_TOP);
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
                if (NULL != ptr_Uart4Queue->pCqSendTop)
                {
                    if (2 > j)
                    {
                        *(ptr_Uart4Queue->pCqSendTop) = frame_headbuf[j];
                    }
                    else if (6 > j)
                    {
                        *(ptr_Uart4Queue->pCqSendTop) =  *(u8 *)(buf + j - 2);
                    }
                    else
                    {
                        *(ptr_Uart4Queue->pCqSendTop) =  u8DataCheck;
                    }

                    ptr_Uart4Queue->pCqSendTop         = ptr_Uart4Queue->NextCell(ptr_Uart4Queue, SND_QUEEN, QUEEN_TOP);
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
                if (NULL != ptr_Uart4Queue->pCqSendTop)
                {
                    if (2 > j)
                    {
                        *(ptr_Uart4Queue->pCqSendTop) =  frame_headbuf[j];
                    }
                    else if (6 > j)
                    {
                        *(ptr_Uart4Queue->pCqSendTop) =  *(u8 *)(buf + j - 2);
                    }
                    else if (6 == j)
                    {
                        *(ptr_Uart4Queue->pCqSendTop) =  0xAF;
                    }
                    else if (7 == j)
                    {
                        *(ptr_Uart4Queue->pCqSendTop) =  u8DataCheck;
                    }

                    ptr_Uart4Queue->pCqSendTop         = ptr_Uart4Queue->NextCell(ptr_Uart4Queue, SND_QUEEN, QUEEN_TOP);
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
    frame_headbuf[2] = inst_PrivateProtocol.u8Address;
    frame_headbuf[3] = (u16cmdMsg >> 8) & 0xFF;
    frame_headbuf[4] = u16cmdMsg & 0xFF;
    frame_headbuf[5] = (buflen >> 24) & 0xFF;
    frame_headbuf[6] = (buflen >> 16) & 0xFF;
    frame_headbuf[7] = (buflen >> 8) & 0xFF;
    frame_headbuf[8] = (buflen >> 0) & 0xFF;
    frame_headbuf[9] = (frame_headbuf[3] + frame_headbuf[4] + frame_headbuf[5] + frame_headbuf[6] + frame_headbuf[7] + frame_headbuf[8]) & 0xFF;

    for (j = 0; j < buflen + 12; j++)
    {
        if (NULL != ptr_Uart4Queue->pCqSendTop)
        {
            if (10 > j)
            {
                *(ptr_Uart4Queue->pCqSendTop) = frame_headbuf[j];
            }
            else if ((10 + buflen) > j)
            {
                *(ptr_Uart4Queue->pCqSendTop) = *(u8 *)(buf + j - 10);
            }
            else if ((buflen + 12 - 2) == j)
            {
                *(ptr_Uart4Queue->pCqSendTop) = inst_PrivateProtocol.u8Reserve;
            }
            else if ((buflen + 12 - 1) == j)
            {
                *(ptr_Uart4Queue->pCqSendTop) = inst_PrivateProtocol.u8FrameEnd;
            }

            ptr_Uart4Queue->pCqSendTop         = ptr_Uart4Queue->NextCell(ptr_Uart4Queue, SND_QUEEN, QUEEN_TOP);
        }
    }
}

static void SendMessage(Support_ProtocolList proto, u16 u16msg, u8 *buf, u32 buflen)
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
    u8 u8headcheck = 0;

    static u8 u8frameheadbuf[SEARCHFRAMEHEAD_LEN];
    static u8 u8frameheadbuf_cnt = 0;

    u32Top = (u32)ptr_Uart4Queue->pCqRecvTop;
    u32Btm = (u32)ptr_Uart4Queue->pCqRecvBtm;

    if (u32Top != u32Btm)
    {
        cqBtm = ptr_Uart4Queue->pCqRecvBtm;
        u8frameheadbuf[u8frameheadbuf_cnt] = *(cqBtm);
        u8frameheadbuf_cnt ++;
        cqBtm = ptr_Uart4Queue->NextCell(ptr_Uart4Queue, RCV_QUEEN, QUEEN_BTM);
        ptr_Uart4Queue->pCqRecvBtm = cqBtm;

        if(u8frameheadbuf[0] == inst_PrivateProtocol.u8FrameStart[0])
        {
            if(2 <= u8frameheadbuf_cnt)
            {
                if(u8frameheadbuf[1] == inst_PrivateProtocol.u8FrameStart[1])
                {
                    if(3 <= u8frameheadbuf_cnt)
                    {
                        if(u8frameheadbuf[2] == inst_PrivateProtocol.u8Address)
                        {
                            if(10 == u8frameheadbuf_cnt)
                            {
                                u8headcheck = 0;

                                for(i = 0; i < 6; i++)
                                {
                                    u8headcheck += u8frameheadbuf[3 + i];
                                }

                                if (u8headcheck == u8frameheadbuf[9])
                                {
                                    inst_UartFunc.protoType = proto_private;

                                    inst_UartFunc.u16cmdType = 0;
                                    inst_UartFunc.u16cmdType = u8frameheadbuf[3];
                                    inst_UartFunc.u16cmdType <<= 8;
                                    inst_UartFunc.u16cmdType |= u8frameheadbuf[4];

                                    inst_UartFunc.u32BufLen = 0;
                                    inst_UartFunc.u32BufLen = u8frameheadbuf[5];
                                    inst_UartFunc.u32BufLen <<= 8;
                                    inst_UartFunc.u32BufLen |= u8frameheadbuf[6];
                                    inst_UartFunc.u32BufLen <<= 8;
                                    inst_UartFunc.u32BufLen |= u8frameheadbuf[7];
                                    inst_UartFunc.u32BufLen <<= 8;
                                    inst_UartFunc.u32BufLen |= u8frameheadbuf[8];

                                    inst_UartFunc.u32BufLen = inst_UartFunc.u32BufLen + 2;

                                    for(i = 0; i < u8frameheadbuf_cnt; i++)
                                    {
                                        u8frameheadbuf[0] = 0x0;
                                    }

                                    u8frameheadbuf_cnt = 0;
                                    return 1;
                                }
                                else
                                {
                                    for(i = 0; i < u8frameheadbuf_cnt; i++)
                                    {
                                        u8frameheadbuf[0] = 0x0;
                                    }

                                    u8frameheadbuf_cnt = 0;
                                }
                            }
                        }
                    }
                }
                else
                {
                    u8frameheadbuf[0] = u8frameheadbuf[1];
                    u8frameheadbuf[1] = 0x0;
                    u8frameheadbuf_cnt = 1;
                }
            }
        }
        else if (u8frameheadbuf[0] == inst_PelcoD_Protocol.u8FrameStart[0])
        {
            if (2 == u8frameheadbuf_cnt)
            {
                if (u8frameheadbuf[1] == inst_PelcoD_Protocol.u8Address)
                {
                    inst_UartFunc.protoType = protoPELCO_D;
                    inst_UartFunc.u8Address = u8frameheadbuf[1];
                    inst_UartFunc.u32BufLen = 5;

                    u8frameheadbuf_cnt = 0;
                    u8frameheadbuf[0] = 0x0;
                    u8frameheadbuf[1] = 0x0;
                    return 1;
                }
                else
                {
                    u8frameheadbuf[0] = u8frameheadbuf[1];
                    u8frameheadbuf[1] = 0x0;
                    u8frameheadbuf_cnt = 1;
                }
            }
        }
        else if (u8frameheadbuf[0] == inst_PelcoP_Protocol.u8FrameStart[0])
        {
            if (2 == u8frameheadbuf_cnt)
            {
                if (u8frameheadbuf[1] == inst_PelcoP_Protocol.u8Address)
                {
                    inst_UartFunc.protoType = protoPELCO_P;
                    inst_UartFunc.u8Address = u8frameheadbuf[1];
                    inst_UartFunc.u32BufLen = 6;

                    u8frameheadbuf_cnt = 0;
                    u8frameheadbuf[0] = 0x0;
                    u8frameheadbuf[1] = 0x0;
                    return 1;
                }
                else
                {
                    u8frameheadbuf[0] = u8frameheadbuf[1];
                    u8frameheadbuf[1] = 0x0;
                    u8frameheadbuf_cnt = 1;
                }
            }
        }
        else
        {
            if (u8frameheadbuf_cnt == 2)
            {
                u8frameheadbuf[0] = u8frameheadbuf[1];
                u8frameheadbuf[1] = 0x0;
                u8frameheadbuf_cnt = 1;
            }
            else
            {
                u8frameheadbuf_cnt = 0;
                u8frameheadbuf[0] = 0x0;
            }
        }
    }

    return 0;
}

static inline u8 SearchFrameEnd(void)
{
    u8 u8DataCheck = 0x0;
    u8 i = 0;
    u8 rtn_data = 0;

    static u32 u32BufLen = 0;
    u8 *u8pDataBuf = inst_UartFunc.u8pBuf;

    if (ptr_Uart4Queue->pCqRecvBtm != ptr_Uart4Queue->pCqRecvTop)
    {
        *(u8 *)(u8pDataBuf + u32BufLen) = *(ptr_Uart4Queue->pCqRecvBtm);
        ptr_Uart4Queue->pCqRecvBtm = ptr_Uart4Queue->NextCell(ptr_Uart4Queue, RCV_QUEEN, QUEEN_BTM);
        u32BufLen++;
    }

    if (u32BufLen == inst_UartFunc.u32BufLen)
    {
        u32BufLen = 0;

        switch (inst_UartFunc.protoType)
        {
            case proto_private:
            {
                if ((inst_PrivateProtocol.u8Reserve  == u8pDataBuf[inst_UartFunc.u32BufLen - 2]) && (inst_PrivateProtocol.u8FrameEnd == u8pDataBuf[inst_UartFunc.u32BufLen - 1]))
                {
                    rtn_data = 1;
                }
                else
                {
                    rtn_data = 2;
                }

                break;
            }

            case protoPELCO_D:
            {
                u8DataCheck = 0x0;
                u8DataCheck += inst_UartFunc.u8Address;

                for (i = 0; i < inst_UartFunc.u32BufLen - 1; i++)
                {
                    u8DataCheck += *(u8 *)(u8pDataBuf + i);
                }

                if (u8DataCheck == u8pDataBuf[inst_UartFunc.u32BufLen - 1])
                {
                    rtn_data = 1;
                }
                else
                {
                    rtn_data = 2;
                }

                break;
            }

            case protoPELCO_P:
            {
                u8DataCheck = 0x0;
                u8DataCheck ^= inst_UartFunc.u8Address;

                for (i = 0; i < inst_UartFunc.u32BufLen - 2; i++)
                {
                    u8DataCheck ^= *(u8 *)(u8pDataBuf + i);
                }

                if ((inst_PelcoP_Protocol.u8Reserve == u8pDataBuf[inst_UartFunc.u32BufLen - 2]) && (u8DataCheck == u8pDataBuf[inst_UartFunc.u32BufLen - 1]))
                {
                    rtn_data = 1;
                }
                else
                {
                    rtn_data = 2;
                }

                break;
            }

            default :
                break;
        }
    }

    return rtn_data;
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
        switch (inst_UartFunc.protoType)
        {
            case proto_private:
                ProcessCmd_private(inst_UartFunc.u8pBuf, inst_UartFunc.u32BufLen);
                break;

            case protoPELCO_D:
            case protoPELCO_P:
                ProcessCmd_Pelco_D_P(inst_UartFunc.protoType, inst_UartFunc.u8pBuf, inst_UartFunc.u32BufLen);
                break;

            default:
                break;
        }

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
    if (ptr_Uart4Queue->pCqSendBtm != ptr_Uart4Queue->pCqSendTop)
    {

        while (ptr_Uart4Queue->pCqSendBtm != ptr_Uart4Queue->pCqSendTop)
        {
            inst_UartFunc.uart->SendByte(inst_UartFunc.uart->uart, *(ptr_Uart4Queue->pCqSendBtm));
            ptr_Uart4Queue->pCqSendBtm = ptr_Uart4Queue->NextCell(ptr_Uart4Queue, SND_QUEEN, QUEEN_BTM);
        }

    }
}


static inline void ProcessCmd_private(u8 *buf, u32 len)
{
    uint8_t data = 0, i, data1;
    uint16_t u16data = 0;
    uint32_t u32data = 0;

    static uint8_t rtn_data[100];

    data = *buf;

    switch (inst_UartFunc.u16cmdType)
    {
        //////////////////////////////////////////////////////////////////////////////////
        case msgUpdateLink:
            ptr_RecvCtrl->Uart->SendMessage(proto_private, msgUpdateLink, buf, 0);
            break;

        case msgUpdateStart:
            ptr_Param->SetUpdateFlag();
            NVIC_SystemReset();
            break;

        //////////////////////////////////////////////////////////////////////////////////
        case msgPrivateBoardLink:
            ptr_RecvCtrl->Uart->SendMessage(proto_private, msgPrivateBoardLink, buf, 0);
            break;

        case msgPrivateSetPower:
            u16data = *(buf + 0);
            u16data = u16data << 8;
            u16data |= *(buf + 1);

            if (ptr_Temp->setTempValueH < ptr_Temp->curTempValue)
            {
                if (ptr_Laser->laserState == LASER_OPEN)
                {
                    ptr_Laser->SetPower(u16data);
                    rtn_data[0] = 0;
                    u32data = f32_to_i32(ptr_Temp->curTempValue);
                    rtn_data[1] = (u32data >> 24) & 0xFF;
                    rtn_data[2] = (u32data >> 16) & 0xFF;
                    rtn_data[3] = (u32data >> 8) & 0xFF;
                    rtn_data[4] = (u32data >> 0) & 0xFF;
                    ptr_RecvCtrl->Uart->SendMessage(proto_private, msgPrivateLaserOpenTempHigh, rtn_data, 5);
                }
                else
                {
                    rtn_data[0] = 1;
                    u32data = f32_to_i32(ptr_Temp->curTempValue);
                    rtn_data[1] = (u32data >> 24) & 0xFF;
                    rtn_data[2] = (u32data >> 16) & 0xFF;
                    rtn_data[3] = (u32data >> 8) & 0xFF;
                    rtn_data[4] = (u32data >> 0) & 0xFF;
                    ptr_RecvCtrl->Uart->SendMessage(proto_private, msgPrivateLaserOpenTempHigh, rtn_data, 5);
                }
            }
            else
            {
                ptr_Laser->SetPower(u16data);
                rtn_data[0] = 0;
                u32data = f32_to_i32(ptr_Temp->curTempValue);
                rtn_data[1] = (u32data >> 24) & 0xFF;
                rtn_data[2] = (u32data >> 16) & 0xFF;
                rtn_data[3] = (u32data >> 8) & 0xFF;
                rtn_data[4] = (u32data >> 0) & 0xFF;
                ptr_RecvCtrl->Uart->SendMessage(proto_private, msgPrivateLaserOpenTempHigh, rtn_data, 5);
            }

            break;

        case msgPrivateSetTemp:
            u16data = *(buf + 0);
            u16data = u16data << 8;
            u16data |= *(buf + 1);
            ptr_Temp->SetTemp(u16data);
            break;

        case msgPrivateAdcValueRtn:
            rtn_data[0] = (ptr_Laser->curLaserPower >> 8) & 0xFF;
            rtn_data[1] = (ptr_Laser->curLaserPower >> 0) & 0xFF;

            u32data = f32_to_i32(ptr_Temp->curTempValue);
            rtn_data[2] = (u32data >> 24) & 0xFF;
            rtn_data[3] = (u32data >> 16) & 0xFF;
            rtn_data[4] = (u32data >> 8) & 0xFF;
            rtn_data[5] = (u32data >> 0) & 0xFF;

            ptr_RecvCtrl->Uart->SendMessage(proto_private, msgPrivateAdcValueRtn, rtn_data, 6);
            break;

        case msgPrivateLaserCaliInfoQuery:
            rtn_data[0] = ptr_Laser->caliInfo.caliPointNum;

            for (i = 0; i < ptr_Laser->caliInfo.caliPointNum; i++)
            {
                rtn_data[1 + i * 6] = (ptr_Laser->caliInfo.realPowerPoint[i] >> 8) & 0xFF;
                rtn_data[2 + i * 6] = (ptr_Laser->caliInfo.realPowerPoint[i] >> 0) & 0xFF;

                rtn_data[3 + i * 6] = (ptr_Laser->caliInfo.setedCurrentPoint[i] >> 8) & 0xFF;
                rtn_data[4 + i * 6] = (ptr_Laser->caliInfo.setedCurrentPoint[i] >> 0) & 0xFF;

                rtn_data[5 + i * 6] = (ptr_Laser->caliInfo.pdValuePoint[i] >> 8) & 0xFF;
                rtn_data[6 + i * 6] = (ptr_Laser->caliInfo.pdValuePoint[i] >> 0) & 0xFF;
            }

            ptr_RecvCtrl->Uart->SendMessage(proto_private, msgPrivateLaserCaliInfoQuery, rtn_data, 1 + ptr_Laser->caliInfo.caliPointNum * 6);
            break;

        case msgPrivateLaserCaliStart:
            ptr_Laser->cali_mode = 1;
            ptr_laserDac->SetValueByCurrentMA(LASER_POWER_CH, 0);
            ptr_Laser->Open();
            break;

        case msgPrivateLaserCaliStop:
            ptr_Laser->cali_mode = 0;
            ptr_Laser->Close();
            break;

        case msgPrivateLaserCaliInfoClear:
            ptr_Laser->ClearAllCaliPoint();

            rtn_data[0] = ptr_Laser->caliInfo.caliPointNum;
            ptr_RecvCtrl->Uart->SendMessage(proto_private, msgPrivateLaserCaliPointNumRtn, rtn_data, 1);
            break;

        case msgPrivateLaserCaliCurSet:
            u16data = *(buf + 0);
            u16data = u16data << 8;
            u16data |= *(buf + 1);

            ptr_laserDac->SetValueByCurrentMA(LASER_POWER_CH, u16data);
            break;

        case msgPrivateLaserCali:
            u16data = *(buf + 0);
            u16data = u16data << 8;
            u16data |= *(buf + 1);

            ptr_Laser->AddCaliPoint(u16data);

            rtn_data[0] = ptr_Laser->caliInfo.caliPointNum;
            ptr_RecvCtrl->Uart->SendMessage(proto_private, msgPrivateLaserCaliPointNumRtn, rtn_data, 1);

            break;

        case msgPrivateLaserCaliParamSave:
            ptr_Param->SaveLaserCali();
            break;

        case msgPrivateBoardInfoQuary:

//        rtn_data[0] = (ptr_Laser->setLaserPower >> 8) & 0xFF;
//        rtn_data[1] = ptr_Laser->setLaserPower & 0xFF;
//        rtn_data[2] = (ptr_Temp->setTempValue >> 8) & 0xFF;
//        rtn_data[3] = ptr_Temp->setTempValue & 0xFF;
//
//        ptr_RecvCtrl->Uart->SendMessage(proto_private, msgPrivateBoardInfoQuary, rtn_data, 4);
            break;

        case msgPrivateLaserParamSave:
            ptr_Param->SaveLaserParam();
            break;

        default :
            break;
    }
}


static void ProcessCmd_Pelco_D_P(Support_ProtocolList proto, u8 *buf, u32 len)
{
    u16 u16cmd = 0x0, u16data;
    u8  u8data0 = 0;
    u8  u8data1 = 0;
    u8  u8PresetNo = 0;

    u16cmd = 0;
    u16cmd = *(buf);
    u16cmd = u16cmd << 8;
    u16cmd |= *(buf + 1);

    u8data0 = *(buf + 2);
    u8data1 = *(buf + 3);

    u16data = u8data0;
    u16data = u16data << 8;
    u16data |= u8data1;


    if (0x0000 == (u16cmd & 0x0001))
    {
        switch (u16cmd)
        {
            case msgPelcoClose:
            {
                break;
            }

            case msgPelcoOpen:
            {
                break;
            }

            case msgPelcoNear:
            {
                break;
            }

            case msgPelcoFar:
            {
                break;
            }

            case msgPelcoWide:
            {
                ps_Lens->LensLaser(2);//WIDE
                break;
            }

            case msgPelcoTele:
            {
                ps_Lens->LensLaser(1);//TELE
                break;
            }

            default:
                break;
        }//end swtich

        if ((u16cmd & 0x0006) != 0)
        {
            if ((u16cmd & 0x0002) != 0)
            {
                if (u8data0 != 0)
                {
                    if (pelco_stop_signal == 1)
                    {
                        pelco_stop_signal = 0;
                    }
                }
            }
            else
            {
                if (u8data0 != 0)
                {
                    if (pelco_stop_signal == 1)
                    {
                        pelco_stop_signal = 0;
                    }
                }
            }
        }

        if ((u16cmd & 0x0018) != 0)
        {
            if ((u16cmd & 0x0008) != 0)
            {
                if (u8data1 != 0)
                {
                    if (pelco_stop_signal == 1)
                    {
                        pelco_stop_signal = 0;
                    }
                }
            }
            else
            {
                if (u8data1 != 0)
                {
                    if (pelco_stop_signal == 1)
                    {
                        pelco_stop_signal = 0;
                    }
                }
            }
        }

        if (msgPelcoStop == u16cmd)
        {
            ps_Lens->LensLaser(3);//STOP
            pelco_stop_signal = 1;
        }
    }
    else
    {
        u8PresetNo = u8data1;

        switch (u16cmd)
        {
            case msgPelcoSetPreset://set preset
            {
                if ((u8PresetNo <= 20) || (u8PresetNo >= 80))
                {
                }

                break;
            }

            case msgPelcoGotoPreset:
            {
                if ((u8PresetNo <= 20) || (u8PresetNo >= 80))
                {
                    ps_Lens->LensSelfCheck();
                }

                switch (u8PresetNo)
                {
                    default:
                        break;
                }

                break;
            }

            case msgPelcoClearPreset:
            {
                break;
            }

            case msgPelcoOpenAux:
            {
                ptr_laserDac->SetValueByMVol(LASER_POWER_CH, 750);
                ptr_Laser->Open();
                break;
            }

            case msgPelcoCloseAux:
            {
                ptr_laserDac->SetValueByMVol(LASER_POWER_CH, 0);
                ptr_Laser->Close();
                break;
            }

            case msgPelcoSetPanPositin:
            {
                break;
            }

            case msgPelcoSetTiltPosion:
            {
                break;
            }

            case msgPelcoSetZoomPosion:
            {
                ps_Lens->SetCamZoomPosi(u16data);
                break;
            }

            case msgPelcoQueryPanPositin:
            {
                break;
            }

            case msgPelcoQueryTiltPosion:
            {
                break;
            }

            case msgPelcoQueryZoomPosion:
            {
                u8 databack[4] = {0x00, 0x5d, 0x00, 0x00};
                u16 zoomvalue;
                zoomvalue = ptr_adc124->GetZoomAD();
                databack[2] = zoomvalue >> 8;
                databack[3] = zoomvalue & 0xff;
                ptr_RecvCtrl4->Uart->SendMessage(protoPELCO_D, 1, databack, 4);
                break;
            }

            default :
                break;
        }
    }
}
