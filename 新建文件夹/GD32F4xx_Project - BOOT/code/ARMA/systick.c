#include "headfile.h"

/*
*	滴答定时器 val和load寄存器为24位，因此最大延时时间为(2^24/(180*10^6/8))=745ms
*/
unsigned short Base_ms = 0,Base_us = 0;

static void Systick_Init(unsigned short usClock);
static void delay_us(unsigned short us_us);
static void delay_ms(unsigned short us_ms);
static void delay_s(unsigned short us_s);

static Struct_Systick s_Systick = 
{
	.Systick_Init = Systick_Init,
	.delay_us = delay_us,
	.delay_ms = delay_ms,
	.delay_s = delay_s,
};
Struct_Systick *ps_Systick = &s_Systick;

static void Systick_Init(unsigned short usClock)
{

	SysTick->CTRL &= 0xfffffffb;	//AHB/8
	SysTick->VAL = 0;
	
	Base_us = usClock / 8;
	Base_ms = Base_us * 1000;
}
static void delay_us(unsigned short us_us)
{
	unsigned int uiCTRL_Value= 0;
	
	SysTick->LOAD = us_us * Base_us;
	SysTick->VAL = 0;
	SysTick->CTRL = 1;
	
	do
	{
		uiCTRL_Value = SysTick->CTRL;
	}
	while((uiCTRL_Value & 0x01) && !(uiCTRL_Value >> 16));
	
	SysTick->CTRL = 0;
	SysTick->VAL = 0;
}
//ms延时最大745
static void delay_ms(unsigned short us_ms)
{
	unsigned int uiCTRL_Value= 0;
	
	SysTick->LOAD = us_ms * Base_ms;
	SysTick->VAL = 0;
	SysTick->CTRL = 1;
	
	do
	{
		uiCTRL_Value = SysTick->CTRL;
	}
	while((uiCTRL_Value & 0x01) && !(uiCTRL_Value >> 16));
	
	SysTick->CTRL = 0;
	SysTick->VAL = 0;
}
static void delay_s(unsigned short us_s)
{
	for(unsigned short usCnt = 0; usCnt < us_s*2; usCnt++)
	{
		delay_ms(500);
	}
}
