#ifndef SERVICES_H
#define SERVICES_H

#include "project_status.h"

#ifdef __cplusplus
extern "C" {
#endif

project_status_t services_init(void);
void services_poll(void);

#ifdef __cplusplus
}
#endif

#endif /* SERVICES_H */
