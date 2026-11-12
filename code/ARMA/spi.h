#ifndef	__SPI_H_
#define	__SPI_H_

typedef struct
{
	void (*SPIInit)(void);
	unsigned short (*GetSPI0Data)(unsigned short usSendData);
	unsigned short (*GetSPI0Data_DMA)(unsigned short usSendData);
	void (*SPI1_SendData)(unsigned char ucData);
	unsigned short (*GetSPI2Data)(unsigned short usSendData);
	unsigned short (*GetSPI2Data_DMA)(unsigned short usSendData);
		
	void (*SPIInit_Simulation)(void);
	unsigned short (*GetSPI0Data_Simulation)(unsigned short usData);
	unsigned short (*GetSPI2Data_Simulation)(unsigned short usData);
}Struct_SPI;

extern Struct_SPI *ps_SPI;

#endif

