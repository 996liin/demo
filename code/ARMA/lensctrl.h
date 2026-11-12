#ifndef __LENSCTRL_H_
#define __LENSCTRL_H_

#define LENS_CAM	1
#define LENS_IR	2

#define LENSRUN_TELE		1
#define LENSRUN_WIDE		2
#define LENSRUN_NEAR		3
#define LENSRUN_FAR			4
#define LENSRUN_STOP		0

#define SPEEDCYCLE		10	//ÖÜÆÚ20ms

typedef struct
{
	void (*init)(void);
	void (*LensSelfCheck)(void);
	void (*LensTimer)(void);
	void (*CallPreset)(unsigned short usPreNum);
	void (*CamAutoFocus)(void);
	void (*IrAutoFocus)(void);
	void (*SetCamZoomPosi)(unsigned short usTargetValue);
	void (*SetCamFocusPosi)(unsigned short usTargetValue);
	void (*SetIrZoomPosi)(unsigned short usTargetValue);
	void (*SetIrFocusPosi)(unsigned short usTargetValue);
	void (*LensCam)(unsigned char cmd);
	void (*LensLaser)(unsigned char cmd);
	unsigned short usZoomValue;
	unsigned short usFocusValue;
	unsigned char  ucCamSelfCheckFlag;
		
}Struct_Lens;
extern Struct_Lens *ps_Lens;

typedef struct
{
	unsigned char ucCamAutoFocusFlag;
	unsigned char ucIrAutoFocusFlag;
	unsigned char ucTempTrigAutoFocusFlag;
	unsigned char ucTempTrigAutoFocusTemp;
}Struct_LensPara;


typedef struct
{
	unsigned char ucCamZoomArr;
	unsigned char ucCamFocusArr;
	unsigned char ucIrZoomArr;
	unsigned char ucIrFocusArr;
	unsigned char ucPtArr;
	
	unsigned char ucPreRunFlag;
	unsigned short usPreNum;
	
}struct_Arr;
extern struct_Arr *ps_Arr;
extern void LocatLensPosi(void);
#endif

