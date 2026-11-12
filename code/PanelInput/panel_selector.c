#include "panel_selector.h"
#include "panel_config.h"

/* ==========================================================================
 * 多档位旋转选择开关驱动实现
 *
 * 注意：这里不使用 A/B 相脉冲计数，只读取当前档位的静态编码。
 * 具体编码方式由 panel_config.c 中的 mode / position_map 决定。
 * ========================================================================== */

static uint8_t panel_selector_read_pattern(const PanelSelectorConfig *cfg)
{
    uint8_t i;
    uint8_t pattern = 0u;

    for (i = 0u; i < cfg->line_count; i++)
    {
        uint8_t level;
        uint8_t active;

        if (PanelPort_PinIsValid(&cfg->pins[i]) == 0u)
        {
            continue;
        }

        level = PanelPort_ReadPin(&cfg->pins[i]);

        if (cfg->active_level == PANEL_ACTIVE_LOW)
        {
            active = (level == 0u) ? 1u : 0u;
        }
        else
        {
            active = (level != 0u) ? 1u : 0u;
        }

        if (active != 0u)
        {
            pattern |= (uint8_t)(1u << i);
        }
    }

    return pattern;
}

static uint8_t panel_selector_count_bits(uint8_t value)
{
    uint8_t count = 0u;

    while (value != 0u)
    {
        count = (uint8_t)(count + (value & 1u));
        value >>= 1;
    }

    return count;
}

static uint8_t panel_selector_gray_to_bin(uint8_t gray)
{
    uint8_t bin = 0u;

    while (gray != 0u)
    {
        bin ^= gray;
        gray >>= 1;
    }

    return bin;
}

static uint8_t panel_selector_decode(const PanelSelectorConfig *cfg,
                                     uint8_t pattern, uint8_t *valid)
{
    uint8_t mask;
    uint8_t position = 0u;

    *valid = 0u;
    mask   = (uint8_t)((1u << cfg->line_count) - 1u);

    /* 优先使用用户自定义译码表 */
    if (cfg->position_map != 0)
    {
        position = cfg->position_map[pattern & mask];

        if (position != 0xFFu)
        {
            *valid = 1u;
        }

        return position;
    }

    switch (cfg->mode)
    {
        case PANEL_SELECTOR_ONEHOT:
        {
            /* 只有一根线有效时，才算一个合法档位 */
            if (panel_selector_count_bits((uint8_t)(pattern & mask)) == 1u)
            {
                uint8_t index = 0u;
                uint8_t value = (uint8_t)(pattern & mask);

                while ((value & 1u) == 0u)
                {
                    value >>= 1;
                    index++;
                }

                position = (uint8_t)(index + cfg->position_offset);
            }
            break;
        }

        case PANEL_SELECTOR_BINARY:
        {
            position = (uint8_t)((pattern & mask) + cfg->position_offset);
            break;
        }

        case PANEL_SELECTOR_GRAY:
        {
            position = (uint8_t)(panel_selector_gray_to_bin((uint8_t)(pattern & mask)) +
                                 cfg->position_offset);
            break;
        }

        default:
            break;
    }

    if ((position != 0u) && (position <= cfg->position_count))
    {
        *valid = 1u;
    }

    return position;
}

void PanelSelector_Init(PanelSelector *sel, const PanelSelectorConfig *cfg, uint8_t id)
{
    uint8_t i;
    uint8_t valid = 0u;

    if ((sel == 0) || (cfg == 0))
    {
        return;
    }

    sel->cfg               = cfg;
    sel->id                = id;
    sel->initialized       = 0u;
    sel->stable_pattern    = 0u;
    sel->candidate_pattern = 0u;
    sel->stable_position   = 0u;
    sel->debounce_cnt      = 0u;

    if ((cfg->line_count == 0u) || (cfg->line_count > PANEL_SELECTOR_MAX_LINES))
    {
        return;
    }

    for (i = 0u; i < cfg->line_count; i++)
    {
        if (PanelPort_PinIsValid(&cfg->pins[i]) == 0u)
        {
            return;
        }

        if (cfg->active_level == PANEL_ACTIVE_LOW)
        {
            PanelPort_InitInput(&cfg->pins[i], PANEL_PULL_UP);
        }
        else
        {
            PanelPort_InitInput(&cfg->pins[i], PANEL_PULL_DOWN);
        }
    }

    sel->stable_pattern    = panel_selector_read_pattern(cfg);
    sel->candidate_pattern = sel->stable_pattern;
    sel->stable_position   = panel_selector_decode(cfg, sel->stable_pattern, &valid);

    if (valid == 0u)
    {
        sel->stable_position = 0u;
    }

    sel->initialized = 1u;
}

void PanelSelector_Scan(PanelSelector *sel, uint32_t now_ms, PanelEventSink sink)
{
    uint8_t pattern;

    if ((sel == 0) || (sel->initialized == 0u))
    {
        return;
    }

    pattern = panel_selector_read_pattern(sel->cfg);

    /* 对整组档位线做消抖，避免旋转经过中间位置时产生抖动事件 */
    if (pattern != sel->candidate_pattern)
    {
        sel->candidate_pattern = pattern;
        sel->debounce_cnt      = 0u;
    }
    else if (sel->debounce_cnt < sel->cfg->debounce_ms)
    {
        sel->debounce_cnt = (uint16_t)(sel->debounce_cnt + PANEL_SCAN_PERIOD_MS);
    }

    if ((sel->debounce_cnt >= sel->cfg->debounce_ms) &&
        (sel->stable_pattern != sel->candidate_pattern))
    {
        uint8_t position;
        uint8_t valid = 0u;

        sel->stable_pattern = sel->candidate_pattern;
        position = panel_selector_decode(sel->cfg, sel->stable_pattern, &valid);

        /* 无效档位（例如旋转到两个档位之间）不产生事件，保持上一次档位 */
        if ((valid != 0u) && (position != sel->stable_position))
        {
            sel->stable_position = position;
            PanelEvent_Post(sink, PANEL_EVT_SELECTOR_CHANGED, sel->id,
                            (int16_t)position, now_ms);
        }
    }
}

uint8_t PanelSelector_GetPosition(const PanelSelector *sel)
{
    if ((sel == 0) || (sel->initialized == 0u))
    {
        return 0u;
    }

    return sel->stable_position;
}
