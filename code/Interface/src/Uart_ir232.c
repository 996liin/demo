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

_tUartFunc *ptr_Ir232Func = &inst_UartFunc;

static void init(void)
{
    ptr_IR232Queue->InitCmdQueue(ptr_IR232Queue);
    inst_UartFunc.u8pBuf = (u8 *)malloc(sizeof(u8) * RCV_QUEUE_SIZE);
    inst_UartFunc.UartSendData = ps_Usart->Usart1_SendData;
}


static void processSendInternalCmd(u8 *buf, u32 buflen)
{
    u32 i = 0;

    for (i = 0; i < buflen; i++)
    {
        if (NULL != ptr_IR232Queue->pCqSendTop)
        {
            *(ptr_IR232Queue->pCqSendTop) =  *(u8 *)(buf + i);
            ptr_IR232Queue->pCqSendTop         = ptr_IR232Queue->NextCell(ptr_IR232Queue, SND_QUEEN, QUEEN_TOP);
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
                if (NULL != ptr_IR232Queue->pCqSendTop)
                {
                    if (2 > j)
                    {
                        *(ptr_IR232Queue->pCqSendTop) = frame_headbuf[j];
                    }
                    else if (6 > j)
                    {
                        *(ptr_IR232Queue->pCqSendTop) =  *(u8 *)(buf + j - 2);
                    }
                    else
                    {
                        *(ptr_IR232Queue->pCqSendTop) =  u8DataCheck;
                    }

                    ptr_IR232Queue->pCqSendTop         = ptr_IR232Queue->NextCell(ptr_IR232Queue, SND_QUEEN, QUEEN_TOP);
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
                if (NULL != ptr_IR232Queue->pCqSendTop)
                {
                    if (2 > j)
                    {
                        *(ptr_IR232Queue->pCqSendTop) =  frame_headbuf[j];
                    }
                    else if (6 > j)
                    {
                        *(ptr_IR232Queue->pCqSendTop) =  *(u8 *)(buf + j - 2);
                    }
                    else if (6 == j)
                    {
                        *(ptr_IR232Queue->pCqSendTop) =  0xAF;
                    }
                    else if (7 == j)
                    {
                        *(ptr_IR232Queue->pCqSendTop) =  u8DataCheck;
                    }

                    ptr_IR232Queue->pCqSendTop         = ptr_IR232Queue->NextCell(ptr_IR232Queue, SND_QUEEN, QUEEN_TOP);
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
        if (NULL != ptr_IR232Queue->pCqSendTop)
        {
            if (10 > j)
            {
                *(ptr_IR232Queue->pCqSendTop) = frame_headbuf[j];
            }
            else if ((10 + buflen) > j)
            {
                *(ptr_IR232Queue->pCqSendTop) = *(u8 *)(buf + j - 10);
            }
            else if ((buflen + 12 - 2) == j)
            {
                *(ptr_IR232Queue->pCqSendTop) = inst_PrivateProtocol.u8Reserve;
            }
            else if ((buflen + 12 - 1) == j)
            {
                *(ptr_IR232Queue->pCqSendTop) = inst_PrivateProtocol.u8FrameEnd;
            }

            ptr_IR232Queue->pCqSendTop         = ptr_IR232Queue->NextCell(ptr_IR232Queue, SND_QUEEN, QUEEN_TOP);
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

    u32Top = (u32)ptr_IR232Queue->pCqRecvTop;
    u32Btm = (u32)ptr_IR232Queue->pCqRecvBtm;

    if (u32Top != u32Btm)
    {
        cqBtm = ptr_IR232Queue->pCqRecvBtm;
        u8frameheadbuf[u8frameheadbuf_cnt] = *(cqBtm);
        u8frameheadbuf_cnt ++;
        cqBtm = ptr_IR232Queue->NextCell(ptr_IR232Queue, RCV_QUEEN, QUEEN_BTM);
        ptr_IR232Queue->pCqRecvBtm = cqBtm;

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

    if (ptr_IR232Queue->pCqRecvBtm != ptr_IR232Queue->pCqRecvTop)
    {
        *(u8 *)(u8pDataBuf + u32BufLen) = *(ptr_IR232Queue->pCqRecvBtm);
        ptr_IR232Queue->pCqRecvBtm = ptr_IR232Queue->NextCell(ptr_IR232Queue, RCV_QUEEN, QUEEN_BTM);
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
    if (ptr_IR232Queue->pCqSendBtm != ptr_IR232Queue->pCqSendTop)
    {
        if (Databack_count > 5)
        {
            return;
        }

        while (ptr_IR232Queue->pCqSendBtm != ptr_IR232Queue->pCqSendTop)
        {
            inst_UartFunc.UartSendData(ptr_IR232Queue->pCqSendBtm, 1);
            ptr_IR232Queue->pCqSendBtm = ptr_IR232Queue->NextCell(ptr_IR232Queue, SND_QUEEN, QUEEN_BTM);
        }
    }
}


static inline void ProcessCmd_private(u8 *buf, u32 len)
{
    switch(inst_UartFunc.u8Address)
    {
//			case 0x01://??
//			{
//				ps_Pt->H_Angle = (*buf<<24|*(buf+1)<<16|*(buf+2)<<8|*(buf+3));
//				ps_Pt->V_Angle = (*(buf+4)<<24|*(buf+5)<<16|*(buf+6)<<8|*(buf+7));
////				rtn_data[4] = (u8)(inst_UartFunc.u16cmdType);
////				memcpy(&rtn_data[9],buf,len);
////				ps_Pt->UartSendData(rtn_data,19);
//				break;
//			}
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

    if (0x0000 == (u16cmd & 0x0001))
    {
        if((u16cmd & 0x001e) != 0)
        {
            if(1)
            {

                if (ps_Pt->pt_auto_speed)
                {
                    u8 spd_p, spd_t;

                    if(inst_UartFunc.u8Address == inst_PelcoD_Protocol.u8Address)  //ps_IR->zoomvalueOffset     ZOOMIRRANGE
                    {
                        if (ps_IR->zoomvalueOffset > ZOOMIRRANGE)  //最长焦
                        {
                            spd_p = 5;
                            spd_t = 5;
                        }
                        else
                        {
                            spd_p =  (ZOOMIRRANGE - ps_IR->zoomvalueOffset) * u8data0 / ZOOMIRRANGE;
                            spd_t =  (ZOOMIRRANGE - ps_IR->zoomvalueOffset) * u8data1 / ZOOMIRRANGE;
                        }

                        if(spd_p < 5)
                        {
                            spd_p = 5;
                        }
                        else if (spd_p > 63)
                        {
                            spd_p = 63;
                        }

                        if(spd_t < 5)
                        {
                            spd_t = 5;
                        }
                        else if (spd_t > 63)
                        {
                            spd_t = 63;
                        }

                        if(*(buf + 2) >= spd_p)
                        {
                            *(buf + 2) = spd_p;
                        }

                        if(*(buf + 3) >= spd_t)
                        {
                            *(buf + 3) = spd_t;
                        }
                    }
                }



//					u8 spd_p,spd_t;
//					if(inst_UartFunc.u8Address == inst_PelcoD_Protocol.u8Address+1)
//					{
//						spd_p = ps_CCD->Search_zoom_view(ps_CCD->zoomvalue) * u8data0/MAXZOOM_CCD;
//						spd_t = ps_CCD->Search_zoom_view(ps_CCD->zoomvalue) * u8data1/MAXZOOM_CCD ;
//						if(spd_p < 4)
//						{
//							spd_p = 4;
//						}
//						if(spd_t < 3)
//						{
//							spd_t = 3;
//						}
//						if(*(buf + 2) >= spd_p)
//						{
//							*(buf + 2) = spd_p;
//						}
//						if(*(buf + 3) >= spd_t)
//						{
//							*(buf + 3) = spd_t;
//						}
//					}
//					else if(inst_UartFunc.u8Address == inst_PelcoD_Protocol.u8Address)
//					{
//						spd_p =  u8data0 *(MAXZOOMIR-ps_IR->zoomvalueOffset)/MAXZOOMIR;
//						spd_t =  u8data1 *(MAXZOOMIR-ps_IR->zoomvalueOffset)/MAXZOOMIR;
//						if(spd_p < 5)
//						{
//							spd_p = 5;
//						}
//						if(spd_t < 5)
//						{
//							spd_t = 5;
//						}
//						if(*(buf + 2) >= spd_p)
//						{
//							*(buf + 2) = spd_p;
//						}
//						if(*(buf + 3) >= spd_t)
//						{
//							*(buf + 3) = spd_t;
//						}
//					}
            }
        }

        if (msgPelcoStop == u16cmd)
        {
            if(inst_UartFunc.u8Address == inst_PelcoD_Protocol.u8Address + 1)
            {
                ps_CCD->Camera_Stop();
            }
            else if(inst_UartFunc.u8Address == inst_PelcoD_Protocol.u8Address)
            {
                ps_IR->Camera_zoom_Stop();
            }
        }

        if(!ps_Pt->start_track_flag)
        {
            ptr_PtFunc->SendMessage(protoPELCO_D, inst_UartFunc.u8Address, 0, buf, 4);
//				ps_Systick->delay_ms(5);
//				ptr_PtFunc->SendMessage(protoPELCO_D,inst_UartFunc.u8Address,0,buf,4);

        }

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
                if(inst_UartFunc.u8Address == inst_PelcoD_Protocol.u8Address + 1)
                {
                    ps_CCD->Camera_FocusNear();
                }
                else if(inst_UartFunc.u8Address == inst_PelcoD_Protocol.u8Address)
                {
                    ps_IR->Camera_FocusNear();
                }

                break;
            }

            case msgPelcoFar:
            {
                if(inst_UartFunc.u8Address == inst_PelcoD_Protocol.u8Address + 1)
                {
                    ps_CCD->Camera_FocusFar();
                }
                else if(inst_UartFunc.u8Address == inst_PelcoD_Protocol.u8Address)
                {
                    ps_IR->Camera_FocusFar();
                }

                break;
            }

            case msgPelcoWide:
            {
                if(inst_UartFunc.u8Address == inst_PelcoD_Protocol.u8Address + 1)
                {
                    ps_CCD->Camera_ZoomWide();
                }
                else if(inst_UartFunc.u8Address == inst_PelcoD_Protocol.u8Address)
                {
                    ps_IR->Camera_ZoomWide();
                }
                else {}

                break;
            }

            case msgPelcoTele:
            {
                if(inst_UartFunc.u8Address == inst_PelcoD_Protocol.u8Address + 1)
                {
                    ps_CCD->Camera_ZoomTele();
                }
                else if(inst_UartFunc.u8Address == inst_PelcoD_Protocol.u8Address)
                {
                    ps_IR->Camera_ZoomTele();
                }
                else {}

                break;
            }

            default:
                break;
        }
    }
    else
    {
        if((u16cmd != 0x51) && (u16cmd != 0x53) && (u16cmd != 0x5d) && (u16cmd != 0x4d) && (u16cmd != 0x4f))
        {
            if(1)
            {
                ptr_PtFunc->SendMessage(protoPELCO_D, inst_UartFunc.u8Address, 0, buf, 4);
            }
        }

        switch (u16cmd)
        {
            case msgPelcoSetPreset://set preset
            {
                if (u8data1 >= 131)
                {
                    Preset_Save(u8data1);
                }

                break;
            }

            case msgPelcoGotoPreset:
            {
                if (u8data1 >= 131)
                {
                    Preset_Call(u8data1);
                }
                else
                {
                    switch (u8data1)
                    {
                        case 55:
                        {
                            static u8 uc_com[12] = {0x55, 0xAA, 0x02, 0x00, 0x04, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0xF0};
                            ps_Usart->Uart3_SendData(uc_com, 12);
                            ps_IR->hot_open_flag = 1;
                            break;
                        }

                        case 63:
                        {
                            ptr_Wiper->open_flag = 1;
                            break;
                        }

                        case 64:
                        {
                            ps_IR->Camera_AutoFocusOn();
                            break;
                        }

                        case 65:
                        {
                            ps_IR->Camera_AutoFocusOff();
                            break;
                        }

                        case 66:
                        {
                            ps_IR->Camera_AutoFocusOnce();
                            break;
                        }

                        case 67:
                        {
                            ps_Pt->slow_move_flag = 1;
                            break;
                        }

                        case 76:
                        {
                            ps_Pt->slow_move_flag = 0;
                            break;
                        }

                        case 72:
                        {
                            ps_IR->Shutter_Ctrl(1);
                            break;
                        }

                        case 73:
                        {
                            ps_IR->Shutter_Ctrl(0);
                            break;
                        }

                        case 48:
                        {
                            if(inst_UartFunc.u8Address == inst_PelcoD_Protocol.u8Address + 1)
                            {}
                            else if(inst_UartFunc.u8Address == inst_PelcoD_Protocol.u8Address)
                            {
                                ps_IR->colorize_count++;

                                if(ps_IR->colorize_count > 10)
                                {
                                    ps_IR->colorize_count = 0;
                                }

                                ps_IR->set_colorize(ps_IR->colorize_count);
                            }

                            break;
                        }

                        default:
                            break;
                    }
                }


                //ptr_DebugFunc->SendMessage(protoPELCO_D,inst_UartFunc.u8Address,0,buf,4);//调用预置位命令转发给远程板
                break;
            }

            case msgPelcoClearPreset:
            {
                if (u8data1 >= 131)
                {
                    Preset_Delete(u8data1);
                }
                else
                {
                    switch(u8data1)
                    {
                        case 55:
                        {
                            static u8 uc_com[12] = {0x55, 0xAA, 0x02, 0x00, 0x04, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0xF0};
                            ps_Usart->Uart3_SendData(uc_com, 12);
                            ps_IR->hot_open_flag = 0;
                            break;
                        }

                        default:
                            break;
                    }
                }


                break;
            }

            case msgPelcoOpenAux:
            {

                break;
            }

            case msgPelcoCloseAux:
            {

                break;
            }

            case msgPelcoSetPanPositin:
            {

                break;
            }

            case msgPelcoSetTiltPosion:
            {
                u8 ucCom[4] = {0x00, 0x4d, 0x00, 0x00};
                u16data = 36000 - u16data;//海康
                ucCom[2] = (u16data >> 8) & 0xff;
                ucCom[3] = u16data & 0xff;
                ptr_PtFunc->SendMessage(protoPELCO_D, inst_UartFunc.u8Address, 0, ucCom, 4);
                break;
            }

            case msgPelcoSetZoomPosion:
            {
                break;
            }

            case msgPelcoQueryPanPositin:
            {
                ps_Pt->databack_com = COMIR;
                ps_Pt->databack_pelcod(P, inst_UartFunc.u8Address);
                break;
            }

            case msgPelcoQueryTiltPosion:
            {
                ps_Pt->databack_com = COMIR;
                ps_Pt->databack_pelcod(T, inst_UartFunc.u8Address);
                break;
            }

            case msgPelcoQueryZoomPosion:
            {
//            ps_Pt->databack_com = COMIR;
//						ps_Pt->databack_pelcod(IR,inst_UartFunc.u8Address);
                break;
            }

            case 0x5d:
            {
                //ps_IR->zoomvalue = u16data;
                break;
            }

            case 0x6e01:
            {
                unsigned char ucUpdateFlag = 1;
                ps_AT24CXX->AT24CXX_Write(UPDATEADDR, &ucUpdateFlag, 1);
                ps_Systick->delay_ms(100);
                NVIC_SystemReset();
                break;
            }

            default :
                break;
        }
    }
}
