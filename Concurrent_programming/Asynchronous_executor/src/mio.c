#include "mio.h"

#include <stdint.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <unistd.h>

#include "debug.h"
#include "executor.h"
#include "waker.h"
#include "err.h"

// Maximum number of events to handle per epoll_wait call.
#define MAX_EVENTS 64

struct Mio {
    int epoll_fd;
    struct epoll_event events[MAX_EVENTS];
    Executor* e;
};


Mio* mio_create(Executor* executor) {
    int epoll_fd = epoll_create1(0);
    ASSERT_SYS_OK(epoll_fd);
    struct Mio* m = (Mio*) malloc(sizeof(Mio));
    m->epoll_fd = epoll_fd;
    m->e = executor;

    return m;
}

void mio_destroy(Mio* mio) {
    free(mio);
}

int mio_register(Mio* mio, int fd, uint32_t events, Waker waker)
{
    debug("Registering (in Mio = %p) with fd = %d\n", mio, fd);
    struct epoll_event event;
    event.events = events;
    event.data.ptr = waker.future;

    return epoll_ctl(mio->epoll_fd, EPOLL_CTL_ADD, fd, &event);
}

int mio_unregister(Mio* mio, int fd)
{
    debug("Unregistering (from Mio = %p) fd = %d\n", mio, fd);

    return epoll_ctl(mio->epoll_fd, EPOLL_CTL_DEL, fd, NULL);
}

void mio_poll(Mio* mio)
{
    debug("Mio (%p) polling\n", mio);

    int event_count = epoll_wait(mio->epoll_fd, mio->events, MAX_EVENTS, -1);
    for (int i = 0; i < event_count; i++) {
        Waker waker = {.executor = mio->e, .future = mio->events[i].data.ptr};
        waker_wake(&waker);
    }
}
