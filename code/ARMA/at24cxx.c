#include "headfile.h"
#include <stdlib.h> // 用于 rand()
#include <string.h> // 用于 memcmp() 

u8 Buf_Free[20] = {0}; // 用于在EEPROM里保存、读取数据时的临时变量

#define IIC_SCL_H   gpio_bit_set(GPIOB, GPIO_PIN_8)
#define IIC_SCL_L   gpio_bit_reset(GPIOB, GPIO_PIN_8)
#define IIC_SDA_H   gpio_bit_set(GPIOB, GPIO_PIN_9)
#define IIC_SDA_L   gpio_bit_reset(GPIOB, GPIO_PIN_9)

#define READ_SDA    gpio_input_bit_get(GPIOB, GPIO_PIN_9)

/* AT24C512 硬件参数 */
#define AT24C512_PAGE_SIZE  128  // AT24C512 的一页是 128 字节 

static void AT24CXX_Init(void);
static void AT24CXX_Read(unsigned short ReadAddr, unsigned char *pBuffer, unsigned short NumToRead);
static void AT24CXX_Write(unsigned short WriteAddr, unsigned char *pBuffer, unsigned short NumToWrite);

extern unsigned char first_power_on;

static Struct_AT24CXX s_AT24CXX =
{
    .AT24CXXInit   = AT24CXX_Init,
    .AT24CXX_Read  = AT24CXX_Read,
    .AT24CXX_Write = AT24CXX_Write,
};
Struct_AT24CXX *ps_AT24CXX = &s_AT24CXX;

// 初始化IIC 
static void IIC_Init(void)
{
    rcu_periph_clock_enable(RCU_GPIOB);
    
    // 结合原理图：SCL(PB8) 和 SDA(PB9) 外部已有5.11K上拉电阻，
    // 因此内部不需要再开启上拉(设为 GPIO_PUPD_NONE)。
    // 必须保持开漏输出(GPIO_OTYPE_OD)模式，以配合外部上拉。
    gpio_mode_set(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_8 | GPIO_PIN_9); 
    gpio_output_options_set(GPIOB, GPIO_OTYPE_OD, GPIO_OSPEED_50MHZ, GPIO_PIN_8 | GPIO_PIN_9);
    
    // 初始状态释放总线
    IIC_SCL_H;
    IIC_SDA_H;
}

static void IIC_Start(void)
{
    IIC_SDA_H;
    IIC_SCL_H;
    ps_Systick->delay_us(4);
    IIC_SDA_L; // SCL高电平期间，SDA由高变低产生START
    ps_Systick->delay_us(4);
    IIC_SCL_L; // 钳住I2C总线，准备发送或接收数据
}

static void IIC_Stop(void)
{
    IIC_SCL_L;
    IIC_SDA_L; // STOP前确保SDA拉低
    ps_Systick->delay_us(4);
    IIC_SCL_H;
    ps_Systick->delay_us(4);
    IIC_SDA_H; // SCL高电平期间，SDA由低变高产生STOP
    ps_Systick->delay_us(4);
}

static unsigned char IIC_Wait_Ack(void)
{
    unsigned short ucErrTime = 0; // GD32主频很高，稍微加大超时计数器防误判

    IIC_SDA_H; // 主机释放SDA线，交给从机控制
    ps_Systick->delay_us(1);
    IIC_SCL_H;
    ps_Systick->delay_us(1);
    
    while(READ_SDA)
    {
        ucErrTime++;
        if(ucErrTime > 5000) 
        {
            IIC_Stop();
            return 1; // 接收ACK失败
        }
    }
    IIC_SCL_L; // 时钟拉低，完成一个ACK周期
    return 0;
}

static void IIC_Ack(void)
{
    IIC_SCL_L;
    IIC_SDA_L; // 主机拉低SDA，产生应答
    ps_Systick->delay_us(4);
    IIC_SCL_H;
    ps_Systick->delay_us(4);
    IIC_SCL_L;
}

static void IIC_NAck(void)
{
    IIC_SCL_L;
    IIC_SDA_H; // 主机释放SDA，产生非应答
    ps_Systick->delay_us(4);
    IIC_SCL_H;
    ps_Systick->delay_us(4);
    IIC_SCL_L;
}

static void IIC_Send_Byte(unsigned char txd)
{
    unsigned char t;
    IIC_SCL_L; // 拉低时钟开始数据传输
    for(t = 0; t < 8; t++)
    {
        if((txd & 0x80) >> 7)
            IIC_SDA_H;
        else
            IIC_SDA_L;
            
        txd <<= 1;
        ps_Systick->delay_us(4);
        IIC_SCL_H;
        ps_Systick->delay_us(4); 
        IIC_SCL_L;
        ps_Systick->delay_us(4);
    }
}

static unsigned char IIC_Read_Byte(unsigned char ack)
{
    unsigned char i, receive = 0;
    IIC_SDA_H; // 释放SDA总线，准备读取
    
    for(i = 0; i < 8; i++ )
    {
        IIC_SCL_L;
        ps_Systick->delay_us(4);
        IIC_SCL_H;
        receive <<= 1;
        if(READ_SDA) receive++;
        ps_Systick->delay_us(4);
    }
    
    if(!ack)
        IIC_NAck(); // 读完发NACK
    else
        IIC_Ack();  // 读完发ACK
        
    return receive;
}

// ---------------- 核心效率提升区域 ---------------- 

/**
 * @brief 等待EEPROM内部写周期完成 (ACK Polling)
 * 代替原本死板的 delay_ms(20)
 */
static void AT24CXX_WaitEepromReady(void)
{
    unsigned char ack_status = 1;
    unsigned short timeout = 0;
    
    while(ack_status && (timeout < 500)) 
    {
        IIC_Start();
        IIC_Send_Byte(0xA0); // 发送写设备地址
        ack_status = IIC_Wait_Ack();
        IIC_Stop();
        
        ps_Systick->delay_us(100);
        timeout++;
    }
}

/**
 * @brief 任意长度顺序读取 (Sequential Read)
 */
static void AT24CXX_Read(unsigned short ReadAddr, unsigned char *pBuffer, unsigned short NumToRead)
{
    if(NumToRead == 0) return;

    IIC_Start();
    IIC_Send_Byte(0xA0);            // 发送写命令(哑写，用于设置地址指针)
    IIC_Wait_Ack();
    IIC_Send_Byte(ReadAddr >> 8);   // 发送高地址
    IIC_Wait_Ack();
    IIC_Send_Byte(ReadAddr & 0xFF); // 发送低地址
    IIC_Wait_Ack();

    IIC_Start();
    IIC_Send_Byte(0xA1);            // 重新START，发送读命令 
    IIC_Wait_Ack();

    while(NumToRead)
    {
        if(NumToRead == 1)
        {
            *pBuffer = IIC_Read_Byte(0); // 最后一个字节发送NACK
        }
        else
        {
            *pBuffer = IIC_Read_Byte(1); // 前面的字节全部发送ACK，EEPROM会自动地址递增
        }
        pBuffer++;
        NumToRead--;
    }
    IIC_Stop(); // 产生停止条件
}

/**
 * @brief 单页内连续写入
 */
static void AT24CXX_WritePage(unsigned short WriteAddr, unsigned char *pBuffer, unsigned short NumToWrite)
{
    unsigned short i;
    
    IIC_Start();
    IIC_Send_Byte(0xA0);             // 发送写命令
    IIC_Wait_Ack();
    IIC_Send_Byte(WriteAddr >> 8);   // 发送高地址
    IIC_Wait_Ack();
    IIC_Send_Byte(WriteAddr & 0xFF); // 发送低地址
    IIC_Wait_Ack();

    for(i = 0; i < NumToWrite; i++)
    {
        IIC_Send_Byte(pBuffer[i]);   // 连续写入该页数据
        IIC_Wait_Ack();
    }
    
    IIC_Stop();                      // 产生停止条件，EEPROM开始将缓存搬移进非易失存储器
    AT24CXX_WaitEepromReady();       // 等待硬件烧录完成
}

/**
 * @brief 任意长度连续写入 (自动处理跨页拆包)
 */
static void AT24CXX_Write(unsigned short WriteAddr, unsigned char *pBuffer, unsigned short NumToWrite)
{
    unsigned short page_offset;
    unsigned short space_in_page;
    unsigned short write_len;

    while(NumToWrite > 0)
    {
        page_offset = WriteAddr % AT24C512_PAGE_SIZE;      // 当前地址在页内的偏移
        space_in_page = AT24C512_PAGE_SIZE - page_offset;  // 当前页剩余空间

        if(NumToWrite <= space_in_page)
        {
            write_len = NumToWrite; // 剩下的数据够在一页内写完
        }
        else
        {
            write_len = space_in_page; // 剩下的数据写不完，把当前页填满即可
        }

        AT24CXX_WritePage(WriteAddr, pBuffer, write_len);

        WriteAddr += write_len;
        pBuffer += write_len;
        NumToWrite -= write_len;
    }
}

// ---------------- 系统业务代码 ---------------- 

static unsigned char AT24CXX_Check(void)
{
    unsigned char temp, retry_cnt = 0;
    
    // 借用优化后的读写函数读写标志位
    AT24CXX_Read(SYSTEM_POWERON_FRIST, &temp, 1); 

    if(temp == 0x55)
    {
        return 0;
    }
    else // 排除第一次初始化的情况
    {
        do {
            retry_cnt++;
            temp = 0x55;
            AT24CXX_Write(SYSTEM_POWERON_FRIST, &temp, 1);
            
            temp = 0; // 清空准备回读验证
            AT24CXX_Read(SYSTEM_POWERON_FRIST, &temp, 1);
        } while ((temp != 0x55) && (retry_cnt < 5));
    }
    //首次上电需要初始化一些变量
    return 1;
}

static void AT24CXX_Init(void)
{
    IIC_Init();
    first_power_on = AT24CXX_Check();
}


/**
 * @brief AT24C512 稳健性压力测试函数
 * @param iterations 测试的总轮数
 * @return AT24_TestResult_t 包含测试统计结果的结构体
 */
AT24_TestResult_t AT24CXX_Robustness_Test(uint32_t iterations) {
    AT24_TestResult_t result = {0, 0, 0};
    result.total_tests = iterations;

    uint16_t random_addr;
    uint16_t random_len;
    uint8_t *test_write_buf;
    uint8_t *test_read_buf;

    // 最大单次测试长度设为 512 字节（涵盖多页写入）
    const uint16_t MAX_TEST_LEN = 512;
    test_write_buf = (uint8_t *)malloc(MAX_TEST_LEN);
    test_read_buf = (uint8_t *)malloc(MAX_TEST_LEN);

    if (test_write_buf == NULL || test_read_buf == NULL) {
        // 内存分配失败
        return result;
    }

    for (uint32_t i = 0; i < iterations; i++) {
        // 1. 生成随机长度 (1 到 MAX_TEST_LEN 字节)
        random_len = (rand() % MAX_TEST_LEN) + 1;

        // 2. 生成随机地址 (确保 addr + len 不超过芯片容量 65536)
        // AT24C512 地址范围 0x0000 - 0xFFFF
        random_addr = rand() % (65536 - random_len);

        // 3. 填充随机数据
        for (uint16_t j = 0; j < random_len; j++) {
            test_write_buf[j] = (uint8_t)(rand() % 256);
        }

        // 4. 执行写入过程
        ps_AT24CXX->AT24CXX_Write(random_addr, test_write_buf, random_len);

        // 5. 立即读出校验
        memset(test_read_buf, 0, random_len); // 清空接收缓存
        ps_AT24CXX->AT24CXX_Read(random_addr, test_read_buf, random_len);

        // 6. 比较数据
        if (memcmp(test_write_buf, test_read_buf, random_len) == 0) {
            result.success_count++;
        } else {
            result.failure_count++;
            // 这里可以添加断点或打印错误信息，记录失败的地址和长度
        }
    }

    free(test_write_buf);
    free(test_read_buf);
    return result;
}
