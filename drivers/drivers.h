#ifndef DRIVERS_H
#define DRIVERS_H

#include "project_status.h"

#ifdef __cplusplus
extern "C" {
#endif

project_status_t drivers_init(void);
void drivers_poll(void);

#ifdef __cplusplus
}
#endif

#endif /* DRIVERS_H */
