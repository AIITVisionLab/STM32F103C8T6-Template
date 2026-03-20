#include "project_assert.h"

#include <stdint.h>

void project_assert_failed(const char *file, uint32_t line)
{
  (void)file;
  (void)line;

  __asm volatile("cpsid i");

  while (1)
  {
    __asm volatile("nop");
  }
}

void assert_failed(uint8_t *file, uint32_t line)
{
  project_assert_failed((const char *)file, line);
}
