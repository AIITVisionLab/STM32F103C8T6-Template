#include "platform_msp.h"

void HAL_MspInit(void)
{
  platform_msp_init();
}

void HAL_MspDeInit(void)
{
  platform_msp_deinit();
}
