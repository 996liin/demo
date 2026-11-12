#include "headfile.h"

/*
	GD32的systick功能完全同STM32的，寄存器映射相同；
	库函数给出的为中断版本(SysTick_Config()函数)；
	当前实现为延时版本，不使用中断
	108M主频
*/

static unsigned short Base_ms = 0,Base_us = 0;

static void SystickInit(unsigned short usClock);
static void delay_us(unsigned short us_us);
static void delay_ms(unsigned short us_ms);
static void delay_s(unsigned short us_s);

static Struct_Systick s_Systick = 
{
	.SystickInit = SystickInit,
	.delay_us = delay_us,
	.delay_ms = delay_ms,
	.delay_s = delay_s,
};
Struct_Systick *ps_Systick = &s_Systick;

static void SystickInit(unsigned short usClock)
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
//ms延时最大1118
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
	for(unsigned short usCnt = 0; usCnt < us_s; usCnt++)
	{
		delay_ms(1000);
	}
}
