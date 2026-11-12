#ifndef _LASER_H_
#define _LASER_H_

typedef struct
{
  void (*CmdCtrl)(unsigned char cmd,unsigned short data);
	void (*init)(void);
	void (*UartSendData)(unsigned char *pdata,unsigned short usdatalen);
	void (*Process)(void);
	unsigned char OpenFlag;
	unsigned char AutoCtrlTimeout;
	unsigned short position;
}struct_Laser;
extern struct_Laser *ps_Laser;

#endif
