#ifndef __PANEL_SWITCH_H__
#define __PANEL_SWITCH_H__

#include "panel_types.h"
#include "panel_port.h"

/* ==========================================================================
 * 保持型拨动开关驱动
 *
 * 与按键的区别：
 * 只在“稳定状态发生变化”时产生一次 PANEL_EVT_SWITCH_CHANGED，
 * 不会在每次扫描时重复产生事件。
 *
 * 支持 1 路引脚（ON/OFF）和 2 路引脚（ON/OFF/ON 等多状态）扩展。
 * ========================================================================== */

#define PANEL_SWITCH_MAX_PINS  2u

typedef struct
{
    const char *name;                           /* 例如 "SWITCH1" */
    PanelPortPin pins[PANEL_SWITCH_MAX_PINS];   /* 开关输入引脚 */
    uint8_t pin_count;                          /* 实际使用几路，1 或 2 */
    uint8_t active_level;                       /* PANEL_ACTIVE_LOW / HIGH */
    uint16_t debounce_ms;                       /* 整组状态消抖时间 */
    uint8_t state_count;                        /* 有效状态数量，ON/OFF 填 2 */
    const uint8_t *state_map;                   /* 可选映射表，长度 1<<pin_count；
                                                 * 值为 0xFF 表示该原始组合无效 */
} PanelSwitchConfig;

typedef struct
{
    const PanelSwitchConfig *cfg;
    uint8_t  id;
    uint8_t  initialized;
    uint8_t  stable_pattern;    /* 消抖后的原始输入组合 */
    uint8_t  candidate_pattern; /* 候选组合 */
    uint8_t  stable_state;      /* 映射后的逻辑状态 */
    uint16_t debounce_cnt;
} PanelSwitch;

void PanelSwitch_Init(PanelSwitch *sw, const PanelSwitchConfig *cfg, uint8_t id);
void PanelSwitch_Scan(PanelSwitch *sw, uint32_t now_ms, PanelEventSink sink);
uint8_t PanelSwitch_GetState(const PanelSwitch *sw);

#endif /* __PANEL_SWITCH_H__ */
