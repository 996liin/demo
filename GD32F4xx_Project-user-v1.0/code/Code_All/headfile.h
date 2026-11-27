#ifndef __HEADFILE_H__
#define __HEADFILE_H__

/* ==========================================================================
 * 统一头文件入口
 *
 * 应用层（main.c）只包含本文件；PanelInput 驱动内部仍然只包含自己需要的
 * 头文件，避免驱动层反过来依赖应用层。
 * ========================================================================== */
#include "stdio.h"
#include "gd32f4xx.h"
#include "stdlib.h"
#include "string.h"

#include "systick.h"
#include "usart.h"
#include "fwdgt.h"

#include "at24cxx.h"
#include "iap_flag.h"
#include "iap_app.h"

#include "panel_config.h"
#include "panel_input.h"

#endif /* __HEADFILE_H__ */
