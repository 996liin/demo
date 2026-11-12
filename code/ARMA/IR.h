#ifndef __IR_H_
#define __IR_H_


#define  INTERNET
#define  MAXZOOMIR  2100

#define  ZOOMIRRANGE  (1350.0)


#define NEW_LENS_FOCUS_RADIO	(1.748217224355458)


typedef struct 
{
	unsigned short zoomvalueAF;//自动聚焦板的实际返回值
	unsigned short zoomvalueOffset;//减去最小值后的偏移值
	unsigned short focusvalue;
	unsigned char colorize_count;
	void (*init)(void);
	void (*zoom_inquiry)(void);
	void (*focus_inquiry)(void);
  void (*Camera_ZoomTele)(void);
	void (*Camera_ZoomWide)(void);
	void (*Camera_FocusFar)(void);
	void (*Camera_FocusNear)(void);
	void (*Camera_AutoFocusOn)(void);
	void (*Camera_AutoFocusOff)(void);
  void (*Camera_AutoFocusOnce)(void);
	void (*Camera_PreserCall)(unsigned short num);
	void (*Camera_zoom_Stop)(void);
	void (*Camera_focus_Stop)(void);
	void (*Camera_GoToTarget)(unsigned char select, unsigned short value_target);
	void (*Cmd_Ctrl)(unsigned char cmd,unsigned short data);
	void (*UartSendData)(unsigned char *pdata,unsigned short usdatalen);
  void (*set_colorize)(unsigned char select);
	void (*Elec_zoom)(unsigned char select);
  void (*Shutter_Ctrl)(unsigned char select);
	
	void (*SetLensSpeed)(unsigned char select, unsigned char data);
	
	unsigned char hot_open_flag;
	void (*ProcessInq)(void);
	unsigned char Elec_zoom_value;
	unsigned short ZoomMinAF;//保存自动聚焦板返回的最小值
	unsigned char runstate;
	unsigned short stoptimecnt;
	unsigned short locatetimecnt;
	unsigned char  focus_step_run;
	
}struct_IR;

extern struct_IR *ps_IR;
#endif

