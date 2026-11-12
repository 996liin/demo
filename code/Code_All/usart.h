#ifndef	__USART_H_
#define	__USART_H_

typedef struct
{
	void (*UartInit)(void);
	void (*Usart0_SendData)(unsigned char *pdata,unsigned short usdatalen);
	void (*Usart1_SendData)(unsigned char *pdata,unsigned short usdatalen);
	void (*Usart2_SendData)(unsigned char *pdata,unsigned short usdatalen);
	void (*Uart3_SendData)(unsigned char *pdata,unsigned short usdatalen);
	void (*Uart4_SendData)(unsigned char *pdata,unsigned short usdatalen);
	void (*Usart5_SendData)(unsigned char *pdata,unsigned short usdatalen);
	void (*Uart6_SendData)(unsigned char *pdata,unsigned short usdatalen);
	void (*Uart7_SendData)(unsigned char *pdata,unsigned short usdatalen);
		
}Struct_Usart;
typedef struct
{
	unsigned char		Length;
	unsigned char		CheckSum;
	unsigned char		Rxd[25];
	unsigned char    RxdCache[25];
	unsigned char		RxdCnt;
	unsigned char	FlagRxdEnd;
} USER_USART_RX;
extern Struct_Usart *ps_Usart;
extern USER_USART_RX s_COMLRF;
extern USER_USART_RX s_COMCCD;
extern  void DataBack_Angle(void);
extern void OSD_Refresh(void);
#endif

