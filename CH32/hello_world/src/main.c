/******************************************************************************
 * @file     main.c
 * @brief    CH32V00x Hello World Example
 * @version  1.0
 * @date     2026-08-31
 ******************************************************************************/

#include "debug.h"

int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    SystemCoreClockUpdate();
    Delay_Init();
#if (SDI_PRINT == SDI_PR_OPEN)
    SDI_Printf_Enable();
#else
    USART_Printf_Init(115200);
#endif
    printf("SystemClk:%d\r\n", SystemCoreClock);
    printf( "ChipID:%08x\r\n", DBGMCU_GetCHIPID() );
    printf("This is printf example\r\n");

    while(1)
    {
        Delay_Ms(1000);
        printf("Hello World\r\n");
    }
}
 