#ifndef PROJEKT1_UTILS_MEMORY_MANAGEMENT_H_
#define PROJEKT1_UTILS_MEMORY_MANAGEMENT_H_

#include <errno.h>
#include <malloc.h>

#define MEMORY_ALLOCATION_CHECK(pointer)                                       \
  {                                                                            \
    if (!pointer) {                                                            \
      errno = ENOMEM;                                                          \
      return NULL;                                                             \
    }                                                                          \
  }

#endif // PROJEKT1_UTILS_MEMORY_MANAGEMENT_H_
