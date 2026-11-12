#ifndef _PTCTRL_H_
#define _PTCTRL_H_
#include "typedef.h"
#include "UartFunction.h"
typedef struct
{
	void (*init)(void);
	_tUartFunc *uart;
	s32 H_Angle;
	s32 V_Angle;
	s32 R_Angle;
	u8  slow_move_flag;
	u8  pt_auto_speed;   //³¤½¹ÏÞËÙ
	u8  start_track_flag;
	u8  databack_enable_flag;
	void (*UartSendData)(unsigned char *pdata,unsigned short usdatalen);
	void (*databack_pelcod)(unsigned char device,unsigned char addr);
	unsigned char databack_com;
	unsigned char reboot_flag;
}struct_Pt;

typedef enum
{
  P = 1,
	T   ,
	CCDZOOM  ,
	CCDFOCUS,
	IRZOOM,
	IRFOCUS
}device;
typedef enum
{
  COMDEBUG = 1,
	COMIR   ,
	COMCCD  ,
	COM3588 ,
}comdevice;
extern struct_Pt *ps_Pt;

#endif

