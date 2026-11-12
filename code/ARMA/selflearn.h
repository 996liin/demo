#ifndef _SELFLEARN_H_
#define _SELFLEARN_H_

typedef struct 
{
	void (*SelfLearnInit)(unsigned char ucOpera);
	void (*SelfLearnTimer)(void);
	void (*SelfLearnProcess)(MESSAGE Msg);
	
}Struct_SelfLearn;
extern Struct_SelfLearn *ps_SelfLearn;

//每个点的状态
typedef enum
{
	Ststus_Null = 0,
	Status_Start,
	Status_CamZoom,
	Status_CamFocus,
	Status_IrZoom,
	Status_IrFocus,
	Status_PtH,
	Status_PtV,
	Status_PtHV,
	Status_Stop,	
	Status_End,	
}PointStatus;	

//第一个点的数据
typedef struct 
{
	unsigned short usHAngle;
	unsigned short usVAngle;
	unsigned short usCamZoom;
	unsigned short usCamFocus;
	unsigned short usIrZoom;
	unsigned short usIrFocus;
}Struct_FirstPointValue;

//每个点的数据
typedef struct 
{
	PointStatus eStatus;
	unsigned short usData1;
	unsigned short usData2;
	unsigned char ucPtSpeed;
	unsigned short usTime;	//此时间为是上一个点的停留时间
	
}Struct_PointValue;

//参数
typedef struct 
{
	unsigned char ucSetFlag;
	unsigned char ucPointNum;
	Struct_FirstPointValue s_FirstPointValue;
	Struct_PointValue s_PointValue[100];
}Struct_SelfLearnPara;

#endif

