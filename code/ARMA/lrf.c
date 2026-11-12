#include "headfile.h"
//#include "lrf.h"
static void init(void);
static void LrfCtrl(unsigned char cmd,unsigned short data);
static void LrfDataBack(void);
static struct_Lrf s_Lrf = 
{
	.init = init,
	.LrfCtrl = LrfCtrl,
	.LrfDataBack = LrfDataBack,
};
struct_Lrf *ps_Lrf = &s_Lrf;


static void init(void)
{
  s_Lrf.UartSendData = ps_Usart->Uart3_SendData;
}
static void LrfCtrl(unsigned char cmd,unsigned short data)
{

		
}
static void LrfDataBack(void)
{
  
}


