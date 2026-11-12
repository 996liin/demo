#ifndef _LND_LIGHT_H_
#define _LND_LIGHT_H_

typedef struct
{
  void (*CmdCtrl)(unsigned char cmd,unsigned short data);
	void (*init)(void);
	void (*UartSendData)(unsigned char *pdata,unsigned short usdatalen);
	
}struct_light;
extern struct_light *ps_light;

#endif

