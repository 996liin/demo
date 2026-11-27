#ifndef _FLASH_H_
#define _FLASH_H_

////扇区的大小
//#define SECTOR_SIZE_0to3		0x4000
//#define SECTOR_SIZE_4			0x10000
//#define SECTOR_SIZE_5to11		0x20000

////FLASH起始地址
//#define FLASH_BASE_ADDR		0x08000000

////用户程序的起始片区
//#define USER_BASE_SECTOR	FLASH_Sector_5
////用于存储用户程序的片区数
//#define USER_SECTOR_NUM		3
////用户程序的起始地址，需要根据起始片区计算
//#define USER_BASE_ADDR		(FLASH_BASE_ADDR + SECTOR_SIZE_0to3 * 4 + SECTOR_SIZE_4)

////缓存区的起始片区
//#define BUFFER_BASE_SECTOR	FLASH_Sector_8
////用于缓存区的片区数
//#define BUFFER_SECTOR_NUM	3
////缓存区的起始地址，需要根据起始片区计算
//#define BUFFER_BASE_ADDR	(FLASH_BASE_ADDR + SECTOR_SIZE_0to3 * 4 + SECTOR_SIZE_4 + SECTOR_SIZE_5to11 * 3)

////FLASH起始地址
#define FLASHBASEADDR		0x08000000
//用户应用程序存放的地址
#define USERBASEADDR		0x08008000
//缓冲区程序存放地址
#define BUFFERBASEADDR		0x08040000
	//大容量FLASH一页2K字节
#define FLASHPAGESIZE		0x800
	//大容量FLASH共有256页
#define FLASGPAGENUM		0x100

//用户存放应用程序的起始页
#define USERPAGESTART		(USERBASEADDR - FLASHBASEADDR)/FLASHPAGESIZE
//用户存放应用程序页数
#define USERPAGENUM			(BUFFERBASEADDR - USERBASEADDR)/FLASHPAGESIZE
#define BUFFERPAGESTART		 FLASGPAGENUM/2
#define BUFFERPAGENUM		 FLASGPAGENUM/2

typedef struct _STRUCT_FLASH_
{
	void (*FlashTimer)(void);
	void (*FlashRead)(unsigned int uiReadAddr, unsigned char *pReadData,unsigned short usReadSize);
	unsigned char (*FlashErase)(unsigned short usPageStart,unsigned short usPageNum);
	unsigned char (*FlashWrite)(unsigned int uiWriteAddr, unsigned char *pWriteData,unsigned short usWriteSize);
}Struct_Flash;
extern Struct_Flash *ps_Flash;
#endif

