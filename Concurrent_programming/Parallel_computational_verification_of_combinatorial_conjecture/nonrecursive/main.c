#include <stddef.h>

#include "common/io.h"
#include "common/sumset.h"
#include "task.h"
#include "own_stack.h"

static void solve(InputData* input_data, Solution* best_solution) {
    SumsetWrapper* a0 = init_sumset_wrapper(&input_data -> a_start, NULL);
    a0->refs=1;
    SumsetWrapper* b0 = init_sumset_wrapper(&input_data -> b_start, NULL);
    b0->refs=1;
    Task* task = init_task(a0, b0);
    Stack* stack = init_stack();
    push(stack, task);

    while (!empty(stack)) {
        Task* t = pop(stack);
        SumsetWrapper * a = t->a;
        SumsetWrapper * b = t->b;

        if (is_sumset_intersection_trivial(t->a->s, t->b->s)) { // s(a) ∩ s(b) = {0}.
            for (size_t i = a->s->last; i <= input_data->d; ++i) {
                if (!does_sumset_contain(b->s, i)) {
                    Sumset* a_with_i = (Sumset*)malloc(sizeof(Sumset));
                    sumset_add(a_with_i, a->s, i);
                    Task* new_t = init_task(init_sumset_wrapper(a_with_i, a), b);
                    push(stack, new_t);
                }
            }
        } else if ((a->s->sum == b->s->sum) && (get_sumset_intersection_size(a->s, b->s) == 2)) { // s(a) ∩ s(b) = {0, ∑b}.
            if (b->s->sum > best_solution->sum)
                solution_build(best_solution, input_data, a->s, b->s);
        }

        destroy_task(t);
    }
    destroy_stack(stack);
    free(a0);
    free(b0);
}


int main()
{
    InputData input_data;
    input_data_read(&input_data);
//     input_data_init(&input_data, 12, 35, (int[]){0}, (int[]){1, 0});

    Solution best_solution;
    solution_init(&best_solution);

    // ...
    solve(&input_data, &best_solution);

    solution_print(&best_solution);
    return 0;
}
