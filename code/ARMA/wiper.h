
#ifndef __WIPER_H
#define __WIPER_H

#define FLAG_WIPER
typedef struct
{
	void (*init)(void);
	void (*Process)(void);
	unsigned char open_flag;
	unsigned char switch_flag;
	unsigned char selfcheck_flag;
	unsigned char ctrl_timeout_flag;
}_tWiper;
extern _tWiper *ptr_Wiper;


#endif /* __WIPER_H */
