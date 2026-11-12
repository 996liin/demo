#include "headfile.h"
/*
	HDC1080:
	用于读取温度值和湿度值
*/
static void HDC1080Init(void);
static short GetHDC1080Value(unsigned char ucOpera);

static struct_HDC1080 s_HDC1080 = 
{
	.HDC1080Init = HDC1080Init,
	.GetHDC1080Value = GetHDC1080Value,
	
};
struct_HDC1080 *ps_HDC1080 = &s_HDC1080;
static void HDC1080Init(void)
{
	rcu_periph_clock_enable(RCU_GPIOC);
	rcu_periph_clock_enable(RCU_GPIOA);
	gpio_mode_set(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_9);
	gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,GPIO_PIN_9);
	gpio_mode_set(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_8);
	gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,GPIO_PIN_8);
}
static void ReadState(void)
{
	gpio_mode_set(GPIOC, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO_PIN_9);
}
static void WriteState(void)
{
	gpio_mode_set(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_9);
	gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,GPIO_PIN_9);
}
static void HDC1080Start(void)
{
	SDAHIGH;
	Delay2us;
	SCKHIGH;
	Delay5us;
	SDALOW;
	Delay5us;
	SCKLOW;
}
static void HDC1080Stop(void)
{

	SDALOW;
	Delay2us;
	SCKHIGH;
	Delay5us;
	SDAHIGH;
	Delay5us;
	SCKLOW;
}
static void HDC1080SendAck(unsigned char ack)
{
	Delay2us;
	if (1 == ack)
		SDAHIGH;
	else
		SDALOW;
	SCKHIGH;
	Delay5us;
	SCKLOW;
	Delay2us;

}
static unsigned char HDC1080RecvAck(void)
{
	ReadState();	
	unsigned char Recv = 0;
	SCKHIGH;
	Delay2us;
	Recv = SDASTATE;
	SCKLOW;
	Delay5us;
	WriteState();
	return Recv;
}
static void HDC1080SendByte(unsigned char Data)
{
	Delay2us;
	unsigned char i = 0;
	for (i = 0; i < 8; i++)
	{
		
		if (Data & 0x80)
		{
			SDAHIGH;
		}
		else
		{
			SDALOW;
		}
		Delay2us;
		
		Data = Data << 1;
	
		SCKHIGH;
		
		Delay5us;
		
		SCKLOW;
		
		Delay5us;
	}
	HDC1080RecvAck();
}
static unsigned char HDC1080RecvByte(void)
{
	ReadState();
	Delay2us;
	unsigned char Data = 0;
	unsigned char bit = 0;
	for (int i = 0; i < 8; i++)
	{
		SCKHIGH;
		Delay2us;
		if (1 == SDASTATE)
		{
			bit = 0x01;
		}
		else
		{
			
			bit = 0x00;
		}
		SCKLOW;
		Delay2us;
		Data = (Data << 1)|bit;
	}
	WriteState();
	return Data;
}
static void HDC1080TriggerTemp(void)
{
	HDC1080Start();
	HDC1080SendByte(0x80);
	HDC1080SendByte(0x00);
	HDC1080Stop();
	Delayms(50);
}
static void HDC1080TriggerHumidity(void)
{
	HDC1080Start();
	HDC1080SendByte(0x80);
	HDC1080SendByte(0x01);
	HDC1080Stop();
	Delayms(50);

}
static void HDC1080Read(short *temperature,unsigned short *humidity)
{
	HDC1080TriggerTemp();
	Delayms(50);
	HDC1080Start();
	HDC1080SendByte(0x80);
	HDC1080Start();
	HDC1080SendByte(0x81);
	*temperature = HDC1080RecvByte();
	HDC1080SendAck(0);
	*temperature = ((*temperature) << 8) + HDC1080RecvByte();
	HDC1080SendAck(1);
	HDC1080Stop();
	*temperature = (*temperature*165)>>16;
	*temperature = *temperature - 40;
	
	HDC1080TriggerHumidity();
	Delayms(50);
	HDC1080Start();
	HDC1080SendByte(0x80);
	HDC1080Start();
	HDC1080SendByte(0x81);
	*humidity = HDC1080RecvByte();
	HDC1080SendAck(0);
	*humidity = ((*humidity) << 8) + HDC1080RecvByte();
	HDC1080SendAck(1);
	HDC1080Stop();
	*humidity = (*humidity*100)>>16;
}
//形参：1为获取温度值，其他为获取湿度值
static short GetHDC1080Value(unsigned char ucOpera)
{
	short sTemperature = 0;
	unsigned short usHumidity = 0;
	HDC1080Read(&sTemperature,&usHumidity);
	
	if(1 == ucOpera)
	{
		return sTemperature;
	}
	else
	{
		return usHumidity;
	}
}
