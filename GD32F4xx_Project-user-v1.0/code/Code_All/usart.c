#include "headfile.h"



static void UartInit(void);
static void Uart3Init(unsigned int uibaudrate);
static void Uart3_SendData(unsigned char *pdata, unsigned short usdatalen);


static Struct_Usart s_Usart =
{
    .UartInit         = UartInit,
    .Uart3_SendData   = Uart3_SendData,
};
Struct_Usart *ps_Usart = &s_Usart;

static void UartInit(void)
{
    Uart3Init(115200);
    

} 

static void Uart3Init(unsigned int uibaudrate)
{
    usart_deinit(UART3);
    usart_disable(UART3);
    rcu_periph_clock_enable(RCU_GPIOC);
    rcu_periph_clock_enable(RCU_UART3);

    /* UART3: TX = PC10, RX = PC11, AF8 */
    gpio_af_set(GPIOC, GPIO_AF_8, GPIO_PIN_10 | GPIO_PIN_11);
    gpio_mode_set(GPIOC, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_10 | GPIO_PIN_11);
    gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_10 | GPIO_PIN_11);
    
    usart_baudrate_set(UART3, uibaudrate);
    usart_parity_config(UART3, USART_PM_NONE);
    usart_word_length_set(UART3, USART_WL_8BIT);
    usart_stop_bit_set(UART3, USART_STB_1BIT);
    usart_hardware_flow_coherence_config(UART3, USART_HCM_NONE);
    usart_transmit_config(UART3, USART_TRANSMIT_ENABLE);
    usart_receive_config(UART3, USART_RECEIVE_ENABLE);
    usart_data_first_config(UART3, USART_MSBF_LSB);
    usart_enable(UART3);
	
	nvic_irq_enable(UART3_IRQn, 5, 0);
    usart_interrupt_enable(UART3, USART_INT_RBNE);
}

static void Uart3_SendData(unsigned char *pdata, unsigned short usdatalen)
{
    unsigned short i;

    if (pdata == 0)
    {
        return;
    }

    for (i = 0u; i < usdatalen; i++)
    {
        uint32_t timeout = 0x1FFFFu;

        usart_data_transmit(UART3, (uint16_t)pdata[i]);

        /* 带超时等待，避免串口异常时卡死整个主循环 */
        while (usart_flag_get(UART3, USART_FLAG_TBE) == RESET)
        {
            if (timeout == 0u)
            {
                break;
            }
            timeout--;
        }
    }
}


void UART3_IRQHandler(void)
{
    uint8_t data;

    /*
     * 正常接收数据：
     * 读取数据寄存器后，RBNE 标志会被清除。
     */
    if (RESET != usart_interrupt_flag_get(UART3, USART_INT_FLAG_RBNE))
    {
        data = (uint8_t)usart_data_receive(UART3);

        IAP_UartRxPush(data);
    }
    /*
     * 接收错误：
     * 读取数据寄存器，清除接收错误状态。
     */
    else if (RESET != usart_interrupt_flag_get(UART3, USART_INT_FLAG_RBNE_ORERR))
    {
        (void)usart_data_receive(UART3);
    }
        
}