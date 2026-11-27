#include "headfile.h"

/*
	TIM2-TIM7、TIM12-TIM14	:	CLK=90M
	TIM1、TIM8-TIM11		:	CLK=180M
*/
static void Tim2_Init_1ms(void);

static Struct_Timer s_Timer = 
{
	.Tim2_Init_1ms = Tim2_Init_1ms,
};
Struct_Timer *ps_Timer = &s_Timer;

/*	Tim2	1ms	*/
static void Tim2_Init_1ms(void)
{
	rcu_periph_clock_enable(RCU_TIMER2);
		
	timer_parameter_struct initpara;
	initpara.clockdivision = TIMER_CKDIV_DIV1;
	initpara.period = 100-1;
	initpara.prescaler = 1200-1;
	initpara.alignedmode = TIMER_COUNTER_EDGE;//这个模式是什么意思
	initpara.counterdirection = TIMER_COUNTER_UP;
	initpara.repetitioncounter = 0;
	timer_init(TIMER2,&initpara);
	
	timer_enable(TIMER2);
	nvic_irq_enable(TIMER2_IRQn,0,0);
	timer_interrupt_enable(TIMER2,TIMER_INT_UP);
}
void TIMER2_IRQHandler(void)
{
	if(RESET != timer_interrupt_flag_get(TIMER2,TIMER_INT_FLAG_UP))
	{
		timer_interrupt_flag_clear(TIMER2,TIMER_INT_FLAG_UP);
		
		ps_Flash->FlashTimer();
		ps_IAP->IAPTimer();
	}
}
