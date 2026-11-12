#ifndef __SHOOT_H_
#define __SHOOT_H_

typedef struct
{
   unsigned char EnableFlag;
	 uint16 Protect_time_count;//激光武器打开时间保护
   void (*LaserWeaponCtrl)(unsigned char ucEnable);
	 void (*LaserOpen)(void);
	 void (*LaserClose)(void);
	 void (*PowerSet)(unsigned char nPercent);

}struct_weapon;

#define PROTECT_TIME  10000//暂定保护时间10秒
extern struct_weapon s_weapon;
#endif



