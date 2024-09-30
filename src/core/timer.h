#ifndef TIMER_H
#define TIMER_H
#include "../util/types.h"

typedef struct Timer {
    f32 time;
    f32 target;

    b8 on_time; // when it's on time
    b8 busy;    // when it's working
} Timer;

// rimwe_create is conflict with time.h library, so I change it to timer_init
Timer *timer_init(void);
void timer_destroy(Timer *timer);
void timer_start(Timer *timer, f32 target);
void timer_update(Timer *timer);
void timer_reset(Timer *timer);
#endif
