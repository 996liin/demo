#include "headfile.h"

unsigned fsmctrl = 0;
void ProtAnal(void)
{
//  unsigned char data[19] = {0};	
//  memcpy(data,ps_COM3588->Rxd,19);
//	switch(data[2])
//	{
//		case 0x01://转台
//		{ 
//			ps_Usart->Uart6_SendData(data,19);
//			break;
//		}
//		case 0x02://可见光
//		{
//				ps_CCD->Cmd_Ctrl(data[4],data[9]<<8|data[10]);
//				break;
//		}
//		case 0x03://热成像
//		{
//		    ps_IR->Cmd_Ctrl(data[4],data[9]<<8|data[10]);
//				break;
//		}
//		case 0x04://选通激光照明
//		{
//			  ps_Gate->CmdCtrl(data[4],data[9]<<8|data[10]);
//				break;
//		}
//		case 0x05://白光照明
//		{
//				
//				break;
//		}
//		case 0x06://指示光
//		{
//		    ps_IR->Cmd_Ctrl(data[4],data[9]<<8|data[10]);
//				break;
//		}
//		case 0x07://测距机
//		{
//			  ps_Lrf->LrfCtrl(data[4],data[9]<<8|data[10]);
//				break;
//		}
//	  default:
//			break;
//	}
}
void KeyBoardCmd(void)
{
  
}
