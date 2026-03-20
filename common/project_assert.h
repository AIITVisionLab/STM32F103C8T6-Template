#ifndef PROJECT_ASSERT_H
#define PROJECT_ASSERT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void project_assert_failed(const char *file, uint32_t line);

#define PROJECT_ASSERT(expr)                                                   \
  ((expr) ? (void)0U : project_assert_failed(__FILE__, (uint32_t)__LINE__))

#ifdef __cplusplus
}
#endif

#endif /* PROJECT_ASSERT_H */
