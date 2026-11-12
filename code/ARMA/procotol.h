#ifndef   __PROCOTOL_H_
#define  __PROCOTOL_H_

typedef enum
{
    msgPrivateBoardLink,

    msgPrivateCtrlRemotor,

    msgPrivateAdcValueRtn,
    msgPrivateBoardInfoQuary,

    msgPrivateLaserParamSave,

    msgPrivateLaserCali,
    msgPrivateLaserCaliParamSave,

    msgPrivateLaserCaliCurSet,
    msgPrivateLaserCaliInfoQuery,
    msgPrivateLaserCaliInfoClear,
    msgPrivateLaserCaliStart,
    msgPrivateLaserCaliStop,
    msgPrivateLaserCaliPointNumRtn,

    msgPrivateLaserOpenTempHigh,

    msgPrivateNull,
} PrivateMessage;
//Pelco
typedef enum
{
    //PTZ
    msgPelcoTele               = 0x0020,
    msgPelcoWide               = 0x0040,
    msgPelcoFar                = 0x0080,
    msgPelcoNear               = 0x0100,
    msgPelcoOpen               = 0x0200,
    msgPelcoClose              = 0x0400,
    msgPelcoUp                 = 0x0008,
    msgPelcoDown               = 0x0010,
    msgPelcoLeft               = 0x0004,
    msgPelcoRight              = 0x0002,

    msgPelcoLeftUp             = 0x000C,
    msgPelcoLeftDown           = 0x0014,
    msgPelcoRightUp            = 0x000A,
    msgPelcoRightDown          = 0x0012,

    msgPelcoStop               = 0x0000,

    //扩展功能
    msgPelcoSetPreset          = 0x0003,
    msgPelcoGotoPreset         = 0x0007,
    msgPelcoClearPreset        = 0x0005,
    msgPelcoOpenAux            = 0x0009,
    msgPelcoCloseAux           = 0x000B,
    msgPelcoSetPanPositin      = 0x004B,
    msgPelcoSetTiltPosion      = 0x004D,
    msgPelcoSetZoomPosion      = 0x004F,
    msgPelcoSetFocusPosion     = 0x014F,
    msgPelcoQueryPanPositin    = 0x0051,
    msgPelcoQueryTiltPosion    = 0x0053,
    msgPelcoQueryZoomPosion    = 0x0055,
    msgPelcoQueryFocusPosion   = 0x0057,
    msgPelcoPanPositinResponse = 0x0059,
    msgPelcoTiltPosionResponse = 0x005B,
    msgPelcoZoomPosionResponse = 0x005D,

    //曝光控制  pelcod cmd
    msgPelcoSetExpressMode      = 0x1051,
    msgPelcoSetShutterDef       = 0x1061,
    msgPelcoSetShutterPlus      = 0x1063,
    msgPelcoSetShutterSub       = 0x1065,
    msgPelcoSetShutterValue     = 0x1067,
    msgPelcoSetIrisDef       = 0x1071,
    msgPelcoSetIrisPlus      = 0x1073,
    msgPelcoSetIrisSub       = 0x1075,
    msgPelcoSetIrisValue     = 0x1077,

    msgPelcoQueryZoomAf      = 0x1097,//查询自动聚焦板实际的zoom值
    //热像zoom值不统一，每台设备查询完最小值写入eeprom
    msgPelcoSaveIrZoomMin    = 0x1099,

    msgPelcoQueryBoardVer    = 0x1201,
		
		msgPelcoSetIrManFocusSpeed   = 0x6417,
		msgPelcoSetIrZoomSpeed   = 0x6419,
		msgPelcoSetIrAutoFocusSpeed   = 0x641B,

    msgPelcoNull
} PelcoMessage;
typedef enum
{
    msgUpdateLink			=	0xC010,
    msgUpdateStart			=	0xC020,

    msgUpdateReboot			=	0xCCCC,

    msgUpdateShowStr		=	0xCD00,
    msgUpdateNull			=	0xFFFF,
} updateMessage;

typedef enum
{
    Zoom_Tele,
    Zoom_Wide,
    Zoom_Stop,
    DZoom_On,
    DZoom_Off,
    Focus_Auto,
    Focus_Trigger,
    Focus_Manual,
    Focus_Far,
    Focus_Near,
    Focus_Stop,
    QuryZoomPos,
    SetZoomPos,
    QuryFocusPos,
    SetFocusPos,
    Icr_Auto,
    Icr_On,
    Icr_Off,
    Dnr,
    WD_On,
    WD_Off,
    SmartIR_On,
    SmartIR_Off,
    Menu_On,
    Menu_Off,
    Menu_Enter,
    Menu_Up,
    Menu_Down,
    Menu_Left,
    Menu_Right,
    Menu_ZoomInfo_On,
    Menu_ZoomInfo_Off,
    Output_30FPS,
    Output_25FPS,
    Stab_On,
    Stab_Off,
    Mirror_Image_On,
    Mirror_Image_Off,
    SlowShutterSw,
    GainSet,
    Ae,
    Shutter_default,
    Shutter_plus,
    Shutter_sub,
    Shutter_Set,
    Iris_default,
    Iris_plus,
    Iris_sub,
    IrisSet,
    Freeze_on,
    Freeze_off,

} Cam_Opt_Type;
//GPS模块命令
typedef enum
{
    msgGGA = 0x0000,
    msgGSA,
    msgRMC,
    msgGSV,
    msgGLL,
    msgVTG,

    msgGnssNull			,
} GnssMessage;
extern void ProtAnal(void);
extern void KeyBoardCmd(void);
extern unsigned fsmctrl;
#endif

