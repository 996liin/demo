#include "queue.h"
#include <stdlib.h>
#include <string.h>
static u8 InitCmdQueue(Queue_T *queen);
static Cmd_T *NextCell(Queue_T *queen, unsigned char whitch, unsigned char type);

static Queue_T inst_Uart1Queue =
{
    .InitCmdQueue = InitCmdQueue,
    .NextCell           =   NextCell
};
static Queue_T inst_Uart2Queue =
{
    .InitCmdQueue = InitCmdQueue,
    .NextCell           =   NextCell
};
static Queue_T inst_Uart3Queue =
{
    .InitCmdQueue = InitCmdQueue,
    .NextCell           =   NextCell
};
static Queue_T inst_Uart4Queue =
{
    .InitCmdQueue = InitCmdQueue,
    .NextCell           =   NextCell
};
static Queue_T inst_Uart5Queue =
{
    .InitCmdQueue = InitCmdQueue,
    .NextCell           =   NextCell
};

Queue_T *ptr_Uart1Queue = &inst_Uart1Queue;
Queue_T *ptr_Uart2Queue = &inst_Uart2Queue;
Queue_T *ptr_Uart3Queue = &inst_Uart3Queue;
Queue_T *ptr_Uart4Queue = &inst_Uart4Queue;
Queue_T *ptr_Uart5Queue = &inst_Uart5Queue;

static Cmd_T *NextCell(Queue_T *queen, unsigned char whitch, unsigned char type)
{

    Cmd_T *temp;

    if (RCV_QUEEN == whitch)
    {
        if (type == QUEEN_TOP)
        {
            temp = queen->pCqRecvTop + 1;
        }
        else
        {
            temp = queen->pCqRecvBtm + 1;
        }

        if (temp >= queen->pBase + RCV_QUEUE_SIZE)
        {
            temp = queen->pBase;
        }
    }
    else
    {
        if (type == QUEEN_TOP)
        {
            temp = queen->pCqSendTop + 1;
        }
        else
        {
            temp = queen->pCqSendBtm + 1;
        }

        if (temp >= queen->pBase + RCV_QUEUE_SIZE + SND_QUEUE_SIZE)
        {
            temp = queen->pBase + RCV_QUEUE_SIZE;
        }
    }

    return temp;
}

static u8 InitCmdQueue(Queue_T *queen)
{
    Cmd_T *cqSerialTop;

    cqSerialTop = (Cmd_T *)malloc(sizeof(Cmd_T) * (RCV_QUEUE_SIZE + SND_QUEUE_SIZE));

    if (NULL == cqSerialTop)
    {
        return 1;
    }

    memset(cqSerialTop, 0x00, sizeof(Cmd_T) * (RCV_QUEUE_SIZE + SND_QUEUE_SIZE));

    queen->pBase = cqSerialTop;
    queen->pCqRecvTop = queen->pBase;
    queen->pCqRecvBtm = queen->pBase;

    queen->pCqSendTop = queen->pBase + RCV_QUEUE_SIZE;
    queen->pCqSendBtm = queen->pCqSendTop;

    return 0;
}

