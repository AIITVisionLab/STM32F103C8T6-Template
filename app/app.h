#ifndef APP_H
#define APP_H

#include "project_status.h"

#ifdef __cplusplus
extern "C" {
#endif

project_status_t app_init(void);
void app_poll(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_H */
