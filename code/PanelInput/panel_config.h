#ifndef __PANEL_CONFIG_H__
#define __PANEL_CONFIG_H__

#include "gd32f4xx.h"
#include "panel_types.h"
#include "panel_port.h"
#include "panel_key.h"
#include "panel_switch.h"
#include "panel_selector.h"
#include "panel_encoder.h"

/* ==========================================================================
 * 面板输入配置
 *
 * 面板按 6 个区域组织：
 *   THERMAL CAMERA / DAY CAMERA / LRF / SYSTEM / GUN / DISPLAY
 * 共 21 个有效器件，另有 2 个紧急按键（原理图无网络，不纳入）。
 *
 * 重要：
 * 1. 原理图只提供 ARM_KEY1~23 和 ARM_RENC1~9 网络，
 *    没有 THERMAL/FOV/NUC 等丝印名，因此“实体器件 -> 网络”的接线表
 *    目前尚未确认。
 * 2. panel_config.c 中所有 PIN_xxx 先保持 PANEL_PIN_NONE，不猜引脚。
 * 3. 接线表填完后，把下面的 PANEL_PIN_MAP_READY 改成 1。
 * 4. PANEL_PIN_MAP_READY == 0 时，框架自动进入“网络发现模式”，
 *    扫描 23 路 KEY 和 9 路编码器，用于反查实体器件对应的网络。
 * ========================================================================== */

#define PANEL_PIN_MAP_READY             0

/* ---------------- 扫描周期与消抖 ---------------- */
#define PANEL_SCAN_PERIOD_MS            5u      /* PanelInput_Scan 的调用周期 */
#define PANEL_DEBOUNCE_MS               20u     /* 默认消抖时间 */
#define PANEL_LONG_PRESS_MS             800u    /* 默认长按阈值 */

/* 默认不产生短按事件；需要时置 1。
 * 短按也可以由业务层根据 RELEASED 事件里的 value(按住时长) 自行判断。
 */
#define PANEL_KEY_SHORT_PRESS_ENABLE    0

/* ---------------- 事件队列 ---------------- */
#define PANEL_EVENT_QUEUE_SIZE          32u

/* ---------------- 各类设备最大实例数 ----------------
 * 这些只是数组上限，实际使用数量由 panel_config.c 中的表决定。
 * 当前面板：4 按键 + 10 拨动开关 + 4 档位旋钮 + 3 编码器。
 */
#define PANEL_KEY_MAX                   32u
#define PANEL_SWITCH_MAX                16u
#define PANEL_SELECTOR_MAX              4u
#define PANEL_ENCODER_MAX               16u

/* ---------------- 编码器参数 ---------------- */
/* 每个机械档位对应的 A/B 正交边沿数。
 * EC11 常见为 4；如果实测转一格报多格/少格，优先调整这个值。
 */
#define PANEL_ENCODER_STEPS_PER_DETENT  4u

/* 方向反了就把它改成 1，不需要改接线 */
#define PANEL_ENCODER_REVERSE_DIRECTION 0

/* ---------------- 调试串口 ----------------
 * 选用 UART4（PC12/PD2），不与面板按键/编码器引脚冲突。
 * 如果实际调试口不是 UART4，只改这里和 panel_port.c 的引脚配置。
 */
#define PANEL_DEBUG_ENABLE              1
#define PANEL_DEBUG_BAUDRATE            115200u
#define PANEL_DEBUG_UART                UART4

/* ---------------- 引脚宏 ----------------
 * 用法：PANEL_PIN_E(13) 表示 GPIOE 的 PIN13，同时携带 RCU_GPIOE。
 */
#define PANEL_PIN_A(n)  {GPIOA, GPIO_PIN_##n, RCU_GPIOA}
#define PANEL_PIN_B(n)  {GPIOB, GPIO_PIN_##n, RCU_GPIOB}
#define PANEL_PIN_C(n)  {GPIOC, GPIO_PIN_##n, RCU_GPIOC}
#define PANEL_PIN_D(n)  {GPIOD, GPIO_PIN_##n, RCU_GPIOD}
#define PANEL_PIN_E(n)  {GPIOE, GPIO_PIN_##n, RCU_GPIOE}

/* ---------------- 配置表（定义在 panel_config.c） ---------------- */
extern const PanelKeyConfig      *const g_panel_keys;
extern const uint8_t                    g_panel_key_count;

extern const PanelSwitchConfig   *const g_panel_switches;
extern const uint8_t                    g_panel_switch_count;

extern const PanelSelectorConfig *const g_panel_selectors;
extern const uint8_t                    g_panel_selector_count;

extern const PanelEncoderConfig  *const g_panel_encoders;
extern const uint8_t                    g_panel_encoder_count;

#endif /* __PANEL_CONFIG_H__ */
