#ifndef _FSM_H_
#define _FSM_H_

typedef struct
{
	void (*FsmProcess)(void);
	void (*SetPostion)(short nHSec, short nVSec);
	void (*SetPosRelat)(short nHSec, short nVSec);
	void (*StopPostion)(void);
	void (*SelfCheck)(void);
	unsigned char AutoCtrl_Flag;
	unsigned char ManualCtrl_Flag;
	short SpdH;
	short SpdV;
	int   offset_h;//跟踪模式下偏心跟踪偏移量
  int   offset_v;
}struct_Fsm;
extern struct_Fsm *ps_Fsm;

#endif   
