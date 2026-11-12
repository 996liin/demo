#ifndef	__POWEROFFSAVE_H_
#define	__POWEROFFSAVE_H_

typedef struct
{
	void (*PowerOffSaveRun)(void);
	void (*PowerOffSaveTimer)(void);
	void (*PowerOffSaveProcess)(MESSAGE Msg);
		
}struct_PowerOffSave;
extern struct_PowerOffSave *ps_PowerOffSave;

typedef struct
{
	unsigned char ucPowerOffSaveSW;
	
	
}struct_PowerOffSavePara;

#endif

