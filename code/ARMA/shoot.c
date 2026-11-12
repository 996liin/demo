#include "headfile.h"
static void LaserWeaponCtrl(unsigned char ucEnable);
static void LaserOpen(void);
static void LaserClose(void);
static void PowerSet(unsigned char nPercent);
struct_weapon s_weapon =
{
	 .Protect_time_count = PROTECT_TIME,
	 .EnableFlag = 0,
   .LaserWeaponCtrl = LaserWeaponCtrl,
	 .LaserOpen = LaserOpen,
	 .LaserClose = LaserClose,
	 .PowerSet = PowerSet,
};


static void LaserWeaponCtrl(unsigned char ucEnable)
{
   if(ucEnable == 0x01)
	 {
	    s_weapon.EnableFlag = 1;
		  s_weapon.Protect_time_count = PROTECT_TIME;
	 }
	 else if(ucEnable == 0x02)
	 {
	    s_weapon.EnableFlag = 0;
	 }
}
static void LaserOpen(void)
{
  unsigned char ucCom[5] = {0x55,0x00,0x01,0x00,0x00};
	
	ps_CAN1->CANSendData_Stand(0x12,ucCom,5);
}
static void LaserClose(void)
{
  //unsigned char ucCom[7] = {0xff,0x01,0x01,0x04,0x01,0x00,0x00};
}
static void PowerSet(unsigned char nPercent)
{
  unsigned char ucCom[5] = {0x55,0x00,0x09,0x00,0x00};
	if((nPercent >= 1)&&(nPercent <=70))
	{
	  ucCom[3] = nPercent;
	  ps_CAN1->CANSendData_Stand(0x11,ucCom,5);
	}
	else{}
}


