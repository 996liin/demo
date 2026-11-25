#ifndef __TIMER_H_
#define __TIMER_H_


typedef struct
{
	void (*Tim2_Init_1ms)(void);
		
}Struct_Timer;

extern  Struct_Timer *ps_Timer;

#endif


