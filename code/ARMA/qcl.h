#ifndef __QCL_H_
#define __QCL_H_
#define RECV_MSG_ID     0x202
#define SEMD_MSG_ID     0x204
typedef struct
{
	 struct_CAN   *can;
   void (*Init)(void); 
//	 void (*Open)(void);
//	 void (*Close)(void);
//	 void (*Tele)(void);
//	 void (*Wide)(void);
//	 void (*Stop)(void);
	void (*Ctrl)(unsigned char cmd,unsigned short usdatalen);
	unsigned short usZoomValue;
}struct_qcl;
extern struct_qcl *ps_qcl;



#endif



