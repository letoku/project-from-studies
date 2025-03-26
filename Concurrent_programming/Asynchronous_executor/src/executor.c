#include "executor.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "debug.h"
#include "future.h"
#include "mio.h"
#include "waker.h"

typedef struct Queue {
    size_t max_size;
    size_t size;
    size_t front;
    size_t back;
    Future** queue;
} Queue;

static Queue* create_queue(size_t max_size) {
    Queue* q = (Queue*) malloc(sizeof(Queue));
    q->max_size = max_size;
    q->size = 0;
    q->front = 0;
    q->back = 0;
    q->queue = (Future**) malloc(sizeof(Future*) * max_size);
    return q;
}

static void queue_destroy(Queue* q) {
    free(q->queue);
    free(q);
}

static Future* queue_pop(Queue* q) {
    q->size--;
    Future* f = q->queue[q->front];
    q->front = (q->front + 1) % q->max_size;
    return f;
}

static void queue_push(Queue* q, Future* f) {
    q->size++;
    q->queue[q->back] = f;
    q->back = (q->back + 1) % q->max_size;
}

/**
 * @brief Structure to represent the current-thread executor.
 */
struct Executor {
    Queue* q;
    Mio* mio;
    size_t not_done_tasks;
};

Executor* executor_create(size_t max_queue_size) {
    Executor* e = (Executor*) malloc(sizeof(Executor));
    e->q = create_queue(max_queue_size);
    e->mio = mio_create(e);
    e->not_done_tasks = 0;

    return e;
}

void waker_wake(Waker* waker) {
    Executor* e = (Executor*)waker->executor;
    queue_push(e->q, waker->future);
}

void executor_spawn(Executor* executor, Future* fut) {
    fut->is_active = true;
    queue_push(executor->q, fut);
    executor->not_done_tasks++;
}

void executor_run(Executor* executor) {
    while (executor->not_done_tasks > 0) {
        if (executor->q->size == 0) {
            mio_poll(executor->mio);
        } else {
            Future* f = queue_pop(executor->q);
            Waker w = {
                .future = f,
                .executor = executor
            };
            FutureState fs = f->progress(f, executor->mio, w);
            if (fs != FUTURE_PENDING && f->is_active) {
                executor->not_done_tasks--;
                f->is_active = false;
            }
        }
    }
}

void executor_destroy(Executor* executor) {
    queue_destroy(executor->q);
    mio_destroy(executor->mio);
    free(executor);
}
