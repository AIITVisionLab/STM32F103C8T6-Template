#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdint.h>

#include "project_status.h"

#ifdef __cplusplus
extern "C" {
#endif

project_status_t platform_init(void);
void platform_poll(void);
uint32_t platform_get_tick_ms(void);
uint32_t platform_enter_critical(void);
void platform_exit_critical(uint32_t primask);
void platform_panic(project_status_t status) __attribute__((noreturn));

#ifdef __cplusplus
}
#endif

#endif /* PLATFORM_H */
