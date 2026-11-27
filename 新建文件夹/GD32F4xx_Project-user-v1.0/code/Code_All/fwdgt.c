#include "fwdgt.h"
#include "gd32f4xx.h"

/* IRC40K / 256 ≈ 156.25Hz，按 156 计算重装载值 */
#define FWDGT_DIV256_HZ   156u

static void FWDGTInit(unsigned char ucS);
static void FWDGTProcess(void);

static Struct_FWDGT s_FWDGT =
{
    .FWDGTInit    = FWDGTInit,
    .FWDGTProcess = FWDGTProcess,
};
Struct_FWDGT *ps_FWDGT = &s_FWDGT;

/* 形参为约等于的复位时间，单位秒，最大 26s */
static void FWDGTInit(unsigned char ucS)
{
    if (ucS == 0u)
    {
        ucS = 1u;
    }

    if (ucS > 26u)
    {
        ucS = 26u;
    }

    fwdgt_write_enable();
    fwdgt_config((uint16_t)(ucS * FWDGT_DIV256_HZ), FWDGT_PSC_DIV256);
    fwdgt_counter_reload();
    fwdgt_enable();
}

static void FWDGTProcess(void)
{
    fwdgt_counter_reload();
}
