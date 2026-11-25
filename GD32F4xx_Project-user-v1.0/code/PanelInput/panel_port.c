#include "panel_port.h"
#include "panel_config.h"
#include "gd32f4xx.h"
#include "usart.h"

/* ==========================================================================
 * 面板输入硬件端口实现（GD32F470 + GD32F4xx 标准外设库）
 *
 * 如果后续更换调试串口或 GPIO 操作方式，只需要修改本文件。
 * ========================================================================== */

 /* 判断引脚配置是否有效：PANEL_PIN_NONE（port/pin 都为 0）视为未接线。 */
uint8_t PanelPort_PinIsValid(const PanelPortPin *pin)
{
    if ((pin == 0) || (pin->port == 0u) || (pin->pin == 0u))
    {
        return 0u;
    }

    return 1u;
}

/* 把一只脚配置为 GPIO 输入：
 * 打开对应端口时钟，按 pull 参数选择内部上拉/下拉/浮空；无效引脚直接返回。 */
void PanelPort_InitInput(const PanelPortPin *pin, PanelPull pull)
{
    uint32_t pupd;

    if (PanelPort_PinIsValid(pin) == 0u)
    {
        return;
    }

    rcu_periph_clock_enable((rcu_periph_enum)pin->rcu);

    switch (pull)
    {
        case PANEL_PULL_UP:
            pupd = GPIO_PUPD_PULLUP;
            break;

        case PANEL_PULL_DOWN:
            pupd = GPIO_PUPD_PULLDOWN;
            break;

        case PANEL_PULL_NONE:
        default:
            pupd = GPIO_PUPD_NONE;
            break;
    }

    gpio_mode_set(pin->port, GPIO_MODE_INPUT, pupd, pin->pin);
}

/* 读取引脚原始电平，返回 0 或 1；无效引脚返回 1（当作未按下/未接线的安全值）。 */
uint8_t PanelPort_ReadPin(const PanelPortPin *pin)
{
    if (PanelPort_PinIsValid(pin) == 0u)
    {
        /* 未分配引脚按“高电平/未按下”处理，避免误报 */
        return 1u;
    }

    return (gpio_input_bit_get(pin->port, pin->pin) == SET) ? 1u : 0u;
}

/* 向调试串口发送字符串；PANEL_DEBUG_ENABLE = 0 或 str 为空时直接返回。 */
void PanelPort_DebugWrite(const char *str)
{
#if (PANEL_DEBUG_ENABLE != 0)
    unsigned short len = 0;
    if (str == 0)
    {
        return;
    }
    while (str[len] != '\0')
    {
        len++;
    }

    ps_Usart->Uart3_SendData((unsigned char *)str, len);
#endif
}
