#include "headfile.h"

#define ChannelCamZoom	1
#define ChannelCamFocus	2
#define ChannelIrZoom	3
#define ChannelIrFocus	4
#define ChannelLaser	5
#define ChannelLight	6

unsigned short usCamZoomAD = 0,usCamFocusAD = 0,usIrZoomAD = 0,usIrFocusAD = 0,usLaserAD = 0,usLightAD = 0;
unsigned char ucReadADCValueCnt = 0,ucReadADCValueStep = 0;
unsigned int uiADC124S21Value = 0;
unsigned short ustestCnt = 0;
static unsigned short GetCamZoomAD(void);
static unsigned short GetCamFocusAD(void);
static unsigned short GetIrZoomAD(void);
static unsigned short GetIrFocusAD(void);
static unsigned short GetLaserAD(void);
static unsigned short GetLightAD(void);
static void ADCTimer(void);
static void ADCint(void);
static Struct_ADC s_ADC = 
{
	.GetCamZoomAD = GetCamZoomAD,
	.GetCamFocusAD = GetCamFocusAD,
	.GetIrZoomAD = GetIrZoomAD,
	.GetIrFocusAD = GetIrFocusAD,
	.GetLaserAD = GetLaserAD,
	.GetLightAD = GetLightAD,
	.ADCTimer = ADCTimer,
	.ADCint = ADCint,
};
Struct_ADC *ps_ADC = &s_ADC;

uint16 gt_adc_val[10][2];  
static void ADCint(void)
	{
//	   dma_single_data_parameter_struct dma_single_data_parameter;
//	   rcu_periph_clock_enable(RCU_GPIOA);
//	   rcu_periph_clock_enable(RCU_ADC0);
//	   rcu_periph_clock_enable(RCU_DMA1);
//	   adc_clock_config(ADC_ADCCK_PCLK2_DIV4);
//		 gpio_mode_set(GPIOA, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO_PIN_4); // PA4
//		 gpio_mode_set(GPIOA, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO_PIN_5); // PA4
//		 ADC_Config();
//     ADC_DMAConfig();
		   dma_single_data_parameter_struct dma_single_data_parameter;
	   rcu_periph_clock_enable(RCU_GPIOA);
	   rcu_periph_clock_enable(RCU_ADC0);
	   rcu_periph_clock_enable(RCU_DMA1);
	   adc_clock_config(ADC_ADCCK_PCLK2_DIV4);

	   gpio_mode_set(GPIOA, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO_PIN_4); // PA4
		 gpio_mode_set(GPIOA, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO_PIN_4); // PA5
	   adc_sync_mode_config(ADC_SYNC_MODE_INDEPENDENT);
	   adc_special_function_config(ADC0, ADC_CONTINUOUS_MODE, ENABLE);
	   adc_special_function_config(ADC0, ADC_SCAN_MODE, ENABLE);
	   adc_data_alignment_config(ADC0, ADC_DATAALIGN_RIGHT);
	   adc_channel_length_config(ADC0, ADC_ROUTINE_CHANNEL, 2);
	   adc_routine_channel_config(ADC0, 0, ADC_CHANNEL_4, ADC_SAMPLETIME_15);//PA4
		 adc_routine_channel_config(ADC0, 1, ADC_CHANNEL_5, ADC_SAMPLETIME_15);//PA5
			adc_resolution_config(ADC0, ADC_RESOLUTION_12B);
	   adc_external_trigger_config(ADC0, ADC_ROUTINE_CHANNEL, EXTERNAL_TRIGGER_DISABLE);
	   adc_dma_request_after_last_enable(ADC0);  
			adc_dma_mode_enable(ADC0);
			adc_enable(ADC0);
			ps_Systick->delay_ms(5);
	   adc_calibration_enable(ADC0);
	   dma_deinit(DMA1, DMA_CH0);
			dma_single_data_parameter.periph_addr = (uint32_t)(&ADC_RDATA(ADC0));	
			dma_single_data_parameter.periph_inc = DMA_PERIPH_INCREASE_DISABLE;	//
			dma_single_data_parameter.memory0_addr = (uint32_t)(gt_adc_val);	//

			dma_single_data_parameter.memory_inc = DMA_MEMORY_INCREASE_ENABLE;	//
			dma_single_data_parameter.periph_memory_width = DMA_PERIPH_WIDTH_16BIT;	//
			dma_single_data_parameter.direction = DMA_PERIPH_TO_MEMORY;		//
			dma_single_data_parameter.number = 20;				//
			dma_single_data_parameter.priority = DMA_PRIORITY_HIGH;			//
			dma_single_data_mode_init(DMA1, DMA_CH0, &dma_single_data_parameter);	//
			dma_channel_subperipheral_select(DMA1, DMA_CH0, DMA_SUBPERI0);
			dma_circulation_enable(DMA1, DMA_CH0);
			dma_channel_enable(DMA1, DMA_CH0);
			adc_software_trigger_enable(ADC0, ADC_ROUTINE_CHANNEL);
//	   
	}

unsigned short Get_Adc_Dma_Value(char CHx)
	{
	unsigned char i = 0;
	unsigned short AdcValue = 0;
	for(i=0;i<10;i++)
	{
	  AdcValue+=gt_adc_val[i][CHx];
	}
	AdcValue=AdcValue/10;
	return AdcValue;
	}
/*根据通道号读取AD数值，*/
static void ReadADC124S21Value(unsigned char ucChannel)
{	
#if 1	//库函数版本
	switch (ucChannel)
	{
		case ChannelCamZoom:
		{
			uiADC124S21Value = 0;
			
			for(unsigned char ucCnt = 0; ucCnt < 2; ucCnt++)
			{
				ps_SPI->GetSPI0Data(0x0000);
			}
			for(unsigned char ucCnt = 0; ucCnt < 10; ucCnt++)
			{
				uiADC124S21Value += ps_SPI->GetSPI0Data(0x0000);
			}
			
			usCamZoomAD = uiADC124S21Value / 10;
			
			break;
		}
		case ChannelCamFocus:
		{
			uiADC124S21Value = 0;
			
			for(unsigned char ucCnt = 0; ucCnt < 2; ucCnt++)
			{
				ps_SPI->GetSPI0Data(0x0800);
			}
			for(unsigned char ucCnt = 0; ucCnt < 10; ucCnt++)
			{
				uiADC124S21Value += ps_SPI->GetSPI0Data(0x0800);
			}
			
			usCamFocusAD = uiADC124S21Value / 10;
			
			break;
		}
		case ChannelIrZoom:
		{
			uiADC124S21Value = 0;
			
			for(unsigned char ucCnt = 0; ucCnt < 2; ucCnt++)
			{
				ps_SPI->GetSPI2Data(0x0000);
			}
			for(unsigned char ucCnt = 0; ucCnt < 10; ucCnt++)
			{
				uiADC124S21Value += ps_SPI->GetSPI2Data(0x0000);
			}
			
			usIrZoomAD = uiADC124S21Value / 10;
			
			break;
		}
		case ChannelIrFocus:
		{
			uiADC124S21Value = 0;
			
			for(unsigned char ucCnt = 0; ucCnt < 2; ucCnt++)
			{
				ps_SPI->GetSPI2Data(0x0800);
			}
			for(unsigned char ucCnt = 0; ucCnt < 10; ucCnt++)
			{
				uiADC124S21Value += ps_SPI->GetSPI2Data(0x0800);
			}
			
			usIrFocusAD = uiADC124S21Value / 10;
			
			break;
		}
		case ChannelLaser:
		{
			#if LASERTYPE != LASER_MODULE
	
				uiADC124S21Value = 0;
				
				for(unsigned char ucCnt = 0; ucCnt < 2; ucCnt++)
				{
					ps_SPI->GetSPI0Data(0x1000);
				}
				for(unsigned char ucCnt = 0; ucCnt < 10; ucCnt++)
				{
					uiADC124S21Value += ps_SPI->GetSPI0Data(0x1000);
				}
				
				usLaserAD = uiADC124S21Value / 10;
			
			#endif
	
			break;
		}
		case ChannelLight:
		{
			uiADC124S21Value = 0;
			
			for(unsigned char ucCnt = 0; ucCnt < 2; ucCnt++)
			{
				ps_SPI->GetSPI0Data(0x1800);
			}
			for(unsigned char ucCnt = 0; ucCnt < 10; ucCnt++)
			{
				uiADC124S21Value += ps_SPI->GetSPI0Data(0x1800);
			}
			
			usLightAD = uiADC124S21Value / 10;
			
			break;
		}
		default:
		{
			break;
		}
	}
#else	//模拟版本
	switch (ucChannel)
	{
		case ChannelCamZoom:
		{
			for(unsigned char ucCnt = 0; ucCnt < 2; ucCnt++)
			{
				ps_SPI->GetSPI0Data_Simulation(0x0000);
			}
			for(unsigned char ucCnt = 0; ucCnt < 10; ucCnt++)
			{
				uiADC124S21Value += ps_SPI->GetSPI0Data_Simulation(0x0000);
			}
			
			usCamZoomAD = uiADC124S21Value / 10;
			
			break;
		}
		case ChannelCamFocus:
		{
			for(unsigned char ucCnt = 0; ucCnt < 2; ucCnt++)
			{
				ps_SPI->GetSPI0Data_Simulation(0x0800);
			}
			for(unsigned char ucCnt = 0; ucCnt < 10; ucCnt++)
			{
				uiADC124S21Value += ps_SPI->GetSPI0Data_Simulation(0x0800);
			}
			
			usCamFocusAD = uiADC124S21Value / 10;
			
			break;
		}
		
		case ChannelIrZoom:
		{
			for(unsigned char ucCnt = 0; ucCnt < 2; ucCnt++)
			{
				ps_SPI->GetSPI2Data_Simulation(0x0000);
			}
			for(unsigned char ucCnt = 0; ucCnt < 10; ucCnt++)
			{
				uiADC124S21Value += ps_SPI->GetSPI2Data_Simulation(0x0000);
			}
			
			usIrZoomAD = uiADC124S21Value / 10;
			
			break;
		}
		case ChannelIrFocus:
		{
			for(unsigned char ucCnt = 0; ucCnt < 2; ucCnt++)
			{
				ps_SPI->GetSPI2Data_Simulation(0x0800);
			}
			for(unsigned char ucCnt = 0; ucCnt < 10; ucCnt++)
			{
				uiADC124S21Value += ps_SPI->GetSPI2Data_Simulation(0x0800);
			}
			
			usIrFocusAD = uiADC124S21Value / 10;
			
			break;
		}
		case ChannelLaser:
		{
			for(unsigned char ucCnt = 0; ucCnt < 2; ucCnt++)
			{
				ps_SPI->GetSPI0Data_Simulation(0x1000);
			}
			for(unsigned char ucCnt = 0; ucCnt < 10; ucCnt++)
			{
				uiADC124S21Value += ps_SPI->GetSPI0Data_Simulation(0x1000);
			}
			
			usLaserAD = uiADC124S21Value / 10;
			
			break;
		}
		case ChannelLight:
		{
			for(unsigned char ucCnt = 0; ucCnt < 2; ucCnt++)
			{
				ps_SPI->GetSPI0Data_Simulation(0x1800);
			}
			for(unsigned char ucCnt = 0; ucCnt < 10; ucCnt++)
			{
				uiADC124S21Value += ps_SPI->GetSPI0Data_Simulation(0x1800);
			}
			
			usLightAD = uiADC124S21Value / 10;
			
			break;
		}
		default:
		{
			break;
		}
	}
#endif
}
static void ADCTimer(void)
{
	if(ustestCnt)
		ucReadADCValueCnt++;
}

static unsigned short GetCamZoomAD(void)
{
	uiADC124S21Value = 0;
	
//	for(unsigned char ucCnt = 0; ucCnt < 2; ucCnt++)
//	{
//		ps_SPI->GetSPI0Data(0x0000);
//	}
//	for(unsigned char ucCnt = 0; ucCnt < 10; ucCnt++)
//	{
//		uiADC124S21Value += ps_SPI->GetSPI0Data(0x0000);
//	}
	
	usCamZoomAD = Get_Adc_Dma_Value(0);
	
	return usCamZoomAD;
}
static unsigned short GetCamFocusAD(void)
{
	uiADC124S21Value = 0;
	
//	for(unsigned char ucCnt = 0; ucCnt < 2; ucCnt++)
//	{
//		ps_SPI->GetSPI0Data(0x0800);
//	}
//	for(unsigned char ucCnt = 0; ucCnt < 10; ucCnt++)
//	{
//		uiADC124S21Value += ps_SPI->GetSPI0Data(0x0800);
//	}
	usCamFocusAD = Get_Adc_Dma_Value(1);
	
	return usCamFocusAD;
}
static unsigned short GetIrZoomAD(void)
{
	uiADC124S21Value = 0;
	
	for(unsigned char ucCnt = 0; ucCnt < 2; ucCnt++)
	{
		ps_SPI->GetSPI2Data(0x0000);
	}
	for(unsigned char ucCnt = 0; ucCnt < 10; ucCnt++)
	{
		uiADC124S21Value += ps_SPI->GetSPI2Data(0x0000);
	}
	
	usIrZoomAD = uiADC124S21Value / 10;
			
	return usIrZoomAD;
}
static unsigned short GetIrFocusAD(void)
{
	uiADC124S21Value = 0;
	
	for(unsigned char ucCnt = 0; ucCnt < 2; ucCnt++)
	{
		ps_SPI->GetSPI2Data(0x0800);
	}
	for(unsigned char ucCnt = 0; ucCnt < 10; ucCnt++)
	{
		uiADC124S21Value += ps_SPI->GetSPI2Data(0x0800);
	}
	
	usIrFocusAD = uiADC124S21Value / 10;
			
	return usIrFocusAD;
}
static unsigned short GetLaserAD(void)
{
	#if LASERTYPE != LASER_MODULE
	
	uiADC124S21Value = 0;
	
	for(unsigned char ucCnt = 0; ucCnt < 2; ucCnt++)
	{
		ps_SPI->GetSPI0Data(0x1000);
	}
	for(unsigned char ucCnt = 0; ucCnt < 10; ucCnt++)
	{
		uiADC124S21Value += ps_SPI->GetSPI0Data(0x1000);
	}
	
	usLaserAD = uiADC124S21Value / 10;
	
	#endif
	
	return usLaserAD;
}
static unsigned short GetLightAD(void)
{
	uiADC124S21Value = 0;
	
	for(unsigned char ucCnt = 0; ucCnt < 2; ucCnt++)
	{
		ps_SPI->GetSPI0Data(0x1800);
	}
	for(unsigned char ucCnt = 0; ucCnt < 10; ucCnt++)
	{

		uiADC124S21Value += ps_SPI->GetSPI0Data(0x1800);
	}
	usLightAD = uiADC124S21Value / 10;
	
	return usLightAD;
}

