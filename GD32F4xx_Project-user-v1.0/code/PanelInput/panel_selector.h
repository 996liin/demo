#ifndef __PANEL_SELECTOR_H__
#define __PANEL_SELECTOR_H__

#include "panel_types.h"
#include "panel_port.h"

/* ==========================================================================
 * 多档位旋转选择开关驱动
 *
 * 旋钮只停在固定档位（如 1~8 档），不是增量编码器，不能用 A/B 相计数。
 * 电路结构尚未确认，因此提供三种可选译码方式：
 *   - 独热码：每档一根信号线，只有当前档位那根有效
 *   - 二进制：N 根线组成二进制编码
 *   - 格雷码：N 根线组成格雷码
 * 也可以用 position_map 自定义任意编码。
 *
 * position_labels 只影响串口打印，例如 ZOOM 的 "1X"/"2X"/"4X"/"8X"。
 * ========================================================================== */

#define PANEL_SELECTOR_MAX_LINES  8u

typedef enum
{
    PANEL_SELECTOR_ONEHOT = 0,  /* 独热码：1 根 1 档 */
    PANEL_SELECTOR_BINARY,      /* 二进制编码 */
    PANEL_SELECTOR_GRAY         /* 格雷码 */
} PanelSelectorMode;

typedef struct
{
    const char *name;                               /* 例如 "THERMAL CAMERA/ZOOM" */
    PanelPortPin pins[PANEL_SELECTOR_MAX_LINES];    /* 档位信号线 */
    uint8_t line_count;                             /* 实际线数；0 表示尚未确认 */
    PanelSelectorMode mode;                         /* 译码方式（待实测确认） */
    uint8_t active_level;                           /* 每根线的有效电平 */
    uint16_t debounce_ms;                           /* 整组状态的消抖时间 */
    uint8_t position_offset;                        /* 档位偏移，1 表示档位从 1 开始 */
    uint8_t position_count;                         /* 总档位数，例如 4 或 8 */
    const uint8_t *position_map;                    /* 可选自定义译码表，0xFF 表示无效 */
    const char *const *position_labels;             /* 可选档位显示名，第 1 档对应 [0] */
} PanelSelectorConfig;

typedef struct
{
    const PanelSelectorConfig *cfg;
    uint8_t  id;
    uint8_t  initialized;
    uint8_t  stable_pattern;
    uint8_t  candidate_pattern;
    uint8_t  stable_position;   /* 当前稳定档位，0 表示未知 */
    uint16_t debounce_cnt;
} PanelSelector;

void PanelSelector_Init(PanelSelector *sel, const PanelSelectorConfig *cfg, uint8_t id);
void PanelSelector_Scan(PanelSelector *sel, uint32_t now_ms, PanelEventSink sink);
uint8_t PanelSelector_GetPosition(const PanelSelector *sel);

#endif /* __PANEL_SELECTOR_H__ */
