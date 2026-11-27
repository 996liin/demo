#ifndef __FWDGT_H__
#define __FWDGT_H__

/* ==========================================================================
 * 独立看门狗（FWDGT）
 *
 * FWDGTInit(seconds)：初始化并启动，seconds 约为超时秒数（1~26）。
 * FWDGTProcess()：喂狗，必须在超时前周期调用。
 * ========================================================================== */

typedef struct
{
    void (*FWDGTInit)(unsigned char ucS);
    void (*FWDGTProcess)(void);
} Struct_FWDGT;

extern Struct_FWDGT *ps_FWDGT;

#endif /* __FWDGT_H__ */
