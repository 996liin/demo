
#include "headfile.h"
#include "iap_flag.h"

/*
 * 读取升级标志
 * 返回 EEPROM 中 0xFFFF 地址的内容
 */
uint8_t IAPFlag_Read(void)
{
    uint8_t flag = 0xFF;

    ps_AT24CXX->AT24CXX_Read(
        IAP_FLAG_ADDR,
        &flag,
        1
    );

    return flag;
}

/*
 * 写入升级标志，并读回验证
 * 成功返回 1，失败返回 0
 */
uint8_t IAPFlag_Write(uint8_t flag)
{
    uint8_t read_back = 0xFF;

    ps_AT24CXX->AT24CXX_Write(
        IAP_FLAG_ADDR,
        &flag,
        1
    );

    /* 读回验证 */
    ps_AT24CXX->AT24CXX_Read(
        IAP_FLAG_ADDR,
        &read_back,
        1
    );

    if (read_back == flag)
    {
        return 1;
    }

    return 0;
}

/*
 * APP 请求进入升级模式
 */
uint8_t IAPFlag_RequestUpdate(void)
{
    return IAPFlag_Write(IAP_FLAG_REQUEST);
}

/*
 * 清除升级标志
 */
uint8_t IAPFlag_Clear(void)
{
    return IAPFlag_Write(IAP_FLAG_IDLE);
}
