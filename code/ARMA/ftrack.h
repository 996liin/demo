#ifndef __FTRACK_H_
#define __FTRACK_H_

typedef struct
{
	void (*init)(void);
	void (*UartSendData)(unsigned char *pdata,unsigned short usdatalen);
}struct_Ftrack;
extern struct_Ftrack *ps_Zynq;
#endif


