#include "headfile.h"

static unsigned char ucKWStep = 0;
static unsigned int uiKWRunCnt = 0;
static unsigned char ucWaitArrFlag = 0;
static unsigned char ucCtrlFlag = 0;

static void KWInit(unsigned char ucOpera);
static void KWTimer(void);
static void KWProcess(MESSAGE Msg);
/*
		函数结构体-外部
*/
static struct_KW s_KW = 
{
	.KWInit = KWInit,
	.KWTimer = KWTimer,
	.KWProcess = KWProcess,
	
};
struct_KW *ps_KW = &s_KW;

/*
		参数结构体-内部
*/
static struct_KWPara s_KWPara;

/*
		参数初始化
*/
static void KWInit(unsigned char ucOpera)
{
	if(0 == ucOpera)
	{
		ps_AT24CXX->AT24CXX_Write(KWADDR,(unsigned char *)&s_KWPara,sizeof(s_KWPara));
	}
	else
	{
		ps_AT24CXX->AT24CXX_Read(KWADDR,(unsigned char *)&s_KWPara,sizeof(s_KWPara));
		
		if(s_KWPara.ucKWSW > 1)
		{
			s_KWPara.ucKWSW = 0;
		}
		
		if(s_KWPara.ucKWTime > 60)
		{
			s_KWPara.ucKWTime = 2;
		}
		
		if(s_KWPara.ucOpera > 4)
		{
			s_KWPara.ucOpera = 1;
		}
		
		if(s_KWPara.ucKWAFTime > 240)
		{
			s_KWPara.ucKWAFTime = 30;
		}
		
		if(s_KWPara.ucKWAFSW > 1)
		{
			s_KWPara.ucKWAFSW = 1;
		}
	}
}
/*
		定时器相关
*/
static void KWTimer(void)
{
	if(ucKWStep)
	{
		uiKWRunCnt++;
	}
}
/*
		函数实现
*/
static void KWRun(void)
{
	//巡航、线扫、苹果皮状态下不执行守望
	if(1 == s_KWPara.ucKWSW && 0 == ps_Patrol->GetPatrolStatus() && 0 == ps_Pt->GetLineScanStatus() && 0 == ps_Pt->GetAppleScanStatus() )	
	{
		switch (ucKWStep)
		{
			case 1:
			{
				if( uiKWRunCnt >= s_KWPara.ucKWTime * 60 * 1000)
				{
					uiKWRunCnt = 0;
					ucKWStep = 2;
				}
				break;
			}
			case 2:
			{
				switch (s_KWPara.ucOpera)
				{
					case 1:
					{
						ps_Lens->CallPreset(1);
						ps_Pt->CallPtPreset(1,1);
						
						ucWaitArrFlag = 1;
						break;
					}
					case 2:
					{
						ps_Lens->CallPreset(2);
						ps_Pt->CallPtPreset(1,2);
						
						ucWaitArrFlag = 1;
						break;
					}
					case 3:
					{
						ps_Patrol->PatrolStart(0);
						break;
					}
					case 4:
					{
						ps_Patrol->PatrolStart(1);
						break;
					}
					default:
					{
						break;
					}
				}
				
				ucKWStep = 3;
				
				break;
			}
			case 3:
			{
				if(1 == s_KWPara.ucKWAFSW)
				{
					switch (s_KWPara.ucOpera)
					{
						case 1:
						case 2:
						{
							if(ucWaitArrFlag && ps_Arr->ucCamFocusArr && ps_Arr->ucCamZoomArr && ps_Arr->ucIrFocusArr && ps_Arr->ucIrZoomArr && ps_Arr->ucPtArr)
							{
								ucWaitArrFlag = 0;
								ucKWStep = 4;
								uiKWRunCnt = 0;
								
								//自动聚焦一次
								ps_Lens->CamAutoFocus();
								ps_Lens->IrAutoFocus();
								
							}
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
					uiKWRunCnt = 0;
					ucKWStep = 1;
				}
				
				break;
			}
			case 4:
			{
				switch (s_KWPara.ucOpera)
				{
					case 1:
					case 2:
					{
						if(uiKWRunCnt >= s_KWPara.ucKWAFTime * 60 * 1000)
						{
							uiKWRunCnt = 0;
							
							//自动聚焦一次
							ps_Lens->CamAutoFocus();
							ps_Lens->IrAutoFocus();
						}
						break;
					}
					default:
					{
						break;
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
/*
		线程
*/
static void KWProcess(MESSAGE Msg)
{
	KWRun();
	
	if(msgNull == Msg.usMsgID || (Msg.ucAddr != 0x01 && Msg.ucAddr != 0x02))
	{
		return;
	}
	
	unsigned short usData = Msg.ucData[0] << 8 | Msg.ucData[1];
	ucCtrlFlag = 0;
	
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
		case msgSetPreset:
		{
			if(usData >= setNum161 && usData <= setNum170)
			{
				if(usData >= 161 && usData <= 162)
				{
					s_KWPara.ucKWTime = usData - 160;
				}
				else if(usData >= 163 && usData <= 166)
				{
					s_KWPara.ucKWTime = (usData - 162) * 5;
				}
				else if(usData >= 167 && usData <= 170)
				{
					s_KWPara.ucKWTime = (usData - 164) * 10;
				}
				KWInit(0);
			}
			else
			{
				ucCtrlFlag = 1;
			}
			break;
		}
		case magCallPreset:
		{
			if(callNum212 == usData)	//守望开关
			{
				s_KWPara.ucKWSW = 1;
				uiKWRunCnt = 0;
				ucKWStep = 1;
				KWInit(0);
			}
			else if(callNum213 == usData)
			{
				s_KWPara.ucKWSW = 0;
				KWInit(0);
			}
			if(callNum214 == usData)	//守望后自动聚焦开关
			{
				s_KWPara.ucKWAFSW = 1;
				KWInit(0);
			}
			else if(callNum215 == usData)
			{
				s_KWPara.ucKWAFSW = 0;
				KWInit(0);
			}
			else
			{
				ucCtrlFlag = 1;
			}
			break;
		}
		case msgSetKWTime:	//通过协议设置守望时间 1-200min
		{
			if(0 == usData || usData > 200)
			{
				usData = 2;
			}
			s_KWPara.ucKWTime = usData;
			KWInit(0);
			break;
		}
		case msgSetKWAFTime:	////通过协议设置守望后等待自动聚焦的时间 1-200min
		{
			if(0 == usData || usData > 200)
			{
				usData = 2;
			}
			s_KWPara.ucKWAFTime = usData;
			KWInit(0);
			break;
		}
		case msgGetPtPan:	//查询指令不打断守望
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
	
	if(1 == ucCtrlFlag)
	{
		uiKWRunCnt = 0;
		ucKWStep = 1;
	}
}
