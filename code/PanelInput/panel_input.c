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



static void panel_event_push(const PanelEvent *evt);
static uint8_t panel_event_pop(PanelEvent *evt);
static const char *panel_name_for(const PanelEvent *evt);
static const char *panel_selector_label(uint8_t id, int16_t position);
static void panel_print_state(const char *tag, const char *name, int value);
static void panel_print_all_states(void);
static void panel_debug_print(const PanelEvent *evt);
static void panel_print_banner(void);



/* 把一条事件写入环形队列；队列满时丢弃并累加溢出计数。 */
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

/* 从环形队列取出最早的一条事件；队列为空返回 0。 */
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

/* 根据事件类型和 id 找到对应配置里的设备名，用于串口打印；
 * 找不到时返回 "KEY"/"SWITCH"/"SELECTOR"/"ENCODER" 之类的通用名。 */
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

/* 把 SELECTOR_CHANGED 的档位号（1~N）转成显示标签，如 "1X"/"2X"/"OFF"；
 * 没配标签或档位越界返回 0。 */
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

/* 统一的状态输出格式：TAG name: value */
static void panel_print_state(const char *tag, const char *name, int value)
{
#if (PANEL_DEBUG_ENABLE != 0)
    char buf[96];

    if (name == 0)
    {
        name = "?";
    }

    sprintf(buf, "%s %s: %d\r\n", tag, name, value);
    PanelPort_DebugWrite(buf);
#else
    (void)tag;
    (void)name;
    (void)value;
#endif
}

/* 上电时把所有设备的初始状态打印一遍 */
static void panel_print_all_states(void)
{
#if (PANEL_DEBUG_ENABLE != 0)
    uint8_t i;

    PanelPort_DebugWrite("=== PANEL INITIAL STATE ===\r\n");

    /* 4 个瞬时按键：0 = 松开，1 = 按下 */
    for (i = 0u; i < s_key_count; i++)
    {
        panel_print_state("KEY", g_panel_keys[i].name,
                          (int)PanelInput_GetKeyState(i));
    }

    /* 10 个拨码/自锁开关：0/1 */
    for (i = 0u; i < s_switch_count; i++)
    {
        panel_print_state("SW", g_panel_switches[i].name,
                          (int)PanelInput_GetSwitchState(i));
    }

    /* 4 个多档位开关：1~N，0 = 未知 */
    for (i = 0u; i < s_selector_count; i++)
    {
        panel_print_state("SEL", g_panel_selectors[i].name,
                          (int)PanelInput_GetSelectorPosition(i));
    }

    /* 3 个编码器的 S 相按键：0/1；S 相没接时恒为 0 */
    for (i = 0u; i < s_encoder_count; i++)
    {
        panel_print_state("ENC", g_panel_encoders[i].name,
                          (int)PanelInput_GetEncoderButtonState(i));
    }
#endif
}


/* 状态变化时只打印“变化后的新状态”
 * KEY : 0/1（松开/按下）
 * SW  : 0/1
 * SEL : 1~N
 * ENC : 0/1（S 相按键）
 * 编码器旋转仍保留 CW/CCW 输出；不需要的话删掉那两个 case。
 */
static void panel_debug_print(const PanelEvent *evt)
{
#if (PANEL_DEBUG_ENABLE != 0)
    char        buf[96];
    const char *name;

    name = panel_name_for(evt);

    switch (evt->type)
    {
        case PANEL_EVT_KEY_PRESSED:
            panel_print_state("KEY", name, 1);
            break;

        case PANEL_EVT_KEY_RELEASED:
            panel_print_state("KEY", name, 0);
            break;

        case PANEL_EVT_KEY_LONG_PRESS:
        case PANEL_EVT_KEY_SHORT_PRESS:
            /* 只关心状态，长按/短按不再单独输出 */
            break;

        case PANEL_EVT_SWITCH_CHANGED:
            panel_print_state("SW", name, (int)evt->value);
            break;

        case PANEL_EVT_SELECTOR_CHANGED:
            panel_print_state("SEL", name, (int)evt->value);
            break;

        case PANEL_EVT_ENCODER_CW:
            if (evt->value > 1)
            {
                sprintf(buf, "%s CLOCKWISE x%d\r\n", name, (int)evt->value);
            }
            else
            {
                sprintf(buf, "%s CLOCKWISE\r\n", name);
            }
            PanelPort_DebugWrite(buf);
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
            PanelPort_DebugWrite(buf);
            break;

        case PANEL_EVT_ENCODER_PRESSED:
            panel_print_state("ENC", name, 1);
            break;

        case PANEL_EVT_ENCODER_RELEASED:
            panel_print_state("ENC", name, 0);
            break;

        case PANEL_EVT_ENCODER_LONG_PRESS:
            break;

        default:
            break;
    }
#else
    (void)evt;
#endif
}


/* 上电打印横幅：工作模式、配置的设备数量、实际生效的数量；
 * 有设备因引脚无效未初始化时打印 WARNING。 */
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

    PanelPort_DebugWrite("\r\n=== PANEL INPUT READY ===\r\n");


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

    if ((active_keys < s_key_count) ||
        (active_switches < s_switch_count) ||
        (active_selectors < s_selector_count) ||
        (active_encoders < s_encoder_count))
    {
        PanelPort_DebugWrite("WARNING: some named devices have no valid pin mapping.\r\n");
    }

#endif
}

/* 模块总初始化：
 * 按 panel_config.c 的配置初始化全部按键/拨码/选择开关/编码器，
 * 初始化调试串口并打印横幅。
 * 注意：这里会把事件回调清空，所以 PanelInput_SetEventHandler() 必须放在它之后。 */
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

    panel_print_banner();

    panel_print_all_states();
    s_initialized = 1u;
}

/* 每 PANEL_SCAN_PERIOD_MS 调用一次：
 * 软件时基 +5ms，然后依次扫描所有设备，把产生的事件放入队列。 */
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

/* 主循环里每轮调用：
 * 取出队列中所有事件，先串口打印，再调用注册的业务回调；
 * 没有注册回调时就只有打印。 */
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


/* 轮询方式取一条事件：取到返回 1，队列为空返回 0。
 * 注意：事件会先被 PanelInput_Process() 取走，两者不要混用。 */
uint8_t PanelInput_PollEvent(PanelEvent *evt)
{
    return panel_event_pop(evt);
}

/* 注册业务事件回调；必须在 PanelInput_Init() 之后调用，否则会被 Init 清掉。 */
void PanelInput_SetEventHandler(PanelEventHandler handler)
{
    s_handler = handler;
}

/* 返回模块内部维护的软件毫秒时基（每扫描一次 +5ms），仅供业务层参考。 */
uint32_t PanelInput_GetTickMs(void)
{
    return s_now_ms;
}

/* 按 id 查询按键当前稳定状态；id 越界返回 0。 */
uint8_t PanelInput_GetKeyState(uint8_t id)
{
    if (id < s_key_count)
    {
        return PanelKey_IsPressed(&s_keys[id]);
    }

    return 0u;
}

/* 按 id 查询拨码当前逻辑状态（0/1）；id 越界返回 0。 */
uint8_t PanelInput_GetSwitchState(uint8_t id)
{
    if (id < s_switch_count)
    {
        return PanelSwitch_GetState(&s_switches[id]);
    }

    return 0u;
}

/* 按 id 查询多档位开关当前档位（1~N，0 = 未知）；id 越界返回 0。 */
uint8_t PanelInput_GetSelectorPosition(uint8_t id)
{
    if (id < s_selector_count)
    {
        return PanelSelector_GetPosition(&s_selectors[id]);
    }

    return 0u;
}

/* 按 id 查询编码器 S 相按压状态；S 相已置 NONE 时恒为 0。 */
uint8_t PanelInput_GetEncoderButtonState(uint8_t id)
{
    if (id < s_encoder_count)
    {
        return PanelEncoder_GetButtonState(&s_encoders[id]);
    }

    return 0u;
}
