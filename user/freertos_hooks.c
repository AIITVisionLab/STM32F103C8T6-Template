#include "FreeRTOS.h"
#include "task.h"

volatile const char *g_freertos_assert_file = NULL;
volatile int g_freertos_assert_line = 0;
volatile uint32_t g_freertos_malloc_failed = 0U;
volatile const char *g_freertos_stack_overflow_task = NULL;

static void Trap(void)
{
  taskDISABLE_INTERRUPTS();
  for (;;)
  {
  }
}

void vAssertCalled(const char *file, int line)
{
  g_freertos_assert_file = file;
  g_freertos_assert_line = line;
  Trap();
}

void vApplicationMallocFailedHook(void)
{
  g_freertos_malloc_failed++;
  Trap();
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
  (void) xTask;
  g_freertos_stack_overflow_task = pcTaskName;
  Trap();
}
