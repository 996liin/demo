#include "panel_config.h"


/* ---------------- THERMAL CAMERA ---------------- */
#define TH_SW_PIN          PANEL_PIN_A(10)                  /* THERMAL* 拨动开关 1 */
#define TH_TRACK_SW_PIN    PANEL_PIN_A(11)                  /* TRACKING 拨动开关 2 */
#define TH_FOV_KEY_PIN                 PANEL_PIN_C(7)       /* FOV 瞬时按键 1 */
#define TH_NUC_KEY_PIN                 PANEL_PIN_C(8)       /* NUC 瞬时按键 2 */

#define TH_FOCUS_A_PIN             PANEL_PIN_D(12)           /* FOCUS 编码器 A 相 */
#define TH_FOCUS_B_PIN             PANEL_PIN_D(11)           /* FOCUS 编码器 B 相 */
#define TH_FOCUS_S_PIN             PANEL_PIN_D(10)           /* FOCUS 编码器按压 */

/* ZOOM 档位旋钮：4 档，实际线数/编码方式待确认 */
#define TH_ZOOM_SEL_PINS           { PANEL_PIN_E(14), PANEL_PIN_E(15), \
                                  PANEL_PIN_B(10), PANEL_PIN_B(11) }

/* ---------------- DAY CAMERA ---------------- */
#define DAY_SW_PIN              PANEL_PIN_D(14)                  /* DAY* 拨动开关 3 */
#define DAY_TRACK_SW_PIN        PANEL_PIN_D(13)                  /* TRACKING 拨动开关 4 */

#define DAY_ZOOM_A_PIN          PANEL_PIN_B(15)                   /* DAY ZOOM 编码器 A 相 */
#define DAY_ZOOM_B_PIN          PANEL_PIN_D(8)                   /* DAY ZOOM 编码器 B 相 */
#define DAY_ZOOM_S_PIN          PANEL_PIN_D(9)                   /* DAY ZOOM 编码器按压 */

/* ---------------- LRF ---------------- */
#define LRF_SW_PIN              PANEL_PIN_B(6)                   /* LRF 拨动开关 5 */
#define LRF_MODE_SW_PIN            PANEL_PIN_B(7)                /* MODE 拨动开关 6 */

/* ---------------- SYSTEM ---------------- */
#define SYS_MAIN_SW_PIN            PANEL_PIN_D(7)                /* MAIN 拨动开关 7 */
#define SYS_STAB_SW_PIN            PANEL_PIN_B(3)                /* STABILISATION 拨动开关 8 */

/* WIPER 档位旋钮：1 / 2 / 3 / OFF， */
#define SYS_WIPER_SEL_PINS          { PANEL_PIN_E(9), PANEL_PIN_E(10), \
                                  PANEL_PIN_E(11), PANEL_PIN_E(12) }

/* ---------------- GUN ---------------- */
#define GUN_SW_PIN              PANEL_PIN_E(13)                  /* GUN 拨动开关 9 */

/* AMMO TYPE 档位旋钮：8 档，实际线数/编码方式待确认 */
#define GUN_AMMO_SEL_PINS           { PANEL_PIN_C(4), PANEL_PIN_C(5), \
                                  PANEL_PIN_B(0), PANEL_PIN_B(1), \
                                  PANEL_PIN_E(7), PANEL_PIN_A(9), \
                                  PANEL_PIN_C(6), PANEL_PIN_D(15) }

#define GUN_ALIGN_KEY_PIN              PANEL_PIN_D(4)           /* ALLIGN 瞬时按键 3 */
#define GUN_RETICLE_KEY_PIN             PANEL_PIN_D(5)          /* RETICLE 瞬时按键 4 */

/* ---------------- DISPLAY ---------------- */
#define DISP_SETTINGS_SW_PIN    PANEL_PIN_E(8)                  /* SETTINGS 拨动开关 10 */

#define DISP_BRIGHT_A_PIN            PANEL_PIN_B(12)             /* BRIGHTNESS 编码器 A 相 */
#define DISP_BRIGHT_B_PIN            PANEL_PIN_B(13)             /* BRIGHTNESS 编码器 B 相 */
#define DISP_BRIGHT_S_PIN            PANEL_PIN_B(14)             /* BRIGHTNESS 编码器按压 */

/* COLOUR SCHEME 档位旋钮：8 档，实际线数/编码方式待确认 */
#define DISP_COLOUR_SEL_PINS         { PANEL_PIN_A(3), PANEL_PIN_A(2), \
                                  PANEL_PIN_A(1), PANEL_PIN_A(0), \
                                  PANEL_PIN_D(1), PANEL_PIN_D(0), \
                                  PANEL_PIN_A(15), PANEL_PIN_A(12) }



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
    {"TH/FOV",          TH_FOV_KEY_PIN,
     PANEL_ACTIVE_LOW, PANEL_DEBOUNCE_MS, PANEL_LONG_PRESS_MS},

    {"TH/NUC",          TH_NUC_KEY_PIN,
     PANEL_ACTIVE_LOW, PANEL_DEBOUNCE_MS, PANEL_LONG_PRESS_MS},

    {"GUN/ALIGN",       GUN_ALIGN_KEY_PIN,
     PANEL_ACTIVE_LOW, PANEL_DEBOUNCE_MS, PANEL_LONG_PRESS_MS},

    {"GUN/RETICLE",     GUN_RETICLE_KEY_PIN,
     PANEL_ACTIVE_LOW, PANEL_DEBOUNCE_MS, PANEL_LONG_PRESS_MS}
};

/* ---------------- 保持型拨动开关：10 个 ----------------
 * 暂按“单路 ON/OFF”占位。如果实测是两路 ON/OFF/ON，
 * 请把 pin_count 改成 2 并补 state_map。
 * 有效电平也必须在实测后确认，这里先按外部上拉的低有效配置。
 */
static const PanelSwitchConfig s_switches[] =
{
    {"TH/SW",           {TH_SW_PIN},           1,
     PANEL_ACTIVE_LOW, PANEL_DEBOUNCE_MS, 2, 0},

    {"TH/TRACKING",     {TH_TRACK_SW_PIN},     1,
     PANEL_ACTIVE_LOW, PANEL_DEBOUNCE_MS, 2, 0},

    {"DAY/SW",          {DAY_SW_PIN},          1,
     PANEL_ACTIVE_LOW, PANEL_DEBOUNCE_MS, 2, 0},

    {"DAY/TRACKING",    {DAY_TRACK_SW_PIN},    1,
     PANEL_ACTIVE_LOW, PANEL_DEBOUNCE_MS, 2, 0},

    {"LRF/SW",          {LRF_SW_PIN},          1,
     PANEL_ACTIVE_LOW, PANEL_DEBOUNCE_MS, 2, 0},

    {"LRF/MODE",        {LRF_MODE_SW_PIN},     1,
     PANEL_ACTIVE_LOW, PANEL_DEBOUNCE_MS, 2, 0},

    {"SYS/MAIN",        {SYS_MAIN_SW_PIN},     1,
     PANEL_ACTIVE_LOW, PANEL_DEBOUNCE_MS, 2, 0},

    {"SYS/STAB",        {SYS_STAB_SW_PIN},     1,
     PANEL_ACTIVE_LOW, PANEL_DEBOUNCE_MS, 2, 0},

    {"GUN/SW",          {GUN_SW_PIN},          1,
     PANEL_ACTIVE_LOW, PANEL_DEBOUNCE_MS, 2, 0},

    {"DISP/SETTINGS",   {DISP_SETTINGS_SW_PIN}, 1,
     PANEL_ACTIVE_LOW, PANEL_DEBOUNCE_MS, 2, 0}
};


/* ---------------- 档位旋钮：4 个 ----------------
 * line_count = 0 表示线数尚未确认，驱动会跳过该器件。
 * 确认后填实际线数，并把 mode 改成 ONEHOT / BINARY / GRAY。
 * 不要按编码器 A/B 相处理。
 */
static const PanelSelectorConfig s_selectors[] =
{
    {"TH/ZOOM",         TH_ZOOM_SEL_PINS, 4,
     PANEL_SELECTOR_ONEHOT, PANEL_ACTIVE_LOW, PANEL_DEBOUNCE_MS,
     1, 4, 0, s_zoom_labels},

    {"SYS/WIPER",       SYS_WIPER_SEL_PINS, 4,
     PANEL_SELECTOR_ONEHOT, PANEL_ACTIVE_LOW, PANEL_DEBOUNCE_MS,
     1, 4, 0, s_wiper_labels},

    {"GUN/AMMO",        GUN_AMMO_SEL_PINS, 8,
     PANEL_SELECTOR_ONEHOT, PANEL_ACTIVE_LOW, PANEL_DEBOUNCE_MS,
     1, 8, 0, s_ammo_labels},

    {"DISP/COLOUR",     DISP_COLOUR_SEL_PINS, 8,
     PANEL_SELECTOR_ONEHOT, PANEL_ACTIVE_LOW, PANEL_DEBOUNCE_MS,
     1, 8, 0, s_colour_labels}
};

/* ---------------- EC11 编码器：3 个 ---------------- */
static const PanelEncoderConfig s_encoders[] =
{
    {"TH/FOCUS",
     TH_FOCUS_A_PIN, TH_FOCUS_B_PIN, TH_FOCUS_S_PIN,
     PANEL_ACTIVE_LOW, PANEL_DEBOUNCE_MS, PANEL_LONG_PRESS_MS,
     PANEL_ENCODER_STEPS_PER_DETENT},

    {"DAY/ZOOM",
     DAY_ZOOM_A_PIN, DAY_ZOOM_B_PIN, DAY_ZOOM_S_PIN,
     PANEL_ACTIVE_LOW, PANEL_DEBOUNCE_MS, PANEL_LONG_PRESS_MS,
     PANEL_ENCODER_STEPS_PER_DETENT},

    {"DISP/BRIGHT",
     DISP_BRIGHT_A_PIN, DISP_BRIGHT_B_PIN, DISP_BRIGHT_S_PIN,
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

