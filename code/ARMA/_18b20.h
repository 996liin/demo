#ifndef __18B20_H_
#define __18B20_H_


typedef struct
{
	unsigned char (*DS18B20Init)(void);
	short (*DS18B20_Read)(void);
		
}struct_DS18B20;

extern struct_DS18B20 *ps_DS18B20;

#endif

