#include "headfile.h"

extern unsigned char ucBoardType;

typedef void (*pFunction)(void);

unsigned short usWaitTime = 0, usWaitTimeCnt = 0;
unsigned int uiAppSize = 0, uiCalSum = 0,uiRecvSum = 0;
unsigned char ucBuffer[UPDATE_PACK_BUFFER_SIZE] = {0,};

unsigned char ucHandshakeCmd[7] = {0xFF,0x01,0x6E,0x05,0x00,0x00,0x74};	//握手成功
unsigned char ucTypeChkCmd[7] = {0xFF,0x01,0x6E,0x09,0x00,0x00,0x78};	//类型校验完成，正确or错误

unsigned char ucDataRecvSucc[7] = {0xFF,0x01,0x6E,0x0B,0x00,0x01,0x7B};	//数据帧接收完成
unsigned char ucDataChkErr[7] = {0xFF,0x01,0x6E,0x0B,0x00,0x00,0x7A};	//数据帧校验错误

unsigned char ucUpdateSucc[7] = {0xFF,0x01,0x6E,0x0D,0x00,0x01,0x7D};	//升级成功
unsigned char ucUpdateFai[7] = {0xFF,0x01,0x6E,0x0D,0x00,0x00,0x7C};	//升级失败

Struct_UpdatePack s_UpdatePack = {0,};
struct_devicePara s_devicePara;

UARTQUEUE *pUpdateQueue;
static uint32_t UpdateUART = UART4;

static void IAPTimer(void);
static void IAPProcess(void);

static Struct_IAP s_IAP = 
{
	.IAPTimer = IAPTimer,
	.IAPProcess = IAPProcess,
};
Struct_IAP *ps_IAP = &s_IAP;

static unsigned char GetDevicePara(void)
{
	ps_Systick->delay_ms(30);

    s_devicePara.ucUpdateFlag = IAPFlag_Read();
    s_devicePara.ucType = ucBoardType;

    if (s_devicePara.ucUpdateFlag == IAP_FLAG_REQUEST)
    {
        return 1;
    }

    return 0;

}
static void SendCmdToCamera(uint32_t UART,unsigned char *ucCmd)
{
	ps_Systick->delay_ms(30);
	if(UART == USART0)
	{
		ps_Usart->dubugUsart0SendData(ucCmd,7);
	}
	if(UART == USART1)
	{
		ps_Usart->cam232Usart1SendData(ucCmd,7);
	}
	else if(UART == USART2)
	{
		ps_Usart->wb422Usart2SendData(ucCmd,7);
	}
	else if(UART == UART3)
	{
		ps_Usart->af232Usart3SendData(ucCmd,7);
	}
	else if(UART == UART4)
	{
		ps_Usart->ir232Uart4SendData(ucCmd,7);
	}
	else if(UART == USART5)
	{
		ps_Usart->cam485Uart5SendData(ucCmd,7);
	}
	else if(UART == UART6)
	{
		ps_Usart->Uart6_SendData(ucCmd,7);
	}
	
}
static void SetWaitTime(void)
{
	usWaitTime = 30;
	usWaitTimeCnt = 0;
}
static unsigned char GetOutTime(void)
{
	if(usWaitTimeCnt >= usWaitTime * 1000)
	{
		return 1;
	}
	
	return 0;
}
static void IAPTimer(void)
{
	usWaitTimeCnt++;
}
static unsigned char HandshakeChk(UARTQUEUE *pQueue)
{
	if(pQueue->usHead == pQueue->usTail)
	{
		return 0;
	}
	
	if(0xff == pQueue->ucData[pQueue->usHead])
	{
		if(ps_queue->returnUartQueueLength(*pQueue) >= 7)
		{
			unsigned char ucSum = 0;
			for(unsigned char uci = 1; uci <= 5; uci++)
			{
				ucSum += pQueue->ucData[ps_queue->uartQueueIndex(pQueue->usHead+uci)];
			}
			if(ucSum == pQueue->ucData[ps_queue->uartQueueIndex(pQueue->usHead+6)])
			{
				unsigned short usCmd = pQueue->ucData[ps_queue->uartQueueIndex(pQueue->usHead+2)] << 8 | pQueue->ucData[ps_queue->uartQueueIndex(pQueue->usHead+3)];
				
				pQueue->usHead = ps_queue->uartQueueIndex(pQueue->usHead+7);
				
				if(0x6e03 == usCmd)
				{
					return 1;
				}
			}
			else
			{
				pQueue->usHead = ps_queue->uartQueueIndex(pQueue->usHead+1);
			}
		}
	}
	else
	{
		pQueue->usHead = ps_queue->uartQueueIndex(pQueue->usHead+1);
	}
	
	return 0;
}
static unsigned char WaitHandshake(UARTQUEUE **pQueue,uint32_t *UART)
{
	SetWaitTime();
	
	while(1)
	{
		 
		if(1 == GetOutTime())
		{
			break;
		}
		//轮询判断是哪个串口升级
		if(HandshakeChk(*ps_queue->pUartQueue)) 
		{
			*UART = USART0;
			*pQueue = *ps_queue->pUartQueue;
			return 1;
		}
		else if(HandshakeChk(*ps_queue->pUartQueue+1)) 
		{
			*UART = USART1;
			*pQueue = *ps_queue->pUartQueue+1;
			return 1;
		}
		else if(HandshakeChk(*ps_queue->pUartQueue+2)) 
		{
			*UART = USART2;
			*pQueue = *ps_queue->pUartQueue+2;
			return 1;
		}
		else if(HandshakeChk(*ps_queue->pUartQueue+3)) 
		{
			*UART = UART3;
			*pQueue = *ps_queue->pUartQueue+3;
			return 1;
		}
		else if(HandshakeChk(*ps_queue->pUartQueue+4)) 
		{
			*UART = UART4;
			*pQueue = *ps_queue->pUartQueue+4;
			return 1;
		}
		else if(HandshakeChk(*ps_queue->pUartQueue+5)) 
		{
			*UART = USART5;
			*pQueue = *ps_queue->pUartQueue+5;
			return 1;
		}
		else if(HandshakeChk(*ps_queue->pUartQueue+6)) 
		{
			*UART = UART6;
			*pQueue = *ps_queue->pUartQueue+6;
			return 1;
		}
	}
	
	return 0;
}
//类型校验正确返回0xff 校验错误返回其他非零值 超时返回0
static unsigned char chkType(UARTQUEUE *pQueue)
{	
	SetWaitTime();
	
	while(1)
	{
		if(1 == GetOutTime())
		{
			break;
		}
		
		if(pQueue->usHead == pQueue->usTail)
		{
			continue;
		}
		
		if(0xff == pQueue->ucData[pQueue->usHead])
		{
			if(ps_queue->returnUartQueueLength(*pQueue) >= 29)
			{
				unsigned char ucSum = 0;
				
				for(unsigned char uci = 1; uci <= 27; uci++)
				{
					ucSum += pQueue->ucData[ps_queue->uartQueueIndex(pQueue->usHead+uci)];
				}
				
				if(ucSum == pQueue->ucData[ps_queue->uartQueueIndex(pQueue->usHead+28)])
				{
					unsigned short usCmd = pQueue->ucData[ps_queue->uartQueueIndex(pQueue->usHead+2)] << 8 | pQueue->ucData[ps_queue->uartQueueIndex(pQueue->usHead+3)];
					
					if(0x6e07 == usCmd)
					{
						s_devicePara.ucType = pQueue->ucData[ps_queue->uartQueueIndex(pQueue->usHead+7)];
						
						s_devicePara.uiDate = pQueue->ucData[ps_queue->uartQueueIndex(pQueue->usHead+8)];
						s_devicePara.uiDate = s_devicePara.uiDate << 8 | pQueue->ucData[ps_queue->uartQueueIndex(pQueue->usHead+9)];
						s_devicePara.uiDate = s_devicePara.uiDate << 8 | pQueue->ucData[ps_queue->uartQueueIndex(pQueue->usHead+10)];
						s_devicePara.uiDate = s_devicePara.uiDate << 8 | pQueue->ucData[ps_queue->uartQueueIndex(pQueue->usHead+11)];
						
						s_devicePara.ucVersion[0] = pQueue->ucData[ps_queue->uartQueueIndex(pQueue->usHead+12)];
						s_devicePara.ucVersion[1] = pQueue->ucData[ps_queue->uartQueueIndex(pQueue->usHead+13)];
						s_devicePara.ucVersion[2] = pQueue->ucData[ps_queue->uartQueueIndex(pQueue->usHead+14)];
						s_devicePara.ucVersion[3] = pQueue->ucData[ps_queue->uartQueueIndex(pQueue->usHead+15)];
						
						pQueue->usHead = ps_queue->uartQueueIndex(pQueue->usHead+29);
							
						if(ucBoardType == s_devicePara.ucType)	//校验成功
						{
							return 0xff;
						}
						else		//校验错误
						{
							return s_devicePara.ucType;
						}
					}
					else
					{
						pQueue->usHead = ps_queue->uartQueueIndex(pQueue->usHead+1);
					}
				}
				else
				{
					pQueue->usHead = ps_queue->uartQueueIndex(pQueue->usHead+1);
				}
			}
		}
		else
		{
			pQueue->usHead = ps_queue->uartQueueIndex(pQueue->usHead+1);
		}
	}
	
	return 0;
}
static unsigned char EraseFlashBuffer(void)
{
	return ps_Flash->FlashErase(BUFFERPAGESTART,BUFFERPAGENUM);
}
static unsigned char EraseFlashUser(void)
{
	return ps_Flash->FlashErase(USERPAGESTART,USERPAGENUM);
}
static void SystemDeinit(void)
{
	ps_Systick->delay_ms(100);
	
	timer_deinit(TIMER2);
	usart_deinit(USART1);
	usart_deinit(USART2);
	usart_deinit(UART3);
	usart_deinit(UART4);
	usart_deinit(USART5);
	ps_Systick->delay_ms(100);
}

static void RunUserApp(void)
{
	if( ( ( *(unsigned int *)USERBASEADDR) & 0x2FFE0000 ) == 0x20000000)
	{
		unsigned int uiJumpAppAddr = *(__IO unsigned int*) (USERBASEADDR + 4);
		
		pFunction JumpAppAddr = (pFunction)uiJumpAppAddr;
		
		__set_MSP(*(__IO unsigned int*) USERBASEADDR );
		
		JumpAppAddr();
	}
	
	NVIC_SystemReset();
}

//接收到正确的数据返回1 校验错误返回2 超时返回0
static unsigned char RecvUpdatePack(UARTQUEUE *pQueue,Struct_UpdatePack *ps_UpdatePack)
{
	SetWaitTime();
	
	while(1)
	{
		if(1 == GetOutTime())
		{
			break;
		}
		if(pQueue->usHead == pQueue->usTail)
		{
			continue;
		}
		
		if('s' == pQueue->ucData[pQueue->usHead])
		{
			if(ps_queue->returnUartQueueLength(*pQueue) >= 6)
			{
				if(	0x08 == pQueue->ucData[ps_queue->uartQueueIndex(pQueue->usHead+2)] && \
					0x00 == pQueue->ucData[ps_queue->uartQueueIndex(pQueue->usHead+3)] && \
					0x80 == pQueue->ucData[ps_queue->uartQueueIndex(pQueue->usHead+4)] && \
					0x00 == pQueue->ucData[ps_queue->uartQueueIndex(pQueue->usHead+5)]	  
				   )
				{
					if(ps_queue->returnUartQueueLength(*pQueue) >= UPDATE_PACK_DATA_SIZE)
					{
						uiCalSum = 0;
						for(unsigned short usCnt = 1; usCnt <= UPDATE_PACK_DATA_SIZE - 4; usCnt++)
						{
							uiCalSum += pQueue->ucData[ps_queue->uartQueueIndex(pQueue->usHead+usCnt)];
						}
						uiRecvSum = 0;
						uiRecvSum = ((unsigned int)pQueue->ucData[ps_queue->uartQueueIndex(pQueue->usHead + (UPDATE_PACK_DATA_SIZE - 3))] << 16) 
									| ((unsigned int)pQueue->ucData[ps_queue->uartQueueIndex(pQueue->usHead + (UPDATE_PACK_DATA_SIZE - 2))] << 8) 
									| ((unsigned int)pQueue->ucData[ps_queue->uartQueueIndex(pQueue->usHead + (UPDATE_PACK_DATA_SIZE - 1))]);
						
 						if(uiRecvSum == uiCalSum)
						{
							for(unsigned short usCnt = 0; usCnt < UPDATE_PACK_DATA_SIZE; usCnt++)
							{
								*((unsigned char *)ps_UpdatePack + usCnt) = pQueue->ucData[ps_queue->uartQueueIndex(pQueue->usHead+usCnt)];
							}
							
							pQueue->usHead = ps_queue->uartQueueIndex(pQueue->usHead+UPDATE_PACK_DATA_SIZE);
							
							return 1;
						}
						else
						{
							pQueue->usHead = ps_queue->uartQueueIndex(pQueue->usHead+1);
							
							return 2;
						}
					}
				}
				else
				{
					pQueue->usHead = ps_queue->uartQueueIndex(pQueue->usHead+1);
				}
			}
		}
		else
		{
			pQueue->usHead = ps_queue->uartQueueIndex(pQueue->usHead+1);
		}
	}
	
	return 0;
}

static unsigned char WriteDataToBufferAddr(unsigned int uiAddr,Struct_UpdatePack s_UpdatePack)
{
	for(unsigned char ucCnt = 0; ucCnt < 3; ucCnt++)
	{
		if(1 == ps_Flash->FlashWrite(uiAddr,s_UpdatePack.ucBuffer,UPDATE_PACK_BUFFER_SIZE))
		{
			uiAppSize += UPDATE_PACK_BUFFER_SIZE;
			return 1;
		}
	}
	
	return 0;
}
static unsigned char WriteDataToUserAddr(unsigned int uiBufferAddr, unsigned int uiUserAddr,unsigned int uiSize)
{
	unsigned char ucWriteStatus = 0;
	
	for(unsigned short usCnt = 0; usCnt < uiSize / UPDATE_PACK_BUFFER_SIZE; usCnt++)
	{
		ps_Flash->FlashRead(uiBufferAddr,ucBuffer,UPDATE_PACK_BUFFER_SIZE);
		
		ucWriteStatus = 0;
		
		for(unsigned short usCnt = 0; usCnt < 3; usCnt++)
		{
			if(1 == ps_Flash->FlashWrite(uiUserAddr,ucBuffer,UPDATE_PACK_BUFFER_SIZE))
			{
				ucWriteStatus = 1;
				break;
			}
		}
		if(0 == ucWriteStatus)
		{
			return 0;
		}
		
		uiBufferAddr += UPDATE_PACK_BUFFER_SIZE;
		uiUserAddr += UPDATE_PACK_BUFFER_SIZE;
	}
	return 1;
}
int test[10] = {0,};
static void IAPProcess(void)
{
	if(1 == GetDevicePara())
	{
		if(1 == WaitHandshake(&pUpdateQueue,&UpdateUART))	
		{			
			SendCmdToCamera(UpdateUART,ucHandshakeCmd);
			
			unsigned char ucData = chkType(pUpdateQueue);
			
			if(0xff == ucData)
			{
				uiAppSize = 0;
				if(1 == EraseFlashBuffer())
				{
					SendCmdToCamera(UpdateUART,ucTypeChkCmd);
					
					unsigned char ucRecvUpdatePackStatus = 0;
					unsigned int uiBufferAddr = BUFFERBASEADDR;
					unsigned int ucCmd[10] = {0,};
					test[0] = (int)&ucRecvUpdatePackStatus;
					test[1] = (int)&uiBufferAddr;
					test[2] = (int)ucCmd;
					while(1)
					{
						ucRecvUpdatePackStatus = RecvUpdatePack(pUpdateQueue,&s_UpdatePack);
						
						if(1 == ucRecvUpdatePackStatus)
						{						
							if(1 == WriteDataToBufferAddr(uiBufferAddr,s_UpdatePack))
							{
								SendCmdToCamera(UpdateUART,ucDataRecvSucc);
								
								if(1 == s_UpdatePack.ucType)
								{
									if(0 == EraseFlashUser())
									{
										ucUpdateFai[5] = 1;
										ucUpdateFai[6] += 1;
										SendCmdToCamera(UpdateUART,ucUpdateFai);
										break;
									}
									if(1 == WriteDataToUserAddr(BUFFERBASEADDR, USERBASEADDR, uiAppSize))
									{
										/*
										* APP 已经写入成功。
										* 清除 EEPROM 升级标志，避免下次上电重复进入升级模式。
										*/
										if (IAPFlag_Clear())
										{
											SendCmdToCamera(UpdateUART, ucUpdateSucc);
										}
										else
										{
											/*
											* EEPROM 标志清除失败。
											* 不向上位机报告升级完全成功。
											*/
											SendCmdToCamera(UpdateUART, ucUpdateFai);
										}
										break;
									}
									else
									{
										ucUpdateFai[5] = 2;
										ucUpdateFai[6] += 2;
										SendCmdToCamera(UpdateUART,ucUpdateFai);
										break;
									}
								}
							}
							else
							{
								ucUpdateFai[5] = 3;
								ucUpdateFai[6] += 3;
								SendCmdToCamera(UpdateUART,ucUpdateFai);
								break;
							}
						}
						else if(2 == ucRecvUpdatePackStatus)
						{
							SendCmdToCamera(UpdateUART,ucDataChkErr);
							continue;
						}
						else	
						{
							ucUpdateFai[5] = 4;
							ucUpdateFai[6] += 4;
							SendCmdToCamera(UpdateUART,ucUpdateFai);
							break;
						}
						
						uiBufferAddr += UPDATE_PACK_BUFFER_SIZE;
					}
				}
				else
				{
					ucUpdateFai[5] = 5;
					ucUpdateFai[6] += 5;
					SendCmdToCamera(UpdateUART,ucUpdateFai);
				}
			}
			else if(ucData)
			{
				ucTypeChkCmd[5] = ucData;
				ucTypeChkCmd[6] += ucData;
				SendCmdToCamera(UpdateUART,ucTypeChkCmd);
			}
			else
			{
				ucUpdateFai[5] = 6;
				ucUpdateFai[6] += 6;
				SendCmdToCamera(UpdateUART,ucUpdateFai);
			}
		}
		else
		{
			ucUpdateFai[5] = 7;
			ucUpdateFai[6] += 7;
			SendCmdToCamera(UpdateUART,ucUpdateFai);
		}
	}
	SystemDeinit();
	RunUserApp();
}

