
#include "headfile.h"
static void init(void);
static void Process(void);
static unsigned char selfcheck_timeout_count = 0;
static unsigned char open_timeout_count = 0;
static _tWiper inst_Wiper =
{
  .init           = init,
	.Process        = Process,
	.open_flag      = 0,
	.switch_flag    = 0,
	.selfcheck_flag = 1,
	.ctrl_timeout_flag = 0,

};
_tWiper *ptr_Wiper = &inst_Wiper;

static void init(void)
{

}
static void Process(void)
{
	
	 if(inst_Wiper.ctrl_timeout_flag == 1)//100ms´ø¿í
	 {
	   inst_Wiper.ctrl_timeout_flag = 0;
		 if(inst_Wiper.selfcheck_flag)
		 {
				 selfcheck_timeout_count++;
				 static u8 uc_com[12]={0x55,0xAA,0x02,0x00,0x03,0x00,0x00,0x00,0x01,0x01,0x00,0xF0};
				 ps_Usart->Uart3_SendData(uc_com,12);
				 if(selfcheck_timeout_count>20)
				 {
					 selfcheck_timeout_count = 0;
					 inst_Wiper.selfcheck_flag = 0;
				 }
		 }
		 else
		 {
				 if(inst_Wiper.open_flag == 1)
				 {
					 open_timeout_count++;
					 static u8 uc_com[12]={0x55,0xAA,0x02,0x00,0x03,0x00,0x00,0x00,0x01,0x01,0x00,0xF0};
					 ps_Usart->Uart3_SendData(uc_com,12);
					 if(open_timeout_count >= 200)//20s
					 {
					   open_timeout_count = 0;
						 inst_Wiper.open_flag = 0;
					 }
				 }
				 else if(inst_Wiper.open_flag == 0)
				 {
					 static u8 uc_com[12]={0x55,0xAA,0x02,0x00,0x03,0x00,0x00,0x00,0x01,0x00,0x00,0xF0};
					 ps_Usart->Uart3_SendData(uc_com,12);
					 inst_Wiper.open_flag = 3;
				 } 
		  }
	 }
}

