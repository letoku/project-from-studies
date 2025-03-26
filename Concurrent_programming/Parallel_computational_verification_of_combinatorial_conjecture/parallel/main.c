#include <stddef.h>
#include <malloc.h>
#include <pthread.h>

#include "common/err.h"
#include "common/io.h"
#include "common/sumset.h"
#include "concurrent_task.h"
#include "nonrecursive/own_stack.h"


#define TASKS_TAKEN 1
#define CYCLES_BETWEEN_GIVING_TASKS 5000

#define SWAP(x, y, T) do { T SWAP = x; x = y; y = SWAP; } while (0)

typedef struct ThreadArgs {
    InputData* input_data;
    Solution* best_solution;
    Stack* global_stack;
    int waiting_threads;
    pthread_mutex_t mutex; // Mutex for exclusive access to args.
    pthread_cond_t not_empty; // Signaled when global is not empty.
} ThreadArgs;

static inline ThreadArgs* thread_args_init(InputData* input_data, Solution* best_solution, Stack* global_stack) {
    ThreadArgs* args = (ThreadArgs*)malloc(sizeof(ThreadArgs));
    args->input_data = input_data;
    args->best_solution = best_solution;
    args->global_stack = global_stack;
    args->waiting_threads = 0;
    ASSERT_ZERO(pthread_mutex_init(&args->mutex, NULL));
    ASSERT_ZERO(pthread_cond_init(&args->not_empty, NULL));

    return args;
}

void thread_args_destroy(ThreadArgs* args) {
    ASSERT_ZERO(pthread_mutex_destroy(&args->mutex));
    ASSERT_ZERO(pthread_cond_destroy(&args->not_empty));
    free(args);
}

void take_tasks (ThreadArgs* args, Stack* s) {
    for (int i = 0; i < TASKS_TAKEN; i++) {
        if (!empty(args->global_stack)) {
            Task* new_t = pop(args->global_stack);
            push(s, new_t);
        }
    }
}

bool get_from_global_stack(ThreadArgs* args, Stack* stack) {
    ASSERT_ZERO(pthread_mutex_lock(&args->mutex));
    args->waiting_threads++;
    while (empty(args->global_stack)) {
        if (args->waiting_threads == args->input_data->t) {  // All threads are waiting for tasks - means that all tasks have been done.
            pthread_cond_broadcast(&args->not_empty);  // Wake up all other waitiing threads.
            break;
        }
        ASSERT_ZERO(pthread_cond_wait(&args->not_empty, &args->mutex));
    }
    if (args->waiting_threads == args->input_data->t && empty(args->global_stack)) {
        pthread_cond_broadcast(&args->not_empty);  // Wake up all other waitiing threads.
        ASSERT_ZERO(pthread_mutex_unlock(&args->mutex));
        return true;  // Stop thread.
    }
    args->waiting_threads--;
    take_tasks(args, stack);
    ASSERT_ZERO(pthread_mutex_unlock(&args->mutex));
    return false;
}

bool preprocess(ThreadArgs* args, int counter) {
    if (counter >= CYCLES_BETWEEN_GIVING_TASKS && args->waiting_threads > 0) {
        ASSERT_ZERO(pthread_mutex_lock(&args->mutex));
        return true;
    }
    return false;
}

int after_process(ThreadArgs* args, int counter, bool giving_tasks_to_global) {
    if (giving_tasks_to_global) {
        pthread_cond_broadcast(&args->not_empty);
        ASSERT_ZERO(pthread_mutex_unlock(&args->mutex));
        counter = 0;
    } else {
        counter++;
    }
    return counter;
}

int process_trivial_intersection(ThreadArgs* args, ConcurrentSumsetWrapper * a, ConcurrentSumsetWrapper * b, Stack* stack, int counter) {
    bool giving_tasks_to_global = preprocess(args, counter);

    for (size_t i = a->s->last; i <= args->input_data->d; ++i) {
        if (!does_sumset_contain(b->s, i)) {
            Sumset* a_with_i = (Sumset*)malloc(sizeof(Sumset));
            sumset_add(a_with_i, a->s, i);
            Task* new_t = init_task(init_sumset_wrapper(a_with_i, a), b);
            if (giving_tasks_to_global) {
                push(args->global_stack, new_t);
                pthread_cond_broadcast(&args->not_empty);
            } else {
                push(stack, new_t);
            }
        }
    }

    counter = after_process(args, counter, giving_tasks_to_global);
    return counter;
}

void process_non_trivial_intersection(ThreadArgs* args, Sumset* a, Sumset* b) {
    if ((a->sum == b->sum) && (get_sumset_intersection_size(a, b) == 2)) { // s(a) ∩ s(b) = {0, ∑b}.
        if (b->sum > args->best_solution->sum)
            solution_build(args->best_solution, args->input_data, a, b);
    }
}

void* solver_main(void* arguments)
{
    // init ----------------------------
    ThreadArgs* args = arguments;
    int counter = 0;
    Stack* stack = init_stack();

    // main loop -----------------------
    while (true) {
        if (empty(stack)) {
            bool should_end = get_from_global_stack(args, stack);
            if (should_end)
                break;
        }

        Task* t = pop(stack);
        ConcurrentSumsetWrapper* a = t->a;
        ConcurrentSumsetWrapper* b = t->b;
        if (is_sumset_intersection_trivial(a->s, b->s)) { // s(a) ∩ s(b) = {0}.
            counter = process_trivial_intersection(args, a, b, stack, counter);
        } else {
            process_non_trivial_intersection(args, a->s, b->s);
        }
        destroy_task(t);
    }

    destroy_stack(stack);
    return NULL;
}

void cleanup(pthread_t* solving_threads, ThreadArgs* args, Stack* global_stack, ConcurrentSumsetWrapper* a0, ConcurrentSumsetWrapper* b0) {
    free(solving_threads);
    thread_args_destroy(args);
    free(global_stack);
    free(a0);
    free(b0);
}


void solve_init(ConcurrentSumsetWrapper* a, ConcurrentSumsetWrapper* b, ThreadArgs* args) {
    if (is_sumset_intersection_trivial(a->s, b->s)) { // s(a) ∩ s(b) = {0}.
        for (size_t i = a->s->last; i <= args->input_data->d; ++i) {
            if (!does_sumset_contain(b->s, i)) {
                Sumset* a_with_i = (Sumset*)malloc(sizeof(Sumset));
                sumset_add(a_with_i, a->s, i);
                Task* new_t = init_task(init_sumset_wrapper(a_with_i, a), b);
                push(args->global_stack, new_t);
            }
        }
    } else {
        process_non_trivial_intersection(args, a->s, b->s);
    }
}

static void solve(InputData* input_data, Solution* best_solution) {
    ConcurrentSumsetWrapper* a0 = init_sumset_wrapper(&input_data -> a_start, NULL);
    a0->refs=1;
    ConcurrentSumsetWrapper* b0 = init_sumset_wrapper(&input_data -> b_start, NULL);
    b0->refs=1;
    Stack* global_stack = init_stack();

    pthread_t* solving_threads = (pthread_t *)malloc(input_data->t * sizeof(pthread_t));
    ThreadArgs* args = thread_args_init(input_data, best_solution, global_stack);
    if (a0->s->sum > b0->s->sum) {
        SWAP(a0, b0, ConcurrentSumsetWrapper*);
    }
    solve_init(a0, b0, args);

    for (int i = 0; i < input_data->t; i++) {
        ASSERT_ZERO(pthread_create(&solving_threads[i], NULL, solver_main, (void*)args));
    }

    for (int i = 0; i < input_data->t; i++) {
        pthread_join(solving_threads[i], NULL);
    }

    cleanup(solving_threads, args, global_stack, a0, b0);
}


int main()
{
    InputData input_data;
    input_data_read(&input_data);
//    input_data_init(&input_data, 1, 3, (int[]){0}, (int[]){1, 0});

    Solution best_solution;
    solution_init(&best_solution);

    // ...
    solve(&input_data, &best_solution);

    solution_print(&best_solution);
    return 0;
}
