#ifndef	__USART_H_
#define	__USART_H_

typedef struct
{
	void (*uartInit)(void);
	
	void (*dubugUsart0SendData)(unsigned char *pdata,unsigned short usDatalen);
	
	void (*cam232Usart1SendData)(unsigned char *pdata,unsigned short usDatalen);
	
	void (*wb422Usart2SendData)(unsigned char *pdata,unsigned short usDatalen);
	
	void (*af232Usart3SendData)(unsigned char *pdata,unsigned short usDatalen);
	
	void (*ir232Uart4SendData)(unsigned char *pdata,unsigned short usDatalen);
	
	void (*cam485Uart5SendData)(unsigned char *pdata,unsigned short usDatalen);
	
	void (*Uart6_SendData)(unsigned char *pdata,unsigned short usdatalen);
}Struct_Usart;

extern Struct_Usart *ps_Usart;


#endif

