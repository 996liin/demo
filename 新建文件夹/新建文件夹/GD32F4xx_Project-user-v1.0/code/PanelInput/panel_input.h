#ifndef __PANEL_INPUT_H__
#define __PANEL_INPUT_H__

#include "panel_types.h"

/* ==========================================================================
 * 面板输入统一管理
 *
 * 典型用法：
 *     PanelInput_Init();
 *     while (1)
 *     {
 *         PanelInput_Scan();      // 每 PANEL_SCAN_PERIOD_MS 调用一次
 *         PanelInput_Process();   // 分发事件并打印调试信息
 *     }
 *
 * 如果业务层希望自己轮询事件，可以只调用 PanelInput_Scan()，
 * 再用 PanelInput_PollEvent() 取事件；此时不要再调用 PanelInput_Process()，
 * 否则事件会被 Process 先取走。
 * ========================================================================== */

typedef void (*PanelEventHandler)(const PanelEvent *evt);

/* 初始化所有输入设备、GPIO、调试串口和事件队列 */
void PanelInput_Init(void);

/* 周期扫描。必须每 PANEL_SCAN_PERIOD_MS 毫秒调用一次。 */
void PanelInput_Scan(void);

/* 事件分发：取出队列中的事件，打印调试信息，并调用已注册的回调 */
void PanelInput_Process(void);

/* 轮询取事件。返回 1 表示取到事件。 */
uint8_t PanelInput_PollEvent(PanelEvent *evt);

/* 注册统一事件回调，后续业务功能可以挂在这里 */
void PanelInput_SetEventHandler(PanelEventHandler handler);

/* 面板软件时间戳(ms)，用于业务层判断按键时长等 */
uint32_t PanelInput_GetTickMs(void);

/* 读取当前稳定状态 */
uint8_t PanelInput_GetKeyState(uint8_t id);
uint8_t PanelInput_GetSwitchState(uint8_t id);
uint8_t PanelInput_GetSelectorPosition(uint8_t id);
uint8_t PanelInput_GetEncoderButtonState(uint8_t id);

#endif /* __PANEL_INPUT_H__ */
