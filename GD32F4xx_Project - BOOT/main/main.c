/*
	HSE=8M
	SYSCLK=180M
	AHBCLK=180M
	APB1CLK=45M
	APB2CLK=90M
	TIM2-TIM7、TIM12-TIM14:CLK=90M
	TIM1、TIM8-TIM11:CLK=180M
*/


#include "headfile.h"

static void gpioInit(void);

/*
	type为电路板类型，
	type=1为STM32F1主控板主芯片，
	type=2为GD32F1主控板主芯片，
	type=3为STM32F4主控板主芯片，
	type=4为STM32F1/STM32F4主控板从芯片，
	type=5为GD32F1主控板从芯片，
	type=6为无热化板，
	type=7为设备箱板；
	type=8为转台板；
*/

unsigned char ucBoardType = 1;
unsigned int uiBuf[4] = {0};
int main(void)
{	
  uiBuf[0] = rcu_clock_freq_get(CK_SYS);
	uiBuf[1] = rcu_clock_freq_get(CK_AHB);
	uiBuf[2] = rcu_clock_freq_get(CK_APB1);
	uiBuf[3] = rcu_clock_freq_get(CK_APB2);
	
	//nvic_vector_table_set(NVIC_VECTTAB_FLASH,0x8000);
	
	nvic_priority_group_set(NVIC_PRIGROUP_PRE0_SUB4);
	
	ps_Systick->Systick_Init(240);
		
	gpioInit();
	
	ps_AT24CXX->AT24CXX_Init();
	
	ps_Usart->uartInit();
  
	ps_Timer->Tim2_Init_1ms();
	ps_IAP->IAPProcess();
	
	
	return 0;
}
static void gpioInit(void)
{
	rcu_periph_clock_enable(RCU_GPIOD);
	rcu_periph_clock_enable(RCU_GPIOC);
	rcu_periph_clock_enable(RCU_GPIOB);
	rcu_periph_clock_enable(RCU_GPIOA);
	rcu_periph_clock_enable(RCU_GPIOE);
	
	//MOTOR1 
	gpio_mode_set(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLDOWN, GPIO_PIN_9);
	gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,GPIO_PIN_9);
	gpio_bit_write(GPIOA,GPIO_PIN_9,RESET);
	
		//MOTOR1 
//	gpio_mode_set(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLDOWN, GPIO_PIN_ALL);
//	gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,GPIO_PIN_ALL);
//		//MOTOR1 
//	gpio_mode_set(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLDOWN, GPIO_PIN_ALL);
//	gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,GPIO_PIN_ALL);
//		//MOTOR1 
//	gpio_mode_set(GPIOD, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLDOWN, GPIO_PIN_ALL);
//	gpio_output_options_set(GPIOD, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,GPIO_PIN_ALL);
//		//MOTOR1 
//	gpio_mode_set(GPIOE, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLDOWN, GPIO_PIN_ALL);
//	gpio_output_options_set(GPIOE, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,GPIO_PIN_ALL);
	
}
