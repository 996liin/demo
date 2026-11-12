#ifndef _PATROL_H_
#define _PATROL_H_


typedef struct
{
	void (*PatrolParaInit)(unsigned char ucOperation);
	void (*PatrolTimer)(void);
	void (*PatrolStart)(unsigned char ucLine);
	unsigned char (*GetPatrolStatus)(void);
	void (*PatrolProcess)(MESSAGE Msg);
	
}struct_Patrol;
extern struct_Patrol *ps_Patrol;

typedef struct
{
	unsigned char ucPatrolSpeed;
	unsigned char ucPatrolTime;
	
}struct_PatrolPara;

#endif
