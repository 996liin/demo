#include "headfile.h"
#include "iap_app.h"
#include "iap_flag.h"

/* ==================== 配置参数 ==================== */

#define IAP_RX_BUF_SIZE         128u
#define PELCOD_FRAME_SIZE       7u

#define PELCOD_FRAME_HEAD       0xFFu
#define PELCOD_CMD1_UPDATE      0x6Eu
#define PELCOD_CMD2_UPDATE      0x01u

/* ==================== 接收缓冲区 ==================== */

/*
 * UART 中断负责写入数据；
 * APP 主循环负责读取数据。
 */
static volatile uint8_t s_rx_buf[IAP_RX_BUF_SIZE];

static volatile uint16_t s_rx_head = 0;
static volatile uint16_t s_rx_tail = 0;

/* Pelco-D 接收帧 */
static uint8_t s_frame[PELCOD_FRAME_SIZE];
static uint8_t s_frame_count = 0;

/* ==================== 函数声明 ==================== */

static uint8_t IAP_RxPop(uint8_t *data);
static void IAP_ParseByte(uint8_t data);
static void IAP_EnterBootloader(void);

/* ==================== 初始化 ==================== */

void IAP_AppInit(void)
{
    uint16_t i;

    s_rx_head = 0;
    s_rx_tail = 0;
    s_frame_count = 0;

    for (i = 0; i < PELCOD_FRAME_SIZE; i++)
    {
        s_frame[i] = 0;
    }
}

/* ==================== UART 接收数据入队 ==================== */

/*
 * 由 UART3 接收中断调用。
 *
 * 这里只保存收到的字节，不在中断中解析协议，
 * 避免在中断中执行 EEPROM 写入和软件复位。
 */
void IAP_UartRxPush(uint8_t data)
{
    uint16_t next;

    next = s_rx_tail + 1u;

    if (next >= IAP_RX_BUF_SIZE)
    {
        next = 0;
    }

    /* 缓冲区满时丢弃新数据 */
    if (next != s_rx_head)
    {
        s_rx_buf[s_rx_tail] = data;
        s_rx_tail = next;
    }
}

/* ==================== 接收数据出队 ==================== */

static uint8_t IAP_RxPop(uint8_t *data)
{
    if (s_rx_head == s_rx_tail)
    {
        return 0;
    }

    *data = s_rx_buf[s_rx_head];

    s_rx_head++;

    if (s_rx_head >= IAP_RX_BUF_SIZE)
    {
        s_rx_head = 0;
    }

    return 1;
}

/* ==================== 进入 BOOT ==================== */

static void IAP_EnterBootloader(void)
{
    /*
     * 写入 EEPROM 升级标志：
     *
     * 地址：0xFFFF
     * 数据：0xA5
     *
     * IAPFlag_RequestUpdate() 内部会回读验证。
     */

    if (IAPFlag_RequestUpdate())
    {
        /*
         * EEPROM 写入成功。
         * 等待 100ms 后软件复位。
         */
        ps_Systick->delay_ms(100);

        NVIC_SystemReset();
    }

    /*
     * 如果 EEPROM 写入失败：
     * 不执行复位，继续运行 APP。
     * 上位机可以重新发送升级命令。
     */
}

/* ==================== Pelco-D 数据解析 ==================== */

/*
 * Pelco-D 标准帧：
 *
 * Byte0：FF      帧头
 * Byte1：ADDR    设备地址
 * Byte2：CMD1
 * Byte3：CMD2
 * Byte4：DATA1
 * Byte5：DATA2
 * Byte6：CHECKSUM
 *
 * 校验和：
 * (ADDR + CMD1 + CMD2 + DATA1 + DATA2) & 0xFF
 *
 * 升级命令：
 * CMD1 = 0x6E
 * CMD2 = 0x01
 */
static void IAP_ParseByte(uint8_t data)
{
    uint16_t checksum = 0;
    uint8_t i;
    uint8_t next_start;

    /* 等待帧头 */
    if (s_frame_count == 0)
    {
        if (data == PELCOD_FRAME_HEAD)
        {
            s_frame[0] = data;
            s_frame_count = 1;
        }

        return;
    }

    /* 接收后续字节 */
    s_frame[s_frame_count] = data;
    s_frame_count++;

    /* 未接收完整帧 */
    if (s_frame_count < PELCOD_FRAME_SIZE)
    {
        return;
    }

    /* 计算 Pelco-D 校验和 */
    for (i = 1; i <= 5; i++)
    {
        checksum += s_frame[i];
    }

    checksum &= 0xFFu;

    /* 校验通过，判断是否为升级命令 */
    if ((uint8_t)checksum == s_frame[6])
    {
        if ((s_frame[2] == PELCOD_CMD1_UPDATE) &&
            (s_frame[3] == PELCOD_CMD2_UPDATE))
        {
            IAP_EnterBootloader();
        }
    }

    /*
     * 处理帧结束后的数据。
     *
     * 如果当前帧内部存在下一个 0xFF，
     * 将它作为下一帧的帧头，避免丢失连续帧。
     */
    next_start = 0;

    for (i = 1; i < PELCOD_FRAME_SIZE; i++)
    {
        if (s_frame[i] == PELCOD_FRAME_HEAD)
        {
            next_start = i;
            break;
        }
    }

    if (next_start != 0)
    {
        uint8_t j;
        uint8_t remain;

        remain = PELCOD_FRAME_SIZE - next_start;

        for (j = 0; j < remain; j++)
        {
            s_frame[j] = s_frame[next_start + j];
        }

        s_frame_count = remain;
    }
    else
    {
        s_frame_count = 0;
    }
}

/* ==================== APP 升级触发处理 ==================== */

void IAP_AppProcess(void)
{
    uint8_t data;

    while (IAP_RxPop(&data))
    {
        IAP_ParseByte(data);
    }
}