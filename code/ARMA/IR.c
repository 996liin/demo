#include "headfile.h"
static void init(void);
static void Camera_ZoomTele(void);
static void Camera_ZoomWide(void);
static void Camera_FocusFar(void);
static void Camera_FocusNear(void);
static void Camera_AutoFocusOn(void);
static void Camera_AutoFocusOff(void);
static void Camera_AutoFocusOnce(void);
static void Camera_PreserCall(unsigned short num);
static void zoom_inquiry(void);
static void focus_inquiry(void);
static void Camera_zoom_Stop(void);
static void Camera_focus_Stop(void);
static void Cmd_Ctrl(unsigned char cmd, unsigned short data);
static void Camera_GoToTarget(unsigned char select, unsigned short value_target);
static void set_colorize(unsigned char select);
static void Elec_zoom(unsigned char select);
static void Shutter_Ctrl(unsigned char select);
static void ProcessInq(void);


static void SetLensSpeed(unsigned char select, unsigned char data);
static uint8_t flag_inq_zoomfocus;
struct_IR s_IR =
{
    .init               = init,
    .zoom_inquiry       = zoom_inquiry,
    .focus_inquiry      = focus_inquiry,
    .Camera_ZoomTele    = Camera_ZoomTele,
    .Camera_ZoomWide    = Camera_ZoomWide,
    .Camera_FocusFar    = Camera_FocusFar,
    .Camera_FocusNear   = Camera_FocusNear,
    .Camera_zoom_Stop   = Camera_zoom_Stop,
    .Camera_focus_Stop  = Camera_focus_Stop,
    .Camera_GoToTarget  = Camera_GoToTarget,
    .Cmd_Ctrl           = Cmd_Ctrl,
    .Camera_AutoFocusOn = Camera_AutoFocusOn,
    .Camera_AutoFocusOff = Camera_AutoFocusOff,
    .Camera_AutoFocusOnce = Camera_AutoFocusOnce,
    .Camera_PreserCall    = Camera_PreserCall,
    .colorize_count       = 0,
    .Elec_zoom            = Elec_zoom,
    .Shutter_Ctrl         = Shutter_Ctrl,
    .set_colorize         = set_colorize,
    .Elec_zoom_value      = 1,
    .runstate             = stop,
    .ProcessInq           = ProcessInq,
    .SetLensSpeed					=	SetLensSpeed,
    .stoptimecnt            = 0,
    .locatetimecnt        = 0,
    .hot_open_flag        = 0,
    .focus_step_run       = 0,
    .zoomvalueOffset      =  0

};
struct_IR *ps_IR  = &s_IR;
static void init(void)
{
    u8 ucIrZoomMin[2];
    s_IR.UartSendData = ps_Usart->Usart5_SendData;//控制热像命令发送给自动聚焦板
    ps_AT24CXX->AT24CXX_Read(IRZOOMMAXADDR, ucIrZoomMin, 2);
    s_IR.ZoomMinAF = ucIrZoomMin[0] << 8 | ucIrZoomMin[1];

    if(s_IR.ZoomMinAF >= 0xffff)
    {
        s_IR.ZoomMinAF = 0x0712;
    }
}

static uint8_t zoom_state = 0;
static uint8_t focus_state = 0;

static void Camera_ZoomTele(void)
{
    zoom_state = 1;
    #ifdef INTERNET
    unsigned char cmd_zoom_tele[7] = {0xFF, 0x01, 0x00, 0x20, 0x00, 0x00, 0x21};
    s_IR.UartSendData(cmd_zoom_tele, 7);
    s_IR.runstate = zoom;
    s_IR.stoptimecnt = 0;
    #else
    unsigned char cmd_zoom_tele[9] = {0xF0, 0x05, 0x36, 0xFD, 0x10, 0x00, 0x00, 0x43, 0xff};
    s_IR.UartSendData(cmd_zoom_tele, 9);
    #endif
//	 unsigned char uc_cmd[8] = {0xf0, 0x04, 0x26, 0x80, 0x37, 0x9d, 0x7a, 0xff};
//	 ps_Usart->Usart5_SendData(uc_cmd,8);//发送给热像机芯控制伪彩
}
static void Camera_ZoomWide(void)
{
    zoom_state = 1;
    #ifdef INTERNET
    unsigned char cmd_zoom_wide[7] = {0xFF, 0x01, 0x00, 0x40, 0x00, 0x00, 0x41};
    s_IR.UartSendData(cmd_zoom_wide, 7);
    s_IR.runstate = zoom;
    s_IR.stoptimecnt = 0;
    #else
    unsigned char cmd_zoom_wide[9] = {0xF0, 0x05, 0x36, 0xFD, 0x10, 0x00, 0x01, 0x44, 0xff};
    s_IR.UartSendData(cmd_zoom_wide, 9);
    #endif
//	unsigned char cmd_zoom_tele[6] = {0xF0, 0x02, 0x26, 0x81, 0xa7, 0xff};
//	 s_IR.UartSendData(cmd_zoom_tele, 6);
}
static void Camera_FocusFar(void)
{
    focus_state = 1;
    #ifdef INTERNET
    unsigned char cmd_focus_far[7] = {0xFF, 0x01, 0x00, 0x80, 0x00, 0x00, 0x81};
    unsigned char cmd_focus_step_far[7] = {0xFF, 0x01, 0x00, 0x07, 0x00, 0x57, 0x5F};

    if(!s_IR.focus_step_run)
    {
        s_IR.UartSendData(cmd_focus_far, 7);
    }
    else
    {
        s_IR.UartSendData(cmd_focus_step_far, 7);
    }

    s_IR.runstate = focus;
    s_IR.stoptimecnt = 0;
    #else
    unsigned char cmd_focus_far[9] = {0xF0, 0x05, 0x36, 0xFD, 0x01, 0x00, 0x01, 0x35, 0xff};
    s_IR.UartSendData(cmd_focus_far, 9);
    #endif
}
static void Camera_FocusNear(void)
{
    focus_state = 1;
    #ifdef INTERNET
    unsigned char cmd_focus_near[7] = {0xFF, 0x01, 0x01, 0x00, 0x00, 0x00, 0x02};
    unsigned char cmd_focus_step_near[7] = {0xFF, 0x01, 0x00, 0x07, 0x00, 0x58, 0x60};

    if(!s_IR.focus_step_run)
    {
        s_IR.UartSendData(cmd_focus_near, 7);
    }
    else
    {
        s_IR.UartSendData(cmd_focus_step_near, 7);
    }

    s_IR.runstate = focus;
    s_IR.stoptimecnt = 0;
    #else
    unsigned char cmd_focus_near[9] = {0xF0, 0x05, 0x36, 0xFD, 0x01, 0x00, 0x00, 0x34, 0xff};
    s_IR.UartSendData(cmd_focus_near, 9);
    #endif
}
static void Camera_AutoFocusOn(void)
{
    #ifdef INTERNET
    unsigned char cmd_autofocus_open[7] = {0xFF, 0x01, 0x00, 0x07, 0x00, 0x42, 0x4a};
    s_IR.UartSendData(cmd_autofocus_open, 7);
    #else
    unsigned char cmd_autofocus_on[9] = {0xF0, 0x05, 0x36, 0xFD, 0x02, 0x00, 0x02, 0x37, 0xff};
    s_IR.UartSendData(cmd_autofocus_on, 9);
    #endif
}

static void SetLensSpeed(unsigned char select, unsigned char data)
{
    #ifdef INTERNET
    unsigned char cmd_buffer[7] = {0xFF, 0x01, 0x64, 0x00, 0x00, 0x00, 0x00};

    switch (select)
    {
        case 1:  //手动聚焦  FF 01 64 17 DH DL CS  (0-100)
            cmd_buffer[3] = 0x17;
            cmd_buffer[5] = data;
            cmd_buffer[6] = (cmd_buffer[1] + cmd_buffer[2] + cmd_buffer[3] + cmd_buffer[4] + cmd_buffer[5]) & 0xFF;
            s_IR.UartSendData(cmd_buffer, 7);
            break;

        case 2:  //变倍   FF 01 64 19 DH DL CS (0-100)
            cmd_buffer[3] = 0x19;
            cmd_buffer[5] = data;
            cmd_buffer[6] = (cmd_buffer[1] + cmd_buffer[2] + cmd_buffer[3] + cmd_buffer[4] + cmd_buffer[5]) & 0xFF;
            s_IR.UartSendData(cmd_buffer, 7);
            break;

        case 3:   //自动聚焦   FF 01 64 1B DH DL CS (0-100)
            cmd_buffer[3] = 0x1B;
            cmd_buffer[5] = data;
            cmd_buffer[6] = (cmd_buffer[1] + cmd_buffer[2] + cmd_buffer[3] + cmd_buffer[4] + cmd_buffer[5]) & 0xFF;
            s_IR.UartSendData(cmd_buffer, 7);
            break;

        default:
            break;
    }

    #else
    #endif
}



static void Camera_AutoFocusOff(void)
{
    #ifdef INTERNET
    unsigned char cmd_autofocus_close[7] = {0xFF, 0x01, 0x00, 0x07, 0x00, 0x41, 0x49};
    s_IR.UartSendData(cmd_autofocus_close, 7);
    #else
    unsigned char cmd_autofocus_off[9] = {0xF0, 0x05, 0x36, 0xFD, 0x02, 0x00, 0x01, 0x36, 0xff};
    s_IR.UartSendData(cmd_autofocus_off, 9);
    #endif

}
static void Camera_AutoFocusOnce(void)
{
    #ifdef INTERNET
    unsigned char cmd_autofocus_once[7] = {0xFF, 0x01, 0x07, 0x00, 0xa2, 0x00, 0xaa};
    s_IR.UartSendData(cmd_autofocus_once, 7);
    #else
    unsigned char cmd_autofocus_once[9] = {0xF0, 0x05, 0x36, 0xFD, 0x03, 0x00, 0x00, 0x36, 0xff};
    s_IR.UartSendData(cmd_autofocus_once, 9);
    #endif
}
static void Camera_PreserCall(unsigned short num)
{
//  unsigned char uc_cmd[7] = {0xFF, 0x02, 0x00, 0x07, 0x00, 0x00 ,0x00};
//	uc_cmd[4] = (num >> 8)&0xff;
//	uc_cmd[5] = num & 0xff;
//	uc_cmd[6] = (uc_cmd[1] + uc_cmd[2] + uc_cmd[3] + uc_cmd[4] + uc_cmd[5])&0xff;
//	s_IR.UartSendData(uc_cmd, 7);
}

static void Camera_PreserDel(unsigned short num)
{
//  unsigned char uc_cmd[7] = {0xFF, 0x02, 0x00, 0x05, 0x00, 0x00 ,0x00};
//	uc_cmd[4] = (num >> 8)&0xff;
//	uc_cmd[5] = num & 0xff;
//	uc_cmd[6] = (uc_cmd[1] + uc_cmd[2] + uc_cmd[3] + uc_cmd[4] + uc_cmd[5])&0xff;
//	s_IR.UartSendData(uc_cmd, 7);
}
static void Camera_zoom_Stop(void)
{
    #ifdef INTERNET
    unsigned char cmd_stop[7] = {0xFF, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01};

    if (zoom_state)
    {
        zoom_state = 0;
        s_IR.UartSendData(cmd_stop, 7);
        //ps_Systick->delay_ms(20);
        //zoom_inquiry();
    }

    if (focus_state)
    {
        focus_state = 0;
        s_IR.UartSendData(cmd_stop, 7);
    }

    s_IR.runstate = stop;
    s_IR.stoptimecnt = 0;
    #else
    unsigned char cmd_zoom_stop[9] = {0xF0, 0x05, 0x36, 0xFD, 0x10, 0x00, 0x02, 0x45, 0xff};
    unsigned char cmd_focus_stop[9] = {0xF0, 0x05, 0x36, 0xFD, 0x01, 0x00, 0x02, 0x36, 0xff};
    s_IR.UartSendData(cmd_focus_stop, 9);
    ps_Systick->delay_ms(20);
    s_IR.UartSendData(cmd_zoom_stop, 9);
//	 ps_Systick->delay_ms(20);
//	 s_IR.zoom_inquiry();
    #endif
}
static void Camera_focus_Stop(void)
{
    #ifdef INTERNET
    unsigned char cmd_stop[7] = {0xFF, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01};

    if (focus_state)
    {
        focus_state = 0;
        s_IR.UartSendData(cmd_stop, 7);
    }

    s_IR.runstate = stop;
    s_IR.stoptimecnt = 0;
    #else
    unsigned char cmd_focus_stop[9] = {0xF0, 0x05, 0x36, 0xFD, 0x01, 0x00, 0x02, 0x36, 0xff};
    s_IR.UartSendData(cmd_focus_stop, 9);
    #endif
}
static void zoom_inquiry(void)
{
    #ifdef INTERNET
    unsigned char cmd_zoom_inquiry[7] = {0xFF, 0x01, 0x00, 0x55, 0x00, 0x00, 0x56};
    s_IR.UartSendData(cmd_zoom_inquiry, 7);
    #else
    unsigned char cmd_zoom_inquiry[8] = {0xF0, 0x04, 0x36, 0xFD, 0x23, 0x01, 0x57, 0xff};
    s_IR.UartSendData(cmd_zoom_inquiry, 8);
    #endif
}
static void focus_inquiry(void)
{
    #ifdef INTERNET
    unsigned char cmd_zoom_inquiry[7] = {0xFF, 0x01, 0x00, 0x85, 0x00, 0x00, 0x86};
    s_IR.UartSendData(cmd_zoom_inquiry, 7);
    #else
    unsigned char cmd_zoom_inquiry[8] = {0xF0, 0x04, 0x36, 0xFD, 0x23, 0x01, 0x57, 0xff};
    s_IR.UartSendData(cmd_zoom_inquiry, 8);
    #endif
}
static void set_colorize(unsigned char select)
{
    unsigned char uc_cmd[7] = {0xf0, 0x03, 0x26, 0x32, 0x00, 0x00, 0xff};
    uc_cmd[4] = select;
    uc_cmd[5] = (uc_cmd[2] + uc_cmd[3] + uc_cmd[4]) & 0xff;
    ps_Usart->Usart1_SendData(uc_cmd, 7); //发送给热像机芯控制伪彩
}
static void Elec_zoom(unsigned char select)
{
    unsigned char uc_cmd[7] = {0xf0, 0x03, 0x26, 0x08, 0x00, 0x00, 0xff};

    switch (select)
    {
        case 0x01:
            uc_cmd[4] = 0x00;
            break;

        case 0x02:
            uc_cmd[4] = 0x0f;
            break;

        case 0x04:
            uc_cmd[4] = 0x02;
            break;

        case 0x08:
            uc_cmd[4] = 0x03;
            break;

        default:
            break;
    }

    uc_cmd[5] = (uc_cmd[2] + uc_cmd[3] + uc_cmd[4]) & 0xff;
    ps_Usart->Usart1_SendData(uc_cmd, 7); //发送给热像机芯控制数字变倍
}
static void Shutter_Ctrl(unsigned char select)
{
    unsigned char uc_cmd_open[7] = {0xF0, 0x03, 0x26, 0x07, 0x0F, 0x3C, 0xFF};
    unsigned char uc_cmd_close[7] = {0xF0, 0x03, 0x26, 0x07, 0x00, 0x2d, 0xFF};
    unsigned char uc_cmd1[9] = {0xF0, 0x05, 0x26, 0x1E, 0x0E, 0xdc, 0x05, 0x33, 0xff};

    switch (select)
    {
        case 0x01:
            ps_Usart->Usart1_SendData(uc_cmd_open, 7); //发送给热像机芯打开/关闭自动校正
            ps_Systick->delay_ms(100);
            ps_Usart->Usart1_SendData(uc_cmd1, 9);
            break;

        case 0x00:
            ps_Usart->Usart1_SendData(uc_cmd_close, 7); //发送给热像机芯打开/关闭自动校正
            break;

        default:
            break;
    }

}
static void Camera_GoToTarget(unsigned char select, unsigned short value_target)
{
//   unsigned char uc_cmd[10] = {0xf0, 0x06, 0x36, 0xFD, 0x00 ,0x00, 0x00, 0x00,0x00,0xff};
//	if(select == 0)
//	{
//		uc_cmd[4] = 0x23;
//	}
//	else if(select == 1)
//	{
//		uc_cmd[4] = 0x24;
//	}
//	uc_cmd[6] = (value_target >> 8) & 0xFF;
//	uc_cmd[7] =  value_target & 0xFF;
//	uc_cmd[8] = (uc_cmd[2] + uc_cmd[3] + uc_cmd[4] + uc_cmd[5] + uc_cmd[6] + uc_cmd[7]) & 0xFF;
//	s_IR.UartSendData(uc_cmd, 10);
    unsigned char uc_cmd[7] = {0xFF, 0x01, 0x00, 0x4f, 0x00, 0x00, 0x00};

    if(select == 0)
    {
        s_IR.runstate = locate_zoom;
        //s_IR.zoomvalueOffset = value_target;
        value_target += s_IR.ZoomMinAF - 10;//回传防止越界做了+10处理，此处需-10
        uc_cmd[3] = 0x4f;
    }
    else if(select == 1)
    {
        s_IR.runstate = locate_focus;
        uc_cmd[3] = 0x7f;

        //新镜头与老镜头之间有系数关系
        value_target = value_target * NEW_LENS_FOCUS_RADIO;
    }

    uc_cmd[4] = value_target >> 8;
    uc_cmd[5] = value_target & 0xff;
    uc_cmd[6] = (uc_cmd[1] + uc_cmd[2] + uc_cmd[3] + uc_cmd[4] + uc_cmd[5]) & 0xff;
    s_IR.UartSendData(uc_cmd, 7);
    s_IR.stoptimecnt = 0;
}
static void Cmd_Ctrl(unsigned char cmd, unsigned short data)
{
    switch (cmd)
    {
        case 0x01:
        {
            s_IR.Camera_ZoomTele();
            break;
        }

        case 0x02:
        {
            s_IR.Camera_ZoomWide();
            break;
        }

        case 0x03:
        {
            s_IR.Camera_zoom_Stop();
            break;
        }

        case 0x04:
        {
            s_IR.Camera_FocusFar();
            break;
        }

        case 0x05:
        {
            s_IR.Camera_FocusNear();
            break;
        }

        case 0x06:
        {
            s_IR.Camera_focus_Stop();
            break;
        }

        case 0x07:
        {
            s_IR.Camera_PreserCall(data);
            break;
        }

        case 0x10:
        {
            s_IR.Camera_GoToTarget(0, data);
            break;
        }

        case 0x50:
        {
            s_IR.Camera_AutoFocusOn();
            break;
        }

        case 0x51:
        {
            s_IR.Camera_AutoFocusOff();
            break;
        }

        case 0x52:
        {
            break;
        }

        default:
            break;
    }
}
static void ProcessInq(void)
{
    if((s_IR.runstate == stop) || (s_IR.runstate == locate_zoom) || (s_IR.runstate == locate_focus))
    {
        s_IR.stoptimecnt ++;

        if(s_IR.stoptimecnt >= 25)
        {
            s_IR.stoptimecnt = 0;
            s_IR.runstate = null;
        }
    }

    if(s_IR.runstate != null)
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
