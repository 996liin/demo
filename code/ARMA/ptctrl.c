#include "headfile.h"
//#include "lrf.h"
static void init(void);
static void databack_pelcod(unsigned char device,unsigned char addr);
static struct_Pt s_Pt = 
{
	.init             = init,
	.databack_pelcod  = databack_pelcod,
	.H_Angle          = 23456,
	.V_Angle          =-4523,
	.slow_move_flag   = 0,
	.start_track_flag = 0,
	.pt_auto_speed		=	0,
	.databack_enable_flag = 1,
	.reboot_flag       = 1,
};
struct_Pt *ps_Pt = &s_Pt;


static void init(void)
{
	s_Pt.uart = ptr_PtFunc;
	s_Pt.uart->init();
  s_Pt.UartSendData = ps_Usart->Uart4_SendData;
}
static void databack_pelcod(unsigned char device,unsigned char addr)
{
	 unsigned char ucCom[4] = {0}; 
   switch (device)
	 {
		 case P:
		 {
			 ucCom[1] = 0x59;
			 ucCom[2] = (ps_Pt->H_Angle) >> 8;
	     ucCom[3] = (ps_Pt->H_Angle) & 0xff;
       break;			 
		 }
		 case T:
		 {
			 ucCom[1] = 0x5b;
			 u16 angle_t;
			 if(s_Pt.databack_com == COMIR)
			 {
			   angle_t = (36000 - ps_Pt->V_Angle);
			 }
			 else if(ps_Pt->databack_com == COMDEBUG)
			 {
			   if(ps_Pt->V_Angle < 0)
				 {
				   angle_t = ps_Pt->V_Angle + 36000;
				 }
				 else 
				 {
				   angle_t = ps_Pt->V_Angle;
				 }
			 }
			 ucCom[2] = (angle_t) >> 8;
	     ucCom[3] = (angle_t) & 0xff;
       break;			 
		 }
		 case CCDZOOM:
		 {
			 ucCom[1] = 0x5d;
			 ucCom[2] = ps_CCD->zoomvalue >> 8;
	     ucCom[3] = ps_CCD->zoomvalue & 0xff;
       break;			 
		 }
		 case IRZOOM:
		 {
			 ucCom[1] = 0x5d;
			 ucCom[2] = ps_IR->zoomvalueOffset >> 8;
	     ucCom[3] = ps_IR->zoomvalueOffset & 0xff;
       break;			 
		 }
		 case CCDFOCUS:
		 {
			 ucCom[1] = 0x5f;
			 ucCom[2] = ps_CCD->focusvalue >> 8;
	     ucCom[3] = ps_CCD->focusvalue & 0xff;
       break;			 
		 }
		 case IRFOCUS:
		 {
			 ucCom[1] = 0x5f;
			 ucCom[2] = ps_IR->focusvalue >> 8;
	     ucCom[3] = ps_IR->focusvalue & 0xff;
       break;			 
		 }
	   default:break;
	 }
	 if(s_Pt.databack_com == COMDEBUG)
	 {
		 ps_Pt->databack_com = 0;
	   ptr_DebugFunc->SendMessage(protoPELCO_D,addr,0,ucCom,4);
	 }
	 else if(s_Pt.databack_com == COMIR)
	 {
//		 ps_Pt->databack_com = 0;
//	   ptr_Ir232Func->SendMessage(protoPELCO_D,addr,0,ucCom,4);
	 }
	 else if(s_Pt.databack_com == COMCCD)
	 {
		 ps_Pt->databack_com = 0;
	   ptr_Ccd422Func->SendMessage(protoPELCO_D,addr,0,ucCom,4);
	 }
	 else if(s_Pt.databack_com == COM3588)
	 {
		 ps_Pt->databack_com = 0;
	   ptr_3588Func->SendMessage(protoPELCO_D,addr,0,ucCom,4);
	 }
	 else{}
}
