#include "headfile.h"
static void CmdCtrl(unsigned char cmd,unsigned short data);
static void init(void);

/*
	函数结构体-外部
*/
static struct_light s_light = 
{
	.CmdCtrl = CmdCtrl,
	.init        = init,
};
struct_light *ps_light = &s_light;


static void init(void)
{
	
}
static void CmdCtrl(unsigned char cmd,unsigned short data)
{
   
}
