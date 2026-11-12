#ifndef QUEUE_H_
#define QUEUE_H_

#include "stm32f10x.h"

#define QUEEN_TOP										(0)
#define QUEEN_BTM										(1)

#define RCV_QUEEN										(0)
#define SND_QUEEN										(1)

#define RCV_QUEUE_SIZE              (512)
#define SND_QUEUE_SIZE              (256)

typedef unsigned char Cmd_T;

typedef struct __Queue_T
{
		Cmd_T *pBase;
	
    Cmd_T	*pCqRecvTop;
    Cmd_T	*pCqRecvBtm;
    Cmd_T	*pCqSendTop;
    Cmd_T	*pCqSendBtm;

    u8 (*InitCmdQueue)(struct __Queue_T *queen);
		Cmd_T* (*NextCell)(struct __Queue_T *queen, unsigned char whitch, unsigned char type);

} Queue_T;


//´®¿Ú»º³å¶ÓÁÐ
extern Queue_T *ptr_Uart1Queue;
extern Queue_T *ptr_Uart2Queue;
extern Queue_T *ptr_Uart3Queue;
extern Queue_T *ptr_Uart4Queue;
extern Queue_T *ptr_Uart5Queue;

#endif
