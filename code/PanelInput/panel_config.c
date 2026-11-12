#include "panel_config.h"

/* ==========================================================================
 * 面板输入配置表 —— 按实体面板的 6 个区域组织
 *
 * 原理图依据：HSH_DISPLAY_KEY_V3_0.pdf 第 3 页（03_ARM）及第 5~8 页
 * （05_KEY_I / 06_KEY_II / 07_KEY_III / 08_KEY_IV）。
 *
 * 实体器件清单（共 21 个有效器件）：
 *
 *   THERMAL CAMERA
 *     - 拨动开关 THERMAL*、TRACKING
 *     - 瞬时按键 FOV、NUC
 *     - 带按压编码器 FOCUS(- <--> +)
 *     - 档位旋钮 ZOOM：1X / 2X / 4X / 8X
 *
 *   DAY CAMERA
 *     - 拨动开关 DAY*、TRACKING
 *     - 带按压编码器 ZOOM
 *
 *   LRF
 *     - 拨动开关 LRF、MODE
 *
 *   SYSTEM
 *     - 拨动开关 MAIN、STABILISATION
 *     - 档位旋钮 WIPER：1 / 2 / 3 / OFF
 *
 *   GUN
 *     - 拨动开关 GUN
 *     - 档位旋钮 AMMO TYPE：1~8
 *     - 瞬时按键 ALLIGN、RETICLE
 *
 *   DISPLAY
 *     - 拨动开关 SETTINGS
 *     - 带按压编码器 BRIGHTNESS(- <--> +)
 *     - 档位旋钮 COLOUR SCHEME：1~8
 *
 *   另有 2 个紧急按键：原理图中没有对应网络，按要求不纳入本框架。
 *
 * --------------------------------------------------------------------------
 * 当前唯一缺失的信息：实体器件 -> 原理图网络 -> MCU 引脚 的接线表。
 *
 * 原理图只提供 ARM_KEY1~23 / ARM_RENC1~9 这些“网络名”，
 * 第 5~8 页也只有上拉、串阻、滤波电容，没有 THERMAL/FOV/NUC 等丝印名。
 * 因此下面所有 PIN_xxx 先保持 PANEL_PIN_NONE，绝不猜测。
 *
 * 填表方法：
 *   1. 用万用表通断档，一端接某个实体器件，另一端依次碰 KEY / RENC
 *      连接器焊盘，找到对应的 ARM_KEYx / ARM_RENCx 网络；
 *   2. 把 PIN_xxx 从 PANEL_PIN_NONE 改成对应的 NET_KEYx / NET_RENCx_A/B/S；
 *   3. 档位旋钮还要确认线数 line_count 和编码方式 mode；
 *   4. 全部填完后，把 panel_config.h 中的 PANEL_PIN_MAP_READY 改成 1。
 *
 * 在 PANEL_PIN_MAP_READY == 0 时，框架进入“网络发现模式”：
 * 扫描 23 路 KEY 和 9 路编码器，操作哪个实体器件，串口就打印
 * 对应的 KEYx / ENCODERx，可用来完成上面的接线表。
 * ========================================================================== */

/* ==========================================================================
 * 一、原理图网络 -> MCU 引脚
 *
 * 这些宏来自 PDF 第 3 页网络标签与引脚编号的坐标配对。
 * 填 PIN_xxx 时直接引用 NET_xxx 即可，不需要重复写 GPIO。
 * ========================================================================== */

/* ARM_KEY1~23 */
#define NET_KEY1    PANEL_PIN_E(13)
#define NET_KEY2    PANEL_PIN_E(14)
#define NET_KEY3    PANEL_PIN_E(15)
#define NET_KEY4    PANEL_PIN_B(10)     /* 与 USART2 冲突 */
#define NET_KEY5    PANEL_PIN_B(11)     /* 与 USART2 冲突 */
#define NET_KEY6    PANEL_PIN_E(8)      /* 与 UART6 冲突 */
#define NET_KEY7    PANEL_PIN_E(9)
#define NET_KEY8    PANEL_PIN_E(10)
#define NET_KEY9    PANEL_PIN_E(11)
#define NET_KEY10   PANEL_PIN_E(12)
#define NET_KEY11   PANEL_PIN_C(4)
#define NET_KEY12   PANEL_PIN_C(5)
#define NET_KEY13   PANEL_PIN_B(0)
#define NET_KEY14   PANEL_PIN_B(1)
#define NET_KEY15   PANEL_PIN_E(7)      /* 与 UART6 冲突 */
#define NET_KEY16   PANEL_PIN_A(3)      /* 与 USART1 冲突 */
#define NET_KEY17   PANEL_PIN_A(2)      /* 与 USART1 冲突 */
#define NET_KEY18   PANEL_PIN_A(1)
#define NET_KEY19   PANEL_PIN_A(0)
#define NET_KEY20   PANEL_PIN_D(1)
#define NET_KEY21   PANEL_PIN_D(0)
#define NET_KEY22   PANEL_PIN_A(15)
#define NET_KEY23   PANEL_PIN_A(12)

/* ARM_RENC1~9：A/B 相与 S 按压信号 */
#define NET_RENC1_A PANEL_PIN_D(12)
#define NET_RENC1_B PANEL_PIN_D(11)
#define NET_RENC1_S PANEL_PIN_D(10)

#define NET_RENC2_A PANEL_PIN_B(15)
#define NET_RENC2_B PANEL_PIN_D(8)
#define NET_RENC2_S PANEL_PIN_D(9)

#define NET_RENC3_A PANEL_PIN_B(12)
#define NET_RENC3_B PANEL_PIN_B(13)
#define NET_RENC3_S PANEL_PIN_B(14)

#define NET_RENC4_A PANEL_PIN_A(9)      /* 与 USART0 冲突 */
#define NET_RENC4_B PANEL_PIN_A(10)     /* 与 USART0 冲突 */
#define NET_RENC4_S PANEL_PIN_A(11)

#define NET_RENC5_A PANEL_PIN_C(6)      /* 与 USART5 冲突 */
#define NET_RENC5_B PANEL_PIN_C(7)      /* 与 USART5 冲突 */
#define NET_RENC5_S PANEL_PIN_C(8)

#define NET_RENC6_A PANEL_PIN_D(15)     /* 与 RS485 方向脚冲突 */
#define NET_RENC6_B PANEL_PIN_D(14)
#define NET_RENC6_S PANEL_PIN_D(13)

#define NET_RENC7_A PANEL_PIN_B(5)
#define NET_RENC7_B PANEL_PIN_B(6)
#define NET_RENC7_S PANEL_PIN_B(7)

#define NET_RENC8_A PANEL_PIN_D(6)
#define NET_RENC8_B PANEL_PIN_D(7)
#define NET_RENC8_S PANEL_PIN_B(3)

#define NET_RENC9_A PANEL_PIN_D(3)
#define NET_RENC9_B PANEL_PIN_D(4)
#define NET_RENC9_S PANEL_PIN_D(5)

/* ==========================================================================
 * 二、实体器件 -> 网络 接线表（待确认）
 *
 * 把 PANEL_PIN_NONE 改成 NET_KEYx / NET_RENCx_A/B/S。
 * 例如确认 FOV 接在 ARM_KEY7 上：
 *     #define PIN_FOV     NET_KEY7
 * ========================================================================== */

/* ---------------- THERMAL CAMERA ---------------- */
#define PIN_THERMAL_SW          PANEL_PIN_NONE   /* THERMAL* 拨动开关 */
#define PIN_THERMAL_TRACKING    PANEL_PIN_NONE   /* TRACKING 拨动开关 */
#define PIN_FOV                 PANEL_PIN_NONE   /* FOV 瞬时按键 */
#define PIN_NUC                 PANEL_PIN_NONE   /* NUC 瞬时按键 */

#define PIN_FOCUS_A             PANEL_PIN_NONE   /* FOCUS 编码器 A 相 */
#define PIN_FOCUS_B             PANEL_PIN_NONE   /* FOCUS 编码器 B 相 */
#define PIN_FOCUS_S             PANEL_PIN_NONE   /* FOCUS 编码器按压 */

/* ZOOM 档位旋钮：4 档，实际线数/编码方式待确认 */
#define SEL_ZOOM_PINS           { PANEL_PIN_NONE, PANEL_PIN_NONE, \
                                  PANEL_PIN_NONE, PANEL_PIN_NONE }

/* ---------------- DAY CAMERA ---------------- */
#define PIN_DAY_SW              PANEL_PIN_NONE   /* DAY* 拨动开关 */
#define PIN_DAY_TRACKING        PANEL_PIN_NONE   /* TRACKING 拨动开关 */

#define PIN_DAY_ZOOM_A          PANEL_PIN_NONE   /* DAY ZOOM 编码器 A 相 */
#define PIN_DAY_ZOOM_B          PANEL_PIN_NONE   /* DAY ZOOM 编码器 B 相 */
#define PIN_DAY_ZOOM_S          PANEL_PIN_NONE   /* DAY ZOOM 编码器按压 */

/* ---------------- LRF ---------------- */
#define PIN_LRF_SW              PANEL_PIN_NONE   /* LRF 拨动开关 */
#define PIN_LRF_MODE            PANEL_PIN_NONE   /* MODE 拨动开关 */

/* ---------------- SYSTEM ---------------- */
#define PIN_SYS_MAIN            PANEL_PIN_NONE   /* MAIN 拨动开关 */
#define PIN_SYS_STAB            PANEL_PIN_NONE   /* STABILISATION 拨动开关 */

/* WIPER 档位旋钮：1 / 2 / 3 / OFF，实际线数/编码方式待确认 */
#define SEL_WIPER_PINS          { PANEL_PIN_NONE, PANEL_PIN_NONE, \
                                  PANEL_PIN_NONE, PANEL_PIN_NONE }

/* ---------------- GUN ---------------- */
#define PIN_GUN_SW              PANEL_PIN_NONE   /* GUN 拨动开关 */

/* AMMO TYPE 档位旋钮：8 档，实际线数/编码方式待确认 */
#define SEL_AMMO_PINS           { PANEL_PIN_NONE, PANEL_PIN_NONE, \
                                  PANEL_PIN_NONE, PANEL_PIN_NONE, \
                                  PANEL_PIN_NONE, PANEL_PIN_NONE, \
                                  PANEL_PIN_NONE, PANEL_PIN_NONE }

#define PIN_ALLIGN              PANEL_PIN_NONE   /* ALLIGN 瞬时按键 */
#define PIN_RETICLE             PANEL_PIN_NONE   /* RETICLE 瞬时按键 */

/* ---------------- DISPLAY ---------------- */
#define PIN_DISPLAY_SETTINGS    PANEL_PIN_NONE   /* SETTINGS 拨动开关 */

#define PIN_BRIGHT_A            PANEL_PIN_NONE   /* BRIGHTNESS 编码器 A 相 */
#define PIN_BRIGHT_B            PANEL_PIN_NONE   /* BRIGHTNESS 编码器 B 相 */
#define PIN_BRIGHT_S            PANEL_PIN_NONE   /* BRIGHTNESS 编码器按压 */

/* COLOUR SCHEME 档位旋钮：8 档，实际线数/编码方式待确认 */
#define SEL_COLOUR_PINS         { PANEL_PIN_NONE, PANEL_PIN_NONE, \
                                  PANEL_PIN_NONE, PANEL_PIN_NONE, \
                                  PANEL_PIN_NONE, PANEL_PIN_NONE, \
                                  PANEL_PIN_NONE, PANEL_PIN_NONE }

#if (PANEL_PIN_MAP_READY != 0)

/* ==========================================================================
 * 三、实名器件配置表
 *
 * 只有 PANEL_PIN_MAP_READY == 1 时才参与编译。
 * 如果某个 PIN_xxx 仍为 PANEL_PIN_NONE，对应器件会被驱动自动跳过，
 * 上电横幅里的 ACTIVE 计数会小于 CONFIG 计数。
 * ========================================================================== */

/* ---------------- 档位显示名 ---------------- */
static const char *const s_zoom_labels[]   = {"1X", "2X", "4X", "8X"};
static const char *const s_wiper_labels[]  = {"1", "2", "3", "OFF"};
static const char *const s_ammo_labels[]   = {"1", "2", "3", "4",
                                              "5", "6", "7", "8"};
static const char *const s_colour_labels[] = {"1", "2", "3", "4",
                                              "5", "6", "7", "8"};

/* ---------------- 瞬时按键：4 个 ---------------- */
static const PanelKeyConfig s_keys[] =
{
    {"THERMAL CAMERA/FOV",  PIN_FOV,   PANEL_ACTIVE_LOW,
     PANEL_DEBOUNCE_MS, PANEL_LONG_PRESS_MS},

    {"THERMAL CAMERA/NUC",  PIN_NUC,   PANEL_ACTIVE_LOW,
     PANEL_DEBOUNCE_MS, PANEL_LONG_PRESS_MS},

    {"GUN/ALLIGN",          PIN_ALLIGN, PANEL_ACTIVE_LOW,
     PANEL_DEBOUNCE_MS, PANEL_LONG_PRESS_MS},

    {"GUN/RETICLE",         PIN_RETICLE, PANEL_ACTIVE_LOW,
     PANEL_DEBOUNCE_MS, PANEL_LONG_PRESS_MS}
};

/* ---------------- 保持型拨动开关：10 个 ----------------
 * 暂按“单路 ON/OFF”占位。如果实测是两路 ON/OFF/ON，
 * 请把 pin_count 改成 2 并补 state_map。
 * 有效电平也必须在实测后确认，这里先按外部上拉的低有效配置。
 */
static const PanelSwitchConfig s_switches[] =
{
    {"THERMAL CAMERA/THERMAL*",      {PIN_THERMAL_SW},       1,
     PANEL_ACTIVE_LOW, PANEL_DEBOUNCE_MS, 2, 0},

    {"THERMAL CAMERA/TRACKING",      {PIN_THERMAL_TRACKING}, 1,
     PANEL_ACTIVE_LOW, PANEL_DEBOUNCE_MS, 2, 0},

    {"DAY CAMERA/DAY*",              {PIN_DAY_SW},           1,
     PANEL_ACTIVE_LOW, PANEL_DEBOUNCE_MS, 2, 0},

    {"DAY CAMERA/TRACKING",          {PIN_DAY_TRACKING},     1,
     PANEL_ACTIVE_LOW, PANEL_DEBOUNCE_MS, 2, 0},

    {"LRF/LRF",                      {PIN_LRF_SW},           1,
     PANEL_ACTIVE_LOW, PANEL_DEBOUNCE_MS, 2, 0},

    {"LRF/MODE",                     {PIN_LRF_MODE},         1,
     PANEL_ACTIVE_LOW, PANEL_DEBOUNCE_MS, 2, 0},

    {"SYSTEM/MAIN",                  {PIN_SYS_MAIN},         1,
     PANEL_ACTIVE_LOW, PANEL_DEBOUNCE_MS, 2, 0},

    {"SYSTEM/STABILISATION",         {PIN_SYS_STAB},         1,
     PANEL_ACTIVE_LOW, PANEL_DEBOUNCE_MS, 2, 0},

    {"GUN/GUN",                      {PIN_GUN_SW},           1,
     PANEL_ACTIVE_LOW, PANEL_DEBOUNCE_MS, 2, 0},

    {"DISPLAY/SETTINGS",             {PIN_DISPLAY_SETTINGS}, 1,
     PANEL_ACTIVE_LOW, PANEL_DEBOUNCE_MS, 2, 0}
};

/* ---------------- 档位旋钮：4 个 ----------------
 * line_count = 0 表示线数尚未确认，驱动会跳过该器件。
 * 确认后填实际线数，并把 mode 改成 ONEHOT / BINARY / GRAY。
 * 不要按编码器 A/B 相处理。
 */
static const PanelSelectorConfig s_selectors[] =
{
    {"THERMAL CAMERA/ZOOM", SEL_ZOOM_PINS, 0,
     PANEL_SELECTOR_ONEHOT, PANEL_ACTIVE_LOW, PANEL_DEBOUNCE_MS,
     1, 4, 0, s_zoom_labels},

    {"SYSTEM/WIPER",        SEL_WIPER_PINS, 0,
     PANEL_SELECTOR_ONEHOT, PANEL_ACTIVE_LOW, PANEL_DEBOUNCE_MS,
     1, 4, 0, s_wiper_labels},

    {"GUN/AMMO TYPE",       SEL_AMMO_PINS, 0,
     PANEL_SELECTOR_ONEHOT, PANEL_ACTIVE_LOW, PANEL_DEBOUNCE_MS,
     1, 8, 0, s_ammo_labels},

    {"DISPLAY/COLOUR SCHEME", SEL_COLOUR_PINS, 0,
     PANEL_SELECTOR_ONEHOT, PANEL_ACTIVE_LOW, PANEL_DEBOUNCE_MS,
     1, 8, 0, s_colour_labels}
};

/* ---------------- EC11 编码器：3 个 ---------------- */
static const PanelEncoderConfig s_encoders[] =
{
    {"THERMAL CAMERA/FOCUS", PIN_FOCUS_A, PIN_FOCUS_B, PIN_FOCUS_S,
     PANEL_ACTIVE_LOW, PANEL_DEBOUNCE_MS, PANEL_LONG_PRESS_MS,
     PANEL_ENCODER_STEPS_PER_DETENT},

    {"DAY CAMERA/ZOOM",      PIN_DAY_ZOOM_A, PIN_DAY_ZOOM_B, PIN_DAY_ZOOM_S,
     PANEL_ACTIVE_LOW, PANEL_DEBOUNCE_MS, PANEL_LONG_PRESS_MS,
     PANEL_ENCODER_STEPS_PER_DETENT},

    {"DISPLAY/BRIGHTNESS",   PIN_BRIGHT_A, PIN_BRIGHT_B, PIN_BRIGHT_S,
     PANEL_ACTIVE_LOW, PANEL_DEBOUNCE_MS, PANEL_LONG_PRESS_MS,
     PANEL_ENCODER_STEPS_PER_DETENT}
};

const PanelKeyConfig *const g_panel_keys = s_keys;
const uint8_t g_panel_key_count =
    (uint8_t)(sizeof(s_keys) / sizeof(s_keys[0]));

const PanelSwitchConfig *const g_panel_switches = s_switches;
const uint8_t g_panel_switch_count =
    (uint8_t)(sizeof(s_switches) / sizeof(s_switches[0]));

const PanelSelectorConfig *const g_panel_selectors = s_selectors;
const uint8_t g_panel_selector_count =
    (uint8_t)(sizeof(s_selectors) / sizeof(s_selectors[0]));

const PanelEncoderConfig *const g_panel_encoders = s_encoders;
const uint8_t g_panel_encoder_count =
    (uint8_t)(sizeof(s_encoders) / sizeof(s_encoders[0]));

#else

/* ==========================================================================
 * 四、网络发现模式（PANEL_PIN_MAP_READY == 0）
 *
 * 扫描全部 23 路 ARM_KEY 和 9 路 ARM_RENC。
 * 一次只操作一个实体器件，观察串口打印的 KEYx / ENCODERx，
 * 记录到上面的 PIN_xxx，即可完成接线表。
 *
 * 注意：拨动开关和多档位旋钮在这个模式下也会以 KEYx 的形式出现，
 * 这是正常的，因为驱动只看到“网络电平变化”，还不知道它是什么器件。
 * ========================================================================== */

/* ---------------- 23 路按键网络 ---------------- */
static const PanelKeyConfig s_discovery_keys[] =
{
    {"KEY1",  NET_KEY1,  PANEL_ACTIVE_LOW, PANEL_DEBOUNCE_MS, PANEL_LONG_PRESS_MS},
    {"KEY2",  NET_KEY2,  PANEL_ACTIVE_LOW, PANEL_DEBOUNCE_MS, PANEL_LONG_PRESS_MS},
    {"KEY3",  NET_KEY3,  PANEL_ACTIVE_LOW, PANEL_DEBOUNCE_MS, PANEL_LONG_PRESS_MS},
    {"KEY4",  NET_KEY4,  PANEL_ACTIVE_LOW, PANEL_DEBOUNCE_MS, PANEL_LONG_PRESS_MS},
    {"KEY5",  NET_KEY5,  PANEL_ACTIVE_LOW, PANEL_DEBOUNCE_MS, PANEL_LONG_PRESS_MS},
    {"KEY6",  NET_KEY6,  PANEL_ACTIVE_LOW, PANEL_DEBOUNCE_MS, PANEL_LONG_PRESS_MS},
    {"KEY7",  NET_KEY7,  PANEL_ACTIVE_LOW, PANEL_DEBOUNCE_MS, PANEL_LONG_PRESS_MS},
    {"KEY8",  NET_KEY8,  PANEL_ACTIVE_LOW, PANEL_DEBOUNCE_MS, PANEL_LONG_PRESS_MS},
    {"KEY9",  NET_KEY9,  PANEL_ACTIVE_LOW, PANEL_DEBOUNCE_MS, PANEL_LONG_PRESS_MS},
    {"KEY10", NET_KEY10, PANEL_ACTIVE_LOW, PANEL_DEBOUNCE_MS, PANEL_LONG_PRESS_MS},
    {"KEY11", NET_KEY11, PANEL_ACTIVE_LOW, PANEL_DEBOUNCE_MS, PANEL_LONG_PRESS_MS},
    {"KEY12", NET_KEY12, PANEL_ACTIVE_LOW, PANEL_DEBOUNCE_MS, PANEL_LONG_PRESS_MS},
    {"KEY13", NET_KEY13, PANEL_ACTIVE_LOW, PANEL_DEBOUNCE_MS, PANEL_LONG_PRESS_MS},
    {"KEY14", NET_KEY14, PANEL_ACTIVE_LOW, PANEL_DEBOUNCE_MS, PANEL_LONG_PRESS_MS},
    {"KEY15", NET_KEY15, PANEL_ACTIVE_LOW, PANEL_DEBOUNCE_MS, PANEL_LONG_PRESS_MS},
    {"KEY16", NET_KEY16, PANEL_ACTIVE_LOW, PANEL_DEBOUNCE_MS, PANEL_LONG_PRESS_MS},
    {"KEY17", NET_KEY17, PANEL_ACTIVE_LOW, PANEL_DEBOUNCE_MS, PANEL_LONG_PRESS_MS},
    {"KEY18", NET_KEY18, PANEL_ACTIVE_LOW, PANEL_DEBOUNCE_MS, PANEL_LONG_PRESS_MS},
    {"KEY19", NET_KEY19, PANEL_ACTIVE_LOW, PANEL_DEBOUNCE_MS, PANEL_LONG_PRESS_MS},
    {"KEY20", NET_KEY20, PANEL_ACTIVE_LOW, PANEL_DEBOUNCE_MS, PANEL_LONG_PRESS_MS},
    {"KEY21", NET_KEY21, PANEL_ACTIVE_LOW, PANEL_DEBOUNCE_MS, PANEL_LONG_PRESS_MS},
    {"KEY22", NET_KEY22, PANEL_ACTIVE_LOW, PANEL_DEBOUNCE_MS, PANEL_LONG_PRESS_MS},
    {"KEY23", NET_KEY23, PANEL_ACTIVE_LOW, PANEL_DEBOUNCE_MS, PANEL_LONG_PRESS_MS}
};

/* ---------------- 9 路编码器网络 ---------------- */
static const PanelEncoderConfig s_discovery_encoders[] =
{
    {"ENCODER1", NET_RENC1_A, NET_RENC1_B, NET_RENC1_S,
     PANEL_ACTIVE_LOW, PANEL_DEBOUNCE_MS, PANEL_LONG_PRESS_MS,
     PANEL_ENCODER_STEPS_PER_DETENT},

    {"ENCODER2", NET_RENC2_A, NET_RENC2_B, NET_RENC2_S,
     PANEL_ACTIVE_LOW, PANEL_DEBOUNCE_MS, PANEL_LONG_PRESS_MS,
     PANEL_ENCODER_STEPS_PER_DETENT},

    {"ENCODER3", NET_RENC3_A, NET_RENC3_B, NET_RENC3_S,
     PANEL_ACTIVE_LOW, PANEL_DEBOUNCE_MS, PANEL_LONG_PRESS_MS,
     PANEL_ENCODER_STEPS_PER_DETENT},

    {"ENCODER4", NET_RENC4_A, NET_RENC4_B, NET_RENC4_S,
     PANEL_ACTIVE_LOW, PANEL_DEBOUNCE_MS, PANEL_LONG_PRESS_MS,
     PANEL_ENCODER_STEPS_PER_DETENT},

    {"ENCODER5", NET_RENC5_A, NET_RENC5_B, NET_RENC5_S,
     PANEL_ACTIVE_LOW, PANEL_DEBOUNCE_MS, PANEL_LONG_PRESS_MS,
     PANEL_ENCODER_STEPS_PER_DETENT},

    {"ENCODER6", NET_RENC6_A, NET_RENC6_B, NET_RENC6_S,
     PANEL_ACTIVE_LOW, PANEL_DEBOUNCE_MS, PANEL_LONG_PRESS_MS,
     PANEL_ENCODER_STEPS_PER_DETENT},

    {"ENCODER7", NET_RENC7_A, NET_RENC7_B, NET_RENC7_S,
     PANEL_ACTIVE_LOW, PANEL_DEBOUNCE_MS, PANEL_LONG_PRESS_MS,
     PANEL_ENCODER_STEPS_PER_DETENT},

    {"ENCODER8", NET_RENC8_A, NET_RENC8_B, NET_RENC8_S,
     PANEL_ACTIVE_LOW, PANEL_DEBOUNCE_MS, PANEL_LONG_PRESS_MS,
     PANEL_ENCODER_STEPS_PER_DETENT},

    {"ENCODER9", NET_RENC9_A, NET_RENC9_B, NET_RENC9_S,
     PANEL_ACTIVE_LOW, PANEL_DEBOUNCE_MS, PANEL_LONG_PRESS_MS,
     PANEL_ENCODER_STEPS_PER_DETENT}
};

const PanelKeyConfig *const g_panel_keys = s_discovery_keys;
const uint8_t g_panel_key_count =
    (uint8_t)(sizeof(s_discovery_keys) / sizeof(s_discovery_keys[0]));

const PanelSwitchConfig *const g_panel_switches   = 0;
const uint8_t                  g_panel_switch_count = 0u;

const PanelSelectorConfig *const g_panel_selectors    = 0;
const uint8_t                    g_panel_selector_count = 0u;

const PanelEncoderConfig *const g_panel_encoders = s_discovery_encoders;
const uint8_t g_panel_encoder_count =
    (uint8_t)(sizeof(s_discovery_encoders) / sizeof(s_discovery_encoders[0]));

#endif /* PANEL_PIN_MAP_READY */
