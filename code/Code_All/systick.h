#ifndef _SYSTICK_H_
#define _SYSTICK_H_


typedef struct
{
	void (*SystickInit)(unsigned short usClock);
	void (*delay_us)(unsigned short us_us);
	void (*delay_ms)(unsigned short us_ms);
	void (*delay_s)(unsigned short us_s);
}Struct_Systick;

extern Struct_Systick *ps_Systick;

#endif


