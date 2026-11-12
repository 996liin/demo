#ifndef __ADC_H_
#define __ADC_H_


typedef struct
{
	unsigned short (*GetCamZoomAD)(void);
	unsigned short (*GetCamFocusAD)(void);
	unsigned short (*GetIrZoomAD)(void);
	unsigned short (*GetIrFocusAD)(void);
	unsigned short (*GetLaserAD)(void);
	unsigned short (*GetLightAD)(void);	
	
	void (*ADCTimer)(void);
	void (*ADCint)(void);
}Struct_ADC;
//extern uint16 gt_adc_va[2];
extern Struct_ADC *ps_ADC;
/*????*/
/*??????*/
/*ADC?? BEGIN*/
#define USER_ADCx                        ADC0                     //ADC??:ADC0 ADC1 ADC2
#define USER_ADCx_RCU                    RCU_ADC0                 //ADC????:RCU_ADC0 RCU_ADC1 RCU_ADC2
#define USER_ADC_CHANNEL_AMOUNT          (2)                      //????:1~16
#define USER_ADC_CLK_METHOD              ADC_ADCCK_PCLK2_DIV4     //??ADC????,ADC ????????? 14M,??"gd32f4xx_adc.h"
#define USER_ADC_SAMPLETIME_PERIOD       ADC_SAMPLETIME_15        //????:ADC_SAMPLETIME_3 ADC_SAMPLETIME_15 ADC_SAMPLETIME_28 ADC_SAMPLETIME_56 
															      //         ADC_SAMPLETIME_84 ADC_SAMPLETIME_112 ADC_SAMPLETIME_144 ADC_SAMPLETIME_480
															   
#define USER_DMA_ADC_CHANNEL             DMA_CH0                  //DMA??:ADC0:DMA_CH0 DMA_CH4 
                                                                  //         ADC1:DMA_CH2 DMA_CH3 
															      //         ADC2:DMA_CH0 DMA_CH3
															   
#define USER_DMA_ADC_SUBPERI             DMA_SUBPERI0             //DMA???:ADC0:DMA_SUBPERI0 
                                                                  //          ADC1:DMA_SUBPERI1 
															      //          ADC2:DMA_SUBPERI2
																  
#define DMA_ADC_IRQHandler               DMA1_Channel0_IRQHandler //DMA1_Channel0_IRQHandler         
                                                                  //DMA1_Channel1_IRQHandler       
                                                                  //DMA1_Channel2_IRQHandler 
				                                                  //DMA1_Channel3_IRQHandler 
                                                                  //DMA1_Channel4_IRQHandler       
																  
#define DMA_ADC_IRQn                     DMA1_Channel0_IRQn       //DMA0_Channel0_IRQn         
                                                                  //DMA0_Channel1_IRQn       
                                                                  //DMA0_Channel2_IRQn 
				                                                  //DMA0_Channel3_IRQn 
                                                                  //DMA0_Channel4_IRQn       									 
#define DMA_ADC_PRIORITY_PRE             0                        //?????
#define DMA_ADC_PRIORITY_SUB             1                        //????
/*ADC?? END*/

#endif

