#include "headfile.h"



static unsigned char ucFlashOutTimeFlag = 0;
static unsigned short usFlashOutTimeCnt = 0;

static void FlashTimer(void);
static void FlashRead(unsigned int uiReadAddr, unsigned char *pReadData,unsigned short usReadSize);
static unsigned char FlashErase(unsigned short usPageStart,unsigned short usPageNum);
static unsigned char FlashWrite(unsigned int uiWriteAddr, unsigned char *pWriteData,unsigned short usWriteSize);

static Struct_Flash s_Flash = 
{
	.FlashTimer = FlashTimer,
	.FlashRead = FlashRead,
	.FlashErase = FlashErase,
	.FlashWrite = FlashWrite,
};
Struct_Flash *ps_Flash = &s_Flash;

static void FlashTimer(void)
{
	if(ucFlashOutTimeFlag)
	{
		usFlashOutTimeCnt ++;
	}
}

static void FlashRead(unsigned int uiReadAddr, unsigned char *pReadData,unsigned short usReadSize)
{
	for(unsigned short usCnt = 0; usCnt < usReadSize; usCnt++)
	{
		*(pReadData + usCnt) = *(unsigned char *)(uiReadAddr + usCnt);
	}
}

static unsigned char FlashErase(unsigned short usPageStart,unsigned short usPageNum)
{
	fmc_unlock();
	
	ucFlashOutTimeFlag = 1;
	usFlashOutTimeCnt = 0;
	
	for(unsigned short usCnt = 0; usCnt < usPageNum; usCnt++)
	{
		while(FMC_READY != fmc_page_erase( (usPageStart + usCnt) * FLASHPAGESIZE + FLASHBASEADDR ))
		{
			if(usFlashOutTimeCnt >= 10)
			{
				ucFlashOutTimeFlag = 0;
				usFlashOutTimeCnt = 0;
				fmc_lock();
				return 0;
			}
		}
	}
	
	ucFlashOutTimeFlag = 0;
	usFlashOutTimeCnt = 0;
	
	fmc_lock();
	
	return 1;
}
static unsigned char FlashWrite(unsigned int uiWriteAddr, unsigned char *pWriteData,unsigned short usWriteSize)
{
	fmc_unlock();
	
	unsigned short usData = 0;
	unsigned int uiNowWriteAddr = uiWriteAddr;
	
	for(unsigned short usCnt = 0; usCnt < usWriteSize; usCnt += 2)
	{
		usData = *(pWriteData + usCnt) << 8 | *(pWriteData + usCnt + 1);
		
		ucFlashOutTimeFlag = 1;
		usFlashOutTimeCnt = 0;
		
		while(FMC_READY != fmc_halfword_program(uiNowWriteAddr ,usData))
		{
			if(usFlashOutTimeCnt >= 10)
			{
				ucFlashOutTimeFlag = 0;
				usFlashOutTimeCnt = 0;
				fmc_lock();
				return 0;
			}
		}
		
		ucFlashOutTimeFlag = 0;
		usFlashOutTimeCnt = 0;
		
		uiNowWriteAddr += 2;
	}
	
	fmc_lock();
	
	return 1;
}



