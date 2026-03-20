#include "platform_irq.h"

void NMI_Handler(void)
{
  platform_irq_fault(PLATFORM_FAULT_NMI);
}

void HardFault_Handler(void)
{
  platform_irq_fault(PLATFORM_FAULT_HARD);
}

void MemManage_Handler(void)
{
  platform_irq_fault(PLATFORM_FAULT_MEMMANAGE);
}

void BusFault_Handler(void)
{
  platform_irq_fault(PLATFORM_FAULT_BUS);
}

void UsageFault_Handler(void)
{
  platform_irq_fault(PLATFORM_FAULT_USAGE);
}

void SVC_Handler(void)
{
}

void DebugMon_Handler(void)
{
}

void PendSV_Handler(void)
{
}

void SysTick_Handler(void)
{
  platform_irq_systick();
}
