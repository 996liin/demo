#include "headfile.h"
static void GateInit(void);
static void CmdCtrl(unsigned short cmd, u8 *buf ,u32 len);
static struct_Gate s_Gate = 
{
	.GateInit = GateInit,
	.CmdCtrl = CmdCtrl,
};
struct_Gate *ps_Gate = &s_Gate;


static void GateInit(void)
{

}
static void CmdCtrl(unsigned short cmd, u8 *buf ,u32 len)
{
	//ptr_ftrackFunc->SendMessage(proto_private,cmd,buf,len-2);
  switch (cmd)
	{
		case 0xb103://zoom stop
		{	 
			  ps_Lens->LensLaser(3);
			  ps_Lens->LensCam(3);
				break;
		}
		case 0xb122://zoom+
		{
				ps_Lens->LensLaser(1);
				break;
		}
		case 0xb123://zoom-
		{
			  ps_Lens->LensLaser(2);
				break;
		}
		case 0xb104://focus near
		{
				ps_Lens->LensCam(1);
				break;
		}
		case 0xb105://focus far
		{
			  ps_Lens->LensCam(2);
				break;
		}
		case 0xb108://selfcheck
		{
			
			ps_Lens->LensSelfCheck();
		  break;
		}
		case 0xb109:
		{
			//ps_Lens->SetCamZoomPosi(data);
		  break;
		}
		case 0xb140://设置增益，使用10ch电源板6通道的da
//			static uint8_t data[20];
//		  static uint16_t temp;
//		  temp = *(buf+1) * 200;
//			data[0] = 0x06;
//			data[1] = (temp >> 8) & 0xff;
//			data[2] = (temp) & 0xff;
//		  ptr_3588Func->u8Address = 0x05;
//			ptr_ftrackFunc->SendMessage(proto_private,0xb10c,data,3);
			break;
		default:
			break;
	}	
}
