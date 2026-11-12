#include "headfile.h"


static void TimerInit(void);

uint8 Databack_count = 0;
uint8 Inquiry_CCD_count = 0;
uint16 gnss_back_count = 0;
uint16 wiper_ctrl_count = 0;
uint8 laser_sync_count = 0;
static Struct_Timer s_Timer =
{
    .TimerInit = TimerInit,
    .Databack_Flag = 0,
    .Inquiry_CCD_Databack_Flag = 0,
    .Gnss_Databack_Flag  = 0,
};
Struct_Timer *ps_Timer = &s_Timer;

/*	timer1	1ms	*/
static void Timer1_Init_1ms(void)
{
    rcu_periph_clock_enable(RCU_TIMER1);

    timer_parameter_struct initpara;
    initpara.clockdivision = TIMER_CKDIV_DIV1;
    initpara.prescaler = 1200 - 1;
    initpara.period = 100 - 1;
    initpara.alignedmode = TIMER_COUNTER_EDGE;
    initpara.counterdirection = TIMER_COUNTER_UP;
    initpara.repetitioncounter = 0;
    timer_init(TIMER1, &initpara);

    timer_enable(TIMER1);
    nvic_irq_enable(TIMER1_IRQn, 0, 0);
    timer_interrupt_enable(TIMER1, TIMER_INT_UP);
}
void TIMER1_IRQHandler(void)
{
    timer_interrupt_flag_clear(TIMER1, TIMER_INT_FLAG_UP);
    //ps_Lens->LensTimer();


    if(ps_Pt->reboot_flag)
    {
        gnss_back_count++;

        if(gnss_back_count >= 30000)
        {
            ps_Pt->reboot_flag = 0;
            gnss_back_count = 0;
            ps_Timer->Gnss_Databack_Flag = 1;
        }
    }

//    wiper_ctrl_count++;
//    laser_sync_count++;
    Inquiry_CCD_count++;
    Databack_count++;

    if(Inquiry_CCD_count >= 200)
    {
        Inquiry_CCD_count = 0;
        ps_Timer->Inquiry_CCD_Databack_Flag = 1;
    }

    if(Databack_count >= 16)
    {
        Databack_count = 0;
        s_Timer.Databack_Flag = 1;
    }

//    if(wiper_ctrl_count >= 100)
//    {
//        wiper_ctrl_count = 0;
//        ptr_Wiper->ctrl_timeout_flag = 1;
//    }

//    if(laser_sync_count >= 100)
//    {
//        laser_sync_count = 0;
//        ps_Laser->AutoCtrlTimeout = 1;
//    }

//    if(s_weapon.EnableFlag == 1)
//    {
//        s_weapon.Protect_time_count--;
//    }

//    if(s_weapon.Protect_time_count <= 0)
//    {
//        s_weapon.EnableFlag = 0;
//        s_weapon.Protect_time_count = PROTECT_TIME;
//    }
}
/*	timer2	500ms	*/
static void Timer2_Init_50us(void)
{
    rcu_periph_clock_enable(RCU_TIMER2);

    timer_parameter_struct initpara;
    initpara.clockdivision = TIMER_CKDIV_DIV1;
    initpara.prescaler = 60 - 1;
    initpara.period = 100 - 1;
    initpara.alignedmode = TIMER_COUNTER_EDGE;
    initpara.counterdirection = TIMER_COUNTER_UP;
    initpara.repetitioncounter = 0;
    timer_init(TIMER2, &initpara);

    timer_enable(TIMER2);
    nvic_irq_enable(TIMER2_IRQn, 0, 0);
    timer_interrupt_enable(TIMER2, TIMER_INT_UP);
}
void TIMER2_IRQHandler(void)
{
    if(RESET != timer_interrupt_flag_get(TIMER2, TIMER_INT_FLAG_UP))
    {
        //ps_Lens->LensTimer();
        timer_interrupt_flag_clear(TIMER2, TIMER_INT_FLAG_UP);
    }
}

static void TimerInit(void)
{
    Timer1_Init_1ms();
    //Timer2_Init_50us();
}
