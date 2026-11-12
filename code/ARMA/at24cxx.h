#ifndef __AT24CXX_H_
#define __AT24CXX_H_


#include <stdint.h>

/******************* 测试结果结构体 ********************/
typedef struct {
    uint32_t total_tests;    // 总测试次数
    uint32_t success_count;  // 成功次数
    uint32_t failure_count;  // 失败次数
} AT24_TestResult_t;

AT24_TestResult_t AT24CXX_Robustness_Test(uint32_t iterations);
/*****************************************************/

typedef struct
{
	void (*AT24CXXInit)(void);
	void (*AT24CXX_Read)(unsigned short ReadAddr,unsigned char *pBuffer,unsigned short NumToRead);
	void (*AT24CXX_Write)(unsigned short WriteAddr,unsigned char *pBuffer,unsigned short NumToWrite);
		
}Struct_AT24CXX;
extern Struct_AT24CXX *ps_AT24CXX;


#define SYSTEM_POWERON_FRIST											0xA000

#define UPDATEADDR			0xffff	//升级
#define UARTBADUADDR		0x20	//记录各串口的波特率
#define CAMSELFDATAADDR		0x50	//可见光自检数据  	10B
#define IRSELFDATAADDR		0x60	//热像自检数据  	10B
#define LENSPARAADDR		0x70	//镜头参数 			2B
#define PTPARAADDR			0x80	//转台参数 			6B
#define PATROLADDR			0x90	//巡航参数 			2B
#define KWADDR				0xa0	//守望参数 			5B
#define LASERADDR			0xb0	//激光参数			1B
#define PTALARMADDR			0xc0	//驱动报警			98B
#define POWEROFFSAVEADDR	0x130	//断电记忆			1B
#define SELFLEARNADDR		0x140	//自学习			1000+B
#define CCDPARAADDR			0x600	//红外以及白光机芯参数
#define _3DPRAADDR			0x650	//3D定位参数
#define IRZOOMMAXADDR			0xa00	//升级
#define PRESETADDR			0x3000


#define PRESET_SAVE_START_ADDR											0xb000

#endif
