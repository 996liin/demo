#include "headfile.h"
static void Init(void); 
//static void QclOpen(void);
//static void QclClose(void);
//static void QclTele(void);
//static void QclWide(void);
//static void QclStop(void);
static void QclCtrl(unsigned char cmd,unsigned short usdatalen);
static struct_qcl  inst_qcl_ctrl =
{
  .Init           = Init,
	.Ctrl           = QclCtrl,
	.usZoomValue    = 0x1234
};
struct_qcl *ps_qcl = &inst_qcl_ctrl;
static void Init(void)
{
   inst_qcl_ctrl.can = ps_CAN0;
	 inst_qcl_ctrl.can->CANInit();
}
//static void QclOpen(void)
//{
//  unsigned char ucCom[5] = {0xe7,0x00,0x01,0x00,0x00};
//	ps_CAN1->CANSendData_Stand(0x12,ucCom,5);
//}
//static void QclClose(void)
//{
//  unsigned char ucCom[5] = {0xe7,0x00,0x00,0x00,0x00};
//	ps_CAN1->CANSendData_Stand(0x12,ucCom,5);
//}
//static void QclTele(void)
//{
//  unsigned char ucCom[5] = {0xe7,0x00,0x04,0x00,0x00};
//	ps_CAN1->CANSendData_Stand(0x12,ucCom,5);
//}
//static void QclWide(void)
//{
//  unsigned char ucCom[5] = {0xe7,0x00,0x05,0x00,0x00};
//	ps_CAN1->CANSendData_Stand(0x12,ucCom,5);
//}
//static void QclStop(void)
//{
//  unsigned char ucCom[5] = {0xe7,0x00,0x06,0x00,0x00};
//	ps_CAN1->CANSendData_Stand(0x12,ucCom,5);
//}
static void QclCtrl(unsigned char cmd,unsigned short usdatalen)
{
  unsigned char ucCom[6] = {0xe7,0x00,0x06,0x00,0x00,0x7e};
	ucCom[2] = cmd;
	ucCom[3] = (usdatalen>>8)&0xff;
	ucCom[4] = usdatalen&0xff;
	inst_qcl_ctrl.can->CANSendData_Stand(SEMD_MSG_ID,ucCom,6);
}



