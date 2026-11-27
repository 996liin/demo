#include "headfile.h"
#include <stdlib.h>

UARTQUEUE usrtQueueBuf[8] = {0,};

static void uartQueuePush(unsigned char ucQueueNum,unsigned char ucData);
static unsigned short uartQueueIndex(unsigned short usIndex);
static unsigned short returnUartQueueLength(UARTQUEUE s_queue);

static Struct_Queue s_queue = 
{
	.uartQueueIndex = uartQueueIndex,
	.returnUartQueueLength = returnUartQueueLength,
	.uartQueuePush = uartQueuePush,
	
	.pUartQueue = usrtQueueBuf,
};
Struct_Queue *ps_queue = &s_queue;

static unsigned short uartQueueIndex(unsigned short usIndex)
{
	if(usIndex >= UARQUEUESIZE)
	{
		usIndex -= UARQUEUESIZE;
	}
	
	return usIndex;
}
static unsigned short returnUartQueueLength(UARTQUEUE s_queue)
{
	short sLength = s_queue.usTail - s_queue.usHead;
	
	while(sLength < 0)
	{
		sLength += UARQUEUESIZE;
	}
	
	return sLength;
}
static void uartQueuePush(unsigned char ucQueueNum,unsigned char ucData)
{
	usrtQueueBuf[ucQueueNum].ucData[usrtQueueBuf[ucQueueNum].usTail] = ucData;
	usrtQueueBuf[ucQueueNum].usTail = uartQueueIndex(usrtQueueBuf[ucQueueNum].usTail+1);
}
