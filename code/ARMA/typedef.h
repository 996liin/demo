#ifndef TYPEDEF_H_
#define TYPEDEF_H_

typedef char           int8; 
typedef unsigned char  uint8; 
typedef uint8  u8; 
typedef int8  s8; 

typedef short          int16; 
typedef unsigned short uint16; 
typedef uint16  u16; 
typedef int16  s16; 

typedef int            int32; 
typedef unsigned int   uint32; 
typedef uint32  u32; 
typedef int32  s32; 

typedef long long      int64; 
typedef unsigned long long uint64; 

typedef float      f32; 
typedef double     f64; 

#define S_ERROR 1
#define S_OK    0

//将单精度的浮点数据变为32位的整形数据发送出去
//例如在地址0x0存放浮点数据0.125（单精度为3E000000）
//即*(0x0) = 0x3E000000;则0.125的浮点数据转换结果为0x3E000000,如果直接将0.125转换为整形则会
//变成0，如果是0.125扩大1000倍发送出去，在两边需要算除法，精度会有损失
#define f32_to_i32(val) (uint32)(*(uint32 *)(&val))

//将32位的整形数据表示的浮点数据，转换为浮点数据
//例如，接收到的数据为0xBF200000放到地址0x04中去
//&val = 0x04;将0x04地址中的数据0xBF200000转换为浮点数据后取到的结果为-0.625
#define i32_to_f32(val) (f32)(*(f32 *)(&val))

#endif

