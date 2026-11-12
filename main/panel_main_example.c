/*
 * 面板输入检测独立运行示例
 *
 * 重要：
 * 本文件不要和现有 main.c 同时加入同一个 Keil Target，否则会出现两个 main。
 * 推荐做法（任选其一）：
 *   1. 新建一个 Keil Target（例如 GD32F470_Panel），只加入
 *      startup + system + Firmware + PanelInput + 本文件；
 *   2. 或在现有 Target 中暂时把 main.c 设为 Exclude from Build，
 *      再把本文件加入 main 组。
 *
 * 面板调试阶段不要调用 ps_Usart->UartInit()：
 * 它会占用 PE7/PE8(UART6)、PB10/PB11(USART2)、PA2/PA3(USART1)、
 * PA9/PA10(USART0)、PC6/PC7(USART5) 等引脚，
 * 而这些引脚在面板板上正好是按键/编码器网络。
 */

#include "gd32f4xx.h"
#include "systick.h"
#include "panel_config.h"
#include "panel_input.h"

int main(void)
{
    /* 如果板子使用 0x8000 偏移的 Bootloader，保留下面一行；
     * 如果是直接下载运行，改为 nvic_vector_table_set(NVIC_VECTTAB_FLASH, 0x0);
     */
    nvic_vector_table_set(NVIC_VECTTAB_FLASH, 0x8000);
    nvic_priority_group_set(NVIC_PRIGROUP_PRE0_SUB4);

    /* 仅用于主循环节拍和串口打印前的延时，不参与面板扫描计时 */
    ps_Systick->SystickInit(240);

    PanelInput_Init();

    while (1)
    {
        PanelInput_Scan();      /* 必须每 PANEL_SCAN_PERIOD_MS 调用一次 */
        PanelInput_Process();   /* 分发事件并打印串口调试信息 */

        ps_Systick->delay_ms(PANEL_SCAN_PERIOD_MS);
    }
}
