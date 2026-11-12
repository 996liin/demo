#include "headfile.h"
#include "string.h"

/*
		参数
*/
static unsigned char ucPatrolRunFlag = 0;
static unsigned char ucPatrolRunStep = 0;
static unsigned int uiPatrolWaitCnt = 0;
static unsigned char ucPatrolLine = 0;
static unsigned char ucPatrolPresetNum = 0;
static unsigned char ucOutTimeFlag = 0;
static unsigned int uiOutTimeCnt = 0;
static unsigned char ucWaitArrFlag = 0;


static unsigned short usLastHAngle= 0;
static unsigned short usLastVAngle= 0;

static void PatrolParaInit(unsigned char ucOperation);
static void PatrolTimer(void);
static void PatrolStart(unsigned char ucLine);
static unsigned char GetPatrolStatus(void);
static void PatrolProcess(MESSAGE Msg);
/*
		函数结构体-外部
*/
static struct_Patrol s_Patrol = 
{
	.PatrolParaInit = PatrolParaInit,
	.PatrolTimer = PatrolTimer,
	.PatrolStart = PatrolStart,
	.GetPatrolStatus = GetPatrolStatus,
	.PatrolProcess = PatrolProcess,
};
struct_Patrol *ps_Patrol = &s_Patrol;
/*
		参数结构体-内部
*/
static struct_PatrolPara s_PatrolPara = {0,};


/*
		巡航参数初始化
*/
static void PatrolParaInit(unsigned char ucOperation)
{
	if(0 == ucOperation)
	{
		ps_AT24CXX->AT24CXX_Write(PATROLADDR,(unsigned char *)&s_PatrolPara,sizeof(s_PatrolPara));
	}
	else
	{
		ps_AT24CXX->AT24CXX_Read(PATROLADDR,(unsigned char *)&s_PatrolPara,sizeof(s_PatrolPara));
		
		if(s_PatrolPara.ucPatrolSpeed > 64)
		{
			s_PatrolPara.ucPatrolSpeed = 20;
		}
		if(s_PatrolPara.ucPatrolTime > 240)
		{
			s_PatrolPara.ucPatrolTime = 4;
		}
	}
}
/*
		巡航速度设置
*/
static void PatrolSpeedSet(unsigned char ucSpeed)
{
	s_PatrolPara.ucPatrolSpeed = ucSpeed;
	PatrolParaInit(0);
	
	unsigned char ucData[7] = {0xff,0x01,0x00,0x67,0x00,0x00,0x00};
	ucData[4] = ucSpeed ;
	ucData[5] = ucSpeed ;
	ucData[6] = (ucData[1]+ucData[2]+ucData[3]+ucData[4]+ucData[5])&0xff;
	ps_Usart->Usart2_SendData(ucData,7);
}
/*
		巡航时间设置
*/
static void PatrolTimeSet(unsigned char ucTime)
{
	s_PatrolPara.ucPatrolTime = ucTime;
	PatrolParaInit(0);
}
/*
		定时器相关
*/
static void PatrolTimer(void)
{
	if(ucOutTimeFlag)
	{
		uiOutTimeCnt++;
		if(0 == uiOutTimeCnt % 1000)
		{
			if(abs(usLastHAngle - ps_Pt->GetPtHAngle()) <= 10 && abs(usLastVAngle - ps_Pt->GetPtVAngle()) <= 10)
			{
				ps_Arr->ucPtArr = 1;
			}
			else
			{
				usLastHAngle = ps_Pt->GetPtHAngle();
				usLastVAngle = ps_Pt->GetPtVAngle();
				ps_Arr->ucPtArr = 0;
			}
		}
		else if(uiOutTimeCnt >= 120 * 1000)
		{
			ucOutTimeFlag = 0;
			uiOutTimeCnt = 0;
			ucPatrolRunStep = 3;
		}
	}
	if(2 == ucPatrolRunStep)
	{
		uiPatrolWaitCnt++;
	}
}
/*
		巡航函数实现
*/
static void PatrolStart(unsigned char ucLine)
{
	if(ucLine > 1)
	{
		return;
	}
	if(ucLine != ucPatrolLine)
	{
		ucPatrolPresetNum = 0;
		ucPatrolLine = ucLine;
	}
	
	//校验当前预置位
	unsigned char ucPresetEnable = 0;
	ps_AT24CXX->AT24CXX_Read(PRESETADDR+(ucPatrolLine*64+1+ucPatrolPresetNum)*11,&ucPresetEnable,1);
	
	//当前预置位未设置
	if(1 != ucPresetEnable)
	{
		//校验该巡线首个预置位
		ucPatrolPresetNum = 0;
		ps_AT24CXX->AT24CXX_Read(PRESETADDR+(ucPatrolLine*64+1+ucPatrolPresetNum)*11,&ucPresetEnable,1);
		
		//该巡线首个预置位未设置
		if(1 != ucPresetEnable)
		{
			return;
		}
	}
	
	ucPatrolRunFlag = 1;
	ucPatrolRunStep = 1;
	ucOutTimeFlag = 1;
	uiOutTimeCnt = 0;
	
	ucWaitArrFlag = 1;
	
	//调预置位
	ps_Lens->CallPreset(ucPatrolLine*64+1+ucPatrolPresetNum);
	ps_Pt->CallPtPreset(1,ucPatrolLine*64+1+ucPatrolPresetNum);
}
static void PatrolRun(void)
{
	if(1 == ucPatrolRunFlag)
	{
		switch (ucPatrolRunStep)
		{
			case 1:
			{
				if(ucWaitArrFlag && ps_Arr->ucCamFocusArr && ps_Arr->ucCamZoomArr && ps_Arr->ucIrFocusArr && ps_Arr->ucIrZoomArr && ps_Arr->ucPtArr)
				{
					ucWaitArrFlag = 0;
					ucOutTimeFlag = 0;
					ucPatrolRunStep = 2;
					uiPatrolWaitCnt = 0;
				}
				break;
			}
			case 2:
			{
				if(uiPatrolWaitCnt >= s_PatrolPara.ucPatrolTime * 1000)
				{
					uiPatrolWaitCnt = 0;
					ucPatrolRunStep = 3;
				}
				break;
			}
			case 3:
			{
				ucPatrolPresetNum++;
				if(0 == ucPatrolPresetNum % 64)
				{
					ucPatrolPresetNum = 0;
				}
				//校验当前预置位
				unsigned char ucPresetEnable = 0;
				ps_AT24CXX->AT24CXX_Read(PRESETADDR+(ucPatrolLine*64+1+ucPatrolPresetNum)*11,&ucPresetEnable,1);
				
				//当前预置位未设置
				if(1 != ucPresetEnable)
				{
					//校验该巡线首个预置位
					ucPatrolPresetNum = 0;
					ps_AT24CXX->AT24CXX_Read(PRESETADDR+(ucPatrolLine*64+1+ucPatrolPresetNum)*11,&ucPresetEnable,1);
					
					//该巡线首个预置位未设置
					if(1 != ucPresetEnable)
					{
						ucPatrolRunFlag = 0;
						return;
					}
				}
				
				ucPatrolRunStep = 1;
				ucOutTimeFlag = 1;
				uiOutTimeCnt = 0;
				ucWaitArrFlag = 1;
				
				//调预置位
				ps_Lens->CallPreset(ucPatrolLine*64+1+ucPatrolPresetNum);
				ps_Pt->CallPtPreset(1,ucPatrolLine*64+1+ucPatrolPresetNum);
				
				break;
			}
			default:
			{
				break;
			}
		}
	}
}
static unsigned char GetPatrolStatus(void)
{
	unsigned char ucReturnData = 0;
	
	if(ucPatrolRunFlag)
	{
		ucReturnData = ucPatrolLine + 1;
	}
	else
	{
		ucReturnData = 0;
	}
	
	return ucReturnData;
}
/*
		线程
*/
static void PatrolProcess(MESSAGE Msg)
{
	PatrolRun();
	
	if(msgNull == Msg.usMsgID || (Msg.ucAddr != 0x01 && Msg.ucAddr != 0x02))
	{
		return;
	}
	
	unsigned char ucCtrlFlag = 0;
	unsigned short usData = Msg.ucData[0] << 8 | Msg.ucData[1];
		
	switch (Msg.usMsgID)
	{
		case msgStop:
		{
			if(0x0000 == usData)
			{
				ucCtrlFlag = 1;
			}
			break;
		}
		case msgSetPreset:	//设置预置位
		{
			if(usData >= setNum200 && usData <= setNum207)
			{
				unsigned char ucData = 0;
				if(200 == usData)
				{
					ucData = 4;
				}
				else if(usData >= 201 && usData <= 203)
				{
					ucData = (usData - 200) * 10;
				}
				else if(usData >= 204 && usData <= 207)
				{
					ucData = (usData - 203) * 60;
				}
				PatrolTimeSet(ucData);
			}
			else
			{
				ucCtrlFlag = 1;
			}
			break;
		}
		case magCallPreset:	//调用预置位
		{
			if(callNum131 == usData || callNum132 == usData)
			{
				PatrolStart(usData - callNum131);
			}
			else if(callNum152 == usData)	//继续巡航
			{
				if(ucPatrolRunFlag)
				{
					ucPatrolRunFlag = 1;
				}
			}
			else
			{
				ucCtrlFlag = 1;
			}
			break;
		}
		case msgPatrolRun:	//巡航
		{
			PatrolStart(usData);
			break;
		}
		case msgSetPtPresetSpeed:	//速度
		{
			if(0 == usData || usData > 64)
			{
				usData = 20;
			}
			PatrolSpeedSet(usData);
			break;
		}
		case msgSetPatrolTime:	//时间
		{
			if(0 == usData || usData > 240)
			{
				usData = 4;
			}
			PatrolTimeSet(usData);
			break;
		}
		case msgGetPtPan:	//查询指令不打断巡航
		case msgGetPtTilt:
		case msgGetZoomAD:
		case msgGetFocus:
		case msgPtDeviceAlarm:	//驱动重启不打断巡航 因为报警后需要读取巡航状态
		{
			break;
		}
		default :	//其他指令打断巡航
		{
			ucCtrlFlag = 1;
			break;
		}
	}
	
	//打断巡航操作
	if(1 == ucCtrlFlag)
	{
		ucCtrlFlag = 0;
		ucPatrolRunFlag = 0;
	}
}
