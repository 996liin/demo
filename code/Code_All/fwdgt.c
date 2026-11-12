#include "headfile.h"

static void FWDGTInit(unsigned char ucS);
static void FWDGTProcess(void);

Struct_FWDGT s_FWDGT = 
{
	.FWDGTInit = FWDGTInit,
	.FWDGTProcess = FWDGTProcess,
};
Struct_FWDGT *ps_FWDGT = &s_FWDGT;

//形参为触发复位的时间 单位S,最大26s
static void FWDGTInit(unsigned char ucS)	
{
	fwdgt_write_enable();
	fwdgt_config(ucS*125,FWDGT_PSC_DIV256);
	fwdgt_counter_reload();
	fwdgt_enable();
}

static void FWDGTProcess(void)
{
	fwdgt_counter_reload();
}

