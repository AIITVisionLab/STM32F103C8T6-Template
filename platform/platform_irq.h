#ifndef PLATFORM_IRQ_H
#define PLATFORM_IRQ_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
  PLATFORM_FAULT_NMI = 0,
  PLATFORM_FAULT_HARD,
  PLATFORM_FAULT_MEMMANAGE,
  PLATFORM_FAULT_BUS,
  PLATFORM_FAULT_USAGE
} platform_fault_t;

void platform_irq_systick(void);
void platform_irq_fault(platform_fault_t fault) __attribute__((noreturn));

#ifdef __cplusplus
}
#endif

#endif /* PLATFORM_IRQ_H */
