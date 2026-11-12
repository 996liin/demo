#include "headfile.h"
unsigned char VERSION_STRING[]  = "Version: Soft-HSHV3.2.7  HSH_LW_IMAGEP_V2.0\r\ncompiled on " __DATE__ " at " __TIME__;
/*
*	当前程序将APB2做了二分频处理，后续配置要注意(例如TIM0、TIM7等)
*/
static void updateParaInit(void);
unsigned char first_power_on = 0;
unsigned char sys_init_ok = 0;
unsigned char debug_ccd_sf_param = 0;

int main(void)
{
    nvic_vector_table_set(NVIC_VECTTAB_FLASH, 0x8000);
    nvic_priority_group_set(NVIC_PRIGROUP_PRE0_SUB4);
    ps_Systick->SystickInit(240);

    ps_Usart->UartInit();
    ps_Timer->TimerInit();

    //ps_CAN1->CANInit();
    //ps_ADC->ADCint();
    //ps_HDC1080->HDC1080Init();
    ps_AT24CXX->AT24CXXInit();
    updateParaInit();
    ps_IR->init();
    ps_CCD->init();
    //ps_Lrf->init();
    //ps_qcl->Init();
    //ps_Lens->init();
    //ptr_3588Func->init();
    ps_Pt->init();
    //ps_Laser->init();
    //ptr_ftrackFunc->init();
    ptr_Ccd422Func->init();
    ptr_Ir232Func->init();
    ptr_IrAfFunc->init();
    ptr_TempCtrlFunc->init();
    ptr_DebugFunc->init();
    ps_FWDGT->FWDGTInit(5);


}

static void updateParaInit(void)
{
    ps_AT24CXX->AT24CXX_Read(UPDATEADDR, (unsigned char *)ps_devicePara, sizeof(struct_devicePara));

    if(ps_devicePara->ucUpdateFlag)
    {
        ps_devicePara->ucUpdateFlag = 0;
        ps_AT24CXX->AT24CXX_Write(UPDATEADDR, (unsigned char *)ps_devicePara, sizeof(struct_devicePara));
    }
}
