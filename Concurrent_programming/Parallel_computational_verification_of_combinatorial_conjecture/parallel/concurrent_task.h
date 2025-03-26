#pragma once
#include "common/sumset.h"
#include "concurrent_sumset_wrapper.h"
#include <malloc.h>

#ifndef MULTISET_NONRECURSIVE_TASK_H_
#define MULTISET_NONRECURSIVE_TASK_H_

typedef struct ConcurrentTask {
    ConcurrentSumsetWrapper *a, *b;
} Task;

static inline Task* init_task(ConcurrentSumsetWrapper* a, ConcurrentSumsetWrapper* b) {
    Task* task = (Task *)malloc(sizeof(Task));
    if (a->s->sum <= b->s->sum) {
        task->a = a;
        task->b = b;
    } else {
        task->a = b;
        task->b = a;
    }

    atomic_fetch_add(&task->a->refs, 1);
    atomic_fetch_add(&task->b->refs, 1);

    return task;
}

static inline void destroy_task(Task* task) {
    destroy_sumset_wrapper(task->a);
    destroy_sumset_wrapper(task->b);
    free(task);
}

#endif // MULTISET_NONRECURSIVE_TASK_H_
