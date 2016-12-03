#include <string.h>
#include <uv.h>
#include "ac5x.h"
#include "ac5x_timer.h"
#include "ac5x_debug.h"

#define MAX_TIMER 1000

typedef enum {
    AC5X_TIMER_ONCE,
    AC5X_TIMER_LOOP,
    AC5X_TIMER_TYPE_MAX
}AC5X_TIMER_TYPE;

typedef struct {
    U32 id;
    U32 type;
    BOOL isClosing;
    uv_timer_t handle;
    AC5X_TIMER_CB cb;
    void *priv;
}AC5X_TIMER;

static AC5X_TIMER s_timer[MAX_TIMER];

static void init(AC5X_TIMER *timer) {
    if (NULL == timer) {
        return;
    }

    memset(timer, 0, sizeof(AC5X_TIMER));
    timer->id = MAX_U32;
    timer->isClosing = FALSE;
    return;
}

static void on_close(uv_handle_t* handle) {
    if (NULL == handle) {
        return;
    }

    AC5X_TIMER *timer = (AC5X_TIMER *)handle->data;
    init(timer);
}

static void on_timeout(uv_timer_t* handle) {
    if (NULL == handle) {
        return;
    }

    AC5X_TIMER *timer = (AC5X_TIMER *)handle->data;
    if (NULL != timer->cb) {
        (timer->cb)(timer->priv);
    }

    if (AC5X_TIMER_ONCE == timer->type && FALSE == timer->isClosing) {
        timer->isClosing = TRUE;
        uv_timer_stop(&timer->handle);
        uv_close((uv_handle_t *)&timer->handle,on_close);
    }
}

U32 ac5x_timer_init(void) {
    U32 i;
    for (i = 0; i < MAX_TIMER; i++) {
        init(& s_timer[i]);
    }

    return AC5X_OK;
}

U32 ac5x_timer_start(U32 timeOut, U32 repeat, AC5X_TIMER_CB cb, void *priv) {
    AC5X_TIMER *timer = NULL;
    U32 i;

    for (i = 0; i < MAX_TIMER; i++) {
        if (MAX_U32 == s_timer[i].id) {
            break;
        }
    }

    if (MAX_TIMER <= i) {
        return MAX_U32;
    }

    timer = &s_timer[i];
    timer->handle.data = (void *)timer;
    timer->id = i;
    timer->cb = cb;
    timer->priv = priv;
    if (0 == repeat) {
        timer->type = AC5X_TIMER_ONCE;
    }
    else {
        timer->type = AC5X_TIMER_LOOP;
    }

    uv_timer_init(uv_default_loop(), &timer->handle);
    if (0 != uv_timer_start(&timer->handle, on_timeout, (uint64_t)timeOut, (uint64_t)repeat)) {
        uv_close((uv_handle_t *)&timer->handle, on_close);
        return MAX_U32;
    }

    AC5X_DEBUG("");
    return i;
}

void ac5x_timer_stop(U32 timerId) {
    if (MAX_TIMER <= timerId) {
        return;
    }

    AC5X_TIMER *timer = &s_timer[timerId];
    if (MAX_U32 == timer->id && TRUE == timer->isClosing) {
        return;
    }

    timer->isClosing = TRUE;
    uv_timer_stop(&timer->handle);
    uv_close((uv_handle_t *)&timer->handle, on_close);
}

