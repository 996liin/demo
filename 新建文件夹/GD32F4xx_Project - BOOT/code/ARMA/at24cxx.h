#ifndef __AT24CXX_H_
#define __AT24CXX_H_

typedef struct
{
	void (*AT24CXX_Init)(void);
	void (*AT24CXX_Read)(unsigned short ReadAddr,unsigned char *pBuffer,unsigned short NumToRead);
	void (*AT24CXX_Write)(unsigned short WriteAddr,unsigned char *pBuffer,unsigned short NumToWrite);
		
}Struct_AT24CXX;
extern Struct_AT24CXX *ps_AT24CXX;

#define UPDATEADDR			0xFFFF	//升级 				12B
//#define UARTBADUADDR		0x20	//记录各串口的波特率	32B	


#endif
