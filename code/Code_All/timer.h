#ifndef __TIMER_H_
#define __TIMER_H_


typedef struct
{
	void (*TimerInit)(void);
	volatile unsigned char Databack_Flag;
	unsigned char Gnss_Databack_Flag;
	volatile unsigned char Inquiry_CCD_Databack_Flag;
		
}Struct_Timer;
extern uint8 Databack_count;
extern  Struct_Timer *ps_Timer;
#endif


