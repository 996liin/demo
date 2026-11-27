#ifndef __PANEL_KEY_H__
#define __PANEL_KEY_H__

#include "panel_types.h"
#include "panel_port.h"

/* ==========================================================================
 * 普通瞬时按键驱动
 *
 * 功能：软件消抖、按下/释放事件、可选长按/短按。
 * 不包含任何业务功能。
 * ========================================================================== */

/* 单个按键的静态配置 */
typedef struct
{
    const char *name;           /* 调试输出用的名字，例如 "KEY1" */
    PanelPortPin pin;           /* GPIO 映射 */
    uint8_t active_level;       /* PANEL_ACTIVE_LOW / PANEL_ACTIVE_HIGH */
    uint16_t debounce_ms;       /* 消抖时间 */
    uint16_t long_press_ms;     /* 长按阈值，0 表示不检测长按 */
} PanelKeyConfig;

/* 单个按键的运行状态 */
typedef struct
{
    const PanelKeyConfig *cfg;
    uint8_t  id;                /* 同类设备下标 */
    uint8_t  initialized;
    uint8_t  stable_pressed;    /* 消抖后的稳定状态 */
    uint8_t  candidate_pressed; /* 正在消抖的候选状态 */
    uint16_t debounce_cnt;      /* 消抖计时(ms) */
    uint32_t press_ms;          /* 按下时刻 */
    uint8_t  long_sent;         /* 本次按下是否已经报过长按 */
} PanelKey;

void PanelKey_Init(PanelKey *key, const PanelKeyConfig *cfg, uint8_t id);
void PanelKey_Scan(PanelKey *key, uint32_t now_ms, PanelEventSink sink);
uint8_t PanelKey_IsPressed(const PanelKey *key);

#endif /* __PANEL_KEY_H__ */
