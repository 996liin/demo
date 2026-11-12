#ifndef GNSSCTRL_H_
#define GNSSCTRL_H_

#include "headfile.h"

typedef struct _Information
{
	unsigned char Real_Locate;					//实时定位有效位
	unsigned char Located;					  	//定位有效位
	unsigned char Locate_Mode;					//定位模式 2D / 3D
	char UTC_Time[7];							//时间
	char UTC_Date[7];							//日期
	char Latitude[12];							//纬度
	char NS_Indicator;							//N=北半球,S=南半球
	char Longitude[13];							//经度
	char EW_Indicator;							//E=东经,W=西经
	double Speed;								//地面速率
	double Course;								//地面航向
	double PDOP;								//位置精度
	double HDOP;								//水平精度
	double VDOP;								//垂直精度
	double MSL_Altitude;						//MSL海拔高度
	unsigned char Use_EPH_Sum;		 			//使用的卫星数量
	unsigned char User_EPH[12];					//当前搜索到的卫星编号
	unsigned short EPH_State[12][4]; 			//当前使用的12颗卫星的编号、仰角、方位角、信噪比
} Information; //GNSS信息

typedef struct _Gnss
{
	Information info;
	void (*DataBack)(void);
	void (*init)(void);
} GNSS;

extern GNSS *ptr_Gnss;
extern GNSS *ptr_Gnss_test ;

#endif
