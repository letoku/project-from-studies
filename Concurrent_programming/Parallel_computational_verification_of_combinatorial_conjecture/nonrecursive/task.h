#pragma once
#include "common/sumset.h"
#include "sumset_wrapper.h"
#include <malloc.h>

#ifndef MULTISET_NONRECURSIVE_TASK_H_
#define MULTISET_NONRECURSIVE_TASK_H_

typedef struct Task {
    SumsetWrapper *a, *b;
} Task;

static inline Task* init_task(SumsetWrapper* a, SumsetWrapper* b) {
    Task* task = (Task *)malloc(sizeof(Task));
    if (a->s->sum <= b->s->sum) {
        task->a = a;
        task->b = b;
    } else {
        task->a = b;
        task->b = a;
    }
    task->a->refs++;
    task->b->refs++;

    return task;
}

static inline void destroy_task(Task* task) {
    destroy_sumset_wrapper(task->a);
    destroy_sumset_wrapper(task->b);
    free(task);
}

#endif // MULTISET_NONRECURSIVE_TASK_H_
