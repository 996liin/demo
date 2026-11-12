#ifndef _CCDCTRL_H_
#define _CCDCTRL_H_
#include "stdint.h"
#include "procotol.h"
#define MAXZOOM_CCD  6120

#define MAXZOOM_CAM_ZOOM  (16384.0)

typedef struct
{
	unsigned short zoomvalue;
	unsigned short focusvalue;
	void (*init)(void);
	void (*zoom_inquiry)(void);
	void (*focus_inquiry)(void);
	void (*Camera_GoToTarget)(unsigned char select, unsigned short value_target);
	void (*Process)(void);
	void (*Camera_ZoomTele)(void);
	void (*Camera_ZoomWide)(void);
	void (*Camera_FocusFar)(void);
	void (*Camera_FocusNear)(void);
	void (*Camera_IrisOpen)(void);
	void (*Camera_IrisClose)(void);
	void (*Camera_Stop)(void);
	void (*Cmd_Ctrl)(unsigned char cmd,unsigned short data);
	void (*ProcessInq)(void);
  void (*Set_PT_OSD)(unsigned short h,signed short v);
	void (*UartSendData)(unsigned char *pdata,unsigned short usdatalen);
	uint16_t (*Search_zoom_view)(uint16_t usCamPosi);
	unsigned char icr_open_flag;
	unsigned char fog_open_flag;
	unsigned char query_witch;
	void (*ProcessCmd)(Cam_Opt_Type cam_cmd, uint16_t data);
	unsigned char runstate;
	unsigned short stoptimecnt;
}struct_CCD;
typedef enum
{
  zoom,
	focus,
	stop,
	locate_zoom,
	locate_focus,
	null
}struct_runstate;
extern struct_CCD *ps_CCD;

typedef struct
{
	/*r热像参数*/
	unsigned char ucMode;
	unsigned char ucBrightness;
	unsigned char ucContrast;
	unsigned char ucGray;	//灰度增强
	unsigned char ucIDE_1;	//细节增强 3个参数
	unsigned char ucIDE_2;
	unsigned char ucIDE_3;
	unsigned char ucColor;
	unsigned char ucFilter1SW;	//时域滤波
	unsigned char ucFilter2SW;	//空域滤波
	unsigned char ucFilter1Value;		
	unsigned char ucFilter2Value;		
	unsigned char ucImage;		//镜像
	unsigned char ucCross;
	unsigned short usCrossX;
	unsigned short usCrossY;
	unsigned char ucAutoCheck;
	unsigned char ucAutoCheckTime;
	unsigned char ucDZoom;
	
	unsigned char ucDDE;

}struct_CCDPara;


/*设备参数*/
typedef struct DEVICEPARA
{
	unsigned char ucUpdateFlag;
	unsigned short usUpdateNum;
	unsigned char ucType;
	unsigned char ucVersion[4];
	unsigned int uiDate;
	
}struct_devicePara;
extern struct_devicePara *ps_devicePara;

#endif


