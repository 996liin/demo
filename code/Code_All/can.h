#ifndef	__CAN_H_
#define	__CAN_H_

typedef struct
{
	void (*CANInit)(void);
	void (*CANSendData_Stand)(uint32_t ID,uint8_t* msg,uint32_t len);
	void (*CANSendData_Extend)(unsigned int uiSendID,unsigned char *ucData,unsigned char ucLen);
	
}struct_CAN;
extern struct_CAN *ps_CAN1;
extern struct_CAN *ps_CAN0;

#endif

