#pragma once
#include "common/sumset.h"
#include "concurrent_task.h"
#include <malloc.h>
#include <stdatomic.h>

#ifndef MULTISET_NONRECURSIVE_SUMSET_WRAPPER_H_
#define MULTISET_NONRECURSIVE_SUMSET_WRAPPER_H_

typedef struct ConcurrentSumsetWrapper {
    Sumset* s;
    atomic_int refs;
    struct ConcurrentSumsetWrapper* prev;
} ConcurrentSumsetWrapper;

static inline ConcurrentSumsetWrapper* init_sumset_wrapper(Sumset* a, ConcurrentSumsetWrapper* prev_sw)
{
    ConcurrentSumsetWrapper* sw = (ConcurrentSumsetWrapper*)malloc(sizeof(ConcurrentSumsetWrapper));
    sw -> s = a;
    sw -> refs = 0;
    sw -> prev = prev_sw;
    if (prev_sw != NULL) {
        atomic_fetch_add(&prev_sw -> refs, 1);
    }

    return sw;
}

static inline void destroy_sumset_wrapper(ConcurrentSumsetWrapper* sw) {
    atomic_int refs = atomic_fetch_sub(&sw->refs, 1);
    if (refs == 1) {
        destroy_sumset_wrapper(sw -> prev);
        free(sw->s);
        free(sw);
    }
}

#endif // MULTISET_NONRECURSIVE_SUMSET_WRAPPER_H_
