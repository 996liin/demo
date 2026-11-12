#include "headfile.h"
static void SetPostion(short nHsec, short nVsec);
static void SetPosRelat(short nHSec, short nVSec);//相对定位
static void FsmProcess(void);
static void StopPostion(void);
static void SelfCheck(void);
unsigned int uTick = 0;
struct_Fsm s_Fsm = 
{
	
	.FsmProcess = FsmProcess,
  .SetPostion = SetPostion,
	. SetPosRelat = SetPosRelat,
	.StopPostion = StopPostion,
	.SelfCheck = SelfCheck,
	.AutoCtrl_Flag = 0,
	.ManualCtrl_Flag = 0,
	.SpdH = 0,
	.SpdV = 0,
	.offset_h = 0,//跟踪模式下偏心跟踪偏移量
  .offset_v = 0,
};

struct_Fsm *ps_Fsm = &s_Fsm;
void SendCmd(unsigned char ucCmd, const unsigned char* pData, int nDataLen)
{
    
}
static void SetPostion(short nHSec, short nVSec)//绝对定位
{
    int nDataLen = 4;
    unsigned char szData[4];
    szData[0] = (nHSec >> 8);
    szData[1] = (nHSec & 0xFF);

    szData[2] = (nVSec >> 8);
    szData[3] = (nVSec & 0xFF);
	  SendCmd(0x32, szData, nDataLen);
}
static void SetPosRelat(short nHSec, short nVSec)//相对定位
{
    int nDataLen = 4;
    unsigned char szData[4];
    szData[0] = (nHSec >> 8);
    szData[1] = (nHSec & 0xFF);

    szData[2] = (nVSec >> 8);
    szData[3] = (nVSec & 0xFF);
	  SendCmd(0x34, szData, nDataLen);
}
static void StopPostion(void)
{
   int nDataLen = 4;
    unsigned char szData[4];
	  SendCmd(0x33, szData, nDataLen);
}
static void SelfCheck(void)
{
  SendCmd(0x31, NULL, 0);
}

static void FsmProcess(void)
{
   

}
