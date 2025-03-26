#pragma once
#include <malloc.h>
#include "common/sumset.h"
#include "task.h"

#ifndef MULTISET_NONRECURSIVE_SUMSET_WRAPPER_H_
#define MULTISET_NONRECURSIVE_SUMSET_WRAPPER_H_

typedef struct SumsetWrapper {
    Sumset* s;
    int refs;
    struct SumsetWrapper* prev;
} SumsetWrapper;

static inline SumsetWrapper* init_sumset_wrapper(Sumset* a, SumsetWrapper* prev_sw)
{
    SumsetWrapper* sw = (SumsetWrapper*)malloc(sizeof(SumsetWrapper));
    sw -> s = a;
    sw -> refs = 0;
    sw -> prev = prev_sw;
    if (prev_sw != NULL) {
        prev_sw -> refs++;
    }

    return sw;
}

static inline void destroy_sumset_wrapper(SumsetWrapper* sw) {
    sw->refs--;
    if (sw->refs == 0) {
        destroy_sumset_wrapper(sw -> prev);
        free(sw->s);
        free(sw);
    }
}

#endif // MULTISET_NONRECURSIVE_SUMSET_WRAPPER_H_
