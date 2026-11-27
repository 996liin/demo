#ifndef __IAP_APP_H__
#define __IAP_APP_H__

#include <stdint.h>

/* APP 端升级触发模块 */
void IAP_AppInit(void);

/* 在主循环中调用，负责解析接收到的 Pelco-D 数据 */
void IAP_AppProcess(void);

/* 在 UART3 接收中断中调用，将收到的字节放入接收缓冲区 */
void IAP_UartRxPush(uint8_t data);

#endif