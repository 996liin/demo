#ifndef	__QUEUE_H_
#define	__QUEUE_H_

#define UARQUEUESIZE	600

typedef struct
{
	unsigned short usHead;
	unsigned short usTail;
	unsigned char ucData[UARQUEUESIZE];
	
}UARTQUEUE;

typedef struct
{
	UARTQUEUE *pUartQueue[8];
	unsigned short (*uartQueueIndex)(unsigned short usIndex);
	unsigned short (*returnUartQueueLength)(UARTQUEUE s_uartQueue);
	void (*uartQueuePush)(unsigned char ucQueueNum,unsigned char ucData);
	
}Struct_Queue;
extern Struct_Queue *ps_queue;


#endif

