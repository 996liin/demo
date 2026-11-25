#ifndef _IAP_H_
#define _IAP_H_


typedef struct STRUCT_IAP
{
	void (*IAPTimer)(void);
	void (*IAPProcess)(void);
}Struct_IAP;
extern Struct_IAP *ps_IAP;

#define UPDATE_PACK_DATA_SIZE		523
#define UPDATE_PACK_BUFFER_SIZE		512

typedef struct STRUCT_UPDATEPACK
{
	unsigned char ucHead;
	unsigned char ucType;
	unsigned char ucAddr[4];
	unsigned char ucSize[2];
	unsigned char ucBuffer[UPDATE_PACK_BUFFER_SIZE];
	unsigned char ucSum[3];
	
}Struct_UpdatePack;

/*设备参数*/
typedef struct DEVICEPARA
{
	unsigned char ucUpdateFlag;
	unsigned short usUpdateNum;
	unsigned char ucType;
	unsigned char ucVersion[4];
	unsigned int uiDate;
	
}struct_devicePara;


#endif

