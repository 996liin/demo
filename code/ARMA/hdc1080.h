#ifndef __HDC1080_H_
#define __HDC1080_H_


typedef struct
{
	void (*HDC1080Init)(void);
	short (*GetHDC1080Value)(unsigned char ucOpera);
}struct_HDC1080;

extern struct_HDC1080 *ps_HDC1080;
#define SDAHIGH (gpio_bit_set(GPIOC,GPIO_PIN_9))
#define SDALOW (gpio_bit_reset(GPIOC,GPIO_PIN_9))
#define SCKHIGH (gpio_bit_set(GPIOA,GPIO_PIN_8))
#define SCKLOW (gpio_bit_reset(GPIOA,GPIO_PIN_8))
#define SDASTATE (gpio_input_bit_get(GPIOC,GPIO_PIN_9))
#define Delayms(ms) (ps_Systick->delay_ms(ms))
#define Delay5us (ps_Systick->delay_us(5))
#define Delay2us (ps_Systick->delay_us(2))
#endif
