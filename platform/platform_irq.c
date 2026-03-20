#include "platform_irq.h"

#include "platform.h"
#include "stm32f1xx_hal.h"

static volatile platform_fault_t s_last_fault = PLATFORM_FAULT_NMI;

void platform_irq_systick(void)
{
  HAL_IncTick();
}

void platform_irq_fault(platform_fault_t fault)
{
  s_last_fault = fault;
  platform_panic(PROJECT_STATUS_INIT_FAILED);
}
