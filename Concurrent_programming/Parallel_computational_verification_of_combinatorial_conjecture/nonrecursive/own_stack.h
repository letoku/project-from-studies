#pragma once
#include "common/sumset.h"
#include "task.h"

#ifndef MULTISET_NONRECURSIVE_OWN_STACK_H_
#define MULTISET_NONRECURSIVE_OWN_STACK_H_

typedef struct StackElement {
    void* element;
    struct StackElement* next;
} SE;

typedef struct Stack {
    SE* top;
} Stack;

static inline Stack* init_stack()
{
    Stack* s = (Stack*)malloc(sizeof(Stack));
    s->top = NULL;

    return s;
}

static inline void push(Stack* s, void* t)
{
    SE* se = (SE*)malloc(sizeof(SE));
    se->element = t;
    se->next = s->top;
    s->top = se;
}

static inline void* pop(Stack* s)
{
    SE* top = s->top;
    Task* t = top->element;
    s->top = top->next;
    free(top);
    return t;
}

static inline bool empty(Stack* s)
{
    if (s->top == NULL) {
        return true;
    }
    return false;
}

static inline void destroy_stack(Stack* s) {
    while (!empty(s)) {
        SE* next = s->top->next;
        free(s->top->element);
        free(s->top);
        s->top = next;
    }
    free(s);
}


#endif // MULTISET_NONRECURSIVE_OWN_STACK_H_
