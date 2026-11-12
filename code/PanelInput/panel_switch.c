#include "panel_switch.h"
#include "panel_config.h"

/* ==========================================================================
 * 保持型拨动开关驱动实现
 *
 * 消抖对象是“整组输入引脚的组合”，而不是单根引脚，
 * 这样多状态开关在切换过程中不会产生中间态的重复事件。
 * ========================================================================== */

static uint8_t panel_switch_read_pattern(const PanelSwitchConfig *cfg)
{
    uint8_t i;
    uint8_t pattern = 0u;

    for (i = 0u; i < cfg->pin_count; i++)
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

static uint8_t panel_switch_map_state(const PanelSwitchConfig *cfg,
                                      uint8_t pattern, uint8_t *valid)
{
    uint8_t mask;
    uint8_t state;

    mask = (uint8_t)((1u << cfg->pin_count) - 1u);

    if (cfg->state_map != 0)
    {
        state  = cfg->state_map[pattern & mask];
        *valid = (state != 0xFFu) ? 1u : 0u;

        return state;
    }

    *valid = 1u;

    /* 默认把原始组合直接当作逻辑状态：
     * 1 根线 -> 0/1；2 根线 -> 0/1/2/3。
     * 如果实际开关是 ON/OFF/ON 等结构，请用 state_map 明确映射。
     */
    return (uint8_t)(pattern & mask);
}

void PanelSwitch_Init(PanelSwitch *sw, const PanelSwitchConfig *cfg, uint8_t id)
{
    uint8_t i;
    uint8_t valid = 0u;

    if ((sw == 0) || (cfg == 0))
    {
        return;
    }

    sw->cfg               = cfg;
    sw->id                = id;
    sw->initialized       = 0u;
    sw->stable_pattern    = 0u;
    sw->candidate_pattern = 0u;
    sw->stable_state      = 0u;
    sw->debounce_cnt      = 0u;

    if ((cfg->pin_count == 0u) || (cfg->pin_count > PANEL_SWITCH_MAX_PINS))
    {
        return;
    }

    for (i = 0u; i < cfg->pin_count; i++)
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

    sw->stable_pattern    = panel_switch_read_pattern(cfg);
    sw->candidate_pattern = sw->stable_pattern;
    sw->stable_state      = panel_switch_map_state(cfg, sw->stable_pattern, &valid);

    if (valid == 0u)
    {
        sw->stable_state = 0u;
    }

    sw->initialized = 1u;
}

void PanelSwitch_Scan(PanelSwitch *sw, uint32_t now_ms, PanelEventSink sink)
{
    uint8_t pattern;

    if ((sw == 0) || (sw->initialized == 0u))
    {
        return;
    }

    pattern = panel_switch_read_pattern(sw->cfg);

    /* 对整组组合做消抖 */
    if (pattern != sw->candidate_pattern)
    {
        sw->candidate_pattern = pattern;
        sw->debounce_cnt      = 0u;
    }
    else if (sw->debounce_cnt < sw->cfg->debounce_ms)
    {
        sw->debounce_cnt = (uint16_t)(sw->debounce_cnt + PANEL_SCAN_PERIOD_MS);
    }

    if ((sw->debounce_cnt >= sw->cfg->debounce_ms) &&
        (sw->stable_pattern != sw->candidate_pattern))
    {
        uint8_t state;
        uint8_t valid = 0u;

        sw->stable_pattern = sw->candidate_pattern;
        state = panel_switch_map_state(sw->cfg, sw->stable_pattern, &valid);

        /* 无效组合（例如切换途中的中间态）直接忽略，保持上一次稳定状态 */
        if ((valid != 0u) && (state != sw->stable_state))
        {
            sw->stable_state = state;
            PanelEvent_Post(sink, PANEL_EVT_SWITCH_CHANGED, sw->id,
                            (int16_t)state, now_ms);
        }
    }
}

uint8_t PanelSwitch_GetState(const PanelSwitch *sw)
{
    if ((sw == 0) || (sw->initialized == 0u))
    {
        return 0u;
    }

    return sw->stable_state;
}
