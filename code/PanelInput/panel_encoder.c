#include "panel_encoder.h"
#include "panel_config.h"

/* ==========================================================================
 * EC11 增量编码器驱动实现
 *
 * A/B 相使用经典四状态正交解码表：
 *   index = (上一次AB << 2) | 本次AB
 * 非法跳变返回 0，机械抖动产生的往返跳变会互相抵消，不会累积成步数。
 * ========================================================================== */

static const int8_t s_quad_table[16] =
{
     0, -1,  1,  0,
     1,  0,  0, -1,
    -1,  0,  0,  1,
     0,  1, -1,  0
};

static uint8_t panel_encoder_read_ab(const PanelEncoderConfig *cfg)
{
    uint8_t a;
    uint8_t b;

    a = PanelPort_ReadPin(&cfg->pin_a);
    b = PanelPort_ReadPin(&cfg->pin_b);

    return (uint8_t)((a << 1) | b);
}

static uint8_t panel_encoder_read_s_pressed(const PanelEncoderConfig *cfg)
{
    uint8_t level;

    if (PanelPort_PinIsValid(&cfg->pin_s) == 0u)
    {
        return 0u;
    }

    level = PanelPort_ReadPin(&cfg->pin_s);

    if (cfg->active_level == PANEL_ACTIVE_LOW)
    {
        return (level == 0u) ? 1u : 0u;
    }

    return (level != 0u) ? 1u : 0u;
}

void PanelEncoder_Init(PanelEncoder *enc, const PanelEncoderConfig *cfg, uint8_t id)
{
    if ((enc == 0) || (cfg == 0))
    {
        return;
    }

    enc->cfg                 = cfg;
    enc->id                  = id;
    enc->initialized         = 0u;
    enc->last_ab             = 0u;
    enc->accum               = 0;
    enc->s_stable_pressed    = 0u;
    enc->s_candidate_pressed = 0u;
    enc->s_debounce_cnt      = 0u;
    enc->s_press_ms          = 0u;
    enc->s_long_sent         = 0u;

    /* 每个机械档位对应的正交边沿数，防止配置为 0 导致除零 */
    enc->steps_per_detent = (cfg->steps_per_detent == 0u) ? 4u : cfg->steps_per_detent;

    if ((PanelPort_PinIsValid(&cfg->pin_a) == 0u) ||
        (PanelPort_PinIsValid(&cfg->pin_b) == 0u))
    {
        return;
    }

    /* A/B 相外部已有 5.11K 上拉，这里再开内部上拉作为冗余 */
    PanelPort_InitInput(&cfg->pin_a, PANEL_PULL_UP);
    PanelPort_InitInput(&cfg->pin_b, PANEL_PULL_UP);

    /* S 相按压信号 */
    if (PanelPort_PinIsValid(&cfg->pin_s) != 0u)
    {
        if (cfg->active_level == PANEL_ACTIVE_LOW)
        {
            PanelPort_InitInput(&cfg->pin_s, PANEL_PULL_UP);
        }
        else
        {
            PanelPort_InitInput(&cfg->pin_s, PANEL_PULL_DOWN);
        }

        enc->s_stable_pressed    = panel_encoder_read_s_pressed(cfg);
        enc->s_candidate_pressed = enc->s_stable_pressed;
    }

    enc->last_ab     = panel_encoder_read_ab(cfg);
    enc->initialized = 1u;
}

void PanelEncoder_Scan(PanelEncoder *enc, uint32_t now_ms, PanelEventSink sink)
{
    uint8_t curr_ab;
    int8_t  edge;
    int16_t steps;

    if ((enc == 0) || (enc->initialized == 0u))
    {
        return;
    }

    /* ---------------- A/B 相旋转解码 ---------------- */
    curr_ab = panel_encoder_read_ab(enc->cfg);
    edge    = s_quad_table[(uint8_t)(((enc->last_ab << 2) | curr_ab) & 0x0Fu)];
    enc->last_ab = curr_ab;

    if (edge != 0)
    {
        enc->accum = (int16_t)(enc->accum + edge);

        steps = (int16_t)(enc->accum / (int16_t)enc->steps_per_detent);

        if (steps != 0)
        {
            /* 保留不足一个档位的余量，避免慢速旋转丢步 */
            enc->accum = (int16_t)(enc->accum -
                                   (steps * (int16_t)enc->steps_per_detent));

#if (PANEL_ENCODER_REVERSE_DIRECTION != 0)
            steps = (int16_t)(-steps);
#endif

            if (steps > 0)
            {
                PanelEvent_Post(sink, PANEL_EVT_ENCODER_CW, enc->id,
                                steps, now_ms);
            }
            else
            {
                PanelEvent_Post(sink, PANEL_EVT_ENCODER_CCW, enc->id,
                                (int16_t)(-steps), now_ms);
            }
        }
    }

    /* ---------------- S 相按压独立处理 ---------------- */
    if (PanelPort_PinIsValid(&enc->cfg->pin_s) != 0u)
    {
        uint8_t pressed;

        pressed = panel_encoder_read_s_pressed(enc->cfg);

        if (pressed != enc->s_candidate_pressed)
        {
            enc->s_candidate_pressed = pressed;
            enc->s_debounce_cnt      = 0u;
        }
        else if (enc->s_debounce_cnt < enc->cfg->debounce_ms)
        {
            enc->s_debounce_cnt = (uint16_t)(enc->s_debounce_cnt +
                                             PANEL_SCAN_PERIOD_MS);
        }

        if ((enc->s_debounce_cnt >= enc->cfg->debounce_ms) &&
            (enc->s_stable_pressed != enc->s_candidate_pressed))
        {
            enc->s_stable_pressed = enc->s_candidate_pressed;

            if (enc->s_stable_pressed != 0u)
            {
                enc->s_press_ms  = now_ms;
                enc->s_long_sent = 0u;
                PanelEvent_Post(sink, PANEL_EVT_ENCODER_PRESSED, enc->id,
                                0, now_ms);
            }
            else
            {
                uint32_t held_ms = now_ms - enc->s_press_ms;

                if (held_ms > 32767u)
                {
                    held_ms = 32767u;
                }

                PanelEvent_Post(sink, PANEL_EVT_ENCODER_RELEASED, enc->id,
                                (int16_t)held_ms, now_ms);
            }
        }

        if ((enc->s_stable_pressed != 0u) &&
            (enc->cfg->long_press_ms > 0u) &&
            (enc->s_long_sent == 0u) &&
            ((now_ms - enc->s_press_ms) >= enc->cfg->long_press_ms))
        {
            enc->s_long_sent = 1u;
            PanelEvent_Post(sink, PANEL_EVT_ENCODER_LONG_PRESS, enc->id,
                            (int16_t)enc->cfg->long_press_ms, now_ms);
        }
    }
}

uint8_t PanelEncoder_GetButtonState(const PanelEncoder *enc)
{
    if ((enc == 0) || (enc->initialized == 0u))
    {
        return 0u;
    }

    return enc->s_stable_pressed;
}
