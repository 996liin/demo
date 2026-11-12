
#ifndef __PERIPHERALCONFIGURATION_H
#define __PERIPHERALCONFIGURATION_H
	
/** GPIO
 *
 */
//位带操作,实现51类似的GPIO控制功能
//具体实现思想,参考<<CM3权威指南>>第五章(87页~92页).
//IO口操作宏定义
#define BITBAND(addr, bitnum) ((addr & 0xF0000000)+0x2000000+((addr &0xFFFFF)<<5)+(bitnum<<2)) 
//xc 位带寻址区和别名区对应关系为1位用32位（4Byte）表示，一字则用32Byte表示，则一个0x400宽的GPIOA 用(addr &0xFFFFF)<<5寻基址	 
#define MEM_ADDR(addr)  *((volatile unsigned long  *)(addr)) 
#define BIT_ADDR(addr, bitnum)   MEM_ADDR(BITBAND(addr, bitnum)) 
//IO口地址映射
//wyh-2023.9.14
#define GPIOA_ODR_Addr    (GPIO_BASE+20) 
#define GPIOB_ODR_Addr    (GPIO_BASE+0x0400+20) 
#define GPIOC_ODR_Addr    (GPIO_BASE+0x0800+20) 
#define GPIOD_ODR_Addr    (GPIO_BASE+0x0C00+20) 
#define GPIOE_ODR_Addr    (GPIO_BASE+0x1000+20) 
#define GPIOF_ODR_Addr    (GPIO_BASE+0x1400+20)   
#define GPIOG_ODR_Addr    (GPIO_BASE+0x1800+20)  

#define GPIOA_IDR_Addr    (GPIO_BASE+16) 
#define GPIOB_IDR_Addr    (GPIO_BASE+0x0400+16) 
#define GPIOC_IDR_Addr    (GPIO_BASE+0x0800+16) 
#define GPIOD_IDR_Addr    (GPIO_BASE+0x0C00+16) 
#define GPIOE_IDR_Addr    (GPIO_BASE+0x1000+16) 
#define GPIOF_IDR_Addr    (GPIO_BASE+0x1400+16) 
#define GPIOG_IDR_Addr    (GPIO_BASE+0x1800+16) 	   
//IO口操作,只对单一的IO口!
//确保n的值小于16!
#define PAout(n)   BIT_ADDR(GPIOA_ODR_Addr,n)  //输出 
#define PAin(n)    BIT_ADDR(GPIOA_IDR_Addr,n)  //输入 	   
#define PBout(n)   BIT_ADDR(GPIOB_ODR_Addr,n)  //输出 
#define PBin(n)    BIT_ADDR(GPIOB_IDR_Addr,n)  //输入 	
#define PCout(n)   BIT_ADDR(GPIOC_ODR_Addr,n)  //输出 
#define PCin(n)    BIT_ADDR(GPIOC_IDR_Addr,n)  //输入 	
#define PDout(n)   BIT_ADDR(GPIOD_ODR_Addr,n)  //输出 
#define PDin(n)    BIT_ADDR(GPIOD_IDR_Addr,n)  //输入  
#define PEout(n)   BIT_ADDR(GPIOE_ODR_Addr,n)  //输出 
#define PEin(n)    BIT_ADDR(GPIOE_IDR_Addr,n)  //输入 
#define PFout(n)   BIT_ADDR(GPIOF_ODR_Addr,n)  //输出 
#define PFin(n)    BIT_ADDR(GPIOF_IDR_Addr,n)  //输入 
#define PGout(n)   BIT_ADDR(GPIOG_ODR_Addr,n)  //输出 
#define PGin(n)    BIT_ADDR(GPIOG_IDR_Addr,n)  //输入
/* end */

/** TIMER
 *
 */
#define QG_TIM1	0
#define QG_TIM2	1
#define QG_TIM3	2
#define QG_TIM4	3
#define QG_TIM5	4
#define QG_TIM6	5
#define QG_TIM7	6
#define QG_TIM8	7
/* end */

/** USART
 *
 */
#define QG_USART1 	0 
#define QG_USART2 	1 
#define QG_USART3 	2 
#define QG_UART4 	3 
#define QG_UART5 	4
/* end */

/** SPI
 *
 */
#define QG_SPI1	0
#define QG_SPI2	1
#define QG_SPI3	2
/* end */

/** ADC
 *
 */
#define QG_ADC1 0
#define QG_ADC2 1
#define QG_ADC3 2
/* end */

/** EXTI
 *
 */
#define QG_EXTI_Line0		0
#define QG_EXTI_Line1		1
#define QG_EXTI_Line2		2
#define QG_EXTI_Line3		3
#define QG_EXTI_Line4		4
#define QG_EXTI_Line9_5		5
#define QG_EXTI_Line15_10	6



#endif /* __PERIPHERALCONFIGURATION_H */
