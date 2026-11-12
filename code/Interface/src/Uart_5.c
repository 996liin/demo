#include "UartFunction.h"
#include "Queue.h"
#include <stdlib.h>
#include "message.h"
#include "delay.h"
#include <string.h>
#include "SysParam.h"
#include "Timer.h"

static void init(u32 buad);
static void SendMessage(Support_ProtocolList proto, u16 u16msg, u8 *buf, u16 buflen);
static void ProtocolProcess(void);

static void RecvCmdProcess(void);
static void SendCmdProcess(void);

static inline void ProcessCmd_private(u8 *buf, u16 len);
static inline void ProcessCmd_Pelco_D_P(Support_ProtocolList proto, u8 *buf, u16 len);

/***************************支持的协议***********************************/
static tProtocol inst_PrivateProtocol =
{
    .u8FrameStart[0]        = 0x8A,
    .u8RecerverID[0]    = 0x01,
    .u8Reserve              = 0x00,
    .u8FrameEnd             = 0xA8
};
static tProtocol inst_PelcoD_Protocol =
{
    .u8FrameStart[0]        = 0xFF,
    .u8RecerverID[0]    = 0x01,
    .u8Reserve              = 0x00,
    .u8FrameEnd             = 0x00
};

static tProtocol inst_PelcoP_Protocol =
{
    .u8FrameStart[0]        = 0xA0,
    .u8RecerverID[0]    = 0x01,
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

_tUartFunc *ptr_Uart5Func = &inst_UartFunc;

static u8 pelco_stop_signal = 1;

static void init(u32 buad)
{
    inst_UartFunc.uart = ptr_Uart5;
    inst_UartFunc.uart->Init(buad);
    inst_UartFunc.u8pBuf = (u8 *)malloc(sizeof(u8) * RCV_QUEUE_SIZE);
}

static void processSendPrivateCmd(u16 u16msg, u8 *buf, u16 buflen)
{
    u16 u16cmdMsg = msgPrivateNull;
    u8 j = 0;

    u16 frame_headbuf[6] = {0x00};

    u16cmdMsg = u16msg;

    //6字节头部
    frame_headbuf[0] = inst_PrivateProtocol.u8FrameStart[0];
    frame_headbuf[1] = inst_PrivateProtocol.u8RecerverID[0];
    frame_headbuf[2] = (u16cmdMsg >> 8) & 0x00FF;
    frame_headbuf[3] = u16cmdMsg & 0x00FF;
    frame_headbuf[4] = (buflen >> 8) & 0x00FF;;
    frame_headbuf[5] = buflen & 0x00FF;

    for (j = 0; j < buflen + 8; j++)
    {
        if (NULL != ptr_Uart5Queue->pCqSendTop)
        {
            if (6 > j)
            {
                *(ptr_Uart5Queue->pCqSendTop) = frame_headbuf[j];
            }
            else if ((6 + buflen) > j)
            {
                *(ptr_Uart5Queue->pCqSendTop) = *(u8 *)(buf + j - 6);
            }
            else if ((buflen + 8 - 2) == j)
            {
                *(ptr_Uart5Queue->pCqSendTop) = inst_PrivateProtocol.u8Reserve;
            }
            else if ((buflen + 8 - 1) == j)
            {
                *(ptr_Uart5Queue->pCqSendTop) = inst_PrivateProtocol.u8FrameEnd;
            }

            ptr_Uart5Queue->pCqSendTop         = ptr_Uart5Queue->NextCell(ptr_Uart5Queue, SND_QUEEN, QUEEN_TOP);
        }
    }
}

static void processSendInternalCmd(u8 *buf, u16 buflen)
{
    u16 i = 0;

    for (i = 0; i < buflen; i++)
    {
        if (NULL != ptr_Uart5Queue->pCqSendTop)
        {
            *(ptr_Uart5Queue->pCqSendTop) =  *(u8 *)(buf + i);
            ptr_Uart5Queue->pCqSendTop         = ptr_Uart5Queue->NextCell(ptr_Uart5Queue, SND_QUEEN, QUEEN_TOP);
        }
    }
}

static void processSendPelcoDPCmd(Support_ProtocolList proto, u8 address, u8 *buf, u16 buflen)
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
                if (NULL != ptr_Uart5Queue->pCqSendTop)
                {
                    if (2 > j)
                    {
                        *(ptr_Uart5Queue->pCqSendTop) = frame_headbuf[j];
                    }
                    else if (6 > j)
                    {
                        *(ptr_Uart5Queue->pCqSendTop) =  *(u8 *)(buf + j - 2);
                    }
                    else
                    {
                        *(ptr_Uart5Queue->pCqSendTop) =  u8DataCheck;
                    }

                    ptr_Uart5Queue->pCqSendTop         = ptr_Uart5Queue->NextCell(ptr_Uart5Queue, SND_QUEEN, QUEEN_TOP);
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
                if (NULL != ptr_Uart5Queue->pCqSendTop)
                {
                    if (2 > j)
                    {
                        *(ptr_Uart5Queue->pCqSendTop) =  frame_headbuf[j];
                    }
                    else if (6 > j)
                    {
                        *(ptr_Uart5Queue->pCqSendTop) =  *(u8 *)(buf + j - 2);
                    }
                    else if (6 == j)
                    {
                        *(ptr_Uart5Queue->pCqSendTop) =  0xAF;
                    }
                    else if (7 == j)
                    {
                        *(ptr_Uart5Queue->pCqSendTop) =  u8DataCheck;
                    }

                    ptr_Uart5Queue->pCqSendTop         = ptr_Uart5Queue->NextCell(ptr_Uart5Queue, SND_QUEEN, QUEEN_TOP);
                }
            }

            break;
        }

        default:
            break;
    }
}

static void SendMessage(Support_ProtocolList proto, u16 u16msg, u8 *buf, u16 buflen)
{
    switch (proto)
    {
        case proto_internal:
            processSendInternalCmd(buf, buflen);
            break;

        case protoPELCO_D:
        case protoPELCO_P:
            processSendPelcoDPCmd(proto, inst_PelcoD_Protocol.u8RecerverID[0], buf, buflen);
            break;

        case proto_private:
            processSendPrivateCmd(u16msg, buf, buflen);
            break;

        default:
            break;
    }

    SendCmdProcess();  //发送命令处理
}


static void ProtocolProcess(void)
{
    RecvCmdProcess();  //接收命令处理
    //SendCmdProcess();  //发送命令处理
}

//搜索协议头
#define SEARCHFRAMEHEAD_LEN 20
static inline u8 SearchFrameHead(void)
{
    Cmd_T *cqBtm;

    u32 u32Top = 0;
    u32 u32Btm = 0;

    //用来取队列中的数据  进行比较
    static u8 u8frameheadbuf[SEARCHFRAMEHEAD_LEN];
    static u8 u8frameheadbuf_cnt = 0;

    //取缓冲队列的头尾
    u32Top = (u32)ptr_Uart5Queue->pCqRecvTop;
    u32Btm = (u32)ptr_Uart5Queue->pCqRecvBtm;

    //如果缓冲区有数据 就进行比较
    if (u32Top != u32Btm)
    {
        //定位到队列尾部
        cqBtm = ptr_Uart5Queue->pCqRecvBtm;

        //取当前的数据  将队列前移一次
        u8frameheadbuf[u8frameheadbuf_cnt] = *(cqBtm);
        u8frameheadbuf_cnt ++;
        cqBtm = ptr_Uart5Queue->NextCell(ptr_Uart5Queue, RCV_QUEEN, QUEEN_BTM);
        ptr_Uart5Queue->pCqRecvBtm = cqBtm;

        if (u8frameheadbuf[0] == inst_PrivateProtocol.u8FrameStart[0])
        {
            if (2 <= u8frameheadbuf_cnt)
            {
                if (u8frameheadbuf[1] == inst_PrivateProtocol.u8RecerverID[0])
                {
                    //当接收到6个字节之后 表示帧头读取完成，返回读帧头成功
                    if (6 == u8frameheadbuf_cnt)
                    {
                        inst_UartFunc.protoType = proto_private;

                        inst_UartFunc.u16cmdType = 0;
                        inst_UartFunc.u16cmdType = u8frameheadbuf[2];
                        inst_UartFunc.u16cmdType <<= 8;
                        inst_UartFunc.u16cmdType |= u8frameheadbuf[3];

                        inst_UartFunc.u16BufLen = 0;
                        inst_UartFunc.u16BufLen = u8frameheadbuf[4];
                        inst_UartFunc.u16BufLen <<= 8;
                        inst_UartFunc.u16BufLen |= u8frameheadbuf[5];

                        //数据 + 预留位 + 帧尾
                        inst_UartFunc.u16BufLen = inst_UartFunc.u16BufLen + 2;

                        //如果帧头 和 数据 都对 则清空接收缓冲区 并 返回 找到帧头标致
                        memset(u8frameheadbuf, 0x00, u8frameheadbuf_cnt);
                        u8frameheadbuf_cnt = 0;
                        return 1;
                    }
                }
                else
                {
                    //如果第二个字节不是地址 则 丢弃 重新进行判断
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
                if (u8frameheadbuf[1] == inst_PelcoD_Protocol.u8RecerverID[0])
                {
                    inst_UartFunc.protoType = protoPELCO_D;
                    inst_UartFunc.u8Address = u8frameheadbuf[1];
                    inst_UartFunc.u16BufLen = 5;

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
                if (u8frameheadbuf[1] == inst_PelcoP_Protocol.u8RecerverID[0])
                {
                    inst_UartFunc.protoType = protoPELCO_P;
                    inst_UartFunc.u8Address = u8frameheadbuf[1];
                    inst_UartFunc.u16BufLen = 6;

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
            //接收到为错误数据
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

    static u16 u16BufLen = 0;
    //定义数据指针 指向数据缓冲区
    u8 *u8pDataBuf = inst_UartFunc.u8pBuf;

    //有数据就接收 放到数据缓冲区中
    if (ptr_Uart5Queue->pCqRecvBtm != ptr_Uart5Queue->pCqRecvTop)
    {
        *(u8 *)(u8pDataBuf + u16BufLen) = *(ptr_Uart5Queue->pCqRecvBtm);
        //移动节点
        ptr_Uart5Queue->pCqRecvBtm = ptr_Uart5Queue->NextCell(ptr_Uart5Queue, RCV_QUEEN, QUEEN_BTM);
        //计数加1
        u16BufLen++;
    }

    //如果接收到了想要的字节数  则 判断是不是帧尾  如果是 返回1     ；  不是 返回2
    if (u16BufLen == inst_UartFunc.u16BufLen)
    {
        u16BufLen = 0;   //计数清0

        switch (inst_UartFunc.protoType)
        {
            case proto_private:
            {
                if ((inst_PrivateProtocol.u8Reserve  == u8pDataBuf[inst_UartFunc.u16BufLen - 2]) && (inst_PrivateProtocol.u8FrameEnd == u8pDataBuf[inst_UartFunc.u16BufLen - 1]))
                {
                    rtn_data = 1;   //检测正常 返回1
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

                for (i = 0; i < inst_UartFunc.u16BufLen - 1; i++)
                {
                    u8DataCheck += *(u8 *)(u8pDataBuf + i);
                }

                if (u8DataCheck == u8pDataBuf[inst_UartFunc.u16BufLen - 1])
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

                for (i = 0; i < inst_UartFunc.u16BufLen - 2; i++)
                {
                    u8DataCheck ^= *(u8 *)(u8pDataBuf + i);
                }

                if ((inst_PelcoP_Protocol.u8Reserve == u8pDataBuf[inst_UartFunc.u16BufLen - 2]) && (u8DataCheck == u8pDataBuf[inst_UartFunc.u16BufLen - 1]))
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

        if (1 == u8Rtn) //搜索到帧头
        {
            u8Status = 1;
        }
        else if (0 == u8Rtn) //没有找到帧头 继续搜帧头
        {
            u8Status = 0;
        }
    }

    if (1 == u8Status)
    {
        //返回0 表示还未搜到一个完整的数据帧 继续搜索
        u8Rtn = SearchFrameEnd();

        if (1 == u8Rtn) //搜到帧尾
        {
            u8Status = 2;
        }
        else if (2 == u8Rtn) //错误帧  重新进行搜索
        {
            u8Status = 0;
        }
    }

    //对协议进行分发处理
    if (2 == u8Status)
    {
        switch (inst_UartFunc.protoType)
        {
            case proto_private:
                ProcessCmd_private(inst_UartFunc.u8pBuf, inst_UartFunc.u16BufLen);
                break;

            case protoPELCO_D:
            case protoPELCO_P:
                ProcessCmd_Pelco_D_P(inst_UartFunc.protoType, inst_UartFunc.u8pBuf, inst_UartFunc.u16BufLen);
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
    //如果缓冲区中有数据 则一次发完
    if (ptr_Uart5Queue->pCqSendBtm != ptr_Uart5Queue->pCqSendTop)
    {
        inst_UartFunc.uart->ChangeDir(UART_DIR_SEND);

        while (ptr_Uart5Queue->pCqSendBtm != ptr_Uart5Queue->pCqSendTop)
        {
            inst_UartFunc.uart->SendByte(inst_UartFunc.uart->uart, *(ptr_Uart5Queue->pCqSendBtm));
            ptr_Uart5Queue->pCqSendBtm = ptr_Uart5Queue->NextCell(ptr_Uart5Queue, SND_QUEEN, QUEEN_BTM);
        }

        inst_UartFunc.uart->ChangeDir(UART_DIR_RECIVE);
    }

}

static inline void ProcessCmd_private(u8 *buf, u16 len)
{
    switch (inst_UartFunc.u16cmdType)
    {

        default :
            break;
    }
}

static void ProcessCmd_Pelco_D_P(Support_ProtocolList proto, u8 *buf, u16 len)
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
        //如果最低位为0  PTZ控制
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
                break;
            }

            case msgPelcoTele:
            {
                break;
            }

            default:
                break;
        }//end swtich


        //有水平方向指令
        if ((u16cmd & 0x0006) != 0)
        {
            if ((u16cmd & 0x0002) != 0) //right move
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

        //俯仰方向指令
        if ((u16cmd & 0x0018) != 0)
        {
            if ((u16cmd & 0x0008) != 0) //up
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
            pelco_stop_signal = 1;
        }
    }
    else
    {
        u8PresetNo = u8data1;

        //预置位  辅助开关 等命令
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
                break;
            }

            case msgPelcoSetZoomPosion:
            {
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
                break;
            }

            default :
                break;
        }
    }
}
