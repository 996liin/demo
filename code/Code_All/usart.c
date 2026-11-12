#include "headfile.h"
//#define TEST
#include "Version.h"
#include "stdio.h"

extern unsigned char debug_ccd_sf_param;

extern unsigned char VERSION_STRING[];
static void UartInit(void);
static void Usart0Init(unsigned int uibaudrate);//借用fsm422控制ccd PTZ
static void Usart0_SendData(unsigned char *pdata,unsigned short usdatalen);
static void Usart1Init(unsigned int uibaudrate);//ttl接口，3588通信
static void Usart1_SendData(unsigned char *pdata,unsigned short usdatalen);
static void Usart2Init(unsigned int uibaudrate);//max3490  借用精跟422控制 ccd VISCA
static void Usart2_SendData(unsigned char *pdata,unsigned short usdatalen);
static void Uart3Init(unsigned int uibaudrate);//max3490   借用lrf控制加热板
static void Uart3_SendData(unsigned char *pdata,unsigned short usdatalen);
static void Uart4Init(unsigned int uibaudrate);//max3490   Debug接口
static void Uart4_SendData(unsigned char *pdata,unsigned short usdatalen);
static void Usart5Init(unsigned int uibaudrate);//max3485   借用laser485控制IR TTL
static void Usart5_SendData(unsigned char *pdata,unsigned short usdatalen);
static void Uart6Init(unsigned int uibaudrate);//max3490    转台接口
static void Uart6_SendData(unsigned char *pdata,unsigned short usdatalen);
static void Uart7Init(unsigned int uibaudrate);//max3232   热像232
static void Uart7_SendData(unsigned char *pdata,unsigned short usdatalen);
USER_USART_RX s_COMKeyBD;
USER_USART_RX *ps_COMKeyBD = &s_COMKeyBD;
USER_USART_RX s_COMLRF =
{
  .FlagRxdEnd = 0,
};

USER_USART_RX s_COMCCD = 
{
   .FlagRxdEnd = 0,
};
USER_USART_RX s_COMLASER = 
{
   .FlagRxdEnd = 0,
};
static Struct_Usart s_Usart = 
{
	.UartInit = UartInit,
	.Usart0_SendData = Usart0_SendData,
	.Usart1_SendData = Usart1_SendData,
	.Usart2_SendData = Usart2_SendData,
	.Uart3_SendData = Uart3_SendData,
	.Uart4_SendData = Uart4_SendData,
	.Usart5_SendData = Usart5_SendData,
	.Uart6_SendData = Uart6_SendData,
	.Uart7_SendData = Uart7_SendData,
};
Struct_Usart *ps_Usart = &s_Usart;	

unsigned int uiBaudrate[5] = {9600,9600,57600,115200,9600},uiBaudrateRead[5] = {0,};

static void UartInit(void)
{
	Usart0Init(115200);     //cam ptz
	Usart1Init(19200);      // -- 热像
	Usart2Init(115200);  
	Uart3Init(115200);    //加热
	Uart4Init(115200);     // --云台
	Usart5Init(9600);     //--聚焦
	Uart6Init(115200);   // --外部
	Uart7Init(9600);     //cam visca
	
	ps_Systick->delay_ms(500);
	char buffer[100];
	sprintf(buffer,"%s\n", VERSION_STRING);
	Usart0_SendData((unsigned char*)buffer,strlen(buffer));
	Uart6_SendData((unsigned char*)buffer,strlen(buffer));
	//Usart2_SendData((unsigned char*)buffer,strlen(buffer));
	//Uart6_SendData((unsigned char*)buffer,strlen(buffer));
//	ps_AT24CXX->AT24CXX_Read(UARTBADUADDR,(unsigned char *)uiBaudrateRead,sizeof(uiBaudrateRead));
//	
//	for(unsigned char uci = 0; uci < 5; uci++)
//	{
//		if(uiBaudrateRead[uci] != uiBaudrate[uci])
//		{
//			ps_AT24CXX->AT24CXX_Write(UARTBADUADDR,(unsigned char *)uiBaudrate,sizeof(uiBaudrate));
//			break;
//		}
//	}
}

/*		Usart0		*/
static void Usart0Init(unsigned int uibaudrate)
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
		
		// 增加软件超时机制，防止硬件死锁
		uint32_t timeout = 0x1FFFF; 
		while(!usart_flag_get(USART0, USART_FLAG_TBE))
		{
				timeout--;
				if(timeout == 0) 
				{
						break; // 如果卡太久了，直接放弃，避免系统瘫痪
				}
		}
	}
}
void USART0_IRQHandler(void)
{
	// 检查是否发生接收缓冲区非空（RBNE）或溢出错误（ORERR）中断
  // 注意：RBNE_ORERR 标志是 RBNE 和 ORERR 的“或”逻辑，即 RBNE 或 ORERR 任一有效时此标志就有效
	if(RESET != usart_interrupt_flag_get(USART0,USART_INT_FLAG_RBNE_ORERR))
	{
		// 读取数据寄存器（返回值被丢弃）
    // 目的：清除 RBNE 或 ORERR 标志（读取数据寄存器会自动清除 RBNE 和 ORERR 标志）
    // 缺陷：如果仅仅是 RBNE 触发了中断（正常接收），这里读取并丢弃数据会导致该字节丢失，
    //       同时清除了 RBNE 标志，使得后面第二个 if 再也检测不到 RBNE，从而无法将数据存入队列。
		usart_data_receive(USART0);
	}
	// 再次单独检查接收缓冲区非空标志 RBNE
  // 期望：当有正常数据时，进入此分支存入队列
	if(RESET != usart_interrupt_flag_get(USART0,USART_INT_FLAG_RBNE))
	{
		  u8 recvData = 0;
		// 读取接收到的数据字节
			recvData = usart_data_receive(USART0);
		// 检查队列的接收顶部指针是否有效（非空）
			if (NULL != ptr_CCD422Queue->pCqRecvTop)
			{
				 // 将接收到的数据存入队列顶部指针指向的位置
					*(ptr_CCD422Queue->pCqRecvTop) = recvData;
				// 更新队列顶部指针为下一个可用位置（通过队列的 NextCell 函数，操作 RCV_QUEEN 队列的顶部）
					ptr_CCD422Queue->pCqRecvTop    = ptr_CCD422Queue->NextCell(ptr_CCD422Queue, RCV_QUEEN, QUEEN_TOP);
			}
			// 手动清除 RBNE 标志（理论上读取数据寄存器已自动清除，但此处再次清除也可，无副作用）
		usart_interrupt_flag_clear(USART0,USART_INT_FLAG_RBNE);
	}
}


 /* 
* 1. 当产生 RBNE 中断（正常收到一个字节）时：
*    - 第一个 if 判断 RBNE_ORERR 为真，执行 usart_data_receive() 并丢弃数据，同时硬件自动清除 RBNE 标志。
*    - 然后第二个 if 判断 RBNE 标志，此时 RBNE 已被清除，所以不会执行，数据永远不会存入队列。
* 2. 当发生溢出错误（ORERR）且 RBNE 也有效时，第一个 if 读取数据丢弃，第二个 if 因为 RBNE 被清除也不会执行。
* 3. 只有当溢出错误发生后，RBNE 被清除但 ORERR 仍存在时（错误标志未自动清除），第二个 if 仍为假，数据依然丢失。
* 
* 正确做法：应分别处理错误和正常接收，例如：
*   - 先单独检查 ORERR，并清除之；
*   - 再单独检查 RBNE，读取数据并存入队列。
* 当前代码是错误的，无法正常接收数据。
*/



/*		Usart1		*/
static void Usart1Init(unsigned int uibaudrate)
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
//	gpio_bit_write(GPIOD,GPIO_PIN_4,SET);
//	gpio_bit_write(GPIOD,GPIO_PIN_7,SET);
//	ps_Systick->delay_ms(3);
	
	for(uint8_t i = 0; i < usdatalen; i++)
	{
		usart_data_transmit(USART1,*(pdata+i));
		// 增加软件超时机制，防止硬件死锁
		uint32_t timeout = 0x1FFFF; 
		while(!usart_flag_get(USART1, USART_FLAG_TBE))
		{
				timeout--;
				if(timeout == 0) 
				{
						break; // 如果卡太久了，直接放弃，避免系统瘫痪
				}
		}
	}
	
//	ps_Systick->delay_ms(3);
//	gpio_bit_write(GPIOD,GPIO_PIN_4,RESET);
//	gpio_bit_write(GPIOD,GPIO_PIN_7,RESET);
}
void USART1_IRQHandler(void)
{
	if(RESET != usart_interrupt_flag_get(USART1,USART_INT_FLAG_RBNE_ORERR))
	{
		usart_data_receive(USART1);
	}
	if(RESET != usart_interrupt_flag_get(USART1,USART_INT_FLAG_RBNE))
	{
      u8 recvData = 0;
			recvData = usart_data_receive(USART1);
//			if (NULL != ptr_Rk3588Queue->pCqRecvTop)
//			{
//					*(ptr_Rk3588Queue->pCqRecvTop) = recvData;
//					ptr_Rk3588Queue->pCqRecvTop    = ptr_Rk3588Queue->NextCell(ptr_Rk3588Queue, RCV_QUEEN, QUEEN_TOP);
//			}
		
			if (NULL != ptr_IR232Queue->pCqRecvTop)
			{
					*(ptr_IR232Queue->pCqRecvTop) = recvData;
					ptr_IR232Queue->pCqRecvTop    = ptr_IR232Queue->NextCell(ptr_IR232Queue, RCV_QUEEN, QUEEN_TOP);
			}
			usart_interrupt_flag_clear(USART1,USART_INT_FLAG_RBNE);
	}
}

/*		Usart2		*/
static void Usart2Init(unsigned int uibaudrate)
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
	for(uint8_t i = 0; i < usdatalen; i++)
	{
		usart_data_transmit(USART2,*(pdata+i));
		// 增加软件超时机制，防止硬件死锁
		uint32_t timeout = 0x1FFFF; 
		while(!usart_flag_get(USART2, USART_FLAG_TBE))
		{
				timeout--;
				if(timeout == 0) 
				{
						break; // 如果卡太久了，直接放弃，避免系统瘫痪
				}
		}
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
    u8 recvData = 0;
		recvData = usart_data_receive(USART2);
//		if (NULL != ptr_DebugQueue->pCqRecvTop)
//		{
//				*(ptr_DebugQueue->pCqRecvTop) = recvData;
//				ptr_DebugQueue->pCqRecvTop    = ptr_DebugQueue->NextCell(ptr_DebugQueue, RCV_QUEEN, QUEEN_TOP);
//		}
		usart_interrupt_flag_clear(USART2,USART_INT_FLAG_RBNE);
	}
}

/*		Uart3		*/
static void Uart3Init(unsigned int uibaudrate)
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

static void Uart3_SendData(unsigned char *pdata,unsigned short usdatalen)
{
	for(unsigned short i = 0; i < usdatalen; i++)
	{
		usart_data_transmit(UART3,*(pdata+i));
		// 增加软件超时机制，防止硬件死锁
		uint32_t timeout = 0x1FFFF; 
		while(!usart_flag_get(UART3, USART_FLAG_TBE))
		{
				timeout--;
				if(timeout == 0) 
				{
						break; // 如果卡太久了，直接放弃，避免系统瘫痪
				}
		}
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
		  u8 recvData = 0;
			recvData = usart_data_receive(UART3);
			if (NULL != ptr_TempCtrlQueue->pCqRecvTop)
			{
					*(ptr_TempCtrlQueue->pCqRecvTop) = recvData;
					ptr_TempCtrlQueue->pCqRecvTop    = ptr_TempCtrlQueue->NextCell(ptr_TempCtrlQueue, RCV_QUEEN, QUEEN_TOP);
			}
		
		usart_interrupt_flag_clear(UART3,USART_INT_FLAG_RBNE);
	}
}
/*		Uart4		*/
static void Uart4Init(unsigned int uibaudrate)
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
//	gpio_bit_set(GPIOD,GPIO_PIN_3);
//	ps_Systick->delay_ms(5);
	
	for(unsigned short i = 0; i < usdatalen; i++)
	{
		usart_data_transmit(UART4,*(pdata+i));
		// 增加软件超时机制，防止硬件死锁
		uint32_t timeout = 0x1FFFF; 
		while(!usart_flag_get(UART4, USART_FLAG_TBE))
		{
				timeout--;
				if(timeout == 0) 
				{
						break; // 如果卡太久了，直接放弃，避免系统瘫痪
				}
		}
	}
	
//	ps_Systick->delay_ms(5);
//	gpio_bit_reset(GPIOD,GPIO_PIN_3);
	
}
void UART4_IRQHandler(void)
{
	if(RESET != usart_interrupt_flag_get(UART4,USART_INT_FLAG_RBNE_ORERR))
	{
		usart_data_receive(UART4);
	}
	if(RESET != usart_interrupt_flag_get(UART4,USART_INT_FLAG_RBNE))
	{
		u8 recvData = 0;
		recvData = usart_data_receive(UART4);

			if (NULL != ptr_PtctrlQueue->pCqRecvTop)
			{
					*(ptr_PtctrlQueue->pCqRecvTop) = recvData;
					ptr_PtctrlQueue->pCqRecvTop    = ptr_PtctrlQueue->NextCell(ptr_PtctrlQueue, RCV_QUEEN, QUEEN_TOP);
			}
		
		
		usart_interrupt_flag_clear(UART4,USART_INT_FLAG_RBNE);
	}
}
static void Usart5Init(unsigned int uibaudrate)
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
//----------------------485方向引脚-----------------------------------------------		
    rcu_periph_clock_enable(RCU_GPIOD);
		gpio_mode_set(GPIOD, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLDOWN, GPIO_PIN_15);
		gpio_output_options_set(GPIOD, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,GPIO_PIN_15);
		gpio_bit_reset(GPIOD,GPIO_PIN_15);
}
static void Usart5_SendData(unsigned char *pdata,unsigned short usdatalen)
{
//	gpio_bit_set(GPIOD,GPIO_PIN_15);
//	ps_Systick->delay_ms(1);
	
	for(unsigned short i = 0; i < usdatalen; i++)
	{
		usart_data_transmit(USART5,*(pdata+i));
		// 增加软件超时机制，防止硬件死锁
		uint32_t timeout = 0x1FFFF; 
		while(!usart_flag_get(USART5, USART_FLAG_TBE))
		{
				timeout--;
				if(timeout == 0) 
				{
						break; // 如果卡太久了，直接放弃，避免系统瘫痪
				}
		}
	}
	
//	ps_Systick->delay_ms(1);
//	gpio_bit_reset(GPIOD,GPIO_PIN_15);
	
}
void USART5_IRQHandler(void)
{
	if(RESET != usart_interrupt_flag_get(USART5,USART_INT_FLAG_RBNE_ORERR))
	{
		usart_data_receive(USART5);
	}
	if(RESET != usart_interrupt_flag_get(USART5,USART_INT_FLAG_RBNE))
	{
		  u8 recvData = 0;
			recvData = usart_data_receive(USART5);
		
		if (NULL != ptr_IRAFQueue->pCqRecvTop)
		{
				*(ptr_IRAFQueue->pCqRecvTop) = recvData;
				ptr_IRAFQueue->pCqRecvTop    = ptr_IRAFQueue->NextCell(ptr_IRAFQueue, RCV_QUEEN, QUEEN_TOP);
		}
		
		//如果是调试自动聚焦模式，将收到的数据透传到外部422
		if (debug_ccd_sf_param)
		{
				if (NULL != ptr_DebugQueue->pCqSendTop)
        {
            *(ptr_DebugQueue->pCqSendTop) =  recvData;
            ptr_DebugQueue->pCqSendTop    = ptr_DebugQueue->NextCell(ptr_DebugQueue, SND_QUEEN, QUEEN_TOP);
        }
		}
			usart_interrupt_flag_clear(USART1,USART_INT_FLAG_RBNE);
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
////----------------------485方向引脚-----------------------------------------------		
//    rcu_periph_clock_enable(RCU_GPIOD);
//		gpio_mode_set(GPIOD, GPIO_OTYPE_PP, GPIO_PUPD_PULLUP, GPIO_PIN_15);
//		gpio_output_options_set(GPIOD, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,GPIO_PIN_15);
}
static void Uart6_SendData(unsigned char *pdata,unsigned short usdatalen)
{
	for(unsigned short i = 0; i < usdatalen; i++)
	{
		usart_data_transmit(UART6,*(pdata+i));
		// 增加软件超时机制，防止硬件死锁
		uint32_t timeout = 0x1FFFF; 
		while(!usart_flag_get(UART6, USART_FLAG_TBE))
		{
				timeout--;
				if(timeout == 0) 
				{
						break; // 如果卡太久了，直接放弃，避免系统瘫痪
				}
		}
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
		 u8 recvData = 0;
			recvData = usart_data_receive(UART6);

		if (NULL != ptr_DebugQueue->pCqRecvTop)
		{
				*(ptr_DebugQueue->pCqRecvTop) = recvData;
				ptr_DebugQueue->pCqRecvTop    = ptr_DebugQueue->NextCell(ptr_DebugQueue, RCV_QUEEN, QUEEN_TOP);
		}
		
	  //如果是调试自动聚焦模式，将收到的数据透传到AF板
		if (debug_ccd_sf_param)
		{
        if (NULL != ptr_IRAFQueue->pCqSendTop)
        {
            *(ptr_IRAFQueue->pCqSendTop) =  recvData;
            ptr_IRAFQueue->pCqSendTop    = ptr_IRAFQueue->NextCell(ptr_IRAFQueue, SND_QUEEN, QUEEN_TOP);
        }
		}
		
		usart_interrupt_flag_clear(UART6,USART_INT_FLAG_RBNE);
	}
}
static void Uart7Init(unsigned int uibaudrate)
{
    usart_deinit(UART7);
		usart_disable(UART7);
		rcu_periph_clock_enable(RCU_GPIOE);
		rcu_periph_clock_enable(RCU_UART7);
		gpio_af_set(GPIOE,GPIO_AF_8,GPIO_PIN_0|GPIO_PIN_1);
		gpio_mode_set(GPIOE, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_0|GPIO_PIN_1);
		gpio_output_options_set(GPIOE, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,GPIO_PIN_0|GPIO_PIN_1);
		nvic_irq_enable(UART7_IRQn,5,0);

		usart_interrupt_enable(UART7,USART_INT_RBNE);
		usart_baudrate_set(UART7,uibaudrate);
		usart_parity_config(UART7,USART_PM_NONE);
		usart_word_length_set(UART7,USART_WL_8BIT);
		usart_stop_bit_set(UART7,USART_STB_1BIT);
		usart_hardware_flow_coherence_config(UART7,USART_HCM_NONE);
		
		usart_transmit_config(UART7,USART_TRANSMIT_ENABLE);
		usart_receive_config(UART7,USART_RECEIVE_ENABLE);
		
		usart_data_first_config(UART7,USART_MSBF_LSB);
		usart_enable(UART7);
}
static void Uart7_SendData(unsigned char *pdata,unsigned short usdatalen)
{
	for(unsigned short i = 0; i < usdatalen; i++)
	{
		usart_data_transmit(UART7,*(pdata+i));
		// 增加软件超时机制，防止硬件死锁
		uint32_t timeout = 0x1FFFF; 
		while(!usart_flag_get(UART7, USART_FLAG_TBE))
		{
				timeout--;
				if(timeout == 0) 
				{
						break; // 如果卡太久了，直接放弃，避免系统瘫痪
				}
		}
	}
}
void UART7_IRQHandler(void)
{
	if(RESET != usart_interrupt_flag_get(UART7,USART_INT_FLAG_RBNE_ORERR))
	{
		usart_data_receive(UART7);
	}
	if(RESET != usart_interrupt_flag_get(UART7,USART_INT_FLAG_RBNE))
	{
		s_COMCCD.Rxd[s_COMCCD.RxdCnt] = usart_data_receive(UART7);
		if(s_COMCCD.Rxd[0] == 0x90)
		{
				++s_COMCCD.RxdCnt;
		}
		else
		{
				s_COMCCD.RxdCnt = 0;
		}
		if(s_COMCCD.RxdCnt == 2)
		{
		   if(s_COMCCD.Rxd[1] != 0x50)
			 {
			   s_COMCCD.RxdCnt = 0;
			 }
		}
		if(ps_CCD->query_witch == 3)
		{
		  s_COMCCD.Length = 4;
		}
		else
		{
		  s_COMCCD.Length = 7;
		}
		if(s_COMCCD.RxdCnt == s_COMCCD.Length)
		{
				s_COMCCD.RxdCnt = 0;
				if(s_COMCCD.Rxd[1] == 0x50)
				{
						if(s_COMCCD.Rxd[s_COMCCD.Length-1] == 0xff)
						{
								s_COMCCD.FlagRxdEnd = 1;
						}
				}
		}
		usart_interrupt_flag_clear(UART7,USART_INT_FLAG_RBNE);
	}
}
void DataBack_Angle(void)
{  
	if(ps_Pt->databack_enable_flag)
	{
		unsigned char ucCom[17] = {0x55,0xaa,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xf0}; 
		 u16 angle_t;
		 if(ps_Pt->V_Angle < 0)
		 {
			 angle_t = ps_Pt->V_Angle + 36000;
		 }
		 else
		 {
		   angle_t = ps_Pt->V_Angle;
		 }
		 ucCom[2] = (ps_Pt->H_Angle) >> 8;
	   ucCom[3] = (ps_Pt->H_Angle) & 0xff;
		 ucCom[4] = (angle_t) >> 8;
		 ucCom[5] = (angle_t) & 0xff;
		 ucCom[6] = ps_IR->zoomvalueOffset >> 8;
	   ucCom[7] = ps_IR->zoomvalueOffset & 0xff;
		 ucCom[8] = ps_CCD->zoomvalue >> 8;
	   ucCom[9] = ps_CCD->zoomvalue & 0xff;
		 ucCom[10] = ps_IR->focusvalue >> 8;
	   ucCom[11] = ps_IR->focusvalue & 0xff;
		 ucCom[12] = ps_CCD->focusvalue >> 8;
	   ucCom[13] = ps_CCD->focusvalue & 0xff;
		 ucCom[14] = ps_CCD->icr_open_flag;
		 ucCom[15] = (ucCom[2]+ucCom[3]+ucCom[4]+ucCom[5]+ucCom[6]+ucCom[7]+ucCom[8]+ucCom[9]+ucCom[10]+ucCom[11]+ucCom[12]+ucCom[13]+ucCom[14])&0xff;
	   ps_Usart->Uart6_SendData(ucCom,17);
//	 ps_Pt->databack_com = COMDEBUG;
//	 ps_Pt->databack_pelcod(P,1);
//	 ps_Pt->databack_com = COMDEBUG;
//	 ps_Pt->databack_pelcod(T,1);
	}	
}
void OSD_Refresh(void)
{
	 unsigned char ucCom_ir1[10] = {0xF0,0x06,0x36,0x70,0x23,0x00,0x00,0x54,0x1D,0xFF};//p
	 unsigned char ucCom_ir2[9] = {0xF0,0x05,0x36,0x70,0x22,0x00,0x54,0x1D,0xFF};//t
	 signed char angle_t;
  // ps_CCD->Set_PT_OSD(ps_Pt->H_Angle,ps_Pt->V_Angle);
	 ucCom_ir1[6] = ps_Pt->H_Angle/100 >> 8;
	 ucCom_ir1[7] = ps_Pt->H_Angle/100 & 0xff;
	 ucCom_ir1[8] = (ucCom_ir1[2]+ucCom_ir1[3]+ucCom_ir1[4]+ucCom_ir1[5]+ucCom_ir1[6]+ucCom_ir1[7])&0xff;
	 ps_Usart->Usart1_SendData(ucCom_ir1,10);
	 angle_t = ps_Pt->V_Angle/70;//osd显示范围为±90，实际角度只有±65，客户要求全量程显示所以/65而不是/100
	 if(angle_t > 90)
	 {
	   angle_t = 90;
	 }
	 if(angle_t < - 90)
	 {
	   angle_t = -90;
	 }
	 ucCom_ir2[6] = (u8)angle_t;
	 ucCom_ir2[7] = (ucCom_ir2[2]+ucCom_ir2[3]+ucCom_ir2[4]+ucCom_ir2[5]+ucCom_ir2[6])&0xff;
	 ps_Usart->Usart1_SendData(ucCom_ir2,9);
}
