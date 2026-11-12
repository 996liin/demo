#include "headfile.h"
#include "string.h"
/*
*	收到开始录制的命令后记录当前的位置并开始计时
*	收到控制（以及停止录制）命令记录本次控制状态以及在上个点的停留时间
*	收到停止命令记录当前位置,并将计时清零以及将num++
*/

static unsigned char ucSelfLearnRunFlag = 0;
static unsigned char ucSelfLearnRunStep = 0;
static unsigned char ucSelfLearnRunPoint = 0;
static unsigned short usSelfLearnRunCnt = 0;

//保存
static unsigned char ucSelfLearnSaveFlag = 0;
static unsigned short usSelfLearnSaveCnt = 0;

//等待转台停止的时间
static unsigned short usWaitStopCnt = 0;

//超时
static unsigned int uiOutTimeCnt = 0;
	
static void SelfLearnInit(unsigned char ucOpera);
static void SelfLearnTimer(void);
static void SelfLearnProcess(MESSAGE Msg);

/*
	函数结构体-外部
*/
static Struct_SelfLearn s_SelfLearn = 
{
	.SelfLearnInit = SelfLearnInit,
	.SelfLearnTimer = SelfLearnTimer,
	.SelfLearnProcess = SelfLearnProcess,
};
Struct_SelfLearn *ps_SelfLearn = &s_SelfLearn;

/*
	参数结构体-内部
*/
Struct_SelfLearnPara s_SelfLearnPara = {0,};

/*
	参数初始化
*/
static void SelfLearnInit(unsigned char ucOpera)
{
	if(0 == ucOpera)
	{
		ps_AT24CXX->AT24CXX_Write(SELFLEARNADDR,(unsigned char *)&s_SelfLearnPara,sizeof(s_SelfLearnPara));
	}
	else
	{
		ps_AT24CXX->AT24CXX_Read(SELFLEARNADDR,(unsigned char *)&s_SelfLearnPara,sizeof(s_SelfLearnPara));
		
		if(s_SelfLearnPara.ucSetFlag != 1)
		{
			s_SelfLearnPara.ucSetFlag = 0;
			
			s_SelfLearnPara.ucPointNum = 0;
			memset((unsigned char *)&s_SelfLearnPara.s_PointValue,0,sizeof(s_SelfLearnPara.s_PointValue));
			memset((unsigned char *)&s_SelfLearnPara.s_FirstPointValue,0,sizeof(s_SelfLearnPara.s_FirstPointValue));
		}
	}
}
/*
	定时器相关
*/
static void SelfLearnTimer(void)
{
	if(ucSelfLearnRunFlag)
	{
		usSelfLearnRunCnt++;
	}
	if(ucSelfLearnSaveFlag)
	{
		usSelfLearnSaveCnt++;
	}
	if(usWaitStopCnt)
	{
		usWaitStopCnt++;
	}
	if(uiOutTimeCnt)
	{
		uiOutTimeCnt++;
	}
}
/*
	保存数据
*/
static void SelfLearnSaveData(PointStatus eStatus,unsigned char ucPtSpeed)
{
	if(Status_Start == eStatus)
	{
		ucSelfLearnSaveFlag = 1;
		ucSelfLearnRunFlag = 0;
		usSelfLearnSaveCnt = 0;
		s_SelfLearnPara.ucPointNum = 0;
		
		s_SelfLearnPara.s_FirstPointValue.usCamZoom = ps_ADC->GetCamZoomAD();
		s_SelfLearnPara.s_FirstPointValue.usCamFocus = ps_ADC->GetCamFocusAD();
		s_SelfLearnPara.s_FirstPointValue.usIrZoom = ps_ADC->GetIrZoomAD();
		s_SelfLearnPara.s_FirstPointValue.usIrFocus = ps_ADC->GetIrFocusAD();
		s_SelfLearnPara.s_FirstPointValue.usHAngle = ps_Pt->GetPtHAngle();
		s_SelfLearnPara.s_FirstPointValue.usVAngle = ps_Pt->GetPtVAngle();
	}
	else if(Status_Stop == eStatus)
	{
		usSelfLearnSaveCnt = 0;
		
		switch(s_SelfLearnPara.s_PointValue[s_SelfLearnPara.ucPointNum].eStatus)
		{
			case Status_CamZoom:
			{
				s_SelfLearnPara.s_PointValue[s_SelfLearnPara.ucPointNum++].usData1 = ps_ADC->GetCamZoomAD();
				break;
			}
			case Status_CamFocus:
			{
				s_SelfLearnPara.s_PointValue[s_SelfLearnPara.ucPointNum++].usData1 = ps_ADC->GetCamFocusAD();
				break;
			}
			case Status_IrZoom:
			{
				s_SelfLearnPara.s_PointValue[s_SelfLearnPara.ucPointNum++].usData1 = ps_ADC->GetIrZoomAD();
				break;
			}
			case Status_IrFocus:
			{
				s_SelfLearnPara.s_PointValue[s_SelfLearnPara.ucPointNum++].usData1 = ps_ADC->GetIrFocusAD();
				break;
			}
			case Status_PtH:
			{
				s_SelfLearnPara.s_PointValue[s_SelfLearnPara.ucPointNum++].usData1 = ps_Pt->GetPtHAngle();
				break;
			}
			case Status_PtV:
			{
				s_SelfLearnPara.s_PointValue[s_SelfLearnPara.ucPointNum++].usData1 = ps_Pt->GetPtVAngle();
				
				break;
			}
			case Status_PtHV:
			{
				s_SelfLearnPara.s_PointValue[s_SelfLearnPara.ucPointNum].usData1 = ps_Pt->GetPtHAngle();
				s_SelfLearnPara.s_PointValue[s_SelfLearnPara.ucPointNum++].usData2 = ps_Pt->GetPtVAngle();
				break;
			}
			default:
			{
				break;
			}
		}
	}
	else
	{
		s_SelfLearnPara.s_PointValue[s_SelfLearnPara.ucPointNum].eStatus = eStatus;
		s_SelfLearnPara.s_PointValue[s_SelfLearnPara.ucPointNum].usTime = usSelfLearnSaveCnt;
		s_SelfLearnPara.s_PointValue[s_SelfLearnPara.ucPointNum].ucPtSpeed = ucPtSpeed;
		
		if(Status_End == eStatus)
		{
			ucSelfLearnSaveFlag = 0;
			
			if(0 == s_SelfLearnPara.ucPointNum)
			{
				SelfLearnInit(1);
			}
			else
			{
				s_SelfLearnPara.ucSetFlag = 1;
				SelfLearnInit(0);
			}
		}
	}
}
/*
	运行
*/
static void SelfLearnCallFirstPoint(void)
{
	if(1 == s_SelfLearnPara.ucSetFlag)
	{
		ucSelfLearnRunStep = 1;
		ucSelfLearnRunPoint = 0;
		ucSelfLearnRunFlag = 1;
		usSelfLearnRunCnt = 0;
		uiOutTimeCnt = 1;
		
		ps_Lens->SetCamZoomPosi(s_SelfLearnPara.s_FirstPointValue.usCamZoom);
		ps_Lens->SetCamFocusPosi(s_SelfLearnPara.s_FirstPointValue.usCamFocus);
		ps_Lens->SetIrZoomPosi(s_SelfLearnPara.s_FirstPointValue.usIrZoom);
		ps_Lens->SetIrFocusPosi(s_SelfLearnPara.s_FirstPointValue.usIrFocus);
		
		ps_Pt->SetLocatSpeed(30,30);
		ps_Systick->delay_ms(50);
		ps_Pt->LocatHAngle(s_SelfLearnPara.s_FirstPointValue.usHAngle);
		ps_Systick->delay_ms(50);
		ps_Pt->LocatVAngle(s_SelfLearnPara.s_FirstPointValue.usVAngle);
		ps_Systick->delay_ms(50);
	}
}
static void SelfLearnRun(void)
{
	if(1 == ucSelfLearnRunFlag)
	{
		if(1 == ucSelfLearnRunStep)	//起点
		{
			//等待到达起点	120s的超时保护
			if( (
				 abs(s_SelfLearnPara.s_FirstPointValue.usCamZoom - ps_ADC->GetCamZoomAD()) <= 15 && abs(s_SelfLearnPara.s_FirstPointValue.usCamFocus - ps_ADC->GetCamFocusAD()) <= 15 &&\
				 abs(s_SelfLearnPara.s_FirstPointValue.usIrZoom - ps_ADC->GetIrZoomAD()) <= 15 && abs(s_SelfLearnPara.s_FirstPointValue.usIrFocus - ps_ADC->GetIrFocusAD()) <= 15 &&\
				 (abs(s_SelfLearnPara.s_FirstPointValue.usHAngle - ps_Pt->GetPtHAngle()) <= 10 || abs(36000 - abs(s_SelfLearnPara.s_FirstPointValue.usHAngle - ps_Pt->GetPtHAngle()) <= 10)) &&\
				 (abs(s_SelfLearnPara.s_FirstPointValue.usVAngle - ps_Pt->GetPtVAngle()) <= 10 || abs(36000 - abs(s_SelfLearnPara.s_FirstPointValue.usVAngle - ps_Pt->GetPtVAngle()) <= 10))
				)  || uiOutTimeCnt >= 120 * 1000
			  )
			{
				uiOutTimeCnt = 0;
				ucSelfLearnRunStep = 2;
				usSelfLearnRunCnt = 0;
			}
		}
		else if(2 == ucSelfLearnRunStep)	//停留时间
		{
			if(usSelfLearnRunCnt >= s_SelfLearnPara.s_PointValue[ucSelfLearnRunPoint].usTime)
			{
				ucSelfLearnRunStep = 3;
			}
		}
		else if(3 == ucSelfLearnRunStep)	//去下一个点
		{
			ucSelfLearnRunStep = 4;
			
			switch(s_SelfLearnPara.s_PointValue[ucSelfLearnRunPoint].eStatus)
			{
				case Status_CamZoom:
				{
					ps_Lens->SetCamZoomPosi(s_SelfLearnPara.s_PointValue[ucSelfLearnRunPoint].usData1);
					uiOutTimeCnt = 1;
					break;
				}
				case Status_CamFocus:
				{
					ps_Lens->SetCamFocusPosi(s_SelfLearnPara.s_PointValue[ucSelfLearnRunPoint].usData1);
					uiOutTimeCnt = 1;
					break;
				}
				case Status_IrZoom:
				{
					ps_Lens->SetIrZoomPosi(s_SelfLearnPara.s_PointValue[ucSelfLearnRunPoint].usData1);
					uiOutTimeCnt = 1;
					break;
				}
				case Status_IrFocus:
				{
					ps_Lens->SetIrFocusPosi(s_SelfLearnPara.s_PointValue[ucSelfLearnRunPoint].usData1);
					uiOutTimeCnt = 1;
					break;
				}
				case Status_PtH:
				{
					ps_Pt->SetLocatSpeed(s_SelfLearnPara.s_PointValue[ucSelfLearnRunPoint].ucPtSpeed,s_SelfLearnPara.s_PointValue[ucSelfLearnRunPoint].ucPtSpeed);
					ps_Systick->delay_ms(40);
					ps_Pt->LocatHAngle(s_SelfLearnPara.s_PointValue[ucSelfLearnRunPoint].usData1);
					ps_Systick->delay_ms(40);
					uiOutTimeCnt = 1;
					break;
				}
				case Status_PtV:
				{
					ps_Pt->SetLocatSpeed(s_SelfLearnPara.s_PointValue[ucSelfLearnRunPoint].ucPtSpeed,s_SelfLearnPara.s_PointValue[ucSelfLearnRunPoint].ucPtSpeed);
					ps_Systick->delay_ms(40);
					ps_Pt->LocatVAngle(s_SelfLearnPara.s_PointValue[ucSelfLearnRunPoint].usData1);
					ps_Systick->delay_ms(40);
					uiOutTimeCnt = 1;
					break;
				}
				case Status_PtHV:
				{
					ps_Pt->SetLocatSpeed(s_SelfLearnPara.s_PointValue[ucSelfLearnRunPoint].ucPtSpeed,s_SelfLearnPara.s_PointValue[ucSelfLearnRunPoint].ucPtSpeed);
					ps_Systick->delay_ms(40);
					ps_Pt->LocatHAngle(s_SelfLearnPara.s_PointValue[ucSelfLearnRunPoint].usData1);
					ps_Systick->delay_ms(40);
					ps_Pt->LocatVAngle(s_SelfLearnPara.s_PointValue[ucSelfLearnRunPoint].usData2);
					ps_Systick->delay_ms(40);
					uiOutTimeCnt = 1;
					break;
				}
				default:
				{
					break;
				}
			}
		}
		else if(4 == ucSelfLearnRunStep)	//检测到位
		{
			switch(s_SelfLearnPara.s_PointValue[ucSelfLearnRunPoint].eStatus)
			{
				case Status_End:
				{
					SelfLearnCallFirstPoint();
					break;
				}
				case Status_CamZoom:
				{
					if(abs(s_SelfLearnPara.s_PointValue[ucSelfLearnRunPoint].usData1 - ps_ADC->GetCamZoomAD()) <= 15 || uiOutTimeCnt >= 30 * 1000)
					{
						uiOutTimeCnt = 0;
						ucSelfLearnRunStep = 2;
						usSelfLearnRunCnt = 0;
						ucSelfLearnRunPoint++;
						if(ucSelfLearnRunPoint > s_SelfLearnPara.ucPointNum)
						{
							SelfLearnCallFirstPoint();
						}
					}
					break;
				}
				case Status_CamFocus:
				{
					if(abs(s_SelfLearnPara.s_PointValue[ucSelfLearnRunPoint].usData1 - ps_ADC->GetCamFocusAD()) <= 15 || uiOutTimeCnt >= 30 * 1000)
					{
						uiOutTimeCnt = 0;
						ucSelfLearnRunStep = 2;
						usSelfLearnRunCnt = 0;
						ucSelfLearnRunPoint++;
						if(ucSelfLearnRunPoint > s_SelfLearnPara.ucPointNum)
						{
							SelfLearnCallFirstPoint();
						}
					}
					break;
				}
				case Status_IrZoom:
				{
					if(abs(s_SelfLearnPara.s_PointValue[ucSelfLearnRunPoint].usData1 - ps_ADC->GetIrZoomAD()) <= 15 || uiOutTimeCnt >= 30 * 1000)
					{
						uiOutTimeCnt = 0;
						ucSelfLearnRunStep = 2;
						usSelfLearnRunCnt = 0;
						ucSelfLearnRunPoint++;
						if(ucSelfLearnRunPoint > s_SelfLearnPara.ucPointNum)
						{
							SelfLearnCallFirstPoint();
						}
					}
					break;
				}
				case Status_IrFocus:
				{
					if(abs(s_SelfLearnPara.s_PointValue[ucSelfLearnRunPoint].usData1 - ps_ADC->GetIrFocusAD()) <= 15 || uiOutTimeCnt >= 30 * 1000)
					{
						uiOutTimeCnt = 0;
						ucSelfLearnRunStep = 2;
						usSelfLearnRunCnt = 0;
						ucSelfLearnRunPoint++;
						if(ucSelfLearnRunPoint > s_SelfLearnPara.ucPointNum)
						{
							SelfLearnCallFirstPoint();
						}
					}
					break;
				}
				case Status_PtH:
				{
					if(abs(s_SelfLearnPara.s_PointValue[ucSelfLearnRunPoint].usData1 - ps_Pt->GetPtHAngle()) <= 10 || abs(36000 - abs(s_SelfLearnPara.s_PointValue[ucSelfLearnRunPoint].usData1 - ps_Pt->GetPtHAngle()) <= 10) || uiOutTimeCnt >= 120 * 1000)
					{
						uiOutTimeCnt = 0;
						ucSelfLearnRunStep = 2;
						usSelfLearnRunCnt = 0;
						ucSelfLearnRunPoint++;
						if(ucSelfLearnRunPoint > s_SelfLearnPara.ucPointNum)
						{
							SelfLearnCallFirstPoint();
						}
					}
					break;
				}
				case Status_PtV:
				{
					if(abs(s_SelfLearnPara.s_PointValue[ucSelfLearnRunPoint].usData1 - ps_Pt->GetPtVAngle()) <= 10 || abs(36000 - abs(s_SelfLearnPara.s_PointValue[ucSelfLearnRunPoint].usData1 - ps_Pt->GetPtVAngle()) <= 10) || uiOutTimeCnt >= 120 * 1000)
					{
						uiOutTimeCnt = 0;
						ucSelfLearnRunStep = 2;
						usSelfLearnRunCnt = 0;
						ucSelfLearnRunPoint++;
						if(ucSelfLearnRunPoint > s_SelfLearnPara.ucPointNum)
						{
							SelfLearnCallFirstPoint();
						}
					}
					break;
				}
				case Status_PtHV:
				{
					if(
						( (abs(s_SelfLearnPara.s_PointValue[ucSelfLearnRunPoint].usData1 - ps_Pt->GetPtHAngle()) <= 10 || abs(36000 - abs(s_SelfLearnPara.s_PointValue[ucSelfLearnRunPoint].usData1 - ps_Pt->GetPtHAngle()) <= 10)) && \
						  (abs(s_SelfLearnPara.s_PointValue[ucSelfLearnRunPoint].usData2 - ps_Pt->GetPtVAngle()) <= 10 || abs(36000 - abs(s_SelfLearnPara.s_PointValue[ucSelfLearnRunPoint].usData2 - ps_Pt->GetPtVAngle()) <= 10)) 
						) || uiOutTimeCnt >= 120 * 1000
					  )
					{
						uiOutTimeCnt = 0;
						ucSelfLearnRunStep = 2;
						usSelfLearnRunCnt = 0;
						ucSelfLearnRunPoint++;
						if(ucSelfLearnRunPoint > s_SelfLearnPara.ucPointNum)
						{
							SelfLearnCallFirstPoint();
						}
					}
					break;
				}
				default:
				{
					break;
				}
			}
		}
	}
}
/*
	线程
*/
static void SelfLearnProcess(MESSAGE Msg)
{
	if(usWaitStopCnt >= 800)
	{
		usWaitStopCnt = 0;
		SelfLearnSaveData(Status_Stop,0);
	}
	
	SelfLearnRun();
	
	if(Msg.usMsgID == msgNull  || (Msg.ucAddr != 0x01 && Msg.ucAddr != 0x02))
	{
		return;
	}
	unsigned short usData = Msg.ucData[0] << 8 | Msg.ucData[1];
	unsigned char ucCtrlFlag = 0;
	switch (Msg.usMsgID)
	{
		case magCallPreset:
		{
			if(callNum143 == usData)
			{
				SelfLearnSaveData(Status_Start,0);
			}
			else if(callNum144 == usData)
			{
				SelfLearnSaveData(Status_End,0);
			}
			else if(callNum145 == usData)
			{
				if(ucSelfLearnSaveFlag)
				{
					SelfLearnSaveData(Status_End,0);
				}
				
				SelfLearnCallFirstPoint();
			}
			else
			{
				ucCtrlFlag = 1;
			}
			
			break;
		}
		case msgStop:	//停止
		{
			if(0x0000 == usData)
			{
				if(1 == ucSelfLearnSaveFlag)
				{
					//为等待转台停下后的角度回传，等待一定时间后再记录最终的位置；镜头直接记录位置即可。
					if( Status_PtH == s_SelfLearnPara.s_PointValue[s_SelfLearnPara.ucPointNum].eStatus || \
						Status_PtV == s_SelfLearnPara.s_PointValue[s_SelfLearnPara.ucPointNum].eStatus || \
						Status_PtHV == s_SelfLearnPara.s_PointValue[s_SelfLearnPara.ucPointNum].eStatus
					   )
					{
						usWaitStopCnt = 1;
					}
					else
					{
						usWaitStopCnt = 800;
					}					
				}
				else
				{
					ucCtrlFlag = 1;
				}
			}			
			break;
		}
		case msgZoomTele:	//变倍
		case msgZoomWide:
		{
			if(1 == ucSelfLearnSaveFlag)
			{
				if(1 == Msg.ucAddr)
				{
					SelfLearnSaveData(Status_CamZoom,0);
				}
				else
				{
					SelfLearnSaveData(Status_IrZoom,0);
				}
			}
			else
			{
				ucCtrlFlag = 1;
			}
			
			break;
		}
		case msgFocusFar:	//聚焦
		case msgFocusNear:
		{
			if(1 == ucSelfLearnSaveFlag)
			{
				if(1 == Msg.ucAddr)
				{
					SelfLearnSaveData(Status_CamFocus,0);
				}
				else
				{
					SelfLearnSaveData(Status_IrFocus,0);
				}
			}
			else
			{
				ucCtrlFlag = 1;
			}
			
			break;
		}
		case msgPtUp:	//上下
		case msgPtDown:
		{
			if(1 == ucSelfLearnSaveFlag)
			{
				SelfLearnSaveData(Status_PtV,Msg.ucData[1]);
			}
			else
			{
				ucCtrlFlag = 1;
			}
			break;
		}
		case msgPtLeft:	//左右
		case msgPtRight:
		{
			if(1 == ucSelfLearnSaveFlag)
			{
				SelfLearnSaveData(Status_PtH,Msg.ucData[0]);
			}
			else
			{
				ucCtrlFlag = 1;
			}
			break;
		}
		case msgPtLeftUp:	//斜角
		case msgPtLeftDown:
		case msgPtRightUp:
		case msgPtRightDown:
		{
			if(1 == ucSelfLearnSaveFlag)
			{
				SelfLearnSaveData(Status_PtHV,Msg.ucData[1]);
			}
			else
			{
				ucCtrlFlag = 1;
			}
			break;
		}
		case msgGetPtPan:	//查询指令不打断巡航
		case msgGetPtTilt:
		case msgGetZoomAD:
		case msgGetFocus:
		{
			break;
		}
		default:
		{
			ucCtrlFlag = 1;
			break;
		}
	}
	
	if(ucCtrlFlag)
	{
		ucCtrlFlag = 0;
		if(ucSelfLearnSaveFlag)
		{
			SelfLearnSaveData(Status_End,0);
		}
		if(ucSelfLearnRunFlag)
		{
			ucSelfLearnRunFlag = 0;
		}
	}
}
