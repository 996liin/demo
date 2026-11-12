#include "headfile.h"
static void init(void);
struct_Ftrack s_Ftrack = 
{
  .init = init,
};
struct_Ftrack *ps_Zynq = &s_Ftrack;
static void init(void)
{
  //s_Ftrack.UartSendData = ps_Usart->Usart2_SendData;
}



