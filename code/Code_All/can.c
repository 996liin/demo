#include "headfile.h"

/*
	* CAN挂载在APB1上，最大时钟54M(ARM36M),
	* 波特率500K
	* FIFO0 标准帧模式
	* FIFO1 扩展帧模式
*/

static can_receive_message_struct FIFO0RecvMsg = {0,},FIFO1RecvMsg = {0,};

static void CAN1Init(void);
static void CAN1SendData_Stand(uint32_t ID,uint8_t* msg,uint32_t len);
static void CAN1SendData_Extend(unsigned int uiSendID,unsigned char *ucData,unsigned char ucLen);
static void CAN0Init(void);
static void CAN0SendData_Stand(uint32_t ID,uint8_t* msg,uint32_t len);
static void CAN0SendData_Extend(unsigned int uiSendID,unsigned char *ucData,unsigned char ucLen);
static void CANFilterInit(void);
/*
	函数结构体-外部
*/
static struct_CAN s_CAN1 = 
{
	.CANInit = CAN1Init,
	.CANSendData_Stand = CAN1SendData_Stand,
	.CANSendData_Extend = CAN1SendData_Extend,
};
struct_CAN *ps_CAN1 = &s_CAN1;
static struct_CAN s_CAN0 = 
{
	.CANInit = CAN0Init,
	.CANSendData_Stand = CAN0SendData_Stand,
	.CANSendData_Extend = CAN0SendData_Extend,
};
struct_CAN *ps_CAN0 = &s_CAN0;
/*
	CAN初始化
*/
static void CAN1GPIOInit(void)
{
/* enable can clock */
    
   rcu_periph_clock_enable(RCU_CAN0);
	  rcu_periph_clock_enable(RCU_CAN1);
   // rcu_periph_clock_enable(RCU_CAN1);
    rcu_periph_clock_enable(RCU_GPIOB);
    
    /* configure CAN1 GPIO */
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_12);
    gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_12);
    gpio_af_set(GPIOB, GPIO_AF_9, GPIO_PIN_12);
    
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_13);
    gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_13);
    gpio_af_set(GPIOB, GPIO_AF_9, GPIO_PIN_13);
}
static void CAN0GPIOInit(void)
{
/* enable can clock */
    
   rcu_periph_clock_enable(RCU_CAN0);
    rcu_periph_clock_enable(RCU_GPIOA);
    
    /* configure CAN1 GPIO */
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_11);
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_11);
    gpio_af_set(GPIOA, GPIO_AF_9, GPIO_PIN_11);
    
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_12);
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_12);
    gpio_af_set(GPIOA, GPIO_AF_9, GPIO_PIN_12);
}
static void CANModeInit(void)
{	
	
}

static void CANFilterInit(void)
{
	
}
static void CANNVICInit(void)
{
//	nvic_irq_enable(USBD_LP_CAN0_RX0_IRQn,0,0);
//	nvic_irq_enable(CAN0_RX1_IRQn ,0,0);
//	
//	can_interrupt_enable(CAN0,CAN_INT_RFNE0);
//	can_interrupt_enable(CAN0,CAN_INT_RFNE1);
}
static void CAN1Init(void)
{
	uint8 baudrate_Index = 5;
	uint32 can_Baudrate;
	can_parameter_struct            can_parameter;
		can_filter_parameter_struct     can_filter;
	
		uint8_t ntsg1=0,ntsg2=0;
		uint16_t presc=0;		
		CAN1GPIOInit();
		if(baudrate_Index==1)
			can_Baudrate =20000;
		else if(baudrate_Index==2)
			can_Baudrate =50000;
		else if(baudrate_Index==3)
			can_Baudrate =100000;
		else if(baudrate_Index==4)
			can_Baudrate =125000;
		else if(baudrate_Index==5)
			can_Baudrate =250000;
		else if(baudrate_Index==6)
			can_Baudrate =500000;
		else if(baudrate_Index==7)
			can_Baudrate =1000000;
		else
			can_Baudrate =500000;
		 
		 
	  //????????,??????

			if(can_Baudrate==20000)
			{
					presc=300;
					ntsg1=CAN_BT_BS1_5TQ;
					ntsg2=CAN_BT_BS2_4TQ;
			}
			else if(can_Baudrate==50000)
			{
					presc=120;
					ntsg1=CAN_BT_BS1_5TQ;
					ntsg2=CAN_BT_BS2_4TQ;
			}

				else if(can_Baudrate==100000)
			{
					presc=60;
					ntsg1=CAN_BT_BS1_5TQ;
					ntsg2=CAN_BT_BS2_4TQ;
			}
				else if(can_Baudrate==125000)
			{
					presc=48;
					ntsg1=CAN_BT_BS1_5TQ;
					ntsg2=CAN_BT_BS2_4TQ;
			}

				else if(can_Baudrate==250000)
			{
					presc=24;
					ntsg1=CAN_BT_BS1_5TQ;
					ntsg2=CAN_BT_BS2_4TQ;
			}

				else if(can_Baudrate==500000)
			{
					presc=12;
					ntsg1=CAN_BT_BS1_5TQ;
					ntsg2=CAN_BT_BS2_4TQ;
			}

					else if(can_Baudrate==1000000)
			{
					presc=6;
					ntsg1=CAN_BT_BS1_5TQ;
					ntsg2=CAN_BT_BS2_4TQ;
			}
			else//??500K
			{
					presc=12;
					ntsg1=CAN_BT_BS1_5TQ;
					ntsg2=CAN_BT_BS2_4TQ;
			}
			
	
		    /* initialize CAN register */
    //can_deinit(CAN0);
  
    /* initialize CAN */
    can_parameter.time_triggered = DISABLE;
    can_parameter.auto_bus_off_recovery = DISABLE; //?????????????
    can_parameter.auto_wake_up = DISABLE;
    can_parameter.auto_retrans = DISABLE;
    can_parameter.rec_fifo_overwrite = DISABLE;//???? FIFO ????0:???? FIFO ????
    can_parameter.trans_fifo_order = DISABLE;
    can_parameter.working_mode = CAN_NORMAL_MODE;
    can_parameter.resync_jump_width = CAN_BT_SJW_1TQ;
    can_parameter.time_segment_1 = ntsg1;
    can_parameter.time_segment_2 =	ntsg2;
  
    can_parameter.prescaler = presc;
    ErrStatus data = can_init(CAN1, &can_parameter);
		
			
			
		can_filter.filter_number = 0;


    /* initialize filter */ 
    can_filter.filter_mode = CAN_FILTERMODE_MASK;
    can_filter.filter_bits = CAN_FILTERBITS_32BIT;
    can_filter.filter_list_high = 0x0000;
    can_filter.filter_list_low = 0x0000;
    can_filter.filter_mask_high = 0x0000;
    can_filter.filter_mask_low = 0x0000;  
		
		
		can_filter.filter_fifo_number = CAN_FIFO0;
		
		
    can_filter.filter_enable = ENABLE;
		
    can_filter_init(&can_filter);
		can_filter.filter_number = 15;
		can_filter_init(&can_filter);
		 /* enable CAN receive FIFO0 not empty interrupt */
    
		
		
		nvic_irq_enable(CAN1_RX0_IRQn,0,1);
		can_interrupt_enable(CAN1, CAN_INTEN_RFNEIE0);
}
static void CAN0Init(void)
{
	uint8 baudrate_Index = 5;
	uint32 can_Baudrate;
	can_parameter_struct            can_parameter;
		can_filter_parameter_struct     can_filter;
	
		uint8_t ntsg1=0,ntsg2=0;
		uint16_t presc=0;
		
		CAN0GPIOInit();

		if(baudrate_Index==1)
			can_Baudrate =20000;
		else if(baudrate_Index==2)
			can_Baudrate =50000;
		else if(baudrate_Index==3)
			can_Baudrate =100000;
		else if(baudrate_Index==4)
			can_Baudrate =125000;
		else if(baudrate_Index==5)
			can_Baudrate =250000;
		else if(baudrate_Index==6)
			can_Baudrate =500000;
		else if(baudrate_Index==7)
			can_Baudrate =1000000;
		else
			can_Baudrate =500000;
		 
		 
	  //????????,??????

			if(can_Baudrate==20000)
			{
					presc=300;
					ntsg1=CAN_BT_BS1_5TQ;
					ntsg2=CAN_BT_BS2_4TQ;
			}
			else if(can_Baudrate==50000)
			{
					presc=120;
					ntsg1=CAN_BT_BS1_5TQ;
					ntsg2=CAN_BT_BS2_4TQ;
			}

				else if(can_Baudrate==100000)
			{
					presc=60;
					ntsg1=CAN_BT_BS1_5TQ;
					ntsg2=CAN_BT_BS2_4TQ;
			}
				else if(can_Baudrate==125000)
			{
					presc=48;
					ntsg1=CAN_BT_BS1_5TQ;
					ntsg2=CAN_BT_BS2_4TQ;
			}

				else if(can_Baudrate==250000)
			{
					presc=24;
					ntsg1=CAN_BT_BS1_5TQ;
					ntsg2=CAN_BT_BS2_4TQ;
			}

				else if(can_Baudrate==500000)
			{
					presc=12;
					ntsg1=CAN_BT_BS1_5TQ;
					ntsg2=CAN_BT_BS2_4TQ;
			}

					else if(can_Baudrate==1000000)
			{
					presc=6;
					ntsg1=CAN_BT_BS1_5TQ;
					ntsg2=CAN_BT_BS2_4TQ;
			}
			else//??500K
			{
					presc=12;
					ntsg1=CAN_BT_BS1_5TQ;
					ntsg2=CAN_BT_BS2_4TQ;
			}
			
	
		    /* initialize CAN register */
    //can_deinit(CAN0);
  
    /* initialize CAN */
    can_parameter.time_triggered = DISABLE;
    can_parameter.auto_bus_off_recovery = DISABLE; //?????????????
    can_parameter.auto_wake_up = DISABLE;
    can_parameter.auto_retrans = DISABLE;
    can_parameter.rec_fifo_overwrite = DISABLE;//???? FIFO ????0:???? FIFO ????
    can_parameter.trans_fifo_order = DISABLE;
    can_parameter.working_mode = CAN_NORMAL_MODE;
    can_parameter.resync_jump_width = CAN_BT_SJW_1TQ;
    can_parameter.time_segment_1 = ntsg1;
    can_parameter.time_segment_2 =	ntsg2;
  
    can_parameter.prescaler = presc;
    ErrStatus data = can_init(CAN0, &can_parameter);
		
			
			
		can_filter.filter_number = 0;


    /* initialize filter */ 
    can_filter.filter_mode = CAN_FILTERMODE_MASK;
    can_filter.filter_bits = CAN_FILTERBITS_32BIT;
    can_filter.filter_list_high = 0x0000;
    can_filter.filter_list_low = 0x0000;
    can_filter.filter_mask_high = 0x0000;
    can_filter.filter_mask_low = 0x0000;  
		can_filter.filter_fifo_number = CAN_FIFO0;
    can_filter.filter_enable = ENABLE;
		
    can_filter_init(&can_filter);
		can_filter.filter_number = 15;
		can_filter_init(&can_filter);
		 /* enable CAN receive FIFO0 not empty interrupt */
    
		
		
		nvic_irq_enable(CAN0_RX0_IRQn,0,0);
		can_interrupt_enable(CAN0, CAN_INTEN_RFNEIE0);
}
static void CANErrorCheck(void)
{
	if(can_receive_error_number_get(CAN1) >= 64|| can_transmit_error_number_get(CAN1) >= 64)
	{
		can_deinit(CAN1);
		ps_Systick->delay_ms(100);
		CAN1Init();
	}
}
/*
*	发送函数，区分标准ID和扩展ID
*/
static void CAN1SendData_Stand(uint32_t ID,uint8_t* msg,uint32_t len)
{
	uint8_t mail_Index=0;
		uint8_t i=0;	
		uint32 Canopen_Wait_Send_Succeed_time=0;
	can_trasnmit_message_struct transmit_message;
	
	transmit_message.tx_sfid = ID;
	transmit_message.tx_efid = 0x00;
	transmit_message.tx_ft = CAN_FT_DATA; 		//???
	transmit_message.tx_ff = CAN_FF_STANDARD;	//???
	transmit_message.tx_dlen = len;
	for(i=0;i<len; i++)
	{
		transmit_message.tx_data[i] = msg[i];
	}
	
	mail_Index = can_message_transmit(CAN1, &transmit_message);

	
	//mail_Index = can_message_transmit(CAN0, &transmit_message);
		
//		Canopen_Wait_Send_Succeed_time = lwTickCount;
		
//		while(can_transmit_states(CAN0,mail_Index)!=CAN_TRANSMIT_OK)
//		{
////				if(lwTickCount>(Canopen_Wait_Send_Succeed_time + 2))
////				{
////					
////						return 1;
////						
////				}

//				
//		}
//			return 0;
}
static void CAN0SendData_Stand(uint32_t ID,uint8_t* msg,uint32_t len)
{
	uint8_t mail_Index=0;
	uint8_t i=0;	
	uint32 Canopen_Wait_Send_Succeed_time=0;
	can_trasnmit_message_struct transmit_message;
	
	transmit_message.tx_sfid = ID;
	transmit_message.tx_efid = 0x00;
	transmit_message.tx_ft = CAN_FT_DATA; 		//???
	transmit_message.tx_ff = CAN_FF_STANDARD;	//???
	transmit_message.tx_dlen = len;
	for(i=0;i<len; i++)
	{
		transmit_message.tx_data[i] = msg[i];
	}
	
	mail_Index = can_message_transmit(CAN0, &transmit_message);

	
	//mail_Index = can_message_transmit(CAN0, &transmit_message);
		
//		Canopen_Wait_Send_Succeed_time = lwTickCount;
		
//		while(can_transmit_states(CAN0,mail_Index)!=CAN_TRANSMIT_OK)
//		{
////				if(lwTickCount>(Canopen_Wait_Send_Succeed_time + 2))
////				{
////					
////						return 1;
////						
////				}

//				
//		}
//			return 0;
}
static void CAN1SendData_Extend(unsigned int uiSendID,unsigned char *ucData,unsigned char ucLen)
{
	CANErrorCheck();
	
	can_trasnmit_message_struct SendMsg = {0,};
	
	SendMsg.tx_sfid = 0;
	SendMsg.tx_efid = uiSendID;
	SendMsg.tx_ff = CAN_FF_EXTENDED;
	SendMsg.tx_ft = CAN_FT_DATA;
	SendMsg.tx_dlen = ucLen;
	for(unsigned char ucCnt = 0; ucCnt < ucLen; ucCnt++)
	{
		SendMsg.tx_data[ucCnt] = ucData[ucCnt];
	}
	
	can_message_transmit(CAN1,&SendMsg);
}
static void CAN0SendData_Extend(unsigned int uiSendID,unsigned char *ucData,unsigned char ucLen)
{
	CANErrorCheck();
	
	can_trasnmit_message_struct SendMsg = {0,};
	
	SendMsg.tx_sfid = 0;
	SendMsg.tx_efid = uiSendID;
	SendMsg.tx_ff = CAN_FF_EXTENDED;
	SendMsg.tx_ft = CAN_FT_DATA;
	SendMsg.tx_dlen = ucLen;
	for(unsigned char ucCnt = 0; ucCnt < ucLen; ucCnt++)
	{
		SendMsg.tx_data[ucCnt] = ucData[ucCnt];
	}
	
	can_message_transmit(CAN0,&SendMsg);
}

void CAN1_RX0_IRQHandler(void)
{
	can_message_receive(CAN1, CAN_FIFO0, &FIFO1RecvMsg);
	if(FIFO1RecvMsg.rx_sfid == RECV_MSG_ID)
	{
	  if((FIFO1RecvMsg.rx_data[0] == 0xe7)&&(FIFO1RecvMsg.rx_data[2] == 0x10))
		{
		  ps_qcl->usZoomValue = FIFO1RecvMsg.rx_data[3]<<8|FIFO1RecvMsg.rx_data[4];
		}
	
	}
//	CANSendData_Extend(FIFO1RecvMsg.rx_efid,FIFO1RecvMsg.rx_data,FIFO1RecvMsg.rx_dlen);
}
void CAN0_RX0_IRQHandler(void)
{
	can_message_receive(CAN0, CAN_FIFO0, &FIFO1RecvMsg);
	if(FIFO1RecvMsg.rx_sfid == RECV_MSG_ID)
	{
	  if((FIFO1RecvMsg.rx_data[0] == 0xe7)&&(FIFO1RecvMsg.rx_data[2] == 0x10))
		{
		  ps_qcl->usZoomValue = FIFO1RecvMsg.rx_data[3]<<8|FIFO1RecvMsg.rx_data[4];
		}
	
	}
//	CANSendData_Extend(FIFO1RecvMsg.rx_efid,FIFO1RecvMsg.rx_data,FIFO1RecvMsg.rx_dlen);
}
