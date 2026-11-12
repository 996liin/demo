#ifndef __FWDGT_H_
#define __FWDGT_H_

typedef struct
{
	void (*FWDGTInit)(unsigned char ucS);
	void (*FWDGTProcess)(void);
		
}Struct_FWDGT;
extern  Struct_FWDGT *ps_FWDGT;



#endif
