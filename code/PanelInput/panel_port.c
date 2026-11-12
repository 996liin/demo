#include "panel_port.h"
#include "panel_config.h"
#include "gd32f4xx.h"

/* ==========================================================================
 * 面板输入硬件端口实现（GD32F470 + GD32F4xx 标准外设库）
 *
 * 如果后续更换调试串口或 GPIO 操作方式，只需要修改本文件。
 * ========================================================================== */

uint8_t PanelPort_PinIsValid(const PanelPortPin *pin)
{
    if ((pin == 0) || (pin->port == 0u) || (pin->pin == 0u))
    {
        return 0u;
    }

    return 1u;
}

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

uint8_t PanelPort_ReadPin(const PanelPortPin *pin)
{
    if (PanelPort_PinIsValid(pin) == 0u)
    {
        /* 未分配引脚按“高电平/未按下”处理，避免误报 */
        return 1u;
    }

    return (gpio_input_bit_get(pin->port, pin->pin) == SET) ? 1u : 0u;
}

void PanelPort_DebugInit(void)
{
#if (PANEL_DEBUG_ENABLE != 0)
    rcu_periph_clock_enable(RCU_GPIOC);
    rcu_periph_clock_enable(RCU_GPIOD);
    rcu_periph_clock_enable(RCU_UART4);

    /* UART4: TX = PC12, RX = PD2, AF8
     * 该组引脚不与面板按键/编码器冲突，作为本框架的调试口。
     */
    gpio_af_set(GPIOC, GPIO_AF_8, GPIO_PIN_12);
    gpio_af_set(GPIOD, GPIO_AF_8, GPIO_PIN_2);

    gpio_mode_set(GPIOC, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_12);
    gpio_mode_set(GPIOD, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_2);

    gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_12);
    gpio_output_options_set(GPIOD, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_2);

    usart_deinit(PANEL_DEBUG_UART);
    usart_baudrate_set(PANEL_DEBUG_UART, PANEL_DEBUG_BAUDRATE);
    usart_parity_config(PANEL_DEBUG_UART, USART_PM_NONE);
    usart_word_length_set(PANEL_DEBUG_UART, USART_WL_8BIT);
    usart_stop_bit_set(PANEL_DEBUG_UART, USART_STB_1BIT);
    usart_hardware_flow_coherence_config(PANEL_DEBUG_UART, USART_HCM_NONE);
    usart_transmit_config(PANEL_DEBUG_UART, USART_TRANSMIT_ENABLE);
    usart_enable(PANEL_DEBUG_UART);
#endif
}

void PanelPort_DebugWrite(const char *str)
{
#if (PANEL_DEBUG_ENABLE != 0)
    if (str == 0)
    {
        return;
    }

    while (*str != '\0')
    {
        uint32_t timeout = 0x1FFFFu;

        usart_data_transmit(PANEL_DEBUG_UART, (uint8_t)(*str));
        str++;

        /* 带超时的发送等待，避免串口异常时卡死整个扫描循环 */
        while (usart_flag_get(PANEL_DEBUG_UART, USART_FLAG_TBE) == RESET)
        {
            if (timeout == 0u)
            {
                break;
            }
            timeout--;
        }
    }
#endif
}
