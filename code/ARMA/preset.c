
#include "headfile.h"
USER_PresetCalling_TypeDef PresetCalling;


void Preset_Save(uint16_t preset_num)
{
    uint8_t buffer[10];

    buffer[0] = ps_IR->zoomvalueOffset >> 8;
    buffer[1] = ps_IR->zoomvalueOffset & 0xff;
    buffer[2] = ps_CCD->zoomvalue >> 8;
    buffer[3] = ps_CCD->zoomvalue & 0xff;
    buffer[4] = ps_IR->focusvalue >> 8;
    buffer[5] = ps_IR->focusvalue & 0xff;
    buffer[6] = ps_CCD->focusvalue >> 8;
    buffer[7] = ps_CCD->focusvalue & 0xff;

    buffer[8] =  0x5A;

    ps_AT24CXX->AT24CXX_Write(PRESET_SAVE_START_ADDR + preset_num * 16, buffer, 9);
}


void Preset_Delete(uint16_t preset_num)
{
    uint8_t buffer[10];
    buffer[8] =  0xFF;
    ps_AT24CXX->AT24CXX_Write(PRESET_SAVE_START_ADDR + preset_num * 16, buffer, 9);
}

void Preset_Delete_All(void)
{

}

uint8_t Preset_Call(uint16_t preset_num)
{
    uint8_t buffer[10];
    uint16_t u16data = 0;

    if((preset_num == 0) || (preset_num == 99))
    {
        preset_num = 0;
    }

    ps_AT24CXX->AT24CXX_Read(PRESET_SAVE_START_ADDR + preset_num * 16, buffer, 9);

    if (buffer[8] ==  0x5A)
    {
        u16data = buffer[0];
        u16data = (u16data << 8) | buffer[1];
        ps_IR->Camera_GoToTarget(0, u16data);

        u16data = buffer[2];
        u16data = (u16data << 8) | buffer[3];
        ps_CCD->Camera_GoToTarget(0, u16data);

        u16data = buffer[4];
        u16data = (u16data << 8) | buffer[5];
        ps_IR->Camera_GoToTarget(1, u16data);
    }
    else
    {
        return 0;
    }


    return 1;
}
