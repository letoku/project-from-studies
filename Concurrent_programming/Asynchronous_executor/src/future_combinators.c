#include "future_combinators.h"
#include <stdlib.h>

#include "future.h"
#include "waker.h"

static FutureState then_progress_fn(Future* fut, Mio* mio, Waker waker) {
    ThenFuture* self = (ThenFuture*)fut;
    if (!self->fut1_completed) {
        FutureState fs = self->fut1->progress(self->fut1, mio, waker);
        switch (fs) {
            case FUTURE_COMPLETED:
                self->fut1_completed = true;
                self->fut2->arg = self->fut1->ok;   // passing result to fut2
                waker_wake(&waker);
                return FUTURE_PENDING;
            case FUTURE_FAILURE:
                self->base.errcode = self->fut1->errcode;
                return FUTURE_FAILURE;
            case FUTURE_PENDING:
                return FUTURE_PENDING;
        }
    } else {
        FutureState fs = self->fut2->progress(self->fut2, mio, waker);
        switch (fs) {
            case FUTURE_COMPLETED:
                self->base.ok = self->fut2->ok;
                return FUTURE_COMPLETED;
            case FUTURE_FAILURE:
                self->base.errcode = self->fut2->errcode;
                return FUTURE_FAILURE;
            case FUTURE_PENDING:
                return FUTURE_PENDING;
        }
    }
}

ThenFuture future_then(Future* fut1, Future* fut2)
{
    return (ThenFuture) {
         .base = future_create(then_progress_fn),
         .fut1 = fut1,
         .fut2 = fut2,
         .fut1_completed = false
    };
}

typedef struct JoinSubFuture {
    Future base; // Base future structure
    Future* sub_fut;
    int sub_fut_i;
    JoinFuture* j;
} JoinSubFuture;


static void wake_up_join(JoinSubFuture* self, Waker waker) {
    Waker w = {
        .future = (Future*)self->j,
        .executor = waker.executor
    };
    waker_wake(&w);
}

static FutureState join_sub_fut_progress_fn(Future* fut, Mio* mio, Waker waker) {
    JoinSubFuture* self = (JoinSubFuture*) fut;
    FutureState fs = self->sub_fut->progress(self->sub_fut, mio, waker);
    switch (fs) {
        case FUTURE_COMPLETED:
            if (self->sub_fut_i == 1) {
                self->j->fut1_completed = FUTURE_COMPLETED;
                self->j->result.fut1.ok = self->sub_fut->ok;
            } else {
                self->j->fut2_completed = FUTURE_COMPLETED;
                self->j->result.fut2.ok = self->sub_fut->ok;
            }
            if (self->j->fut1_completed != FUTURE_PENDING && self->j->fut2_completed != FUTURE_PENDING) {
                wake_up_join(self, waker);
            }
            return FUTURE_COMPLETED;
        case FUTURE_FAILURE:
            if (self->sub_fut_i == 1) {
                self->j->fut1_completed = FUTURE_FAILURE;
                self->j->result.fut1.errcode = self->sub_fut->errcode;
            } else {
                self->j->fut2_completed = FUTURE_FAILURE;
                self->j->result.fut2.errcode = self->sub_fut->errcode;
            }
            if (self->j->fut1_completed != FUTURE_PENDING && self->j->fut2_completed != FUTURE_PENDING) {
                wake_up_join(self, waker);
            }
            return FUTURE_FAILURE;
        case FUTURE_PENDING:
            waker_wake(&waker);
            return FUTURE_PENDING;
    }
}

static FutureState join_progress_fn(Future* fut, Mio* mio, Waker waker) {
    JoinFuture* self = (JoinFuture*)fut;
    static JoinSubFuture* jsf1 = NULL;
    static JoinSubFuture* jsf2 = NULL;
    FutureState fs = FUTURE_PENDING;

    if (!jsf1) {
        jsf1 = (JoinSubFuture*) malloc(sizeof(JoinSubFuture));
        jsf1->base = future_create(join_sub_fut_progress_fn);
        jsf1->sub_fut = self->fut1;
        jsf1->sub_fut_i = 1;
        jsf1->j = self;
        Waker w = {
            .future = (Future*)jsf1,
            .executor = waker.executor
        };
        waker_wake(&w);
    }

    if (!jsf2) {
        jsf2 = (JoinSubFuture*) malloc(sizeof(JoinSubFuture));
        jsf2->base = future_create(join_sub_fut_progress_fn);
        jsf2->sub_fut = self->fut2;
        jsf2->sub_fut_i = 2;
        jsf2->j = self;
        Waker w = {
            .future = (Future*)jsf2,
            .executor = waker.executor
        };
        waker_wake(&w);
    }

    if (self->fut1_completed != FUTURE_PENDING && self->fut2_completed != FUTURE_PENDING) {
        if (self->fut1_completed == FUTURE_COMPLETED && self->fut2_completed == FUTURE_COMPLETED) {
            fs = FUTURE_COMPLETED;
        } else {
            fs = FUTURE_FAILURE;
        }
        free(jsf1);
        free(jsf2);
    }

    return fs;
}

JoinFuture future_join(Future* fut1, Future* fut2)
{
    return (JoinFuture) {
         .base = future_create(join_progress_fn),
         .fut1 = fut1,
         .fut2 = fut2,
         .fut1_completed = FUTURE_PENDING,
         .fut2_completed = FUTURE_PENDING,
         .result = {
             .fut1 = {
                 .errcode = FUTURE_SUCCESS,
                 .ok = NULL
             },
             .fut2 = {
                 .errcode = FUTURE_SUCCESS,
                 .ok = NULL
             }
         }
    };
}

typedef struct SelectSubFuture {
    Future base; // Base future structure
    Future* sub_fut;
    int sub_fut_i;
    bool completed;
    SelectFuture* s;
} SelectSubFuture;


static bool check_if_wake(SelectSubFuture* self) {
    if (self->s->which_completed != SELECT_COMPLETED_NONE && !self->completed) {
        return true;
    }

    return false;
}

static void wake_up_select(SelectSubFuture* self, Waker waker) {
    Waker w = {
        .future = (Future*)self->s,
        .executor = waker.executor
    };
    self->completed = true;
    waker_wake(&w);
}

static FutureState select_sub_fut_progress_fn(Future* fut, Mio* mio, Waker waker) {
    SelectSubFuture* self = (SelectSubFuture*) fut;
    if (check_if_wake(self)) {
        wake_up_select(self, waker);
        return FUTURE_COMPLETED;
    }

    FutureState fs = self->sub_fut->progress(self->sub_fut, mio, waker);
    switch (fs) {
        case FUTURE_COMPLETED:
            self->completed = true;
            self->s->base.ok = self->base.ok;
            if (self->sub_fut_i == 1) {
                self->s->which_completed = SELECT_COMPLETED_FUT1;
            } else {
                self->s->which_completed = SELECT_COMPLETED_FUT2;
            }
            return FUTURE_COMPLETED;
        case FUTURE_FAILURE:
            self->completed = true;
            self->s->base.errcode = self->base.errcode;
            if (self->sub_fut_i == 1) {
                self->s->which_completed = SELECT_FAILED_FUT1;
            } else {
                self->s->which_completed = SELECT_FAILED_FUT2;
            }
            return FUTURE_FAILURE;
        case FUTURE_PENDING:
            waker_wake(&waker);
            return FUTURE_PENDING;
    }
}


static FutureState select_progress_fn(Future* fut, Mio* mio, Waker waker) {
    SelectFuture* self = (SelectFuture*)fut;
    static SelectSubFuture* ssf1 = NULL;
    static SelectSubFuture* ssf2 = NULL;
    FutureState fs = FUTURE_PENDING;

    if (!ssf1) {
        ssf1 = (SelectSubFuture*) malloc(sizeof(SelectSubFuture));
        ssf1->base = future_create(select_sub_fut_progress_fn);
        ssf1->sub_fut = self->fut1;
        ssf1->sub_fut_i = 1;
        ssf1->completed = false;
        ssf1->s = self;
        Waker w = {
            .future = (Future*)ssf1,
            .executor = waker.executor
        };
        waker_wake(&w);
    }

    if (!ssf2) {
        ssf2 = (SelectSubFuture*) malloc(sizeof(SelectSubFuture));
        ssf2->base = future_create(select_sub_fut_progress_fn);
        ssf2->sub_fut = self->fut2;
        ssf2->sub_fut_i = 2;
        ssf2->completed = false;
        ssf2->s = self;
        Waker w = {
            .future = (Future*)ssf2,
            .executor = waker.executor
        };
        waker_wake(&w);
    }

    if (ssf1->completed && ssf2->completed) {
        if (self->which_completed == SELECT_COMPLETED_FUT1 || self->which_completed == SELECT_COMPLETED_FUT2) {
            fs = FUTURE_COMPLETED;
        } else {
            fs = FUTURE_FAILURE;
        }
        free(ssf1);
        free(ssf2);
    }

    return fs;
}

SelectFuture future_select(Future* fut1, Future* fut2)
{
    return (SelectFuture) {
        .base = future_create(select_progress_fn),
        .fut1 = fut1,
        .fut2 = fut2,
        .which_completed = SELECT_COMPLETED_NONE
    };
}
