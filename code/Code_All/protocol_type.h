#ifndef PROTOCOL_TYPE_H_
#define PROTOCOL_TYPE_H_

#include "typedef.h"
#define GNSS_PROTOCOL_HEAD_LEN (6)
#define GNSS_PROTOCOL_END_LEN  (5)
//支持的协议列表
typedef enum
{
		proto_internal = 0x00,
    protoPELCO_D 		= 0x01,
    protoPELCO_P,
    proto_private,
    proto_GNSS,
    protoNull
} Support_ProtocolList;


typedef struct
{
    uint8  u8FrameStart[2]; /*帧头*/
    uint8  u8Address;       /*设备地址*/
    uint8  u8Reserve;       /*保留数据*/
	uint16 check;			 /*校验数据*/
    uint8  u8FrameEnd;      /*帧尾*/
} tProtocol;

typedef struct
{
    u8  u8FrameStart[GNSS_PROTOCOL_HEAD_LEN];    /*帧头*/
    u8  u8Reserve;         						 /*保留数据*/
    u8  u8FrameEnd[GNSS_PROTOCOL_END_LEN];       /*帧尾*/
} tGnssProtocol;
#endif

