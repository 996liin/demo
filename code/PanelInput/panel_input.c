#include "panel_input.h"
#include "panel_config.h"
#include "panel_key.h"
#include "panel_switch.h"
#include "panel_selector.h"
#include "panel_encoder.h"
#include "panel_port.h"

#include <stdio.h>

/* ==========================================================================
 * 面板输入统一管理实现
 *
 * 扫描与分发都在主循环中完成，不使用中断，也不在驱动内部做阻塞延时。
 * ========================================================================== */

static PanelKey      s_keys[PANEL_KEY_MAX];
static PanelSwitch   s_switches[PANEL_SWITCH_MAX];
static PanelSelector s_selectors[PANEL_SELECTOR_MAX];
static PanelEncoder  s_encoders[PANEL_ENCODER_MAX];

static uint8_t s_key_count;
static uint8_t s_switch_count;
static uint8_t s_selector_count;
static uint8_t s_encoder_count;

static PanelEvent s_queue[PANEL_EVENT_QUEUE_SIZE];
static uint16_t   s_q_head;
static uint16_t   s_q_tail;
static uint16_t   s_q_count;

static uint32_t          s_now_ms;
static PanelEventHandler s_handler;
static uint8_t           s_initialized;
static uint32_t          s_dropped;

/* --------------------------------------------------------------------------
 * 事件队列
 * -------------------------------------------------------------------------- */
static void panel_event_push(const PanelEvent *evt)
{
    if (s_q_count >= PANEL_EVENT_QUEUE_SIZE)
    {
        /* 队列满时丢弃新事件并计数，避免覆盖尚未处理的事件 */
        s_dropped++;
        return;
    }

    s_queue[s_q_tail] = *evt;
    s_q_tail = (uint16_t)((s_q_tail + 1u) % PANEL_EVENT_QUEUE_SIZE);
    s_q_count++;
}

static uint8_t panel_event_pop(PanelEvent *evt)
{
    if ((evt == 0) || (s_q_count == 0u))
    {
        return 0u;
    }

    *evt = s_queue[s_q_head];
    s_q_head = (uint16_t)((s_q_head + 1u) % PANEL_EVENT_QUEUE_SIZE);
    s_q_count--;

    return 1u;
}

/* --------------------------------------------------------------------------
 * 调试输出
 * -------------------------------------------------------------------------- */
static const char *panel_name_for(const PanelEvent *evt)
{
    switch (evt->type)
    {
        case PANEL_EVT_KEY_PRESSED:
        case PANEL_EVT_KEY_RELEASED:
        case PANEL_EVT_KEY_LONG_PRESS:
        case PANEL_EVT_KEY_SHORT_PRESS:
            if ((g_panel_keys != 0) && (evt->id < g_panel_key_count) &&
                (g_panel_keys[evt->id].name != 0))
            {
                return g_panel_keys[evt->id].name;
            }
            return "KEY";

        case PANEL_EVT_SWITCH_CHANGED:
            if ((g_panel_switches != 0) && (evt->id < g_panel_switch_count) &&
                (g_panel_switches[evt->id].name != 0))
            {
                return g_panel_switches[evt->id].name;
            }
            return "SWITCH";

        case PANEL_EVT_SELECTOR_CHANGED:
            if ((g_panel_selectors != 0) && (evt->id < g_panel_selector_count) &&
                (g_panel_selectors[evt->id].name != 0))
            {
                return g_panel_selectors[evt->id].name;
            }
            return "SELECTOR";

        case PANEL_EVT_ENCODER_CW:
        case PANEL_EVT_ENCODER_CCW:
        case PANEL_EVT_ENCODER_PRESSED:
        case PANEL_EVT_ENCODER_RELEASED:
        case PANEL_EVT_ENCODER_LONG_PRESS:
            if ((g_panel_encoders != 0) && (evt->id < g_panel_encoder_count) &&
                (g_panel_encoders[evt->id].name != 0))
            {
                return g_panel_encoders[evt->id].name;
            }
            return "ENCODER";

        default:
            break;
    }

    return "PANEL";
}

/* 档位旋钮的显示名，例如 ZOOM 的 1X/2X/4X/8X。
 * 没有配置 position_labels 时返回 0，调试输出改用数字档位。
 */
static const char *panel_selector_label(uint8_t id, int16_t position)
{
    const PanelSelectorConfig *cfg;
    int index;

    if ((g_panel_selectors == 0) || (id >= g_panel_selector_count))
    {
        return 0;
    }

    cfg = &g_panel_selectors[id];

    if ((cfg->position_labels == 0) ||
        (position < 1) ||
        (position > (int16_t)cfg->position_count))
    {
        return 0;
    }

    index = (int)position - 1;

    return cfg->position_labels[index];
}

static void panel_debug_print(const PanelEvent *evt)
{
#if (PANEL_DEBUG_ENABLE != 0)
    char        buf[96];
    const char *name;

    name = panel_name_for(evt);

    switch (evt->type)
    {
        case PANEL_EVT_KEY_PRESSED:
            sprintf(buf, "%s PRESSED\r\n", name);
            break;

        case PANEL_EVT_KEY_RELEASED:
            sprintf(buf, "%s RELEASED\r\n", name);
            break;

        case PANEL_EVT_KEY_LONG_PRESS:
            sprintf(buf, "%s LONG PRESS\r\n", name);
            break;

        case PANEL_EVT_KEY_SHORT_PRESS:
            sprintf(buf, "%s SHORT PRESS\r\n", name);
            break;

        case PANEL_EVT_SWITCH_CHANGED:
            if (evt->value == 0)
            {
                sprintf(buf, "%s OFF\r\n", name);
            }
            else if (evt->value == 1)
            {
                sprintf(buf, "%s ON\r\n", name);
            }
            else
            {
                sprintf(buf, "%s STATE %d\r\n", name, (int)evt->value);
            }
            break;

        case PANEL_EVT_SELECTOR_CHANGED:
        {
            const char *label = panel_selector_label(evt->id, evt->value);

            if (label != 0)
            {
                sprintf(buf, "%s POSITION: %s\r\n", name, label);
            }
            else
            {
                sprintf(buf, "%s POSITION: %d\r\n", name, (int)evt->value);
            }
            break;
        }

        case PANEL_EVT_ENCODER_CW:
            if (evt->value > 1)
            {
                sprintf(buf, "%s CLOCKWISE x%d\r\n", name, (int)evt->value);
            }
            else
            {
                sprintf(buf, "%s CLOCKWISE\r\n", name);
            }
            break;

        case PANEL_EVT_ENCODER_CCW:
            if (evt->value > 1)
            {
                sprintf(buf, "%s COUNTERCLOCKWISE x%d\r\n", name, (int)evt->value);
            }
            else
            {
                sprintf(buf, "%s COUNTERCLOCKWISE\r\n", name);
            }
            break;

        case PANEL_EVT_ENCODER_PRESSED:
            sprintf(buf, "%s PRESSED\r\n", name);
            break;

        case PANEL_EVT_ENCODER_RELEASED:
            sprintf(buf, "%s RELEASED\r\n", name);
            break;

        case PANEL_EVT_ENCODER_LONG_PRESS:
            sprintf(buf, "%s LONG PRESS\r\n", name);
            break;

        default:
            return;
    }

    PanelPort_DebugWrite(buf);
#else
    (void)evt;
#endif
}

static void panel_print_banner(void)
{
#if (PANEL_DEBUG_ENABLE != 0)
    char    buf[128];
    uint8_t i;
    uint8_t active_keys      = 0u;
    uint8_t active_switches  = 0u;
    uint8_t active_selectors = 0u;
    uint8_t active_encoders  = 0u;

    for (i = 0u; i < s_key_count; i++)
    {
        if (s_keys[i].initialized != 0u)
        {
            active_keys++;
        }
    }

    for (i = 0u; i < s_switch_count; i++)
    {
        if (s_switches[i].initialized != 0u)
        {
            active_switches++;
        }
    }

    for (i = 0u; i < s_selector_count; i++)
    {
        if (s_selectors[i].initialized != 0u)
        {
            active_selectors++;
        }
    }

    for (i = 0u; i < s_encoder_count; i++)
    {
        if (s_encoders[i].initialized != 0u)
        {
            active_encoders++;
        }
    }

#if (PANEL_PIN_MAP_READY != 0)
    PanelPort_DebugWrite("\r\n=== PANEL INPUT READY ===\r\n");
#else
    PanelPort_DebugWrite("\r\n=== PANEL INPUT READY (PIN DISCOVERY MODE) ===\r\n");
    PanelPort_DebugWrite("Named device pins are not mapped; scanning KEY1..KEY23 / ENCODER1..9.\r\n");
#endif

    sprintf(buf, "CONFIG: KEYS=%u SWITCHES=%u SELECTORS=%u ENCODERS=%u\r\n",
            (unsigned)s_key_count,
            (unsigned)s_switch_count,
            (unsigned)s_selector_count,
            (unsigned)s_encoder_count);
    PanelPort_DebugWrite(buf);

    sprintf(buf, "ACTIVE: KEYS=%u SWITCHES=%u SELECTORS=%u ENCODERS=%u\r\n",
            (unsigned)active_keys,
            (unsigned)active_switches,
            (unsigned)active_selectors,
            (unsigned)active_encoders);
    PanelPort_DebugWrite(buf);

#if (PANEL_PIN_MAP_READY != 0)
    if ((active_keys < s_key_count) ||
        (active_switches < s_switch_count) ||
        (active_selectors < s_selector_count) ||
        (active_encoders < s_encoder_count))
    {
        PanelPort_DebugWrite("WARNING: some named devices have no valid pin mapping.\r\n");
    }
#endif
#endif
}

/* --------------------------------------------------------------------------
 * 对外接口
 * -------------------------------------------------------------------------- */
void PanelInput_Init(void)
{
    uint8_t i;

    s_q_head       = 0u;
    s_q_tail       = 0u;
    s_q_count      = 0u;
    s_now_ms       = 0u;
    s_handler      = 0;
    s_dropped      = 0u;
    s_key_count    = 0u;
    s_switch_count = 0u;
    s_selector_count = 0u;
    s_encoder_count  = 0u;
    s_initialized    = 0u;

    if ((g_panel_keys != 0) && (g_panel_key_count > 0u))
    {
        s_key_count = (g_panel_key_count > PANEL_KEY_MAX)
                      ? (uint8_t)PANEL_KEY_MAX : g_panel_key_count;

        for (i = 0u; i < s_key_count; i++)
        {
            PanelKey_Init(&s_keys[i], &g_panel_keys[i], i);
        }
    }

    if ((g_panel_switches != 0) && (g_panel_switch_count > 0u))
    {
        s_switch_count = (g_panel_switch_count > PANEL_SWITCH_MAX)
                         ? (uint8_t)PANEL_SWITCH_MAX : g_panel_switch_count;

        for (i = 0u; i < s_switch_count; i++)
        {
            PanelSwitch_Init(&s_switches[i], &g_panel_switches[i], i);
        }
    }

    if ((g_panel_selectors != 0) && (g_panel_selector_count > 0u))
    {
        s_selector_count = (g_panel_selector_count > PANEL_SELECTOR_MAX)
                           ? (uint8_t)PANEL_SELECTOR_MAX : g_panel_selector_count;

        for (i = 0u; i < s_selector_count; i++)
        {
            PanelSelector_Init(&s_selectors[i], &g_panel_selectors[i], i);
        }
    }

    if ((g_panel_encoders != 0) && (g_panel_encoder_count > 0u))
    {
        s_encoder_count = (g_panel_encoder_count > PANEL_ENCODER_MAX)
                          ? (uint8_t)PANEL_ENCODER_MAX : g_panel_encoder_count;

        for (i = 0u; i < s_encoder_count; i++)
        {
            PanelEncoder_Init(&s_encoders[i], &g_panel_encoders[i], i);
        }
    }

    PanelPort_DebugInit();
    panel_print_banner();

    s_initialized = 1u;
}

void PanelInput_Scan(void)
{
    uint8_t i;

    if (s_initialized == 0u)
    {
        return;
    }

    /* 软件时间戳，扫描周期必须与 PANEL_SCAN_PERIOD_MS 一致 */
    s_now_ms += PANEL_SCAN_PERIOD_MS;

    for (i = 0u; i < s_key_count; i++)
    {
        PanelKey_Scan(&s_keys[i], s_now_ms, panel_event_push);
    }

    for (i = 0u; i < s_switch_count; i++)
    {
        PanelSwitch_Scan(&s_switches[i], s_now_ms, panel_event_push);
    }

    for (i = 0u; i < s_selector_count; i++)
    {
        PanelSelector_Scan(&s_selectors[i], s_now_ms, panel_event_push);
    }

    for (i = 0u; i < s_encoder_count; i++)
    {
        PanelEncoder_Scan(&s_encoders[i], s_now_ms, panel_event_push);
    }
}

void PanelInput_Process(void)
{
    PanelEvent evt;

    while (panel_event_pop(&evt) != 0u)
    {
        panel_debug_print(&evt);

        if (s_handler != 0)
        {
            s_handler(&evt);
        }
    }

    if (s_dropped != 0u)
    {
#if (PANEL_DEBUG_ENABLE != 0)
        char buf[64];

        sprintf(buf, "! EVENT QUEUE OVERFLOW: %lu\r\n",
                (unsigned long)s_dropped);
        PanelPort_DebugWrite(buf);
#endif
        s_dropped = 0u;
    }
}

uint8_t PanelInput_PollEvent(PanelEvent *evt)
{
    return panel_event_pop(evt);
}

void PanelInput_SetEventHandler(PanelEventHandler handler)
{
    s_handler = handler;
}

uint32_t PanelInput_GetTickMs(void)
{
    return s_now_ms;
}

uint8_t PanelInput_GetKeyState(uint8_t id)
{
    if (id < s_key_count)
    {
        return PanelKey_IsPressed(&s_keys[id]);
    }

    return 0u;
}

uint8_t PanelInput_GetSwitchState(uint8_t id)
{
    if (id < s_switch_count)
    {
        return PanelSwitch_GetState(&s_switches[id]);
    }

    return 0u;
}

uint8_t PanelInput_GetSelectorPosition(uint8_t id)
{
    if (id < s_selector_count)
    {
        return PanelSelector_GetPosition(&s_selectors[id]);
    }

    return 0u;
}

uint8_t PanelInput_GetEncoderButtonState(uint8_t id)
{
    if (id < s_encoder_count)
    {
        return PanelEncoder_GetButtonState(&s_encoders[id]);
    }

    return 0u;
}
