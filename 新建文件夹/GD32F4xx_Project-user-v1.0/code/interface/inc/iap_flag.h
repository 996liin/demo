#ifndef __IAP_FLAG_H__
#define __IAP_FLAG_H__

#include <stdint.h>

/* AT24C512 Éý¼¶±êÖ¾ */
#define IAP_FLAG_ADDR       0xFFFFu
#define IAP_FLAG_REQUEST    0xA5u
#define IAP_FLAG_IDLE       0x00u

uint8_t IAPFlag_Read(void);
uint8_t IAPFlag_Write(uint8_t flag);

uint8_t IAPFlag_RequestUpdate(void);
uint8_t IAPFlag_Clear(void);

#endif
