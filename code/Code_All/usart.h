#ifndef __USART_H__
#define __USART_H__

#include "gd32f4xx.h"

/* ==========================================================================
 * 串口驱动（沿用旧项目方案）
 *
 * 对外只提供：
 *   UartInit()       初始化全部串口
 *   Uart4_SendData() 按字节阻塞发送
 *
 * 接收不使用轮询 ReadByte，而是在 UART4_IRQHandler 里把字节压入环形队列。
 *
 * ========================================================================== */

typedef struct
{
    void (*UartInit)(void);
    void (*Uart3_SendData)(unsigned char *pdata, unsigned short usdatalen);
} Struct_Usart;

extern Struct_Usart *ps_Usart;

#endif
