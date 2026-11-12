
#ifndef __PRESET_H
#define __PRESET_H

#include <stdint.h>

typedef struct
{
	uint8_t	Flag;
	uint16_t ccd_zoom;
	uint16_t ccd_focus;
	uint16_t ir_zoom;
	uint16_t ir_focus;
} USER_PresetCalling_TypeDef;

extern USER_PresetCalling_TypeDef PresetCalling;

extern void Preset_Save(uint16_t preset_num);
extern void Preset_Delete(uint16_t preset_num);
extern void Preset_Delete_All(void);
extern uint8_t Preset_Call(uint16_t preset_num);

#endif /* PRESET_H */
