#ifndef AC5X_TIMER_H
#define AC5X_TIMER_H

typedef void (* AC5X_TIMER_CB)(void *priv);
U32 ac5x_timer_init(void);
U32 ac5x_timer_start(U32 timeOut, U32 repeat, AC5X_TIMER_CB cb, void *priv);
void ac5x_timer_stop(U32 timerId);

#endif
