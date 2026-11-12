#ifndef _LRF_H_
#define _LRF_H_

typedef struct
{
	void (*init)(void);
	void (*LrfCtrl)(unsigned char cmd,unsigned short data);
	void (*LrfDataBack)(void);
	unsigned int DistanceOne;
	unsigned int DistanceTwo;
	unsigned int DistanceThree;//3个目标由近到远
	void (*UartSendData)(unsigned char *pdata,unsigned short usdatalen);
}struct_Lrf;
extern struct_Lrf *ps_Lrf;

#endif

