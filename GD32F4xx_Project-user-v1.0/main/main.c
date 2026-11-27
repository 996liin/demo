#include "headfile.h"

unsigned char VERSION_STRING[]  = "Version: Soft-HSHV1.1  HSH_LW_IMAGEP_V2.0\r\ncompiled on " __DATE__ " at " __TIME__;

/*
 * 当前程序将 APB2 做了二分频处理，后续配置要注意（例如 TIM0、TIM7 等）。
 */
static void updateParaInit(void);

unsigned char first_power_on = 0;
unsigned char sys_init_ok = 0;
unsigned char debug_ccd_sf_param = 0;

int main(void)
{
       //__disable_irq();


    nvic_vector_table_set(NVIC_VECTTAB_FLASH, 0x8000); 
    nvic_priority_group_set(NVIC_PRIGROUP_PRE0_SUB4);
	
	ps_Systick->SystickInit(240);


	ps_AT24CXX->AT24CXXInit();
	ps_Usart->UartInit();
	updateParaInit();
	IAP_AppInit();
	ps_Usart->UartInit();


	/* 面板输入初始化；内部会通过 ps_Usart->UartInit() 初始化调试串口 */
	PanelInput_Init();
	//__enable_irq();

	ps_FWDGT->FWDGTInit(5);

	uint32_t num = 1;

    while (1)
    {
		char buffer[100];
		
		sprintf(buffer, "运行了：%d s\r\n", num++);
		ps_Usart->Uart3_SendData((unsigned char *)buffer, strlen(buffer));
		ps_Systick->delay_s(2);
		
		
		/* 处理远程升级触发命令 */
        IAP_AppProcess();
		updateParaInit();
        PanelInput_Scan();      /* 必须每 PANEL_SCAN_PERIOD_MS 调用一次 */
        PanelInput_Process();   /* 分发事件并打印串口调试信息 */

        ps_Systick->delay_ms(PANEL_SCAN_PERIOD_MS);
		ps_FWDGT->FWDGTProcess();
		
    }
}


static void updateParaInit(void)
{
   unsigned char updateFlag = IAPFlag_Read();

    if (updateFlag == IAP_FLAG_REQUEST)
    {
        if (IAPFlag_Clear())
        {
            ps_Usart->Uart3_SendData(
                (unsigned char *)"IAP FLAG CLEAR OK\r\n",
                strlen("IAP FLAG CLEAR OK\r\n")
            );
        }
        else
        {
            ps_Usart->Uart3_SendData(
                (unsigned char *)"IAP FLAG CLEAR FAIL\r\n",
                strlen("IAP FLAG CLEAR FAIL\r\n")
            );
        }
    }
}
