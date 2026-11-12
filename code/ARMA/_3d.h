#ifndef _3D_H_
#define _3D_H_

typedef struct 
{
	unsigned char ucAlarmData[30];
	void (*_3DParaInit)(unsigned char ucOpera);
	void (*_3DProcess)(MESSAGE Msg);
}Struct_3D;
extern Struct_3D *ps_3D;


typedef struct 
{
	unsigned char ucManual3DSW;
	unsigned char unAuto3DSW;
}Struct_3DPara;

#endif

