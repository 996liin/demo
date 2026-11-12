#include "headfile.h"
/*
*	可见光以及红外机芯控制
*/
static uint16_t CamZoomViewBuf[35][2] =        //实际标定AD值与视场角对应关系
{
    {	0x4000	,	132	,	}	,
    {	0x3fe2	,	135	,	}	,
    {	0x3e20	,	190	,	}	,
    {	0x3c39	,	256	,	}	,
    {	0x39ed,	372,	}	,
    {	0x38af	,	450	,	}	,
    {	0x3788	,	548,	}	,
    {	0x32a9	,	802	,	}	,
    {	0x3171	,	868	,	}	,
    {	0x2e7a	,	1039	,	}	,
    {	0x2c41	,	1186	,	}	,
    {	0x29c4	,	1360	,	}	,
    {	0x2762	,	1535	,	}	,
    {	0x24a2	,	1761,	}	,
    {	0x230b,	1902	,	}	,
    {	0x2190	,	2028	,	}	,
    {	0x2131	,	2063,	}	,
    {	0x2066	,	2135	,	}	,
    {	0x1f8d	,	2214	,	}	,
    {	0x1e3a	,	2346	,	}	,
    {	0x1c37,	2541	,	}	,
    {	0x1b28	,	2654	,	}	,
    {	0x1969	,	2839	,	}	,
    {	0x1773	,	3051	,	}	,
    {	0x15f8,	3214	,	}	,
    {	0x146f,	3390	,	}	,
    {	0x11e4	,	3700	,	}	,
    {	0xfef	,	3929	,	}	,
    {	0xc99	,	4334	,	}	,
    {	0xa0f,	4655	,	}	,
    {	0x8ca	,	4831	,	}	,
    {	0x5b8,	5229	,	}	,
    {	0x3eb,	5455	,	}	,
    {	0x247	,	5692	,	}	,
    {	0,	6120	,	}	,


};
static uint16_t Search_zoom_view(uint16_t usCamPosi);
static uint8_t flag_inq_zoomfocus;
//static unsigned char ucCamAutoCtrlFlag = 1;
//static unsigned char ucIrAutoCtrlFlag = 0;
//static unsigned int uiAutoCtrlDayCnt = 0;
//static unsigned int uiAutoCtrlDayNightCnt = 0;
static unsigned char query_witch = 1;//下发哪条查询指令
static void Camera_GoToTarget(unsigned char select, unsigned short value_target);
static void zoom_inquiry(void);
static void focus_inquiry(void);
static void init(void);
static void Process(void);
static void Camera_ZoomTele(void);
static void Camera_ZoomWide(void);
static void Camera_FocusFar(void);
static void Camera_FocusNear(void);
static void Camera_IrisOpen(void);
static void Camera_IrisClose(void);
static void Camera_Stop(void);
static void Cmd_Ctrl(unsigned char cmd, unsigned short data);
static void Set_PT_OSD(unsigned short h, signed short v);
static void ProcessCmd(Cam_Opt_Type cam_cmd, uint16_t data);
static void ProcessInq(void);
/*
	函数结构体-外部
*/
static struct_CCD s_CCD =
{
    .init            = init,
    .zoom_inquiry       = zoom_inquiry,
    .focus_inquiry       = focus_inquiry,
    .Camera_GoToTarget  = Camera_GoToTarget,
    .Process         = Process,
    .Camera_ZoomTele    = Camera_ZoomTele,
    .Camera_ZoomWide    = Camera_ZoomWide,
    .Camera_FocusFar    = Camera_FocusFar,
    .Camera_FocusNear   = Camera_FocusNear,
    .Camera_IrisOpen    = Camera_IrisOpen,
    .Camera_IrisClose   = Camera_IrisClose,
    .Camera_Stop        = Camera_Stop,
    .Cmd_Ctrl           = Cmd_Ctrl,
    .Set_PT_OSD         = Set_PT_OSD,
    .icr_open_flag      = 0,
    .fog_open_flag      = 0,
    .Search_zoom_view   = Search_zoom_view,
    .ProcessCmd         = ProcessCmd,
    .ProcessInq         = ProcessInq,
    .runstate           = null,
    .stoptimecnt            = 0,
};
struct_CCD *ps_CCD = &s_CCD;
/*
	参数结构体-内部
*/
//static struct_CCDPara s_CCDPara;

static uint8_t zoom_state = 0;
static uint8_t focus_state = 0;

/*
	设备参数结构体-外部
*/

static struct_devicePara s_devicePara;
struct_devicePara *ps_devicePara = &s_devicePara;
/*
	S600GA-制冷机芯打包函数
*/
static void IrComPack(unsigned char ucCom1, unsigned char ucCom2, unsigned char ucCom3, unsigned char ucCom4)
{
//	unsigned char ucCom[] = {0x55,0xaa,ucCom1,ucCom2,ucCom3,ucCom4,0x00,0xf0};
//	ucCom[6] = (ucCom1^ucCom2^ucCom3^ucCom4)&0xff;
    //ps_Usart->Uart4_SendData(ucCom,8);
}
/*
	初始化
*/
static void init(void)
{
    s_CCD.UartSendData = ps_Usart->Uart7_SendData;
}


/*
	定时器相关
*/
static void CCDTimer(void)
{

}
//摄像机字符叠加 7047
static unsigned char ucDispEn[7] = {0x5a, 0x04, 0x03, 0x00, 0x00, 0x00, 0xa5};
static unsigned char ucDispBuf[200] = {0x5a, 0x00, 0x04, 0x00,};
static void CamDispEnable(unsigned char ucLine, unsigned char ucEnable)
{
//	ucDispEn[3] = ucLine;
//	ucDispEn[4] = ucEnable;
//	ucDispEn[5] = (ucDispEn[4]+ucDispEn[3]+ucDispEn[2]+ucDispEn[1])&0xff;
//	s_CCD.UartSendData(ucDispEn,7);
}
static void CamDispFunc(unsigned char ucLine, unsigned char *pStr, unsigned char ucSize)
{
    ucDispBuf[1] = ucSize + 3;
    ucDispBuf[3] = ucLine;

    for(unsigned char ucCnt = 0; ucCnt < ucSize; ucCnt++)
    {
        ucDispBuf[4 + ucCnt] = pStr[ucCnt];
    }

    ucDispBuf[ucSize + 4] = 0;

    for(unsigned char ucCnt = 0; ucCnt < ucSize + 3; ucCnt++)
    {
        ucDispBuf[ucSize + 4] += ucDispBuf[1 + ucCnt];
    }

    ucDispBuf[ucSize + 5] = 0xa5;

    s_CCD.UartSendData(ucDispBuf, ucSize + 6);
}
/*
	线程
*/

static void Process(void)
{
    if(s_COMCCD.FlagRxdEnd)
    {
        s_COMCCD.FlagRxdEnd = 0;
        unsigned short value;

        if(s_CCD.query_witch == 1)
        {
            value = s_COMCCD.Rxd[2] & 0x0F;
            value = ( value << 4) | (s_COMCCD.Rxd[3] & 0x0F);
            value = ( value << 4) | (s_COMCCD.Rxd[4] & 0x0F);
            value = ( value << 4) | (s_COMCCD.Rxd[5] & 0x0F);
            s_CCD.zoomvalue = value;
            s_CCD.query_witch = 0;
        }
        else if(s_CCD.query_witch == 2)
        {
            value = s_COMCCD.Rxd[2] & 0x0F;
            value = ( value << 4) | (s_COMCCD.Rxd[3] & 0x0F);
            value = ( value << 4) | (s_COMCCD.Rxd[4] & 0x0F);
            value = ( value << 4) | (s_COMCCD.Rxd[5] & 0x0F);
            s_CCD.focusvalue = value / 10;
            s_CCD.query_witch = 0;
        }
        else if(s_CCD.query_witch == 3)
        {
            if(s_COMCCD.Rxd[2] == 2)
            {
                s_CCD.icr_open_flag = 1;
            }
            else if(s_COMCCD.Rxd[2] == 3)
            {
                s_CCD.icr_open_flag = 0;
            }

        }
        else {}
    }
}
static void Camera_GoToTarget(unsigned char select, unsigned short value_target)
{
    unsigned char cmd[9] = {0x81, 0x01, 0x04, NULL, NULL, NULL, NULL, NULL, 0xFF};
    unsigned char cmd_focus_manual[6] = {0x81, 0x01, 0x04, 0x38, 0x03, 0xFF}; //聚焦手动
    unsigned char cmd_focus_auto[6] = {0x81, 0x01, 0x04, 0x38, 0x02, 0xFF}; //聚焦自动
    unsigned char cmd_focus_far_1speed[6] = {0x81, 0x01, 0x04, 0x08, 0x21, 0xFF};

    if(select == 0)
    {
        cmd[3] = 0x47;
        //s_CCD.zoomvalue = value_target;
        s_CCD.runstate = locate_zoom;
    }
    else if(select == 1)
    {
        cmd[3] = 0x48;
        //s_CCD.focusvalue = value_target;
        s_CCD.runstate = locate_focus;
        value_target = value_target * 10;
        s_CCD.UartSendData(cmd_focus_far_1speed, 6);//设置聚焦位置前需要先控制一下聚焦
    }

    cmd[4] = (value_target >> 12) & 0x0F;
    cmd[5] = (value_target >> 8) & 0x0F;
    cmd[6] = (value_target >> 4) & 0x0F;
    cmd[7] = value_target & 0x0F;
    s_CCD.UartSendData(cmd, 9);
    s_CCD.stoptimecnt = 0;
}

static void zoom_inquiry(void)
{
    unsigned char cmd_zoom_inquiry[5] = {0x81, 0x09, 0x04, 0x47, 0xFF};
    //unsigned char cmd_zoom_inquiry[5] = {0x81, 0x09, 0x04, 0x4b, 0xFF};
    s_CCD.UartSendData(cmd_zoom_inquiry, 5);
    s_CCD.query_witch = 1;
}
static void focus_inquiry(void)
{
    unsigned char cmd_focus_inquiry[5] = {0x81, 0x09, 0x04, 0x48, 0xFF};
    s_CCD.UartSendData(cmd_focus_inquiry, 5);
    s_CCD.query_witch = 2;
}
static void icrmode_inquiry(void)
{
    unsigned char cmd_icr_inquiry[5] = {0x81, 0x09, 0x04, 0x01, 0xFF};
    s_CCD.UartSendData(cmd_icr_inquiry, 5);
    s_CCD.query_witch = 3;
}
/* 长焦方向 */
static void Camera_ZoomTele(void)
{
    unsigned char cmd_zoom_tele[6] = {0x81, 0x01, 0x04, 0x07, 0x02, 0xFF};
    //s_CCD.UartSendData(cmd_zoom_tele, 6);
    s_CCD.runstate = zoom;
    zoom_state = 1;
    s_CCD.stoptimecnt = 0;
}
/* 广角方向 */
static void Camera_ZoomWide(void)
{
    unsigned char cmd_zoom_wide[6] = {0x81, 0x01, 0x04, 0x07, 0x03, 0xFF};
    //s_CCD.UartSendData(cmd_zoom_wide, 6);
    s_CCD.runstate = zoom;
    s_CCD.stoptimecnt = 0;
    zoom_state = 1;
}
/* 聚焦远 */
static void Camera_FocusFar(void)
{
    unsigned char cmd_focus_far[6] = {0x81, 0x01, 0x04, 0x08, 0x02, 0xFF};
    //s_CCD.UartSendData(cmd_focus_far, 6);
    s_CCD.runstate = focus;
    s_CCD.stoptimecnt = 0;
    focus_state = 1;
}
/* 聚焦近 */
static void Camera_FocusNear(void)
{
    unsigned char cmd_focus_near[6] = {0x81, 0x01, 0x04, 0x08, 0x03, 0xFF};
    //s_CCD.UartSendData(cmd_focus_near, 6);
    s_CCD.runstate = focus;
    s_CCD.stoptimecnt = 0;
    focus_state = 1;
}
/* 光圈增大 */
static void Camera_IrisOpen(void)
{
    unsigned char cmd_iris_up[6] = {0x81, 0x01, 0x04, 0x0B, 0x02, 0xFF};
    //s_CCD.UartSendData(cmd_iris_up, 6);
}
/* 光圈减小 */
static void Camera_IrisClose(void)
{
    unsigned char cmd_iris_dn[6] = {0x81, 0x01, 0x04, 0x0B, 0x03, 0xFF};
    //s_CCD.UartSendData(cmd_iris_dn, 6);
}
/* 变倍、聚焦、光圈 、激光都停止 */
static void Camera_Stop(void)
{
    unsigned char cmd_zoom_stop[6] = {0x81, 0x01, 0x04, 0x07, 0x00, 0xFF};
    unsigned char cmd_focus_stop[6] = {0x81, 0x01, 0x04, 0x08, 0x00, 0xFF};
    //unsigned char cmd_iris_reset[6] = {0x81, 0x01, 0x04, 0x0B, 0x00, 0xFF};

    if (focus_state)
    {
        focus_state = 0;
        s_CCD.UartSendData(cmd_focus_stop, 6);
    }

    if (zoom_state)
    {
        zoom_state = 0;
        s_CCD.UartSendData(cmd_zoom_stop, 6);
    }

    s_CCD.runstate = stop;
    s_CCD.stoptimecnt = 0;
}
static void Cmd_Ctrl(unsigned char cmd, unsigned short data)
{
    unsigned char cmd_icr_open[6] = {0x81, 0x01, 0x04, 0x01, 0x02, 0xFF}; //打开黑白
    unsigned char cmd_icr_close[6] = {0x81, 0x01, 0x04, 0x01, 0x03, 0xFF}; //打开黑白
    unsigned char cmd_icr_auto_off[6] = {0x81, 0x01, 0x04, 0x51, 0x03, 0xFF}; //关闭自动
    unsigned char cmd_icr_auto_on[6] = {0x81, 0x01, 0x04, 0x51, 0x02, 0xFF}; //打开自动

    switch (cmd)
    {
        case 0x01:
        {
            s_CCD.Camera_ZoomTele();
            break;
        }

        case 0x02:
        {
            s_CCD.Camera_ZoomWide();
            break;
        }

        case 0x03:
        {
            s_CCD.Camera_Stop();
            break;
        }

        case 0x04:
        {
            s_CCD.Camera_FocusFar();
            break;
        }

        case 0x05:
        {
            s_CCD.Camera_FocusNear();
            break;
        }

        case 0x06:
        {
            s_CCD.Camera_Stop();
            break;
        }

        case 0x07:
        {
            s_CCD.UartSendData(cmd_icr_auto_off, 6);
            ps_Systick->delay_ms(50);
            s_CCD.UartSendData(cmd_icr_open, 6);
            ps_Systick->delay_ms(100);
            icrmode_inquiry();

            break;
        }

        case 0x08:
        {
            s_CCD.UartSendData(cmd_icr_auto_off, 6);
            ps_Systick->delay_ms(50);
            s_CCD.UartSendData(cmd_icr_close, 6);
            ps_Systick->delay_ms(100);
            icrmode_inquiry();
            break;
        }

        case 0x10:
        {
            s_CCD.Camera_GoToTarget(0, data);
            break;
        }

        default:
            break;
    }
}
static void Set_PT_OSD(unsigned short h, signed short v)
{
    #if 0
    u8 data_back[13] = {0};
    data_back[0] = 0x81;
    data_back[1] = 0x01;
    data_back[2] = 0x09;
    data_back[3] = 0x79;
    h += 18000;

    if(h >= 36000)
    {
        h -= 36000;
    }

    data_back[4] = (h >> 12) & 0x0f;
    data_back[5] = (h >> 8) & 0x0f;
    data_back[6] = (h >> 4) & 0x0f;
    data_back[7] = h & 0x0f;
    v += 18000;

    if(v >= 36000)
    {
        v -= 36000;
    }

    data_back[8] = (v >> 12) & 0x0f;
    data_back[9] = (v >> 8) & 0x0f;
    data_back[10] = (v >> 4) & 0x0f;
    data_back[11] = v & 0x0f;
    data_back[12] = 0xff;
    s_CCD.UartSendData(data_back, 13);
    #else
    unsigned char ucCom[7] = {0xff, 0x01, 0x00, 0x59, 0x00, 0x00, 0x00};
    ucCom[3] = 0x59;
    ucCom[4] = h >> 8;
    ucCom[5] = h & 0xff;
    ptr_Ccd422Func->SendMessage(protoPELCO_D, ptr_Ccd422Func->u8Address, 0, &ucCom[2], 4);
    ucCom[3] = 0x5b;

    if(v < 0)
    {
        v += 36000;
    }

//	 if(v<0)
//	 {
//		 v = -v;
//	 }
//	 else
//	 {
//		 v = 36000 - v;
//	 }
    ucCom[4] = v >> 8;
    ucCom[5] = v & 0xff;
    ptr_Ccd422Func->SendMessage(protoPELCO_D, ptr_Ccd422Func->u8Address, 0, &ucCom[2], 4);
    #endif

}
static uint16_t Search_zoom_view(uint16_t usCamPosi)
{
    uint16_t usReturnData = 0;

    if(usCamPosi > CamZoomViewBuf[0][0])
    {
        usCamPosi = CamZoomViewBuf[0][0];
    }
    else if(usCamPosi < CamZoomViewBuf[34][0])
    {
        usCamPosi = CamZoomViewBuf[34][0];
    }

    for(unsigned short usCnt = 0; usCnt < 34; usCnt++)
    {
        if(usCamPosi <= CamZoomViewBuf[usCnt][0] && usCamPosi >= CamZoomViewBuf[usCnt + 1][0])
        {
            usReturnData = CamZoomViewBuf[usCnt][1] + \
                           (CamZoomViewBuf[usCnt + 1][1] - CamZoomViewBuf[usCnt][1]) * \
                           (CamZoomViewBuf[usCnt][0] - usCamPosi) / \
                           (CamZoomViewBuf[usCnt][0] - CamZoomViewBuf[usCnt + 1][0]);
            break;
        }
    }

    return usReturnData;
}
static void ProcessCmd(Cam_Opt_Type cam_cmd, uint16_t data)
{
    uint8_t m_Camera_UART_Buff[10];
    uint8_t realSend = 0;
    uint8_t m_Camera_UART_Send_Count = 6;

    m_Camera_UART_Buff[0] = 0x81;
    m_Camera_UART_Buff[1] = 0x01;
    m_Camera_UART_Buff[5] = 0xFF;

    switch (cam_cmd)
    {
        case Zoom_Tele:
            zoom_state = 1;
            realSend = 1;

//    	if (data == 0)
//    		data = 7;
            if (data > 7)
                data = 7;

            m_Camera_UART_Buff[2] = 0x04;
            m_Camera_UART_Buff[3] = 0x07;

            if(!data)
            {
                m_Camera_UART_Buff[4] = 0;
            }
            else
            {
                m_Camera_UART_Buff[4] = (0x20 + data);
            }

            s_CCD.runstate = zoom;
            s_CCD.stoptimecnt = 0;
            break;

        case Zoom_Wide:
            zoom_state = 1;
            realSend = 1;

//    	if (data == 0)
//    		data = 7;
            if (data > 7)
                data = 7;

            m_Camera_UART_Buff[2] = 0x04;
            m_Camera_UART_Buff[3] = 0x07;

            if(!data)
            {
                m_Camera_UART_Buff[4] = 0;
            }
            else
            {
                m_Camera_UART_Buff[4] = (0x30 + data);
            }

            s_CCD.runstate = zoom;
            s_CCD.stoptimecnt = 0;
            break;

        case Zoom_Stop:
            if (1)
            {
                realSend = data;
                m_Camera_UART_Buff[2] = 0x04;
                m_Camera_UART_Buff[3] = 0x07;
                m_Camera_UART_Buff[4] = 0x00;
            }
            else
            {
                return;
            }

            s_CCD.runstate = stop;
            s_CCD.stoptimecnt = 0;
            break;

        case DZoom_On:
            m_Camera_UART_Buff[2] = 0x04;
            m_Camera_UART_Buff[3] = 0x06;
            m_Camera_UART_Buff[4] = 0x02;
            break;

        case DZoom_Off:
            m_Camera_UART_Buff[2] = 0x04;
            m_Camera_UART_Buff[3] = 0x06;
            m_Camera_UART_Buff[4] = 0x03;
            break;

        case Focus_Auto:
            realSend = 1;
            //??????
            m_Camera_UART_Buff[2] = 0x04;
            m_Camera_UART_Buff[3] = 0x38;
            m_Camera_UART_Buff[4] = 0x02;
            s_CCD.UartSendData(m_Camera_UART_Buff, m_Camera_UART_Send_Count);
            ps_Systick->delay_ms(10);

            //??????
            m_Camera_UART_Buff[2] = 0x04;
            m_Camera_UART_Buff[3] = 0x57;
            m_Camera_UART_Buff[4] = 0x00;
            break;

        case Focus_Trigger:
            realSend = 1;
            //??????
            m_Camera_UART_Buff[2] = 0x04;
            m_Camera_UART_Buff[3] = 0x38;
            m_Camera_UART_Buff[4] = 0x02;
            s_CCD.UartSendData(m_Camera_UART_Buff, m_Camera_UART_Send_Count);
            ps_Systick->delay_ms(50);

            //????
            m_Camera_UART_Buff[2] = 0x04;
            m_Camera_UART_Buff[3] = 0x57;
            m_Camera_UART_Buff[4] = 0x02;
            break;

        case Focus_Manual:
            realSend = 1;
            m_Camera_UART_Buff[2] = 0x04;
            m_Camera_UART_Buff[3] = 0x38;
            m_Camera_UART_Buff[4] = 0x03;
            break;

        case Focus_Far:
            focus_state = 1;
            realSend = 1;

//    	if (data == 0)
//    		data = 7;
            if (data > 7)
                data = 7;

            m_Camera_UART_Buff[2] = 0x04;
            m_Camera_UART_Buff[3] = 0x08;

            if(!data)
            {
                m_Camera_UART_Buff[4] = 0;
            }
            else
            {
                m_Camera_UART_Buff[4] = (0x20 + data);
            }

            s_CCD.runstate = focus;
            s_CCD.stoptimecnt = 0;
            break;

        case Focus_Near:
            focus_state = 1;
            realSend = 1;

//    	if (data == 0)
//    		data = 7;
            if (data > 7)
                data = 7;

            m_Camera_UART_Buff[2] = 0x04;
            m_Camera_UART_Buff[3] = 0x08;

            if(!data)
            {
                m_Camera_UART_Buff[4] = 0;
            }
            else
            {
                m_Camera_UART_Buff[4] = (0x30 + data);
            }

            s_CCD.runstate = focus;
            s_CCD.stoptimecnt = 0;
            break;

        case Focus_Stop:
            if (1)
            {
                realSend = 1;
                m_Camera_UART_Buff[2] = 0x04;
                m_Camera_UART_Buff[3] = 0x08;
                m_Camera_UART_Buff[4] = 0x00;
            }
            else
            {
                return;
            }

            s_CCD.runstate = stop;
            s_CCD.stoptimecnt = 0;
            break;

        case QuryZoomPos:

            break;

        case SetZoomPos:

            break;

        case QuryFocusPos:

            break;

        case SetFocusPos:

            break;

        case Icr_Auto:
            realSend = 1;

            m_Camera_UART_Buff[2] = 0x04;
            m_Camera_UART_Buff[3] = 0x51;
            m_Camera_UART_Buff[4] = 0x02;
            break;

        case Icr_On:
            realSend = 1;
            m_Camera_UART_Buff[2] = 0x04;
            m_Camera_UART_Buff[3] = 0x51;
            m_Camera_UART_Buff[4] = 0x03;
            s_CCD.UartSendData(m_Camera_UART_Buff, m_Camera_UART_Send_Count);
            ps_Systick->delay_ms(10);

            m_Camera_UART_Buff[2] = 0x04;
            m_Camera_UART_Buff[3] = 0x01;
            m_Camera_UART_Buff[4] = 0x02;
            break;

        case Icr_Off:
            realSend = 1;
            m_Camera_UART_Buff[2] = 0x04;
            m_Camera_UART_Buff[3] = 0x51;
            m_Camera_UART_Buff[4] = 0x03;
            s_CCD.UartSendData(m_Camera_UART_Buff, m_Camera_UART_Send_Count);
            ps_Systick->delay_ms(10);

            m_Camera_UART_Buff[2] = 0x04;
            m_Camera_UART_Buff[3] = 0x01;
            m_Camera_UART_Buff[4] = 0x03;
            break;

        case Dnr:
            realSend = 1;
            m_Camera_UART_Buff[2] = 0x04;
            m_Camera_UART_Buff[3] = 0x53;
            m_Camera_UART_Buff[4] = data;
            break;

        case WD_On:
            realSend = 1;
            m_Camera_UART_Buff[2] = 0x04;
            m_Camera_UART_Buff[3] = 0x3D;
            m_Camera_UART_Buff[4] = 0x02;
            break;

        case WD_Off:
            realSend = 1;
            m_Camera_UART_Buff[2] = 0x04;
            m_Camera_UART_Buff[3] = 0x3D;
            m_Camera_UART_Buff[4] = 0x03;
            break;

        case SmartIR_On:
            realSend = 1;
            m_Camera_UART_Buff[2] = 0x06;
            m_Camera_UART_Buff[3] = 0x09;
            m_Camera_UART_Buff[4] = 0x02;
            break;

        case SmartIR_Off:
            realSend = 1;
            m_Camera_UART_Buff[2] = 0x06;
            m_Camera_UART_Buff[3] = 0x09;
            m_Camera_UART_Buff[4] = 0x03;
            break;

        case Menu_On:
            realSend = 1;
            m_Camera_UART_Buff[2] = 0x04;
            m_Camera_UART_Buff[3] = 0x15;
            m_Camera_UART_Buff[4] = 0x02;
            break;

        case Menu_Off:
            realSend = 1;
            m_Camera_UART_Buff[2] = 0x04;
            m_Camera_UART_Buff[3] = 0x15;
            m_Camera_UART_Buff[4] = 0x03;
            break;

        case Menu_Enter:
            realSend = 1;
            m_Camera_UART_Buff[2] = 0x06;
            m_Camera_UART_Buff[3] = 0x00;
            m_Camera_UART_Buff[4] = 0x01;
            break;

        case Menu_Up:
            realSend = 1;
            m_Camera_UART_Buff[2] = 0x06;
            m_Camera_UART_Buff[3] = 0x00;
            m_Camera_UART_Buff[4] = 0x02;
            break;

        case Menu_Down:
            realSend = 1;
            m_Camera_UART_Buff[2] = 0x06;
            m_Camera_UART_Buff[3] = 0x00;
            m_Camera_UART_Buff[4] = 0x04;
            break;

        case Menu_Left:
            realSend = 1;
            m_Camera_UART_Buff[2] = 0x06;
            m_Camera_UART_Buff[3] = 0x00;
            m_Camera_UART_Buff[4] = 0x08;
            break;

        case Menu_Right:
            realSend = 1;
            m_Camera_UART_Buff[2] = 0x06;
            m_Camera_UART_Buff[3] = 0x00;
            m_Camera_UART_Buff[4] = 0x10;
            break;

        case Menu_ZoomInfo_On:
            realSend = 1;
            m_Camera_UART_Buff[2] = 0x06;
            m_Camera_UART_Buff[3] = 0x12;
            m_Camera_UART_Buff[4] = 0x02;
            break;

        case Menu_ZoomInfo_Off:
            realSend = 1;
            m_Camera_UART_Buff[2] = 0x06;
            m_Camera_UART_Buff[3] = 0x12;
            m_Camera_UART_Buff[4] = 0x03;
            break;

        case Output_30FPS:
            m_Camera_UART_Send_Count = 8;
            m_Camera_UART_Buff[2] = 0x04;
            m_Camera_UART_Buff[3] = 0x24;
            m_Camera_UART_Buff[4] = 0x72;
            m_Camera_UART_Buff[5] = 0x00;
            m_Camera_UART_Buff[6] = 0x0E;
            m_Camera_UART_Buff[7] = 0xFF;
            break;

        case Output_25FPS:
            m_Camera_UART_Send_Count = 8;
            m_Camera_UART_Buff[2] = 0x04;
            m_Camera_UART_Buff[3] = 0x24;
            m_Camera_UART_Buff[4] = 0x72;
            m_Camera_UART_Buff[5] = 0x01;
            m_Camera_UART_Buff[6] = 0x01;
            m_Camera_UART_Buff[7] = 0xFF;
            break;

        case Stab_On:
            realSend = 1;
            m_Camera_UART_Buff[2] = 0x04;
            m_Camera_UART_Buff[3] = 0x34;
            m_Camera_UART_Buff[4] = 0x02;
            break;

        case Stab_Off:
            realSend = 1;
            m_Camera_UART_Buff[2] = 0x04;
            m_Camera_UART_Buff[3] = 0x34;
            m_Camera_UART_Buff[4] = 0x03;
            break;

        case Mirror_Image_On:
            realSend = 1;
            m_Camera_UART_Buff[2] = 0x04;
            m_Camera_UART_Buff[3] = 0x61;
            m_Camera_UART_Buff[4] = 0x02;
            break;

        case Mirror_Image_Off:
            realSend = 1;
            m_Camera_UART_Buff[2] = 0x04;
            m_Camera_UART_Buff[3] = 0x61;
            m_Camera_UART_Buff[4] = 0x03;
            break;

        case SlowShutterSw:
            realSend = 1;
            m_Camera_UART_Buff[2] = 0x04;
            m_Camera_UART_Buff[3] = 0x5A;

            if (data == 0)
            {
                m_Camera_UART_Buff[4] = 0x03;
            }
            else if (data == 1)
            {
                m_Camera_UART_Buff[4] = 0x02;
            }

            break;

        case GainSet:
            realSend = 1;
            m_Camera_UART_Send_Count = 9;

            m_Camera_UART_Buff[2] = 0x04;
            m_Camera_UART_Buff[3] = 0x4C;
            m_Camera_UART_Buff[4] = 0x00;
            m_Camera_UART_Buff[5] = 0x00;
            m_Camera_UART_Buff[6] = data & 0xF0;
            m_Camera_UART_Buff[6] = m_Camera_UART_Buff[6] >> 4;
            m_Camera_UART_Buff[6] = m_Camera_UART_Buff[6] & 0x0F;
            m_Camera_UART_Buff[7] = data & 0x0F;;
            m_Camera_UART_Buff[8] = 0xFF;
            break;

        case Ae:
            realSend = 1;
            m_Camera_UART_Buff[2] = 0x04;
            m_Camera_UART_Buff[3] = 0x39;

            if (data == 0)
            {
                m_Camera_UART_Buff[4] = 0x00;
            }
            else if (data == 1)
            {
                m_Camera_UART_Buff[4] = 0x03;
            }
            else if (data == 2)
            {
                m_Camera_UART_Buff[4] = 0x0A;
            }
            else if (data == 3)
            {
                m_Camera_UART_Buff[4] = 0x0B;
            }

            break;

        case Shutter_default:
            realSend = 1;
            m_Camera_UART_Buff[2] = 0x04;
            m_Camera_UART_Buff[3] = 0x0A;
            m_Camera_UART_Buff[4] = 0x00;
            break;

        case Shutter_plus:
            realSend = 1;
            m_Camera_UART_Buff[2] = 0x04;
            m_Camera_UART_Buff[3] = 0x0A;
            m_Camera_UART_Buff[4] = 0x02;

            break;

        case Shutter_sub:
            realSend = 1;
            m_Camera_UART_Buff[2] = 0x04;
            m_Camera_UART_Buff[3] = 0x0A;
            m_Camera_UART_Buff[4] = 0x03;
            break;

        case Shutter_Set:
            realSend = 1;
            m_Camera_UART_Send_Count = 9;

            m_Camera_UART_Buff[2] = 0x04;
            m_Camera_UART_Buff[3] = 0x4A;
            m_Camera_UART_Buff[4] = 0x00;
            m_Camera_UART_Buff[5] = 0x00;
            m_Camera_UART_Buff[6] = data & 0xF0;
            m_Camera_UART_Buff[6] = m_Camera_UART_Buff[6] >> 4;
            m_Camera_UART_Buff[6] = m_Camera_UART_Buff[6] & 0x0F;
            m_Camera_UART_Buff[7] = data & 0x0F;
            m_Camera_UART_Buff[8] = 0xFF;
            break;

        case Iris_default:
            realSend = 1;
            m_Camera_UART_Buff[2] = 0x04;
            m_Camera_UART_Buff[3] = 0x0B;
            m_Camera_UART_Buff[4] = 0x00;
            break;

        case Iris_plus:
            realSend = 1;
            m_Camera_UART_Buff[2] = 0x04;
            m_Camera_UART_Buff[3] = 0x0B;
            m_Camera_UART_Buff[4] = 0x02;
            break;

        case Iris_sub:
            realSend = 1;
            m_Camera_UART_Buff[2] = 0x04;
            m_Camera_UART_Buff[3] = 0x0B;
            m_Camera_UART_Buff[4] = 0x03;
            break;

        case IrisSet:
            realSend = 1;
            m_Camera_UART_Send_Count = 9;

            m_Camera_UART_Buff[2] = 0x04;
            m_Camera_UART_Buff[3] = 0x4B;
            m_Camera_UART_Buff[4] = 0x00;
            m_Camera_UART_Buff[5] = 0x00;
            m_Camera_UART_Buff[6] = data & 0xF0;
            m_Camera_UART_Buff[6] = m_Camera_UART_Buff[6] >> 4;
            m_Camera_UART_Buff[6] = m_Camera_UART_Buff[6] & 0x0F;
            m_Camera_UART_Buff[7] = data & 0x0F;
            m_Camera_UART_Buff[8] = 0xFF;
            break;

        case Freeze_on:
            realSend = 1;
            m_Camera_UART_Buff[2] = 0x04;
            m_Camera_UART_Buff[3] = 0x62;
            m_Camera_UART_Buff[4] = 0x02;
            break;

        case Freeze_off:
            realSend = 1;
            m_Camera_UART_Buff[2] = 0x04;
            m_Camera_UART_Buff[3] = 0x62;
            m_Camera_UART_Buff[4] = 0x03;
            break;

        default:
            break;
    }

    if (realSend)
    {
        s_CCD.UartSendData(m_Camera_UART_Buff, m_Camera_UART_Send_Count);
    }
}
static void ProcessInq(void)
{
    if((s_CCD.runstate == stop) || (s_CCD.runstate == locate_zoom) || (s_CCD.runstate == locate_focus))
    {
        s_CCD.stoptimecnt ++;

        if(s_CCD.stoptimecnt >= 25)
        {
            s_CCD.stoptimecnt = 0;
            s_CCD.runstate = null;
        }
    }

    if(s_CCD.runstate != null)
    {
        if(!flag_inq_zoomfocus)
        {
            flag_inq_zoomfocus = 1;
            zoom_inquiry();
        }
        else
        {
            flag_inq_zoomfocus = 0;
            focus_inquiry();
        }
    }
}


