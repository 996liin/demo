#ifndef __PANEL_TYPES_H__
#define __PANEL_TYPES_H__

#include <stdint.h>

/* ==========================================================================
 * 面板输入公共类型定义
 * 所有驱动模块只依赖本文件，不直接依赖具体业务层。
 * ========================================================================== */

/* 统一事件类型 */
typedef enum
{
    PANEL_EVT_NONE = 0,

    /* 普通按键 */
    PANEL_EVT_KEY_PRESSED,          /* 按下 */
    PANEL_EVT_KEY_RELEASED,         /* 释放，value = 按住时长(ms) */
    PANEL_EVT_KEY_LONG_PRESS,       /* 长按，value = 长按阈值(ms) */
    PANEL_EVT_KEY_SHORT_PRESS,      /* 短按（默认关闭，见 panel_config.h） */

    /* 保持型拨动开关 */
    PANEL_EVT_SWITCH_CHANGED,       /* 状态变化，value = 新状态 */

    /* 多档位旋转选择开关 */
    PANEL_EVT_SELECTOR_CHANGED,     /* 档位变化，value = 新档位 */

    /* 旋转编码器 */
    PANEL_EVT_ENCODER_CW,           /* 顺时针，value = 本次步数 */
    PANEL_EVT_ENCODER_CCW,          /* 逆时针，value = 本次步数 */
    PANEL_EVT_ENCODER_PRESSED,      /* 编码器按下 */
    PANEL_EVT_ENCODER_RELEASED,     /* 编码器释放，value = 按住时长(ms) */
    PANEL_EVT_ENCODER_LONG_PRESS    /* 编码器长按 */
} PanelEventType;

/* 统一事件结构体
 * id     : 同类设备数组中的下标，例如 KEY1 的 id = 0
 * value  : 含义由 type 决定，见上面注释
 * tick_ms: 事件产生时的面板软件时间戳，从 PanelInput_Init 后开始累加
 */
typedef struct
{
    PanelEventType type;
    uint8_t        id;
    int16_t        value;
    uint32_t       tick_ms;
} PanelEvent;

/* 输入有效电平 */
typedef enum
{
    PANEL_ACTIVE_LOW = 0,   /* 低电平有效：外部上拉，开关闭合到 GND */
    PANEL_ACTIVE_HIGH = 1   /* 高电平有效 */
} PanelActiveLevel;

/* 驱动层事件接收回调。驱动不保存事件，直接交给统一事件队列。 */
typedef void (*PanelEventSink)(const PanelEvent *evt);

/* 构造并投递一个事件的小工具，供各驱动复用 */
static inline void PanelEvent_Post(PanelEventSink sink, PanelEventType type,
                                   uint8_t id, int16_t value, uint32_t tick_ms)
{
    PanelEvent evt;

    if (sink == 0)
    {
        return;
    }

    evt.type    = type;
    evt.id      = id;
    evt.value   = value;
    evt.tick_ms = tick_ms;
    sink(&evt);
}

#endif /* __PANEL_TYPES_H__ */
