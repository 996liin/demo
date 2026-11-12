#include "headfile.h"
uint16_t Z_AWYH[15][2] =        //实际标定AD值与视场角对应关系
{
    {	16186	,	6120	,	}	,
		{	15591	,	5761	,	}	,
		{	14051	,	4993	,	}	,
		{	12628	,	4417	,	}	,
		{	10538	,	3985	,	}	,
		{	8917	,	3409	,	}	,
		{	7836	,	3025	,	}	,
		{	6755	,	2641	,	}	,
		{	5404	,	2305	,	}	,
		{	4864	,	2188	,	}	,
		{	3783	,	2017	,	}	,
		{	2702	,	1848	,	}	,
		{	1892	,	1560	,	}	,
		{	811	  ,	1368	,	}	,
		{	0	    ,	1272	,	}	,

};
#define LASERRUNDIR_WIDE	1
#define LASERRUNDIR_TELE	2
#define LASERRUNDIR_ELSE	3
static void CmdCtrl(unsigned char cmd,unsigned short data);
static void init(void);
//static void LaserSWCtrl(unsigned char ucEnable);
//static void LaserWide(unsigned char step);
//static void LaserTele(unsigned char step);
static inline void SetLaserPosi(unsigned short usLaserPosi);
static inline void SetLaserBright(unsigned char level);
static inline void SetLaserStrobing(unsigned char cmd,unsigned short data);
static void Process(void);
/*
	函数结构体-外部
*/
static struct_Laser s_Laser = 
{
	.CmdCtrl     = CmdCtrl,
	.init        = init,
	.OpenFlag    = 0,
	.Process     = Process,
};
struct_Laser *ps_Laser = &s_Laser;


static void init(void)
{
	s_Laser.UartSendData = ps_Usart->Usart5_SendData;
}
static inline void LaserSWCtrl(unsigned char ucEnable)
{
	if(1 == ucEnable)
	{
	   unsigned char ucCom[7] = {0xff,0x01,0x01,0x01,0x01,0x00,0x04};
		 s_Laser.UartSendData(ucCom,7);
		 ps_Systick->delay_ms(50);
		 SetLaserStrobing(0x0d,0);
		 ps_Systick->delay_ms(50);
		 SetLaserBright(0xfa);
		 ps_Systick->delay_ms(50);
		 SetLaserPosi(0x2000); 
		 s_Laser.OpenFlag = 1;
		 
	}
	else
	{
		 unsigned char ucCom[7] = {0xff,0x01,0x01,0x01,0x00,0x00,0x03};
		 s_Laser.UartSendData(ucCom,7);
		 s_Laser.OpenFlag = 0;
	}
}
static inline void LaserWide(unsigned char step)
{
	 unsigned char ucCom[7] = {0xff,0x01,0x01,0x04,0x01,0x00,0x00};
	 ucCom[5] = step;
	 ucCom[6] = (ucCom[1]+ucCom[2]+ucCom[3]+ucCom[4]+ucCom[5])&0xff;
	 s_Laser.UartSendData(ucCom,7);
}
static inline void LaserTele(unsigned char step)
{
   unsigned char ucCom[7] = {0xff,0x01,0x01,0x04,0x00,0x00,0x00};
	 ucCom[5] = step;
	 ucCom[6] = (ucCom[1]+ucCom[2]+ucCom[3]+ucCom[4]+ucCom[5])&0xff;
	 s_Laser.UartSendData(ucCom,7);
}
static inline void SetLaserPosi(unsigned short usLaserPosi)
{
		unsigned char ucCom[7] = {0xff,0x01,0x01,0x05,0x00,0x00,0x00};
		ucCom[4] = usLaserPosi >> 8;
		ucCom[5] = usLaserPosi & 0xff;
		ucCom[6] = (ucCom[1]+ucCom[2]+ucCom[3]+ucCom[4]+ucCom[5])&0xff;
		s_Laser.UartSendData(ucCom,7);
}
/*
  设置激光亮度
*/
static inline void SetLaserBright(unsigned char level)
{
	  unsigned char ucCom[7] = {0xff,0x01,0x01,0x03,0x00,0x00,0x04};
		ucCom[4] = level;
		ucCom[6] = (ucCom[1]+ucCom[2]+ucCom[3]+ucCom[4]+ucCom[5])&0xff;
		s_Laser.UartSendData(ucCom,7);
}
/*
设置激光频闪
*/
static inline void SetLaserStrobing(unsigned char cmd,unsigned short data)
{
  switch (cmd)
	{
		case 0x0b:
		{
			unsigned char ucCom[7] = {0xff,0x01,0x01,0x0b,0x00,0x00,0x00};
			ucCom[4] = data >> 8;
			ucCom[5] = data & 0xff;
			ucCom[6] = (ucCom[1]+ucCom[2]+ucCom[3]+ucCom[4]+ucCom[5])&0xff;
			s_Laser.UartSendData(ucCom,7);
		  break;
		}
    case 0x0c:
		{
			unsigned char ucCom[7] = {0xff,0x01,0x01,0x0c,0x00,0x00,0x00};
			ucCom[4] = data >> 8;
			ucCom[5] = data & 0xff;
			ucCom[6] = (ucCom[1]+ucCom[2]+ucCom[3]+ucCom[4]+ucCom[5])&0xff;
			s_Laser.UartSendData(ucCom,7);
		  break;
		}
    case 0x0d:
		{
			unsigned char ucCom[7] = {0xff,0x01,0x01,0x0d,0x01,0x00,0x00};
			ucCom[5] = data & 0xff;
			ucCom[6] = (ucCom[1]+ucCom[2]+ucCom[3]+ucCom[4]+ucCom[5])&0xff;
			s_Laser.UartSendData(ucCom,7);
		  break;
		}		
		default:
		break;
	}
}
static void CmdCtrl(unsigned char cmd,unsigned short data)
{
   switch (cmd)
	 {
		  case 0xEF:
			{
				LaserSWCtrl(1);
				break;
			}
			case 0xE0:
			{
				LaserSWCtrl(0);
				break;
			}
			case 0x03:
			{
				break;
			}
			case 0x04:
			{
				break;
			}
			case 0x01:
			{
				LaserWide(0x30);
				break;
			}
			case 0x02:
			{
				LaserTele(0x30);
				break;
			}
			case 0x10:
			{
				SetLaserPosi(data);
				break;
			}
			case 0x11:
			{
				SetLaserBright(data << 8);
				break;
			}
			case 0x0d:
			{
				SetLaserStrobing(0x0d,data);
			}
			 default:
			 break;	 
	 }
}
static uint16_t Search_laser_pos(uint16_t usCamPosi)
{
		uint16_t usReturnData = 0;
		if(usCamPosi > Z_AWYH[0][0])
		{
			usCamPosi = Z_AWYH[0][0];
		}
		else if(usCamPosi < Z_AWYH[14][0])
		{
			usCamPosi = Z_AWYH[14][0];
		}

		for(unsigned short usCnt = 0; usCnt < 14; usCnt++)
		{
			if(usCamPosi <= Z_AWYH[usCnt][0] && usCamPosi >= Z_AWYH[usCnt+1][0])
			{
					usReturnData = Z_AWYH[usCnt][1] - \
					(Z_AWYH[usCnt][1] - Z_AWYH[usCnt+1][1]) * \
					(Z_AWYH[usCnt][0] - usCamPosi) / \
					(Z_AWYH[usCnt][0] - Z_AWYH[usCnt+1][0]);	
				break;
			}
		}
	return usReturnData;
}

static void LaserSyncCtrl(void)
{
	//if(inst_laser_ctrl.ucSyncFlag)
	if(1)
	{
	   SetLaserPosi(Search_laser_pos(ps_CCD->zoomvalue));
		
	}
}
static void Process(void)
{
	if(s_Laser.AutoCtrlTimeout)
	{
		s_Laser.AutoCtrlTimeout = 0;
		if(s_Laser.OpenFlag == 1)
		{
			LaserSyncCtrl();
		} 
	}		
}

