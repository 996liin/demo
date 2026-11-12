#include "headfile.h"

static void _3DParaInit(unsigned char ucOpera);
static void _3DProcess(MESSAGE Msg);
static Struct_3D s_3D = 
{
	._3DParaInit = _3DParaInit,
	._3DProcess = _3DProcess,
};
Struct_3D *ps_3D = &s_3D;

static Struct_3DPara s_3DPara;

/*
	参数初始化
*/
static void _3DParaInit(unsigned char ucOpera)
{
	if(0 == ucOpera)
	{
		ps_AT24CXX->AT24CXX_Write(_3DPRAADDR,(unsigned char *)&s_3DPara,sizeof(Struct_3DPara));
	}
	else
	{
		ps_AT24CXX->AT24CXX_Read(_3DPRAADDR,(unsigned char *)&s_3DPara,sizeof(Struct_3DPara));
		
		if(s_3DPara.ucManual3DSW > 1)
		{
			s_3DPara.ucManual3DSW = 0;
		}
		if(s_3DPara.unAuto3DSW > 1)
		{
			s_3DPara.unAuto3DSW = 0;
		}
	}
}
/*
*	形参为目标的左上右下坐标，低字节在前
*/
static void Ir3DRun(unsigned char *pCoor,unsigned short usIrZoomAD)
{
	unsigned short usX1 = *pCoor << 8 | *(pCoor+1);
	unsigned short usY1 = *(pCoor+2) << 8 | *(pCoor+3);
	unsigned short usX2 = *(pCoor+4) << 8 | *(pCoor+5);
	unsigned short usY2 = *(pCoor+6) << 8 | *(pCoor+7);
	
	unsigned short usScrX = 0, usScrY = 0, usScrCentreX = 0, usScrCentreY = 0;
	
	#if (IRCCD == IRCCD_HK640 || IRCCD == IRCCD_S640 || IRCCD == IRCCD_S600GA || IRCCD == IRCCD_S640_B20)
	
	usScrX = 640;
	usScrY = 512;
	usScrCentreX = usScrX / 2;
	usScrCentreY = usScrY / 2;
	
	#elif (IRCCD == IRCCD_HK384 || IRCCD == IRCCD_S384)
	
	usScrX = 384;
	usScrY = 288;
	usScrCentreX = usScrX / 2;
	usScrCentreY = usScrY / 2;
	
	#endif
	
	if(usX1 > usScrX || usX2 > usScrX || usY1 > usScrY || usY2 > usScrY)
	{
		return;
	}
	
	//目标坐标中心点
	unsigned short usTarCentreX = (usX1 + usX2) / 2;
	unsigned short usTarCentreY = (usY1 + usY2) / 2;
	
	//相对角度，正值为右、下
	short sRelativeHAngle = ps_Lens->GetIrViewZoom(1,usIrZoomAD) * (usTarCentreX - usScrCentreX) / usScrX;
	short sRelativeVAngle = ps_Lens->GetIrViewZoom(2,usIrZoomAD) * (usTarCentreY - usScrCentreY) / usScrY;
	
	if(sRelativeHAngle > 0)
	{
		ps_Pt->RelativeRightAngle(sRelativeHAngle);
	}
	else
	{
		ps_Pt->RelativeLeftAngle(abs(sRelativeHAngle));
	}
	
	ps_Systick->delay_ms(100);
	if(sRelativeVAngle > 0)
	{
		ps_Pt->RelativeDownAngle(sRelativeVAngle);
	}
	else
	{
		ps_Pt->RelativeUpAngle(abs(sRelativeVAngle));
	}
	
	//框选
	if((usX1 != usX2) && (usY1 != usY2))
	{
		if(1 == ps_Lens->GetIrZoomCheckDir())
		{
			if(usIrZoomAD - 700 > 0)
			{
				ps_Lens->SetIrZoomPosi(usIrZoomAD - 700);
			}
			else
			{
				ps_Lens->SetIrZoomPosi(0);
			}
		}
		else
		{
			ps_Lens->SetIrZoomPosi(usIrZoomAD + 700);
		}
	}

}
static void Cam3DRun(unsigned char *pCoor,unsigned short usCamZoomAD)
{
	unsigned short usX1 = *pCoor << 8 | *(pCoor+1);
	unsigned short usY1 = *(pCoor+2) << 8 | *(pCoor+3);
	unsigned short usX2 = *(pCoor+4) << 8 | *(pCoor+5);
	unsigned short usY2 = *(pCoor+6) << 8 | *(pCoor+7);
	
	unsigned short usScrX = 0, usScrY = 0, usScrCentreX = 0, usScrCentreY = 0;
	
	usScrX = 1920;
	usScrY = 1080;
	usScrCentreX = usScrX / 2;
	usScrCentreY = usScrY / 2;
	
	if(usX1 > usScrX || usX2 > usScrX || usY1 > usScrY || usY2 > usScrY)
	{
		return;
	}
	
	//目标坐标中心点
	unsigned short usTarCentreX = (usX1 + usX2) / 2;
	unsigned short usTarCentreY = (usY1 + usY2) / 2;
	
	//相对角度，正值为右、下
	short sRelativeHAngle = ps_Lens->GetCamViewZoom(1,usCamZoomAD) * (usTarCentreX - usScrCentreX) / usScrX;
	short sRelativeVAngle = ps_Lens->GetCamViewZoom(2,usCamZoomAD) * (usTarCentreY - usScrCentreY) / usScrY;
	
	if(sRelativeHAngle > 0)
	{
		ps_Pt->RelativeRightAngle(sRelativeHAngle);
	}
	else
	{
		ps_Pt->RelativeLeftAngle(abs(sRelativeHAngle));
	}
	
	ps_Systick->delay_ms(50);
	
	if(sRelativeVAngle > 0)
	{
		ps_Pt->RelativeDownAngle(sRelativeVAngle);
	}
	else
	{
		ps_Pt->RelativeUpAngle(abs(sRelativeVAngle));
	}
	
	//框选
	if((usX1 != usX2) && (usY1 != usY2))
	{
		if(1 == ps_Lens->GetCamZoomCheckDir())
		{
			if(usCamZoomAD - 700 > 0)
			{
				ps_Lens->SetCamZoomPosi(usCamZoomAD - 700);
			}
			else
			{
				ps_Lens->SetCamZoomPosi(0);
			}
		}
		else
		{
			ps_Lens->SetCamZoomPosi(usCamZoomAD + 700);
		}
	}

}

unsigned char ucSendToPC[100] = {0x6e,0x00,30,1,3,9,0x00,};
void alarmPacket(unsigned char *pAlarmData)
{
	ucSendToPC[6] = ps_Pt->GetPtHAngle() & 0xff;
	ucSendToPC[7] = ps_Pt->GetPtHAngle() >> 8;
	ucSendToPC[8] = ps_Pt->GetPtVAngle() & 0xff;
	ucSendToPC[9] = ps_Pt->GetPtVAngle() >> 8;
	for(unsigned char uci = 0; uci <= 15; uci++)
	{
		ucSendToPC[10+uci] = 0;
	}
	ucSendToPC[26] = 1;	//1个报警目标
	memcpy(ucSendToPC,pAlarmData,8);
	
	ucSendToPC[35] = 0;
	for(unsigned char uci = 0; uci < 1+20+9; uci++)
	{
		ucSendToPC[35] += ucSendToPC[uci+5];
	}
	ucSendToPC[36] = 0xe6;
	
	ps_Usart->Usart1_SendData(ucSendToPC,37);
}
static void _3DProcess(MESSAGE Msg)
{
	if(Msg.usMsgID == msgNull || Msg.ucAddr != 0x02)
	{
		return;
	}
	
	switch(Msg.usMsgID)
	{
		case msgSRCom:
		{
			if(2 == Msg.ucData[0])	//3d定位参数
			{
				switch (Msg.ucData[1])
				{
					case 0x00:	//状态查询
					{
						break;
					}
					case 0x01:	//手动3D开关
					{
						s_3DPara.ucManual3DSW = 1;
						s_3DPara.unAuto3DSW = 0;
						_3DParaInit(0);
						break;
					}
					case 0x02:	//自动3D开关
					{
						s_3DPara.ucManual3DSW = 0;
						s_3DPara.unAuto3DSW = 1;
						_3DParaInit(0);
						break;
					}
					case 0x03:	//3D定位指令
					{
						if(0x00 == Msg.ucData[2])	//热像缩小
						{
							if(0 == ps_Lens->GetIrZoomCheckDir())	//tele方向ad值较大
							{
								if(ps_ADC->GetIrZoomAD() - 700 > 0)
								{
									ps_Lens->SetIrZoomPosi(ps_ADC->GetIrZoomAD() - 700);
								}
								else
								{
									ps_Lens->SetIrZoomPosi(0);
								}
							}
							else
							{
								ps_Lens->SetIrZoomPosi(ps_ADC->GetIrZoomAD() + 700);
							}
						}
						else if(0x01 == Msg.ucData[2])	//热像放大
						{
							static unsigned char pData[8] = {0,};
							for(unsigned char ucCnt = 0; ucCnt < 8; ucCnt++)
							{
								pData[ucCnt] = Msg.ucData[3+ucCnt];
							}
							
							Ir3DRun(pData,ps_ADC->GetIrZoomAD());
						}
						else if(0x02 == Msg.ucData[2])	//可见光放大
						{
							static unsigned char pData[8] = {0,};
							for(unsigned char ucCnt = 0; ucCnt < 8; ucCnt++)
							{
								pData[ucCnt] = Msg.ucData[3+ucCnt];
							}
							
							Cam3DRun(pData,ps_ADC->GetCamZoomAD());
						}
						else if(0x03 == Msg.ucData[2])	//可见光缩小
						{
							if(0 == ps_Lens->GetCamZoomCheckDir())	//tele方向ad值较大
							{
								if(ps_ADC->GetCamZoomAD() - 700 > 0)
								{
									ps_Lens->SetCamZoomPosi(ps_ADC->GetCamZoomAD() - 700);
								}
								else
								{
									ps_Lens->SetCamZoomPosi(0);
								}
							}
							else
							{
								ps_Lens->SetCamZoomPosi(ps_ADC->GetCamZoomAD() + 700);
							}
						}
						break;
					}
					default:
						break;
				}
			}
		}
		default:
			break;
	}
}
	
	
