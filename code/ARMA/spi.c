#include "headfile.h"

unsigned short usOutTimeCnt = 0;
unsigned short usSPI0SendData[8] = {0x0000,0x0000,0x0800,0x0800,0x1000,0x1000,0x1800,0x1800},usSPI2SendData = 0,usSPI0RecvData[8] = {0},usSPI2RecvData[10] = {0};

static void SPIInit(void);
static unsigned short GetSPI0Data(unsigned short usSendData);
static unsigned short GetSPI0Data_DMA(unsigned short usSendData);
static void SPI1_SendData(unsigned char ucData);
static unsigned short GetSPI2Data(unsigned short usSendData);
static unsigned short GetSPI2Data_DMA(unsigned short usSendData);

static void SPIInit_Simulation(void);
static unsigned short GetSPI0Data_Simulation(unsigned short usData);
static unsigned short GetSPI2Data_Simulation(unsigned short usData);

static Struct_SPI s_SPI = 
{
	.SPIInit = SPIInit,
	.GetSPI0Data = GetSPI0Data,
	.GetSPI0Data_DMA = GetSPI0Data_DMA,
	.SPI1_SendData = SPI1_SendData,
	.GetSPI2Data = GetSPI2Data,
	.GetSPI2Data_DMA = GetSPI2Data_DMA,
	
	.SPIInit_Simulation = SPIInit_Simulation,
	.GetSPI0Data_Simulation = GetSPI0Data_Simulation,
	.GetSPI2Data_Simulation = GetSPI2Data_Simulation,
};
Struct_SPI *ps_SPI = &s_SPI;

/*DMA配置*/
static void DMAInit(void)
{
	
}
/*	spi0 : 用于ADC数据采集	挂载在APB2，但是做了二分频 CLKMAX=108M/2=54M	AD芯片CLK:0.8-3.2M*/
static void SPI0Init(void)
{
//	rcu_periph_clock_enable(RCU_SPI0);
//	rcu_periph_clock_enable(RCU_AF);	
//	rcu_periph_clock_enable(RCU_GPIOA);
//	
//	//NSS
//	gpio_init(GPIOA,GPIO_OTYPE_PP,GPIO_OSPEED_50MHZ,GPIO_PIN_4);
//	//SCK
//	gpio_init(GPIOA,GPIO_MODE_AF_PP,GPIO_OSPEED_50MHZ,GPIO_PIN_5);
//	//MISO
//	gpio_init(GPIOA,GPIO_MODE_IN_FLOATING,GPIO_OSPEED_50MHZ,GPIO_PIN_6);
//	//MOSI
//	gpio_init(GPIOA,GPIO_MODE_AF_PP,GPIO_OSPEED_50MHZ,GPIO_PIN_7);
//	
//	spi_parameter_struct spi_para;
//	spi_para.device_mode = SPI_MASTER;
//	spi_para.trans_mode = SPI_TRANSMODE_FULLDUPLEX;
//	spi_para.frame_size = SPI_FRAMESIZE_16BIT;
//	spi_para.nss = SPI_NSS_SOFT;
//	spi_para.endian = SPI_ENDIAN_MSB;
//	spi_para.clock_polarity_phase = SPI_CK_PL_HIGH_PH_2EDGE;
//	spi_para.prescale = SPI_PSC_256;
//	spi_init(SPI0,&spi_para);
//	
//	spi_crc_polynomial_set(SPI0,7);
//	spi_crc_on(SPI0);
//	
//	//spi_dma_enable(SPI0,SPI_DMA_TRANSMIT);
//	//spi_dma_enable(SPI0,SPI_DMA_RECEIVE);
//	
//	spi_enable(SPI0);	
	
}
static unsigned short GetSPI0Data(unsigned short usSendData)
{
	unsigned short usReturnData = 0;
	
	gpio_bit_write(GPIOA,GPIO_PIN_4,RESET);
	
	usOutTimeCnt = 0;
	while((SPI_STAT(SPI0) & 0x02) == 0)
	{
		usOutTimeCnt++;
		if(usOutTimeCnt >= 1000)
		{
			break;
		}
	}
	SPI_DATA(SPI0) = usSendData;
	
	usOutTimeCnt = 0;
	while((SPI_STAT(SPI0) & 0x01) == 0)
	{
		usOutTimeCnt++;
		if(usOutTimeCnt >= 1000)
		{
			break;
		}
	}
	usReturnData = SPI_DATA(SPI0);
	
	gpio_bit_write(GPIOA,GPIO_PIN_4,SET);
	
	return usReturnData;
}

static unsigned short GetSPI0Data_DMA(unsigned short usSendData)
{
//	gpio_bit_write(GPIOA,GPIO_PIN_4,RESET);
//	
//	for(unsigned char uci = 0; uci < 12; uci++)
//	{
//		usOutTimeCnt = 0;
//		while((SPI_STAT(SPI0) & 0x02) == 0)
//		{
//			usOutTimeCnt++;
//			if(usOutTimeCnt >= 1000)
//			{
//				break;
//			}
//		}
//		usSPI0SendData = usSendData;
//		SPI_DATA(SPI0) = usSPI0SendData;
//	}

//	ps_Systick->delay_ms(1);
//	
//	unsigned int uiReturnData = 0;
//	for(unsigned char uci = 0; uci < 10; uci++)
//	{
//		uiReturnData += usSPI0RecvData[uci];
//	}
//	
//	gpio_bit_write(GPIOA,GPIO_PIN_4,SET);
	if(usSendData > 4)
	{
		usSendData = 1;
	}
	return usSPI0RecvData[usSendData*2-1];//(uiReturnData & 0xffff);
}
/*	spi1 : 用于字符叠加	挂载在APB1 CLKMAX=54M	*/
static void SPI1Init(void)
{
//	rcu_periph_clock_enable(RCU_SPI1);
//	rcu_periph_clock_enable(RCU_AF);	
//	
//	rcu_periph_clock_enable(RCU_GPIOB);
//	rcu_periph_clock_enable(RCU_GPIOE);
//	
//	//NSS
//	gpio_init(GPIOB,GPIO_OTYPE_PP,GPIO_OSPEED_50MHZ,GPIO_PIN_12);
//	//SCK
//	gpio_init(GPIOB,GPIO_MODE_AF_PP,GPIO_OSPEED_50MHZ,GPIO_PIN_13);
//	//MISO
//	gpio_init(GPIOB,GPIO_MODE_IN_FLOATING,GPIO_OSPEED_50MHZ,GPIO_PIN_14);
//	//MOSI
//	gpio_init(GPIOB,GPIO_MODE_AF_PP,GPIO_OSPEED_50MHZ,GPIO_PIN_15);
//	
//	//字符叠加芯片的使能
//	gpio_init(GPIOE,GPIO_OTYPE_PP,GPIO_OSPEED_50MHZ,GPIO_PIN_15);
//	gpio_bit_write(GPIOE,GPIO_PIN_15,SET);
//	
//	spi_parameter_struct spi_para;
//	spi_para.device_mode = SPI_MASTER;
//	spi_para.trans_mode = SPI_TRANSMODE_FULLDUPLEX;
//	spi_para.frame_size = SPI_FRAMESIZE_8BIT;
//	spi_para.nss = SPI_NSS_SOFT;
//	spi_para.endian = SPI_ENDIAN_LSB;
//	spi_para.clock_polarity_phase = SPI_CK_PL_HIGH_PH_2EDGE;
//	spi_para.prescale = SPI_PSC_64;
//	spi_init(SPI1,&spi_para);
//	
//	spi_crc_polynomial_set(SPI1,7);
//	spi_crc_on(SPI1);
//	
//	spi_enable(SPI1);
}
static void SPI1_SendData(unsigned char ucData)
{
	gpio_bit_write(GPIOB,GPIO_PIN_12,RESET);
	
	SPI_DATA(SPI0) = ucData;
	while((SPI_STAT(SPI0) & 0x02) == 0);
	
	gpio_bit_write(GPIOB,GPIO_PIN_12,SET);
}

/*	spi2 : 用于ADC数据采集	挂载在APB1 CLKMAX=54M	AD芯片CLK:0.8-3.2M */
static void SPI2Init(void)
{
//	rcu_periph_clock_enable(RCU_SPI2);
//	rcu_periph_clock_enable(RCU_AF);	
//	rcu_periph_clock_enable(RCU_GPIOA);
//	rcu_periph_clock_enable(RCU_GPIOB);
//	
//	//PA15 remap
//	gpio_pin_remap_config(GPIO_SWJ_SWDPENABLE_REMAP,ENABLE);
//	
//	//NSS
//	gpio_init(GPIOA,GPIO_OTYPE_PP,GPIO_OSPEED_50MHZ,GPIO_PIN_15);
//	//SCK
//	gpio_init(GPIOB,GPIO_MODE_AF_PP,GPIO_OSPEED_50MHZ,GPIO_PIN_3);
//	//MISO
//	gpio_init(GPIOB,GPIO_MODE_IN_FLOATING,GPIO_OSPEED_50MHZ,GPIO_PIN_4);
//	//MOSI
//	gpio_init(GPIOB,GPIO_MODE_AF_PP,GPIO_OSPEED_50MHZ,GPIO_PIN_5);
//	
//	spi_parameter_struct spi_para;
//	spi_para.device_mode = SPI_MASTER;
//	spi_para.trans_mode = SPI_TRANSMODE_FULLDUPLEX;
//	spi_para.frame_size = SPI_FRAMESIZE_16BIT;
//	spi_para.nss = SPI_NSS_SOFT;
//	spi_para.endian = SPI_ENDIAN_MSB;
//	spi_para.clock_polarity_phase = SPI_CK_PL_LOW_PH_1EDGE;
//	spi_para.prescale = SPI_PSC_256;
//	spi_init(SPI2,&spi_para);
//	
//	spi_crc_polynomial_set(SPI2,7);
//	spi_crc_on(SPI2);
//	
//	spi_dma_enable(SPI2,SPI_DMA_TRANSMIT);
//	spi_dma_enable(SPI2,SPI_DMA_RECEIVE);
//	
//	spi_enable(SPI2);
}
static unsigned short GetSPI2Data(unsigned short usSendData)
{
	unsigned short usReturnData = 0;
	
	gpio_bit_write(GPIOA,GPIO_PIN_15,RESET);
	
	usOutTimeCnt = 0;
	while((SPI_STAT(SPI2) & 0x02) == 0)
	{
		usOutTimeCnt++;
		if(usOutTimeCnt >= 1000)
		{
			break;
		}
	}
	SPI_DATA(SPI2) = usSendData;
	
	usOutTimeCnt = 0;
	while((SPI_STAT(SPI2) & 0x01) == 0)
	{
		usOutTimeCnt++;
		if(usOutTimeCnt >= 1000)
		{
			break;
		}
	}
	usReturnData = SPI_DATA(SPI2);
	
	gpio_bit_write(GPIOA,GPIO_PIN_15,SET);
	
	return usReturnData;
}
static unsigned short GetSPI2Data_DMA(unsigned short usSendData)
{
	gpio_bit_write(GPIOA,GPIO_PIN_15,RESET);
	
	for(unsigned char uci = 0; uci < 12; uci++)
	{
		usOutTimeCnt = 0;
		while((SPI_STAT(SPI2) & 0x02) == 0)
		{
			usOutTimeCnt++;
			if(usOutTimeCnt >= 1000)
			{
				break;
			}
		}
		SPI_DATA(SPI2) = usSendData;
	}

	gpio_bit_write(GPIOA,GPIO_PIN_15,SET);
	
	unsigned int uiReturnData = 0;
	for(unsigned char uci = 0; uci < 10; uci++)
	{
		uiReturnData += usSPI2RecvData[uci];
	}
	
	return (uiReturnData & 0xffff);
}
static void SPIInit(void)
{
	//DMAInit();
	SPI0Init();
	SPI1Init();
	SPI2Init();
}
/*
	模拟SPI通信
*/
//#define SPI0_SCK_HIGH	gpio_bit_write(GPIOA,GPIO_PIN_5,SET)
//#define SPI0_SCK_LOW	gpio_bit_write(GPIOA,GPIO_PIN_5,RESET)
//#define SPI0_MOSI_HIGH	gpio_bit_write(GPIOA,GPIO_PIN_7,SET)
//#define SPI0_MOSI_LOW	gpio_bit_write(GPIOA,GPIO_PIN_7,RESET)
//#define SPI0_MISO		gpio_input_bit_get(GPIOA,GPIO_PIN_6)

//#define SPI2_SCK_HIGH	gpio_bit_write(GPIOB,GPIO_PIN_3,SET)
//#define SPI2_SCK_LOW	gpio_bit_write(GPIOB,GPIO_PIN_3,RESET)
//#define SPI2_MOSI_HIGH	gpio_bit_write(GPIOB,GPIO_PIN_5,SET)
//#define SPI2_MOSI_LOW	gpio_bit_write(GPIOB,GPIO_PIN_5,RESET)
//#define SPI2_MISO		gpio_input_bit_get(GPIOB,GPIO_PIN_4)

#define	DELAY(n)		ps_Systick->delay_us(n)

static void SPI0Init_Simulation(void)
{
//	rcu_periph_clock_enable(RCU_GPIOA);
//	
//	//NSS
//	gpio_init(GPIOA,GPIO_OTYPE_PP,GPIO_OSPEED_50MHZ,GPIO_PIN_4);
//	//SCK
//	gpio_init(GPIOA,GPIO_OTYPE_PP,GPIO_OSPEED_50MHZ,GPIO_PIN_5);
//	//MISO
//	gpio_init(GPIOA,GPIO_MODE_IN_FLOATING,GPIO_OSPEED_50MHZ,GPIO_PIN_6);
//	//MOSI
//	gpio_init(GPIOA,GPIO_OTYPE_PP,GPIO_OSPEED_50MHZ,GPIO_PIN_7);
//	
//	//直接拉低片选
//	gpio_bit_write(GPIOA,GPIO_PIN_4,RESET);
}
static unsigned short GetSPI0Data_Simulation(unsigned short usData)
{
	unsigned short usReturnData = 0;
//	
//	for(unsigned char ucCnt = 0; ucCnt < 16; ucCnt++)
//	{
//		SPI0_SCK_LOW;
//		DELAY(1);
//		
//		//接收数据，放在下降沿之后
//		usReturnData <<= 1;
//		if(SPI0_MISO)
//		{
//			usReturnData |= 0x0001;
//		}
//		else
//		{
//			usReturnData &= 0xfffe;
//		}
//		
//		//发送数据，要放在上升沿之前
//		if(usData & 0x8000)
//		{
//			SPI0_MOSI_HIGH;
//		}
//		else
//		{
//			SPI0_MOSI_LOW;
//		}
//		
//		DELAY(1);
//		SPI0_SCK_HIGH;
//		DELAY(1);

//		usData <<= 1;
//	}
//	
//	SPI0_SCK_HIGH;
//	
	return usReturnData;
}
static void SPI2Init_Simulation(void)
{
//	rcu_periph_clock_enable(RCU_AF);	
//	rcu_periph_clock_enable(RCU_GPIOA);
//	rcu_periph_clock_enable(RCU_GPIOB);
//	
//	//PA15 remap
//	gpio_pin_remap_config(GPIO_SWJ_SWDPENABLE_REMAP,ENABLE);
//	
//	//NSS
//	gpio_init(GPIOA,GPIO_OTYPE_PP,GPIO_OSPEED_50MHZ,GPIO_PIN_15);
//	//SCK
//	gpio_init(GPIOB,GPIO_OTYPE_PP,GPIO_OSPEED_50MHZ,GPIO_PIN_3);
//	//MISO
//	gpio_init(GPIOB,GPIO_OTYPE_OD,GPIO_OSPEED_50MHZ,GPIO_PIN_4);
//	//MOSI
//	gpio_init(GPIOB,GPIO_OTYPE_PP,GPIO_OSPEED_50MHZ,GPIO_PIN_5);
//	
//	//直接拉低片选
//	gpio_bit_write(GPIOA,GPIO_PIN_15,RESET);
}
static unsigned short GetSPI2Data_Simulation(unsigned short usData)
{
	unsigned short usReturnData = 0;
//	
//	for(unsigned char ucCnt = 0; ucCnt < 16; ucCnt++)
//	{
//		SPI2_SCK_LOW;
//		DELAY(1);
//		
//		//接收数据，放在下降沿之后
//		usReturnData <<= 1;
//		if(SPI2_MISO)
//		{
//			usReturnData |= 0x0001;
//		}
//		else
//		{
//			usReturnData &= 0xfffe;
//		}
//		
//		//发送数据，要放在上升沿之前
//		if(usData & 0x8000)
//		{
//			SPI2_MOSI_HIGH;
//		}
//		else
//		{
//			SPI2_MOSI_LOW;
//		}
//		
//		DELAY(1);
//		SPI2_SCK_HIGH;
//		DELAY(1);

//		usData <<= 1;
//	}
//	
//	SPI2_SCK_HIGH;
//	
	return usReturnData;
}
static void SPIInit_Simulation(void)
{
	SPI0Init_Simulation();
	SPI2Init_Simulation();
}
