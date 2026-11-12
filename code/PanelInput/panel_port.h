#ifndef __PANEL_PORT_H__
#define __PANEL_PORT_H__

#include <stdint.h>

/* ==========================================================================
 * 硬件端口抽象层（BSP）
 *
 * 驱动层只通过本文件访问 GPIO 和调试串口。
 * 如果以后更换 MCU、改用其他调试串口，只需要修改 panel_port.c。
 * ========================================================================== */

/* 一个 MCU 输入引脚的三要素 */
typedef struct
{
    uint32_t port;  /* GPIOA ... GPIOE */
    uint32_t pin;   /* GPIO_PIN_x */
    uint32_t rcu;   /* RCU_GPIOx */
} PanelPortPin;

/* 未分配引脚占位，配置表中使用它表示“该设备暂未接线” */
#define PANEL_PIN_NONE  {0u, 0u, 0u}

typedef enum
{
    PANEL_PULL_NONE = 0,
    PANEL_PULL_UP,
    PANEL_PULL_DOWN
} PanelPull;

/* 引脚是否已经分配 */
uint8_t PanelPort_PinIsValid(const PanelPortPin *pin);

/* 把引脚配置为输入，并按需打开内部上拉/下拉。
 * 注意：本板按键/编码器外部已有 5.11K 上拉，内部上拉只是冗余保护。
 */
void PanelPort_InitInput(const PanelPortPin *pin, PanelPull pull);

/* 读取引脚原始电平，返回 0 或 1 */
uint8_t PanelPort_ReadPin(const PanelPortPin *pin);

/* 调试串口初始化与输出（默认 UART4，115200-8-N-1，见 panel_config.h） */
void PanelPort_DebugInit(void);
void PanelPort_DebugWrite(const char *str);

#endif /* __PANEL_PORT_H__ */
