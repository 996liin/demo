#include "UartFunction.h"
#include "Queue.h"
#include <stdlib.h>
#include "message.h"
#include "delay.h"
#include <string.h>
#include "Timer.h"
#include "RecvCtrl.h"

static void init(u32 buad);
static void SendMessage(Support_ProtocolList proto, u16 u16msg, u8 *buf, u16 buflen);
static void ProtocolProcess(void);

static void RecvCmdProcess(void);
static void SendCmdProcess(void);

static  void ProcessCmd_Pelco_D_P(Support_ProtocolList proto, u8 *buf, u16 len);

/***************************支持的协议***********************************/
static tProtocol inst_PelcoD_Protocol =
{
    .u8FrameStart[0]        = 0xFF,
    .u8RecerverID[0]    = 0x01,
    .u8Reserve              = 0x00,
    .u8FrameEnd             = 0xE6
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

_tUartFunc *ptr_Uart2Func = &inst_UartFunc;

static void init(u32 buad)
{
    inst_UartFunc.uart = ptr_Uart2;
    inst_UartFunc.uart->Init(buad);
    inst_UartFunc.u8pBuf = (u8 *)malloc(sizeof(u8) * RCV_QUEUE_SIZE);
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
                if (NULL != ptr_Uart2Queue->pCqSendTop)
                {
                    if (2 > j)
                    {
                        *(ptr_Uart2Queue->pCqSendTop) = frame_headbuf[j];
                    }
                    else if (6 > j)
                    {
                        *(ptr_Uart2Queue->pCqSendTop) =  *(u8 *)(buf + j - 2);
                    }
                    else
                    {
                        *(ptr_Uart2Queue->pCqSendTop) =  u8DataCheck;
                    }

                    ptr_Uart2Queue->pCqSendTop         = ptr_Uart2Queue->NextCell(ptr_Uart2Queue, SND_QUEEN, QUEEN_TOP);
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
                if (NULL != ptr_Uart2Queue->pCqSendTop)
                {
                    if (2 > j)
                    {
                        *(ptr_Uart2Queue->pCqSendTop) =  frame_headbuf[j];
                    }
                    else if (6 > j)
                    {
                        *(ptr_Uart2Queue->pCqSendTop) =  *(u8 *)(buf + j - 2);
                    }
                    else if (6 == j)
                    {
                        *(ptr_Uart2Queue->pCqSendTop) =  0xAF;
                    }
                    else if (7 == j)
                    {
                        *(ptr_Uart2Queue->pCqSendTop) =  u8DataCheck;
                    }

                    ptr_Uart2Queue->pCqSendTop         = ptr_Uart2Queue->NextCell(ptr_Uart2Queue, SND_QUEEN, QUEEN_TOP);
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
        case protoPELCO_D:
        case protoPELCO_P:
            processSendPelcoDPCmd(proto, inst_PelcoD_Protocol.u8RecerverID[0], buf, buflen);
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
    u32Top = (u32)ptr_Uart2Queue->pCqRecvTop;
    u32Btm = (u32)ptr_Uart2Queue->pCqRecvBtm;

    //如果缓冲区有数据 就进行比较
    if (u32Top != u32Btm)
    {
        //定位到队列尾部
        cqBtm = ptr_Uart2Queue->pCqRecvBtm;

        //取当前的数据  将队列前移一次
        u8frameheadbuf[u8frameheadbuf_cnt] = *(cqBtm);
        u8frameheadbuf_cnt ++;
        cqBtm = ptr_Uart2Queue->NextCell(ptr_Uart2Queue, RCV_QUEEN, QUEEN_BTM);
        ptr_Uart2Queue->pCqRecvBtm = cqBtm;

        if (u8frameheadbuf[0] == inst_PelcoD_Protocol.u8FrameStart[0])
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
    if (ptr_Uart2Queue->pCqRecvBtm != ptr_Uart2Queue->pCqRecvTop)
    {
        *(u8 *)(u8pDataBuf + u16BufLen) = *(ptr_Uart2Queue->pCqRecvBtm);
        //移动节点
        ptr_Uart2Queue->pCqRecvBtm = ptr_Uart2Queue->NextCell(ptr_Uart2Queue, RCV_QUEEN, QUEEN_BTM);
        //计数加1
        u16BufLen++;
    }

    //如果接收到了想要的字节数  则 判断是不是帧尾  如果是 返回1     ；  不是 返回2
    if (u16BufLen == inst_UartFunc.u16BufLen)
    {
        u16BufLen = 0;   //计数清0

        switch (inst_UartFunc.protoType)
        {
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
    if (ptr_Uart2Queue->pCqSendBtm != ptr_Uart2Queue->pCqSendTop)
    {
        while (ptr_Uart2Queue->pCqSendBtm != ptr_Uart2Queue->pCqSendTop)
        {
            inst_UartFunc.uart->SendByte(inst_UartFunc.uart->uart, *(ptr_Uart2Queue->pCqSendBtm));
            ptr_Uart2Queue->pCqSendBtm = ptr_Uart2Queue->NextCell(ptr_Uart2Queue, SND_QUEEN, QUEEN_BTM);
        }
    }
}

static void ProcessCmd_Pelco_D_P(Support_ProtocolList proto, u8 *buf, u16 len)
{
    //命令转发
    ptr_RecvCtrl->sendCmd(proto, msgPrivateNull, buf, len - 1);
}
