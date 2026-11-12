#ifndef UARTFUNCTION_H_
#define UARTFUNCTION_H_

#include "protocol_type.h"
#include "typedef.h"
typedef struct 
{
	Support_ProtocolList protoType;    			//协议类型
	uint8  u8Address;
	
	uint16 u16cmdType;      			   			//命令类型
	uint16 u16BufLen; 
	uint32 u32BufLen;       			   			//数据长度
	uint8 *u8pBuf;         			   			//数据
	
	void (*init)(void);
	void (*SendMessage)(Support_ProtocolList proto, u8 addr,uint16 u16msg,uint8 *buf, uint32 buflen);
	void (*ProtocolProcess)(void);
	void (*UartSendData)(unsigned char *pdata,unsigned short usdatalen);
}_tUartFunc;

extern _tUartFunc *ptr_3588Func;
extern _tUartFunc *ptr_ftrackFunc;
extern _tUartFunc *ptr_Ir232Func;
extern _tUartFunc *ptr_IrAfFunc;
extern _tUartFunc *ptr_PtFunc;
extern _tUartFunc *ptr_Ccd422Func;
extern _tUartFunc *ptr_TempCtrlFunc;
extern _tUartFunc *ptr_DebugFunc;
#endif
