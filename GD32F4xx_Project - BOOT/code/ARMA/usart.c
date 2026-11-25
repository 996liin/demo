#include "headfile.h"

/*	
	USART1	J12+J33		:	232/TTL���Ե�������;ͨ���̵������л����Զ��۽���
	USART22	J20+J15		:	422/485���Ե������֣�Ӳ��ֻ���˸��õ�����
	USART3	J6/J6/J16	:	232/TTL/485���Ե������֣�Ӳ��ֻ����δ���õ�����
	UART4	J14			:	232
	UART5	J33			:	485���õ��貢���˴�оƬusart12�Լ��Զ��۽���485��
	USART6	J17			:	422��Ӳ��ֻ����δ���õ�����
	UART7	J15			:	232
	UART8	J7			:	422
*/

static void uartInit(void);
static void Usart0_Init(unsigned int uibaudrate);
static void Usart0_SendData(unsigned char *pdata,unsigned short usdatalen);
static void Usart1_Init(unsigned int uibaudrate);
static void Usart1_SendData(unsigned char *pdata,unsigned short usdatalen);
static void Usart2_Init(unsigned int uibaudrate);
static void Usart2_SendData(unsigned char *pdata,unsigned short usdatalen);
static void Usart3_Init(unsigned int uibaudrate);
static void Usart3_SendData(unsigned char *pdata,unsigned short usdatalen);
static void Uart4_Init(unsigned int uibaudrate);
static void Uart4_SendData(unsigned char *pdata,unsigned short usdatalen);
static void Uart5_Init(unsigned int uibaudrate);
static void Uart5_SendData(unsigned char *pdata,unsigned short usdatalen);
static void Uart6Init(unsigned int uibaudrate);//update
static void Uart6_SendData(unsigned char *pdata,unsigned short usdatalen);

static Struct_Usart s_Usart = 
{
	.uartInit = uartInit,
	.dubugUsart0SendData  = Usart0_SendData,
	.cam232Usart1SendData = Usart1_SendData,
	.wb422Usart2SendData = Usart2_SendData,
	.af232Usart3SendData = Usart3_SendData,
	.ir232Uart4SendData = Uart4_SendData,
	.cam485Uart5SendData = Uart5_SendData,
	.Uart6_SendData = Uart6_SendData,
	
};
Struct_Usart *ps_Usart = &s_Usart;	

static void uartInit(void)
{
//	unsigned int uibaudrate[5] = {9600,9600,9600,9600,9600};//,uibaudrateRead[5] = {0,};
	//Usart0_Init(115200);
	//Usart1_Init(115200);
	//Usart2_Init(9600);
	Usart3_Init(115200);
	//Uart4_Init(9600);
	//Uart5_Init(9600);
	//Uart6Init(115200);

}
static void Usart0_Init(unsigned int uibaudrate)
{
	  usart_deinit(USART0);
		usart_disable(USART0);
		rcu_periph_clock_enable(RCU_GPIOA);
		rcu_periph_clock_enable(RCU_USART0);
		gpio_af_set(GPIOA,GPIO_AF_7,GPIO_PIN_9|GPIO_PIN_10);
		gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_9|GPIO_PIN_10);
		gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,GPIO_PIN_9|GPIO_PIN_10);
		nvic_irq_enable(USART0_IRQn,5,0);

		usart_interrupt_enable(USART0,USART_INT_RBNE);
		usart_baudrate_set(USART0,uibaudrate);
		usart_parity_config(USART0,USART_PM_NONE);
		usart_word_length_set(USART0,USART_WL_8BIT);
		usart_stop_bit_set(USART0,USART_STB_1BIT);
		usart_hardware_flow_coherence_config(USART0,USART_HCM_NONE);
		
		usart_transmit_config(USART0,USART_TRANSMIT_ENABLE);
		usart_receive_config(USART0,USART_RECEIVE_ENABLE);
		
		usart_data_first_config(USART0,USART_MSBF_LSB);
		usart_enable(USART0);
}

static void Usart0_SendData(unsigned char *pdata,unsigned short usdatalen)
{
	for(uint8_t i = 0; i < usdatalen; i++)
	{
		usart_data_transmit(USART0,*(pdata+i));
		while(!usart_flag_get(USART0,USART_FLAG_TBE));
	}
}
void USART0_IRQHandler(void)
{
	if(RESET != usart_interrupt_flag_get(USART0,USART_INT_FLAG_RBNE_ORERR))
	{
		usart_data_receive(USART0);
	}
	if(RESET != usart_interrupt_flag_get(USART0,USART_INT_FLAG_RBNE))
	{
		ps_queue->uartQueuePush(0,usart_data_receive(USART0));
		usart_interrupt_flag_clear(USART0,USART_INT_FLAG_RBNE);
	}
}

/*		Usart1		*/
static void Usart1_Init(unsigned int uibaudrate)
{
  usart_deinit(USART1);
		usart_disable(USART1);
		rcu_periph_clock_enable(RCU_GPIOA);
		rcu_periph_clock_enable(RCU_USART1);
		gpio_af_set(GPIOA,GPIO_AF_7,GPIO_PIN_2|GPIO_PIN_3);
		gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_2|GPIO_PIN_3);
		gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,GPIO_PIN_2|GPIO_PIN_3);
		nvic_irq_enable(USART1_IRQn,5,0);

		usart_interrupt_enable(USART1,USART_INT_RBNE);
		usart_baudrate_set(USART1,uibaudrate);
		usart_parity_config(USART1,USART_PM_NONE);
		usart_word_length_set(USART1,USART_WL_8BIT);
		usart_stop_bit_set(USART1,USART_STB_1BIT);
		usart_hardware_flow_coherence_config(USART1,USART_HCM_NONE);
		
		usart_transmit_config(USART1,USART_TRANSMIT_ENABLE);
		usart_receive_config(USART1,USART_RECEIVE_ENABLE);
		
		usart_data_first_config(USART1,USART_MSBF_LSB);
		usart_enable(USART1);
	
}

static void Usart1_SendData(unsigned char *pdata,unsigned short usdatalen)
{
	for(uint8_t i = 0; i < usdatalen; i++)
	{
		usart_data_transmit(USART1,*(pdata+i));
		while(!usart_flag_get(USART1,USART_FLAG_TBE));
	}
}
void USART1_IRQHandler(void)
{
	if(RESET != usart_interrupt_flag_get(USART1,USART_INT_FLAG_RBNE_ORERR))
	{
		usart_data_receive(USART1);
	}
	if(RESET != usart_interrupt_flag_get(USART1,USART_INT_FLAG_RBNE))
	{
      ps_queue->uartQueuePush(1,usart_data_receive(USART1));
			usart_interrupt_flag_clear(USART1,USART_INT_FLAG_RBNE);
	}
}
/*		Usart2		*/
static void Usart2_Init(unsigned int uibaudrate)
{
	 usart_deinit(USART2);
		usart_disable(USART2);
		rcu_periph_clock_enable(RCU_GPIOB);
		rcu_periph_clock_enable(RCU_USART2);
		gpio_af_set(GPIOB,GPIO_AF_7,GPIO_PIN_10|GPIO_PIN_11);
		gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_10|GPIO_PIN_11);
		gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,GPIO_PIN_10|GPIO_PIN_11);
		nvic_irq_enable(USART2_IRQn,5,0);

		usart_interrupt_enable(USART2,USART_INT_RBNE);
		usart_baudrate_set(USART2,uibaudrate);
		usart_parity_config(USART2,USART_PM_NONE);
		usart_word_length_set(USART2,USART_WL_8BIT);
		usart_stop_bit_set(USART2,USART_STB_1BIT);
		usart_hardware_flow_coherence_config(USART2,USART_HCM_NONE);
		
		usart_transmit_config(USART2,USART_TRANSMIT_ENABLE);
		usart_receive_config(USART2,USART_RECEIVE_ENABLE);
		
		usart_data_first_config(USART2,USART_MSBF_LSB);
		usart_enable(USART2);
}
static void Usart2_SendData(unsigned char *pdata,unsigned short usdatalen)
{
	for(unsigned short i = 0; i < usdatalen; i++)
	{
		usart_data_transmit(USART2,*(pdata+i));
		while(!usart_flag_get(USART2,USART_FLAG_TBE));
	}
}
void USART2_IRQHandler(void)
{
	if(RESET != usart_interrupt_flag_get(USART2,USART_INT_FLAG_RBNE_ORERR))
	{
		usart_data_receive(USART2);
	}
	if(RESET != usart_interrupt_flag_get(USART2,USART_INT_FLAG_RBNE))
	{
	   ps_queue->uartQueuePush(2,usart_data_receive(USART2));
		usart_interrupt_flag_clear(USART2,USART_INT_FLAG_RBNE);
	}
}

/*		Usart3		*/
static void Usart3_Init(unsigned int uibaudrate)
{
	usart_deinit(UART3);
		usart_disable(UART3);
		rcu_periph_clock_enable(RCU_GPIOC);
		rcu_periph_clock_enable(RCU_UART3);
		gpio_af_set(GPIOC,GPIO_AF_8,GPIO_PIN_10|GPIO_PIN_11);
		gpio_mode_set(GPIOC, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_10|GPIO_PIN_11);
		gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,GPIO_PIN_10|GPIO_PIN_11);
		nvic_irq_enable(UART3_IRQn,5,0);

		usart_interrupt_enable(UART3,USART_INT_RBNE);
		usart_baudrate_set(UART3,uibaudrate);
		usart_parity_config(UART3,USART_PM_NONE);
		usart_word_length_set(UART3,USART_WL_8BIT);
		usart_stop_bit_set(UART3,USART_STB_1BIT);
		usart_hardware_flow_coherence_config(UART3,USART_HCM_NONE);
		
		usart_transmit_config(UART3,USART_TRANSMIT_ENABLE);
		usart_receive_config(UART3,USART_RECEIVE_ENABLE);
		
		usart_data_first_config(UART3,USART_MSBF_LSB);
		usart_enable(UART3);
}

static void Usart3_SendData(unsigned char *pdata,unsigned short usdatalen)
{
	for(unsigned short i = 0; i < usdatalen; i++)
	{
		usart_data_transmit(UART3,*(pdata+i));
		while(!usart_flag_get(UART3,USART_FLAG_TBE));
	}
}

void UART3_IRQHandler(void)
{
	if(RESET != usart_interrupt_flag_get(UART3,USART_INT_FLAG_RBNE_ORERR))
	{
		usart_data_receive(UART3);
	}
	if(RESET != usart_interrupt_flag_get(UART3,USART_INT_FLAG_RBNE))
	{
    ps_queue->uartQueuePush(3,usart_data_receive(UART3));
		usart_interrupt_flag_clear(UART3,USART_INT_FLAG_RBNE);
	}
}
/*		Uart4		*/
static void Uart4_Init(unsigned int uibaudrate)
{	
	
	usart_deinit(UART4);
		usart_disable(UART4);
		rcu_periph_clock_enable(RCU_GPIOC);
	  rcu_periph_clock_enable(RCU_GPIOD);
		rcu_periph_clock_enable(RCU_UART4);
		gpio_af_set(GPIOC,GPIO_AF_8,GPIO_PIN_12);
	  gpio_af_set(GPIOD,GPIO_AF_8,GPIO_PIN_2);
		gpio_mode_set(GPIOC, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_10|GPIO_PIN_12);
	  gpio_mode_set(GPIOD, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_10|GPIO_PIN_2);
		gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,GPIO_PIN_12);
	  gpio_output_options_set(GPIOD, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,GPIO_PIN_2);
		nvic_irq_enable(UART4_IRQn,5,0);

		usart_interrupt_enable(UART4,USART_INT_RBNE);
		usart_baudrate_set(UART4,uibaudrate);
		usart_parity_config(UART4,USART_PM_NONE);
		usart_word_length_set(UART4,USART_WL_8BIT);
		usart_stop_bit_set(UART4,USART_STB_1BIT);
		usart_hardware_flow_coherence_config(UART4,USART_HCM_NONE);
		
		usart_transmit_config(UART4,USART_TRANSMIT_ENABLE);
		usart_receive_config(UART4,USART_RECEIVE_ENABLE);
		
		usart_data_first_config(UART4,USART_MSBF_LSB);
		usart_enable(UART4);
}

 static void Uart4_SendData(unsigned char *pdata,unsigned short usdatalen)
{
	for(unsigned short i = 0; i < usdatalen; i++)
	{
		usart_data_transmit(UART4,*(pdata+i));
		while(!usart_flag_get(UART4,USART_FLAG_TBE));
	}
}

void UART4_IRQHandler(void)
{
	if(RESET != usart_interrupt_flag_get(UART4,USART_INT_FLAG_RBNE_ORERR))
	{
		usart_data_receive(UART4);
	}
	if(RESET != usart_interrupt_flag_get(UART4,USART_INT_FLAG_RBNE))
	{
		ps_queue->uartQueuePush(4,usart_data_receive(UART4));
		usart_interrupt_flag_clear(UART4,USART_INT_FLAG_RBNE);
	}
}
/*		Uart5		*/
static void Uart5_Init(unsigned int uibaudrate)
{	
	usart_deinit(USART5);
		usart_disable(USART5);
		rcu_periph_clock_enable(RCU_GPIOC);
		rcu_periph_clock_enable(RCU_USART5);
		gpio_af_set(GPIOC,GPIO_AF_8,GPIO_PIN_6|GPIO_PIN_7);
		gpio_mode_set(GPIOC, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_6|GPIO_PIN_7);
		gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,GPIO_PIN_6|GPIO_PIN_7);
		nvic_irq_enable(USART5_IRQn,5,0);

		usart_interrupt_enable(USART5,USART_INT_RBNE);
		usart_baudrate_set(USART5,uibaudrate);
		usart_parity_config(USART5,USART_PM_NONE);
		usart_word_length_set(USART5,USART_WL_8BIT);
		usart_stop_bit_set(USART5,USART_STB_1BIT);
		usart_hardware_flow_coherence_config(USART5,USART_HCM_NONE);
		
		usart_transmit_config(USART5,USART_TRANSMIT_ENABLE);
		usart_receive_config(USART5,USART_RECEIVE_ENABLE);
		
		usart_data_first_config(USART5,USART_MSBF_LSB);
		usart_enable(USART5);
//----------------------485��������-----------------------------------------------		
    rcu_periph_clock_enable(RCU_GPIOD);
		gpio_mode_set(GPIOD, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLDOWN, GPIO_PIN_15);
		gpio_output_options_set(GPIOD, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,GPIO_PIN_15);
		gpio_bit_reset(GPIOD,GPIO_PIN_15);
}

static void Uart5_SendData(unsigned char *pdata,unsigned short usdatalen)
{
	gpio_bit_set(GPIOD,GPIO_PIN_15);
	ps_Systick->delay_ms(2);
	
	for(unsigned short i = 0; i < usdatalen; i++)
	{
		usart_data_transmit(USART5,*(pdata+i));
		while(!usart_flag_get(USART5,USART_FLAG_TBE));
	}
	
	ps_Systick->delay_ms(2);
	gpio_bit_reset(GPIOD,GPIO_PIN_15);
	
}
void USART5_IRQHandler(void)
{
	if(RESET != usart_interrupt_flag_get(USART5,USART_INT_FLAG_RBNE_ORERR))
	{
		usart_data_receive(USART5);
	}
	if(RESET != usart_interrupt_flag_get(USART5,USART_INT_FLAG_RBNE))
	{
		ps_queue->uartQueuePush(5,usart_data_receive(USART5));
		usart_interrupt_flag_clear(USART5,USART_INT_FLAG_RBNE);
	}
}

static void Uart6Init(unsigned int uibaudrate)
{
    usart_deinit(UART6);
		usart_disable(UART6);
		rcu_periph_clock_enable(RCU_GPIOE);
		rcu_periph_clock_enable(RCU_UART6);
		gpio_af_set(GPIOE,GPIO_AF_8,GPIO_PIN_7|GPIO_PIN_8);
		gpio_mode_set(GPIOE, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_7|GPIO_PIN_8);
		gpio_output_options_set(GPIOE, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,GPIO_PIN_7|GPIO_PIN_8);
		nvic_irq_enable(UART6_IRQn,5,0);

		usart_interrupt_enable(UART6,USART_INT_RBNE);
		usart_baudrate_set(UART6,uibaudrate);
		usart_parity_config(UART6,USART_PM_NONE);
		usart_word_length_set(UART6,USART_WL_8BIT);
		usart_stop_bit_set(UART6,USART_STB_1BIT);
		usart_hardware_flow_coherence_config(UART6,USART_HCM_NONE);
		
		usart_transmit_config(UART6,USART_TRANSMIT_ENABLE);
		usart_receive_config(UART6,USART_RECEIVE_ENABLE);
		
		usart_data_first_config(UART6,USART_MSBF_LSB);
		usart_enable(UART6);
////----------------------485��������-----------------------------------------------		
//    rcu_periph_clock_enable(RCU_GPIOD);
//		gpio_mode_set(GPIOD, GPIO_OTYPE_PP, GPIO_PUPD_PULLUP, GPIO_PIN_15);
//		gpio_output_options_set(GPIOD, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,GPIO_PIN_15);
}
static void Uart6_SendData(unsigned char *pdata,unsigned short usdatalen)
{
	for(unsigned short i = 0; i < usdatalen; i++)
	{
		usart_data_transmit(UART6,*(pdata+i));
		while(!usart_flag_get(UART6,USART_FLAG_TBE));
	}
}
void UART6_IRQHandler(void)
{
	if(RESET != usart_interrupt_flag_get(UART6,USART_INT_FLAG_RBNE_ORERR))
	{
		usart_data_receive(UART6);
	}
	if(RESET != usart_interrupt_flag_get(UART6,USART_INT_FLAG_RBNE))
	{
		ps_queue->uartQueuePush(6,usart_data_receive(UART6));
		usart_interrupt_flag_clear(UART6,USART_INT_FLAG_RBNE);
	}
}