#ifndef PROJECT_STATUS_H
#define PROJECT_STATUS_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
  PROJECT_STATUS_OK = 0,
  PROJECT_STATUS_INVALID_ARG = 1,
  PROJECT_STATUS_INIT_FAILED = 2,
  PROJECT_STATUS_UNSUPPORTED = 3,
  PROJECT_STATUS_TIMEOUT = 4
} project_status_t;

#ifdef __cplusplus
}
#endif

#endif /* PROJECT_STATUS_H */
