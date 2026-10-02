#include "stm32h7xx_hal.h"

static int newlib_errno_value;
int *__errno(void)
{
    return &newlib_errno_value;
}

void ExitRun0Mode(void)
{
    HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);
}
