#include "headfile.h"
#include "UartFunction.h"
#include <stdlib.h>
#include <string.h>

static void init(void);
static void SendMessage(Support_ProtocolList proto, u8 addr, u16 u16msg, u8 *buf, u32 buflen);
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
    .ProtocolProcess               =               ProtocolProcess
};

_tUartFunc *ptr_PtFunc = &inst_UartFunc;

static void init(void)
{
    ptr_PtctrlQueue->InitCmdQueue(ptr_PtctrlQueue);
    inst_UartFunc.u8pBuf = (u8 *)malloc(sizeof(u8) * RCV_QUEUE_SIZE);
    inst_UartFunc.UartSendData = ps_Usart->Uart4_SendData;
}


static void processSendInternalCmd(u8 *buf, u32 buflen)
{
    u32 i = 0;

    for (i = 0; i < buflen; i++)
    {
        if (NULL != ptr_PtctrlQueue->pCqSendTop)
        {
            *(ptr_PtctrlQueue->pCqSendTop) =  *(u8 *)(buf + i);
            ptr_PtctrlQueue->pCqSendTop         = ptr_PtctrlQueue->NextCell(ptr_PtctrlQueue, SND_QUEEN, QUEEN_TOP);
        }
    }
}

static void processSendPelcoDPCmd(Support_ProtocolList proto, u8 address, u8 *buf, u32 buflen)
{
    u8 u8DataCheck = 0x0;
    u8 i = 0;
    u8 j = 0;

    u8 frame_headbuf[2] = {0x0};

    if(address != 5)
    {
        address = 1;//云台只能响应地址1
    }

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
                if (NULL != ptr_PtctrlQueue->pCqSendTop)
                {
                    if (2 > j)
                    {
                        *(ptr_PtctrlQueue->pCqSendTop) = frame_headbuf[j];
                    }
                    else if (6 > j)
                    {
                        *(ptr_PtctrlQueue->pCqSendTop) =  *(u8 *)(buf + j - 2);
                    }
                    else
                    {
                        *(ptr_PtctrlQueue->pCqSendTop) =  u8DataCheck;
                    }

                    ptr_PtctrlQueue->pCqSendTop         = ptr_PtctrlQueue->NextCell(ptr_PtctrlQueue, SND_QUEEN, QUEEN_TOP);
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
                if (NULL != ptr_PtctrlQueue->pCqSendTop)
                {
                    if (2 > j)
                    {
                        *(ptr_PtctrlQueue->pCqSendTop) =  frame_headbuf[j];
                    }
                    else if (6 > j)
                    {
                        *(ptr_PtctrlQueue->pCqSendTop) =  *(u8 *)(buf + j - 2);
                    }
                    else if (6 == j)
                    {
                        *(ptr_PtctrlQueue->pCqSendTop) =  0xAF;
                    }
                    else if (7 == j)
                    {
                        *(ptr_PtctrlQueue->pCqSendTop) =  u8DataCheck;
                    }

                    ptr_PtctrlQueue->pCqSendTop         = ptr_PtctrlQueue->NextCell(ptr_PtctrlQueue, SND_QUEEN, QUEEN_TOP);
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

    for (j = 0; j < buflen + 11; j++)
    {
        if (NULL != ptr_PtctrlQueue->pCqSendTop)
        {
            if (9 > j)
            {
                *(ptr_PtctrlQueue->pCqSendTop) = frame_headbuf[j];
            }
            else if ((9 + buflen) > j)
            {
                *(ptr_PtctrlQueue->pCqSendTop) = *(u8 *)(buf + j - 9);
            }
            else if ((buflen + 11 - 2) == j)
            {
                *(ptr_PtctrlQueue->pCqSendTop) = inst_PrivateProtocol.u8Reserve;
            }
            else if ((buflen + 11 - 1) == j)
            {
                *(ptr_PtctrlQueue->pCqSendTop) = inst_PrivateProtocol.u8FrameEnd;
            }

            ptr_PtctrlQueue->pCqSendTop         = ptr_PtctrlQueue->NextCell(ptr_PtctrlQueue, SND_QUEEN, QUEEN_TOP);
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
            processSendPelcoDPCmd(proto, addr, buf, buflen);
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

    u32Top = (u32)ptr_PtctrlQueue->pCqRecvTop;
    u32Btm = (u32)ptr_PtctrlQueue->pCqRecvBtm;

    if (u32Top != u32Btm)
    {
        cqBtm = ptr_PtctrlQueue->pCqRecvBtm;
        u8frameheadbuf[u8frameheadbuf_cnt] = *(cqBtm);
        u8frameheadbuf_cnt ++;
        cqBtm = ptr_PtctrlQueue->NextCell(ptr_PtctrlQueue, RCV_QUEEN, QUEEN_BTM);
        ptr_PtctrlQueue->pCqRecvBtm = cqBtm;

        if(u8frameheadbuf[0] == inst_PrivateProtocol.u8FrameStart[0])
        {
            if(2 <= u8frameheadbuf_cnt)
            {
                if(u8frameheadbuf[1] == inst_PrivateProtocol.u8FrameStart[1])
                {
                    if(9 == u8frameheadbuf_cnt)
                    {
                        inst_UartFunc.protoType = proto_private;
                        inst_UartFunc.u8Address = u8frameheadbuf[2];
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
                if ((u8frameheadbuf[1] == inst_PelcoD_Protocol.u8Address) || (u8frameheadbuf[1] == inst_PelcoD_Protocol.u8Address + 1))
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

    if (ptr_PtctrlQueue->pCqRecvBtm != ptr_PtctrlQueue->pCqRecvTop)
    {
        *(u8 *)(u8pDataBuf + u32BufLen) = *(ptr_PtctrlQueue->pCqRecvBtm);
        ptr_PtctrlQueue->pCqRecvBtm = ptr_PtctrlQueue->NextCell(ptr_PtctrlQueue, RCV_QUEEN, QUEEN_BTM);
        u32BufLen++;
    }

    if (u32BufLen == inst_UartFunc.u32BufLen)
    {
        u32BufLen = 0;

        switch (inst_UartFunc.protoType)
        {
            case proto_private:
            {
                if (inst_PrivateProtocol.u8FrameEnd == u8pDataBuf[inst_UartFunc.u32BufLen - 1])
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
    if (ptr_PtctrlQueue->pCqSendBtm != ptr_PtctrlQueue->pCqSendTop)
    {
        if (Databack_count > 5)
        {
            return;
        }

        while (ptr_PtctrlQueue->pCqSendBtm != ptr_PtctrlQueue->pCqSendTop)
        {
            inst_UartFunc.UartSendData(ptr_PtctrlQueue->pCqSendBtm, 1);
            ptr_PtctrlQueue->pCqSendBtm = ptr_PtctrlQueue->NextCell(ptr_PtctrlQueue, SND_QUEEN, QUEEN_BTM);
        }
    }
}


static inline void ProcessCmd_private(u8 *buf, u32 len)
{
    switch(inst_UartFunc.u8Address)
    {
        case 0x01://ת̨
        {
//				 float f_a;
//         s32 s_A;
//				ps_Pt->H_Angle = (*buf<<24|*(buf+1)<<16|*(buf+2)<<8|*(buf+3));
//				s_A = (*buf<<24|*(buf+1)<<16|*(buf+2)<<8|*(buf+3));
//				f_a = i32_to_f32(s_A);
//				ps_Pt->H_Angle = f_a *1000;
//				s_A = (*(buf+4)<<24|*(buf+5)<<16|*(buf+6)<<8|*(buf+7));
//				f_a = i32_to_f32(s_A);
//				ps_Pt->V_Angle = f_a *1000;
//				s_A = (*(buf+8)<<24|*(buf+9)<<16|*(buf+10)<<8|*(buf+11));
//				f_a = i32_to_f32(s_A);
//				ps_Pt->R_Angle = f_a *1000;
//				rtn_data[4] = (u8)(inst_UartFunc.u16cmdType);
//				memcpy(&rtn_data[9],buf,len);
//				ps_Pt->UartSendData(rtn_data,19);
//				ps_Pt->H_Angle = (*buf<<24|*(buf+1)<<16|*(buf+2)<<8|*(buf+3));
//				ps_Pt->V_Angle = (*(buf+4)<<24|*(buf+5)<<16|*(buf+6)<<8|*(buf+7));
            break;
        }

        default:
            break;
    }
}


static void ProcessCmd_Pelco_D_P(Support_ProtocolList proto, u8 *buf, u32 len)
{
    u16 u16cmd = 0x0, u16data;
    u8  u8data0 = 0;
    u8  u8data1 = 0;
    u16cmd = 0;
    u16cmd = *(buf);
    u16cmd = u16cmd << 8;
    u16cmd |= *(buf + 1);
    u8data0 = *(buf + 2);
    u8data1 = *(buf + 3);
    u16data = u8data0;
    u16data = u16data << 8;
    u16data |= u8data1;
    ptr_3588Func->SendMessage(protoPELCO_D, inst_UartFunc.u8Address, 0, buf, 4);

    if (0x0000 == (u16cmd & 0x0001))
    {

    }
    else
    {
        switch (u16cmd)
        {
            case 0x0059:
            {
                if(u16data != 0x100)
                {
                    ps_Pt->H_Angle = u16data;
                }

                break;
            }

            case 0x005b:
            {
                if(u16data > 18000)//飞越云台抬头角度360递减
                {
                    ps_Pt->V_Angle = (36000 - u16data);
                }
                else
                {
                    ps_Pt->V_Angle = (s32)(-u16data);
                }

                break;
            }

            case msgPelcoGotoPreset:
            {
                switch (u8data1)
                {
                    case 100:
                    {

                        break;
                    }

                    case 101:
                    {

                        break;
                    }

                    default:
                        break;
                }

                ptr_DebugFunc->SendMessage(protoPELCO_D, inst_PelcoD_Protocol.u8Address, 0, buf, 4); //µ÷ÓÃÔ¤ÖÃÎ»ÃüÁî×ª·¢¸øÔ¶³Ì°å
                break;
            }

            default :
                break;
        }
    }
}
