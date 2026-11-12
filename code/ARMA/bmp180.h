#ifndef __BMP180_H_
#define __BMP180_H_


typedef struct
{
	void (*BMP180Init)(void);
	unsigned int (*GetAirPressure)(void);
		
}struct_BMP180;

extern struct_BMP180 *ps_BMP180;


#define SDAHIGH (gpio_bit_write(GPIOD,GPIO_PIN_0,SET))
#define SDALOW (gpio_bit_write(GPIOD,GPIO_PIN_0,RESET))
#define SCKHIGH (gpio_bit_write(GPIOD,GPIO_PIN_1,SET))
#define SCKLOW (gpio_bit_write(GPIOD,GPIO_PIN_1,RESET))
#define SDASTATE (gpio_input_bit_get(GPIOD,GPIO_PIN_0))
#define Delayms(ms) (ps_Systick->delay_ms(ms))
#define Delay5us (ps_Systick->delay_us(5))
#define Delay2us (ps_Systick->delay_us(2))

#endif
