#ifndef	__KEEPWATCH_H_
#define	__KEEPWATCH_H_

typedef struct
{
	void (*KWInit)(unsigned char ucOpera);
	void (*KWTimer)(void);
	void (*KWProcess)(MESSAGE Msg);
		
}struct_KW;
extern struct_KW *ps_KW;

typedef struct
{
	unsigned char ucKWSW;
	unsigned char ucKWTime;
	unsigned char ucOpera;
	unsigned char ucKWAFSW;
	unsigned char ucKWAFTime;
	
}struct_KWPara;

#endif

