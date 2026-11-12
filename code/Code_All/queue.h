#ifndef	__QUEUE_H_
#define	__QUEUE_H_

#define QUEEN_TOP										(0)
#define QUEEN_BTM										(1)

#define RCV_QUEEN										(0)
#define SND_QUEEN										(1)

#define RCV_QUEUE_SIZE              (256)
#define SND_QUEUE_SIZE              (100)
#include "typedef.h"
typedef unsigned char Cmd_T;

typedef struct __Queue_T
{
		Cmd_T *pBase;
	
    Cmd_T	*pCqRecvTop;
    Cmd_T	*pCqRecvBtm;
    Cmd_T	*pCqSendTop;
    Cmd_T	*pCqSendBtm;

    uint8 (*InitCmdQueue)(struct __Queue_T *queen);
		Cmd_T* (*NextCell)(struct __Queue_T *queen, unsigned char whitch, unsigned char type);

} Queue_T;


//´®¿Ú»º³å¶ÓÁÐ
extern Queue_T *ptr_Rk3588Queue;
extern Queue_T *ptr_FtrackQueue;
extern Queue_T *ptr_IR232Queue;
extern Queue_T *ptr_IRAFQueue;
extern Queue_T *ptr_CCD422Queue ;
extern Queue_T *ptr_DebugQueue;
extern Queue_T *ptr_TempCtrlQueue;
extern Queue_T *ptr_PtctrlQueue;


#endif

