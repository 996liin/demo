#ifndef _GATE_H_
#define _GATE_H_

typedef struct
{
	void (*GateInit)(void);
	void (*CmdCtrl)(unsigned short cmd, u8 *buf ,u32 len);
}struct_Gate;
extern struct_Gate *ps_Gate;

#endif
