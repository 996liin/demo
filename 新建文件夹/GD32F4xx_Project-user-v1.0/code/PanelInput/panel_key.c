#include "panel_key.h"
#include "panel_config.h"

/* ==========================================================================
 * 普通瞬时按键驱动实现
 * ========================================================================== */

static uint8_t panel_key_read_pressed(const PanelKeyConfig *cfg);

/* 读取按键当前电平，按 active_level 判断是否按下；不做消抖。 */
static uint8_t panel_key_read_pressed(const PanelKeyConfig *cfg)
{
    uint8_t level;

    level = PanelPort_ReadPin(&cfg->pin);

    if (cfg->active_level == PANEL_ACTIVE_LOW)
    {
        return (level == 0u) ? 1u : 0u;
    }

    return (level != 0u) ? 1u : 0u;
}

/* 初始化一路瞬时按键：
 * 配置 GPIO 输入和内部上/下拉，并记录上电时的状态，
 * 避免上电瞬间已经按住时误报一次按下事件。 */
void PanelKey_Init(PanelKey *key, const PanelKeyConfig *cfg, uint8_t id)
{
    if ((key == 0) || (cfg == 0))
    {
        return;
    }

    key->cfg               = cfg;
    key->id                = id;
    key->initialized       = 0u;
    key->stable_pressed    = 0u;
    key->candidate_pressed = 0u;
    key->debounce_cnt      = 0u;
    key->press_ms          = 0u;
    key->long_sent         = 0u;

    if (PanelPort_PinIsValid(&cfg->pin) == 0u)
    {
        return;
    }

    if (cfg->active_level == PANEL_ACTIVE_LOW)
    {
        PanelPort_InitInput(&cfg->pin, PANEL_PULL_UP);
    }
    else
    {
        PanelPort_InitInput(&cfg->pin, PANEL_PULL_DOWN);
    }

    /* 上电时读取一次当前状态，避免把“上电时已经按住”误报成一次按下 */
    key->stable_pressed    = panel_key_read_pressed(cfg);
    key->candidate_pressed = key->stable_pressed;
    key->initialized       = 1u;
}

/* 每 PANEL_SCAN_PERIOD_MS 调用一次，做两级消抖并产生事件：
 *   PRESSED       按下
 *   LONG_PRESS    按住超过 long_press_ms（只报一次）
 *   RELEASED      松开，value = 按住时长(ms)
 *   SHORT_PRESS   松开且短于长按阈值（需 PANEL_KEY_SHORT_PRESS_ENABLE = 1）
 * 同一个按键的按下/松开不会重复上报。 */
void PanelKey_Scan(PanelKey *key, uint32_t now_ms, PanelEventSink sink)
{
    uint8_t pressed;

    if ((key == 0) || (key->initialized == 0u))
    {
        return;
    }

    pressed = panel_key_read_pressed(key->cfg);

    /* ---- 第一步：对原始电平做连续稳定计数 ---- */
    if (pressed != key->candidate_pressed)
    {
        key->candidate_pressed = pressed;
        key->debounce_cnt      = 0u;
    }
    else if (key->debounce_cnt < key->cfg->debounce_ms)
    {
        key->debounce_cnt = (uint16_t)(key->debounce_cnt + PANEL_SCAN_PERIOD_MS);
    }

    /* ---- 第二步：消抖完成且状态确实变化，才产生事件 ---- */
    if ((key->debounce_cnt >= key->cfg->debounce_ms) &&
        (key->stable_pressed != key->candidate_pressed))
    {
        key->stable_pressed = key->candidate_pressed;

        if (key->stable_pressed != 0u)
        {
            /* 按下 */
            key->press_ms  = now_ms;
            key->long_sent = 0u;
            PanelEvent_Post(sink, PANEL_EVT_KEY_PRESSED, key->id, 0, now_ms);
        }
        else
        {
            /* 释放，value 带上按住时长，方便业务层自行判断短按 */
            uint32_t held_ms = now_ms - key->press_ms;

            if (held_ms > 32767u)
            {
                held_ms = 32767u;
            }

#if (PANEL_KEY_SHORT_PRESS_ENABLE != 0)
            if ((key->cfg->long_press_ms > 0u) && (key->long_sent == 0u) &&
                (held_ms < key->cfg->long_press_ms))
            {
                PanelEvent_Post(sink, PANEL_EVT_KEY_SHORT_PRESS, key->id,
                                (int16_t)held_ms, now_ms);
            }
#endif

            PanelEvent_Post(sink, PANEL_EVT_KEY_RELEASED, key->id,
                            (int16_t)held_ms, now_ms);
        }
    }

    /* ---- 第三步：按住期间检测长按，每次按下只报一次 ---- */
    if ((key->stable_pressed != 0u) &&
        (key->cfg->long_press_ms > 0u) &&
        (key->long_sent == 0u) &&
        ((now_ms - key->press_ms) >= key->cfg->long_press_ms))
    {
        key->long_sent = 1u;
        PanelEvent_Post(sink, PANEL_EVT_KEY_LONG_PRESS, key->id,
                        (int16_t)key->cfg->long_press_ms, now_ms);
    }
}

/* 返回当前消抖后的稳定按下状态：1 = 按下，0 = 松开。 */
uint8_t PanelKey_IsPressed(const PanelKey *key)
{
    if ((key == 0) || (key->initialized == 0u))
    {
        return 0u;
    }

    return key->stable_pressed;
}
