#ifndef _CONFIG_H_
#define _CONFIG_H_

/*
	热像机芯
*/

#define IRCCD_HK640	1
#define IRCCD_HK384	2
#define IRCCD_S640	3
#define IRCCD_S384	4
#define IRCCD_S600GA	5
#define IRCCD_S640_B20	6


/*
	热像镜头
*/

#define IRLENS_75		1
#define IRLENS_104		2
#define IRLENS_104II	3
#define IRLENS_155		4
#define IRLENS_185		5
#define IRLENS_186		6
#define IRLENS_230		7


/*
	可见光镜头
*/

#define CAMLENS_FUSHINENG750	1
#define CAMLENS_FUSHINENG1000	2
#define CAMLENS_JINGGONG500		3
#define CAMLENS_HUAGUO750		4


/*
	热像自动聚焦触发口
*/

#define IRAFPORT_LASER		1
#define IRAFPORT_TWOZOOM	2
#define IRAFPORT_TOUWU		3


/*
	语言
*/

#define LANGUAGE_CHINESE	1
#define LANGUAGE_ENGLISH	2


/*
	类型
*/

#define TYPE_PT500	1
#define TYPE_SP50	2


/*
	开关
*/

#define SW_ON	1
#define SW_OFF	2

/*
	电路板
*/

#define BOARD_ARMA	1
#define BOARD_ARMB	2
#define BOARD_DZ	3
#define BOARD_POWER	4


/*
	激光；类型
*/
#define LASER_MODULE	1	//激光模组
#define LASER_MOTOR_50mm		2	//自己的激光器 50mm
#define LASER_MOTOR_100mm		3	//自己的激光器 100mm
/*
	设备的硬件信息，用于选择镜头数组、机芯控制等
*/

//电路板选择
#define BOARD		BOARD_ARMA

//任务单号
#define TASK		22222

//热像机芯型号
#define IRCCD		IRCCD_HK640

//热像镜头型号
#define IRLENS		IRLENS_155

//可见光镜头型号
#define CAMLENS		CAMLENS_JINGGONG500

//语言
#define LANGUAGE	LANGUAGE_CHINESE

//热像自动聚焦触发
#define IRAFPORT 	IRAFPORT_LASER

//报警
#define ALARMSW 	SW_OFF

//激光
#define LASERSW 	SW_OFF

//激光类型
#define LASERTYPE	LASER_MOTOR_50mm

//云台类型
#define TYPE		TYPE_SP50


#endif

