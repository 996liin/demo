#ifndef __PANEL_ENCODER_H__
#define __PANEL_ENCODER_H__

#include "panel_types.h"
#include "panel_port.h"

/* ==========================================================================
 * 无限旋转增量编码器驱动（EC11E09244AQ）
 *
 * A/B 两相正交解码，识别顺时针/逆时针以及步数；
 * S 相按压信号单独做消抖，产生按下/释放/长按事件。
 *
 * 旋转和按压完全独立处理，避免互相干扰。
 * ========================================================================== */

typedef struct
{
    const char *name;           /* 例如 "ENCODER1" */
    PanelPortPin pin_a;         /* A 相 */
    PanelPortPin pin_b;         /* B 相 */
    PanelPortPin pin_s;         /* S 按压信号，未使用时填 PANEL_PIN_NONE */
    uint8_t active_level;       /* 仅用于 S 相的有效电平 */
    uint16_t debounce_ms;       /* S 相消抖时间 */
    uint16_t long_press_ms;     /* S 相长按阈值，0 表示不检测 */
    uint8_t steps_per_detent;   /* 每个机械档位对应的正交边沿数，常为 4 */
} PanelEncoderConfig;

typedef struct
{
    const PanelEncoderConfig *cfg;
    uint8_t  id;
    uint8_t  initialized;
    uint8_t  steps_per_detent;

    /* A/B 相解码状态 */
    uint8_t  last_ab;
    int16_t  accum;

    /* S 相按键状态 */
    uint8_t  s_stable_pressed;
    uint8_t  s_candidate_pressed;
    uint16_t s_debounce_cnt;
    uint32_t s_press_ms;
    uint8_t  s_long_sent;
} PanelEncoder;

void PanelEncoder_Init(PanelEncoder *enc, const PanelEncoderConfig *cfg, uint8_t id);
void PanelEncoder_Scan(PanelEncoder *enc, uint32_t now_ms, PanelEventSink sink);
uint8_t PanelEncoder_GetButtonState(const PanelEncoder *enc);

#endif /* __PANEL_ENCODER_H__ */
